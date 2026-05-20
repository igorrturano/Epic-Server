# Ability Pipeline — Agent Reference

Canonical, code-grounded map of the V3 ability system: how an ability goes
from a config row + a class file to an effect that fires at a combat
moment, and how the activation lifecycle (hotbar press → window → expire)
works. Companion to [`combat-pipeline.md`](combat-pipeline.md) — that doc
says *where* in a swing each moment fires; this doc says *how* a fired
moment resolves to a class method, and how abilities get activated in the
first place.

Everything here was traced firsthand to the cited `file:line` against the
**live code** (the old `combat-guide` skill was stale — e.g. it referenced
an `AbilityRegistryBase` that does not exist). **If you change ability
dispatch, activation, or the class contract, update this doc.**

## TL;DR

- An ability is **one class file** under
  `pkg/systems/abilities/include/classes/<archetype>/<Name>.inc`,
  subclassing `CombatAbility`, plus **one config row** in
  `configs/habilidades` (name/cost/cooldown/icon — *not* behavior).
- Two completely separate trigger paths feed the same class:
  1. **Activation path** (player presses a hotbar ability): validates
     cost/cooldown, consumes them, starts `AbilityMonitor.src` which holds
     the ability "active" for a duration window.
  2. **Combat-moment path** (a swing happens): `CombatAbilityController`
     bridges the moment to the ability's hook method.
- **Active** hooks (`OnHit`, `OnBeforeDamage`, …) only fire if the ability
  is currently **activated** (in the `ACTIVE_ABILITIES` prop, within its
  window). **Passive** hooks (`OnPassive*`) fire just from **owning** the
  ability (in the `ABILITIES` prop) — no activation needed.
- Everything crosses `Start_Script`/`run_script_to_completion`
  boundaries, so: string-action dispatch, **struct** returns (never class
  instances), and re-wrap the mobile on the far side. This constraint
  shapes the entire architecture.
- Archetype is derived from the ID range; each archetype has its own
  registry + executor script. Combat code only includes the **lightweight**
  `AbilityRegistry.inc` and dispatches across a script boundary, keeping
  the heavy class graph out of the combat data segment.

## File map

| Concern | File | Role |
|---|---|---|
| Interface | `include/models/IAbility.inc` | `uninit` method contract |
| Base | `include/models/Ability.inc` | Metadata from cfg + `ABILITY_ACTIVATION` + `ABILITY_ID` enums + lifecycle stubs |
| Combat base | `include/models/CombatAbility.inc` | All combat-moment hooks + `ExecuteOn` router + `SingleUse`/`IsDefenderSide` |
| Concrete | `include/classes/<archetype>/<Name>.inc` | One ability; overrides hooks |
| Registry (light) | `include/AbilityRegistry.inc` | `ABILITY_CLASSES` dict + `GetAbilityArchetype` + `CreateAbility`. No class includes. |
| Registry (archetype) | `include/AbilityRegistry{Guerreiro,Ladino,Sabio,Caminho}.inc` | Includes class files, populates `ABILITY_CLASSES[ID] := @Class` |
| Executor | `AbilityExecutor{Archetype}.src` | `return AbilityExecutionController().Run(params)` |
| Service | `include/AbilityService.inc` | `AbilityExecutionController`: the string-action boundary |
| Combat router | `AbilityController.src` | Thin: derive archetype → run executor (used by combat path) |
| Monitor driver | `AbilityMonitor.src` | Activation lifecycle program (used by activation path) |
| Monitor model | `include/controllers/AbilityMonitor.inc` | `AbilityMonitor` class + `ABILITY_EVENT_ID` |
| Combat bridge | `:combat:Controller/CombatAbilityController.inc` | Maps combat moments → executor calls |
| Hotbar entry | `:hotbar:include/hotbar_actions.inc` | `ExecuteAbility()` — activation entry point |
| Owned state | `MobileWrapper.inc:704` `GetAbilities` | `ABILITIES` prop: level→id (what you *own*) |
| Active state | `MobileWrapper.inc:834` `GetActiveAbilities` | `ACTIVE_ABILITIES` prop: moment→{id→serialized monitor} |

## The class contract

`Ability` (`Ability.inc:27`) reads **only metadata** from
`CFGFile("abilities","configs/habilidades")` keyed by the `ABILITY_ID`
enum value — name, class, folk, description, requirement, HP/STAM/MANA
cost, cooldown (`delay`), icon (`:46-70`). Behavior is **never** in config;
it lives in the subclass.

`CombatAbility` (`CombatAbility.inc:5`) adds:

