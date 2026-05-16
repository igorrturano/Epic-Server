# Spec: Civic Taming vs Military Beastmastery + Pet-Brain Consolidation

Status: APPROVED DESIGN — pending implementation go-ahead. Phased build.
Owner: Igor. Branch base: `feat/weapons-rebalance` (new branch for this epic).

## 1. Locked design decisions

### 1.1 Skill scope split
- **Taming (CIVIC)** — peaceful/bond: tame, bond/fidelizar, general obedience
  (pare/siga/corra/ande/novo dono), pet-slot economy, **animal lore /
  inspection**, **acalmar (calm hostile)**, herding, pet-heal aux, breeding,
  harvest_honey.
- **Beastmastery (MILITARY)** — combat only: `obedienceCheckCombat`
  (attack/guard). Cacador scaling DEFERRED to a follow-up spec.
- Move from Beastmastery → Taming: `animalLore.src:49,64,80`,
  `acalmar.src:56,59`.
- Keep the existing bridge verbatim: `EffectiveSkill` / `EffectiveTrueSkill`
  (`attributes.inc:307-360`), `ResolveSkillTier` (`skills.inc:499-515`).
  Do NOT touch Handle_Animal legacy-attribute removal (that is Phase 6, later).

### 1.2 Pet economy
- **War pet** = owned creature with `mobile.alignment == 1`. Cap = **1** for
  every player. `+1` only if owner has the Cacador Camouflage ability
  (minimal read on the *existing* ability ID; no rename/effect change — that
  is deferred).
- **Grey pet** = owned creature with `mobile.alignment == 0` (e.g. bird),
  *even if tamed*. Non-combatant: refuses combat commands, force-flees,
  contributes 0 threat, never targeted by hostile AI. Slot pool =
  `floor(EffectiveSkill(who,"Taming") / 20)`.
- **Mounts** = always exempt. A creature serving as a mount consumes neither
  the war nor the grey pool, regardless of alignment. (Mirrors today's
  free-slot-while-ridden.)
- Over-cap legacy pets: **grandfather**. The cap blocks only NEW
  tame/transfer/resurrect. Never force-remove an existing pet.

### 1.3 Brains
- Two new epicai brains, both **`Run_Script`'d** (respect the 64KB brain data
  segment, per `feedback_brain_data_segment`):
  - **TamedBrain** — owned animals (war + grey).
  - **ControlledBrain** — summons / undead thralls / merchant NPCs.
- Dispatcher: add two cases at `brain.src:54,61-67`.
- Port the pet-command event protocol from `ghaia/tamed.src`:
  `FOLLOW_CMD 0xA001`, `ATTACK_CMD 0xA002`, `NEW_MASTER_CMD 0xA003`,
  `GUARD_CMD 0xA004`, `GO_LOC_CMD 0xA008`; speech parsing; obedience
  (Taming general / Beastmastery combat); happiness/bond; `TakeItem`.
- Grey pets in TamedBrain: drop combat commands, force flee.
- **Lazy migration**: legacy `:ghaia:tamed`, `:ghaia:controlled`,
  `:brainai:tamed` get a thin shim at the top that, on first tick/event,
  repoints `.script` to the new epicai brain, sets `#brain_version`, and
  `RestartScript`. Legacy files remain as shims until all pets have cycled;
  physical deletion is a later cleanup phase gated on telemetry.

### 1.4 Aggro / threat
- New shared predicate `IsNonCombatantPet(creature)` = owned/tamed AND
  (`alignment == 0` OR currently a mount). One shared include consumed by
  both combat and threat code.
- `CanFight` (`epicNPCCombat.inc:556`): hostile AI never selects a
  non-combatant pet as a target.
- `AddThreat` (`threatModel.inc:52`): a non-combatant pet's hits add 0
  threat (no aggro pull onto it).
- `ShouldFlee` (`epicNPCCombat.inc:1985`): returns 1 for non-combatant pet.

### 1.5 Crook + Provisioning
- `herd.src` movement/relocate branch (`167-195`): gate on
  `EffectiveSkill(who,"Provisioning")` instead of Taming, restricted to
  grey/mount targets. Keep the SHEPHERD ability gate. Breeding /
  `Engravidar` / Taming-follow remain on Taming, untouched.

### 1.6 Explicitly out of scope (separate later specs)
Full Beastmastery→Cacador scaling; Camouflage rename/effect rework; Phase 6
Handle_Animal attribute deletion; physical deletion of legacy brain files.

## 2. Key file map (verified)
- Bridge: `attributes.inc:307-360`, `skills.inc:499-515`,
  `skillCheck.inc:52-55`. Buckets: `epicConstants.inc:83-93`.
- Skill sites: `animalLore.src:49,64,80,175`, `acalmar.src:56,59`,
  `tamed.src:864/925`, `taming.inc:213-249`, `healing.src:191`,
  `herd.src:167-195,245`, `harvest_honey.src:70`, `controlled.src:283`.
- Brain dispatch: `brain.src:54,61-67`. Legacy: `ghaia/tamed.src`,
  `ghaia/controlled.src`, `brainAI/tamed.src`.
- Assignment chokepoints: `taming.src:254`, `taming.inc:380` (RessuscitarPet),
  `summon.inc:31`, `controlarmortovivo.src:20`, `merchant_menu.src:453`,
  `adestrar.src:175`, `taming/textcmd/coun/tame.src:18`, 5 necro spell files.
- Economy callers: assoviar.src, stablepost.src, openPaperdoll.src,
  animallore/logon.src, taming.src:195, tamed.src:647, controlled.src:492.
- Aggro: `epicNPCCombat.inc` (CanFight 556, ShouldFlee 1985),
  `threatModel.inc:52` (AddThreat).

## 3. Phase plan (≤5 files/phase, commit per phase, one-line msgs)

- **P1 Skill scope** — `animalLore.src`, `acalmar.src`. Beastmastery→Taming.
- **P2 Non-combatant classifier + AI** — new `petClassify` include;
  `epicNPCCombat.inc` (CanFight, ShouldFlee); `threatModel.inc` (AddThreat).
- **P3 Pet economy** — `taming.inc` (war/grey budgets, mount exempt),
  tame-gate in `taming.src`/`adestrar.src`, display in
  `animallore/logon.src`/`openPaperdoll.src`. Grandfather = gate-only.
- **P4 TamedBrain** (sub-phased) — new tamedBrain `Run_Script`;
  `brain.src` dispatcher; shim in `ghaia/tamed.src`; repoint
  `taming.src:254` + `taming.inc:380`.
- **P5 ControlledBrain** (sub-phased) — new controlledBrain;
  dispatcher case; repoint `summon.inc:31`, `controlarmortovivo.src:20`,
  `merchant_menu.src:453`, necro spells; shims in `ghaia/controlled.src`,
  `brainAI/tamed.src`.
- **P6 Crook→Provisioning** — `herd.src` movement branch only.

Verification each phase: user compiles (no ecompile in bash); in-game smoke
test the touched path; commit.
