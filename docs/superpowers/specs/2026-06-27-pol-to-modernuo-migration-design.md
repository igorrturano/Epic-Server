# POL → ModernUO Migration Design

> **Status:** Design aprovado. Pré-implementação.
> **Depende de:** `2026-06-22-core-engine-architecture-design.md` (fronteira Core/Engine, MVVM, adapter pattern).
> **Escopo:** Estratégia de migração completa — estrutura de projetos, organização de pastas, ordem de execução do slice vertical.

---

## 1. Contexto e decisões fechadas

- **Migração full port.** POL fecha. Sem execução paralela de shards.
- **Clean start (wipe).** Sem migração de dados de jogadores.
- **Cliente epicuo inalterado.** O protocolo custom de packets permanece byte-idêntico — só reimplementamos o lado servidor em C#.
- **Character creation no cliente.** O servidor recebe a vocação escolhida e aplica; não há mais pedra de migração nem gump server-side de criação.
- **Combat e Craft em rework.** Não é port 1:1 — são redesigns. O slice vertical provê o ambiente de teste cedo para validar as decisões de design com feedback real.
- **Abilities redesenhadas** como escolhas (não tarefas), alinhadas aos princípios de game design do Epic.

Referência para o "o quê migrar": `docs/epic-systems-map.md` — vereditos CORE/REWORK/CORTAR para todos os 51 sistemas.

---

## 2. Estrutura de projetos

Dois projetos novos adicionados à solution ModernUO existente:

```
Projects/
  Server/           ← Engine ModernUO. Nunca tocar.
  UOContent/        ← Conteúdo vanilla UO. Estender, não forkar.
  Application/      ← Entry point ModernUO. Nunca tocar.
  Logger/           ← Nunca tocar.
  Game.Core/        ← NOVO. Regras puras de jogo. Zero referências de engine.
  Epic.Content/     ← NOVO. Camada adapter. Referencia Game.Core + Server.
```

### Regra de dependência (enforced pelo compilador)

```
Server ← Epic.Content → Game.Core
              ↑
         UOContent (vanilla, estendido)
```

`Game.Core` não referencia nada fora do BCL. Se qualquer arquivo dentro dele tentar `using Server.Mobile` ou similar — **não compila**. A disciplina de separação é imposta pelo compilador, não pela memória de ninguém.

### O que vai onde

| Tipo de código | Projeto |
|---|---|
| Regra pura (fórmula de hit, cooldown, XP) | `Game.Core` |
| Item custom (`SteamArmor : BaseArmor`) | `Epic.Content/Items/` |
| Mobile custom (`EpicPlayer : PlayerMobile`) | `Epic.Content/Mobiles/` |
| Packet handler custom (`0xD7` subcommands) | `Epic.Content/Network/Packets/` |
| Lifecycle hooks (logon, death, equip…) | `Epic.Content/Hooks/` |
| Sistema adapter (lê Mobile, chama Game.Core, aplica resultado) | `Epic.Content/Systems/` |
| Adapter de tipo (`Position2D ↔ Point3D`) | `Epic.Content/Adapters/` |

---

## 3. Estrutura de pastas — `Game.Core`

Regra de ouro: **o Core decide o quê acontece. A cola aplica e mostra.**

Tudo aqui opera em *snapshots* (structs passados como entrada) e devolve *results* (structs de saída). A engine lê o resultado e aplica. Nenhuma referência a `Mobile`, packet, tile, animação.

```
Game.Core/
  Combat/
    Models/        CombatSnapshot.cs, DamageResult.cs, HitResult.cs
    Rules/         CombatRules.cs, DarknessHitModifier.cs, ArmorRules.cs
    Interfaces/    ICombatant.cs, ICombatView.cs

  Attributes/
    Models/        AttributeSnapshot.cs
                   └─ STR/DEX/INT, HP/Mana/Stamina/DeathPoints
                      Fortitude/Reflexes/Will (intrínsecos)
                      Addiction STR/DEX/INT, Hunger, Level, Skills
    Rules/         AttributeRules.cs
    Interfaces/    IAttributeHolder.cs

  Abilities/
    Models/        AbilityDefinition.cs, AbilityResult.cs
    Rules/         AbilityRules.cs, CooldownRules.cs
    Interfaces/    IAbilityUser.cs

  Spells/
    Models/        SpellDefinition.cs, CastResult.cs
    Rules/         SpellRules.cs, CastRules.cs
    Interfaces/    ISpellCaster.cs

  Progression/
    Models/        ProgressionSnapshot.cs   (XP, level, vocation)
    Rules/         ProgressionRules.cs
    Interfaces/    IProgressable.cs

  Identity/
    Models/        IdentitySnapshot.cs   (nome real, estado de disfarce, facção)
    Rules/         IdentityRules.cs      (IsIncognito, CanReveal, DisplayName)
    Interfaces/    IIdentifiable.cs

  Environment/
    Models/        EnvironmentSnapshot.cs   (nível de luz, hora, clima)
    Rules/         DarknessRules.cs
    Interfaces/    IEnvironmentContext.cs

  Shared/
    Position2D.cs
    Enums/         DamageType.cs, VocationId.cs, SchoolOfMagic.cs, ResistType.cs
```