- `SingleUse` (default `false`) — one-shot window abilities (`:7`)
- `IsDefenderSide` (declared on `Ability:44`, default `0`) — passive side
  gate
- All combat hooks, defaulting to `return false` (no-op):
  - **Active** (`:38-83`): `OnBeforeHit` (defender), `OnMiss` (defender),
    `OnHit`, `OnBeforeDamage`, `OnAfterDamage`, `OnCritical`, `OnKill`
    (attacker), `OnParry` (defender)
  - **Passive** (`:89-123`): `OnPassiveBeforeDamage`, `OnPassiveOnKill`,
    `OnPassiveOnParry`, `OnPassiveAfterDamage`, `OnPassiveOnMiss`,
    `OnPassiveBeforeHitAttacker`, `OnPassiveBeforeHitDefender`,
    `OnPassiveOnEquip`, `OnPassiveOnUnequip`
- `ExecuteOn(attackInfo)` (`:129-152`) — the **active** router: a `case`
  on `this.ActivationMoment` that dispatches to the one matching `On*`
  hook. Passives do **not** go through `ExecuteOn`; they're called by name
  in `AbilityService.RunPassive`.

Canonical example — `combatente/MurderBlow.inc` (Golpe Mortal, id 201):

```escript
class MurderBlow(CombatAbility)
    function MurderBlow(this)
        Super(ABILITY_ID::GOLPE_MORTAL);              // metadata from cfg
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
        this.SingleUse := true;                       // one-shot window
    endfunction

    function GetDurationFor(this, mobile)             // the activation window
        var d := CInt(mobile.GetSkill(TACTICS) / 10);
        return d < 2 ? 2 : d;
    endfunction

    function OnBeforeDamage(this, attackInfo)         // ACTIVE hook
        attackInfo.SetIgnoreArmor(true); ...          // (read by CalculateDefenderResist)
    endfunction

    function OnPassiveOnKill(this, attackInfo)        // PASSIVE hook (always-on)
        ... attacker.Attack(nextTarget); ...
    endfunction
endclass
```

`MurderBlow.inc:1-109`. Note one class can carry **both** an active and a
passive hook; they're driven independently by the two paths below.

### `ABILITY_ACTIVATION` moments (`Ability.inc:9-25`)

`OnMiss`, `BeforeHit`, `BeforeHitDefender`, `BeforeDamage`, `OnHit`,
`OnCritical`, `AfterDamage`, `OnKill`, `OnParry`, `OnUse`, `OnEquip`,
`OnUnequip`. `ON_USE` is the default for `CombatAbility`
(`CombatAbility.inc:15`); player-pressed abilities use it.

### `ABILITY_ID` ranges → archetype (`AbilityRegistry.inc:11-21`)

`GetAbilityArchetype(id)`: `<1000` → **Guerreiro**, `<2000` → **Ladino**,
`<3000` → **Sabio**, else → **Caminho**. The full numbered enum (every
ability, by sub-path) is `Ability.inc:152-457` — read it there, it's the
live source.

## Registration

`AbilityRegistry{Archetype}.inc` includes every class file then maps the
enum to the class constructor:

```escript
include ":abilities:classes/combatente/MurderBlow";
...
ABILITY_CLASSES[ABILITY_ID::GOLPE_MORTAL] := @MurderBlow;
```

(`AbilityRegistryGuerreiro.inc:1-166`, 50 abilities). `CreateAbility(id)`
/ `_InstantiateAbility(id)` do `ABILITY_CLASSES[id].new()` and error if the
id isn't registered. **A class file with no `ABILITY_CLASSES[...] := @X`
line is dead** — nothing can instantiate it.

`AbilityExecutor{Archetype}.src` is three lines: include the archetype
registry + `AbilityService`, then `return AbilityExecutionController().Run(params)`
(`AbilityExecutorGuerreiro.src:1-9`). This is the only place the heavy
class graph is loaded — behind a script boundary, so combat's data segment
stays small.

## Path 1 — Activation (hotbar press → window)

