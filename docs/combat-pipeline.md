# Combat Pipeline — Agent Reference

Canonical, code-grounded map of a melee/ranged swing from the moment a
mobile enters war mode to the moment HP is deducted and post-damage
abilities fire. Written for agents working on combat, abilities, damage,
resists, or weapons. Everything here was traced firsthand to the cited
`file:line`. **If you change combat flow, update this doc.**

Read this instead of re-tracing the pipeline every time.

## TL;DR

- The eScript side does **not** drive the swing clock. The POL **core
  engine** owns the swing timer (speed derived from weapon speed + DEX/
  stamina, configured engine-side). When the timer fires, the core calls
  the `Attack` system hook.
- The hook wiring is one line: `syshook.cfg` maps `Attack` →
  `hooks/combatHook.ecl`.
- War-mode double-click goes through a **separate** packet hook
  (`requestAttack.src`) — that only validates the *engagement* and fires
  the `ENGAGING` ability moment. It does **not** deal damage. The repeating
  swing the core schedules afterwards is what deals damage.
- One swing = `Attack()` → `CanAttack()` → `TryToAttack()` (per hand) →
  `DoAttack()` → `mainHitScript` → `ProccessCombatDamage()` →
  `ApplyCombatDamage()`.
- Ability hooks are spliced in at fixed moments along that path (before
  hit, before damage, after damage, on crit, on kill, on parry, on miss),
  both **active** and **passive** variants.
- Damage is 7 typed channels carried as `Raw` (pre-resist) and `Base`
  (post-resist). Only the **active posture's** physical channel is applied;
  `GetTotalBaseDamage()` sums all 7 and is ~3× real physical. See the
  pitfall section.

## The four files

| Stage | File | Role |
|---|---|---|
| Engagement | `pkg/packetHooks/RequestAttack/requestAttack.src` | War-mode double-click: LoS/distance/sanctuary gate + `ENGAGING` moment. No damage. |
| Hook wiring | `pkg/systems/combat/syshook.cfg` | Maps core `Attack` → `combatHook.ecl` |
| Entry | `pkg/systems/combat/hooks/combatHook.src` | `Attack()` → validate → roll hit → dispatch |
| Hit script | `pkg/systems/combat/mainHitScript.src` | Per-swing async/sync body: region → wear → damage → FX → skill gain |
| Damage | `pkg/systems/combat/include/Controller/DamageController.inc` | The damage math: build → crit → resist → apply → post |

## Stage 0 — POL core swing clock (engine-side, not in this repo)

POL's combat core handles the *cadence*. When a mobile is in war mode and
has an opponent, the core schedules repeating swings; the delay between
swings is the classic UO speed calculation derived from the weapon's speed
and the attacker's DEX/stamina, tuned by the core's combat config. This is
**C++ engine code**, not eScript — you cannot change the swing rate from
this package; you change the inputs (weapon speed, DEX).

When each scheduled swing fires, the core invokes the **`Attack` system
hook**. The binding lives in `syshook.cfg:16-19`:

```
SystemHookScript hooks/combatHook.ecl
{
	Attack Attack
}
```

So: **core swing timer → `combatHook.src::Attack(attacker, defender)`**.
Everything below is eScript and lives in this repo.

> `combatAdv` / `parryAdv` (`syshook.cfg:1-9`) are the separate skill-gain
> advancement hooks, unrelated to the damage path.

## Stage 1 — Engagement (war-mode double-click)

`requestAttack.src::HandleAttackRequest(who, packet)` fires once when a
player double-clicks a target in war mode — **not** per swing. It only:

- blocks `#sanctuary` targets (`requestAttack.src:39-42`)
- distance-gates: 18 tiles ranged / 6 melee vs NPCs (`:48-61`)
- line-of-sight check (`:63-66`)
- logs the attack request + citizen-witness crime check (`:68-75`)
- fires the **`ENGAGING`** ability moment via
  `ExecuteMasteryAbility(AttackInfo(..., ATTACK_MOMENT::ENGAGING))`
  (`:77`)

Return `0` lets the core proceed to schedule swings; early `return 1`
aborts engagement. No damage is dealt here.

## Stage 2 — `Attack()` entry & validation

`combatHook.src::Attack(attacker, defender)` (`:29-55`):

1. Wraps both refs in `MobileWrapper`, builds `AttackInfo`, sets the
   weapon from the `WEAPON` slot (`:30-33`).
