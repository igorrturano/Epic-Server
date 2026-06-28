# Character Creation Design — POL → ModernUO

> **Status:** Design aprovado. Pré-implementação.
> **Depende de:** `2026-06-27-pol-to-modernuo-migration-design.md` (estrutura Epic.Content/Game.Core, protocolo 0xD7)
> **Depende de:** `2026-06-17-vocation-system-design.md` (7 vocações, árvores, source skills)
> **Escopo:** Client-side wizard (epicuo) + packet 0xD7:0xA0 + server-side validation e aplicação (Epic.Content).

---

## 1. Decisões fechadas

- O cliente coleta todas as escolhas localmente e envia **um único packet atômico** `0xD7:0xA0`.
- O servidor valida as escolhas e deriva todos os números — o cliente nunca envia stats derivados.
- Nome e gênero são definidos pelo flow vanilla de criação de UO antes deste flow começar — não repetem no packet.
- Sem migração de dados. Clean start.
- `ItemGranter` é stub com TODO — o sistema de equipamentos iniciais será redesenhado separadamente.

---

## 2. Flow do wizard — cliente (epicuo)

### Estrutura

**`CharacterCreationGump`** — container raiz. Exibe uma tab bar no topo com todos os passos; passos completados são clicáveis (navegação livre para trás), passos futuros estão bloqueados. Gerencia navegação via `GoTo(step)`, `Advance()`, `Back()`.

**`CharacterDraft`** — classe C# com todos os campos nullable. Vive no gump; todos os painéis de passo leem e escrevem nele. Serializado diretamente para o packet no Confirm.

**`ConflictResolver`** — classe estática com o grafo de dependências hardcoded como dicionário: `field → List<(dependentField, Predicate<CharacterDraft>)>`. Quando qualquer campo muda, o resolver executa o grafo e chama `draft.Clear(dependentField)` para cada entrada cuja predicate retorna true.

**`ICreationStep`** — interface implementada por cada painel de passo:
- `Populate(draft)` — preenche a UI com o estado atual do draft
- `Commit(draft)` — escreve as seleções da UI no draft
- `IsComplete(draft)` — gate para o botão Advance

### Ordem dos passos (13)

| # | Passo | Escreve em `CharacterDraft` |
|---|---|---|
| 1 | `CultureStep` | `CultureId` |
| 2 | `RaceStep` | `RaceId` |
| 3 | `PathStep` | `PathId` |
| 4 | `VocationStep` | `VocationId` |
| 5 | `TraitStep` | `TraitId` |
| 6 | `SkillStep` | `SkillIds[]`, `SlotCount` |
| 7 | `AttributeStep` | `StrSpend`, `DexSpend`, `IntSpend` |
| 8 | `ReligionStep` | `ReligionId` |
| 9 | `LooksStep` | `HairId`, `HairColor`, `BeardId`, `BeardColor`, `SkinColor` |
| 10 | `SignStep` | `SignId` |
| 11 | `BirthdayStep` | `BirthdayDay` |
| 12 | `RPTraitsStep` | `HeightId`, `WeightId`, `EyesId`, `VoiceId`, `Age` |
| 13 | `ConfirmStep` | (read-only — envia packet) |

`ConfirmStep` é um resumo de todas as escolhas. Clicar em Confirm serializa `CharacterDraft` em `0xD7:0xA0` e envia. Se o servidor rejeitar, um modal de erro reabre o gump no passo com problema.

### Grafo de invalidação (`ConflictResolver`)

Mudança de campo invalida campos dependentes **somente se a escolha anterior deixou de ser válida**. Campos sem dependência downstream nunca são tocados.

| Campo alterado | Invalida | Condição |
|---|---|---|
| `CultureId` | `RaceId` | raça não disponível para a nova cultura |
| `RaceId` | `TraitId` | trait é racial e não existe para a nova raça |
| `RaceId` | `HeightId`, `WeightId` | opções são race-specific |
| `RaceId` | `HairId`, `BeardId`, `SkinColor` | opções de aparência incompatíveis |
| `VocationId` | `SkillIds[]` | algum slot tinha a source skill da vocação anterior |
| `PathId` | `SkillIds[]` | algum slot veio do bucket errado |
| `TraitId` (toggle Sabe-Tudo) | `SkillIds[]` | contagem de slots muda (6 → 7), re-pick obrigatório |
| `SignId` | `BirthdayDay` | dia escolhido > dias do mês do novo signo |

Campos independentes (nunca invalidados por outra escolha): `StrSpend`, `DexSpend`, `IntSpend`, `ReligionId`, `EyesId`, `VoiceId`, `Age`, `HairColor`, `BeardColor`.

