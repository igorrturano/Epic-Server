# Epic Boss GM Tool (`epicboss.src`)

Reference for maintaining [pkg/systems/epicspawn/commands/gm/epicboss.src](../pkg/systems/epicspawn/commands/gm/epicboss.src) — the GM-facing, multi-page gump editor used to design and spawn "Epic Bosses" and persist them onto a spawner item.

This document describes **only `epicboss.src`** (read line-by-line). Where the file calls out to functions defined in its includes, those are listed under [External Dependencies](#external-dependencies) and are *not* documented here in depth.

---

## 1. Purpose

`epicboss.src` is a GM command script. It opens a 5-page gump form (`960×720`), collects every parameter needed to define a custom boss NPC (template, stats, skills, resistances, abilities, loot, appearance, behavior), and on confirmation:

1. Builds a flat `boss` struct from the form data.
2. Creates (or reuses) an **Epic Boss Spawner** item.
3. Spawns the boss NPC(s) via `CreateEpicBoss`.
4. Persists the full form data + boss struct onto the spawner so it can respawn and be re-edited later.

It is purely a **front-end / data-collection** tool. The actual NPC construction logic lives in `epicBoss.inc` (`CreateEpicBoss`).

---

## 2. Entry Points

The script is never run as a console command. It is launched with `start_script` and **two params**:

```escript
start_script(":epicspawn:commands/gm/epicboss", array{ who, item });
```

| `params[1]` (`who`)   | The GM mobile that opened the editor. |
| `params[2]` (`spawner`) | The spawner item being edited, or the GM again (treated as "new"). |

Known callers:

- [pkg/systems/epicspawn/epicBossSpawner.src](../pkg/systems/epicspawn/epicBossSpawner.src) — the `use` script of the `EpicBossSpawner` item (`itemdesc.cfg`, graphic `0x9F13`).
- [pkg/items/levers/lever.src:942](../pkg/items/levers/lever.src#L942) — a lever can also open the editor.

`program epicBOSS(params)` ([epicboss.src:38](../pkg/systems/epicspawn/commands/gm/epicboss.src#L38)) starts here.

> **Guard:** if `spawner == who` ([line 41](../pkg/systems/epicspawn/commands/gm/epicboss.src#L41)), `spawner` is forced to `0` and the tool runs in *create-new* mode.

---

## 3. Program Flow

```
program epicBOSS(params)
  ├─ editing existing spawner?  →  load form_data from GetObjProperty(spawner,"form_data")
  └─ else                       →  InitializeFormData()
  │
  ShowPage1(...)  →  returns `input` (a gump-result struct)
  │
  while (input[0] != 0)               ← loop until the gump is closed
     ├─ ExtractPageData(input, last_clicked_button)   ← save the page the user was ON
     ├─ input[0]==101 / input[101]    →  Cancel, break
     ├─ input[0]==100 / input[100]    →  CreateBossFromGump(who, form_data, spawner), break
     ├─ UpdateMenuState(input[0], who)  →  decides which page to show next
     └─ ShowPageN(...)                →  re-enters the loop with new `input`
```

Key behaviour: **`ExtractPageData` is run against `last_clicked_button` (the page the user just left), then `UpdateMenuState` advances `last_clicked_button` to the new page.** Order matters — data is harvested before the page switches.

`input[0]` is the ID of the button that closed the gump. `input[<n>]` holds the value/state of widget `n`.

---

## 4. Module-Level State (⚠ important)

Three variables are declared at **module scope** ([lines 34-36](../pkg/systems/epicspawn/commands/gm/epicboss.src#L34-L36)):

| Variable | Purpose |
|---|---|
| `button_states` | `array{1,0,0,0,0}` — which nav tab is highlighted. |
| `last_clicked_button` | The page currently being edited (`BOTAO_PAGE1..5`). |
| `form_data` | **The single source of truth for the whole form.** |

Because each `start_script` call is a fresh script instance, these behave as per-session globals. **`form_data` is a true global** — most functions mutate it directly. Sub-handlers additionally receive it as a `byref` parameter, but it is the same global object either way. There is no second copy until `CreateBossFromGump` builds the `boss` struct.

`InitializeFormData()` ([line 113](../pkg/systems/epicspawn/commands/gm/epicboss.src#L113)) resets `form_data` to defaults. It is **only** called for a brand-new boss, or when an existing spawner has no saved `form_data` property.

---

## 5. `form_data` Schema

Set by `InitializeFormData()`. All scalar fields start as **strings** (gump text entries return strings); `CreateBossFromGump` is responsible for the `CInt`/`CDbl` conversion into the `boss` struct.

| Field | Default | Notes |
|---|---|---|
| `npctemplate` | `"goblin"` | No `:epicAI:` prefix needed. |
| `name` | `""` | |
| `color` | `"0"` | |
| `tier` | `"0"` | Clamped 1–5 on extraction. |
| `description` | `""` | |
| `trueboss` | `0` | Checkbox. |
| `special_ai` | `""` | Free text; appended into `boss.special` as a pseudo-ability. |
| `url` | `""` | Image URL for the preview/profile box. |
| `detailed_description` | `""` | |
| `amount` | `"1"` | Number of bosses to spawn. |
| `min_respawn` / `max_respawn` | `"60"` / `"120"` | Minutes. |
| `spawn_scale` | `"100"` | Visual size %; `100` = template default. |
| `str` `dex` `int` `hits` `dmg` | `"0"` | `dmg` is a multiplier (`0.1`–`1.0`), stored via `CDbl`. |
| `phresist` `firesist` `coresist` `enresist` `poresist` | `"0"` | |
| `skills_mod` | nested `struct{}` | 24 named skill fields, see §7. |
| `teleport_health` | `"25"` | Initialized but **never set by any widget** (see §12). |
| `aggression` | `"High"` | Initialized but **unused**. |
| `flee_health` | `"0"` | Page 4 entry `76`. |
| `target_selection` | `""` | Set by sub-gump (button `77`). |
| `weapon_type` | `""` | One of `Dual Wield` / `One-Handed` / `Two-Handed` / `Ranged`. |
| `specific_weapon` | `""` | Page 5 entry `84`. |
| `equipments` | `array{}` | List of equipment entry structs. |
| `classe` | `""` | `Guerreiro` / `Sabio` / `Ladino`. Initialized **twice** ([line 184](../pkg/systems/epicspawn/commands/gm/epicboss.src#L184) & [200](../pkg/systems/epicspawn/commands/gm/epicboss.src#L200)). |
| `magic_school` | `""` | Set by sub-gump (button `88`). |
| `drop_chance` | `"25"` | |
| `special` | `array{}` | Weapon special abilities (hit scripts). |
| `loot` | `array{}` | Loot item structs. |
| `deathevents` | `array{}` | |
| `combatevents` | `array{}` | |
| `patrol_loc1/2`, `teleport_loc` | `struct{}` | Legacy; the live data uses `patrol_points` / `teleport_locations` arrays created lazily. |
| `race` | `HUMANO` | |
| `gender` | `0` | `0` = male, `1` = female. |
| `graphic` | `400` | Body graphic, derived from race+gender via `GetRaceGraphic`. |

Fields created **lazily** (not in `InitializeFormData`): `skills_mod`, `armorspecial`, `timedscripts`, `natural_traits`, `patrol_points`, `teleport_locations`, `custom_abilities`, `appearance`, `mount`, `mount_name`, `color_ranges`, `target_priority`, `preferred_target`.

---

## 6. The Five Pages

`UpdateMenuState` maps the clicked nav button to a page and updates the highlighted tab. `AddNavigationButtons` draws the 5-tab bar (`BASIC INFO / ATTRIBUTES / ABILITIES / BEHAVIOR / EQUIPMENT`) on every page.

Every page also draws the same footer pair: **`CREATE BOSS` = button `100`**, **`CANCEL` = button `101`**.

| Page | `ShowPageN` | Theme |
|---|---|---|
| 1 | [`ShowPage1`](../pkg/systems/epicspawn/commands/gm/epicboss.src#L584) | Basic info, respawn timing, NPC profile/preview. |
| 2 | [`ShowPage2`](../pkg/systems/epicspawn/commands/gm/epicboss.src#L705) | Stats, resistances, 24 skill modifiers. |
| 3 | [`ShowPage3`](../pkg/systems/epicspawn/commands/gm/epicboss.src#L928) | Weapon/armor special abilities, combat events, natural traits, death events. |
| 4 | [`ShowPage4`](../pkg/systems/epicspawn/commands/gm/epicboss.src#L1561) | Patrol points, teleport locations, AI targeting, custom behavior. |
| 5 | [`ShowPage5`](../pkg/systems/epicspawn/commands/gm/epicboss.src#L1704) | Weapon type, loot, class, magic school, appearance/race/equipment/mount/colors. |

### Page interaction model

- **Pages 1 & 2** are pure data forms. They have no action buttons — when their gump closes, the result goes straight back to the main loop and `ExtractPageData` reads the fields.
- **Pages 3, 4 & 5** contain action buttons that open sub-gumps. `ShowPage3/4/5` handle those buttons **inline**, mutate `form_data`, then **recursively call themselves** to redraw. They only return to the main loop when the closing button is navigation / create / cancel.

---

## 7. Field & Button ID Map

⚠ **IDs are reused across pages.** This is only safe because exactly one page is on screen at a time and each `ShowPageN` interprets `input[0]` in its own scope. When adding a widget, stay within the page's range and avoid the reserved IDs (`1`–`5`, `100`, `101`).

### Reserved (all pages)
| ID | Meaning |
|---|---|
| `1`–`5` | Navigation tabs (`BOTAO_PAGE1`–`BOTAO_PAGE5`). |
| `100` | CREATE BOSS. |
| `101` | CANCEL. |

### Page 1 — text-entry field IDs
| ID | Field |
|---|---|
| `1` | `npctemplate` |
| `2` | `name` |
| `3` | `color` |
| `9` | `tier` |
| `4` | `description` |
| `45` | `spawn_scale` |
| `5` | `trueboss` (checkbox) |
| `6` | `special_ai` |
| `44` | `amount` |
| `42` / `43` | `min_respawn` / `max_respawn` |
| `7` | `url` |
| `8` | `detailed_description` |

### Page 2 — field IDs
Stats: `10` str, `11` dex, `12` int, `13` hits, `14` dmg.
Resists: `15` phys, `16` fire, `17` cold, `18` energy, `19` poison.
Skills (`skills_mod.*`):

| ID | Skill | ID | Skill |
|---|---|---|---|
| `20` | wrestling | `33` | survival |
| `21` | tactics | `34` | metal_working |
| `22` | magery | `35` | wood_working |
| `23` | awareness | `36` | domestic_work |
| `24` | magic_lore ⚠ *displayed, never extracted* | `37` | gathering |
| `25` | parry | `38` | handle_animal |
| `26` | one_handed | `39` | dual_wield |
| `27` | two_handed | `41` | alchemy |
| `28` | ranged | `42` | theology |
| `29` | sneak | `43` | science |
| `30` | pickpocketing | `44` | arcana |
| `31` | mechanics | | |
| `32` | medicine | | |

### Page 3 — button IDs
| ID | Action |
|---|---|
| `10` | Add Weapon Special Ability → `HandleAddWeaponSpecialAbility` |
| `11` | Add Armor Special Ability → `HandleAddArmorSpecialAbility` |
| `20` | Add Natural Traits → `HandleNaturalTraits` |
| `30` | Add Death Event → `HandleAddDeathEvent` (external) |
| `40` | Add Combat Event → `HandleAddCombatEvent` (external) |
| `200`–`299` | Edit combat event (index = ID − 200) → `ShowCombatEventsList` on the existing event, then re-mirror `actions`→`steps` |
| `300`–`399` | Delete combat event (index = ID − 300) |
| `55`–`64` | Delete death event (index = ID − 54) |
| `110`–`119` | Delete immunity (index = ID − 109) |
| `120`–`129` | Delete resistance (index = ID − 119) |
| `130`–`139` | Delete vulnerability (index = ID − 129) |
| `140`–`149` | Delete weapon special ability (index = ID − 139) |
| `150`–`159` | Delete armor special ability (index = ID − 149) |

Most "delete" ranges support up to 10 entries; the combat-event edit/delete
ranges (`200`–`299` / `300`–`399`) support up to ~99.

### Page 4 — button / field IDs
| ID | Action |
|---|---|
| `70` | Add Patrol Point → `SelectPatrolArea` |
| `71` | Clear All Patrol Points |
| `73` | Set Teleport Location → `SelectTeleportLocations` |
| `74` | Add Custom Behavior → `HandleAddCustomAbility` (external) |
| `75` | Target Priority → `ShowTargetPriorityMenu` (external) |
| `76` | `flee_health` (text entry) |
| `77` | Alvo favorito → `ShowTargetSelectionMenu` (external) |
| `78` | Clear Teleport Locations |

### Page 5 — button / field IDs
| ID | Action |
|---|---|
| `80`/`81`/`82`/`83` | Weapon type radio: Dual Wield / One-Handed / Two-Handed / Ranged |
| `84` | `specific_weapon` (text entry) |
| `85`/`86`/`87` | Class: Guerreiro / Sabio / Ladino → `ShowClassAbilities` (external) |
| `88` | Magic School → `ShowMagicSchoolSelection` (external) |
| `90` | Add Loot Item → `HandleAddLootItem` (external) |
| `92`+ | Delete loot item (index = ID − 91) |
| `92` | Edit Appearance → `HandleEditAppearance` |
| `93` | Race & Gender → `HandleAddRace` |
| `94` | Equipment → `HandleAddEquipments` |
| `95` | Mount → `HandleAddMount` |
| `96` | Colors → `HandleAddColors` |

⚠ **ID collision on Page 5:** the loot-delete range (`91+index`) overlaps the appearance buttons `92`–`96`. The delete handler is checked first (`input[0] >= 91 && input[0] <= 91 + loot.size()`), so if a boss has loot items, clicking *Edit Appearance/Race/Equipment/etc.* can be swallowed as a loot deletion. **Treat this as a known footgun when editing Page 5.**

---

## 8. Sub-Gump Handlers (defined in this file)

| Function | Opened by | Effect |
|---|---|---|
| `HandleAddWeaponSpecialAbility` / `DisplayWeaponAbilityOptions` | Page 3 btn `10` | Appends `{name, description, script_path}` to `form_data.special`. Hit scripts under `:combat:weaponHitScripts/`. |
| `HandleAddArmorSpecialAbility` / `DisplayArmorAbilityOptions` | Page 3 btn `11` | Appends to `form_data.armorspecial`. Scripts under `:combat:armorHitScripts/`. |
| `HandleNaturalTraits` / `SelectTraitType` / `TraitExistsInOtherTypes` | Page 3 btn `20` | Maintains `natural_traits.{immunities,resistances,vulnerabilities}`. A trait cannot exist in two categories at once. |
| `SelectPatrolArea` | Page 4 btn `70` | Targets a tile, appends `{x,y,z,realm}` to `patrol_points`. |
| `SelectTeleportLocations` | Page 4 btn `73` | Targets a tile + HP% threshold, appends to `teleport_locations`, sorted high→low HP. |
| `HandleEditAppearance` + `EditHairStyle` / `EditHairColor` / `EditBeardStyle` / `EditBeardColor` / `EditSkinColor` | Page 5 btn `92` | Builds `form_data.appearance`. Reads cfg files (see §10). Beard editing is blocked for female / Drow / Elf. |
| `HandleAddRace` / `GetRaceGraphic` | Page 5 btn `93` | Sets `race`, `gender`, `graphic`. |
| `HandleAddEquipments` + `AddSingleEquipment` / `AddEquipmentFromContainer` / `CreateEquipmentEntry` | Page 5 btn `94` | Builds `form_data.equipments`. `CreateEquipmentEntry` snapshots `objtype`, `color`, `name` **and all ObjProperties** of the targeted item. |
| `HandleAddMount` + `TargetExistingMount` / `SelectMountFromList` | Page 5 btn `95` | Sets `mount` (graphic) + `mount_name`. `SelectMountFromList` reads `npcdesc` and filters `Environment == "Mount"`. |
| `HandleAddColors` / `AddColorRange` | Page 5 btn `96` | Builds `form_data.color_ranges` from basic colors + cultural-theme constants. |

### Sub-gump conventions

- Sub-gumps consistently use **`999` = Cancel** and **`998`/`999` = Done/Cancel** as their closing button IDs.
- Weapon/armor ability selectors use `999` specifically because a plain `0` close is ambiguous with "no selection".
- Paginated sub-gumps (`EditHairStyle`, `EditBeardStyle`, `SelectMountFromList`) recurse with `page ± 1` on Prev/Next buttons.

### Dead / legacy code in this file

`SelectDamageType` ([line 3190](../pkg/systems/epicspawn/commands/gm/epicboss.src#L3190)) and `AddSpecialNature` ([line 3241](../pkg/systems/epicspawn/commands/gm/epicboss.src#L3241)) append to `form_data.natural_traits` as a **flat array** of `{trait, damage}` structs — incompatible with the `{immunities,resistances,vulnerabilities}` struct shape that `HandleNaturalTraits` and Page 3 actually use. Neither function is called anywhere in `epicboss.src`. Treat them as legacy; do not wire them in without reconciling the data shape.

---

## 9. `CreateBossFromGump` — Form → Boss → Spawner

[`CreateBossFromGump(who, form_data, existing_spawner)`](../pkg/systems/epicspawn/commands/gm/epicboss.src#L2492) runs when CREATE BOSS (`100`) is clicked.

1. **Build `boss` struct.** Flattens `form_data`: numeric strings → `CInt`/`CDbl`, copies skill mods (only non-zero/present ones into `boss.skills_mod`), copies special abilities, armor specials, combat events, loot, traits, etc.
   - Combat events are passed straight through; `CreateEpicBoss` normalizes each via `NormalizeCombatEvent` (in `epicBoss.inc`) into a single canonical `steps` array. The legacy dual `actions`+`steps` duplication was removed.
   - `special_ai` text becomes a pseudo-entry `{name:"Special AI", ai:<text>}` appended to `boss.special`.
2. **Spawner resolution:**
   - *Editing* (`existing_spawner` set): reuse it. If a live boss is linked (`ChildSerial`), it is force-killed first — `allowdeath`, `Disable("invul")`, `GuardKill`, then `ApplyRawDamage` for full HP, plus a `SleepMS(500)`.
   - *New*: create an item of objtype **`0xA402`** at the GM's feet via `CreateItemAtLocation`, then `MoveObjectToLocation` with `MOVEOBJECT_FORCELOCATION`.
3. **Spawn loop:** calls `CreateEpicBoss(boss, x, y, z, realm)` `amount` times (`sleepms(100)` between).
4. **Persist to spawner** (if it is an item):
   - `BossStruct` ← the `boss` struct
   - `form_data` ← the **full** form data (so the editor can reload it)
   - `MinDelay` / `MaxDelay` ← respawn minutes
   - `Range` ← `5`, `SpawnerState` ← `ACTIVE`, `SpawnQueue` ← `"Active"`
   - Links children: `ChildSerial` (single boss) or `ChildSerials` (array, multiple bosses); each boss gets `epicspawner` ← `spawner.serial`.
   - `spawner.invisible := 1`.

> Note the objtype mismatch: `itemdesc.cfg` defines `EpicBossSpawner` as graphic `0x9F13`, but `CreateBossFromGump` creates new spawners as objtype **`0xA402`**. If you change one, check the other.

---

## 10. Config Files Read

| Path | Used by |
|---|---|
| `:fls_core:config/hairtypes` | `EditHairStyle`, `EditBeardStyle` (sections `todos`, `beard`, `dwarwen_beard`). |
| `:hair:config/hairColors` | `EditHairColor`, `EditBeardColor` (race-specific sections `Cabelo Imperial/Anao/Elfo/Drow/Orc`). |
| `:fls_core:config/skincolors` | `EditSkinColor` (sections `list`, `elfo`, `drow`, `orc`). |
| `:*:npcdesc` | `SelectMountFromList` (entries with `Environment == "Mount"`). |

---

## 11. External Dependencies

`epicboss.src` includes and relies on functions defined elsewhere — **do not assume their behaviour from this file**:

| Include | Provides (used here) |
|---|---|
| `:epicspawn:epicBoss` → [epicBoss.inc](../pkg/systems/epicspawn/include/epicBoss.inc) | `CreateEpicBoss`, `GetWeaponHitScripts`, `GetArmorHitScripts`, `ShowDualWieldWeapons` / `OneHandedWeapons` / `TwoHandedWeapons` / `RangedWeapons`, `ShowClassAbilities`, `ShowMagicSchoolSelection`, `HandleAddLootItem`, `HandleAddCustomAbility`, `ShowTargetSelectionMenu`, `ShowTargetPriorityMenu`. |
| `:epicspawn:epicCombatEvents` → [epicCombatEvents.inc](../pkg/systems/epicspawn/include/epicCombatEvents.inc) | `HandleAddCombatEvent`. |
| `:epicspawn:epicDeathEvents` → [epicDeathEvents.inc](../pkg/systems/epicspawn/include/epicDeathEvents.inc) | `HandleAddDeathEvent`. |
| `:combat:Model/Damage` | `DAMAGE_TYPE::*` enum (used in trait/damage selectors). |
| `:gumps:include/epicGumps` | The `GF*` gump-framework helpers (`GFECreateGump`, `GFTextEntry`, `GFAddButton`, `GFSendGump`, `GFExtractData`, …). |
| `include/client`, `include/say`, `include/creationUtils` | `SendSysMessage`, `Target`/`TargetCoordinates`, item-creation helpers, `GetHairGumpGraphic` / `GetBeardGumpGraphic`, race constants (`HUMANO`, `ELFO`, …), color constants (`AIGLANA`, `HAROLOTH`, …). |

---

## 12. Known Issues & Maintainer Gotchas

These were found by reading the source — fix carefully, several are load-bearing for in-world bosses already created.

1. **`magic_lore` skill is never saved.** Page 2 draws a Magic Lore entry (field ID `24`), but `ExtractPageData`'s `BOTAO_PAGE2` branch has no extraction for ID `24`. The value is displayed/echoed but lost on page switch. `CreateBossFromGump` *does* copy `magic_lore` if present — it just never is.
2. **`throwing` skill has no widget.** Initialized in `form_data.skills_mod` and copied by `CreateBossFromGump`, but no gump field exists for it.
3. **Page 5 ID collision** between loot-delete buttons (`91+index`) and appearance buttons `92`–`96` — see §7. With loot configured, the appearance/race/etc. buttons can be misread as loot deletions.
4. **`teleport_health` is never set by UI.** Defaults to `"25"`, copied into `boss.teleport_health`, but Page 4 only has a *label* for it — the actual teleport behaviour is driven by `teleport_locations` (button `73`).
5. **`aggression` field is dead** — initialized to `"High"`, never read.
6. **Tier condition bug** at [line 2511](../pkg/systems/epicspawn/commands/gm/epicboss.src#L2511): `(form_data.tier != "0" || form_data.tier != "")` is always true for any truthy value (a string cannot equal both). The `else` branch (`boss.tier := "0"`, a *string* — inconsistent with the `CInt` path) is effectively unreachable when tier is set.
7. **`classee` typo** at [line 2735](../pkg/systems/epicspawn/commands/gm/epicboss.src#L2735): `if (form_data.classe && form_data.classee != "")` — `classee` has an extra `e` and does not exist. The block is redundant anyway (class is also assigned at [line 2755](../pkg/systems/epicspawn/commands/gm/epicboss.src#L2755)).
8. **`classe` initialized twice** in `InitializeFormData` (lines [184](../pkg/systems/epicspawn/commands/gm/epicboss.src#L184) and [200](../pkg/systems/epicspawn/commands/gm/epicboss.src#L200)).
9. **Duplicate `DAMAGE_TYPE::BLUNT`** in the trait/damage option arrays of `SelectTraitType` ([line 3040](../pkg/systems/epicspawn/commands/gm/epicboss.src#L3040)) and `SelectDamageType` ([line 3200](../pkg/systems/epicspawn/commands/gm/epicboss.src#L3200)) — BLUNT appears at positions 1 and 4 instead of one being a distinct type.
10. **Leftover debug `print`** at [line 372](../pkg/systems/epicspawn/commands/gm/epicboss.src#L372) (`"FORM DATA ARCANA:"`). The tool is also chatty with `SendSysMessage` on every page switch and every trait/event removal.
11. **Spawner objtype mismatch** — `itemdesc.cfg` says `0x9F13`, `CreateBossFromGump` creates `0xA402` (see §9).
12. ~~**Combat events stored twice**~~ — *Resolved.* Persisted combat events now carry a single canonical `steps` array, produced by `NormalizeCombatEvent` (`epicBoss.inc`), the one place that defines the persisted shape. The runtime (`combatEventsControl.src`) reads `steps`.
13. **Class-instance / wrapper warning** — this script passes plain structs and ObjRefs through `start_script`, which is correct. Keep it that way: do **not** pass wrapper class instances across the `start_script` boundary (they lose method dispatch).

---

## 13. Quick Maintenance Checklist

When adding a new field:

1. Add it to `InitializeFormData()` with a sensible default (remember: defaults are strings).
2. Draw the widget in the relevant `ShowPageN`, picking an **unused ID in that page's range**.
3. Add extraction in the matching `case` branch of `ExtractPageData` (pages 1/2) **or** handle the button inline in `ShowPage3/4/5`.
4. Copy it into the `boss` struct in `CreateBossFromGump`, converting type as needed.
5. If the new data must survive editing, it is already covered — `CreateBossFromGump` persists the *entire* `form_data` onto the spawner.
6. Confirm `CreateEpicBoss` (in `epicBoss.inc`) actually consumes the new `boss` field — `epicboss.src` only *collects* data; it does not apply it to the NPC.
