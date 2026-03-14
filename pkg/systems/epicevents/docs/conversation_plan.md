# Epic Events: NPC Conversation System (Phase 1)

## Context

The old conversation/quest system (`pkg/mobiles/merchants/pcconversation/` + `pkg/tn/`) is buggy and tightly coupled. We're building a new NPC Conversation system inside Epic Events (`pkg/systems/epicevents/`), leveraging the existing trigger → action architecture.

**Goal**: GMs can create conversational NPCs via `.event` — with dialogue trees, conditional answers (religion, race, etc.), and actions per answer (emote, teleport, give items). Phase 2 will add quest tracking and item turn-in.

---

## Key Decisions

- **CONVERSATION** = new Action type (dialogue tree opens when DOUBLECLICKED fires)
- **NPC creation integrated** in `.event` wizard (Step 0: create new or target existing)
- **Alignment is read-only in POL** → need a `good`-aligned NPC template in cfg
- **New DataFile** `:epicevents:conversations` for conversation data (avoids prop overflow)
- **Each answer** can have conditions + actions + branch to next node
- **Reuse** existing ConditionController, ActionController, EventGumpBuilder patterns

---

## Data Model

### DataFile: `:epicevents:conversations`

Keyed by conversation_id (e.g., `"conv_1741456800_4523"`):

```
Element: <conversation_id>
  Prop "meta"      → struct{ npc_serial, npc_name, created_by, created_at }
  Prop "root_node" → "node_1"
  Prop "node_list" → array{ "node_1", "node_2", "node_3" }
  Prop "node_<id>" → struct{
      id   := "node_1",
      text := "Greetings traveler!",
      answers := array{
          struct{
              id         := "ans_1",
              text       := "I seek the temple.",
              next_node  := "node_2",       // "" = end conversation
              conditions := struct{...},     // same format as ConditionController
              actions    := array{...}       // same format as ActionConfig
          }
      }
  }
```

Each node is a separate DataFile property → individual nodes load/save independently.

### CONVERSATION ActionConfig

```
ActionConfig("CONVERSATION")
  params := struct{ conversation_id := "conv_12345" }
```

---

## New Files

| File | Type | Purpose |
|------|------|---------|
| `include/Model/ConversationNode.inc` | Model | ConversationNode class (id, text, answers) + ToStruct/FromStruct |
| `include/Controller/ConversationController.inc` | Controller | DataFile CRUD: LoadConversation, SaveNode, CreateConversation, DeleteNode, GenerateNodeId |
| `include/View/ConversationGumpBuilder.inc` | View | Player dialogue gump + GM node editor gump + tree overview gump |
| `actions/CONVERSATION.src` | Action | Player dialogue loop: load tree → show node → filter answers by conditions → execute actions → navigate |
| `npcdesc.cfg` (new) | Config | `epic_event_npc` template with `Alignment good`, invul, minimal stats |

## Modified Files

| File | Changes |
|------|---------|
| `include/View/EventGumpBuilder.inc` | Add CONVERSATION to action type selection gump + description + parsing |
| `Event.src` | Add Step 0 (NPC creation/selection) + CONVERSATION action opens conversation editor |

---

## NPC Template

Add to `pkg/systems/epicevents/npcdesc.cfg`:

```
NpcTemplate epic_event_npc
{
    Name            NPC
    Script          :epicevents:NpcEventController
    ObjType         0x190
    Alignment       good
    Privs           invul
    Settings        invul

    Strength        2000
    Intelligence    2000
    Dexterity       2000
    HITS            2000
    MANA            2000
    STAM            2000

    AttackSpeed     80
    AttackDamage    1d1
    AttackAttribute Wrestling
    AttackHitScript :combat:mainHitScript
}
```

Script is already `NpcEventController` so no override needed. Graphic and name are set by the wizard after creation.

---

## GM Wizard Flow

### Step 0: NPC Setup (new, before trigger selection)

