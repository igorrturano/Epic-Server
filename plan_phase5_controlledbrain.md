# Phase 5 Plan — ControlledBrain consolidation

Branch: `feat/taming-beastmastery-petbrain` (worktree). Build after audit review.
Commit per sub-phase, ≤5 files/sub-phase, user compiles between.

## Decisions (locked)
- **Independent live-only `controlledBrain.src`** — own attached script in
  pkg/mobiles/epicAI/, same pattern as tamedBrain.src. Accept ~loop
  duplication with TamedBrain; a DRY-merge is a possible LATER refactor
  once both compile-verify. Do NOT re-architect the unverified Phase 4
  tamedBrain.src now.
- **Full consolidation** — repoint ALL: 5 flagged-summon spells
  (summon.inc:31, invocarespirito:70, montarianefasta:71,
  cimitarravoadora:100, montariaarcana:79) + 2 `:ghaia:controlled`
  producers (controlarmortovivo:20, merchant_menu:453) →
  `:epicai:controlledBrain`. Lazy shims on BOTH `ghaia/controlled.src`
  (always migrate) AND the summoned-branch of `ghaia/tamed.src` (currently
  stays legacy → change to migrate summoned → `:epicai:controlledBrain`).
  End state: ghaia/tamed.src + ghaia/controlled.src both fully dead.

## Key research facts
- `:ghaia:controlled` producers: only controlarmortovivo.src:20 +
  merchant_menu.src:453. Both save orig script in `"script"` prop.
- `ghaia/controlled.src` (1220 LOC, `program TamedAI()`): NO
  obediencecheck / obedienceCheckCombat / bond / feeding-bond. Controlled
  creatures **auto-obey unconditionally**. = TamedBrain minus
  obedience/bond/feed, plus auto-obey + ~490 LOC extra pathfinding
  (CalcPathAndMove, CloseDistance, ChooseAnotherOpponent, Opendoors,
  PetMovement, PetGoTo, getFacingString).
- Summon expiry/cleanup is EXTERNAL (timed-scripts + ErasePet/SummonedPets
  in taming.inc). ControlledBrain does NOT implement timers.
- Economy: controlled/summon creatures are NOT in `TamedPets` (no AddPet);
  summons use `SummonedPets`/GetMaxSummons. So `_TallyPets`/`CanAcquirePet`
  (P3) do not count them — no collision. (Verify in P5b.)
- `IsNonCombatantPet` (P2) already returns 0 for summons (summoned flag)
  and evil-alignment thralls → controlled creatures fight normally; no
  grey-guard needed in ControlledBrain.
- `IsTamedPet` (petidentity.inc): a `:epicai:controlledBrain` creature is
  neither `:ghaia:tamed` nor `:epicai:tamedBrain` → returns 0 already.
  Correct: controlled creatures are NOT tamed pets. Legacy
  `[":ghaia:tamed"]` branch becomes vestigial post-migration (kept until
  cleanup).

## Sub-phases
- **P5a — Dead-code audit of ghaia/controlled.src.** Read full file,
  classify LIVE / ADAPT / DROP into this plan file, commit. PAUSE for
  review before building (mirrors P4b).
- **P5b — Build controlledBrain.src + petidentity check.** Live-only
  attached-script port: `program ControlledBrain()`, `me := self()`,
  auto-obey (no obedience/bond/feed), keep needed pathfinding, UnTame →
  summons `.kill()` else restore `:epicai:brain`. Verify no economy
  collision. ≤5 files. PAUSE for compile.
- **P5c — Repoint producers + lazy shims + final sweep.**
  - 7 producers → `:epicai:controlledBrain`.
  - `ghaia/controlled.src`: shim at top of program → migrate all to
    `:epicai:controlledBrain`, RestartScript, return.
  - `ghaia/tamed.src`: change the summoned-branch (currently falls
    through to legacy) → migrate summoned → `:epicai:controlledBrain`.
    Net: not-summoned→tamedBrain, summoned→controlledBrain; ghaia/tamed
    body fully dead.
  - Final grep sweep: zero `:ghaia:tamed` / `:ghaia:controlled`
    producers or identity tests remain (except vestigial petidentity
    legacy branch). ≤5 files (may split P5c-1/P5c-2).

