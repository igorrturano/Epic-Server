# Equipment / Unequipment Pipeline

Reference: how an item ends up on a mobile's paperdoll, what runs while it
travels there, and what is undone on the way back off. Maps every program,
hook and modifier path actually involved — based on a line-by-line read of
the current code, not on assumptions.

> As of 2026-05-24, the `equip` ObjProperty modifier system has been
> consolidated into a single OOP class (`EquipModStamper`) invoked from
> POL's global `EquipTest` / `UnequipTest` hooks. The per-itemdesc combat
> gate still exists but is restriction-only. The previously-broken jewelry
> path (silent no-op on magic rings, necks, etc.) is now functional. The
> wrapper-driven `AttributeModifier` system for armor/weapons remains a
> second parallel path — see §9.

---

## 1. Who fires what — the POL engine perspective

POL's core fires up to two eScript programs when an item changes layer on
a mobile:

| Direction | Engine call | Program signature                       |
|-----------|-------------|------------------------------------------|
| Equip     | `EquipScript` from the item's `itemdesc.cfg`   | `program <name>(mobile, item)` returning `1` to allow, `0` to block |
| Unequip   | `UnEquipScript` from the item's `itemdesc.cfg` | `program <name>(mobile, item)`                                       |

`EquipScript` runs **before** the item is attached to the layer — it is a
permission gate. `UnEquipScript` runs while the item is **still** on the
layer; the engine moves it after the script returns.

If the itemdesc has no `EquipScript` / `UnEquipScript`, POL silently moves
the item to the layer with no per-itemdesc eScript callback. **However**,
POL also fires two GLOBAL programs on every equip/unequip regardless of
itemdesc — these are POL engine hooks hard-coded by file path:

| Direction | Engine call | Program                                                                                        |
|-----------|-------------|------------------------------------------------------------------------------------------------|
| Equip     | global `EquipTest`   | [scripts/misc/equiptest.src](../scripts/misc/equiptest.src) — frozen/broken gate, mask flag, `#EquippedOn` stamp, and `EquipModStamper(item).ApplyOn(mobile)` |
| Unequip   | global `UnequipTest` | [scripts/misc/unequiptest.src](../scripts/misc/unequiptest.src) — frozen/cursed gate, hidden reveal, `EquipModStamper(item).RemoveFrom(mobile)` |

These run for **every** equippable item including jewelry and clothing.
This is the single entry point for the `equip` ObjProperty modifier
system. The file names cannot be renamed — POL core hard-codes the
paths `scripts/misc/equiptest.ecl` and `scripts/misc/unequiptest.ecl`.

### Where each script is registered today

| Bucket | itemdesc(s) | EquipScript / UnEquipScript |
|---|---|---|
| Combat-class items (every weapon, armor, shield) | [pkg/systems/combat/itemdesc.cfg](../pkg/systems/combat/itemdesc.cfg) — 930+ entries | `equipscript equip` / `unequipscript unequip` → resolves to [`:combat:equip`](../pkg/systems/combat/equip.src) / [`:combat:unequip`](../pkg/systems/combat/unequip.src) |
| Crafting bench gear, metalworking gear, etc. | [pkg/skills/metalworking/itemdesc.cfg](../pkg/skills/metalworking/itemdesc.cfg), [pkg/skills/DomesticWork/itemdesc.cfg](../pkg/skills/DomesticWork/itemdesc.cfg), [pkg/systems/crafting/config/itemdesc.cfg](../pkg/systems/crafting/config/itemdesc.cfg) | same `equip` / `unequip` pair |
| Custom-pizza joke items | [pkg/items/custompizza/itemdesc.cfg](../pkg/items/custompizza/itemdesc.cfg) | `:combat:equip` / `:combat:unequip` (fully qualified) |
| Mounts | [pkg/items/mounts/itemdesc.cfg](../pkg/items/mounts/itemdesc.cfg) | [`mount/equip`](../pkg/items/mounts/mount/equip.src) / [`mount/unequip`](../pkg/items/mounts/mount/unequip.src) — currently no-ops returning `1` |
| Orc mask + the FLS skin set | [pkg/fls/fls_items/orcsmask/itemdesc.cfg](../pkg/fls/fls_items/orcsmask/itemdesc.cfg) | `equiporcmask` / `unequiporcmask` (+ a generic `unequip` reused across skin items) |
| Everything else — rings, bracelets, necklaces, earrings, robes, capes, shirts, pants, shoes, sashes, hair, mundane clothing | [config/itemdesc.cfg](../config/itemdesc.cfg) and most other itemdescs | **no equip/unequip hook** — silent layer attach |