Only shows when trigger = DOUBLECLICKED or ITEM_GIVEN:

```
┌─────────────────────────────────────┐
│  CONFIGURAR NPC DO EVENTO           │
│─────────────────────────────────────│
│  (o) Usar NPC existente             │
│      Clique em um NPC do mundo      │
│                                     │
│  (o) Criar novo NPC                 │
│      Graphic: [0x0190] [Copiar ▸]   │
│      Nome:    [________________]    │
│                                     │
│  [Proximo]                          │
└─────────────────────────────────────┘
```

- **"Copiar"** button → `Target(who)` → copies `.graphic` from targeted creature
- **"Criar novo NPC"** → `CreateNpcFromTemplate(":epicevents:epic_event_npc", x, y, z, 0, realm)` then sets `npc.graphic` and `npc.name`
- **"Usar NPC existente"** → `Target(who)` → validate `.npctemplate`

### Step 3: CONVERSATION Action Type

Added to `BuildActionTypeSelectionGump()`:
```
[>] Conversa (Dialogo)
    Abrir arvore de dialogo interativa com o NPC
```

When selected → opens Conversation Editor (not the normal action edit gump).

### Conversation Node Editor (recursive loop)

```
┌─────────────────────────────────────────┐
│  EDITOR DE CONVERSA - No [node_1]       │
│─────────────────────────────────────────│
│  TEXTO DO DIALOGO                       │
│  ┌─────────────────────────────────────┐│
│  │ Greetings traveler! What brings     ││
│  │ you to these lands?     (scrollable)││
│  └─────────────────────────────────────┘│
│  [Editar Texto]                         │
│                                         │
│  RESPOSTAS                              │
│  ────────────────────────────────────── │
│  [✎][✖] 1. "Busco o templo" → [node_2] │
│          Cond: Religiao=Ush'krar        │
│          Acoes: 1 configurada(s)        │
│                                         │
│  [✎][✖] 2. "Adeus" → [FIM]             │
│                                         │
│  [+ Adicionar Resposta]                 │
│  ────────────────────────────────────── │
│  [Ir para No...]  [Visao Arvore]  [OK]  │
└─────────────────────────────────────────┘
```

- **Edit answer** → opens Answer Editor (text, next node, conditions, actions)
- **Click node link** → navigates to that node (recursive)
- **"Editar Texto"** → `BigRequestGump` for multi-line dialogue input
- **"Ir para No..."** → radio selection of all nodes for quick navigation
- **"Visao Arvore"** → flat overview of all nodes + connections

### Answer Editor

```
┌─────────────────────────────────────────┐
│  EDITAR RESPOSTA                        │
│─────────────────────────────────────────│
│  Texto: [_________________________]     │
│                                         │
│  PROXIMO NO                             │
│  (o) No existente: [radio list]         │
│  (o) Criar novo no                      │
│  (o) Encerrar conversa                  │
│                                         │
│  CONDICAO                               │
│  [Configurar] (Nenhuma)                 │
│  → reuses HandleConditionEdit()         │
│                                         │
│  ACOES                                  │
│  [Adicionar Acao] (0 acoes)             │
│  → reuses action type selection + edit  │
│                                         │
│  [Salvar]  [Cancelar]                   │
└─────────────────────────────────────────┘
```

---

## Player Flow

1. Player double-clicks NPC
2. `NpcEventController` → DOUBLECLICKED event → finds matching config
3. `ActionController.ExecuteActions()` → starts `actions/CONVERSATION.src`
4. `CONVERSATION.src` loads conversation from DataFile, starts at root node
5. **Loop**:
   - Load current node
   - Filter answers via `EvaluateCondition()` (from existing ConditionController)
   - Build player dialogue gump (NPC name, dialogue text in HTMLArea, visible answer buttons)
   - Wait for input
   - Execute selected answer's actions (EMOTE, TELEPORT, SPAWN, give items via existing action scripts)
   - Navigate to `next_node` (or break if empty/"end")
