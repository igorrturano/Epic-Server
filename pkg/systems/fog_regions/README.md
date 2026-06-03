# Sistema de Névoa Regional — Camada E

Sistema GM para criar e gerenciar regiões de névoa atmosférica no mundo do Epic Server. Cada região é um retângulo definido por NW → SE com 11 parâmetros visuais (densidade, raio, cor, etc) e duração (permanente ou timer). Players que entram nas regiões recebem automaticamente o packet 0x0070 v4 (Camada B) renderizando a névoa configurada no cliente customizado (EpicUO).

![Visão geral do sistema](docs/img/fog-overview.png)

## Visão rápida

```
GM comando .fogregion → Wizard de 3 passos → Região cadastrada
                                                      ↓
              Watcher per-player (sleep 2s) ← Player entra na área
                                ↓
              Packet 0x0070 v4 → Cliente renderiza névoa
```

## Uso pelo GM

Comando: `.fogregion`

Abre o menu principal com 3 opções:
- **Criar nova região** — wizard guiado de 3 passos
- **Listar regiões** — visualizar, editar, parar, ativar ou deletar regiões existentes
- **Onde estou agora** — debug rápido: mostra posição atual e qual fog region cobre o ponto

![Menu principal](docs/img/tela-1-menu.png)

### Fluxo de criação

**Passo 1 — Nome e canto NW.** GM digita nome, anda até o canto noroeste do retângulo, clica em "Marcar posição atual como NW".

![Wizard passo 1](docs/img/tela-2-passo1.png)

**Passo 2 — Canto SE.** GM anda até o canto oposto, clica em "Marcar posição atual como SE". Sistema valida que NW < SE e mesmo realm. Mostra dimensão calculada (74 × 70 tiles).

![Wizard passo 2](docs/img/tela-3-passo2.png)

**Passo 3 — Escolher configuração.** Duas opções:
- **Preset pronto** — 6 atmosferas pré-curadas
- **Editor manual** — controle direto das 11 variáveis

![Wizard passo 3](docs/img/tela-4-passo3.png)

### Presets prontos

| Preset | Atmosfera |
|---|---|
| `atmosfera_padrao` | Bruma sutil, neutra |
| `noite_profunda` | Densa noturna, quase preta |
| `bosque_verde_musgo` | Floresta úmida pantanosa |
| `sepia_arido` | Tom árido de deserto/ruínas |
| `vinheta_envolvente` | Cinematográfica, foco no player |
| `vale_enevoado` | Névoa rasteira, chão coberto (vmin > vmax) |

![Tela de presets](docs/img/tela-5a-presets.png)

### Editor manual

13 controles com botões ◀▶ e tooltip [?] explicativo. Botão "Testar" aplica os valores apenas no GM (preview), sem persistir.

![Editor manual](docs/img/tela-5b-editor.png)

**Variáveis editáveis:**

| Variável | Range visual | Step | Tooltip |
|---|---|---|---|
| `density` | 0.001 a 0.05 | 0.001 | Quanta névoa existe na cena |
| `radius` | 50 a 2000 | 10 | Bolha de ar limpo ao redor do player |
| `falloff` | 10 a 500 | 5 | Suavidade da borda do clearing |
| `vmin` | 0.0 a 2.5 | 0.05 | Intensidade no chão |
| `vmax` | 0.0 a 2.5 | 0.05 | Intensidade na altitude |
| `noise` | 0.0 a 1.0 | 0.05 | Força da variação volumétrica |
| `scale` | 0.3 a 5.0 | 0.1 | Tamanho das ondulações |
| `speed` | 0.0 a 1.0 | 0.01 | Velocidade do drift |
| `dark` | 0.0 a 1.0 | 0.05 | Escurecimento de silhuetas |
| `desat` | 0.0 a 1.0 | 0.05 | Dessaturação por distância |
| `color R/G/B` | 0.0 a 1.0 | 0.05 | Cor da névoa (preview ████ inline) |

### Tooltips contextuais

Cada variável tem botão [?] que abre gump pequeno com explicação, valores exemplo, analogia e aviso (quando aplicável).

![Tooltip de variável](docs/img/tela-6-tooltip.png)

### Duração

- **Permanente** — região ativa até GM parar manualmente
- **Por tempo** — 1 a 240 minutos, expira automaticamente

### Lista de regiões

Mostra todas as regiões cadastradas (3 por página). Status visual:
- 🟢 **Verde** — Ativa permanente
- 🟠 **Âmbar** — Ativa com timer (mostra MM:SS restantes)
- ⚫ **Cinza** — Parada

![Lista de regiões](docs/img/tela-7-lista.png)

Ações por região:
- **Editar** — abre editor manual pré-populado com valores atuais (modo edit)
- **Ir** — teleporta GM para o centro do retângulo
- **Parar / Ativar** — toggle de Status
- **Del** — deleta (com confirmação)

### Onde estou agora

Debug rápido. Mostra X/Y/Z/Realm, região POL nativa (se houver), e fog region que cobre o ponto.

![Onde estou](docs/img/tela-8-onde-estou.png)

