# Phase 4 Plan — TamedBrain consolidation

Branch: `feat/taming-beastmastery-petbrain` (worktree). Build only after approval.
Commit per sub-phase, ≤5 files/sub-phase, user compiles between sub-phases.

## The core problem found in research

`":ghaia:tamed"` is overloaded:
- **Assignment** (set the brain): `taming.src:254`, `adestrar.src:175`,
  `taming.inc:435` (RessuscitarPet), `textcmd/coun/tame.src:18`,
  `controlarbestas.src:20`. (Summon/necro/mage/merchant/controlarmortovivo
  → that's the `:ghaia:controlled` side = Phase 5, not here.)
- **Identity test** `script == ":ghaia:tamed"` / `script[":ghaia:tamed"]`
  meaning "is this a tamed pet?": `animalLore.src:40,171`,
  `taming.src:192,346`, `acalmar.src:48`, `herd.src:49,146`,
  `happyCounter.src:11`, `petfiel.src:74` (+ sweep for any missed).

If we repoint the brain without fixing identity tests, tamed-pet detection
silently breaks everywhere. So identity must be decoupled FIRST.

## Architecture decisions

1. **Identity predicate.** Add `IsTamedPet(creature)` to `taming.inc`
   (already included by all consumers). Definition: has `master`, has
   `"owner"` prop, NOT `"summoned"`. Brain script becomes an implementation
   detail. Aligns with `IsNonCombatantPet` (P2).
2. **Brain home.** New `pkg/mobiles/epicAI/scripts/tamedBrain.src`, a
   `Run_Script`'d program (mirrors `initialize.src`/`setup_mount.src`
   precedent) → its own 64KB compile unit, keeps heavy taming/mounts
   includes out of `brain.src`.
3. **Dispatch.** `brain.src` `DetermineBrainType()`: if
   `IsTamedPet(me)` → `brain_type := "tamed"`; new case
   `"tamed": Run_Script(":epicAI:scripts/tamedBrain", me)` reusing the
   existing `Run_Script` pattern at brain.src:46.
4. **Lifecycle (DECIDED).** `tamedBrain.src` keeps the ghaia-style
   permanent loop with `CAN_ACCESS_OFFLINE_MOBILES` (does NOT return to
   brain.src's dormancy model). brain.src delegates pets to it via
   `Run_Script` and that call owns the pet's lifetime. Behavior identical
   to today, zero regression. No idle-CPU win for pets — accepted.
5. **Port strategy — LIVE-ONLY reimplementation (per user).**
   `ghaia/tamed.src` is overloaded with dead/unused code. Do NOT port
   verbatim. P4b starts with a dead-vs-live audit of `ghaia/tamed.src`;
   `tamedBrain.src` reimplements ONLY the necessary live behaviors:
   - command loop: follow / attack / guard / stop / go-loc
   - obedience: `obediencecheck` (Taming) + `obedienceCheckCombat`
     (Beastmastery) — split already done in P1-adjacent code
   - feeding + bond/fidelizar (TakeItem → happiness, NeverRelease)
   - faithful pet (NeverRelease/"fiel" auto-obey, no-transfer)
   - happyCounter integration (keep sub-script; fix liveness check)
   - mount double-click, pack animals, Transfer, UnTame
   - grey/non-combatant guard: `IsNonCombatantPet(me)` refuses
     ATTACK/GUARD and never enters Fight() (engine half done in P2)
   Anything identified as dead (commented blocks, unreachable branches,
   unused helpers/props) is left behind, not carried over. Document the
   audit (live vs dropped) in this file before writing tamedBrain.src.
   `program TamedBrain(npc)` with `me := npc`.
6. **Lazy migration.** Prepend a shim to `ghaia/tamed.src` `program
   TamedAI()`: if `#brain_v < 2` → set it, `me.script := ":epicAI:brain"`,
   `RestartScript(me)`, return. Existing pets self-migrate on next wake;
   `ghaia/tamed.src` body becomes dead, physically deleted in a later
   cleanup phase once telemetry shows zero on the old version.
7. **New-tame repoint.** Change the 5 assignment sites' `":ghaia:tamed"`
   → `":epicAI:brain"` (owner/master/props already set there, so dispatch
   classifies them as tamed).

## Sub-phases

- **P4a — Identity decoupling (no behavior change).** Add `IsTamedPet()`
  to `taming.inc`. Replace every *identity* `script == ":ghaia:tamed"` /
  `script[":ghaia:tamed"]` read with `IsTamedPet()` across consumers
  (animalLore, taming.src, acalmar, herd, happyCounter, petfiel, + grep
  sweep). Predicate is true for today's pets → zero behavior change.
  Likely 6–8 files → done in two ≤5-file batches (P4a-1, P4a-2), each
  committed. Verify: pets still inspectable/herdable/feedable as before.

- **P4b — Dead-code audit + TamedBrain build + dispatch.** First:
  audit `ghaia/tamed.src`, write the live-vs-dropped list into this
  plan file (commit the audit). Then create `tamedBrain.src`
  (live-only reimplementation + grey guards), add `brain.src` dispatch
  (`IsTamedPet` → `"tamed"` → `Run_Script`), update `happyCounter.src`
  liveness to `IsTamedPet`. ≤5 files. Verify on a TEST creature manually
  scripted to `:epicAI:brain`: follow/attack/guard/stop, feed/bond,
  mount, transfer, grey pet refuses attack & flees.

- **P4c — Repoint + lazy shim.** Repoint the 5 new-tame assignment sites
  → `:epicAI:brain`. Add the lazy shim to `ghaia/tamed.src`. ≤5 files.
  Verify: newly tamed creature runs new brain; a pre-existing
  `:ghaia:tamed` pet auto-migrates on next wake and behaves identically.

Phase 5 (separate) = ControlledBrain for summon/necro/mage/merchant/
controlarmortovivo. Legacy file deletion = later cleanup phase.

## Risks / call-outs

- **Lifecycle (4).** DECIDED: permanent ghaia-style loop, no regression,
  no idle-CPU win. Settled — not revisited.
- 1050-line port: bugs likely; P4b verification must be hands-on.
- `IsTamedPet` must exclude summons/controlled (they have master too) via
  the `!summoned` + `"owner"` checks; Phase 5 controlled stays on
  `:ghaia:controlled` until its own phase, so P4 must not reroute them.
- grep sweep for any `:ghaia:tamed` identity test missed by the list above
  is mandatory before P4c (No-Semantic-Search rule).
