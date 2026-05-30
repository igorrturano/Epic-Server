# Spells Pipeline — Agent Reference

Canonical, code-grounded map of the SpellController (SC) framework: how a
cast becomes an applied effect, how the effect lives (ticks, resists,
buff icons), and how it dies (expiry → maintain, or dispel). Companion to
[`combat-pipeline.md`](combat-pipeline.md) (SC's resist layer + spell
damage feed into the damage pipeline) and
[`ability-pipeline.md`](ability-pipeline.md) (spell damage fires ability
hooks).

Every `file:line` here was traced firsthand against the **live code**.
The old `spells-guide` skill was materially stale — every line number was
off by 8–130 lines, it described the resist storage model wrong, and it
omitted `tick_damage`, `SC_Damage`, the `instance_id` token, and the cast
entry chain. **Do not trust the skill. If you change SC, update this
doc.**

## TL;DR

- A spell is **one config row** (with a `SpellScript` path) + **one
  script** `pkg/systems/spells/spell_logic/<archetype>/<name>.src` whose
  `program` is `SpellScript(params)` (`params[1]=caster`, `[2]=targ`,
  `[3]=spellinfo`).
- The script builds **one `opts` struct** and calls
  `SC_ApplyEffect(caster, targ, opts, duration, spellId)`. SC does the
  rest: apply mechanics, persist opts, send buff icon, start expiry timer,
  optional tick driver + on-apply FX.
- Resist is a **two-layer derived model**, *not* a single summed prop —
  the skill got this wrong. `#buff_resist` (spell sum) and
  `polymorph_resistmod` (wildshape) are combined by `_RecomputeSpellResist`
  into the ephemeral `#SpellResist` that the damage pipeline reads.
- Expiry runs through `endspelleffect.src` — the **only** path that offers
  **maintain**. `SC_RemoveEffect` (dispel, replacement, mid-tick break)
  bypasses maintain by design.
- Everything crosses `Start_Script` boundaries; opts is persisted as an
  ObjProperty struct (`SpellOpts_{spellId}`), never a live object.

## Cast entry chain (how a click becomes `SpellScript`)

```
SpellStarter(params)                         cast_spell.src:35
  initializeSpellCasting / performPreCastingChecks / consumeResources
  performCastingProcess (cast bar, moveCheck) / selectSpellTarget
  checkCastingSuccess
  finalizeCasting(mobile, spellinfo, targ)   cast_spell.src:617
    CastSpell(mobile, spellinfo, targ)       include/tnmagery.inc:17
      if spellinfo.script != error:
        Start_Script(spellinfo.script, {mobile, targ, spellinfo})
```

- `spellinfo.script` = `GetConfigString(elem, "SpellScript")`
  (`tnmagery.inc:119` / `:159`). The config elem is resolved by
  `ResolveSpell(spell_id)` → `GetConfigFromId` (`SpellRegistry.inc:72`);
  ID ranges → type are in `GetSpellTypeFromId` (`SpellRegistry.inc:96+`,
  e.g. `<100` cantrip, `<200` evocation, `<600` priest …).
- The spell logic script is the canonical `program SpellScript(params)`
  (verified: `imunidadeveneno.src:11`). It builds `opts` and calls
  `SC_ApplyEffect` / `SC_RemoveEffect`. **SC is not auto-invoked** — a
  spell that doesn't call it doesn't route through this framework.

## Targeting & range (cast-entry side, not SC)

This is the cast driver, but it's where "click a target" becomes the
`targ` SC receives, and it's the layer that *should* gate distance/LoS.

- `selectSpellTarget(mobile, spellinfo, targ)` `cast_spell.src:463` — if
  no pre-supplied `targ`, prompts by the cfg `Target` field
  (`spellinfo.alvo`): `Self` → caster; `Unico` → `Target()` (notoriety
  from `spellinfo.align`); `Area` → `TargetCoordinates()`. Cancel →
  `FailSpell` + abort. **Spell scripts do not prompt their own target**
  for the common path — targeting is already centralized here (a minority
  still call their own `Spell_Target` internally; enumerate before
  assuming).