Botão "Criar região aqui" pré-popula o canto NW com a posição atual e abre o wizard direto no Passo 1.

## Arquitetura

### Detecção de presença — watcher per-player

`fogRegionWatcher.src` roda como script per-player, iniciado automaticamente pelo logon do POL. Loop `sleep(2)` verifica posição do player vs regiões ativas; quando muda de região, dispara packet apropriado.

```
Player loga
  → POL chama pkg/systems/fog_regions/logon.src
  → se #fogWatcherPid resolve p/ processo vivo (GetProcess) → não duplica
  → senão start_script :fog_regions:fogRegionWatcher e guarda scr.pid em #fogWatcherPid
  → Loop while(who.connected):
      se GetFogRegionsVersion() mudou → recarrega cache (LoadRegions); senão reusa
      current_region = MatchRegionAtPosition(regions_cache, who.x, who.y, who.realm)
      if (current_region != last_region):
          SendFogOff(who)             (se estava em outra)
          SendFogPacketForRegion(...)  (se entrou em nova)
      sleep(2)
```

Encerra automaticamente no logoff (loop sai quando `!who.connected`). Dedup por liveness de processo: `#fogWatcherPid` + `GetProcess` no logon (sobrevive a restart/OOM, ao contrário de uma flag persistida). Linha defensiva em `scripts/misc/logoff.src` apaga `#fogWatcherPid`.

**Por que watcher em vez de EnterScript/LeaveScript nativos do POL:** EnterScript precisa de restart do server pra ativar nova região. Watcher per-player funciona imediatamente — GM cria região e testa AGORA. Trade-off escolhido: UX > elegância arquitetural.

### Timer — effect script per-region

`fogRegionEffect.src` roda como script per-region quando GM cria/ativa região modo `timed`. Dorme até `TimerEnd` e atualiza `Status=STOPPED` no arquivo.

3 sanity checks pós-sleep evitam race conditions:
- Região foi deletada? → aborta
- TimerEnd mudou (GM editou)? → aborta (novo effect script vai cuidar)
- Status mudou para STOPPED (GM parou manualmente)? → aborta

### Boot hook

`start.src` re-dispara effect scripts no boot para regiões timed ainda ativas. Trata edge case "server offline durante timer": se TimerEnd já passou enquanto offline, expira direto sem disparar script.

### Persistência

Regiões persistem em `pkg/systems/fog_regions/regions.cfg` (formato POL ConfigFile). Todos os parâmetros de fog armazenados como inteiros escalados para evitar imprecisão de ponto flutuante:

| Parâmetro | Multiplicador | Exemplo |
|---|---|---|
| density | × 100000 | 0.005 → 500 |
| vmin/vmax/noise/scale/dark/desat | × 1000 | 0.95 → 950 |
| speed | × 10000 | 0.05 → 500 |
| color R/G/B | × 255 (Int8) | 0.20 → 51 |
| radius/falloff | direto | — |

## Estrutura de arquivos

```
pkg/systems/fog_regions/
├── pkg.cfg                         manifesto do pacote
├── regions.cfg                     data store de regiões
├── README.md                       este arquivo
├── TESTS.md                        plano de testes manuais
├── start.src                       boot hook (re-dispara timers)
├── logon.src                       hook de logon (inicia watcher)
├── fogRegionWatcher.src            watcher per-player
├── fogRegionEffect.src             timer per-region
├── include/
│   └── fogRegionCore.inc           API do sistema (~280 linhas)
└── gumps/
    └── fogRegionGump.inc           todas as 9 telas GM (~1700 linhas)

pkg/commands/gm/
└── fogregion.src                   entrypoint do comando .fogregion

pkg/commands/config/icp.cfg         registro do comando (1 linha)
scripts/misc/logoff.src             limpeza de #fogWatcherPid (1 linha)
```

## API pública — fogRegionCore.inc

```pol
LoadRegions()                  → array de structs (vazio se cfg vazio)
GetRegionByName(name)          → struct ou error
GetRegionAtPosition(x,y,realm) → struct (primeira ativa que contém) ou error
IsRegionLogicallyActive(r)     → 1 se Status=ACTIVE e (Permanent ou TimerEnd futuro)
SaveRegion(struct)             → upsert no cfg
DeleteRegion(name)             → remove do cfg
SendFogPacketForRegion(who, r) → packet 0x0070 com parâmetros da região
SendFogOff(who)                → packet 0x0070 com enabled=0
SendFogTestPacket(who, params) → packet de teste (editor GM)
SendFogOffToGm(who)            → desliga fog de teste do GM
```

## Protocolo packet 0x0070 v4

29 bytes total. Versionado com backwards compat (v1 = 5 bytes, v2/v3 com campos progressivos, v4 = completo).