### Exemplos de onde o código "espalhado" aterra

| Código (POL) | Aterra em |
|---|---|
| `DarknessHitModifier` em `HitController.inc` | `Game.Core/Combat/Rules/DarknessHitModifier.cs` |
| Regra de Incognito/Desconhecido | `Game.Core/Identity/Rules/IdentityRules.cs` |
| Fórmula de dano, cálculo de armor | `Game.Core/Combat/Rules/CombatRules.cs` |
| Regra de XP/level-up | `Game.Core/Progression/Rules/ProgressionRules.cs` |

---

## 4. Estrutura de pastas — `Epic.Content`

Tudo que é nosso e tem dependência de engine. Subclasses de `BaseArmor`, `PlayerMobile`, packet handlers, hooks.

```
Epic.Content/
  Items/
    Armor/         SteamArmor.cs, ...
    Weapons/
    Containers/
    Misc/

  Mobiles/
    EpicPlayer.cs           (: PlayerMobile)
    EpicCreature.cs         (: BaseCreature)

  Network/
    Packets/
      Hotbar/
        HotbarRequestHandler.cs      (0xD7:0x50 — client pede hotbar)
        HotbarExecuteHandler.cs      (0xD7:0x51 — client executa ação)
        HotbarAbilitiesPacket.cs     (0xD7:0x53 — server envia abilities)
        HotbarSpecialsPacket.cs      (0xD7:0x54)
        HotbarSpellsPacket.cs        (0xD7:0x55)
        HotbarSkillsPacket.cs        (0xD7:0x56)
        HotbarCommandsPacket.cs      (0xD7:0x59)
        HotbarTooltipHandler.cs      (0xD7:0x60/0x61)
      Experience/
        ExperiencePacket.cs          (0xD7:0x57)
      Combat/
        CooldownPacket.cs
        AoeRangePacket.cs            (0xD7:0x5A)
      Buffs/
        CustomBuffPacket.cs          (0xD7:0x62)
      Equipment/
        EquipmentInfoPacket.cs       (0xD7:0x63)
      Fog/
        FogControlPacket.cs          (0xD7:0x0070)
      Stats/
        ExtendedStatsPacket.cs       (0x11 override — stats window completo)
      Handshake/
        RazorHandshake.cs            (0xF0)
        HwidHandler.cs

  Hooks/
    LifecycleHooks.cs      (logon, logoff, oncreate, ondelete, reconnect)
    DeathHooks.cs          (chrdeath, death/respawn)
    EquipHooks.cs          (equiptest, unequiptest)
    SpeechHooks.cs         (speech pipeline, shownames)

  Systems/
    Combat/
      CombatAdapter.cs     (lê Mobile → CombatSnapshot, chama CombatRules, aplica DamageResult)
      DarknessAdapter.cs   (lê ambiente → EnvironmentSnapshot, chama DarknessRules)
    Abilities/
      AbilityDispatcher.cs
    Spells/
      SpellDispatcher.cs
    Faccao/
    Nature/
      NatureSystem.cs      (dia/noite, clima, hazards regionais)
    Loot/
    Crafting/
    Entropy/
    Progression/
      ProgressionAdapter.cs   (envia ExperiencePacket quando XP muda)
    Disguise/
      IncognitoSystem.cs      (aplica/remove disfarce, integra com SpeechHooks + ExtendedStatsPacket)

  Adapters/
    PositionAdapter.cs         (Position2D ↔ Point3D)
    MobileAdapter.cs           (Mobile → AttributeSnapshot)
    EnvironmentAdapter.cs      (estado do mundo → EnvironmentSnapshot)
```

---