- `checkTargetValidity` `cast_spell.src:504` — central gates on every
  target: `repeal` invuln, `corpofechado`, **line of sight**
  (`CheckLineOfSight` for Unico `:521`, `CheckLOSAt` for Area `:527`),
  `charmed_by`.
- **Distance is gated centrally** (since 2026-05-19 refactor). The same
  `checkTargetValidity` branches `spellinfo.alvo`: `Unico` →
  `Distance(mobile, targ)`, `Area` → `CoordinateDistance(...)`, both vs
  `distanceAllowedForSpells(mobile, HandleSpellSkill(mobile), spellinfo.School)`.
  Self-cast scripts that internally `TargetCoordinates` (teleport, berge)
  must still gate locally — central can't see their second prompt.
- **Range + LoS are retryable** — `checkTargetValidity` returns
  `TARGET_RETRY` (const at `cast_spell.src:38`) for those, and
  `selectSpellTarget`'s loop re-issues the cursor instead of fizzling
  the cast. Invuln / corpofechado / charmed remain hard fails.
- **Pre-cast model preserved.** `SpellStarter` order is initialize →
  preCast → consumeResources → castingProcess → selectSpellTarget →
  checkSuccess → finalize. The cast bar runs first; the player then
  picks a target with the cursor up and can walk while doing so
  (`performCastingProcess` cleared `frozen` at the end of the cycles).
  Trade-off: an explicit ESC after the cast bar still fizzles and
  costs mana — the retry loop only catches bad clicks, not cancels.
- The freeze itself was moved out of `initializeSpellCasting` into the
  start of `performCastingProcess` so a Target() prompt would never run
  on a frozen caster (clients auto-cancel cursors when the mobile is
  frozen). Hard-fail branches in `checkTargetValidity` still release
  the cast lock via `_ReleaseCastLock` (`frozen := 0`, erase `#medding`
  and `#Casting`) so a repeal/corpofechado/charmed bounce doesn't leave
  the caster stuck.

### Range formula (`scripts/include/epicMageryUtils.inc:86`)

`distanceAllowedForSpells(who, skill, school)`:

```
base = clamp(5 + cint(skill/10), 7, 12)      // floor 7, cap 12 at skill 70
spec = min(3, cint(skill/30))                // applies if Evocator+Evocatium, Valdruk+Velkyn, or NPC
range = base + spec
      + 5   if caster has SPELL_RANGE (caminho ability)
```

`skill` = caster's casting skill via `HandleSpellSkill(who)` (`:415`) —
**Arcana for arcane casters, Theology for divine**. Smooth +1 tile / 10
skill. Every standard caller passes `HandleSpellSkill(caster)` (uniform
since the range rework). Canonical gate in a spell script:

```
var dist := Distance(caster, targ);            // Chebyshev, z-ignored
if (dist > distanceAllowedForSpells(caster, HandleSpellSkill(caster), school))
    SendSysMessage(caster, "O alvo esta muito distante"); return 0;
endif
```

Reference reach (post-2026-05-20 rebalance — top is 20):

| Profile | 0 | 30 | 50 | 60 | 70+ | 100 |
|---|---|---|---|---|---|---|
| Non-spec | 7 | 8 | 10 | 11 | 12 | 12 |
| Non-spec + SpellRange | 12 | 13 | 15 | 16 | 17 | 17 |
| Specialist / NPC | 7 | 9 | 11 | 13 | 14 | 15 |
| Specialist + SpellRange | 12 | 14 | 16 | 18 | 19 | **20** |

Design intent: low-skill floor of 7 (early-game survivability), mid-game
matches old behavior (11 at 60 skill), late-game plateaus past skill 70.
SpellRange ability always adds a clean +5.

### Coverage (post-2026-05-19 refactor)

Every spell whose cfg `Target` is `Unico` or `Area` is now gated by the
central skill-scaled check in `checkTargetValidity` (~39 inline gates
in the spell scripts were stripped as redundant — including the
necromancer single-target debuffs that were previously ungated).
Self-cast scripts that prompt their own coord (`teleport`, `berge`)
gate locally via `distanceAllowedForSpells` so the math matches.