Legacy file deletion (ghaia/tamed.src, ghaia/controlled.src,
brainAI/tamed.src, makeswarm) = separate later cleanup phase, gated on
telemetry showing zero creatures on the old scripts.

## P5a dead-code audit of ghaia/controlled.src (1220 LOC)

`controlledBrain.src` reimplements ONLY the LIVE set.

### LIVE — port
- Main loop (41-184): event dispatch + simplified state machine.
  **No obediencecheck anywhere — controlled creatures auto-obey.**
- ProcessSpeech (186-313): commands, auto-obey (zero obedience calls).
- TakeItem (315-349): minimal — "eat" cfg → happiness 100; pack mochila.
  (Far simpler than tamed; no bond/diet.)
- Fight / IsDead / OpenPack (351-389).
- Follow (391-434) → uses `CloseDistance()`.
- Guard (436-456); goToLoc (458-484) → uses `PetMovement()`.
- Transfer (486-511): already on `CanAcquirePet` (P3); simpler (no bond).
- AllCommand (549-566), ProcessDoubleClick (568-643, pack gump + mount +
  guard/follow/attack gump + go-loc), EnableMainEvents (645-653),
  Setup (664-701).
- **Pathfinding block (731-1220): LIVE** — CalcPathAndMove, CloseDistance,
  ChooseAnotherOpponent, Opendoors, PetMovement, PetGoTo,
  getFacingString. Follow/goToLoc depend on it. Port faithfully.

### LIVE + ADAPT
- `program ControlledBrain()`, `me := self()` (attached script).
- UnTame (513-547): drop legacy `orig_script`/`"script"`/`:brainai:brain`
  dance → `me.script := ":epicai:brain"`; if summoned → `.kill()`.
  NOTE prop inconsistency: controlled.src checks `"summon"` (271,299)
  but the necro/mage producers set `"summoned"` — UnTame/kill must check
  BOTH flags.
- SpecialHab/CastFaerieHeal (703-729): `:brainai:fadinha` → `:epicai:`
  dual-check; TakeItem Horse3/Llama `:brainai:` → `:epicai:` dual-check.

### DROP — dead/unused
1. `mountables` var (19) — unused (`IsMountable()` builtin used).
2. Commented EVID_FLEE (57-60).
3. Empty SYSEVENT_ENGAGED case (66) — no-op.
4. happiness<10 branch (135-152) — body is ALL comments → dead; drop
   the branch entirely.
5. Commented DOUBLECLICKED pack/mount block (104-125).
6. Commented "Dear…shut up" (194).
7. `companheiro` guards (276-279, 304-307) — dead prop (user-confirmed
   drop in P4).
8. DisableMainEvents (655-662) — never called.
9. Commented happycounter in Setup (692-696); commented snippets in
   TakeItem (340-344).

NOTE: ControlledBrain has **no happyCounter, no bond, no obediencecheck,
no IsCombatPet/grey-guard** (controlled units are combat thralls/summons;
P2 `IsNonCombatantPet` already returns 0 for them). Leaner than TamedBrain.

## Risks / call-outs
- 1220-LOC file: heavy pathfinding block — audit must decide if it's live
  (used by current thralls) or dead bloat; user directive = live-only.
- Same Run_Script-vs-attached lesson as P4: ControlledBrain is the
  ATTACHED script (events go to it), not Run_Script'd, not a brain.src
  dispatch case.
- Transient during migration: a flagged-summon still on `:ghaia:tamed`
  → IsTamedPet returns 1 (legacy branch, pre-existing accepted behavior);
  after shim migrates it → IsTamedPet 0 (correct). Acceptable.
