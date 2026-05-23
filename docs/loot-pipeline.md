# Loot Pipeline — Agent Reference

Canonical, code-grounded map of every path that places items in a
container that a player can eventually pull from: regular NPC kills,
epic boss kills, treasure-chest spawners, and treasure-map digs.
Written for agents working on loot, economy, magic affixes, or boss
rewards. Everything was traced firsthand to the cited `file:line`.
**If you change a loot path, update this doc.**

Read this instead of re-tracing the pipeline.

## TL;DR

- **One generator, four producers.**
  `Loot_Generate(container, loot_index, source := 0)` in
  [pkg/systems/loot/include/lootParser.inc](../pkg/systems/loot/include/lootParser.inc#L209)
  is the single chokepoint. Every loot source feeds it.
- Loot for regular NPCs is **rolled at spawn, not at death**. The
  brain init script (`BrainCriticalScript`) fills the NPC's backpack
  the moment they spawn; death just exposes the corpse. Epic bosses
  get a **second** death-time pass for `on_death` items.
- `loot_index.cfg` keywords: `Item` `Stack` `Random` `Group` `Magic`
  `MagicSpell`. Subtlety: `Group` inside a `LootIndex` references
  **another LootIndex** (recurses through `Loot_Generate`), not a
  group in `loot-groups.cfg`. `loot-groups.cfg` is read only by
  `Random` and the (legacy) `Magic`. See pitfalls.
- True bosses gate their drops with **personal loot**: items are
  cloned per participant and visibility is enforced by a packet hook,
  not by item parenthood.
- **Epic chests use a base + flavor composition.** The spawner calls
  `Loot_Generate(chest, "EpicChestN")` for the shared base table,
  then optionally `Loot_Generate(chest, "EpicChestN_FlavorXxx")`
  for the GM-selected class overlay (Warrior / Thief / Artifice /
  Mage). `Misto` skips the overlay. There are 5 base + 20 flavor
  tables (down from 20 standalone tables). Legacy
  `HumanoGuerreiroChestN` / `HumanoLadinoChestN` /
  `HumanoArtificeChestN` / `HumanoSabioChestN` are retained as
  backstop but the spawner no longer references them.
- Treasure-map chests still use a single level-suffixed key
  (e.g. `treasuremap3`). Both chest paths pass `source = 0` which
  disables coin-tier filtering and the coin-pool drain.
- After `Loot_Generate` returns on a chest, the spawner calls
  `ChestLoot_PostProcess(chest)` to enforce chest-economy invariants
  (iron-lock mundane weapons/armor, strip stat mods from jewelry).
  This is chest-only — NPC loot is untouched.

## The chokepoint files

| Concern | File |
|---|---|
| Loot generation core | [pkg/systems/loot/include/lootParser.inc](../pkg/systems/loot/include/lootParser.inc) |
| Magic affix rolls (legacy) | [pkg/systems/loot/include/magicLoot.inc](../pkg/systems/loot/include/magicLoot.inc) |
| Magic spell-stick items (`MagicSpell` keyword) | [pkg/systems/loot/include/magicItemLoot.inc](../pkg/systems/loot/include/magicItemLoot.inc) |
| Magic-item name composer | [pkg/systems/loot/include/magicNameGen.inc](../pkg/systems/loot/include/magicNameGen.inc) |
| Chest post-process (iron-lock, jewelry strip) | [pkg/systems/loot/include/chestPostProcess.inc](../pkg/systems/loot/include/chestPostProcess.inc) |
| Epic-item property stamper (shared with GM gump) | [pkg/items/epicitem/include/properties.inc](../pkg/items/epicitem/include/properties.inc) |
| Loot index (entries by name) | [pkg/systems/loot/config/loot-index.cfg](../pkg/systems/loot/config/loot-index.cfg) |
| Random/Magic group pools | [pkg/systems/loot/config/loot-groups.cfg](../pkg/systems/loot/config/loot-groups.cfg) |
| Magic affix templates (legacy) | [pkg/systems/loot/config/templates.cfg](../pkg/systems/loot/config/templates.cfg) |
| Custom name → objtype map / magic-name pools | [pkg/systems/loot/config/item-names.cfg](../pkg/systems/loot/config/item-names.cfg) |
| NPC spawn-time loot | [pkg/mobiles/brainAI/scripts/init/generic.src](../pkg/mobiles/brainAI/scripts/init/generic.src) |
| NPC death handler | [pkg/mobiles/death/npcdeath.src](../pkg/mobiles/death/npcdeath.src) |
| Epic boss creation | [pkg/systems/epicspawn/include/epicBoss.inc](../pkg/systems/epicspawn/include/epicBoss.inc) |
| Epic boss GM tool | [pkg/systems/epicspawn/commands/gm/epicboss.src](../pkg/systems/epicspawn/commands/gm/epicboss.src) |
| Personal loot (true boss) | [pkg/items/containers/include/personalLoot.inc](../pkg/items/containers/include/personalLoot.inc) |
| Epic chest spawner | [pkg/systems/chestspawn/epicchestspawner.src](../pkg/systems/chestspawn/epicchestspawner.src) |
| Epic chest GM gump | [pkg/systems/chestspawn/commands/coun/epicchest.src](../pkg/systems/chestspawn/commands/coun/epicchest.src) |
| Treasure-map digging | [pkg/items/treasurehunting/include/treasuredigging.inc](../pkg/items/treasurehunting/include/treasuredigging.inc) |

## Loot_Generate — the only generator

Signature
([lootParser.inc:209](../pkg/systems/loot/include/lootParser.inc#L209)):

```escript
function Loot_Generate(container, loot_index, source := 0)
```

- `container` — backpack, corpse, chest. Anything that can hold items.
- `loot_index` — string key into
  [loot-index.cfg](../pkg/systems/loot/config/loot-index.cfg).
- `source` — the NPC that owns this loot, when applicable. **Used for
  coin-tier filtering** and trueboss multipliers. Chests/maps pass 0.

Body (lines 214–267):

1. Open `:loot:config/loot-index`, find element by key.
2. Iterate the entry's keywords in fixed order: `Item`, `Stack`,
   `Random`, `Group`, `Magic`. Each line is fully resolved before the
   next keyword starts.
3. After all keywords: 5% sealed-book injection
   (`Loot_TryAddSealedBook`, line 255).
4. Coin-pool drain (`Loot_TryDrainPool(container, source)`, line 258)
   — phase 4.5 economy. Only runs when a source NPC exists.
5. If the container is a true-boss corpse, `PersonalLoot_Stamp` is
   called (line 266) — see [§ True boss personal loot](#true-boss-personal-loot).

## The six loot keywords

All six live in `loot-index.cfg` entries. Line syntax is
space-delimited; `Loot_ParseString` reads positional fields. Quantity
fields accept dice notation (`1d10`).

### `Item <objtype|name> <qty_dice> [chance] [level]`

[Loot_CreateItem](../pkg/systems/loot/include/lootParser.inc#L289).
Creates a single non-stacking item per roll. If `level > 1`, routes
through the crafting framework (`GeraItemComplexo`) so the item gets
material/quality complexes. Otherwise plain
`CreateItemInContainer`. Stamps `PROPNAME_DISPENSAVEL_FLAG` so
auto-loot/etc. know the item is dispensable.

### `Stack <objtype|name> <qty_dice> [chance] [level]`

[Loot_CreateStack](../pkg/systems/loot/include/lootParser.inc#L349).
Creates one stackable amount per roll. Two extra concerns:

- **Coin-tier filter**
  ([Loot_CoinAllowedForTier](../pkg/systems/loot/include/lootParser.inc#L25)):
  copper coins drop from any NPC; silver requires NPC tier ≥ 3; gold
  requires tier ≥ 5. Tier comes from `source` NPC's npcdesc — if
  `source = 0`, no filter, so chests and treasure maps can drop any
  coin tier directly via `Stack`.
- **Economy audit**: every minted coin stack is recorded through
  `EA_Record(EA_MINT_LOOT, ...)` (line 362).

### `Random <GroupName> <qty_dice> [chance] [level]`

[Loot_CreateRandom](../pkg/systems/loot/include/lootParser.inc#L376).
This is the one keyword that reads
[loot-groups.cfg](../pkg/systems/loot/config/loot-groups.cfg). It
collects every `Item` and `Stack` line in the named group, rolls
`qty_dice` times, and picks a random entry per roll.

Special case: if the group is `LoreItems`, picks are filtered through
`LoreItems_IsDropped` against the `:unicos:lista` datafile so the
unique drops can't re-drop. New picks are registered with
`LoreItems_Register` (line 479).

### `Group <LootIndexName>`

Plain recursion. Calls `Loot_Generate(container, name, source)` with
the cited name. The infinite-loop guard is one line: `if (line !=
loot_index)` (line 241). A `LootIndex foo` containing `Group foo` is
silently skipped — see pitfalls.

### `Magic <GroupName> <qty> <chance> <level> <template>`

Bridges to
[magicLoot.inc](../pkg/systems/loot/include/magicLoot.inc).
`Loot_CreateMagic` (lootParser.inc:250) → `CreateMagicItem`
([magicLoot.inc:38](../pkg/systems/loot/include/magicLoot.inc#L38)).
The flow:

1. Pick a random base item from the named loot-group.
2. Create it.
3. Apply affixes via
   [SetMagicTemplateBonus](../pkg/systems/loot/include/magicLoot.inc#L64),
   which:
   - Reads the named entry from
     [templates.cfg](../pkg/systems/loot/config/templates.cfg).
   - Either picks a bonus type uniformly (template has an `all` key
     → `GetChosenBonus`) or by cumulative % ranges
     (`GetChosenBonusFromKeys`).
   - Bonus magnitudes (lines 182–185, 209):
     - `status` (STR/DEX/INT): `1d<level> × 3`
     - `vital` (HP/MANA/STAM): `1d<level> × 4`
     - `resistmod` (phys/fire/cold/poison/energy %): `1d<level> × 3`
     - `weaponarmor`: weapon `dmg_mod` / `chancetocritic` or armor
       resist mod (+ small chance to graft an on-hit script).
   - Color from the template's `cores` field; if `Random`, rolls
     1d1000.
4. Stamps `equip` (the bonus dict), `magic = 1`, and `magicColor` on
   the item.

The `Magic` keyword is **not used by any chest table** today; chests
use `MagicSpell` instead (below). `Magic` survives only for the
historical magic-affix path on NPC tables — keep it in mind when
auditing, but new chest loot should reach for `MagicSpell`.

### `MagicSpell <qty_dice> <chance> <tier>`

[Loot_RollMagicSpellLine](../pkg/systems/loot/include/magicItemLoot.inc)
→ `Loot_CreateMagicSpellItem(container, tier)`. Builds a "spell-stick"
magic item that double-clicks to cast a spell. Wraps
[ApplyEpicProperties](../pkg/items/epicitem/include/properties.inc) so
the result is byte-identical to an item built via the `.epicitem` GM
gump — same `equip` / `magic` / `epiceffect` schema, same
`usescript = :epicitem:epicitem_use`.

Flow per item:

1. Pick a base objtype from a small curated pool: rings, bracelets,
   earrings, amulets, longnecklaces, circlets, staves, scepters,
   wizardhats.
2. Pick a random spell from the whitelist:
   - **Schools:** `Cantrips`, `Evocatium`, `Ilusao`, `Necromancia`
     (class-locked schools like Paladin/DemonHunter/Druid/Shaman/
     Swordmage are deliberately excluded — their spells reference
     caster class state).
   - **Circle window scales with tier (sliding):**
     - T1 → circles 0–1
     - T2 → circles 0–2
     - T3 → circles 1–3
     - T4 → circles 2–4
     - T5 → circles 3–5
   - **Targets:** `Unico` or `Area` only (no `Self`; self-buffs on
     item sticks lead to surprise behavior).
3. Build the epic-item `state` struct. Charges are **flat 10** for
   every item regardless of tier or circle.
4. Roll `sacred` / `cursed` / plain magic with **15% / 15% / 70%**.
   Sacred items stamp `epicEntropy = +tier*100`; cursed stamp
   `-tier*100`. The existing equip gate at
   [equip.src:209-221](../pkg/systems/combat/equip.src#L209-L221)
   enforces the threshold on equip.
5. **Jewelry alternative**: if the base is jewelry (ring / bracelet /
   earrings / amulet / necklace / circlet) there's a 50% chance the
   item rolls a vital mod (`hitsmod` / `manamod` / `stammod` = +tier)
   instead of a spell. Spell-stick bases always roll the spell.
6. `ApplyEpicProperties` stamps everything. Item is tagged with
   `epic_magic_loot = 1` so `ChestLoot_PostProcess` skips the
   iron-lock and jewelry-mod-strip passes.
7. Name is composed via
   [Loot_GenerateMagicName](../pkg/systems/loot/include/magicNameGen.inc)
   from the existing `Substantivo` / `Adjetivo` / `Adverbio` pools in
   `item-names.cfg`. Example output: *"Anel da Sereia Brilhante da
   Aurora"*.

If no spell matches the circle window (unlikely with the current
school whitelist), the item falls back to a vital mod so the drop
still feels magical instead of being a plain ring.

## 1. Regular NPC death loot

The flow is split: **filling happens at spawn**, the **corpse just
inherits** the backpack when the NPC dies.

### Spawn

[BrainCriticalScript](../pkg/mobiles/brainAI/scripts/init/generic.src#L10)
fires once per NPC at spawn. It sets up the backpack, equips gear,
and calls
`Loot_Generate(bag, settings["Loot"], npc)` (line 30) where
`settings["Loot"]` is the `Loot` property from
[npcdesc.cfg](../pkg/mobiles/npc/npcdesc.cfg) (resolved by the brain
config loader; remember to strip the `:epicai:` namespace when
looking up an NPC by template name — that pitfall is in the brain
pipeline doc).

### Death

[core_npcDeath](../pkg/mobiles/death/npcdeath.src#L35) runs on every
NPC death. For regular NPCs it does **not** generate new loot — that
already happened at spawn. It handles:

- Doppelganger restoration (lines 42–45).
- `HandleEpicBossLoot(corpse)` (line 87) — no-op for non-boss NPCs.
- `PersonalLoot_Arm` / `PersonalLoot_Stamp` (lines 90–93) — true
  bosses only.
- Equipped weapon/armor durability hit: items not marked `itemunico`
  or `LootedBy` lose 80% of current HP (lines 80–84).
- `GuardKill` / `NoCorpse` checks (lines 399–459) that can suppress
  the loot, the corpse, or both.
- Tamed-pet resurrection branch (lines 96–154): pet may resurrect
  with their gear instead of leaving a lootable corpse.

## 2. Epic boss death loot

Epic bosses are still NPCs, so the **spawn-time pass** above still
runs (the brain still calls `Loot_Generate(bag, settings["Loot"],
npc)` for whatever loot index the boss template declares). On top of
that, the **boss editor** lets GMs attach a list of explicit
templated drops that fire at death.

### Boss creation

[CreateEpicBoss](../pkg/systems/epicspawn/include/epicBoss.inc#L10):

- Spawns the NPC.
- `SetObjProperty(npc, "EpicBossStruct", boss)` (line 24) — used by
  the death handler.
- If `boss.trueboss == 1`, stamps `true_boss` on the NPC (line 61),
  which is what gates personal-loot stamping at death.
- `SetObjProperty(npc, "LootItems", boss.loot)` (line 235) — the
  death-time drop list.

The `boss.loot` array is built in the GM gump
([epicboss.src](../pkg/systems/epicspawn/commands/gm/epicboss.src)):
each entry is a struct with `template`, `drop_chance` (1–100),
`on_death` (bool), `desc`. See
[docs/epicboss-gm-tool.md](epicboss-gm-tool.md) for the editor
contract.

### Death-time drop

[HandleEpicBossLoot](../pkg/mobiles/death/npcdeath.src#L498), called
from `core_npcDeath` line 87:

1. Read `EpicBossStruct` off the corpse.
2. For each entry in `boss_struct.loot`:
   - Skip if `on_death` is falsy.
   - Roll `drop_chance`.
   - `CreateItemInContainerFromTemplate(corpse, loot_item.template)`.
   - Force `item.movable := 1`.

This pass is **additive** to whatever the spawn-time `Loot_Generate`
already produced into the NPC's backpack. There is no double-roll
suppression — the `on_death` flag is the only line between the two
passes.

### True boss personal loot

[personalLoot.inc](../pkg/items/containers/include/personalLoot.inc).
Only true bosses (`true_boss` property) trigger this. Lifecycle:

1. `PersonalLoot_Arm(corpse)` — snapshots all nearby players (range
   32, LoS, `cmdlevel == 0`) into a `loot_participants` serial array
   and sets `loot_unlock_at = ReadGameClock() + 15`.
2. `PersonalLoot_Stamp(corpse)` — iterates corpse contents:
   - Items with an existing `loot_owner` are left alone (idempotent).
   - **Armor and weapons** stay untagged → first-come-first-served
     among participants.
   - **Everything else** is cloned once per participant, and each
     copy is stamped with `loot_owner = <serial>`.
3. The packet hook
   [pkg/packetHooks/personalLootView](../pkg/packetHooks/personalLootView)
   filters container contents on the way out: a participant only
   sees their own stamped copies + the untagged FFA pile.
4. No FFA fallback — owners must grab before the corpse decays.

## 3. Treasure chests (spawner-driven)

The live spawner is
[pkg/systems/chestspawn/epicchestspawner.src](../pkg/systems/chestspawn/epicchestspawner.src).
The GM placement gump is
[pkg/systems/chestspawn/commands/coun/epicchest.src](../pkg/systems/chestspawn/commands/coun/epicchest.src)
(`.epicchest`). The legacy `chestspawner.src` was deleted; if you see
it referenced anywhere, that's stale. `commands/coun/chestspawner.src`
is the `.chestspawner` GM command, not a spawner — don't conflate.

### Base + flavor composition

Each spawner item stores a `chest_type` ObjProperty set at placement
time: `"Warrior"`, `"Thief"`, `"Artifice"`, `"Mage"`, or `"Misto"`.
The spawner composes the chest's loot from a shared base table plus
an optional class overlay.

`MakeEpicChests`
([epicchestspawner.src:86](../pkg/systems/chestspawn/epicchestspawner.src#L86)):

- Resolves two cfg keys:
  - **Base** — `EpicChestN` (level 1–5). Every chest pulls from this.
    Drives coins, generic consumables, mundane gear, jewelry, lore
    items, sealed-book chance, and `MagicSpell` magic items.
  - **Flavor suffix** — `_FlavorWarrior` / `_FlavorThief` /
    `_FlavorArtifice` / `_FlavorMage`. Composed as
    `EpicChestN + flavor_suffix`. `Misto` skips the overlay entirely.
- For each chest, finds a clear location, places the chest item, sets:
  - `IsTreasureChest = 1`
  - `IsEpicChest = 1`
  - `level`
  - `chest_type` (forwarded from the spawner)
  - `PROPNAME_DURABILIDADE_MAX = maxhp * level`
  - `PROPNAME_MATERIAL`
  - `locked = 1`
- `AddRandomTraps(chest, level)` — needle/explosion/poison-gas traps;
  count scales with level.
- `Loot_Generate(chest, "EpicChestN")` — the base pass.
- `Loot_Generate(chest, "EpicChestN" + flavor_suffix)` — only if
  `chest_type != "Misto"`.
- `ChestLoot_PostProcess(chest)` — see below.
- `source = 0` on both calls means **no coin-tier filter and no
  coin-pool drain** — chests are static sources, not
  population-tier-gated.
- Rare chests get a `minigame_lock` flag (level-dependent 10–30%
  chance) to switch lockpicking into the minigame UI.

The chest's destruction/decay timer is started by
`:treasurehunting:chest/destroy` (not by the spawner itself).

### Why base + flavor (not 4 separate tables)

Before the rework there were 20 standalone tables — 4 chest types ×
5 tiers — each duplicating coins / consumables / jewelry / mundane
gear with small variations. Two problems:

- Every balance change required editing 20 places.
- A non-Warrior player looting a Sabio chest got nothing useful from
  the same content everyone else cleared.

Base + flavor solves both. The base table guarantees every player
gets usable loot from any chest. The flavor pack (4–6 lines each)
preserves GM theming: a wizard-boss room can still spawn Mage chests
that drop reagents and scrolls on top of the base contents.

The legacy `HumanoGuerreiroChestN` / `HumanoLadinoChestN` /
`HumanoArtificeChestN` / `HumanoSabioChestN` tables remain in
`loot-index.cfg` as a backstop but the spawner no longer references
them — they will be deleted in a follow-up cleanup pass.

### Puzzle and mimic chest paths

The puzzle-chest branch of the spawner skips the regular
`Loot_Generate` call inside its creation block, so it calls base +
flavor + post-process separately at the puzzle-success point
([epicchestspawner.src:266-271](../pkg/systems/chestspawn/epicchestspawner.src#L266-L271)).
Mimic chests follow the same code path as regular chests (loot is
filled on creation, the mimic property is just a flag).

### ChestLoot_PostProcess

[chestPostProcess.inc](../pkg/systems/loot/include/chestPostProcess.inc).
Runs after both `Loot_Generate` passes complete. Enforces two
invariants that should not leak into NPC loot:

1. **Iron-lock**: mundane weapons and armor have their `material`
   property forced to `FerroTemperadoIngot` (0x6309) so chest drops
   never outshine what crafters can make. Items tagged
   `epic_magic_loot` (from the `MagicSpell` keyword) are exempt —
   they're meant to be magical, not metalwork.
2. **Jewelry stat-mod strip**: removes `strmod`, `dexmod`, `intmod`
   from any item on the `LAYER_RING`, `LAYER_WRIST`, `LAYER_EARS`,
   or `LAYER_NECK` slots. Vital mods on jewelry are deliberate (the
   `MagicSpell` jewelry-alternative path) and are preserved.

The post-process is **chest-only**. NPC corpses don't run through it,
so NPC drop economics are unaffected by this pass.

## 4. Treasure maps

Map digging lives at
[treasuredigging.inc](../pkg/items/treasurehunting/include/treasuredigging.inc).

`DigForMap(who, targ, map)` (line 74) → `StartTreasureDigging(who,
targ, map)` (line 96):

- Read `maplevel := CInt(GetObjProperty(map, "level"))`.
- Build `loot := "treasuremap" + maplevel` (line 113), so a level-3
  map reads `LootIndex treasuremap3`.
- Create the chest 9 tiles below the digger (line 117) and play the
  9-stage emerge animation (lines 167–268), aborting if the digger
  steps off the target tile.
- Set chest properties (`level`, `MapLevel`, `FoundBy`, `movable =
  0`), call `AddRandomTraps(chest, maplevel)`, then
  `Loot_Generate(chest, loot)` (line 139).
- Lock the chest, optionally arm minigame lockpicking.
- `start_script(":treasurehunting:guardians/spawn", ...)` (line 272)
  spawns level-appropriate guardian mobs around the dig — those mobs
  drop their own loot via the regular spawn-time path, **not** the
  chest's index.
- `MarkMapComplete(map, who)` (line 90) consumes the map.

Same `source = 0` treatment as spawner chests: no coin-tier filter,
no coin-pool drain.

## Call chain summary

```
NPC SPAWN
  BrainCriticalScript (init/generic.src:10)
    NPC_SetupBackPack / NPC_SetupEquipment
    Loot_Generate(bag, settings["Loot"], npc)
      Loot_CreateItem        (Item)
      Loot_CreateStack       (Stack)        → Loot_CoinAllowedForTier
      Loot_CreateRandom      (Random)       → reads loot-groups.cfg
      Loot_Generate          (Group, recurses on loot-index.cfg)
      Loot_CreateMagic       (Magic)        → CreateMagicItem
                                               → SetMagicTemplateBonus
      Loot_RollMagicSpellLine(MagicSpell)   → Loot_CreateMagicSpellItem
                                               → pick spell (whitelist + tier circle)
                                               → ApplyEpicProperties
                                               → Loot_GenerateMagicName
      Loot_TryAddSealedBook  (5%)
      Loot_TryDrainPool      (coin economy, source ≠ 0 only)

NPC DEATH
  core_npcDeath (npcdeath.src:35)
    HandleEpicBossLoot(corpse)               ← epic boss on_death items
    PersonalLoot_Arm(corpse)   ─┐
    PersonalLoot_Stamp(corpse) ─┴ true boss only

EPIC BOSS SPAWN
  CreateEpicBoss (epicBoss.inc:10)
    set EpicBossStruct, LootItems, true_boss
    NPC spawns → BrainCriticalScript runs → spawn-time Loot_Generate

CHEST SPAWNER
  MakeEpicChests (epicchestspawner.src:86)
    base_lootname  := "EpicChest" + level
    flavor_suffix  := "_Flavor{Warrior|Thief|Artifice|Mage}"  // "" for Misto
    AddRandomTraps
    Loot_Generate(chest, base_lootname)               [source = 0]
    Loot_Generate(chest, base_lootname + flavor_suffix)  // if flavor != ""
    ChestLoot_PostProcess(chest)                      ← iron-lock + jewelry strip

TREASURE MAP
  DigForMap → StartTreasureDigging (treasuredigging.inc:96)
    loot := "treasuremap" + maplevel
    AddRandomTraps
    Loot_Generate(chest, loot)         [source = 0]
    ChestLoot_PostProcess(chest)
    start_script(:treasurehunting:guardians/spawn)
```

## Pitfalls

1. **`Group` inside a LootIndex references another LootIndex, not a
   `loot-groups.cfg` group.** If you write `LootIndex arrows` with a
   line `Group arrows`, the infinite-loop guard at
   [lootParser.inc:241](../pkg/systems/loot/include/lootParser.inc#L241)
   silently drops it. Use `Stack` or `Item` for the actual loot;
   reserve `Group` for chaining to a **different** LootIndex.
2. **Loot is rolled at spawn, not at death**, for regular NPCs.
   Editing an NPC's `Loot` property only takes effect on the next
   spawn — existing world NPCs keep their pre-rolled inventory.
3. **Epic bosses get two passes.** The spawn-time pass (brain) plus
   the death-time `HandleEpicBossLoot` pass. No deduplication. If
   the same item line lives in both, it can drop twice.
4. **Coin tier filtering depends on `source`.** Chests, maps, and
   any call that passes `source = 0` skip the filter, so a
   level-1 treasure map can legitimately mint gold via `Stack
   moedaouro …`. That's intentional but easy to miss.
5. **`source = 0` also disables the coin-pool drain.** The
   pool-economy audit (`EA_POOL_NPC_DRAW`) is NPC-tier-gated, so
   chests and maps don't participate.
6. **True-boss personal loot is enforced by a packet hook, not by
   parenthood.** Every participant's copy actually exists in the
   corpse; visibility is filtered on the way out. If you bypass the
   packet hook (e.g. iterate corpse contents server-side), you'll
   see all copies. Don't write loot-mover code that assumes one
   item per template per corpse.
7. **Lore items** (`LoreItems` group via `Random`) are tracked
   globally in `:unicos:lista`. Once dropped, they're banned from
   the pool. Don't blow that file away "to test" without a backup.
8. **`Loot` property name resolution** for NPCs goes through the
   brain config loader, which strips the `:epicai:` namespace
   before reading `npcdesc.cfg`. Use the bare template name when
   grepping for an NPC's loot index, not the namespaced form.
9. **Magic affixes write to `equip` dict and `magic = 1`.** If you
   roll a magic item but skip `SetMagicTemplateBonus`, you produce
   a base item with no bonuses but the magic gump color — visually
   magic, mechanically vanilla. Don't introduce a path that creates
   the item without applying the template.
10. **Trueboss coin multiplier (5×) lives in
    `Loot_TryDrainCoinTier`**
    ([lootParser.inc:106](../pkg/systems/loot/include/lootParser.inc#L106)),
    not in any cfg. If you're tuning the pool, that's the file.
11. **`Magic` and `MagicSpell` are different keywords.** `Magic` is
    the legacy affix path (rolls templates.cfg bonuses on mundane
    gear). `MagicSpell` is the new spell-stick path
    (`Loot_CreateMagicSpellItem` → `ApplyEpicProperties`). Chest
    base tables use `MagicSpell`; `Magic` is unused by current chest
    loot and survives only for the historical NPC-loot path. Don't
    edit one when you meant the other.
12. **Chest-loot iron-lock only fires via `ChestLoot_PostProcess`,
    not in `Loot_Generate`.** If you ever add a new chest path
    (e.g. a custom GM command that fills containers via
    `Loot_Generate`), remember to call
    `ChestLoot_PostProcess(chest)` after, or your chests will drop
    crafter-grade mundane gear with random materials. The current
    callers are `epicchestspawner.src` (regular + puzzle paths) and
    `treasuredigging.inc`.
13. **Adding items to all chests of a tier → edit `EpicChestN`.**
    Adding items to one class only → edit
    `EpicChestN_FlavorWarrior/Thief/Artifice/Mage`. Do not resurrect
    the legacy `HumanoXChestN` tables — the spawner no longer reads
    them, so edits there are dead.
14. **`MagicSpell` line syntax is positional and short:**
    `MagicSpell <qty_dice> <chance> <tier>`. No name field — the
    base objtype is picked internally from the curated jewelry/
    spellstick pool. Don't try to pass an item name; the parser
    won't read it.