**Deliberately *not* centralized** — different mechanics, kept as-is:
- *Channel-break radii* (drenarvida/folego/mana, horrorparalisante,
  maldicaoasan, transferenciavital, mage+shaman sugaralma,
  sentir_motivacao tick): re-checked inside the channel loop to break
  when the target walks away; fixed radius is the intent.
- *Touch range* (`> 1`: abrirfechaduramagica, vinculodasepultura,
  faroldosespiritos): adjacency mechanics, not spell-reach.
- *Shinobi* (irekawari, sentir_motivacao initial): retains Sneak/10
  identity scaling and the flat 5 RP-feel cap — `shinobi.cfg` is
  excluded from this refactor.
- *Bardo* (sussurro_do_vento flat 20): intentional flavor.
- *Orphaned virtue portals* in `mage/`: not referenced by any cfg or
  caller — dead code, candidate for a separate cleanup pass.

## File map (verified line numbers)

| Concern | File:line | Role |
|---|---|---|
| Framework | `pkg/systems/spells/include/Controller/SpellController.inc` | All of SC |
| Cast driver | `pkg/systems/spells/cast_spell.src:35` `SpellStarter` | Pre-cast → `finalizeCasting` |
| Cast dispatch | `pkg/systems/spells/include/tnmagery.inc:17` `CastSpell` | `Start_Script(spellinfo.script,…)` |
| Spell logic | `spell_logic/<arch>/<name>.src` `program SpellScript` | Builds opts, calls SC |
| Expiry handler | `pkg/mobiles/timedScripts/subScripts/magery/endspelleffect.src` | Timer fires → maybe maintain → `SC_RemoveEffect` |
| Maintain | `pkg/systems/spells/shared/offerMaintain.src` | Gump → mana → re-`SC_ApplyEffect` |
| Tick driver | `pkg/systems/spells/scTickDriver.src` | Periodic `tick_script` / `tick_damage` |
| Spell damage | `pkg/systems/spells/include/Controller/SC_Damage.inc:27` `SC_DealDamage` | Resist + `DamageFLS` + **fires ability hooks** |
| On-apply FX | `pkg/systems/spells/shared/playApplyFX.src` | Generic `on_apply_script` |
| Resync | `SpellController.inc:499` `SC_ResyncOnLogin` ← `logon.src:6` | Login/restart re-sync |
| Reader (combat) | `MobileWrapper.inc:454` `GetSpellResist` | Used by `DamageController.CalculateDefenderResist` |
| Pitfall site | `pkg/systems/combat/include/Controller/HitController.inc:427-429` | One-shot `#hit_mod` consume |

## `SC_ApplyEffect` — the apply path (`SpellController.inc:40`)

`SC_ApplyEffect(caster, targ, opts, duration, spellId, timerName := "spelleffect")`

1. **Fail closed if `duration < 1`** — returns an error struct, applies
   nothing (`:44`). Otherwise an effect like `frozen:=1` would never get
   undone.
2. **Velkyn focus duration bonus** — only if the caster has a non-zero
   `GetMagicItemVelkynDurationBonus` and the spell's School is `"Velkyn"`
   (`:51-59`).
3. **Buff category mutual exclusion** — if `opts.buff_category`,
   `_SC_RemoveConflictingBuffs` displaces the prior spell in that
   category, then writes `BuffCategoryIndex[category] := spellId`
   (`:64-73`).
4. `_SC_ApplyOpts(targ, opts)` — push mechanical changes (`:75`, see
   grammar below).
5. Stamp auto fields and persist: `opts.spellId`, `opts.totalDuration`,
   **`opts.instance_id := ReadGameClock()`** (per-apply token),
   `opts.caster_serial`, then
   `MobileWrapper(targ).SetProperty("SpellOpts_{spellId}", opts)`
   (`:77-88`).
6. `SendSpellBuff(caster, targ, spellId, duration, _SC_BuildDescription(opts))`
   — client buff icon (`:90-93`).