## 5. Protocolo custom — inventário completo

O cliente epicuo permanece inalterado. `Epic.Content/Network/Packets/` reimplementa o mesmo protocolo em C#.

### Server → Client (0xD7 — GenericAOSCommands)

| Subcomando | Propósito |
|---|---|
| `0x52` | Hotbar completo (dump inicial) |
| `0x53` | Abilities (por vocação) |
| `0x54` | Combat specials |
| `0x55` | Spells |
| `0x56` | Skills ativas |
| `0x57` | XP / Level bar |
| `0x59` | Comandos de texto |
| `0x5A` | AoE range preview |
| `0x61` | Tooltip response |
| `0x62` | Custom buff icons |
| `0x63` | Equipment info update |
| `0x0070` | Atmospheric fog control |

### Client → Server (0xD7)

| Subcomando | Propósito |
|---|---|
| `0x50` | Request hotbar data |
| `0x51` | Execute action (posture/ability/special/spell/command/skill) |
| `0x60` | Tooltip request |

### Outros custom

| Packet | Propósito |
|---|---|
| `0xF0` | Razor handshake |
| HWID | Hardware fingerprint |
| `0x11` (override) | Extended stats window — STR/DEX/INT/vitais + Fortitude/Reflexes/Will + Death Points + Addiction + Hunger + Level |
| `0xD6` (MegaCliloc) | Tooltip overrides para itens |

### Campos custom do stats packet (`0x11`)

Além dos campos vanilla UO, o override envia:

| Campo | Tipo | Nota |
|---|---|---|
| `Fortitude` | Int8 | Intrínseco derivado de STR |
| `Reflexes` | Int8 | Intrínseco derivado de DEX |
| `Will` | Int8 | Intrínseco derivado de INT |
| `DeathPoints` (atual) | Int16 | Recurso de permadeath/ressurreição |
| `DeathPoints` (max) | Int16 | |
| `Addiction STR/DEX/INT` | Int8 ×3 | Sistema de drogas |
| `Hunger` | Int8 | |
| `Level` | Int8 | |
| `Gold` | Int32 | |
| Nome | String | Passa por `IdentityRules.DisplayName()` — mostra "Desconhecido" se mascarado |

---

## 6. Código "espalhado" — mapa de captura

Código que não vivia em sistemas nomeados no POL e onde aterra na nova arquitetura:

| Origem (POL) | Destino |
|---|---|
| `scripts/misc/*.src` — 24 hooks globais de lifecycle | `Epic.Content/Hooks/` |
| `scripts/include/*.inc` — includes globais de utilitário | Divididos: lógica pura → `Game.Core/Shared/`, glue de engine → `Epic.Content/Adapters/` |
| `HitController.inc` — `DarknessHitModifier` | `Game.Core/Combat/Rules/DarknessHitModifier.cs` |
| `pkg/items/disguise/` — lógica de Incognito/Desconhecido | Regra → `Game.Core/Identity/Rules/IdentityRules.cs`; aplicação → `Epic.Content/Systems/Disguise/IncognitoSystem.cs` |
| `pkg/packetHooks/` — todos os packet handlers | `Epic.Content/Network/Packets/` |

---

## 7. Plano de execução — Slice Vertical (Approach B)

### Fase 0 — Scaffold (sem lógica de jogo)

Criar `Game.Core.csproj` e `Epic.Content.csproj`, adicionar à solution, criar todas as pastas e arquivos placeholder com namespaces corretos. Nenhuma lógica de jogo ainda — só a estrutura que enforce as regras de dependência.

Entregável: compilar com `dotnet build` sem erros. O compilador já rejeita imports errados.

---

### Step 1 — Conexão e handshake

**O que porta:** `RazorHandshake.cs` (`0xF0`), `HwidHandler.cs`, `LifecycleHooks.cs` (logon).

**Teste E2E:** Player conecta, aparece no mundo, sem crashes.

---

### Step 2 — Stats window completo

**O que porta:**
- `AttributeSnapshot` em `Game.Core/Attributes/Models/`
- `IdentityRules.DisplayName()` em `Game.Core/Identity/Rules/`
- `MobileAdapter` em `Epic.Content/Adapters/`
- `ExtendedStatsPacket` (`0x11` override) em `Epic.Content/Network/Packets/Stats/`

**Campos cobertos:** STR/DEX/INT, HP/Mana/Stamina, Fortitude/Reflexes/Will, Death Points, Addiction, Hunger, Level. Nome com regra de Incognito.

