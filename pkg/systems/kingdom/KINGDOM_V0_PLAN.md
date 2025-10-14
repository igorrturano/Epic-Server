# Kingdom System v0 - Beta Implementation Plan

**Project**: Epic! Shard Kingdom Management System
**Version**: 0.1.0 (Beta)
**Started**: 2025-10-12
**Status**: Planning → Implementation

---

## Table of Contents

1. [Overview](#overview)
2. [Design Principles](#design-principles)
3. [Architecture](#architecture)
4. [Data Structures](#data-structures)
5. [Phase 1: Foundation](#phase-1-foundation)
6. [Phase 2: Conquest System](#phase-2-conquest-system)
7. [Phase 3: Resources](#phase-3-resources)
8. [Phase 4: Buildings](#phase-4-buildings)
9. [Phase 5: Upkeep & Progression](#phase-5-upkeep--progression)
10. [Phase 6: Polish & Testing](#phase-6-polish--testing)
11. [Function Reference](#function-reference)
12. [Testing Checklist](#testing-checklist)

---

## Overview

### Goal
Create a clean, maintainable kingdom management system for beta testing that allows factions to:
- Conquer territories via champion-style combat
- Build structures and generate resources
- Maintain their kingdom through upkeep
- Progress through 5 levels across 8 cultures

### Scope for v0 (Beta)
**INCLUDED:**
- ✅ Territory conquest via simplified champion waves
- ✅ Kingdom creation and management
- ✅ 8 cultures with 5 levels each
- ✅ Resource generation (8 types: Comida, Moedas, Couro, Metal, Madeira, Pano, Pedra, Joias)
- ✅ Building construction (foundation → scaffolding → timer → complete)
- ✅ Upkeep system (coin-based with Discord notifications)
- ✅ Guild XP and leveling
- ✅ Territory expansion

**DEFERRED TO v1:**
- ❌ War between kingdoms
- ❌ Senate/laws system
- ❌ Building visual progression stages (8-stage system)
- ❌ Advanced building bonuses (auras, buffs)
- ❌ Dynamic events (catastrophes, prosperity)
- ❌ Caravan system

### Key Constraints
1. **Isolation**: All code in `pkg/systems/kingdom/` - no modifications to other packages
2. **Simplicity**: KISS principle - simple over clever
3. **Clean Code**: DRY, SOLID principles
4. **Testability**: Each phase must be testable independently

---

## Design Principles

### SOLID
- **Single Responsibility**: Each module handles one concern
  - `territory.inc` → Territory operations only
  - `resources.inc` → Resource utilities only
  - `buildings.inc` → Building system only
- **Open/Closed**: Configuration-driven (cfg files for tuning)
- **Liskov Substitution**: Consistent interfaces across modules
- **Interface Segregation**: Small, focused include files
- **Dependency Inversion**: Data access through DFTools abstraction

### DRY (Don't Repeat Yourself)
- ONE function for territory lookup
- ONE function for resource counting
- ONE function for kingdom validation
- Reuse existing guild system (don't reinvent)

### KISS (Keep It Simple, Stupid)
- Clear, descriptive function names
- Simple data structures
- No premature optimization
- Remove complexity where possible

---

## Architecture

### Package Structure
```
pkg/systems/kingdom/
├── pkg.cfg                        # Package configuration
│
├── config/
│   ├── settings.cfg               # System settings (timers, costs, rates)
│   ├── cultures.cfg               # 8 cultures + 5 levels each
│   ├── buildings.cfg              # Building types, costs, benefits
│   ├── territories.cfg            # Territory size templates (levels 1-5)
│   └── waves.cfg                  # Champion wave configurations
│
├── include/
│   ├── constants.inc              # All system constants
│   ├── kingdom.inc                # Core kingdom management functions
│   ├── territory.inc              # Territory CRUD operations
│   ├── resources.inc              # Resource utilities
│   ├── buildings.inc              # Building system functions
│   ├── upkeep.inc                 # Upkeep calculations
│   ├── conquest.inc               # Conquest/victory logic
│   └── waves.inc                  # Wave spawning utilities
│
├── datafile/
│   └── datafile.inc               # DFTools wrappers for kingdom data
│
├── core/
│   ├── start.src                  # System initialization
│   ├── resource_controller.src    # Resource generation loop
│   └── upkeep_controller.src      # Upkeep deduction + Discord notifications
│
├── conquest/
│   ├── war_totem.src              # War totem (GM-placed for conquest)
│   ├── kingdom_totem.src          # Kingdom totem (claim territory)
│   ├── wave_controller.src        # Wave spawning and progression
│   └── boss_handler.src           # Boss spawn and victory detection
│
├── items/
│   ├── donation_chest.src         # Kingdom resource storage
│   ├── building_foundation.src    # Building placement
│   ├── building_scaffolding.src   # Construction scaffolding
│   └── management_board.src       # Kingdom management UI (map on table)
│
├── commands/
│   └── gm/
│       └── kingdom.src            # .kingdom GM command (CRUD + admin)
│
└── itemdesc.cfg                   # Item definitions for kingdom items
```

### Data Flow
```
┌─────────────────────────────────────────────────────────────┐
│                      GM: Place War Totem                     │
│                    Configure waves/boss                      │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│              Players: Defeat Waves → Boss Dies              │
│                War Totem → Kingdom Totem                     │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│         Faction Leader: Click Kingdom Totem → Claim         │
│              Create Kingdom + Capital Territory             │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│           Management Board: Build, Manage, Expand           │
│   - Place buildings → Generate resources                    │
│   - Pay upkeep → Gain XP → Level up → Expand territories   │
└─────────────────────────────────────────────────────────────┘
```

---

## Data Structures

### Datafile Organization

#### 1. Kingdoms (`:kingdom:kingdoms`)
Stores kingdom/faction data using DFTools.

```escript
// Element name: faction_name (e.g., "MinhaTropa")
{
  capital_territory: "territory_001",      // Territory ID
  territories: ["territory_001", "territory_002"],  // Array
  culture: "Aigla",                        // One of 8 cultures
  level: 1,                                // 1-5
  xp: 0,                                   // Total guild XP
  founded_date: 1234567890,                // POLCore().systime
  upkeep_warnings: 0                       // Failed upkeep count
}
```

**Access Pattern:**
```escript
var df := DFTools(":kingdom", "kingdoms", faction_name);
var territories := df.GetPropertyValue("territories");
```

---

#### 2. Territories (`:kingdom:territories`)
Stores territory data.

```escript
// Element name: territory_name (e.g., "territory_001")
{
  name: "Fortaleza do Norte",              // Display name
  rect: "1000 1000 1100 1100 britannia",   // x1 y1 x2 y2 realm
  size_level: 1,                           // 1-5 (affects rect size)
  owner_faction: "",                       // Faction name or "" if unclaimed
  is_capital: 0,                           // 1 if capital, 0 otherwise
  rare_resources: ["Iron", "Gold"],        // Array of resource names
  buildings: [],                           // Array of building serials
  war_totem_serial: 0,                     // Serial if unclaimed
  management_board_serial: 0,              // Serial if claimed
  chest_serial: 0,                         // Donation chest serial
  claimed_date: 0,                         // When claimed
  next_resource_time: 0,                   // Next rare resource generation
  next_common_time: 0                      // Next common resource generation
}
```

**Access Pattern:**
```escript
var df := DFTools(":kingdom", "territories", territory_name);
var owner := df.GetPropertyValue("owner_faction");
```

---

#### 3. Resources (`:kingdom:resources`)
Stores faction resource counts.

```escript
// Element name: faction_name
{
  Comida: 0,
  Moedas: 0,
  Couro: 0,
  Metal: 0,
  Madeira: 0,
  Pano: 0,
  Pedra: 0,
  Joias: 0,
  last_sync: 1234567890                   // Last datafile sync
}
```

**Access Pattern:**
```escript
var df := DFTools(":kingdom", "resources", faction_name);
var gold := df.GetPropertyValue("Moedas");
df.SetPropertyValue("Moedas", gold + 100);
```

---

#### 4. Waves (`:kingdom:waves`)
Wave templates for conquest.

```escript
// Element name: wave_template (e.g., "goblin_raid")
{
  description: "Invasão Goblin",
  wave1_template: ":ghaia:goblin",
  wave1_count: 5,
  wave2_template: ":ghaia:goblinwarrior",
  wave2_count: 3,
  wave3_template: ":ghaia:goblinarcher",
  wave3_count: 4,
  boss_template: ":ghaia:goblinchief",
  boss_phrase: "You dare challenge the chief?!",
  boss_sound: 0x1F4,
  victory_reward: ""                      // Optional reward
}
```

---

### Item Properties (CProps)

#### War Totem
```escript
kingdom_territory_name     # Territory this totem controls
kingdom_wave_template    # Wave template name
kingdom_current_wave     # Current wave (0-3, 3=boss)
kingdom_total_kills      # Kills in current wave
kingdom_required_kills   # Required kills to advance
kingdom_active           # Boolean: conquest in progress
```

#### Kingdom Totem (Post-Victory)
```escript
kingdom_territory_name     # Territory ID
kingdom_claimable        # Boolean
```

#### Building Foundation
```escript
kingdom_territory_name     # Which territory
kingdom_building_type    # "farm", "house", etc.
kingdom_building_level   # 1-5
kingdom_faction          # Owner faction
```

#### Building Scaffolding
```escript
kingdom_building_info    # Struct with all building data
kingdom_materials        # Dict: required materials
kingdom_completion_time  # POLCore().systime when done
```

#### Management Board
```escript
kingdom_territory_name     # Territory ID
kingdom_faction          # Owner faction
```

#### Donation Chest
```escript
kingdom_territory_name     # Territory ID
kingdom_faction          # Owner faction
kingdom_resource_cache   # Cached resource counts
kingdom_last_cache       # Last cache update
```

---

### Guild Properties (CProps on Guild Object)

```escript
kingdom_territories      # Array of territory IDs
kingdom_xp              # Total XP
kingdom_level           # 1-5
kingdom_culture         # Culture name
```

---

## Constants Reference

All constants defined in `include/constants.inc`:

### Resource Types
```escript
const RESOURCE_COMIDA := "Comida";
const RESOURCE_MOEDAS := "Moedas";
const RESOURCE_COURO := "Couro";
const RESOURCE_METAL := "Metal";
const RESOURCE_MADEIRA := "Madeira";
const RESOURCE_PANO := "Pano";
const RESOURCE_PEDRA := "Pedra";
const RESOURCE_JOIAS := "Joias";
```

### Cultures
```escript
const CULTURE_AIGLA := "Aigla";
const CULTURE_LESTE := "Leste";
const CULTURE_HAROLOTH := "Haroloth";
const CULTURE_BJORSKE := "Bjorske";
const CULTURE_ALURIEL := "Aluriel";
const CULTURE_POLKINEA := "Polkinea";
const CULTURE_BADURAN := "Baduran";
const CULTURE_GULTHRAK := "Gulthrak";
```

### Timing
```escript
const RESOURCE_INTERVAL_RARE := 3600;      // 1 hour in seconds
const RESOURCE_INTERVAL_COMMON := 7200;    // 2 hours in seconds
const UPKEEP_INTERVAL := 604800;           // 1 week in seconds
const CONSTRUCTION_BASE_TIME := 86400;     // 1 day in seconds
```

### Building Types
```escript
const BUILDING_HOUSE := "house";
const BUILDING_FARM := "farm";
const BUILDING_QUARRY := "quarry";
const BUILDING_LUMBERMILL := "lumbermill";
const BUILDING_TANNERY := "tannery";
```

---

## Phase 1: Foundation

**Goal**: Set up package structure, core includes, and basic territory management.
**Duration**: Week 1
**Deliverable**: Can create territories and view them via GM command.

### Tasks

#### 1.1 Package Setup
- [ ] Create `pkg/systems/kingdom/` directory structure
- [ ] Create `pkg.cfg` with dependencies
- [ ] Create all subdirectories (config, include, datafile, core, conquest, items, commands)
- [ ] Create placeholder files for all includes

#### 1.2 Configuration Files
- [ ] Create `config/settings.cfg` with timing/cost settings
- [ ] Create `config/cultures.cfg` with 8 cultures and level names
- [ ] Create `config/territories.cfg` with 5 size level templates
- [ ] Create `config/buildings.cfg` (basic structure, details in Phase 4)
- [ ] Create `config/waves.cfg` (basic structure, details in Phase 2)

#### 1.3 Core Includes

##### `include/constants.inc`
- [ ] Define all resource type constants
- [ ] Define all culture constants
- [ ] Define timing constants
- [ ] Define building type constants
- [ ] Define wave/conquest constants

##### `include/datafile.inc`
- [ ] Create `KD_OpenKingdomsDF()` - Opens `:kingdom:kingdoms`
- [ ] Create `KD_OpenTerritoriesDF()` - Opens `:kingdom:territories`
- [ ] Create `KD_OpenResourcesDF()` - Opens `:kingdom:resources`
- [ ] Create `KD_OpenWavesDF()` - Opens `:kingdom:waves`
- [ ] Create `KD_GetKingdom(faction_name)` - Returns kingdom data
- [ ] Create `KD_GetTerritory(territory_name)` - Returns territory data
- [ ] Create `KD_SaveKingdom(faction_name, data)` - Saves kingdom data
- [ ] Create `KD_SaveTerritory(territory_name, data)` - Saves territory data

##### `include/territory.inc`
- [ ] Create `TRTY_CreateTerritory(territory_name, name, rect, size_level, rare_resources)` - Creates new territory
- [ ] Create `TRTY_GetTerritoryInfo(territory_name)` - Returns territory struct
- [ ] Create `TRTY_ClaimTerritory(territory_name, faction_name, is_capital)` - Claims territory
- [ ] Create `TRTY_UnclaimTerritory(territory_name)` - Releases territory
- [ ] Create `TRTY_IsInTerritory(x, y, realm, territory_name)` - Checks if coords in territory
- [ ] Create `TRTY_GetTerritoryAtLocation(x, y, realm)` - Finds territory at coords
- [ ] Create `TRTY_ListAllTerritories()` - Returns array of all territory IDs
- [ ] Create `TRTY_ListFactionTerritories(faction_name)` - Returns faction's territories
- [ ] Create `TRTY_DeleteTerritory(territory_name)` - Deletes territory (cleanup)

##### `include/kingdom.inc`
- [ ] Create `KGD_CreateKingdom(faction_name, capital_territory, culture)` - Creates kingdom
- [ ] Create `KGD_GetKingdomInfo(faction_name)` - Returns kingdom struct
- [ ] Create `KGD_AddTerritory(faction_name, territory_name)` - Adds territory to kingdom
- [ ] Create `KGD_RemoveTerritory(faction_name, territory_name)` - Removes territory
- [ ] Create `KGD_DeleteKingdom(faction_name)` - Deletes kingdom entirely
- [ ] Create `KGD_KingdomExists(faction_name)` - Boolean check
- [ ] Create `KGD_GetCapitalTerritory(faction_name)` - Returns capital territory ID
- [ ] Create `KGD_ValidateFactionLeader(who, faction_name)` - Checks if player is leader

#### 1.4 GM Command

##### `commands/gm/kingdom.src`
Create `.kingdom` command with gump menu:
- [ ] Main menu gump with options
- [ ] **Create Territory** - Prompts for: name, size level (1-5), rare resources
  - Creates territory at GM's location
  - Places War Totem at center
  - Saves to datafile
- [ ] **List Territories** - Shows all territories (claimed and unclaimed)
- [ ] **View Territory Info** - Select territory, show full details
- [ ] **Delete Territory** - Remove territory and cleanup
- [ ] **List Kingdoms** - Shows all kingdoms and their territories
- [ ] **View Kingdom Info** - Select kingdom, show full details
- [ ] **Delete Kingdom** - Remove kingdom entirely
- [ ] **Reload Configs** - Reload all cfg files

#### 1.5 Testing Phase 1
- [ ] Compile all scripts
- [ ] Use `.kingdom` to create a test territory
- [ ] Verify territory saved in datafile
- [ ] View territory info
- [ ] Delete territory
- [ ] Verify cleanup

**Phase 1 Complete When:**
- ✅ All includes compile without errors
- ✅ GM can create territories
- ✅ Territories persist in datafile
- ✅ Can view and delete territories

---

## Phase 2: Conquest System

**Goal**: Implement champion-style wave combat for territory conquest.
**Duration**: Week 2
**Deliverable**: Players can fight waves, defeat boss, and claim territory.

### Tasks

#### 2.1 Wave Configuration

##### `config/waves.cfg`
- [ ] Create 3-5 wave templates with different themes:
  - Goblin raid
  - Undead siege
  - Orc warband
  - Elemental invasion
  - Bandit attack
- [ ] Define wave1, wave2, wave3, boss for each template
- [ ] Add boss phrases and sounds

#### 2.2 Conquest Includes

##### `include/conquest.inc`
- [ ] Create `CNQST_StartConquest(war_totem)` - Initializes conquest
- [ ] Create `CNQST_SpawnWave(war_totem, wave_number)` - Spawns wave NPCs
- [ ] Create `CNQST_OnNPCDeath(war_totem, npc)` - Handles NPC death in conquest
- [ ] Create `CNQST_AdvanceWave(war_totem)` - Moves to next wave
- [ ] Create `CNQST_SpawnBoss(war_totem)` - Spawns final boss
- [ ] Create `CNQST_OnBossDeath(war_totem)` - Handles victory
- [ ] Create `CNQST_TransformToKingdomTotem(war_totem)` - Converts totem
- [ ] Create `CNQST_CancelConquest(war_totem)` - Aborts conquest
- [ ] Create `CNQST_GetConquestStatus(war_totem)` - Returns status struct

##### `include/waves.inc`
- [ ] Create `WAVE_LoadTemplate(template_name)` - Loads wave config
- [ ] Create `WAVE_GetWaveInfo(template, wave_number)` - Returns wave details
- [ ] Create `WAVE_GetBossInfo(template)` - Returns boss details
- [ ] Create `WAVE_CalculateSpawnPoints(center_x, center_y, count)` - Returns spawn coords

#### 2.3 War Totem

##### `conquest/war_totem.src`
War totem use script:
- [ ] If GM: Show configuration menu
  - Select wave template
  - Start/stop conquest
  - View status
- [ ] If player: Show conquest status
  - Current wave
  - Kills remaining
  - Instructions
- [ ] Handle totem initialization
- [ ] Set CProps for territory and wave template

#### 2.4 Wave Controller

##### `conquest/wave_controller.src`
Attached to war totem or runs as process:
- [ ] Monitor conquest state
- [ ] Spawn wave NPCs at intervals
- [ ] Track NPC deaths (attach CProp to NPCs with totem serial)
- [ ] Advance to next wave when kills complete
- [ ] Spawn boss on wave 3 completion
- [ ] Detect boss death
- [ ] Call victory function

**Implementation Options:**
- **Option A**: Script attached to totem, runs in loop
- **Option B**: Central process monitoring all active conquests
- **Recommended**: Option A (simpler for v0)

#### 2.5 Boss Handler

##### `conquest/boss_handler.src`
Functions for boss spawning and death:
- [ ] `SpawnBoss(war_totem, boss_template)` - Spawns boss
  - Set CProp on boss linking to war_totem
  - Boss announces phrase
  - Play sound effect
- [ ] `OnBossDeath(boss, killer)` - Called from death script
  - Find war_totem via CProp
  - Call victory function
  - Transform totem
  - Announce victory to area

**Integration Point:** Modify death script or use script attachment

#### 2.6 Kingdom Totem

##### `conquest/kingdom_totem.src`
Kingdom totem (post-victory claim):
- [ ] On use by player:
  - Check if player is faction leader
  - If yes: Prompt "Claim this territory for [faction]?"
  - If accept: Call `ClaimTerritoryForFaction()`
- [ ] `ClaimTerritoryForFaction(who, territory_name)`
  - Get faction from player
  - Check if faction has kingdom
    - If no: Create kingdom (this becomes capital)
    - If yes: Add as expansion territory
  - Claim territory in datafile
  - Spawn management board
  - Spawn donation chest
  - Remove kingdom totem
  - Announce to faction

#### 2.7 GM Command Updates

##### `commands/gm/kingdom.src`
Add conquest-related options:
- [ ] **Test Conquest** - Start conquest on selected territory (bypass waves)
- [ ] **Force Victory** - Instantly win conquest
- [ ] **Reset Conquest** - Reset war totem to unclaimed state

#### 2.8 Testing Phase 2
- [ ] GM creates territory with war totem
- [ ] Configure wave template via totem
- [ ] Start conquest
- [ ] Verify wave 1 spawns
- [ ] Kill all wave 1 NPCs, verify wave 2 spawns
- [ ] Kill all wave 2 NPCs, verify wave 3 spawns
- [ ] Kill all wave 3 NPCs, verify boss spawns
- [ ] Boss announces phrase
- [ ] Kill boss, verify totem transforms to kingdom totem
- [ ] Faction leader clicks kingdom totem
- [ ] Verify kingdom created in datafile
- [ ] Verify territory claimed
- [ ] Verify management board and chest spawned

**Phase 2 Complete When:**
- ✅ Conquest waves work end-to-end
- ✅ Boss spawns and victory detected
- ✅ Faction leader can claim territory
- ✅ Kingdom created with capital territory

---

## Phase 3: Resources

**Goal**: Resource generation and storage system.
**Duration**: Week 3
**Deliverable**: Territories generate resources, stored in donation chest.

### Tasks

#### 3.1 Resource Includes

##### `include/resources.inc`
- [ ] Create `RES_GetFactionResources(faction_name)` - Returns resource dict
- [ ] Create `RES_SetFactionResource(faction_name, resource_type, amount)` - Sets resource count
- [ ] Create `RES_AddFactionResource(faction_name, resource_type, amount)` - Adds resources
- [ ] Create `RES_RemoveFactionResource(faction_name, resource_type, amount)` - Removes (returns success)
- [ ] Create `RES_MapResourceName(resource_name)` - Maps name to objtype
- [ ] Create `RES_ClassifyResource(objtype)` - Returns resource category
- [ ] Create `RES_CountPhysicalResources(chest)` - Counts items in chest
- [ ] Create `RES_SyncChestToDatafile(chest, faction_name)` - Syncs chest → datafile
- [ ] Create `RES_SyncDatafileToChest(chest, faction_name)` - Syncs datafile → chest
- [ ] Create `RES_CreateResourceItem(objtype, amount, props)` - Creates/stacks resource item

**Resource Mapping Table** (in constants or cfg):
```
Iron → 0x1BF2 (Metal)
Gold → 0x0EED (Joias)
Wood → 0x1BD7 (Madeira)
Leather → 0x1081 (Couro)
Cloth → 0x1766 (Pano)
Stone → 0x1779 (Pedra)
Fish → 0x097A (Comida)
Wheat → 0x1EBD (Comida)
Copper Coins → 0xBA63 (Moedas)
```

#### 3.2 Donation Chest

##### `items/donation_chest.src`
- [ ] `use` script: Opens gump showing resources
  - Display all 8 resource categories with counts
  - Option to deposit items (converts to category count)
  - Option to withdraw items (creates items from count)
  - Permission check (only faction members)
- [ ] `canInsert` script: Validates resource type
- [ ] `onInsert` script: Categorizes and updates count
  - Destroy physical item
  - Increment datafile counter
- [ ] `canRemove` script: Blocks direct removal (must use gump)
- [ ] `onRemove` script: (blocked)

**Chest Properties:**
```escript
kingdom_territory_name
kingdom_faction
kingdom_last_sync
```

#### 3.3 Resource Controller

##### `core/resource_controller.src`
Background process that generates resources:
- [ ] `main()` loop:
  - Sleep for check interval (e.g., 60 seconds)
  - Load all territories from datafile
  - For each territory with owner:
    - Check if rare resource generation due (`next_resource_time`)
    - If due: Generate rare resources
    - Check if common resource generation due (`next_common_time`)
    - If due: Generate common resources (based on buildings)
- [ ] `GenerateRareResources(territory_name)`
  - Load territory data
  - Get rare_resources array
  - Get capital territory chest
  - Add resources to datafile
  - Update `next_resource_time`
- [ ] `GenerateCommonResources(territory_name)`
  - Load territory data
  - Get buildings array
  - For each building, check type and level
  - Calculate production
  - Add to datafile
  - Update `next_common_time`

**Process Management:**
- [ ] Register process to prevent duplicates
- [ ] Log start/stop events
- [ ] Handle errors gracefully

#### 3.4 Management Board - Resources View

##### `items/management_board.src`
Add resource viewing (full UI in Phase 5):
- [ ] Show current resource counts for kingdom
- [ ] Show resource generation rates
- [ ] Show last generation times
- [ ] Button to sync chest (for testing)

#### 3.5 Start Script

##### `core/start.src`
- [ ] Start resource_controller.src as background process
- [ ] Log startup
- [ ] Export function `StartKingdomControllers()`

#### 3.6 Testing Phase 3
- [ ] Create kingdom with territory
- [ ] Configure rare resources for territory
- [ ] Start resource controller
- [ ] Wait for generation interval
- [ ] Verify resources added to datafile
- [ ] Open donation chest, verify resources shown
- [ ] Deposit items into chest
- [ ] Verify counts updated
- [ ] Withdraw items from chest
- [ ] Verify counts decreased
- [ ] Test with multiple territories

**Phase 3 Complete When:**
- ✅ Resource controller generates resources
- ✅ Donation chest displays resource counts
- ✅ Can deposit and withdraw resources
- ✅ Datafile and chest stay synced

---

## Phase 4: Buildings

**Goal**: Building construction system.
**Duration**: Week 3-4
**Deliverable**: Can place buildings, construct them, and they produce resources.

### Tasks

#### 4.1 Building Configuration

##### `config/buildings.cfg`
Define 5 building types with 5 levels each:
- [ ] **House** - Increases population cap
  - Level 1-5: Different sizes, population bonuses
- [ ] **Farm** - Produces Comida, Trigo
  - Level 1-5: Increasing production rates
- [ ] **Quarry** - Produces Pedra
  - Level 1-5: Increasing production
- [ ] **Lumbermill** - Produces Madeira
  - Level 1-5: Increasing production
- [ ] **Tannery** - Produces Couro
  - Level 1-5: Increasing production

**Config Structure per Building:**
```
BuildingType house
{
  Level 1
  {
    Name       Small House
    Materials
    {
      Wood    100
      Stone   50
    }
    ConstructionTime  3600
    PopulationBonus   5
    ResourceProduction
    {
      // None for houses
    }
    Blueprint
    {
      // Item placements (x, y, z offsets, objtype, hue)
      Item 0 0 0 0x0064 0
      Item 1 0 0 0x0065 0
      // etc.
    }
  }
  Level 2 { ... }
}
```

#### 4.2 Building Includes

##### `include/buildings.inc`
- [ ] Create `BLD_LoadBuildingConfig()` - Loads buildings.cfg
- [ ] Create `BLD_GetBuildingInfo(building_type, level)` - Returns building struct
- [ ] Create `BLD_CanPlaceBuilding(who, territory_name, building_type, level)` - Validates placement
- [ ] Create `BLD_PlaceFoundation(who, territory_name, building_type, level)` - Places foundation item
- [ ] Create `BLD_StartConstruction(foundation)` - Converts foundation to scaffolding
- [ ] Create `BLD_GetRequiredMaterials(building_type, level)` - Returns materials dict
- [ ] Create `BLD_DepositMaterials(scaffolding, item)` - Adds material to construction
- [ ] Create `BLD_CheckConstructionComplete(scaffolding)` - Checks if all materials deposited
- [ ] Create `BLD_CompleteConstruction(scaffolding)` - Builds final building from blueprint
- [ ] Create `BLD_GetProduction(building_type, level)` - Returns production struct
- [ ] Create `BLD_DemolishBuilding(building_serial, territory_name)` - Removes building

#### 4.3 Building Foundation

##### `items/building_foundation.src`
Foundation item placed by player:
- [ ] On use: Show gump
  - Building info (type, level, requirements)
  - Button: "Start Construction" → Converts to scaffolding
  - Button: "Cancel" → Returns foundation to player
- [ ] Validate placement in territory
- [ ] Set CProps: `kingdom_territory_name`, `kingdom_building_type`, `kingdom_building_level`, `kingdom_faction`

#### 4.4 Building Scaffolding

##### `items/building_scaffolding.src`
Scaffolding during construction:
- [ ] On use: Show construction gump
  - List required materials with progress bars
  - Button: "Deposit Materials" → Target item to deposit
  - Show estimated completion time
  - Button: "Cancel Construction" → Destroys scaffolding (no refund for v0)
- [ ] On material deposit:
  - Validate material type
  - Update materials dict
  - Check if complete
  - If complete: Start timer, set `kingdom_completion_time`
- [ ] Timer handler:
  - Check if `POLCore().systime >= kingdom_completion_time`
  - If yes: Call `BLD_CompleteConstruction()`

**Scaffolding Properties:**
```escript
kingdom_building_info      # Struct: type, level, territory_name, faction
kingdom_materials          # Dict: {Wood: 50/100, Stone: 50/50}
kingdom_completion_time    # POLCore().systime when construction finishes
```

#### 4.5 Final Building

##### `items/building_controller.src`
Final building after construction:
- [ ] On use: Show building info
  - Building name and level
  - Production rate (if applicable)
  - Option: "Demolish" (GM or leader only)
- [ ] Set CProps:
  - `kingdom_building_info` - Full struct
  - `kingdom_pieces` - Array of item serials in blueprint
- [ ] Store production info for resource controller

#### 4.6 Building Construction Flow

**Blueprint System:**
- [ ] `BuildFromBlueprint(x, y, z, realm, building_type, level)`
  - Load blueprint from cfg
  - Create each item at offset positions
  - Set items as non-movable
  - Store serials in array
  - Return controller item serial

**Demolition:**
- [ ] `DemolishBuilding(building_serial)`
  - Load `kingdom_pieces` array
  - Destroy each piece
  - Remove from territory buildings list
  - Destroy controller item
  - Log event

#### 4.7 Management Board - Building UI

##### `items/management_board.src`
Add building management page:
- [ ] List all buildings in territory
- [ ] Show construction in progress
- [ ] Button: "Place New Building"
  - Show available building types
  - Show requirements
  - Give foundation item to player

#### 4.8 Resource Controller Updates

##### `core/resource_controller.src`
Update `GenerateCommonResources()`:
- [ ] For each building in territory:
  - Load building info
  - Get production from cfg
  - Add resources to datafile

#### 4.9 Testing Phase 4
- [ ] Open management board
- [ ] Request foundation for Farm Level 1
- [ ] Place foundation in territory
- [ ] Start construction
- [ ] Deposit required materials (Wood, Stone, etc.)
- [ ] Verify materials progress updates
- [ ] Wait for construction timer
- [ ] Verify building completes
- [ ] Verify blueprint items spawned
- [ ] Wait for resource generation
- [ ] Verify farm produces Comida
- [ ] Test demolition

**Phase 4 Complete When:**
- ✅ Can place and construct buildings
- ✅ Buildings appear from blueprint
- ✅ Buildings produce resources
- ✅ Can demolish buildings

---

## Phase 5: Upkeep & Progression

**Goal**: Upkeep system and guild XP/leveling.
**Duration**: Week 4-5
**Deliverable**: Kingdoms pay upkeep, gain XP, level up, expand territories.

### Tasks

#### 5.1 Upkeep Includes

##### `include/upkeep.inc`
- [ ] Create `UPK_CalculateUpkeep(faction_name)` - Returns upkeep cost (in Moedas)
  - Base cost per territory
  - Additional cost per building
  - Configurable in settings.cfg
- [ ] Create `UPK_PayUpkeep(faction_name, cost)` - Deducts from resources
  - Remove Moedas from datafile
  - Return success/failure
- [ ] Create `UPK_RecordUpkeepFailure(faction_name)` - Increments warning counter
- [ ] Create `UPK_ResetUpkeepWarnings(faction_name)` - Resets counter on successful payment
- [ ] Create `UPK_GetUpkeepStatus(faction_name)` - Returns struct with next due, cost, warnings
- [ ] Create `UPK_SendDiscordNotification(faction_name, type, message)` - Sends Discord webhook

#### 5.2 Upkeep Controller

##### `core/upkeep_controller.src`
Background process for upkeep deduction:
- [ ] `main()` loop:
  - Sleep for check interval (e.g., every hour)
  - Load all kingdoms
  - For each kingdom:
    - Check if upkeep due (weekly)
    - Calculate cost
    - Attempt payment
    - If success: Reset warnings, log, send Discord success
    - If fail: Increment warnings, log, send Discord warning
    - Update next upkeep time
- [ ] Discord integration:
  - Warning message on failure
  - Success message on payment
  - Format: "Kingdom [Name] paid upkeep of X coins" or "WARNING: Kingdom [Name] missed upkeep!"

#### 5.3 Guild XP System

**Option A: Integrate with existing XP system**
- Check if server has guild XP system in `pkg/systems/`
- If yes: Hook into it
- If no: Implement from scratch (Option B)

**Option B: Implement standalone (assuming no existing system)**

##### `include/kingdom.inc` (additions)
- [ ] Create `KGD_AddXP(faction_name, amount)` - Adds XP to kingdom
  - Check if upkeep is paid (blocked if warnings > 0)
  - Add XP to datafile
  - Check for level up
  - If level up: Call `KGD_LevelUp()`
- [ ] Create `KGD_GetXPForLevel(level, culture)` - Returns XP required for level
  - Load from cultures.cfg
- [ ] Create `KGD_LevelUp(faction_name, new_level)` - Handles level up
  - Update level in datafile
  - Grant bonuses (population, morale, etc.)
  - Announce to faction members
  - Check for territory expansion unlock
- [ ] Create `KGD_CanExpand(faction_name)` - Checks if can add territory
  - Level-based cap (e.g., Level 1: 1 territory, Level 5: 5 territories)

**XP Gain Integration:**
- Add hook in death script or combat system:
  ```escript
  // In death script after XP calculation
  if (player has faction with territory) {
    var guild_xp := player_xp * 1.0; // 100% of player XP
    KGD_AddXP(faction_name, guild_xp);
  }
  ```

#### 5.4 Cultures Configuration

##### `config/cultures.cfg`
Define 8 cultures with 5 levels each:
- [ ] Each culture has unique level names
- [ ] XP requirements per level
- [ ] Bonuses per level (population, morale, etc.)

**Example Structure:**
```
Culture Aigla
{
  Level 1
  {
    Name            Aldeia
    XPRequired      0
    PopulationBonus 0
    MoraleBonus     0
    TerritoryLimit  1
  }
  Level 2
  {
    Name            Vila
    XPRequired      10000
    PopulationBonus 10
    MoraleBonus     5
    TerritoryLimit  2
  }
  // ... up to Level 5: Império
}

Culture Leste { ... }
Culture Haroloth { ... }
// etc. for all 8 cultures
```

#### 5.5 Territory Expansion

When kingdom levels up:
- [ ] Check if new territory limit unlocked
- [ ] Announce to faction
- [ ] Allow claiming additional territories via conquest

**Expansion Flow:**
- Faction conquers new territory → Claims it → Added as expansion (not capital)
- Max territories based on level

#### 5.6 Management Board - Full UI

##### `items/management_board.src`
Complete management gump with multiple pages:
- [ ] **Overview Page**
  - Kingdom name, culture, level
  - Total XP and progress to next level
  - Number of territories
  - Total population
- [ ] **Resources Page**
  - Display all 8 resource counts
  - Resource generation rates
  - Button: "Open Donation Chest"
- [ ] **Buildings Page**
  - List all buildings across all territories
  - Show construction in progress
  - Button: "Place New Building"
- [ ] **Territories Page**
  - List all territories
  - Show capital
  - Show resource generation per territory
- [ ] **Upkeep Page**
  - Current upkeep cost
  - Next upkeep due date
  - Upkeep warnings (if any)
  - Resource balance vs upkeep (warning if low)
- [ ] **Members Page** (if applicable)
  - List faction members
  - Contribution tracking (optional for v0)

#### 5.7 Start Script Updates

##### `core/start.src`
- [ ] Start upkeep_controller.src as background process
- [ ] Verify both controllers running

#### 5.8 Testing Phase 5
- [ ] Create kingdom with resources
- [ ] Manually set upkeep due date to trigger
- [ ] Verify upkeep deducted from resources
- [ ] Verify Discord notification sent
- [ ] Test upkeep failure (remove resources)
- [ ] Verify warning counter incremented
- [ ] Verify Discord warning sent
- [ ] Add XP to kingdom
- [ ] Verify level up occurs at threshold
- [ ] Verify level up announcement
- [ ] Verify territory limit increased
- [ ] Conquer and claim second territory
- [ ] Verify expansion territory added

**Phase 5 Complete When:**
- ✅ Upkeep controller deducts resources weekly
- ✅ Discord notifications sent on upkeep events
- ✅ Guild XP accumulates from player kills
- ✅ Kingdoms level up with proper bonuses
- ✅ Can expand to multiple territories

---

## Phase 6: Polish & Testing

**Goal**: Balance, bug fixes, and beta readiness.
**Duration**: Week 5-6
**Deliverable**: Stable beta release for player testing.

### Tasks

#### 6.1 Balance Tuning
- [ ] Resource generation rates
  - Test with 1, 3, 5 territories
  - Ensure resources accumulate reasonably
- [ ] Upkeep costs
  - Should be achievable but require active play
  - Test with different territory/building counts
- [ ] Building construction times
  - Should feel meaningful but not punishing
  - Consider 1 hour to 24 hours depending on level
- [ ] XP requirements for leveling
  - Test progression from Level 1 to Level 5
  - Should take weeks, not days
- [ ] Wave difficulty
  - Ensure waves are challenging but beatable
  - Test with 1, 3, 5 players

#### 6.2 Error Handling
- [ ] Validate all user inputs
- [ ] Graceful handling of missing datafile entries
- [ ] Handle edge cases:
  - Player leaves faction during construction
  - Faction disbands with territories
  - Totem destroyed during conquest
  - Chest destroyed or moved
  - Buildings destroyed manually
- [ ] Add error logging for all critical functions

#### 6.3 Logging System
- [ ] Log all kingdom events:
  - Kingdom creation/deletion
  - Territory claim/unclaim
  - Building construction/demolition
  - Resource generation
  - Upkeep payments/failures
  - Level ups
- [ ] Use consistent log format
- [ ] Log to `:kingdom:logs` datafile or file

#### 6.4 GM Tools
- [ ] **Debug Commands** in `.kingdom`:
  - Add resources to kingdom
  - Add XP to kingdom
  - Force level up
  - Skip construction timer
  - Reset upkeep timer
  - View all datafile entries
- [ ] **Cleanup Tools**:
  - Purge all kingdoms (wipe for testing)
  - Purge all territories
  - Reset specific kingdom

#### 6.5 Player Documentation
- [ ] Create in-game help gump accessible from management board
- [ ] Explain conquest process
- [ ] Explain resource system
- [ ] Explain building construction
- [ ] Explain upkeep and consequences
- [ ] Explain XP and leveling

#### 6.6 Code Review
- [ ] Review all includes for:
  - Consistent naming conventions
  - Proper error handling
  - No hardcoded values (use constants/cfg)
  - Comments on complex logic
- [ ] Ensure SOLID/DRY/KISS principles followed
- [ ] Remove any debug code or commented-out blocks

#### 6.7 Stress Testing
- [ ] Create 10 territories
- [ ] Create 5 kingdoms
- [ ] Run resource controller for 24 hours
- [ ] Run upkeep controller for 24 hours
- [ ] Monitor for memory leaks or errors
- [ ] Test with multiple GMs and players simultaneously

#### 6.8 Migration from Old System (if needed)
- [ ] If any old kingdom data exists, create migration script
- [ ] Map old data structure to new structure
- [ ] Test migration on copy of production data
- [ ] Provide rollback plan

#### 6.9 Final Testing Checklist
- [ ] Complete conquest flow (waves → boss → claim)
- [ ] Resource generation over 24 hours
- [ ] Building construction (all types and levels)
- [ ] Upkeep payment and failure
- [ ] XP gain and level up
- [ ] Territory expansion (claim 2nd territory)
- [ ] Multiple kingdoms coexisting
- [ ] GM tools (create, delete, debug)
- [ ] Faction leader changes (permissions)
- [ ] Faction disbands (cleanup)

**Phase 6 Complete When:**
- ✅ No critical bugs
- ✅ Balance feels reasonable for beta
- ✅ Logging and error handling robust
- ✅ Player documentation complete
- ✅ Ready for beta player testing

---

## Function Reference

### Territory Functions (`include/territory.inc`)

#### `TRTY_CreateTerritory(territory_name, name, rect, size_level, rare_resources)`
Creates a new territory.

**Parameters:**
- `territory_name` (string): Unique ID for territory
- `name` (string): Display name
- `rect` (string): "x1 y1 x2 y2 realm"
- `size_level` (int): 1-5, determines rect size
- `rare_resources` (array): Array of resource names

**Returns:** `1` on success, `0` on failure

**Example:**
```escript
var result := TRTY_CreateTerritory(
  "territory_001",
  "Northern Fortress",
  "1000 1000 1100 1100 britannia",
  1,
  array{"Iron", "Gold"}
);
```

---

#### `TRTY_GetTerritoryInfo(territory_name)`
Retrieves full territory data.

**Parameters:**
- `territory_name` (string): Territory ID

**Returns:** Struct with territory data or `0` if not found

**Example:**
```escript
var info := TRTY_GetTerritoryInfo("territory_001");
print("Owner: " + info.owner_faction);
print("Capital: " + info.is_capital);
```

---

#### `TRTY_ClaimTerritory(territory_name, faction_name, is_capital)`
Claims territory for faction.

**Parameters:**
- `territory_name` (string): Territory ID
- `faction_name` (string): Faction name
- `is_capital` (int): 1 if capital, 0 otherwise

**Returns:** `1` on success, `0` on failure

**Example:**
```escript
TRTY_ClaimTerritory("territory_001", "MinhaTropa", 1);
```

---

#### `TRTY_IsInTerritory(x, y, realm, territory_name)`
Checks if coordinates are within territory bounds.

**Parameters:**
- `x`, `y` (int): Coordinates
- `realm` (string): Realm name
- `territory_name` (string): Territory ID

**Returns:** `1` if inside, `0` if outside

---

### Kingdom Functions (`include/kingdom.inc`)

#### `KGD_CreateKingdom(faction_name, capital_territory, culture)`
Creates a new kingdom.

**Parameters:**
- `faction_name` (string): Faction name
- `capital_territory` (string): Territory ID for capital
- `culture` (string): One of 8 cultures

**Returns:** `1` on success, `0` on failure

**Example:**
```escript
KGD_CreateKingdom("MinhaTropa", "territory_001", CULTURE_AIGLA);
```

---

#### `KGD_AddXP(faction_name, amount)`
Adds XP to kingdom and checks for level up.

**Parameters:**
- `faction_name` (string): Faction name
- `amount` (int): XP to add

**Returns:** `1` on success, `0` on failure (e.g., upkeep not paid)

---

### Resource Functions (`include/resources.inc`)

#### `RES_AddFactionResource(faction_name, resource_type, amount)`
Adds resources to faction.

**Parameters:**
- `faction_name` (string): Faction name
- `resource_type` (string): One of 8 resource types (use constants)
- `amount` (int): Quantity to add

**Returns:** `1` on success

**Example:**
```escript
RES_AddFactionResource("MinhaTropa", RESOURCE_MOEDAS, 100);
```

---

#### `RES_RemoveFactionResource(faction_name, resource_type, amount)`
Removes resources from faction.

**Parameters:**
- `faction_name` (string): Faction name
- `resource_type` (string): Resource type
- `amount` (int): Quantity to remove

**Returns:** `1` if sufficient resources and removed, `0` if insufficient

---

### Building Functions (`include/buildings.inc`)

#### `BLD_PlaceFoundation(who, territory_name, building_type, level)`
Places a building foundation at player's location.

**Parameters:**
- `who` (mobile): Player placing building
- `territory_name` (string): Territory ID
- `building_type` (string): Building type (use constants)
- `level` (int): Building level (1-5)

**Returns:** Foundation item serial or `0` on failure

---

#### `BLD_CompleteConstruction(scaffolding)`
Completes construction, builds from blueprint.

**Parameters:**
- `scaffolding` (item): Scaffolding item

**Returns:** Final building controller serial or `0` on failure

---

### Conquest Functions (`include/conquest.inc`)

#### `CNQST_StartConquest(war_totem)`
Starts conquest on a war totem.

**Parameters:**
- `war_totem` (item): War totem item

**Returns:** `1` on success, `0` on failure

---

#### `CNQST_OnNPCDeath(war_totem, npc)`
Called when NPC in conquest dies.

**Parameters:**
- `war_totem` (item): War totem managing conquest
- `npc` (mobile): NPC that died

**Returns:** `1` if wave complete, `0` otherwise

---

### Upkeep Functions (`include/upkeep.inc`)

#### `UPK_CalculateUpkeep(faction_name)`
Calculates weekly upkeep cost.

**Parameters:**
- `faction_name` (string): Faction name

**Returns:** Cost in Moedas (int)

**Formula:**
```
Base cost + (territories * territory_cost) + (buildings * building_cost)
```

---

## Testing Checklist

### Phase 1: Foundation
- [ ] Package compiles without errors
- [ ] `.kingdom` command works
- [ ] Can create territory
- [ ] Territory saved in `:kingdom:territories` datafile
- [ ] Can view territory info
- [ ] Can delete territory
- [ ] Territory deletion cleans up datafile

### Phase 2: Conquest
- [ ] War totem spawns at territory center
- [ ] GM can configure wave template
- [ ] Conquest starts on activation
- [ ] Wave 1 spawns correct NPCs
- [ ] Killing all wave 1 advances to wave 2
- [ ] Wave 2 spawns
- [ ] Killing all wave 2 advances to wave 3
- [ ] Wave 3 spawns
- [ ] Killing all wave 3 spawns boss
- [ ] Boss announces phrase
- [ ] Boss plays sound effect
- [ ] Killing boss transforms totem to kingdom totem
- [ ] Kingdom totem is claimable
- [ ] Faction leader can claim
- [ ] Non-leader cannot claim
- [ ] Claiming creates kingdom in datafile
- [ ] Capital territory set correctly
- [ ] Management board spawns
- [ ] Donation chest spawns

### Phase 3: Resources
- [ ] Resource controller starts
- [ ] Rare resources generate after interval
- [ ] Common resources generate after interval
- [ ] Resources saved to `:kingdom:resources` datafile
- [ ] Donation chest opens and shows counts
- [ ] Can deposit items into chest
- [ ] Deposited items increment resource count
- [ ] Can withdraw items from chest
- [ ] Withdrawn items decrement resource count
- [ ] Multiple territories generate independently

### Phase 4: Buildings
- [ ] Can request foundation from management board
- [ ] Foundation placed at player location
- [ ] Foundation can start construction
- [ ] Scaffolding shows required materials
- [ ] Can deposit materials
- [ ] Material progress updates
- [ ] Construction completes after timer
- [ ] Blueprint items spawn
- [ ] Building controller item created
- [ ] Building registered in territory
- [ ] Building produces resources
- [ ] Can demolish building
- [ ] Demolition removes all pieces

### Phase 5: Upkeep & Progression
- [ ] Upkeep controller starts
- [ ] Upkeep deducts Moedas weekly
- [ ] Successful payment logs and notifies Discord
- [ ] Failed payment increments warning counter
- [ ] Failed payment notifies Discord
- [ ] Player kills grant guild XP
- [ ] XP blocked if upkeep not paid
- [ ] Kingdom levels up at XP threshold
- [ ] Level up announces to faction
- [ ] Level up increases territory limit
- [ ] Can claim additional territory after level up

### Phase 6: Polish
- [ ] No critical bugs found
- [ ] Error messages are clear
- [ ] Logs capture all important events
- [ ] GM debug commands work
- [ ] Player documentation is accessible
- [ ] Balance feels reasonable
- [ ] System stable over 24+ hours
- [ ] Multiple kingdoms can coexist
- [ ] Faction disbanding cleans up kingdom

---

## Configuration Examples

### `config/settings.cfg`
```
# Kingdom System Settings

# Resource Generation
RareResourceInterval        3600      # 1 hour in seconds
CommonResourceInterval      7200      # 2 hours in seconds

# Upkeep
UpkeepInterval              604800    # 1 week in seconds
UpkeepBaseCost              1000      # Base cost in Moedas
UpkeepCostPerTerritory      500       # Additional cost per territory
UpkeepCostPerBuilding       100       # Additional cost per building
UpkeepWarningThreshold      3         # Warnings before penalties

# Construction
ConstructionBaseTime        3600      # 1 hour in seconds
ConstructionTimeMultiplier  1.5       # Multiplier per level

# XP
XPFromKills                 1.0       # 100% of player XP goes to guild
XPBlockedOnUpkeepFail       1         # Block XP if upkeep not paid

# Discord
DiscordWebhookEnabled       1
DiscordUpkeepNotifications  1
DiscordConquestNotifications 1
```

---

### `config/cultures.cfg`
```
Culture Aigla
{
  Level 1
  {
    Name            Aldeia
    XPRequired      0
    PopulationBonus 0
    MoraleBonus     0
    TerritoryLimit  1
  }
  Level 2
  {
    Name            Vila
    XPRequired      10000
    PopulationBonus 10
    MoraleBonus     5
    TerritoryLimit  2
  }
  Level 3
  {
    Name            Cidade
    XPRequired      50000
    PopulationBonus 25
    MoraleBonus     10
    TerritoryLimit  3
  }
  Level 4
  {
    Name            Reino
    XPRequired      150000
    PopulationBonus 50
    MoraleBonus     20
    TerritoryLimit  4
  }
  Level 5
  {
    Name            Império
    XPRequired      500000
    PopulationBonus 100
    MoraleBonus     50
    TerritoryLimit  5
  }
}

# Repeat for all 8 cultures: Leste, Haroloth, Bjorske, Aluriel, Polkinea, Baduran, Gulthrak
```

---

## Notes for Implementation

### Development Workflow
1. **Start each phase by creating skeleton files**
2. **Compile early and often** - catch syntax errors immediately
3. **Test incrementally** - don't wait until everything is done
4. **Use syslog() liberally** - logging is your friend
5. **One function at a time** - implement, test, commit

### Common Pitfalls to Avoid
- **Don't hardcode values** - use constants or cfg files
- **Don't skip error handling** - always validate inputs
- **Don't forget datafile.Commit()** - changes won't persist otherwise
- **Don't use sleep() in item scripts** - use timers or processes
- **Don't trust user input** - validate everything

### POL-Specific Reminders
- **Datafile elements must be created** with `CreateElement()` before setting properties
- **CProps are lost if item is destroyed** - always save critical data to datafile
- **Scripts run in separate threads** - be careful with shared data
- **Use `sleepms(2)` in loops** to prevent runaway scripts
- **Test with `CacheInteractiveScripts 0`** during development for instant reloads

---

## Appendix: 8 Cultures

1. **Aigla** - Western culture, classic European fantasy
2. **Leste** - Eastern culture, Asian-inspired
3. **Haroloth** - Northern culture, Viking-inspired
4. **Bjorske** - Arctic culture, harsh climate dwellers
5. **Aluriel** - Elven culture, forest dwellers
6. **Polkinea** - Trade culture, merchant-focused
7. **Baduran** - Desert culture, nomadic
8. **Gulthrak** - Orcish culture, tribal warriors

Each culture has unique level names but same mechanical progression (for v0).

---

## Version History

- **v0.1.0** - Initial planning document (2025-10-12)

---

## Future Enhancements (Post-v0)

### Deferred Features
- War system between kingdoms
- Senate and law system
- 8-stage building visual progression
- Building auras and buffs
- Dynamic events (catastrophes, prosperity)
- Caravan system for resource transport
- NPC population with AI
- Rebellions and loyalty system
- Trade routes between kingdoms
- Kingdom customization (flags, colors, motto)

### Potential Improvements
- Mobile app for kingdom management
- Web dashboard for stats
- Tournament system between kingdoms
- Seasonal events
- Kingdom achievements and titles

---

**End of Document**

---

This document is a living document. Update checkboxes as tasks are completed and add notes for future reference.