7. `TS_StartTimer(targ, "{timerName}_{spellId}", duration, spellId, caster, 0)`
   — expiry (`:95`).
8. If `opts.on_apply_script` → `start_script(it, {caster, targ, spellId,
   opts.on_apply_params})` (`:97-99`) — fires on **initial cast and every
   maintain re-apply**.
9. If `(opts.tick_script || opts.tick_damage) && opts.tick_interval` →
   `_SC_LaunchTickDriver` (`:101-103`).
10. If a buff was displaced and `opts.displayname` set → sysmessage
    "X foi substituído por Y" (`:106-108`).

## Opts grammar (verified against `_SC_ApplyOpts`/`_SC_RemoveOpts`)

Set only what the spell modifies; remove is the exact inverse.
`_SC_ApplyOpts` is at `:201`, `_SC_RemoveOpts` at `:284`.

| Field | Apply | Remove | Notes |
|---|---|---|---|
| `slash/piercing/blunt/fire/cold/poison/energy_resist` | `_SC_ApplyResist` (`:367`) | `_SC_RemoveResist` (`:389`) | Sums into `#buff_resist`, then `_RecomputeSpellResist`. **Not** written straight to `#SpellResist`. |
| `str_mod`/`dex_mod`/`int_mod` | `AP_ModifyStatMod(targ, …, +v)` | `AP_ModifyStatMod(…, -v)` | Only correct temp-stat path. Don't touch `*_mod` fields directly. |
| `color` | `targ.color := v` | `targ.color := targ.truecolor` | |
| `graphic` | `targ.graphic := v` | `targ.graphic := targ.trueobjtype` | Polymorph-class. |
| `speed_mod` | `run_speed += v` | `run_speed -= v` | Arithmetic only. |
| `speed_boost` | players: `PlayerSpeedBoost(targ,1)`; NPCs: snapshot `#sc_runspeed`, `run_speed:=250` | inverse per type | Classic haste. |
| `hitchance_mod` | `targ.hitchance_mod += v` | `-= v` | **Durational** native field. NOT `#hit_mod` — see pitfall 1. |
| `evasionchance_mod` | `targ.evasionchance_mod += v` | `-= v` | Durational native field. |
| `frozen` | `targ.frozen := 1` | `:= 0` | |
| `hidden` | `targ.hidden := 1` | `:= 0` | |
| `armor_hit_scripts` (array) | append to `ArmorHitScripts` ObjProp | **`EraseObjProperty(targ,"ArmorHitScripts")`** | Remove wipes the whole prop — stacking two spells that use this is fragile (`:328-330`). |
| `properties` (dict) | `SetObjProperty(targ,k,v)` each | `EraseObjProperty(targ,k)` each | Escape hatch. `#`-prefix = ephemeral. |
| `buff_category` | mutual-exclusion slot | index cleared if still owned | One per category per mobile. |
| `displayname` | replacement msg + maintain gump name | — | |
| `desc` | appended to buff-icon text | — | `_SC_BuildDescription` (`:632`) auto-prepends resist/stat lines. |
| `maintainable` | gates the maintain offer | — | Needs a live caster at expiry. |
| `on_apply_script` + `on_apply_params` | started every apply | — | Args `{caster, targ, spellId, params}`. Fires on maintain too — no one-shot logic here (pitfall 7). |
| `tick_script` + `tick_interval` + `tick_params` | tick driver | driver self-exits | Escape hatch for non-generalizable per-tick logic (santuario, manashield, etc.). Args `{caster, targ, spellId, tick_number, params}`. |
| `tick_damage` `{amount,type}` *or* `{dice,bonus,type}` + `tick_interval` | declarative DoT via `SC_DealDamage` | driver self-exits | `amount` fixed at cast; `dice` re-rolls each tick via `RandomDiceRoll` (`bonus` adds a flat int after). Independent of `tick_script` — set either/both. |
| `tick_heal` `{base,remainder}` + `tick_interval` | declarative HoT via `applyEpicHealing` | driver self-exits | Driver applies `base + (tick_number <= remainder ? 1 : 0)` so the leftover from `total / ticks` distributes across the first `remainder` ticks. |
| `tick_presentation` | driver-only (no apply mutation) | driver-only | One nested struct: `{ fx:{id,speed,frames}, sound, periodic_emote:{text,every,color}, final_overhead, final_emote:{text,color}, caster_feedback }`. All sub-fields optional. Driver plays per-tick FX/sound, emits periodic emote at `tick % every == 0`, fires `final_overhead`/`final_emote` on the last tick, and (for HoTs with `caster_feedback`) reports `+{n} HP ({tick}/{total})` to the caster. **Last-tick = `tick_number >= totalDuration/tick_interval`** — no `total_ticks` plumbing needed. |
| `on_expire_script` + `on_expire_params` | — | fired by `SC_RemoveEffect` **before** revert, args `{0, targ, spellId, params}` (no caster) | Read pre-removal opts here. |
| `spellId`/`totalDuration`/`instance_id`/`caster_serial` | auto-stamped — never set by hand | — | Resync + tick driver rely on these. |