---

## 3. Descrições e tooltips inline

### Regra global

Todo nome de entidade do jogo que apareça numa descrição é um **token de tooltip**. Nenhum nome aparece sem um. Esta regra se aplica ao jogo inteiro — criação de personagem, hotbar, paperdoll, descrições de habilidades.

### Formato de token

Strings de descrição usam `[[type:key]]`:

```
"Unlocks the [[trait:cavar_buracos]] racial trait."
```

O renderer parseia tokens e os exibe como spans sublinhados e hover-able. Ao passar o mouse, um `TooltipPopup` exibe a definição canônica.

### `ClientTooltipRegistry`

Carregado no startup a partir do string table. Cobre todas as entidades nomeáveis:

| Namespace | Exemplo de key | Conteúdo |
|---|---|---|
| `trait` | `cavar_buracos` | +2 ores por extração de mina |
| `skill` | `theology` | Source skill do Devoto. Governa conjuração divina… |
| `ability` | `conjuracao_divina` | Concede acesso ao grimório divino… |
| `vocation` | `devoto` | Descrição canônica da vocação |
| `resist` | `fortitude` | Resiste veneno, doença, stuns físicos… |

Na criação de personagem, todos os tooltips são resolvidos **client-side** a partir de `en.json` — sem round-trip ao servidor, sem personagem ainda. No resto do jogo, conteúdo dinâmico (stats de item, valores de proc) usa `0xD7:0x60/0x61` server-side conforme já definido.

### Estrutura de cada opção selecionável

Todo `IChoiceOption` carrega um `string Description` com exatamente três seções:

1. **Flavor** — uma frase
2. **Grants** — bônus de stat, skills desbloqueadas, habilidades concedidas — todos com números
3. **Restrictions** — o que esta escolha fecha

**Exemplo — Devoto:**
> *"Servants of Aghar who draw power from divine covenant."*
> Unlocks **[[skill:theology]]** as your source of power. Grants **[[ability:conjuracao_divina]]** and a divine spellbook at creation.
> Removes Arcana, Nature, Science, and Regência as skill options.

**Exemplo — Anão:**
> *"Stout people of the mountain halls."*
> **+10 STR**, **+10 max HP** (Badûran culture). Unlocks the **[[trait:cavar_buracos]]** racial trait.
> Native to Badûran; freely available in Aiglana and Leste.

Keys do string table: domínio `"creation"` — e.g. `"creation.vocation.devoto.description"`. Tooltips: domínio `"tooltip"` — e.g. `"tooltip.trait.cavar_buracos"`.

---

## 4. Packet `0xD7:0xA0` — payload

```
0xD7          — packet ID
ushort        — total length
uint          — mobile serial
ushort 0x00A0 — subcommand

// Identity
byte   VocationId       // 0–6
byte   CultureId        // 0–7
byte   RaceId           // 0–5
byte   PathId           // 0=Military 1=Civil
byte   TraitId

// Skills
byte   SlotCount        // 6 ou 7 (Sabe-Tudo)
byte[] SkillIds         // SlotCount bytes, cada um é um SkillId

// Attribute spend (80 pontos distribuídos, incrementos de 5)
byte   StrSpend
byte   DexSpend
byte   IntSpend

// Flavor
byte   ReligionId       // 0–7
byte   SignId           // 0–11
byte   BirthdayDay      // 1–31 (server validates against days in sign's month)

// RP traits (opções race-specific)
byte   HeightId         // 0–4
byte   WeightId         // 0–4
byte   EyesId           // 0–4
byte   VoiceId          // 0–2
ushort Age

// Aparência
ushort HairId           // 0 = sem cabelo
ushort HairColor
ushort BeardId          // 0 = sem barba (obrigatório para fêmea, Elfo, Drow)
ushort BeardColor
ushort SkinColor
```

**~37 bytes de payload.**

### O que o servidor deriva (nunca enviado pelo cliente)

| Campo derivado | De |
|---|---|
| STR/DEX/INT finais | `40 + spend + race_bonus + trait_bonus` |
| HP | `(FinalSTR + vocation_die) × 100` vital-points no Level 1 |
| Mana | derivado de INT |
| Stamina | derivado de DEX |
| Fortitude / Reflexes / Will | intrínsecos dos stats finais |
| Valores iniciais de skill | Primary slots → 40, Secondary → 20 |
| Source skill | 40 (só presente se vocação permitir) |
| Línguas | padrão cultural + bônus de INT |
| Equipamentos iniciais | stub — veja seção 5 |
| Level | sempre 1 |
| `createdate` | timestamp do servidor |
| `citizenship` | sempre Plebeu |