2. `CanAttack(attackInfo)` (`:57-146`) — hard gate. Notable rules:
   - self-attack → 5000 self-damage joke (`:61-65`)
   - LoS, hidden target, must be in war mode (NPCs exempt) (`:67-84`)
   - paralyzed/frozen, distance, facing/behind (`:86-99`)
   - ammo present (players only), `defesatotal` cooldown,
     `arcane_shooting` mana (`:102-115`)
   - charmed/hypnotized, `#PrisaoGelida`, `#sanctuary`, spiritual-blade
     enchantment present (`:122-143`)
3. Sets attack type = attacker's **posture** (`:44`,
   `GetCombatTraits().GetPosture()`).
4. `TryToAttack(attackInfo)` (`:47`).
5. If weapon attribute is `DUALWIELD`: switch to `OFFHAND` and
   `TryToAttack` again (`:49-52`).

## Stage 3 — `TryToAttack()` hit roll

`combatHook.src::TryToAttack(byRef attackInfo)` (`:148-208`):

1. `CalculateHitChance()` (HitController) → `SetHitChance` (`:151`).
2. Main hand: spawns `opportunityAttack`. Off hand: applies
   `CalculateDualWieldPenalty` to the hit-chance mod (`:153-161`).
3. Ranged non-throwable non-magicbow: consume ammo, try to recover
   (`:163-174`).
4. `RollHitChance()` (`:176`), then the **before-hit ability moments** in
   order (`:180-182`):
   - `ExecuteCombatAbilitiesBeforeHit` (defender active)
   - `ExecutePassiveAbilitiesAttackerBeforeHit`
   - `ExecutePassiveAbilitiesDefenderBeforeHit`
   - then `ResolveSubstituicao()` (`:183`)
5. `attackInfo.Succeeded()` (`:186`):
   - **Hit** → main hand only: `ExecuteMasteryAbility` (`:190`), then
     `DoAttack(attackInfo)` (`:193`).
   - **Miss** → FX/SFX + `ExecuteCombatAbilitiesOnMiss` /
     `OnMissAttacker` / `ExecutePassiveAbilitiesOnMissAttacker`
     (`:195-201`).
6. Counter-attack: if `ShouldCounterAttack()` and defender alive,
   `defender.Attack(attacker)` (crosscut) (`:205-207`).

## Stage 4 — `DoAttack()` → `mainHitScript`

`combatHook.src::DoAttack(byRef attackInfo)` (`:210-236`):

- Unfreezes defender; clears attacker concealment unless GM /
  `FORMA_UMBRAL` / in smoke zone (`:211-218`).
- Pet-attack logging when a warmode player hits a tamed NPC (`:220-229`).
- **Main hand** → `Run_Script_To_Completion(:combat:mainHitScript)`
  (synchronous). **Off hand** → `Start_Script` (async, because of the
  off-hand delay) (`:231-235`).

`mainHitScript.src::MainHitScript(attackInfo)` (`:22-51`):

1. `SetScriptController(attacker)` (`:24`).
2. Off-hand: `SleepMS(OFFHAND_DELAY)` (`:26-28`).
3. Dead-defender guard — bail if already dead (race protection)
   (`:30-32`).
4. `CheckBodyRegionToHit` → random armor zone into
   `attackInfo.SetBodyRegion` (`:34`, `:53-57`).
5. `WeaponHitScripts` (`:36`).
6. `WearDownEquip(weapon)` (`:39`).
7. **`ProccessCombatDamage(attackInfo)`** (`:40`) — the damage pipeline,
   Stage 5.
8. `ArmorHitScripts`, FX, SFX (`:41-44`).
9. `ProgressAttackSkills`, `MasteryCombat_Advance`,
   `AchievementCombat_Advance` (`:46-48`).

> `DoMovement` (`mainHitScript.src:59-86`) handles queued special "movement"
> attacks; it is invoked from the hit-script controller path, not the
> linear `MainHitScript` body.

## Stage 5 — `ProccessCombatDamage()` damage pipeline

`DamageController.inc::ProccessCombatDamage(byRef attackInfo)` (`:28-55`).
Linear order:

```
SnapshotDefenderLocation()              # coords before death invalidates ref (:59)
CalculateCombatDamage()                 # (:129) — build → crit → resist
CheckParry()                            # ParryController (:33)
ApplyCombatDamage()                     # deduct HP (:36, :1114)
AttackLogger()                          # lastHit prop + NPC threat event (:37, :1220)
CheckPoison()                           # weapon/ammo/NPC poison (:40, :1277)
ExecuteCombatAbilitiesAfterDamage()     # attacker active (:41)
ExecutePassiveAbilitiesAfterDamage()    # (:42)
if defender dead:
    ExecuteCombatAbilitiesOnKill()      # (:46)
    ExecutePassiveAbilitiesOnKill()     # (:47)
    if crit: TriggerCriticalKillExplosion()  # gib FX (:49, :88)
elif crit:
    TriggerCriticalHitFeedback()        # screen shake (:52, :81)
```

