# Fire Field Leak Remediation + Pira Purificadora Redesign

**Date:** 2026-05-31
**Status:** Approved design, pending spec review

## Problem

Production profiling showed `pkg/items/walkOn/fireField/control.ecl` consuming
~50% of all server CPU. Investigation (see conversation trace) established:

- Every `FireField` item (objtype `0x398c` / `0x3996`) auto-launches one
  `control.ecl` that loops every 250ms for the item's lifetime.
- A field with **no `Duration` property** never expires (`IsExpired()` returns 0
  by design — permanent fields are intentional), so its control loop runs until
  shutdown.
- Gameplay creators all set `Duration` and self-clean (verified: a single Pira
  cast produced 64 fields, all of which terminated cleanly via the `while(field)`
  destroy path — no stale-ref spin, no leak in the loop itself).
- The residue is **immortal fields**: 9 found in prod world data, all with no
  `Duration`, persisting across every world save (count only ever grows).
- **Pira Purificadora** is the worst offender: it spawns ~64 `FireField` items
  per cast, each of which also ignites the **propagação** system on flammable
  terrain, compounding the cost.

The leaked resource is not the item — it is the per-field **running script**
(never freed) plus the **propagação markers/scripts** each field perpetually
re-ignites, multiplied across many fields and hours of uptime.

## Goals

1. Stop Pira Purificadora from creating physical fire-field items entirely.
2. Make intentional permanent (no-`Duration`) fields cheap so they no longer
   drain CPU — without removing the permanent-field capability.
3. Remove the temporary debug instrumentation added during investigation.

Out of scope: the `.firefieldscan` GM command (already removed; existing
immortal fields cleared manually in prod). Other field creators
(dragon, fireBeetle, bafodedragao, catapulta, swordmage) already set `Duration`
and are unaffected.

## Part 1 — Pira Purificadora → NATURE-scaled burning hands

Rewrite `pkg/systems/spells/spell_logic/druid/pirapurificadora.src` as a port of
`pkg/systems/spells/spell_logic/mage/burninghands.src`, scaled off `NATURE`
instead of `ARCANA`. No `CreateItemAtLocation` — damage and visuals only.

Behavior change: Pira goes from a persistent burning **zone** to an **instant
cone hit + per-target fire DoT**. It loses zone-control; it gains leak-immunity.
This is the accepted tradeoff.

- **Plumbing (druid convention):** `caster=params[1]`, `targ=params[2]`,
  `spellinfo=params[3]`; `spell_circle := CInt(spellinfo.Circle[1])` (Circle 2),
  `spell_school := spellinfo.School`.
- **Cone:** keep Pira's reach `range := CInt(AP_GetSkill(caster, NATURE)/20) + 2`
  (not nerfed to burning hands' `/20`). Facing-based `BuildConeRings` +
  `PlayConeEffect`, after `TurnObjectToward(caster, targ.x, targ.y)`.
- **Visual identity kept:** flame effect `0x3709`, fire sounds `0x1de` / `0x208`,
  and flavor text ("*Uma pira purificadora emerge…*").
- **Impact damage (per mob in cone):**
  `epicSpellDamageRoll(caster, spell_circle, spell_school)` then
  `bonusSpellDamageForSpecialists`. Reflex save halves impact
  (`CheckResist(mob, REFLEXES, diff, -bonus)`), with
  `diff := handleSpellResistDiffForSpecialists(caster, spell_school)` and
  `bonus := epicWisdomPathBonus(caster)`. Then `SC_DealDamage(... DAMAGE_TYPE::FIRE)`.
- **Fire DoT:** `SC_ApplyEffect` via `SpellEffectBuilder`, `1d4+1` per 2s tick,
  `dot_total_ticks := CInt(NATURE/15)` clamped to `[4, 10]`, duration
  `dot_total_ticks * tick_interval`. Recast refreshes (no stacking), matching
  burning hands.
- **No SC effect on caster** (the old caster-side timer/tick/expire model is gone).

### Dead code removed

- `pkg/systems/spells/spell_logic/druid/pirapurificadoraTick.src`
- `pkg/systems/spells/spell_logic/druid/pirapurificadoraExpire.src`

(Their generated `.ecl` / `.dep` become orphaned and should be deleted.)

### Config

`pkg/systems/spells/config/druid.cfg` (SpellId 719): update `Desc` and
`FlavorText` from "barreira/parede de fogo" wording to describe the cone burst.

## Part 2 — `control.src` permanent-field guard + debug cleanup

In `pkg/items/walkOn/fireField/control.src`:

- `isPermanent := (Duration missing/error)`.
- **Tick cadence:** permanent fields tick every `PERMANENT_TICK_MS` (2500ms);
  temporary fields keep `FIELD_TICK_MS` (250ms). ~10× CPU reduction for permanents.
- **Skip `DanificaObjetosProximos`** for permanent fields (decorative fire should
  not grind item durability every cycle). Temporary fields keep the existing
  every-`ITEM_DAMAGE_EVERY_N_TICKS` behavior.
- `BurnNearbyMobiles` + walkOn burn remain for both (fire still hurts), at the
  field's own cadence.
- Permanent fields still never expire (intentional). Temporary unchanged.
- **Remove all debug `print`s**: `FIELD_DEBUG` block and the START / EXIT /
  STALE-REF / OVERDUE lines.

In `pkg/systems/propagacao/controladora.src`:

- **Remove** the `PROP_DEBUG` block and the `[PROP CYCLE]` timing print added
  during investigation.

## Operational follow-up (not code)

The 9 prod immortal fields were already cleared manually. No deploy action needed
beyond compiling the changed scripts.

## Files touched

| File | Action |
|---|---|
| `spell_logic/druid/pirapurificadora.src` | rewrite |
| `spell_logic/druid/pirapurificadoraTick.src` | delete |
| `spell_logic/druid/pirapurificadoraExpire.src` | delete |
| `systems/spells/config/druid.cfg` | edit Desc/FlavorText |
| `items/walkOn/fireField/control.src` | permanent guard + remove debug |
| `systems/propagacao/controladora.src` | remove debug |