This split still motivates restriction enforcement (Steam Armor only on
Dwarves, FURIA bars armor, STR checks, etc.) — combat-class items get the
gate, jewelry/clothing do not. But for modifier application, both paths
now flow through the global `EquipTest` / `UnequipTest` hooks. A magic
ring with `strmod` set on its `equip` ObjProperty now correctly applies
on equip and reverses on unequip.

---

## 2. The combat path — high-level flow

For everything in `:combat:itemdesc.cfg` (every weapon, every armor piece,
every shield):

```
POL engine
   │
   │  player drags item onto paperdoll layer
   ▼
[global EquipTest]  scripts/misc/equiptest.src
   │   frozen/broken/incognito gates
   │   stamps #EquippedOn
   │   EquipModStamper(item).ApplyOn(mobile)  ← single source of truth for equip-dict mods
   │   returns 0 → cancel; returns 1 → POL attaches item, continues
   ▼
[:combat:equip]  pkg/systems/combat/equip.src
       permission gate — race/class/state/STR/ownership/curse/entropy checks
       returns 0 → POL detaches; returns 1 → done
       │
       └─► Run_Script_To_Completion(":equipsys:equipitem", {who, item})
               └── pkg/systems/equipsys/equipitem.src
                     builds ArmorWrapper / WeaponWrapper
                     applies wrapper-side AttributeModifiers (skill mods,
                       armor weight DEX penalty, …)
                     fires ability passives
                     queues async UpdateEquipment packet
```

Unequip is the mirror image, gated through:
- global `UnequipTest` ([scripts/misc/unequiptest.src](../scripts/misc/unequiptest.src))
  → cursed-block strict-first; if not cursed, `EquipModStamper(item).RemoveFrom(mobile)`
- per-itemdesc [`:combat:unequip`](../pkg/systems/combat/unequip.src) →
  cursed-block strict-first; `Run_Script_To_Completion(":equipsys:unequipitem", {who, item})`
  to reverse wrapper-side modifiers

`Run_Script_To_Completion` is synchronous. `EquipModStamper.ApplyOn` is
also synchronous. Both modifier passes complete before POL's next event.

---

## 3. The combat gate — [`:combat:equip`](../pkg/systems/combat/equip.src)

This program is the only place that decides whether equipping a combat-
class item is allowed at all. Anything that returns `0` cancels the equip.