## Resist model — the part the skill got wrong

There are **two persisted layers**, combined into one derived view:

- `#buff_resist` — struct of 7 ints, **ephemeral** (`#`-prefix, dropped
  on world save). Sum of all active resist-spell contributions.
  Maintained by `_SC_ApplyResist` (`:367`) / `_SC_RemoveResist` (`:389`).
- `polymorph_resistmod` — int, **persistent**. Wildshape's physical
  contribution (set/cleared outside SC by NaturalBond / endmetamorfose).

`_RecomputeSpellResist(targ)` (`:432`) derives the canonical
**`#SpellResist`** (ephemeral struct) per element:

```
slash/piercing/blunt = max(polymorph_resistmod, buff_sum)   # non-stacking
fire/cold/poison/energy = buff_sum                           # wildshape doesn't touch elementals
```

then `_SC_RefreshArmorPackets` re-emits armor packets for all 7 slots so
`StatusGumpEpic` shows it (`:469`, players only). **Anyone mutating either
layer must call `_RecomputeSpellResist` after** (public wrapper
`SC_RecomputeSpellResist:459`).

Consumed by combat: `MobileWrapper.GetSpellResist()`
(`MobileWrapper.inc:454`) reads `#SpellResist` and returns an
`ArmorResist`; `DamageController.CalculateDefenderResist` applies it as
the multiplicative spell-resist layer (see
[`combat-pipeline.md`](combat-pipeline.md) §5c). Note the wrapper builds
`ArmorResist(slash,piercing,blunt,fire,cold,energy,poison)` — energy/
poison order is swapped vs the struct field order; trust the field names,
not positions.

## Lifecycle: expiry, maintain, dispel

### Natural expiry → maintain (the only maintain path)

```
TS timer "spelleffect_{id}" fires → endspelleffect.src:6
  spellId := CInt(GetObjProperty(mobile,"prop"))      # shared "prop" — race-prone
  opts := MobileWrapper(mobile).GetProperty("SpellOpts_{id}")
  if opts.maintainable && opts.caster_serial:
     caster := SystemFindObjectBySerial(caster_serial)
     if caster.connected && !caster.dead: offer := 1; save opts+duration
  SC_RemoveEffect(mobile, spellId)        # effect goes away
  EraseObjProperty(mobile,"prop")
  if offer: start_script(:spells:shared/offerMaintain,
                         {caster_serial, mobile.serial, spellId, saved_opts, saved_duration})
```

`offerMaintain.src:30`: re-resolve caster/targ (bail on disconnect/death),
spell name + mana from `ResolveSpell` config; **`SUPPORT_CASTER` halves
mana (min 1)** (`:56-62`); `MaintainGump` Yes → `AP_ConsumeVital(MANA)` →
**`SC_ApplyEffect(caster, targ, opts, duration, spellId)` again** (`:74`)
— so `on_apply_script` re-fires (this is why maintainable buffs attach
`playApplyFX`, or the renewal is silent).

### `SC_RemoveEffect` (`SpellController.inc:122`) — dispel / replacement / mid-tick break

