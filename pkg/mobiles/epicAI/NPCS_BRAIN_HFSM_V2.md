# NPC Brain HFSM V2

## What Was Done

Complete refactor of the NPC AI brain system across 6 phases, focused on performance and anti-exploit improvements while maintaining separate brain files (monster/animal/humanoid) for independent domain evolution.

### Architecture Decision: HFSM + Component Toolbox

We evaluated 6 design patterns (Behavior Tree, Utility AI, Hierarchical FSM, Component Toolbox, Middleware Pipeline, Strategy Pattern) against eScript's constraints (no function pointers, no classes, structs + case dispatch). 

**Winner: HFSM + Component Toolbox** because:
- HFSM makes blocking loops naturally non-blocking (flee is a STATE, not a loop)
- Cheapest per-tick overhead (one case dispatch on a string)
- Each brain defines its own state graph - zero coupling
- Component toolbox follows existing patterns (npcDataCache.inc)
- Brains stay separate - different domains, different reasons to change

### Why Separate Brains?
- Animals handle herding, following, wool - monsters don't
- Monsters handle looting, doppelganger - animals don't  
- Humanoids handle class abilities (warrior/rogue/sage), equipment, combat history
- Future brains planned: GuardBrain (factions), TamedBrain (pets)
- Even if 70% looks the same, the 30% difference serves different purposes

---

## Phase 1: Spell Cache

**Problem:** `GetSpellCircle()` iterated 4 config files with `sleepms(2)` per entry. Called O(n^2) times inside `SortSpellsByPriority`'s bubble sort. With 10 spells = ~100 calls x hundreds of config entries = seconds of delay.

**Solution:** `spellCache.inc` - dictionary built once (lazily on first use), O(1) lookups.

**Functions replaced:**
- `GetSpellCircle()` -> `GetSpellCircleCached()` 
- `FindMageSpellInConfigs()` / `FindSpellInConfig()` -> `FindSpellInfoCached()`
- `getAvailableSpells()` -> `GetAvailableSpellsCached(school, tier)`
- `GetAvailableShamanSpells()` -> `GetAvailableShamanSpellsCached(tier)`
- `GetAvailablePriestSpells()` -> `GetAvailablePriestSpellsCached(tier)`

**File:** `include/spellCache.inc` (new)

---

## Phase 2: State Manager (HFSM Framework)

**Problem:** Blocking while-loops in flee (20 iterations x 100ms = 2s), ambush (up to 30s), herding, following. These block ALL event processing - NPC can't respond to damage while fleeing.

**Solution:** `stateManager.inc` - states process ONE step per brain tick, events always fire.

**States:** IDLE, FLEEING, AMBUSH, HERDING, FOLLOWING, COMBAT

**Key functions:**
- `SM_TransitionTo(state, data)` - enter a new state
- `SM_GetState()` - check current state
- `SM_TickFlee()` - one flee step per tick
- `SM_TickAmbush()` - one ambush check per tick
- `SM_TickHerding()` - one movement step per tick
- `SM_TickFollowing()` - one follow step per tick

**File:** `include/stateManager.inc` (new)

---

## Phase 3: All Brains Integrated

Applied state manager to all 3 brain types:

**monsterBrain:** FLEEING, AMBUSH states. Ambush breaks on DAMAGED event.
**animalBrain:** FLEEING, AMBUSH, HERDING, FOLLOWING states. Neutral animals flee-first on ENGAGED/DAMAGED/ENTEREDAREA (before any combat logic).
**humanoidBrain:** FLEEING, AMBUSH states. Ambush breaks on DAMAGED event.

**Blocking loops eliminated:**
| Function | Was | Now |
|----------|-----|-----|
| FleeFrom | 20-iter while + sleepms(100) | FLEEING state, 1 step/tick |
| HandleAmbush | 30s while + sleep(2) | AMBUSH state, 1 check/tick |
| EVID_HERDING | while + sleepms(120) | HERDING state, 1 step/tick |
| FOLLOW_CMD | while + sleepms(300) | FOLLOWING state, 1 step/tick |

---

## Phase 4: Ability Dispatch Table