Checks performed, in order ([equip.src:17-292](../pkg/systems/combat/equip.src#L17-L292)):

| # | Check | Lines |
|---|---|---|
| 1 | `descmod` propagation — adds the item's `descmod` to the mobile's `descmod` array, then re-`SetName`s to force a paperdoll refresh | 19-51 |
| 2 | `restrito_mob` ObjProperty — item is NPC-only | 54-56 |
| 3 | Already-holding-a-weapon — block second weapon on either hand | 59-65 |
| 4 | Shield + two-handed melee in hand — block | 67-77 |
| 5 | **NPC short-circuit:** `if (who.npctemplate) return 1` — every check below is player-only | 78-80 |
| 6 | `desarmar` state — disarmed by ability/spell | 82-85 |
| 7 | `metamorfose` state — transformed, except `ELO_NATURAL` for non-weapon/armor | 87-97 |
| 8 | `pilotando` state — piloting a vehicle | 99-102 |
| 9 | Steam Gadgets 0xFFC6/7/8 — must be installed via Tactical Pauldron (`#steam_install` flag) | 105-112 |
| 10 | Steam Armor 0xFFC1-9 — Dwarves only | 114-122 |
| 11 | Steam Gloves 0xFFC1 — blocks gloves/weapons/shields while worn; also blocks itself if hands/gloves occupied | 124-144 |
| 12 | Barbarian `FURIA` ability — refuses anything tagged `TipoProtecao "Armadura"` | 146-153 |
| 13 | Shaman — bone armor whitelist only | 155-166 |
| 14 | **STR requirement** — `wrapper.GetStrRequired()` vs `AP_GetStat(who, STRENGTH)` | 169-177 |
| 15 | Blessed-weapon color restoration when bless expired | 179-182 |
| 16 | Elemental weapon expiration — restore original color when `#elementalweapon.endtime` past | 184-190 |
| 17 | `armaespiritual` — only the bound swordmage can equip | 192-196 |
| 18 | Swordmage process spawn for `swordmage` items | 199-206 |
| 19 | Epic item entropy threshold — `sacred` requires `entropy ≥ epicEntropy`, `cursed` requires `entropy ≤ epicEntropy` | 209-222 |
| 20 | Plain `cursed` flag — `-100` entropy penalty applied | 224-226 |
| 21 | Legacy `#armor_evasion_penalty` cleanup (Phase 5 residue) | 228-233 |
| 22 | **Orchestrator dispatch** — `Run_Script_To_Completion(":equipsys:equipitem", {who, item})` | 235 |
| 23 | **Legacy mods** — `ApplyEquipObjMods(who, item)` | 236 |
| 24 | Two-handed-parry-bonus flag cleanup | 260-262 |
| 25 | Carry-weight recalc (Steam Legs 0xFFC2, Mochila 0xEDA0) — `MobileWrapper(who).RecalcCarryWeightMod(0, item)` (item passed via `includeItem` because it isn't on its layer yet) | 268-270 |
| 26 | Steam Armor dex penalty — stored on `#steam_dex_penalty`, applied via `AP_ModifyStatMod` | 274-290 |

A FURIA-tagged armor going through the gate trips check 12 and never
reaches the orchestrator at line 235. Conversely a plain magic ring never
enters this file at all, because rings don't carry an `equipscript`.

---

## 4. The orchestrator — [`:equipsys:equipitem`](../pkg/systems/equipsys/equipitem.src)

Lives in [pkg/systems/equipsys/equipitem.src](../pkg/systems/equipsys/equipitem.src),
runs under `Run_Script_To_Completion` so it blocks the combat gate.

```escript
program equipitem(params)
    var who  := MobileWrapper(params[1]);
    var item := params[2];

    var equip;
    if (item.IsA(POLCLASS_ARMOR))
        equip := ArmorWrapper(item);   debugArmor(who.GetObjRef(), equip);
    elseif (item.IsA(POLCLASS_WEAPON))
        equip := WeaponWrapper(item);  updateCombatTraits(who, equip);
        debugWeapon(who.GetObjRef(), equip);
    endif

    equip.Update();                              // push wrapper cache → item props

    // class/race overrides applied to the wrapper IN MEMORY (before passives run)
    if (who.isShaman())
        equip.SetModifier(AttributeModifier(MAGERY, 0));
    endif
    if (item.IsA(POLCLASS_ARMOR) && who.HasAbility(ABILITY_ID::CONEXAO_DIVINA))
        var tp := equip.GetTipoProtecao();
        if (tp["Armadura Leve"] || tp["Armadura Media"])
            equip.SetModifier(AttributeModifier(MAGERY, 0));
        endif
    endif
    if (item.IsA(POLCLASS_ARMOR) && equip.IsShield())
        var chardata := GetObjProperty(who.GetObjRef(), "chardata");
        var isScutum := {0x1B76, 0x1B77, 0xFA62};
        if (chardata.trait == "Força da Legião" && (item.objtype in isScutum))
            // halve the negative dex mod scuta normally carry
            var dexMod := equip.GetModifier(DEXTERITY).GetValue();
            equip.SetModifier(AttributeModifier(DEXTERITY, CInt(dexMod / 2)));
        endif
    endif

    who.TriggerEquipPassives(equip);     // fires ability OnPassiveOnEquip hooks
    who.ApplyItemModifiers(equip);       // walks equip.GetModifiers(), ApplyOn(who)

    // Magic-focus mana bonus for staves/wands/spellbooks
    if (item.IsA(POLCLASS_WEAPON))
        var cat := equip.GetCategory();
        if (cat == WEAPON_CATEGORY::STAFF ||
            cat == WEAPON_CATEGORY::WAND  ||
            cat == WEAPON_CATEGORY::SPELLBOOK)
            ManaModFabricacao(item).ApplyOn(who.GetObjRef());
        endif
    endif

    Start_Script("packets/AssyncUpdateEquipment",
                 array{who, true, item, item.layer});
    return 1;
endprogram
```

Important details that aren't obvious from the surface:

- **Ordering of the modifier-mutation phase matters.** Shaman / Conexão
  Divina / Scutum write directly into the wrapper's cached modifiers
  *before* `TriggerEquipPassives` runs. Passives can then read those
  values via `equip.GetModifier(...)` and ability bonuses are added on
  top by `equip.SetupOnEquip()` (called at the end of
  `TriggerEquipPassives`). Only **after** all of that does
  `ApplyItemModifiers` materialise the modifiers onto the mobile.
- **`equip.Update()`** writes the wrapper's cached `itemInfo`
  dictionary back to the ObjRef. Without it, mutations made in the
  constructor (resists by slot, fabrication mods, etc.) would never
  persist.
- **Async packet update**: the UO equipment packet (used by the client
  for AOS tooltips / paperdoll refresh) is fired in a separate script
  (`packets/AssyncUpdateEquipment`) so the orchestrator can return
  promptly.

### `TriggerEquipPassives` and the two-phase bonus model
[MobileWrapper.inc:726-734](../pkg/systems/wrappers/include/Objects/Model/MobileWrapper.inc#L726-L734)

```escript
function TriggerEquipPassives(this, byRef equip)
    var abilities := this.GetAbilities();
    if (abilities)
        foreach level in (abilities.keys())
            run_script_to_completion(":abilities:AbilityController",
                array{"passive", CInt(abilities[level]),
                      equip, ABILITY_ACTIVATION::ON_EQUIP});
        endforeach
    endif
    equip.SetupOnEquip();
endfunction
```

Each ability the mobile owns gets dispatched through
`:abilities:AbilityController`, which ultimately calls the ability's
`OnPassiveOnEquip(attackInfo)`. The crucial constraint is that the
ability runs **in a separate script** (`run_script_to_completion`).
That means any modifier mutation done inside `OnPassiveOnEquip`
operating on a wrapper object would not survive the script boundary —
because [eScript class instances lose method dispatch across
`Start_Script`](../pkg/systems/wrappers/include/Objects/Model/EquipWrapper.inc#L152-L167).
The workaround is the **`ability_bonuses` property** on the item:

- Passive hook calls `equip.ApplyAbilityBonus(attribute, value)` which
  **writes a dictionary** (`{attribute: value}`) to a dedicated
  ObjProperty on the item ([EquipWrapper.inc:159-167](../pkg/systems/wrappers/include/Objects/Model/EquipWrapper.inc#L159-L167)).
  Method dispatch isn't needed — it's a plain property write.
- After all passives run, `equip.SetupOnEquip()` reads that dict and
  adds each entry on top of the current modifier value
  ([EquipWrapper.inc:173-184](../pkg/systems/wrappers/include/Objects/Model/EquipWrapper.inc#L173-L184)).
- On unequip, `equip.CleanupOnUnequip()` mirrors the values back into
  the modifier (so the subsequent `RemoveItemModifiers` walks the same
  totals that `ApplyItemModifiers` applied) and erases the property.

This is why ability bonuses survive cleanly without double-counting,
and why a wrapper-driven passive system can coexist with the
out-of-process ability scripts.

### `ApplyItemModifiers`
[MobileWrapper.inc:1058-1073](../pkg/systems/wrappers/include/Objects/Model/MobileWrapper.inc#L1058-L1073)

```escript
function ApplyItemModifiers(this, byRef itemWrapped)
    ...
    if (!(itemWrapped is @ArmorWrapper) && !(itemWrapped is @WeaponWrapper))
        return InvalidFormatException(...);
    endif

    foreach mod in (itemWrapped.GetModifiers())
        mod.ApplyOn(this.GetObjRef());
        SleepMS(2);
    endforeach
endfunction
```

The wrapper exposes the modifier list as a dictionary of
`AttributeModifier` objects (DEX/SNEAK/MECHANICS/MAGERY/AWARENESS for
armor; weapon-specific ones for weapons). Each `AttributeModifier` knows
how to mutate the mobile (stat mods, skill mods, etc.). This is the
*intended* single source of truth — but only for armor and weapons.
`EquipWrapper.GetModifiers()` is itself not driven on the base class for
plain jewelry/clothing.

---

## 5. The `equip` ObjProperty modifier system — owned by EquipModStamper

The `equip` ObjProperty dict on an item is the historical home of stat /
vital / resist / skill mods for magic and crafted gear. It still exists,
but apply/remove is now owned by a single class with per-item delta
stamping. The legacy free functions `ApplyEquipObjMods` and
`RemoveEquipObjMods` in `pkg/tn/include/equipment.inc` have been deleted.

**Module**: [pkg/systems/equipsys/include/Model/EquipModStamper.inc](../pkg/systems/equipsys/include/Model/EquipModStamper.inc)

**Class**: `EquipModStamper(item)` — constructed from any equippable item.

**API**:

| Method | Effect |
|---|---|
| `ApplyOn(mobile)` | Reads item's `equip` dict, applies each entry to `mobile`, stamps the exact delta on `#equipMods_<serial>`. Idempotent (no-op while stamp exists). Skips entirely if item is `magic == 1` (unidentified) — no stamp written, so RemoveFrom symmetrically no-ops. |
| `RemoveFrom(mobile)` | Reads `#equipMods_<serial>` from mobile, subtracts the exact stamped delta, erases the stamp. No-op if no stamp (item was never applied — guarantees symmetry with the apply-skip on unidentified). |

**Per-key handling** (`equip` dict keys → mod channels):

| Key | Channel |
|---|---|
| `strmod` / `dexmod` / `intmod` | `AP_ModifyStatMod` |
| `manamod` / `hitsmod` / `stammod` | `#manamod` / `#hitsmod` / `#stammod` accumulators + `RecalcVitals` |
| `ColdResistMod` / `FireResistMod` / `PhysicalResistMod` / `EnergyResistMod` / `PoisonResistMod` | trigger `RecalcVitals` (resist value lives elsewhere — wrapper/material/rune) |
| `setprop` | writes `#{propname}` = `propvalue` from sibling ObjProperties on the item |
| `sacred` / `cursed` | category flags only; never stamped, never mutate the mobile here |
| any other key | treated as a skill identifier, applied via `AP_ModifySkillMod` |

### Identification gate (symmetric)

- No `magic` property → crafted gear, mods always apply.
- `magic == 1` → unidentified, ApplyOn skips; **nothing is stamped**, so
  RemoveFrom on unequip is also a no-op. Symmetric.
- `magic == 2` → identified (Arcana for plain magic, Theology for
  sacred/cursed). ApplyOn applies + stamps.

Identification scripts ([arcana.src](../pkg/skills/arcana/arcana.src),
[theology.src](../pkg/skills/theology/theology.src)) check `#EquippedOn`
after flipping `magic = 2`. If the item is currently worn, they call
`EquipModStamper(item).ApplyOn(wearer)` directly so the bonus appears
immediately. Critical for cursed items, which can't be unequipped to
re-trigger the equip path.

### Why it can't drift

ApplyOn writes the stamp at the exact moment of application using the
values it actually applied. RemoveFrom **never reads the item's `equip`
dict** — it only reads the stamp. So even if:
- the item's `equip` mutates while worn,
- the identification flag flips between equip and unequip,
- the item is destroyed while worn ([onDestroy.src](../pkg/systems/combat/onDestroy.src) calls `RemoveFrom` against the stamp),

…the rollback subtracts exactly what was added.

**Hitsmod underflow guard** preserved from the legacy applier: RemoveFrom
erases `#hitsmod` if the resulting total would be `≤ 0`, preventing a
negative on-character vital mod from accumulating.

### Cursed unequip

The cursed-block lives in **two** places, both strict-first:

1. [scripts/misc/unequiptest.src](../scripts/misc/unequiptest.src) — POL's
   global UnequipTest. Checks `equip_data.Exists("cursed")` (uses `.Exists`
   not `in`, because `in` on a dict is silently false in eScript). If
   cursed, returns 0 BEFORE `RemoveFrom` runs — the stamp survives, and
   POL refuses the unequip. Bonus persists.
2. [pkg/systems/combat/unequip.src](../pkg/systems/combat/unequip.src) —
   defense-in-depth for combat-class items. Same effect.

Removing a cursed item requires the `removermaldicao` priest spell.

---

## 6. The "everything else" path — global hooks fire even when itemdesc is silent

Rings, necklaces, bracelets, earrings, robes, capes, shirts, pants, etc.
have no `EquipScript`/`UnEquipScript` in their itemdesc — so no per-item
combat gate runs. **However**, POL's *global* `EquipTest` /
`UnequipTest` hooks ([scripts/misc/equiptest.src](../scripts/misc/equiptest.src) /
[scripts/misc/unequiptest.src](../scripts/misc/unequiptest.src)) fire for
every equip/unequip event. Those hooks invoke `EquipModStamper`, so
jewelry and clothing get the same modifier treatment as combat gear.

What jewelry/clothing get:
- `#EquippedOn` stamp (used by corpse, death, destroy hooks)
- mask handling (`coverFace` → `ApplyIncognito`)
- frozen/paralyzed and broken-item checks
- hidden-reveal on Body/Arms/Legs zones
- **EquipModStamper apply/remove** — magic rings actually buff the wearer
- cursed-block at the global UnequipTest level — cursed jewelry can't be
  dragged off (verified by `.Exists("cursed")`, not the broken `in`-on-dict)

What jewelry/clothing do NOT get:
- Combat gate restrictions (descmod, STR check, Steam Armor handling, …)
- Wrapper-level `ArmorWrapper`/`WeaponWrapper` modifier application —
  jewelry doesn't go through `:equipsys:equipitem`, so it gets nothing
  from the wrapper modifier list. The `EquipModStamper` path is the
  ONLY mod surface for jewelry today.
- Ability `OnPassiveOnEquip` triggers (those fire only from `:equipsys:equipitem`).

Jewelry that wants to grant skill mods via `AttributeModifier` (the
wrapper path) does NOT work — `:equipsys:equipitem` rejects anything that
isn't `ArmorWrapper` or `WeaponWrapper`. That gap is the next refactor,
out of scope for the EquipModStamper consolidation.

---

## 7. Special equip/unequip programs

A handful of items override the combat path with their own scripts:

- [pkg/items/mounts/mount/equip.src](../pkg/items/mounts/mount/equip.src) /
  [pkg/items/mounts/mount/unequip.src](../pkg/items/mounts/mount/unequip.src)
  — defined as `program EquipScript(mobile, item, start_up)` / matching
  unequip, **both currently no-op (`return 1`)**. Real mount logic lives
  elsewhere (the mount item is a vehicle facade).
- [pkg/fls/fls_items/orcsmask/unequip.src](../pkg/fls/fls_items/orcsmask/unequip.src)
  — declares `program unequipweapon(who, item)` even though it's a head
  piece; orc-mask-specific cleanup. Paired with `equiporcmask` (compiled,
  source either renamed or pre-compiled — referenced from the cfg).
- [pkg/systems/combat/unequipArcaneBow.src](../pkg/systems/combat/unequipArcaneBow.src)
  and [pkg/systems/combat/unequipSpiritualBlade.src](../pkg/systems/combat/unequipSpiritualBlade.src)
  — bespoke unequip programs for two unique weapon items; both also
  declared with the `unequipweapon(who, item)` signature.

None of these chain back into `:combat:equip`/`:combat:unequip`, so any
restriction enforced in the gate is **bypassed** for these items. (Mounts
are obviously a special case; the FLS skin items inherit nothing from the
combat gate either.)

---

## 8. Logon and the asymmetry

[`:equipsys:logon`](../pkg/systems/equipsys/logon.src):

```escript
program logon(player)
    Start_Script("packets/AssyncUpdateEquipment", array{MobileWrapper(player), true});
    return 1;
endprogram
```

On login the engine restores items to their layers from save, but
**neither** `:combat:equip` nor `:equipsys:equipitem` re-runs. The only
thing fired is the equipment-update packet so the client paperdoll
refreshes. Implications:

- `EquipModStamper` writes the `#equipMods_<serial>` stamp directly on
  the mobile as an ObjProperty, which survives save/restore. The applied
  `#manamod` / `#hitsmod` / `#stammod` accumulators are also persistent.
  So no re-apply is needed — the state was already there from the
  original equip event.
- Wrapper-driven `AttributeModifier.ApplyOn` writes also need to survive
  the save cycle for paperdoll math to remain correct after logon. The
  current code assumes they do; this should be verified before any
  further refactor.
- **POL's global `EquipTest` does NOT fire on logon-restore.** If a
  player logs in wearing a magic item that has NO stamp (e.g., an item
  introduced post-refactor whose first equip predated the stamper, or a
  one-off GM grant), the stamp will only be written on the next
  unequip+re-equip. The mod itself was already on the mobile so this is
  cosmetic, but worth knowing if drift surfaces on legacy characters.

---

## 9. Sources of truth (current)

| Datum | Home |
|---|---|
| Base stats from gear (str/dex/int) | `EquipModStamper` reading `equip.strmod` / `dexmod` / `intmod` |
| Vital bonuses (mana/hits/stam) | `EquipModStamper` reading `equip.manamod` / `hitsmod` / `stammod` |
| Skill mods | `EquipModStamper` default branch (any unknown dict key → `AP_ModifySkillMod`) |
| Resists | `ArmorWrapper.GetResists()` for the value (slot + protection + material + rune); `EquipModStamper` triggers `RecalcVitals` when `equip.*ResistMod` is present |
| Ability-driven bonuses | `EquipWrapper.ApplyAbilityBonus` → `ability_bonuses` property → `SetupOnEquip` |
| Damage / speed / crit | `WeaponWrapper` |
| Coverage | `ArmorWrapper.GetCoverage` + slot tables in `:equipsys:Controller/ArmorController` |
| STR required | `EquipWrapper.GetSTRRequired()` (invoked from combat gate) |
| Material / durability / weight | `EquipWrapper.GetMaterial`, `EquipmentController` tables |
| Steam-armor side effects | `MobileWrapper.RecalcCarryWeightMod`, `#steam_dex_penalty` |

Two parallel paths still exist: `EquipModStamper` for `equip`-dict mods,
and `MobileWrapper.ApplyItemModifiers` for `AttributeModifier`-list mods
on armor/weapons. They don't collide today because they touch disjoint
data — wrapper mods are skill mods and armor penalties; stamper mods are
stat/vital/resist/skill from the magic-item layer. Future consolidation
could fold `EquipModStamper` into the wrapper's modifier dict on a
generic `EquipWrapper`.

---

## 10. Unequip — what actually rolls back

[`:combat:unequip`](../pkg/systems/combat/unequip.src):

1. **Curse check** (strict-first): if the `equip` dict has `cursed`,
   abort unless the mobile is dead. (If dead, dispatch
   `:equipsys:unequipitem` so the corpse strips clean; otherwise
   return `0` to cancel.) Must run BEFORE any modifier rollback path.
2. Reverses the `descmod` array bookkeeping and re-`SetName`s.
3. `if (who.npctemplate) return 1` — NPC path bypasses everything below.
4. Erases `twohanded` when removing a weapon.
5. Carry-weight recalc, this time with **`excludeSerial = item.serial`**
   because the leaving item is still on its layer during the hook.
6. Reverts Steam Armor `#steam_dex_penalty` via `AP_ModifyStatMod`.
7. Reverts legacy `#armor_evasion_penalty`.
8. Blocks unequip of Steam Arms if a gadget is still installed.
9. Destroys the companion gloves item when Steam Gloves come off.
10. `Run_Script_To_Completion(":equipsys:unequipitem", {who, item})`.
11. (Removed — `equip`-dict modifier rollback is now owned by
    [scripts/misc/unequiptest.src](../scripts/misc/unequiptest.src) via
    `EquipModStamper.RemoveFrom`, which already fired before this script
    in POL's UnequipTest → UnEquipScript sequence.)

[`:equipsys:unequipitem`](../pkg/systems/equipsys/unequipitem.src) is the
clean inverse of `equipitem`:

- Build the wrapper, re-apply the Shaman/Conexão Divina/Scutum modifier
  overrides (so the mods being removed match the mods that were applied),
- `who.TriggerUnequipPassives(equip)` — fires `OnPassiveOnUnequip` then
  `equip.CleanupOnUnequip()` (re-adds ability bonuses to the modifier
  values one last time, so `RemoveItemModifiers` subtracts the right
  totals, and erases `ability_bonuses`),
- `who.RemoveItemModifiers(equip)` — walks `equip.GetModifiers()` and
  calls `RemoveFrom(who)`,
- Removes `ManaModFabricacao` for staff/wand/spellbook,
- Fires the async equipment packet update with `isEquipping = false`.

The two unequip steps (10 and 11 of the gate) leave the mobile in the
state it had **before** the equip — provided no third party wrote to
`#manamod` etc. in the meantime.

---

## 11. Layers and what each path actually touches

Layer constants live in
[itemtypes.inc:1456-1486](../pkg/utils/itemUtils/include/itemtypes.inc#L1456-L1486).
Annotated by which equip path each typically reaches:

| Layer | # | Equip path today |
|---|---|---|
| `LAYER_HAND1` | 0x1 | combat (weapons) |
| `LAYER_HAND2` | 0x2 | combat (shields, two-handed weapons) |
| `LAYER_SHOES` | 0x3 | silent |
| `LAYER_PANTS` | 0x4 | silent |
| `LAYER_SHIRT` | 0x5 | silent |
| `LAYER_HELM` | 0x6 | combat |
| `LAYER_GLOVES` | 0x7 | combat |
| `LAYER_RING` | 0x8 | silent |
| `LAYER_NECK` | 0xA | silent |
| `LAYER_HAIR` | 0xB | silent |
| `LAYER_WAIST` | 0xC | silent |
| `LAYER_CHEST` | 0xD | combat |
| `LAYER_WRIST` | 0xE | silent |
| `LAYER_BEARD` | 0x10 | silent |
| `LAYER_TUNIC` | 0x11 | silent |
| `LAYER_EARS` | 0x12 | silent |
| `LAYER_ARMS` | 0x13 | combat |
| `LAYER_CAPE` | 0x14 | silent |
| `LAYER_PACK` | 0x15 | (backpack — skipped by `EnumeratePhysicalItems`) |
| `LAYER_ROBE` | 0x16 | silent (treated as Body slot in `ArmorController` if it does enter combat — see [ArmorController.inc:39-56](../pkg/systems/equipsys/include/Controller/ArmorController.inc#L39-L56)) |
| `LAYER_SKIRT` | 0x17 | silent |
| `LAYER_LEGS` | 0x18 | combat |
| `LAYER_MOUNT` | 0x19 | mount/equip,mount/unequip |

`EnumeratePhysicalItems` (used by `UnequipAll`) explicitly skips
`{11, 16, 21, 25}` (`LAYER_TUNIC`, `LAYER_BEARD`, `LAYER_PACK`,
`LAYER_MOUNT`) — [equipment.inc:22-38](../pkg/tn/include/equipment.inc#L22-L38).

---

## 12. Anatomy of the two families

The user's intuition matches the code: there are effectively two
equipment families.

### Family A — "combat-class"
*Definition (today):* item is declared in `pkg/systems/combat/itemdesc.cfg`
(or one of the inheriting itemdescs) and therefore carries
`equipscript equip` / `unequipscript unequip`.

*Members:* weapons, armor pieces, shields. Also: bows of all kinds,
staves, wands, spellbooks, instruments declared in combat.

*Pipeline:* full — gate → orchestrator → wrappers → legacy mods.

*Mutable surfaces:* damage, resists, ability passives, stat mods,
skill mods, vital bonuses, descmod, race/class restrictions, STR
checks, ability bonuses, ManaModFabricacao.

### Family B — "common"
*Definition (today):* item has no per-itemdesc `EquipScript`/`UnEquipScript`.

*Members:* rings, necklaces, bracelets, earrings, shirts, pants, shoes,
robes, capes, sashes, hair, beard, mundane clothes, decorative wear.

*Pipeline:* POL's global `EquipTest`/`UnequipTest` hooks fire (see §6).
No combat gate. No wrapper-level modifier path.

*Mutable surfaces:* `EquipModStamper` applies/removes the `equip`-dict
mods (stats, vitals, resists, skill mods). Ability `OnPassiveOnEquip`
does NOT fire — that's wrapper-gated and still scoped to armor/weapons.

The qualitative split between Families A and B now matters only for
restriction enforcement and wrapper-level effects (passives, damage,
shield attributes). Magic-item bonuses behave identically across both.

---

## 13. Consolidation status

**Done (2026-05-24):**
- `equip`-dict modifier application centralized in `EquipModStamper`
  (one class, one apply/remove pair).
- POL global `EquipTest` / `UnequipTest` are the only entry points for
  apply/remove. Combat gate is restriction-only.
- Per-item delta stamping (`#equipMods_<serial>`) eliminates drift on
  identification flips, dict mutations, and destroy-while-equipped.
- Identification scripts (Arcana, Theology) re-apply immediately when
  the item is currently worn — critical for cursed items that can't be
  unequipped to trigger the equip path normally.
- Cursed-block is strict-first in both `unequiptest.src` (uses
  `.Exists("cursed")`) and `combat/unequip.src` (uses `.keys()`).
- Magic jewelry / clothing now apply their `equip`-dict mods
  (previously a silent no-op).
- Legacy `ApplyEquipObjMods` / `RemoveEquipObjMods` deleted.

**Open (not in this refactor):**
- `MobileWrapper.ApplyItemModifiers` is still armor/weapon-only.
  A generic `EquipWrapper` modifier list for jewelry/clothing would let
  the wrapper own everything and retire the `equip` dict entirely.
- Cursed-dead unequip path in combat/unequip dispatches to
  `:equipsys:unequipitem` directly, bypassing the global `UnequipTest`,
  so the stamper doesn't run there. Functional only because cursed
  items today survive death; revisit if that policy changes.
- A single registered `EquipScript`/`UnEquipScript` on every itemdesc
  (instead of relying on the misleadingly-named global hooks) would
  improve discoverability — but POL hard-codes the global hook paths,
  and renaming requires a core fork.
- No logon recompute of stamps — drifted characters from before this
  refactor are fixed in-game by GM, not automatically.

---

## 14. Quick map for newcomers

If you are touching equipment code:

- A bug "magic item bonus didn't apply" → check the identification gate
  in `EquipModStamper._ShouldApply` (§5). Items with `magic == 1` skip
  apply by design — identify with Arcana (plain magic) or Theology
  (sacred/cursed) to flip to `magic == 2`. If the item is currently worn
  when identified, the identification script applies immediately.
- A bug "bonus didn't reverse on unequip / stat drift" → check
  `#equipMods_<serial>` on the player. If absent, apply never ran. If
  present after unequip, `RemoveFrom` was bypassed (e.g., cursed-block
  short-circuit, or a destroy path that didn't fire `onDestroy`).
- A bug "cursed item can be dragged off" → check both cursed-blocks use
  the right form (`.Exists("cursed")` for dicts; `.keys() with in` for
  arrays). `in` on a dict is silently false.
- A bug "wrapper modifier didn't apply on armor/weapon" → see
  `ApplyItemModifiers` (§4) — separate path from EquipModStamper.
- A bug "I can't equip this with that ability" → restriction is most
  likely one of the 26 checks in §3.
- A bug "my dex is wrong after unequipping Steam Legs" → carry-weight
  and dex penalty are in the gate, not the orchestrator. See §3 #25,
  #26 and `unequip.src` body.
- Adding a new equip-time stat/vital/resist effect on jewelry/clothing
  → write it as a key in the `equip` ObjProperty dict; `EquipModStamper`
  handles the rest.
- Adding a new equip-time effect on armor/weapons that touches the
  wrapper modifier list → use `AttributeModifier` via the wrapper, and
  let `ApplyItemModifiers` handle it.