Reverses everything: fires `on_expire_script` first (pre-revert),
`_SC_RemoveOpts`, clears `BuffCategoryIndex` **only if this spell still
owns the slot** (ownership guard `:142` — do not reorder; replacement
depends on it), erases `SpellOpts_{id}`, `RemoveSpellBuff`. **Does NOT go
through `endspelleffect` → never offers maintain.** Correct for dispel,
mutual-exclusion replacement, and self-cast disengage that breaks on a
tick (e.g. santuario on damage).

## Tick driver (`scTickDriver.src`)

`detach()`'d loop: `sleep(tick_interval)` → re-read
`SpellOpts_{spellId}`; **self-terminates** when opts are missing
(expiry/dispel/replacement) (`:45-48`). Extra guards the skill missed:

- **Instance token**: if `my_instance && opts.instance_id != my_instance`
  → return (`:53-55`). A maintain re-cast stamps a new `instance_id` and
  launches its own driver; the old one exits to avoid double-ticking.
- **Orphan safety net**: if `tick_number > totalDuration/tick_interval +
  2`, the expiry timer failed (known `"prop"` race) — the driver calls
  `SC_RemoveEffect` itself and exits (`:65-72`).
- `tick_damage` → `SC_DealDamage(caster, targ, spellId, amount, type)`
  each tick (`:74-78`), then `tick_script` if set (`:80-82`).

## Spell damage → ability pipeline (`SC_Damage.inc:27`)

`SC_DealDamage(caster, targ, spellId, amount, damageType := DMG_FORCED)`
is the canonical spell-damage entry:

1. `+ GetMagicItemElementalBonus(caster, damageType)` (equipped focus).
2. `DamageFLS(targ, amount, type, caster, "spell_{spellId}")` — resist +
   `DisplayDamage` + `ApplyRawDamage` + last-damage info (from
   `DamageController_legacy`).
3. Builds a **spell-origin `AttackInfo`** (`SetSpellOrigin`), stamps the
   damage into the matching type slot, then fires
   `ExecuteCombatAbilitiesAfterDamage` + `ExecutePassiveAbilitiesAfterDamage`,
   and on `targ.dead` the `OnKill` pair (`:46-52`).

So spell damage triggers the same `AFTER_DAMAGE`/`ON_KILL` ability hooks
as melee — see [`ability-pipeline.md`](ability-pipeline.md). DoT spells
should declare `opts.tick_damage` rather than hand-rolling `ApplyRawDamage`
so this hook wiring is consistent.

## Spell-source bonus damage on melee/ranged swings

Spells that grant **per-swing damage bonuses** to the caster's normal
attacks (Frenesi, Forma Primordial, Lâmina Arcana wielding, Elemental
Weapon enchant, MagicBow arcane bonus) do not flow through `SC_DealDamage`
— they hook into the **physical attack** damage chain at one named seam:

`DamageController.CalculateSpellSourceDamage(byRef attackInfo)` is called
once during `CalculateAttackerDamage`, after `CalculateWeaponDamage` and
`CalculateEntropyDamage`. It dispatches in order to:

| Calculator | Activation | Effect |
|---|---|---|
| `CalculatePrimordialDamage` | `metamorfose` + `primordial_<element>` ObjProps | Mirrors blunt raw into matching elemental channel (druid/shaman wildshape) |
| `CalculateArcaneBowDamage` | `WEAPON_CATEGORY::MAGICBOW` + `fire/cold/energy_arrow` weapon prop | Arcana/5 or /10 to matching elemental |
| `CalculateSpiritualBladeDamage` | `WEAPON_CATEGORY::SPIRITUALBLADE` | Arcana/5 to posture-matched physical |
| `CalculateElementalWeaponDamage` | weapon's `#elementalweapon` ObjProp (charged buff) | Converts part of physical → elemental + Arcana/5 + INT/20, consumes a charge |
| `CalculateEnfurecidoDamage` | attacker's `enfurecido` ObjProp (set by Frenesi via `opts.properties`) | Nature/20 added to slash/pierce/blunt (Fury-style) |