```
hotbar_actions.inc::ExecuteAbility(who, actionId)              (:104)
  GetAbilityBySequentialId → abilityId
  Ability(abilityId)               # metadata only, base class (:113)
  AbilityMonitor(ability).ActivateOn(MobileWrapper(who))       (:119-120)

AbilityMonitor.inc::ActivateOn(mobile)                          (:60)
  validate STAM / MANA / HP / cooldown  → error struct on fail  (:74-96)
  if not GM (cmdlevel): consume costs + SetAbilityCooldown      (:98-114)
  this.Duration := ability.GetDurationFor(mobile)               (:116)
  if a monitor already active for this ability:
      existingMonitor._GetProcess().SendEvent(REFRESH)          (:124)
  else:
      Start_Script(AbilityMonitor.src, {mobile, this})          (:126)

AbilityMonitor.src::AbilityMonitor(params)                      (:18)
  # mobile & monitor came via Start_Script: FIELDS ok, METHODS broken
  archetype := GetAbilityArchetype(id);  executor := AbilityExecutor{archetype}
  shouldRunAsyncOnUse := executor "query_is_on_use"             (:37)
  if async (ON_USE w/ client targeting):
      run_script(executor, "activate_async", ...)  → return     (:40-44)
  result := executor "activate"                                 (:47)
      → AbilityService.Activate (:137):
          re-wrap MobileWrapper(mobile.ObjRef)   # restore methods
          _InstantiateAbility → CanActivate(mobile)  (false ⇒ error)
          mobile.ActivateAbility(ability, monitor)   # serialize monitor
                                                     # into ACTIVE_ABILITIES
          ability.OnActivate(mobile)
          return struct{duration, name}
  if result.hasError: MobileWrapper(...).RollbackAbilityActivation; return
  # Phase 2 — event loop, no ability instance held
  while remaining > 0: event := Wait_For_Event(remaining)       (:64-80)
      ABORT   → "cancelada", break
      COMPLETE→ break          (single-use hit landed; see Path 2)
      REFRESH → reset endTime
      EXPIRE / falsy → break
  # Phase 3
  executor "deactivate"                                         (:83)
      → AbilityService.Deactivate → mobile.DeactivateAbility
          → ability.OnDeactivate(mobile) + erase from ACTIVE_ABILITIES
```

Key points:

- **Costs/cooldown are charged at activation** (`AbilityMonitor.inc`), not
  when the hook fires. GMs (`cmdlevel`) skip consumption (`:98`).
- `GetDurationFor(mobile)` is the **activation window**, not the effect
  duration of a buff. For `SingleUse` abilities it's the time you have to
  land the hit. Base `Ability.GetDurationFor` returns `GetDuration()`
  (the `EffectDuration` var, default 0); `AbilityService.Activate` floors
  it to `1` if `<= 0` (`AbilityService.inc:160-166`) — **so a SingleUse
  ability that forgets to override `GetDurationFor` gets a 1-second window
  and expires before the hit lands.**
- Re-activating a live ability sends `REFRESH` (resets the window) instead
  of stacking a second monitor (`AbilityMonitor.inc:122-127`).
- The monitor is **serialized into an ObjProperty** keyed by
  `activationMoment → abilityId` (`MobileWrapper.ActivateAbility:890`,
  read back + stale-PID cleanup in `GetAbilityMonitor:854`).

## Path 2 — Combat moment (a swing fires a hook)

The combat pipeline calls a bridge function in
`:combat:Controller/CombatAbilityController.inc` at each moment (see the
moment table in [`combat-pipeline.md`](combat-pipeline.md) for exact
`file:line` of each call site). Two dispatchers:

**Active** — `_ExecuteCombatAbilities(mobile, moment, attackInfo)`
(`CombatAbilityController.inc:54`):

```
abilitiesList := mobile.GetActiveAbilities()[moment]   # only ACTIVATED abilities
foreach: run_script_to_completion(:abilities:AbilityController,
                                  {"active", abilityId, attackInfo})
  AbilityController.src → AbilityExecutor{archetype} → AbilityService.RunActive (:74)
    _InstantiateAbility(id).new()                # fresh instance per hit
    executed := ability.ExecuteOn(attackInfo)    # case → the moment's On* hook
    return struct{executed, singleUse}
if executed && singleUse && monitor.Pid:
    GetProcess(Pid).SendEvent(COMPLETE)          # ends the Path-1 window
```

**Passive** — `_ExecutePassiveAbilities(mobile, moment, attackInfo)`
(`:129`):

```
abilities := mobile.GetAbilities()              # OWNED (level→id), no activation
dispatchedToDefender := (mobile.serial != attackInfo.attacker.serial)
foreach: run_script_to_completion(:abilities:AbilityController,
            {"passive", abilityId, attackInfo, moment, dispatchedToDefender})
  → AbilityService.RunPassive (:85)
    side gate: if CInt(ability.IsDefenderSide) != dispatchedToDefender → skip
    case(moment) → ability.OnPassive<Moment>(attackInfo)
```

Key points:

