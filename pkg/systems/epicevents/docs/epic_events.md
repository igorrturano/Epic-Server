# Epic! Events System - Technical Documentation

## Overview

**Epic! Events** is a unified event management framework for creating immersive, interactive dungeon experiences. It consolidates multiple legacy systems (lever, epicboss, talker, rumores) into a single trigger → action architecture with conditional branching and timing controls.

## Core Architecture (MVC Pattern)

### Models (`/include/Model/`)
- **EventConfig.inc** - Root configuration (trigger type, params, actions, delays)
- **TriggerConfig.inc** - Trigger-specific parameters (keyword, radius, required items)
- **ActionConfig.inc** - Action definitions (type, params, conditions, delays)
- **ConditionConfig.inc** - Branching logic (player has item, action succeeded, etc.)
- **CreatureSpawnInfo.inc** - Legacy spawn data (unchanged)

### Controllers (`/include/Controller/`)
- **TriggerController.inc** - Processes events, validates triggers, manages cooldowns
- **ActionController.inc** - Executes actions sequentially, evaluates conditions
- **ConditionController.inc** - Evaluates branching conditions
- **SpawnController.inc** - Creature spawning logic (refactored for action system)

### Views (`/include/View/`)
- **EventGumpBuilder.inc** - Multi-page wizard UI with visual hierarchy

### Entry Points
- **event.src** - GM command handler (`.event`)
- **eventcontroller.src** - Main event loop (attached to items via ControlScript)

---

## System Flow

```
1. GM runs .event command
   ↓
2. Select TRIGGER TYPE (5 options)
   ↓
3. Configure TRIGGER PARAMETERS + EVENT DELAYS
   ↓
4. Configure ACTIONS (sequential execution)
   - Each action has: Type, Params, Delay, Condition
   ↓
5. Place event item (invisible, immovable)
   ↓
6. eventcontroller.src starts listening for trigger
   ↓
7. Trigger fires → Validate → Execute actions → Apply cooldown
```

---

## Trigger Types (5 Total)

### 1. SPEECH
- **Event**: `SYSEVENT_SPEECH`
- **Params**: `keyword` (string), `radius` (tiles for hearing range)
- **Use Case**: "Gods listen to me" summons storm
- **Special**: Items must call `RegisterForSpeechEvents()` before `EnableEvents()`

### 2. ENTEREDAREA
- **Event**: `SYSEVENT_ENTEREDAREA`
- **Params**: `radius` (tiles for detection)
- **Use Case**: Player enters area → spawn enemies
- **Note**: Only triggers for players (not NPCs)

### 3. LEFTAREA
- **Event**: `SYSEVENT_LEFTAREA`
- **Params**: `radius` (tiles for detection)
- **Use Case**: "Coward punishment" - freeze player who runs away
- **Note**: Only triggers for players

### 4. ITEM_GIVEN
- **Event**: `SYSEVENT_ITEM_GIVEN`
- **Params**: `required_items` (array of {objtype, amount})
- **Use Case**: Offering system - give gold for blessing
- **Note**: Currently item-based (NPC support in Phase 2)

### 5. DOUBLECLICKED
- **Event**: `SYSEVENT_DOUBLECLICKED`
- **Params**: None
- **Use Case**: Interact with shrine, altar, portal
- **Note**: Currently item-based (NPC support in Phase 2)

---

## Action Types (3 Current, Expandable)

### 1. EMOTE
**Purpose**: Display atmospheric messages to create storytelling moments

**Parameters**:
- `text` (string) - The emote/message text
- `range` (tiles) - Max distance for players to see (0 = unlimited)
- `show_on_source` (bool) - Display above player (1) or item (0)

**Example**:
```javascript
Action 1: Emote "*The sky darkens...*" (above item, 15 tiles)
Action 2: Emote "*Thunder roars!*" (delay: 5s)
```

### 2. SPAWN
**Purpose**: Spawn creatures using legacy system configuration

**Parameters**: Uses entire `EventConfig.ToStruct()` (template1-3, amount, tier, etc.)

**Example**:
```javascript
Action 1: Spawn 3 orcs (tier 2)
Condition: Player has "Ancient Key"
```

### 3. TELEPORT
**Purpose**: Move player to specific coordinates

**Parameters**:
- `x`, `y`, `z` (integers) - Destination coordinates
- `realm` (string, optional) - Defaults to player's current realm

**Example**:
```javascript
Action: Teleport to (1000, 2000, 0)
Condition: None (always execute)
```

---

## Condition Types (3 Current, Expandable)

### 1. ALWAYS (Default)
No condition - action always executes

### 2. PLAYER_HAS_ITEM
**Checks**: Player's backpack for specific item

**Parameters**:
- `objtype` (hex) - Item objtype
- `amount` (int) - Required quantity (default: 1)

**Example**:
```javascript
Condition: Player has 0xEED (gold) x100
```