**Teste E2E:** Player abre paperdoll, vê todos os stats corretos. Outro player vê "Desconhecido" ao clicar em alguém mascarado.

---

### Step 3 — Hotbar + protocolo completo de packets

**O que porta:** Todos os handlers `0xD7` (request `0x50`, execute `0x51`, responses `0x52–0x59`, tooltip `0x60/0x61`, buffs `0x62`, equipment `0x63`).

**Neste step:** Botões da hotbar aparecem com stubs — existem visualmente, clicar não executa lógica real ainda. O objetivo é validar o protocolo inteiro.

**Teste E2E:** Hotbar popula ao logar, todos os subcomandos chegam e saem sem erro de protocolo.

---

### Step 4 — Combat slice básico

**O que porta:**
- `CombatRules`, `DarknessHitModifier`, `ArmorRules` em `Game.Core/Combat/Rules/`
- `CombatAdapter`, `DarknessAdapter` em `Epic.Content/Systems/Combat/`
- Um tipo de arma em `Epic.Content/Items/Weapons/`
- `CooldownPacket`, `AoeRangePacket`

**Teste E2E:** Player equipa arma, ataca dummy, toma dano, vê números de hit. Cooldown packet dispara. `DarknessHitModifier` aplica penalidade em área escura.

Este é o ponto onde o **rework de combate começa a ter feedback real.**

---

### Step 5 — Death + respawn

**O que porta:** `DeathHooks.cs` (`chrdeath` equivalente), lógica de Death Points, ressurreição.

**Teste E2E:** Loop completo — lutar → morrer (DP zera) → respawn → lutar de novo.

---

### Step 6 — Progression + ExpBar

**O que porta:**
- `ProgressionRules` em `Game.Core/Progression/Rules/`
- `ProgressionAdapter` em `Epic.Content/Systems/Progression/`
- `ExperiencePacket` (`0xD7:0x57`)

**Teste E2E:** Matar dummy concede XP, barra de XP move no cliente.

---

### Step 7+ — Expansão por sistema

Ordem por dependência:

1. **Abilities** (redesenhadas, choice-based) + `AbilityDispatcher`
2. **Spells** — motor de conjuração, escolas (Evocação, Necromancy, Theology, Nature, Science)
3. **Faccao** — guildas, território, lei, contratos
4. **Nature** — dia/noite, clima, hazards regionais
5. **Loot** — tabelas de drop, geração de item mágico
6. **Crafting** (rework quase completo)
7. **Entropy + VoidRift**
8. **Disguise/Incognito** (integração completa com Identity)
9. **Fog regions** (`FogControlPacket`)
10. Sistemas UTILITY restantes — spawning, duel, achievements, etc.

---

## 8. O que NÃO migrar

Sistemas com veredito **CORTAR** no systems map (`epic-systems-map.md`):

| Sistema POL | Motivo |
|---|---|
| `spawnSystem` | Diretório vazio, zero referências |
| `collections` | Stub de 1 LOC, sem uso |
| `contextMenus` | Só stubs, menus reais são inline |
| `language` | Localização nunca materializou |
| `ultimalive` | Só binários, sem source |
| `ageSystem` | `Enabled 0`, nunca deployado |
| `mastery` (impl atual) | Stub morto — o slot é redesenhado em Abilities |

Infra POL-específica que dissolve naturalmente:
- `pkg/fls/*` — substituído pelos built-ins do ModernUO
- `pkg/systems/wrappers/*` — substitui pela hierarquia de classes C# (`EpicPlayer`, `EpicCreature`, etc.)
- `pkg/packetHooks/` — substituído por `Epic.Content/Network/Packets/`
- `scripts/misc/*.src` — substituído por `Epic.Content/Hooks/`

---

## 9. Fora de escopo (specs separados)

1. **Design final do combat rework** — decisão em andamento; Step 4 provê o ambiente de teste.
2. **Design final do craft rework** — quase completo redesign; Step 7 quando o design estiver fechado.
3. **Abilities choice-based** — specs por vocação já existem em `docs/superpowers/specs/2026-06-19-*.md` a `2026-06-21-*.md`.
4. **Character creation no cliente** — spec separado para `epicuo` + protocolo de aplicação server-side.
5. **ModernUO limpo de pé** — instalar .NET 10, clonar, `publish.sh`, conectar com epicuo. Tarefa independente que não bloqueia o design.