6. Conversation ends, control returns to NpcEventController

### Player Dialogue Gump

```
┌─────────────────────────────────────────┐
│  ┌─────┐  NPC_NAME                     │
│  │ pic │  ─────────────────────────     │
│  └─────┘                                │
│  ┌─────────────────────────────────────┐│
│  │ "The ancient temple lies north,     ││
│  │  but beware of the guardians..."    ││
│  │                        (scrollable) ││
│  └─────────────────────────────────────┘│
│                                         │
│  [>] I seek the ancient temple.         │
│  [>] Tell me about the dangers.         │
│  [>] Farewell.                          │
│                                         │
│           [Encerrar Conversa]           │
└─────────────────────────────────────────┘
```

---

## Implementation Order

### Phase 1A: Data Layer
1. Create `ConversationNode.inc` — model with ToStruct/FromStruct
2. Create `ConversationController.inc` — DataFile CRUD operations
3. Create `npcdesc.cfg` — `epic_event_npc` template

### Phase 1B: Player-Facing
4. Create `ConversationGumpBuilder.inc` — player dialogue gump builder
5. Create `CONVERSATION.src` — action script with dialogue loop

### Phase 1C: GM Editor
6. Add GM editor gumps to `ConversationGumpBuilder.inc` — node editor, answer editor, tree overview
7. Modify `EventGumpBuilder.inc` — add CONVERSATION action type
8. Modify `Event.src` — add Step 0 NPC wizard + CONVERSATION action handling (opens editor instead of normal action edit)

---

## Reused Existing Code

| What | From | How |
|------|------|-----|
| Condition evaluation | `ConditionController.inc` | `EvaluateCondition()` filters answer visibility |
| Action execution | `ActionController.inc` | `ExecuteActions()` runs answer actions |
| Condition editor UI | `Event.src` `HandleConditionEdit()` | Reused for per-answer conditions |
| Action editor UI | `Event.src` `HandleActionEdit()` | Reused for per-answer actions |
| Action type selection | `EventGumpBuilder.inc` | Reused in answer action config |
| Text input | `requestGump.inc` | `BigRequestGump()` for dialogue text |
| DataFile pattern | `datafile.inc` | `DFOpenDataFile` / `DFFindElement` |
| Gump framework | `gumps.inc` + `epicGumps.inc` | All gump building functions |

---

## Button/Field ID Ranges

To avoid collisions with existing EventGumpBuilder IDs (1-463):

- Conversation constants: **500-699**
  - 500-519: Node editor buttons
  - 520-539: Answer editor buttons
  - 540-559: Tree overview buttons
  - 560-599: Text entry field IDs
  - 600+: Dynamic answer buttons (600 + answer_index)

---

## Verification

1. **Create NPC**: Run `.event` → select DOUBLECLICKED → "Criar novo NPC" → set graphic/name → verify NPC appears with correct graphic, name, blue alignment
2. **Add conversation**: Add CONVERSATION action → create root node with dialogue → add 2 answers → save
3. **Player test**: Double-click NPC → dialogue appears → answers visible → click answer → correct next node loads
4. **Conditions**: Add religion condition to an answer → test with matching/non-matching player → answer shows/hides correctly
5. **Actions on answer**: Add EMOTE action to an answer → click it → emote displays → conversation continues to next node
6. **Tree navigation**: Create 3+ nodes → GM can navigate between them → tree overview shows all nodes
7. **Persistence**: Restart server → NPC still works → conversation data intact in DataFile

---

## Phase 2 (Future)

- Quest tracking: accept quest → state on player prop → timer → ITEM_GIVEN turn-in
- New condition: `QUEST_ACTIVE` / `QUEST_COMPLETE`
- New action: `START_QUEST` / `COMPLETE_QUEST`
- Item request/validation in conversation flow