### 5a — `CalculateCombatDamage()` (`:129-159`)

```
CalculateAttackerDamage()                       # (:161) build all channels
ExecuteCombatAbilitiesBeforeDamage()            # attacker active (:131)
ExecutePassiveAbilitiesAttackerBeforeDamage()   # (:132)
ExecutePassiveAbilitiesDefenderBeforeDamage()   # (:133)
CalculateFlankingDamage()                       # +1d4 if hit from BACK (:154, :635)
CalculateAttackerCriticalHit()                  # crit roll + multipliers (:155, :915)
CalculateDefenderResist()                       # body/armor/spell % reduction (:157, :654)
```

`CalculateAttackerDamage()` (`:161-177`) layers the raw damage:

| Term | Func | Notes |
|---|---|---|
| Status | `CalculateStatusDamage` (`:200`) | STR (melee), DEX (ranged/finesse: DEX/15 main, /30 off), STR+DEX (throwing). Added to **all 3** physical channels. |
| Skill | `CalculateSkillDamage` (`:248`) | Tactics (or Theology if `metamorfose`). All 3 physical channels. |
| Weapon | `CalculateWeaponDamage` (`:271`) | Dice roll per channel + weapon damage mod. 1H solo-hit upgrades the physical die one tier (`:283`, `_IsOneHandedSoloHit:749`). The weapon/armor mod values come from the crafted `fabricado` prop via `PegaValorFabricacao` — how a material+tier becomes that number is [`craft-pipeline.md`](craft-pipeline.md) (value-math spine). |
| Entropy | `CalculateEntropyDamage` (`:1397`) | +dmg vs undead/demon/void (player entropy ≥12k, or global ≤−20k). |
| Primordial | `CalculatePrimordialDamage` (`:461`) | Metamorfose elemental: blunt raw → element. |
| Arcane bow | `CalculateArcaneBowDamage` (`:488`) | MagicBow: arcana/10 (or /5 w/ ARCANE_MASTERY) elemental. |
| Spiritual blade | `CalculateSpiritualBladeDamage` (`:528`) | arcana/5 flat into active posture. |
| Elemental weapon | `CalculateElementalWeaponDamage` (`:561`) | Sword-mage enchant: converts arcana/2 % physical→element, consumes a charge. |

### 5b — Crit (`CalculateAttackerCriticalHit:915`)

- Skipped for spells (`ShouldSkipCrit`) and **off-hand** dual-wield swings
  (`:918-925`).
- `CalculateCriticalChance` (`:942`): weapon crit + char crit +
  skill/20 − defender crit-reduction; NPC variant uses tier; `#nextcritical`
  forces 100%; `MUMIFICACAO` / `VontadeIndomavel` / `vitalidadeinabalavel`
  / `balance_restored` negate it.
- `ApplyCriticalDamage` (`:1007`): resolves crit hit-scripts from
  weapon/taming/npc cfg. `armorpierce` sets `SetIgnoreArmor(true)`;
  `damage2`/`damage3` multiply the **active posture's** physical + all
  elementals (×2 / ×3 total).

### 5c — Resist (`CalculateDefenderResist:654`)

1. `IgnoresArmor()` early-return — skips **all** armor (crit armorpierce,
   `encontrarbrecha`, etc.) (`:656-659`).
2. **Body resist** — multiplicative %, *outside* the combined cap
   (`:668-675`). Snapshot taken post-body (`postBody`, `:679-687`).
3. **Armor** — per equip in the hit region, ordered front→back or
   back→front by attack direction (`GetEquipsByAttackDirection:860`).
   Multiplicative %, wears down each piece (`:692-708`).
4. **Spell resist** — virtual magic layer, multiplicative (`:712-720`).
   Sourced from `MobileWrapper.GetSpellResist()` (the derived
   `#SpellResist`); how that value is built/maintained by spell buffs is
   [`spells-pipeline.md`](spells-pipeline.md) (resist model).