Each contributor has a sibling `Estimate*Damage` function returning a flat
int, summed into the status-gump min/max preview in
`CalculateEstimatedMinMaxDamage`. **Both halves are required** — a new
spell-source modifier that only adds a `Calculate*` will silently lie in
the min/max preview.

### How to add a new spell-source modifier

1. In the `SpellScript`, set the activation flag via SC opts:
   ```escript
   opts.+properties := dictionary{ "mybufflag" -> 1 };
   ```
   `_SC_ApplyOpts` writes it as a real ObjProperty; `_SC_RemoveOpts` erases
   it on expiry/dispel/replacement. The flag survives logout because it's
   a normal (non-`#`) ObjProperty.
2. Add `CalculateMyBuffDamage(byRef attackInfo)` next to the others in
   `DamageController.inc`. Gate on `GetObjProperty(attacker, "mybufflag")`,
   then call `attackInfo.GetDamage().Increase*RawDamage(bonus)` on the
   channel(s) you want. The posture filter zeros unused physical channels
   later, so adding to all three slash/pierce/blunt is safe.
3. Add the matching `EstimateMyBuffDamage(attackInfo)` that returns the
   same bonus as a flat int.
4. Register both in `CalculateSpellSourceDamage` and the flat-bonus sum in
   `CalculateEstimatedMinMaxDamage`.

Reference implementation: Frenesi
([GrizzlyBear.src](../pkg/systems/spells/spell_logic/shaman/GrizzlyBear.src) →
[CalculateEnfurecidoDamage](../pkg/systems/combat/include/Controller/DamageController.inc)).

## Resync (`SC_ResyncOnLogin:499`, called from `logon.src:6`)

Intentionally **asymmetric** with apply — it does **not** call
`_SC_ApplyOpts` (additive fields would double). It:

- iterates `TS_GetTimerList`, keeps `spelleffect_*` timers; removes any
  with `remaining <= 0` (buffs pause while offline — uses saved
  `duration`, not endtime math) (`:505-522`).
- re-pushes **session-only** state: `PlayerSpeedBoost`, buff icons
  (`SendSpellBuff` with remaining + total), and **re-launches tick
  drivers** (detached scripts die on shutdown), resolving caster offline
  (`:529-549`).
- rebuilds **`#buff_resist` only if missing** (ephemeral; present ⇒
  same-session reconnect, rebuilding would double-count) (`:556-571`).
- `_SC_ResyncMetamorphosisResist` (rebuild `resist_physical_mod` if
  dropped) + `_SC_CleanupOrphans` (no timers ⇒ drop `#buff_resist`, and
  `#SpellResist` if no wildshape) + always `_RecomputeSpellResist`
  (`:577-581`).

If you add a new `_SC_ApplyOpts` side effect, classify it for resync:
session-only (re-push), additive math (leave), persistent server field
(leave), or ephemeral-derived-from-opts (rebuild-if-missing).

## Verified pitfalls

1. **`#hit_mod` ≠ `opts.hitchance_mod`.** `opts.hitchance_mod` adds to the
   native `hitchance_mod` field for the full duration (consumed at
   `HitController.inc:48`). `#hit_mod` is a **one-shot** ObjProperty:
   `HitController.inc:427-429` adds it then `EraseProp`s it on the next
   swing — gone after one attack regardless of the buff timer/icon.
   Symptom: "icon shows 30s but effect gone after one hit." Use
   `opts.hitchance_mod`. (Line numbers corrected from the skill's stale
   419-422.)
2. **`SC_RemoveEffect` bypasses maintain.** Only timer expiry through
   `endspelleffect.src` offers it. By design for dispel/replacement/
   disengage — don't expect "any removal offers maintain."
3. **`on_apply_script` fires on every apply, including maintain re-cast.**
   No one-shot/charge/cooldown logic there — it re-grants free each
   renewal. Put one-shot logic in the `SpellScript` body before
   `SC_ApplyEffect`, or in `on_expire_script`.
4. **Resync must not re-apply additive opts** (`:481-498` comment).
   Adding an `_SC_ApplyOpts` side effect without handling resync
   double-stacks it on reconnect.