### 3. ACTION_SUCCEEDED
**Checks**: If a previous action in the sequence succeeded

**Parameters**:
- `action_id` (int) - Which action number (1-10)

**Example**:
```javascript
Action 2: Emote "*Enemies appear!*"
Condition: Action 1 succeeded (spawn)
```

---

## Timing System

### Action-Level Delays
- **Configured per action** (in seconds)
- **Use case**: Sequential storytelling (emote 1 → wait 5s → emote 2)
- **Default**: 0 (immediate execution)

### Event-Level Cooldowns
- **Configured per event** (in minutes)
- **Min/Max range**: Randomized between values (default: 60-120 min)
- **Use case**: Prevent spam, control encounter frequency
- **Storage**: `next_trigger_time` ObjProperty (systime + seconds)

---

## Data Storage & Serialization

### Storage Method
All configuration stored as ObjProperty on event item:
```javascript
SetObjProperty(item, "config", config.ToStruct());
```

### Serialization Flow
```
SAVE:
EventConfig → ToStruct() → actions array → ActionConfig.ToStruct() → ObjProperty

LOAD:
ObjProperty → FromStruct() → actions array → ActionConfig.FromStruct() → EventConfig
```

**Critical**: Actions are **objects** in memory but **structs** in storage. Must convert both directions.

---

## UI Design Patterns

### Visual Hierarchy (Color Codes)
- **1149** (Bright Cyan) - Section headers
- **53** (Gold) - Important text, selected items
- **2100** (Light Gray) - Normal labels
- **1153** (Dark Gray) - Help text, descriptions

### Section Separators
```javascript
GFTextLine(gump, x, y, 1149, "Section Name");
y := y + 3;
GFGumpPicTiled(gump, x, y, width, 1, 0x0A52);  // Horizontal line
```

### Radio Buttons (Conditions)
```javascript
GFRadioButton(gump, x, y, 0x25F8, 0x25FB, selected, group_id, button_id);
// 0x25F8 = hollow circle, 0x25FB = filled circle
```

### Background Boxes
```javascript
GFResizePic(gump, x, y, 0x2486, width, height);  // Subtle background
```

---

## Example Use Cases

### Example 1: Speech-Triggered Storm
```
Trigger: SPEECH
  Keyword: "Gods listen to me"
  Radius: 8 tiles
  Cooldown: 5-10 minutes

Actions:
  1. Emote "*The sky darkens...*" (above item, 15 tiles)
     Delay: 0s, Condition: Always

  2. Weather: Storm (60s duration, 20 tile range)
     Delay: 3s, Condition: Always

  3. Emote "*Thunder roars!*"
     Delay: 5s (from action 2), Condition: Always
```

### Example 2: Conditional Enemy Spawn
```
Trigger: ENTEREDAREA
  Radius: 8 tiles
  Cooldown: 60-120 minutes

Actions:
  1. Emote "*You feel a chill...*"
     Delay: 0s, Condition: Always

  2. Spawn 3 orcs
     Delay: 2s, Condition: Player has "Ancient Key" (0x1F4)

  3. Emote "*Orcs emerge from shadows!*"
     Delay: 0s, Condition: Action #2 succeeded
```

### Example 3: Coward Punishment
```
Trigger: LEFTAREA
  Radius: 8 tiles
  Cooldown: 1-2 minutes

Actions:
  1. Freeze player (5s paralyze)
     Delay: 0s, Condition: Always

  2. Emote "*COWARD! YOU THINK YOU COULD RUN?!*" (above player)
     Delay: 0s, Condition: Always

  3. Spawn boss
     Delay: 2s, Condition: Boss not already alive
```

---

## Phase 2 Roadmap (Future)

### NPC Event Sources
- Convert talker/rumores NPCs to use Epic! Events
- NPC-based triggers (speech, proximity, gift)
- Shared event system for items AND NPCs

### Advanced Actions
- **Weather Effects**: Storm, earthquake, darkness (from lever system)
- **Boss Spawning**: Integration with epicboss system
- **Timed Scripts**: Apply buffs/debuffs (from timedscripts system)
- **Door/Object Manipulation**: Toggle doors, activate levers
- **Sound Effects**: Play audio for atmosphere

### AI Integration
- Speech events trigger AI responses (from banker system)
- Context-aware storytelling with AI
- Dynamic emote generation based on player actions

### Event Chaining
- Multi-stage events (Phase 1 → Phase 2 → Phase 3)
- Event dependencies (Event A must complete before Event B)
- Global event states (server-wide events)

---

## Critical Implementation Notes

### SPEECH Trigger Registration
```javascript
// CRITICAL: Items must register BEFORE enabling events!
RegisterForSpeechEvents(item, radius);
EnableEvents(SYSEVENT_SPEECH, radius);
```