- **Active fires only if activated**: `GetActiveAbilities()` reads the
  `ACTIVE_ABILITIES` prop populated by Path 1. No activation ⇒ the active
  hook never runs, even though the player owns the ability.
- **Passive fires from ownership**: `GetAbilities()` is the leveled-up
  owned list. The hook runs on every relevant swing regardless of
  activation. The **side gate** (`IsDefenderSide` vs `dispatchedToDefender`,
  `AbilityService.inc:96`) stops a defender-side passive from running while
  the owner is the attacker (and vice-versa) — set `this.IsDefenderSide := 1`
  in the constructor for defender passives.
- **SingleUse completion**: when an active SingleUse hook returns truthy,
  the controller sends `COMPLETE` to the monitor process, which breaks the
  Path-1 loop and runs deactivate. This is how a one-shot window closes the
  instant the hit lands rather than on timeout.
- A **fresh ability instance is built per hit** (`_InstantiateAbility`);
  never store per-swing state on the instance — it won't survive to the
  next swing. Persist on the mobile (props) or `attackInfo`.

## Cross-script boundary rules (why the architecture looks like this)

eScript **class instances lose their method tables** when passed through
`Start_Script` / `run_script_to_completion` — fields survive, method
dispatch silently crashes (`AbilityMonitor.src:11-17` documents this
explicitly). Consequences baked into every layer:

- Dispatch is by **string action** (`"active"`, `"passive"`, `"activate"`,
  `"activate_async"`, `"deactivate"`, `"query_is_on_use"`) — see
  `AbilityService.Run` (`:39-68`) — because a `Start_Script` entry can't
  call a method directly.
- Cross-boundary returns are **structs**, never class instances —
  `_ResultExecuted` / `_ResultActivationError` / `_ResultActivationSuccess`
  (`AbilityService.inc:230-251`).
- The mobile is **re-wrapped** (`MobileWrapper(mobile.ObjRef)`) on the far
  side before any method call (`AbilityService.inc:139`, `:173`, `:193`).
- The monitor is **serialized** into an ObjProperty (StructSerializer),
  not held as a live object, and re-deserialized + stale-PID-checked on
  read (`MobileWrapper.inc:864-874`).

Do not "simplify" by passing a wrapper or ability instance across a script
boundary and calling methods on it — that is the single most common way to
silently break this system.

## Connection to combat-pipeline.md

| `combat-pipeline.md` gives you | `ability-pipeline.md` (this) gives you |
|---|---|
| The exact swing position + `file:line` where each moment's bridge function is called | What that bridge call does: archetype routing → executor → `ExecuteOn`/`OnPassive*` → your class method |
| `requestAttack.src:77` `ENGAGING` (mastery path) | The V3 dispatch machinery for `OnUse`/combat moments |
| The damage object the hook receives (`attackInfo`, posture pitfall) | That a fresh ability instance is built per hit; persist via `attackInfo`/props |

Spell damage also reaches these hooks:
[`spells-pipeline.md`](spells-pipeline.md)'s `SC_DealDamage` builds a
spell-origin `AttackInfo` and fires the `AFTER_DAMAGE`/`ON_KILL`
dispatchers documented here.

Workflow: deciding *when* an effect should trigger → start in
`combat-pipeline.md`'s moment table. Implementing the effect → this doc's
class contract + the two paths. Both docs are the source of truth; the
`combat-guide` skill is deprecated and may be stale — re-trace, don't trust
it.

## When you change something here

- **New ability**: create the class under the right archetype folder,
  `Super(ABILITY_ID::X)`, set `ActivationMoment`, override the hook(s),
  add the cfg row, **and** `ABILITY_CLASSES[ABILITY_ID::X] := @Class` in
  the archetype registry. Missing the registry line = dead class.
- **SingleUse**: you **must** override `GetDurationFor` or it gets a
  1-second window (`AbilityService.inc:164`) and expires pre-hit.
- **Defender passive**: set `this.IsDefenderSide := 1` in the constructor,
  or the side gate (`AbilityService.inc:96`) silently drops it (it would
  otherwise be dispatched on the attacker).
- **New activation moment**: add to `ABILITY_ACTIVATION` (`Ability.inc:9`),
  route it in `CombatAbility.ExecuteOn` (active) and/or
  `AbilityService.RunPassive` (passive), and add the bridge function +
  call site per `combat-pipeline.md`.
- Never call methods on a wrapper/ability/monitor received via
  `Start_Script` params — re-wrap or use field access only.
- eScript identifiers are case-insensitive — verify renames with
  `grep -ri`.