**Problem:** 12+ sequential `if(me.canX())` checks per combat tick, most evaluating to false for any given NPC.

**Solution:** `abilityTable.inc` - pre-built arrays from NPC cache at brain activation.

**How it works:**
1. `BuildCombatAbilities(me)` at brain activation -> array of 1-4 entries (this NPC's actual abilities)
2. `TickCombatAbilities(me, opponent, table)` per tick -> iterate only relevant entries, check range + cooldown
3. `ExecuteCombatAbility(id, me, opponent)` -> case dispatch to actual function

**14 abilities mapped:** AcidSpray, WebShot, ThrowSpecial, Bite, TongueGrab, PoisonBite, CharmOpponent, SporeSpecial, HiveMind, doConfusion, WallClimb, petrify, frightenOpponent, MageBehavior

**Enter-area abilities:** Charge, TunnelTactics, Ambush

**File:** `include/abilityTable.inc` (new)

---

## Phase 5: ObjProperty Reduction

**Problem:** SetObjProperty/GetObjProperty serialize to disk. 204 calls across the codebase, many in per-tick combat paths.

**Converted to script-level vars (zero serialization):**
| ObjProperty | Frequency | Script Var |
|---|---|---|
| `#nearby_cache_{range}_{flags}` | 6+ per tick (serializing mobile arrays!) | `nearby_cache_var` |
| `#allies_8` | Every ally scan | Piggybacks on `GetNearbyMobilesOnce` |
| `#enemies_8` | Every enemy scan | Piggybacks on `GetNearbyMobilesOnce` |
| `#enemy_last_loc` | Every pursuit tick (3 calls) | `enemy_last_loc_var` |
| `initial_allies` | Every pursuit tick | `initial_allies_var` |

**Kept as ObjProperty (cross-script dependencies):**
`#taunter`, `#taunt_opponent`, `flee_health`, `target_priority`, `metamorfose`, `anchor`, `myhabs`, `CustomAbilities`

---

## Phase 6: Anti-Exploit Hardening

**Using POL native properties instead of manual tracking:**

- **`me.lastcoord`** (struct{lastx, lasty, lastz}): If `me.x == me.lastcoord.lastx`, the last move hit a wall. Used in flee, pursuit, and ranged retreat for instant stuck detection.
- **Flee stuck handler:** 3 consecutive wall hits = try random direction. 6 = teleport to anchor.
- **Archer cornering:** If ranged retreat fails (wall hit) and no alternative positions, immediately `SwitchToMelee()`.
- **Pursuit stuck:** After 10s with <30% movement success, teleport to anchor (replaces blocking `Flee()` call).

**Additional blocking loops eliminated in PursueTarget for ranged NPCs:**
| Loop | Was | Now |
|------|-----|-----|
| Ranged retreat | while 10 attempts x sleepms(100) | Single step per tick |
| Ranged direct movement | while 10 attempts x sleepms(100) | Single step per tick |

---

## Phase 7: Ladino Performance Fixes

**Problem:** Ladino (Rogue) NPCs had 3 categories of performance issues surviving the V2 refactor: blocking loops in steal/sneak (up to 2s each freezing events), per-tick disk I/O from `GetObjProperty("myhabs")`, and O(n) config scans in `ProccessHab` on every ability use.

### Fix 1: STEALING/SNEAKING HFSM States

**Blocking loops eliminated:**
| Function | Was | Now |
|----------|-----|-----|
| DoThiefSteal | while(Distance>1) + sleepms(100) | STEALING state, 2-phase (approach + steal), 1 step/tick |
| DoThiefSneakAttack | while(!IsBehind) x 20 + sleepms(100) | SNEAKING state, 1 positioning step/tick |

**States added to `stateManager.inc`:**
- `SM_TickStealing()` - Phase "approach": walk one step toward target. Phase "steal": skill check + item move. 15s timeout.
- `SM_TickSneaking()` - Walk toward behind position, backstab when `IsBehind && Distance <= 1`. Fallback attack after 10 attempts. 20 max attempts.
- Both states break on SYSEVENT_DAMAGED (alongside AMBUSH).

### Fix 2: Cache `myhabs` as Script-Level Vars

**Problem:** `UseAbilities()` called `GetObjProperty(npc, "myhabs")` every combat tick - disk deserialization of the full ability array.

**Solution:** Two new script-level vars in `brain.src`:
| Var | Purpose | Loaded At |
|-----|---------|-----------|
| `myhabs_var` | Cached ability ID array | Brain activation |
| `myhabs_set_var` | Dictionary for O(1) `.exists()` checks | Brain activation |

`ChooseWarriorAbility`, `ChooseRogueAbility`, `ChooseSageAbility` no longer rebuild a `hab_set` dictionary each tick.

### Fix 3: NPC_UseAbilityById (Bypass ProccessHab)

**Problem:** `ProccessHab(npc, abilityId)` iterates ALL config keys with `NormalizeString()` comparison to find the matching ability - O(n) on every ability use. The NPC already has the integer ID.

**Solution:** `NPC_UseAbilityById(npc, abilityId)` in `humanoidBrain.inc`:
- Uses `FindConfigElem(habcfg, abilityId)` - O(1) direct key lookup
- Validates cooldown, stamina, mana
- Calls `Ability(abilityId)` -> `AbilityMonitor().ActivateOn()` directly
- Skips mimic logic and client gump updates (NPC-only path)
- `ProccessHab` remains untouched for player use

### Fix 4: GrantPrivilege at Brain Activation

**Problem:** `GrantPrivilege(me, "hiddenattack")` + `me.enable("hiddenattack")` called every steal/sneak attempt (2-4 POL calls per invocation).

**Solution:** Granted once at brain activation for `me.isThief()` NPCs in both `humanoidBrain` and `monsterBrain`. Privileges: `hiddenattack`, `runwhilestealth`.

---

## Phase 8: Mage AI Optimization + Undead Healing Bug Fix

**Bug Fix:** Undead/demon NPCs (e.g. Skeleton Mage Necromancer) were casting healing spells on themselves and allies, which `applyEpicHealing()` converts to 50% damage — effectively killing themselves. Fixed by checking `category_lower` before any healing decision.

**Where undead checks were added:**
- `CheckForWoundedAllies()` in `epicai.inc` — filters undead/demon allies from healing target list
- `ChooseIdealShamanSpells()` — gates self-heal (Cura Grave/Leve) and AoE heal boost on `!self_is_undead`
- `ChoosePriestSpell()` — gates emergency self-heal and filters undead from ally healing loop

**Performance: Eliminated redundant per-cast work in `NPCMageryUtils.inc`:**

| Optimization | Was | Now |
|---|---|---|
| Spell category arrays | 6 arrays rebuilt every spell decision (every 5s) | Script-level vars, lazy-init once per brain lifetime |
| `getSpellSchool()` | Called 3x per spell decision (cache + GetObjProperty each) | Called 1x, threaded as parameter through chain |
| `#SpellTarget` ObjProperty | 3 disk serialization ops per cast (Set + Get + Erase) | `spell_target_var` script-level var (zero cost) |
| `FindSpellInfoCached()` | Called 2-3x per cast (ChooseSpell + Behavior + CastSpell) | Called 1x, result passed through as struct |
| `ListHostiles(me, 10)` | Uncached POL native scan per spell decision | `NPC_GetPlayersPetsAndSummonsNearby()` (1s cached scan) |
| `sleepms(2)` in small loops | Yielded in DoHarmfulSpell ally loop, priest ally loops | Removed (iterating cached arrays < 10 items) |
| Corpse-targeting spells | `ListItemsNearLocation` scan with `sleepms(2)` per item | "Prolongar Desmaio"/"Levantar os Mortos" added to non-ideal filter |

**Structural change:** `ChooseSpell()` and `ChooseShamanSpell()` now return `struct{ "name", "info" }` instead of a bare spell name, carrying the spell config lookup result through to `NPC_Cast*Spell()` to avoid re-lookup.

**Duplicate target setting eliminated:** Shaman path had target set in both `ChooseShamanSpell` and `HumanoidShamanBehavior`. Now set once in `ChooseShamanSpell` only.

---

## Files Created/Modified

### New Files
- `include/spellCache.inc` - O(1) spell data lookups
- `include/stateManager.inc` - HFSM framework  
- `include/abilityTable.inc` - Pre-built ability dispatch tables

### Modified Files
- `brain.src` - Includes, InitSpellCache, script-level vars (+ myhabs_var, myhabs_set_var), dormant reset
- `include/epicai.inc` - ObjProperty -> script vars, non-blocking pursuit, anti-exploit
- `include/NPCMageryUtils.inc` - 6 functions replaced with cached versions
- `include/epicSpecial.inc` - HandleAmbush converted to state transition, DoThiefSteal/DoThiefSneakAttack converted to HFSM state transitions
- `include/stateManager.inc` - SM_TickStealing(), SM_TickSneaking() added
- `include/monsterBrain.inc` - State machine + ability table + STEALING/SNEAKING dispatch + Ladino privilege grants at activation
- `include/animalBrain.inc` - State machine + ability table + flee-first for neutrals
- `include/humanoidBrain.inc` - State machine + ability table + NPC_UseAbilityById + myhabs caching + STEALING/SNEAKING dispatch + Ladino privilege grants at activation
- `include/npcUtils.inc` - Ally/enemy scan piggybacks on GetNearbyMobilesOnce
- `include/makeRace.inc` - Hair/beard skip-if-exists guards

---

## Important eScript/POL Concepts

### sleepms(2) is a cooperative yield
POL uses cooperative multitasking. `sleepms(2)` in foreach loops is a yield point that lets other NPC scripts run. Cannot be removed blindly. Strategy: eliminate long loops through caching so there's nothing to yield from. Short loops (< 10 cached items) don't need yields.

### Script-level vars vs ObjProperty
- Script-level vars: zero cost, reset when brain goes dormant, only visible within the brain script
- ObjProperty: serializes to disk, survives script restart, visible to any script via serial lookup
- Rule: use script vars for ephemeral combat state, ObjProperty only for cross-script communication

### Adding New Abilities
1. Add to `BuildAbilitiesCache` in `npcDataCache.inc` (if template-based)
2. Add to `BuildCombatAbilities` in `abilityTable.inc` with range
3. Add case to `ExecuteCombatAbility` in `abilityTable.inc`
4. Implement the ability function in `epicSpecial.inc`

### Adding New Brain Types
1. Create `newBrain.inc` with its own event loop and state graph
2. Include `stateManager.inc` for state management
3. Call `SM_TransitionTo("IDLE")` at activation
4. Build ability tables: `combat_abilities := BuildCombatAbilities(me)`
5. Add case to `brain.src` dispatch

---

## Further Performance Improvements

### Already Optimized (from prior work)
- NPC data cache (90% config read reduction)
- Event-driven dormancy (zero CPU when no players nearby)
- Threat list caching (event-driven incremental updates)
- LOS caching (1-second TTL)
- Path caching (8-second TTL)
- Dynamic sleep (100ms combat, 1000ms idle)

### Potential Future Optimizations
1. **Move `firewhilemoving` privilege to npcdesc.cfg `Privs`** for ranged templates - removes runtime GrantPrivilege calls during init
2. **Dead code cleanup** - the old `Flee()` function (50-iteration blocking version) may now be unused
3. **`me.weapon.maxrange`** - POL native property could replace `GetWeaponRangeInfo()` config lookup
4. **`NPC.run_speed` dynamic adjustment** - for charge states (speed up) or exhaustion (slow down)
5. **Batch NPC initialization** - `InitializeEnterArea` sends events with `sleepms(5)` between each. Could batch without sleep for faster activation.
6. **`CheckForIdealTarget` optimization** - still iterates potential opponents with per-iteration scoring. Could use the threat list cache to pre-filter.
7. **Combat history system** (`LoadCombatHistory` in humanoidBrain) - verify this isn't doing unnecessary file I/O per activation.
8. **`ShouldFlee` caching** - called multiple times per event in animalBrain (ENGAGED, DAMAGED, ENTEREDAREA). Could cache the result for the current tick.