5. **`armor_hit_scripts` remove erases the whole `ArmorHitScripts`
   prop** (`:329`). Two spells using it: the first to expire wipes both.
6. **`#`-prefix = dropped on world save** (`#buff_resist`, `#SpellResist`,
   `#sc_runspeed`, `#magiasilenciosa`, `#hit_mod`). SC only rebuilds
   `#buff_resist`/`#SpellResist` on resync; your own `#` opts.properties
   are intentionally not restored — usually what you want for
   effect-bound flags.
7. **`opts.properties` is not magic for combat damage.** Setting a flag
   via `opts.properties` only puts an ObjProperty on the target — nothing
   reads it unless you also add a `Calculate*Damage` + `Estimate*Damage`
   pair into `CalculateSpellSourceDamage` (see the spell-source bonus
   damage section above). The flag is purely a contract between your
   spell and your combat hook.
8. **`tick_presentation` is driver-only, never apply/remove state.**
   It lives outside the `_SC_ApplyOpts` / `_SC_RemoveOpts` inverse pair
   on purpose — re-stamping it on resync would replay starting from
   tick 1 (`SC_ResyncOnLogin` re-launches the driver with a fresh tick
   counter), but the original totals still expire on the same wall
   clock, so the final-tick emote may never fire if the player logged
   back in past the last expected tick. That's an accepted edge case;
   don't try to "fix" it by persisting tick state — the symptom is just
   a missing flavor line, not gameplay impact.
9. **Persisted opts must stay a flat struct, never a class instance.**
   The eScript class-serialization rule (see codebase memory) eats
   method dispatch on `SetProperty`/`Start_Script` round-trips.
   `SpellEffectBuilder` and `TickPresentation` are ephemeral authoring
   objects whose `.ToOpts()` / `.ToStruct()` flatten to plain structs
   before persistence — don't try to persist the builder itself.

## Connection to the other pipelines

| This doc | Hands off to |
|---|---|
| `#SpellResist` derived view | [`combat-pipeline.md`](combat-pipeline.md) §5c — spell-resist layer in `CalculateDefenderResist`, read via `MobileWrapper.GetSpellResist` |
| `opts.hitchance_mod`/`evasionchance_mod` | [`combat-pipeline.md`](combat-pipeline.md) — native fields read in `HitController` |
| `SC_DealDamage` post-damage hooks | [`ability-pipeline.md`](ability-pipeline.md) — `AFTER_DAMAGE`/`ON_KILL` dispatch |

## When you change SC

- New spell: write the config row with `SpellScript`, the
  `program SpellScript(params)` script, build `opts`, call
  `SC_ApplyEffect`. Start from `imunidadeveneno.src` (verified minimal
  maintainable template). `SC_RemoveEffect(targ, CInt(spell.id))` first
  for idempotent re-cast.
- **Prefer `SpellEffectBuilder`** (`:spells:Controller/SpellEffectBuilder`)
  for opts authoring — typed chained setters, returns `this`, and
  `.ToOpts()` flattens to the plain struct `SC_ApplyEffect` consumes.
  `TickPresentation` is its presentation sub-builder. See
  `ignite.src` / `burninghands.src` / `enxame.src` / `druidheal.src`
  for canonical usage.
- DoT/HoT: prefer `opts.tick_damage` (variable per-tick via `dice` or
  fixed via `amount`) and `opts.tick_heal`, paired with
  `opts.tick_presentation`, over a hand-rolled `tick_script`. Reach for
  `tick_script` only when per-tick behavior is non-generalizable
  (mutating ObjProps, conditional disengage, weapon-bound effects).
- New `_SC_ApplyOpts` field: add the exact inverse to `_SC_RemoveOpts`,
  add a `_SC_BuildDescription` line if user-visible, and decide its
  resync class.
- New resist source: it must flow through `#buff_resist` +
  `_RecomputeSpellResist`, never write `#SpellResist` directly.
- eScript identifiers are case-insensitive — verify renames with
  `grep -ri`.
