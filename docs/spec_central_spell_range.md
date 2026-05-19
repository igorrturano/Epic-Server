# Spec — Central Spell Range Gate + Target-First Retry QoL

Status: IMPLEMENTED 2026-05-19 (Phases 1, 2A, 2C, 2D shipped). See
`spells-pipeline.md` "Targeting & range" for the live behavior; this
file is kept as the design record.

Exclusions from the original scope (decided during implementation):
- `shinobi.cfg` excluded entirely (irekawari Sneak-scaled identity,
  sentir_motivacao flat-5 RP-feel kept).
- Bardo `sussurro_do_vento` flat 20 kept (flavor).
- Channel-break radii (drenar* / horror / maldicaoasan /
  transferenciavital / sugaralma / sentir_motivacao tick) kept — they
  are mid-channel break mechanics, not initial-cast reach.
- Touch-range (`> 1`) gates kept — adjacency mechanic.
- Virtue portals in `mage/`: orphaned dead code, not in cast pipeline;
  deferred to a separate cleanup.
- `acalmarbestas.src` (druid/) is a textcmd, not a spell — out of scope.

## Decisions (locked)

1. **Gate scope:** every non-self targeted spell (`spellinfo.alvo` ==
   `Unico` or `Area`), offensive *and* helpful, gets the skill-scaled
   distance cap. Cross-map heals/buffs become impossible.
2. **QoL:** target selection moves *before* mana/resource consumption;
   out-of-range / no-LoS re-prompts the cursor in a loop until a valid
   pick or explicit cancel. Cancel/timeout = clean abort, zero cost.
3. **Ad-hoc gates:** all hardcoded range checks normalized to
   `distanceAllowedForSpells(who, HandleSpellSkill(who), school)`,
   including `teleport` (loses its `Arcana/10` min-4 model unless
   overridden).

## Range formula (already shipped)

`distanceAllowedForSpells(who, skill, school)` —
`r = cint(skill/10)`, `range = 5 + r [+ r if Evocator/Valdruk own-school
or NPC] [+5 SPELL_RANGE]`, `skill = HandleSpellSkill(who)` (Arcana
arcane / Theology divine).

## Architecture

### Central gate — `checkTargetValidity` (cast_spell.src:504)

Add, alongside the existing LoS branch:

- `spellinfo.alvo == "Unico"` → `Distance(mobile, targ)` (Chebyshev).
- `spellinfo.alvo == "Area"` → `CoordinateDistance(mobile.x, mobile.y,
  targ.x, targ.y)` (targ is a coord struct from `TargetCoordinates`).
- `spellinfo.alvo == "Self"` → no gate (covers self-buffs; self-cast
  ground-target spells handle their own — see below).
- Compare to `distanceAllowedForSpells(mobile,
  HandleSpellSkill(mobile), spellinfo.School)`. Over → return a
  distinct "out of range" status (NOT a hard fail — the caller loops).

LoS already lives here; range sits next to it so there is **one**
validity choke point.

### Pipeline reorder — `SpellStarter` (cast_spell.src:35)

Current: initialize → preCast → **consume** → castingProcess →
**selectTarget** → checkSuccess → finalize.

New: initialize → preCast → **selectTarget (with retry loop)** →
consume → castingProcess → checkSuccess → finalize.

- Pre-supplied `targ` (params[4]: war-mode/scroll/ability) skips the
  prompt; still range/LoS-validated; if invalid, fail before consume
  (no cursor to loop on a non-interactive target).
- `selectSpellTarget` retry loop: on out-of-range / no-LoS, sysmessage
  the reason and re-issue `Target()`/`TargetCoordinates()`; only
  explicit cancel (ESC) or timeout aborts. Because this is now
  pre-consume, abort costs nothing — delete the `FailSpell`-refund
  concern entirely.
- Verify nothing in `selectSpellTarget`/`checkTargetValidity` depends
  on state set by `consumeResourcesForSpell` or `performCastingProcess`
  (scroll consumption stays in `checkCastingSuccess`, post-target —
  unchanged).

### Per-script normalization (cannot be central)

Spells with `alvo == Self` that internally `TargetCoordinates` /
`Spell_Target` and self-gate: confirmed `teleport` (`berge` suspected).
Phase 0 enumerates the full set. Each: replace the hardcoded gate with
`distanceAllowedForSpells(who, HandleSpellSkill(who), <school>)`.
`teleport` school = Evocatium ⇒ Evocator doubling applies, min-4 floor
removed (per decision 3; revisit if undesired).

### Strip redundant per-spell gates

The ~39 spells that already gate distance inline (audit `OK`) — remove
the now-duplicated `Distance()` + `distanceAllowedForSpells` block; the
central gate covers them. Removing avoids double-message / divergence.

## Phases

- **Phase 0** (sub-agent, read-only): exact file+line list of
  (a) inline-gated spells to strip, (b) hardcoded-gate spells to
  normalize, (c) self-cast-internal-target spells. Audit 2026-05-19 is
  ~90% of this; Phase 0 finalizes precise edit targets.
- **Phase 1** (1 file): `cast_spell.src` — reorder + central gate +
  retry loop + pre-targeted path. Verify.
- **Phases 2..N** (swarm, ≤5–8 files/agent): strip inline gates;
  normalize ad-hoc + self-internal spells. Verify each batch.
- **Final**: update the "Coverage gap / TBD" lines in
  `docs/spells-pipeline.md` to the implemented reality.

## Open flag

`teleport` reach changes under decision 3 (Evocator doubling, no min
floor). Proceed unless overridden.
