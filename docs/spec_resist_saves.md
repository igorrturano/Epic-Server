# Spec — Resist Saves (Track R)

Status: IMPLEMENTED 2026-05-19 (R2A–R2G shipped). This file is the
design record; see commits `aeddeaa95`..`c6ebdf94f` for the live changes.

R2 deltas vs the original "MISSING" list:
- magicmissile reverted from REFLEX save (auto-hit flavor spell by
  design — confirmed during smoke test).
- frostbolt + ignite WILL-partial patterns (slow-on-save / halve-DoT
  duration) kept as legitimate variants of the WILL convention —
  these aren't pure "negate" but they ARE intentional partial saves;
  forcing them to negate would erase deliberate design.
- turnundead custom-diff and detectarcorrupcao 3-arg CheckResist kept
  as intentional pre-existing variants (divine/utility spells).
- Bug fix folded into R2G: banimentoarcano had `CheckResist(..., bonus)`
  (positive sign) where canonical is `-bonus` — that flipped the
  Wisdom-path effect from "harder to resist" to "easier to resist".
  Real bug, not a stylistic normalization.

## Decisions (locked)

1. **REFLEX convention** = halve damage on save success
   (fireball/meteorswarm/icestorm style; not lightningbolt's fail-is-crit).
2. **Scope** = add saves to the 19 MISSING list **and** normalize the 44
   existing saves to the chosen conventions (REFLEX→halve, WILL→negate,
   movement→REFLEX-halve per `arcanerepulsion`).

## Canonical patterns (grounded in current code)

### REFLEX — halve damage on success
Reference: `fireball.src:96`, `meteorswarm.src:54`, `icestorm.src:54`.

```escript
var diff := handleSpellResistDiffForSpecialists(caster, spell_school);
var bonus := epicWisdomPathBonus(caster);
if (CheckResist(targ, REFLEXES, diff, -bonus))
    dmg := Cint(dmg / 2);
    SendSysMessageEx(targ, "<spell-specific evasion text>", SSM_INFO);
endif
SC_DealDamage(caster, targ, CInt(spell.id), Cint(dmg), DMG_<type>);
```

`CheckResist` returns >0 on **success** (target resists). The `<= 0`
fail-is-crit variant exists in `lightningbolt.src` and will be flipped
during the normalize phase.

### WILL — negate debuff on success
Reference: `ampliardor.src:26`.

```escript
var diff := handleSpellResistDiffForSpecialists(caster, spell_school);
var bonus := epicWisdomPathBonus(caster);
if (!CheckResist(targ, WILL, diff, -bonus))
    // apply effect (SC_ApplyEffect / SetCooldown / etc)
endif
// success path: silent or short "resistiu" msg — keep consistent
```

### REFLEX — partial effect on movement
Reference: `arcanerepulsion.src:54`. The codebase convention for
forced-movement / knockback is REFLEXES-halve (not FORTITUDE-negate as
my initial audit assumed). One save type for the whole spell where
possible.

## Per-spell mapping (R2)

### MISSING — add saves (19)

REFLEX halve (17):
- mage/magicmissile · DMG_FORCED · halve each missile
- mage/chainlightning · DMG_ENERGY · halve per jump
- mage/firebolt · DMG_FIRE
- mage/trovao · DMG_ENERGY
- shaman/firebolt · DMG_FIRE
- shaman/trovao · DMG_ENERGY
- shaman/rayfrost · DMG_COLD
- shaman/earthquake · DMG_FORCED
- priest/chamasagrada · DMG_FORCED
- priest/flamestrike · DMG_FIRE
- demonhunter/disparodivino · DMG_FORCED · check anti-demon bonus path
- trickster/disparomistico · DMG_FORCED
- swordmage/furiaglacial · DMG_COLD
- swordmage/tempestade · DMG_COLD
- swordmage/pandemonio · DMG_FIRE
- druid/tempestadeespinhos · DMG_PHYSICAL
- druid/earthquake · DMG_FORCED

WILL negate (1):
- necromancer/apodrecer · negate the -15 hitchance_mod application
  (mirror ampliardor/atrapalhar canonical pattern; SC_ApplyEffect has
  no save layer itself — gate before the call)

FORTITUDE negate (1):
- shaman/bloodbath · paralysis + kill — FORTITUDE feels right
  thematically but no other spell uses FORTITUDE-negate; if we want
  one save type per spell convention, this might become REFLEX-partial.
  Decision in R2F.

### Normalize — existing (R2G, after MISSING ships)

- `lightningbolt.src` fail-is-crit → halve-on-success (only existing
  REFLEX-crit; everything else already halves).
- Audit walk: 44 existing saves; flag any that deviate from the three
  patterns above and align.

## Phasing

- **R2A** (4 files): mage REFLEX — magicmissile, chainlightning,
  firebolt, trovao. Pilot batch; verify pattern lands cleanly in
  practice before the rest.
- **R2B** (4): shaman REFLEX
- **R2C** (4): priest + demonhunter + trickster REFLEX
- **R2D** (3): swordmage REFLEX
- **R2E** (2): druid REFLEX
- **R2F** (2): apodrecer WILL + bloodbath FORTITUDE
- **R2G** (≤44): normalize existing — swarm

Each phase: edit, commit, you compile + smoke-test, next phase. CLAUDE
cannot run ecompile (project rule).

## Open flag

`bloodbath` save type — FORTITUDE-negate is the audit's
recommendation but no other spell uses that pattern; could be REFLEX-
partial (halve paralysis duration or skip the kill) to match
arcanerepulsion. Flagging for R2F; default = FORTITUDE-negate per
audit unless overridden.