5. **Combined armor+spell cap 80%** — only if `_IsDefenderArmored`
   (`:726-728`, `_ApplyArmorSpellCombinedCap:788`). Clips each channel so
   it never drops below 20% of the post-body snapshot. Naked mages get no
   cap (their spell resist alone never reaches 80%).
6. `_ApplyPhysicalManaShield` (`:741`, `:810`) — Proteção Arcana spell:
   absorbs the posture's physical at `ratio` mana per damage; elemental
   bypasses.

### 5d — `ApplyCombatDamage()` (`:1114-1210`)

- Fires `ExecuteCombatAbilitiesOnHit` first (`:1115`).
- Zeroes the two non-active physical channels, **guarantees min 1** on the
  active posture channel, `DisplayDamage` + `ApplyRawDamage` per channel
  (`:1120-1154`).
- Each elemental channel applied only if `> 0`, else clamped to 0
  (negative-after-resist guard) (`:1156-1186`).
- Post: `SetLastDamageInfo` on lethal hits, pet-owner damage display,
  `AccumulateVinganca` (`:1188-1209`).

## The damage-total pitfall (read before touching damage)

`GetTotalBaseDamage()` / `GetTotalRawDamage()` **sum all 7 channels**.
Status + skill bonuses are added to **all three** physical channels
(slash/piercing/blunt) in `CalculateStatusDamage`/`CalculateSkillDamage`,
but `ApplyCombatDamage` only applies the **active posture's** channel and
zeroes the other two. So a total-based read is ~3× the real physical hit.

Correct pattern for "what did this swing actually do physically":

```escript
var posture := attacker.GetCombatTraits().GetPosture();
var real := 0;
if      (posture == DAMAGE_TYPE::SLASH)    real := damage.GetSlashBaseDamage();
elseif  (posture == DAMAGE_TYPE::PIERCING) real := damage.GetPiercingBaseDamage();
elseif  (posture == DAMAGE_TYPE::BLUNT)    real := damage.GetBluntBaseDamage();
endif
real += damage.GetFireBaseDamage() + damage.GetColdBaseDamage()
      + damage.GetPoisonBaseDamage() + damage.GetEnergyBaseDamage();  // elementals always apply
```

`Raw*` = before resist, `Base*` = after resist. `Increase*RawDamage` also
bumps Base (they track together until `CalculateDefenderResist` decreases
Base via the `Decrease*PercentDamage` calls).

## Ability hook insertion points (combat moments)

Bridged by `CombatAbilityController.inc`. Position in the pipeline:

| Moment | Where it fires | Side |
|---|---|---|
| `ENGAGING` | `requestAttack.src:77` (double-click) | attacker |
| `BeforeHit` / passive before-hit (atk+def) | `TryToAttack` pre-roll, `combatHook.src:180-182` | defender / both |
| `OnMiss` / `OnMissAttacker` / passive | `TryToAttack` miss branch, `combatHook.src:199-201` | both |
| Mastery ability | `TryToAttack` hit branch, main hand, `combatHook.src:190` | attacker |
| `BeforeDamage` + passive atk/def before-damage | `CalculateCombatDamage`, `DamageController.inc:131-133` | both |
| `OnHit` | `ApplyCombatDamage` start, `DamageController.inc:1115` | attacker |
| `OnParry` | `CheckParry` → ParryController | defender |
| `AfterDamage` + passive | post-apply, `DamageController.inc:41-42` | both |
| `OnKill` + passive | defender dead, `DamageController.inc:46-47` | attacker |

For *how* a fired moment resolves to an ability class method — the V3
class contract, the two trigger paths (activation vs combat moment),
registration, `SingleUse`, the `GetDurationFor` window rule, and the
cross-script boundary constraints — see
[`ability-pipeline.md`](ability-pipeline.md), the ability-side companion
to this damage-side doc. (The old `combat-guide` skill is deprecated and
may be stale — use the docs.)

## When you change something here

- New damage source → add it in `CalculateAttackerDamage` (`:161`), use
  `Increase*RawDamage`, respect posture (don't blanket all 3 physical
  unless it's a flat stat-like bonus).
- New resist layer → slot it in `CalculateDefenderResist` and decide
  explicitly whether it sits inside or outside the 80% combined cap.
- New ability moment → wire it through `CombatAbilityController.inc`, then
  call the bridge at the correct pipeline position above. Don't invent a
  new call site without checking the moment table.
- Touching crit → remember off-hand never crits and spells skip crit;
  `damage2/3` multiply only the active posture + elementals.
- Always re-grep; eScript identifiers are case-insensitive (use
  `grep -i`).