| Offset | Tamanho | Campo |
|---|---|---|
| 0 | Int8 | Packet header (0xD7) |
| 1-2 | Int16 BE | Size (29) |
| 3-4 | Int16 BE | Subcommand (0x0070) |
| 5 | Int8 | Enabled (0 ou 1) |
| 6-7 | Int16 BE | DensityInt |
| 8-9 | Int16 BE | Radius |
| 10-11 | Int16 BE | Falloff |
| 12-13 | Int16 BE | VMinInt |
| 14-15 | Int16 BE | VMaxInt |
| 16-17 | Int16 BE | NoiseInt |
| 18-19 | Int16 BE | ScaleInt |
| 20-21 | Int16 BE | SpeedInt |
| 22-23 | Int16 BE | DarkInt |
| 24 | Int8 | ColorR |
| 25 | Int8 | ColorG |
| 26 | Int8 | ColorB |
| 27-28 | Int16 BE | DesatInt |

Camada B (cliente EpicUO) implementa decode + shader. Clientes oficiais ignoram subcommands desconhecidos sem crash.

## Convenção de cores e botões

| ID | Uso |
|---|---|
| 2103 / 2104 | Botões de navegação largos (menus, listas) |
| 2117 / 2118 | Botões compactos ◀▶ (incremento/decremento) |
| 22153 | Gráfico de botão desabilitado (`GF_DUMMY_BTN`) |

| Cor | ID | Uso |
|---|---|---|
| Âmbar | 53 | Títulos, status timed |
| Cyan | 1153 | Descrições secundárias |
| Body | 90 | Texto comum |
| Vermelho | 33 | Erros e avisos |
| Verde | 68 | Status ativa permanente |
| Cinza muted | 230 | Status parada |

## Descobertas de design

### A descoberta da harmonia cor-terreno

Inicialmente o sistema fog era global (comando `.fog`). 22 testes visuais revelaram que o problema das "bolhas escuras feias" não era do shader — era contraste cor-fog vs cor-terreno. Terreno escuro pede fog escuro. Verde pede verde-musgo. Areia pede sépia.

Conclusão: **fog tem que ser regional**, cada zona com paleta harmoniosa com seu terreno. Foi a justificativa arquitetural pra esta Camada E inteira.

### Inteiros escalados

Floats em cfg files dão imprecisão. Internamente trabalha-se com ints escalados, conversão para float apenas no display. O preço (multiplicadores espalhados) compensa pela confiabilidade.

### Watcher vs EnterScript nativo

EnterScript do POL é o caminho "elegante" mas exige restart pra ativar nova região. Pra ferramenta de world-building em desenvolvimento ativo, restart vira atrito de UX. Watcher per-player com sleep(2) custou ~30 linhas extras de código e ganhou criação instantânea.

## Limitações conhecidas

**1. Edição não atualiza watchers ativos.** GM edita parâmetros via "Editar" na lista; players já dentro da região continuam vendo valores antigos até saírem e voltarem. Sem botão "Aplicar agora" — refinement futuro.

**2. Sem cache de regiões.** Cada watcher lê `regions.cfg` a cada 2 segundos. Em shards com 50+ players online simultâneos, considerar cache invalidado por `SaveRegion`/`DeleteRegion`.

**3. Reativar região timed expirada não reseta timer.** Botão "Ativar" na lista mantém `TimerEnd` original. Effect script detecta `sleep_seconds <= 0` e expira imediatamente. Pra reativar com tempo, GM precisa editar duração via editor manual.

**4. Override do `.fog` GM em regiões.** Se GM está dentro de uma região e dispara comando `.fog` manual, o fog manual sobrepõe temporariamente. Próxima entrada na região (sair + voltar) reaplica o preset da região. Comportamento intencional.

## Desenvolvimento

Branch: `feat/nevoa`

Comentários `// FOG-FASE5:` marcam todo o código novo desta camada, facilitando code review e rastreamento de origem.

### Para adicionar uma nova variável de fog no futuro

Pontos de mudança em ordem:
1. `fogRegionCore.inc` — adiciona campo em `_ElemToStruct`, na constante de campos do `SaveRegion`, e no packet de `SendFogPacketForRegion`/`SendFogTestPacket`
2. `fogRegionGump.inc` — adiciona constantes `FR_BTN_ED_NOVA_DEC/INC`, `FR_STEP_NOVA`, `FR_MIN/MAX_NOVA`, default em `GetEditorState`, linha de UI no editor manual, tooltip em `_GetTooltip*`
3. `fogRegionGump.inc` — atualiza handler do botão "Criar" para incluir o campo no struct region
4. Bump da versão do packet 0x0070 (v5) se mudou tamanho
5. Cliente EpicUO — `FogState`, `PacketHandlers`, shader, batcher

### Para reverter o hook de auto-compile (já foi feito)

Hook em `~/.claude/hooks/compile_on_escript_write.py` foi modificado durante desenvolvimento para excluir `/fog_regions/` (compilação manual). Já foi revertido — sem ação pendente.

## Histórico

- **Camada A** (cliente EpicUO) — protocolo inicial v1, fog uniforme
- **Camada B** (servidor + cliente) — protocolo v4, 11 parâmetros, ruído Perlin, darkening, dessaturação, cor fria. Commit `5ea7a4bfb`
- **Camada B.2** — refinos visuais. 22 testes calibrados de presets
- **Camada E** — sistema regional via gump GM, watcher per-player, timer, persistência. Esta camada.
