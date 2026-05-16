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
2. **Brain home (CORRECTED — Option B).** New
   `pkg/mobiles/epicAI/tamedBrain.src`, a standalone **attached** script
   `program TamedBrain()` with `me := self()` (like ghaia/tamed.src is
   today, relocated+modernized into the epicAI pkg). Its own compile
   unit/segment → no 64KB risk; free to include
   `:taming:taming`/`:mounts:mounts`/`:gumps:gumps`. Referenced as
   `":epicai:tamedBrain"` (lowercase, matching npcdesc `:epicai:brain`
   convention; eScript substring matching is case-sensitive so the token
   must be exactly this everywhere).
3. **Dispatch — NONE.** `brain.src` is NOT touched. Run_Script from the
   dispatcher was rejected: object events are delivered to the *attached*
   script, not a Run_Script child, so an event-loop brain must BE the
   attached script. Pets get `me.script := ":epicai:tamedBrain"` at the
   assignment sites / lazy shim and never run the monster brain. P2's
   epicNPCCombat/threatModel predicates still apply (wild NPCs evaluate
   the pet object regardless of the pet's own brain).
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
   → `":epicai:tamedBrain"`. `petidentity.inc` migrated-branch corrected
   from the P4a placeholder `:epicAI:brain` → `:epicai:tamedBrain`.

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

Phase 5 (separate) = ControlledBrain for the FLAGGED-summoned set only:
summon.inc, invocarespirito, montarianefasta, cimitarravoadora,
montariaarcana (all set `"summoned",1`), plus merchant/controlarmortovivo
(:ghaia:controlled) + brainAI makeswarm GM tool. NOTE: raiseundead &
levantarmortos undead do NOT set `"summoned"` — they are pet-like and
already migrate to :epicai:tamedBrain via the P4c shim, so Phase 5 does
NOT need to cover them. Legacy file deletion = later cleanup phase.

## Risks / call-outs

- **Lifecycle (4).** DECIDED: permanent ghaia-style loop, no regression,
  no idle-CPU win. Settled — not revisited.
- 1050-line port: bugs likely; P4b verification must be hands-on.
- `IsTamedPet` must exclude summons/controlled (they have master too) via
  the `!summoned` + `"owner"` checks; Phase 5 controlled stays on
  `:ghaia:controlled` until its own phase, so P4 must not reroute them.
- grep sweep for any `:ghaia:tamed` identity test missed by the list above
  is mandatory before P4c (No-Semantic-Search rule).

## P4b dead-code audit of ghaia/tamed.src (1050 LOC)

Reviewed every function/block. `tamedBrain.src` reimplements ONLY the
LIVE set; DROP set is left behind.

### LIVE — port into tamedBrain.src
- **Main loop** (TamedAI 44-198): event dispatch + state machine
  (opponent→Fight+Follow / following→Follow+look_around / guarding→Guard
  / going_loc→goToLoc) + happiness<10 warning.
- **ProcessSpeech** (211-371): command parsing. Obedience split already
  correct (combat cmds→obedienceCheckCombat[Beastmastery],
  rest→obediencecheck[Taming]).
- **TakeItem** (373-499): feeding + bond/fidelizar + pack-mochila. (Core
  per user: "feeding".)
- **Fight/IsDead/Follow/Guard/goToLoc** (501-634): movement/combat state.
- **Transfer** (636-669): already on `CanAcquirePet` (P3b).
- **AllCommand** (714-731): "todos" broadcast.
- **ProcessDoubleClick** (733-798): gump (guard/follow/attack), pack
  remove, go-to-loc, web `EstimulaAbdomen`.
- **EnableMainEvents** (800-808), **Setup** (819-862),
  **obediencecheck** (864-918, Taming), **obedienceCheckCombat**
  (925-974, Beastmastery), **GetHappy** (1004-1028), **Go** (1030-1050).
- **State/consts**: pack_animals var, custom event consts,
  guarding/following/fighting/opponent/going_loc, owner/owner_serial,
  `SCRIPTOPT_CAN_ACCESS_OFFLINE_MOBILES` (lifecycle = permanent loop),
  `look_around` (200-209).
- **happyCounter.src** sub-script kept (liveness already on IsTamedPet
  from P4a).

### LIVE + ADAPT (behavior change required)
- **Fight()/Guard()**: add `IsNonCombatantPet(me)` guard — grey pet
  refuses to enter Fight() and Guard() never sets opponent (command-side
  half of P2; engine half already blocks targeting/threat/forces flee).
- **UnTame** (671-712): drop the legacy `orig_script` / `"script"` prop /
  `:brainai:brain` fallback dance → restore cleanly to `":epicAI:brain"`
  (wild epicai brain), clear pet props, RestartScript.
- **Program signature**: `program TamedBrain(npc)` invoked via
  `Run_Script` from brain.src; `me := npc` not `self()`.

### DROP — dead/unused, do NOT carry over
1. Commented `EVID_FLEE` block (84-87).
2. `mountables` var (21) — unused; code uses `IsMountable()` builtin.
3. Commented inline happycounter (65-68).
4. Commented "Dear…shut up" (219).
5. `companheiro` guards (334-337, 362-365) — prop NEVER set anywhere
   (confirmed P3 research); dead scaffold.
6. **DisableMainEvents** (810-817) — never called.
7. Commented happycounter block in Setup (847-851).
8. Commented debug prints in obediencecheck (889-890).

### KEEP but FLAG (legacy refs to clean while porting)
- `SpecialHab`/`CastFaerieHeal` (976-1002): faerie-heal, gated on
  `npctemplate == ":brainai:fadinha"` — port, fix namespace to
  `:epicai:` per `[[feedback_npc_namespace]]`.
- Loop line 143 `me.npctemplate == ":brainai:Unicorn"` — same namespace
  fix.
- `defecador` / sheep-wool (Setup 856-860; loop 76-80): livestock realism
  — keep for parity, low risk.
- TakeItem fidelizado cap uses `ESPREITA_SELVAGEM` (444-445): this is the
  "faithful animals" limit, SEPARATE from P3 war/nonwar slots. Keep as-is
  per user ("faithful pet"); reconciling with P3 economy is OUT of P4
  scope — flagged for a possible later spec.

Result: ~1050 LOC → est. ~750 LOC of live logic, minus the 8 DROP items.