### ActionConfig Serialization
```javascript
// SAVE: Convert objects to structs
var actions_data := array{};
foreach action in (this.actions)
    actions_data.append(action.ToStruct());  // ← CRITICAL
endforeach

// LOAD: Reconstruct objects from structs
foreach action_struct in (data.actions)
    var action := ActionConfig("");
    action.FromStruct(action_struct);  // ← CRITICAL
    this.actions.append(action);
endforeach
```

### Cooldown Calculation
```javascript
// Randomize between min and max (in minutes)
var delay_minutes := RandomInt(max_minutes - min_minutes + 1) + min_minutes;
var cooldown_seconds := delay_minutes * 60;
UpdateTriggerCooldown(item, cooldown_seconds);
```

---

## File Reference Quick Map

| File | Purpose | Lines |
|------|---------|-------|
| `event.src` | GM wizard flow | 240 |
| `eventcontroller.src` | Main event loop | 110 |
| `EventConfig.inc` | Root data model | 170 |
| `ActionConfig.inc` | Action data model | 56 |
| `TriggerConfig.inc` | Trigger params | 52 |
| `ConditionConfig.inc` | Condition logic | ~80 (Phase 2) |
| `TriggerController.inc` | Event validation | 113 |
| `ActionController.inc` | Action execution | 135 |
| `ConditionController.inc` | Condition eval | ~100 (Phase 2) |
| `SpawnController.inc` | Legacy spawning | ~200 |
| `EventGumpBuilder.inc` | UI components | 847 |

---

## Testing Checklist

### Trigger Validation
- [ ] SPEECH: Say keyword near item → fires
- [ ] ENTEREDAREA: Walk into radius → fires
- [ ] LEFTAREA: Walk out of radius → fires
- [ ] ITEM_GIVEN: Give required item → fires
- [ ] DOUBLECLICKED: Double-click item → fires

### Action Execution
- [ ] EMOTE: Text appears above item/player
- [ ] SPAWN: Creatures appear at item location
- [ ] TELEPORT: Player moves to coordinates

### Conditions
- [ ] ALWAYS: Action always executes
- [ ] PLAYER_HAS_ITEM: Only executes if item in backpack
- [ ] ACTION_SUCCEEDED: Only executes if previous action worked

### Timing
- [ ] Action delays: Sequential actions respect delay field
- [ ] Event cooldowns: Trigger blocked until cooldown expires
- [ ] Randomization: Cooldown varies between min/max

### UI
- [ ] Radio buttons: Select condition without closing gump
- [ ] Section headers: Clear visual hierarchy
- [ ] Background boxes: Actions visually separated
- [ ] Delay fields: Min/max validation

---

## Common Patterns

### Creating a Simple Emote Event
```
1. .event
2. Select: "Speech (Palavra-chave)"
3. Keyword: "hello"
4. Radius: 8, Delays: 1-1 (1 min cooldown)
5. Add Action: "Emote/Mensagem"
6. Text: "*The statue glows softly*"
7. Range: 10, Show on: Item
8. Condition: "Sempre"
9. Save → Place item
```

### Creating a Quest Reward Spawn
```
1. .event
2. Select: "Entrada em Area (Proximidade)"
3. Radius: 8, Delays: 60-120
4. Add Action: "Spawn de Criaturas"
5. Configure: 3 orcs, tier 2
6. Condition: "Jogador possui item"
7. ObjType: 0x1F4 (quest item), Qtd: 1
8. Save → Place item
```

---

## Debugging

### Enable Debug Logging
Event controller prints debug info automatically:
```
[EpicEvents] DEBUG - Processing trigger type: 'SPEECH' (event type: 4)
[EpicEvents] Speech trigger matched: 'hello'
[EpicEvents] Trigger validated - executing actions
[EpicEvents] Action 1: EMOTE
[EpicEvents] Emote displayed above item: "*The statue glows*"
[EpicEvents] Cooldown set - next trigger in 1.0 minutes
```

### Common Issues
1. **SPEECH not firing**: Check `RegisterForSpeechEvents()` called before `EnableEvents()`
2. **Actions not working**: Verify `ActionConfig.ToStruct()` during save
3. **"Acao invalida!"**: Use radio buttons instead of regular buttons for conditions
4. **Cooldown too long**: Check min/max delay fields (in minutes, not seconds)

---

## Next Steps for Continuation

When resuming work on this system:

1. **Review this document** to understand current architecture
2. **Check `/include/Model/` and `/include/Controller/`** for existing classes
3. **Reference `eventcontroller.src`** for main event loop
4. **Use `EventGumpBuilder.inc`** patterns for new UI
5. **Follow MVC separation** - Models for data, Controllers for logic, Views for UI
6. **Test with `.event` command** - wizard is fully functional

For Phase 2 work, focus on:
- NPC support (convert talker/rumores systems)
- Advanced actions (weather, boss spawning, timed scripts)
- Event chaining (multi-stage events)
- AI integration (context-aware responses)