---

## 5. Servidor — validação (`Game.Core`)

### Estrutura de arquivos

```
Game.Core/
  CharacterCreation/
    Models/
      CharacterCreationRequest.cs
      CreationValidationResult.cs
    Rules/
      CharacterCreationValidator.cs
      CultureRaceMatrix.cs
      VocationSkillMatrix.cs
      TraitEligibilityRules.cs
```

### Regras de validação (`CharacterCreationValidator`)

Executa como chain de checks. Primeira falha short-circuits com um typed error code.

| Check | Regra |
|---|---|
| VocationId | 0–6 |
| CultureId | 0–7 |
| RaceId | válida para CultureId via `CultureRaceMatrix` |
| TraitId | válida para race + culture + vocation via `TraitEligibilityRules` |
| SlotCount | 6 normalmente; 7 somente se TraitId == Sabe-Tudo |
| SkillIds | sem duplicatas; todos no bucket correto para PathId; source skills somente se VocationId permitir via `VocationSkillMatrix`; máximo uma source skill |
| Attribute spend | StrSpend + DexSpend + IntSpend ≤ 80; cada divisível por 5; cada ≥ 0 |
| ReligionId | 0–7 |
| SignId | 0–11 |
| BirthdayDay | 1 ≤ day ≤ dias do mês do signo |
| RPTraits | HeightId/WeightId 0–4 válidos para RaceId; EyesId 0–4; VoiceId 0–2; Age ≤ máximo racial |
| Aparência | BeardId deve ser 0 se fêmea, Elfo ou Drow; HairId/SkinColor válidos para RaceId |

`CreationValidationResult` — lista de `(ErrorCode, FieldName)`. Em qualquer erro, `CharacterCreationErrorPacket` é enviado e o cliente reabre o gump no passo com problema.

---

## 6. Servidor — aplicação (`Epic.Content`)

### Estrutura de arquivos

```
Epic.Content/
  Network/Packets/CharacterCreation/
    CharacterCreationHandler.cs       ← handler 0xD7:0xA0
    CharacterCreationErrorPacket.cs
  Systems/CharacterCreation/
    CharacterCreationSystem.cs        ← orquestra a aplicação
    AttributeDeriver.cs
    SkillApplier.cs
    ItemGranter.cs                    ← stub com TODO
    AppearanceApplier.cs
```

### `CharacterCreationSystem` — ordem de aplicação

1. **`AttributeDeriver`** — computa `FinalSTR = 40 + StrSpend + RaceBonus + TraitBonus`, idem DEX/INT. Aplica via setters de `EpicPlayer`. Deriva HP `= (FinalSTR + VocationDie) × 100` vital-points. Deriva Mana de INT, Stamina de DEX, Fortitude/Reflexes/Will como intrínsecos.

2. **`SkillApplier`** — Primary slots → 40, Secondary → 20. Source skill (se houver) já é Primary slot.

3. **`AppearanceApplier`** — cria itens de cabelo/barba nas layers corretas, define skin color, escreve RP traits (height, weight, eyes, voice). Birthdate computado server-side a partir de sign + day + age.

4. **`ItemGranter`**
   ```csharp
   // TODO: Starting equipment system — research industry patterns (BG3, PoE2, Owlcat)
   // before implementing. Current POL behavior grants armor to mages which is wrong.
   // Candidates: point-buy starter shop, curated kits per vocation, hybrid.
   public static void Grant(EpicPlayer player, CharacterCreationRequest request) { }
   ```

5. **Stamps finais** — `Level = 1`, `createdate = DateTime.UtcNow`, `citizenship = Plebeu`, `creation_completed = true`. Línguas derivadas de culture + FinalINT.

6. **World entry** — teleport para `(1407, 1062, -4, britannia_alt)`. Envia `HotbarAbilitiesPacket` com a special ability de vocação. Envia `ExtendedStatsPacket` para o cliente ver os stats corretos imediatamente.

---

## 7. Fora de escopo (specs separados)

- **Sistema de equipamentos iniciais** — `ItemGranter` é stub. Pesquisar padrões da indústria antes de implementar.
- **Conteúdo das descrições e tooltips** — strings de `en.json`/`pt.json` preenchidas por passo de conteúdo separado.
- **Árvores de habilidade e level-up** — ver `docs/ability-pipeline.md`.
- **Migração de personagens existentes** — wipe total, sem migração.
