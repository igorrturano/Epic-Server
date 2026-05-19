# Resist System — Agent Reference

Canonical, code-grounded reference for the three resistances (Fortitude /
Reflexes / Will) and the resistance *test*. Written for agents working on
combat, spells, hunger, death, or attribute code. Everything here was traced
firsthand to the cited `file:line`. If you change resist behavior, update
this doc.

## TL;DR

- A resist is **not** a stored stat. It is a derived attribute whose value is
  recomputed on demand by an intrinsic-mod hook.
- `Fortitude ← Strength`, `Reflexes ← Dexterity`, `Will ← Intelligence`
  (each contributes `floor(attr / 20)`), plus class/race/culture/trait/
  ability/entropy/corruption terms.
- The value is carried internally **×10** (`INTRINSIC_TENTHS`). `AP_GetStat`
  divides it back down, so `GetResist()` returns the human-facing number
  (the one shown on the status bar).
- The test is `1d20 + GetResist + mod` vs a **fixed difficulty constant**.
  Nat 1 = auto-fail, nat 20 = auto-success. Some origins reroll once.

## Data model & scaling

| Layer | What | Where |
|---|---|---|
| Stored | nothing — resists have no base value persisted | — |
| Derived | `GetIntrinsic{Fortitude,Reflexes,Will}` exported hooks | `pkg/systems/attributes/hooks/intrinsicMods.src` |
| Wiring | `Attribute Fortitude/Reflexes/Will { GetIntrinsicModFunction … }` | `pkg/systems/attributes/attributes.cfg:46-58` |
| Calc | `IntrinsicCalc{Fortitude,Reflexes,Will}` + `IntrinsicCommonStatsIncrease` | `pkg/systems/attributes/include/intrinsicStats.inc` |
| Accessor | `GetResist(who, resist)` = `AP_GetStat(who, resist)` | `pkg/systems/charactercreation/include/resists.inc:12-14` |
| Test | `CheckResist(...)`, `ShouldGetSecondChance(...)` | `pkg/systems/charactercreation/include/resists.inc:16-77` |
| Difficulty | `enum RESIST_DIFF` | `scripts/include/epicConstants.inc:97-104` |

**`INTRINSIC_TENTHS := 10`** (`intrinsicStats.inc:17`). Every
`IntrinsicCalc*` returns `value * INTRINSIC_TENTHS`. The attribute layer
treats stat mods as tenths:

```
AP_GetStat(who,s)      = AP_GetTrueStat(who,s) + AP_GetStatMod(who,s)         # stats.inc:31
AP_GetTrueStat(who,s)  = GetAttributeBaseValue(who,s) / 10.0
AP_GetStatMod(who,s)   = (GetAttributeTemporaryMod(s) + GetAttributeIntrinsicMod(s)) / 10.0
AP_SetStatMod(who,s,p) = SetAttributeTemporaryMod(s, CInt(p*10)); RecalcVitals(who)   # stats.inc:108-110
```

Resists have no base value, so `GetResist == GetAttributeIntrinsicMod(/10)`,
i.e. exactly what `IntrinsicCalc*` returns divided by 10 — the displayed
number (`sendStats` divides by `INTRINSIC_TENTHS` for the paperdoll).

> `GetResist`'s own comment: *"este metodo só existe para compatibilidade
> com o sistema antigo"* — new code should rely on the attribute/intrinsic
> path, but `GetResist`/`CheckResist` remain the canonical read/test API.

## The formulas

All three start at 0, add the terms below, then `return total *
INTRINSIC_TENTHS`. NPCs / chardata-less mobiles short-circuit to **0**
(`if (mobile.npctemplate) return 0; if (chardata == error) return 0;` —
`intrinsicStats.inc:24-31` etc.).

**Common (all three)** — `IntrinsicCommonStatsIncrease` (`:201-213`):
`+2` if `chardata.raca == HUMANO`; `+5` if `HasAbility(BODY_AND_MIND)`.

**Fortitude** — `IntrinsicCalcFortitude` (`:104-136`):
- `+5` trait `"Fortitude do Urso"`
- `+2` ability `CACADOR_IMPLACAVEL`
- `- corruption_penalty` if `GetObjProperty(mobile,"corruption")`
- `+5` if `chardata.classe == "Guerreiro"` else `+2`
- entropy: `> 4000 → + entropy/4000`; `<= -4000 → - abs(entropy)/4000`
- `+ commonStats`
- `+ floor(AP_GetStat(STRENGTH) / 20)`
- `+ #FortitudeMod` ObjProperty

**Reflexes** — `IntrinsicCalcReflexes` (`:138-159`):
- `+5` trait `"Reflexos de Pantera"`
- `+5` ability `REAPROVEITAR_PERGAMINHO`
- `+5` if `chardata.classe == "Ladino"` else `+2`
- `+ commonStats`
- `+ floor(AP_GetStat(DEXTERITY) / 20)`
- `+ #ReflexosMod` ObjProperty
- **no entropy / no corruption term**

**Will** — `IntrinsicCalcWill` (`:161-199`):
- `+5` trait `"Vontade da Aguia"`
- `+5` if `chardata.classe == "Sabio"` else `+2`
- `+2` if `chardata.culture == ALURIEL`
- one of (priority order): `+5` cooldown `"VontadeIndomavel"`; elseif `+5`
  ability `SIGILO_DE_PROTECAO`; elseif `+8` ability `CONEXAO_DIVINA`;
  elseif `+ floor(THEOLOGY/20)` ability `CONSAGRACAO_DIVINA`
- entropy: same ±`/4000` rule as Fortitude
- `+ commonStats`
- `+ floor(AP_GetStat(INTELLIGENCE) / 20)`
- `+ #VontadeMod` ObjProperty

`chardata.classe` is the **archetype** (`"Guerreiro"`/`"Ladino"`/`"Sabio"`),
the legacy value kept for compat — the 30 specific classes map to one of
these (see `classes.cfg`; "Caminhos" Armas/Sabedoria/Subterfúgio are the
dead player-facing names for Guerreiro/Ladino/Sabio).

### Worked example

Humano, archetype Guerreiro, 100 STR / 70 DEX / 10 INT, no traits/abilities,
neutral entropy:
- Fortitude = 5 (class) + 2 (humano) + floor(100/20)=5 = **12**
- Reflexes  = 2 (class) + 2 (humano) + floor(70/20)=3  = **7**
- Will      = 2 (class) + 2 (humano) + floor(10/20)=0  = **4**

Real resist values are small. There is no points pool and no "+20/+10
Caminho" bonus (an old handbook myth).

## The test — `CheckResist(defender, resist_type, dif, mod := 0)`

`resists.inc:16-50`:

```
dado  = RandomDiceRoll("1d20")
valor = GetResist(defender, resist_type)
chance = dado + valor + mod
dado == 1  → return -1   (critical failure, auto-fail, SSM_FAIL emote)
dado == 20 → return  1   (critical success, auto-pass)
chance >= dif → return 1 (pass)
else:
   if ShouldGetSecondChance(defender, resist_type):
       reroll dado once; chance = dado + valor + mod; if chance >= dif return 1
   return 0 (fail)
```

Return contract: `1` pass, `0` fail, `-1` critical fail. **`0` and `-1` are
both falsy-looking but distinct — never treat the return as a boolean;
compare explicitly.** (See the project rule: error/!value pitfalls.)

`dif` is a **fixed constant the caller passes**, not rolled. Use the enum:

```
RESIST_EASY := 12   RESIST_MEDIUM := 19   RESIST_HARD := 24
RESIST_VERY_HARD := 29   RESIST_EXTREMELY_HARD := 34   RESIST_ALMOST_IMPOSSIBLE := 40
```
(`epicConstants.inc:97-104`)

`mod` is an optional flat situational shift (caller-supplied).

### Second chance — `ShouldGetSecondChance(defender, resist_type)`

`resists.inc:52-61`. Rerolls the d20 **once** on a failed (non-critical)
test when ANY holds:
- `chardata.culture == HAROLOTH` and `resist_type == FORTITUDE`
- `HasAbility(AURA_DIVINA)` and `resist_type == FORTITUDE`
- `chardata.raca == ANAO` and `resist_type == WILL`
- `chardata.culture == COLHESIANA` (any resist type)
- `MobileWrapper(defender).GetEntropy() <= -8000` and `resist_type == FORTITUDE`

`GetSecondChanceMessage` (`:63-77`) just supplies the flavor line on success.

## How mods propagate (worked: hunger)

A common confusion: "does X drop resists?" The rule — anything that changes
`AP_GetStat(STRENGTH|DEXTERITY|INTELLIGENCE)` changes the matching resist,
because the intrinsic calc reads `AP_GetStat(...)` live.

Hunger chain (verified): `hunger.src` → `ModStatFome` →
`FomeApplyDebuff` → `AP_ModifyStatMod(who, STRENGTH/INTELLIGENCE/DEXTERITY,
-fome*10)` → `SetAttributeTemporaryMod` → next `GetResist(FORTITUDE)`
recomputes with the reduced STR. Net: fome 3 (−30 STR/DEX/INT) ≈ −1 to each
resist; fome 5 (−50) ≈ −2. There is **no direct "−X to resist tests"
modifier** anywhere — only the attribute chain.

## Consumers (where resist tests fire)

- **Fracture on faint**: `pkg/mobiles/death/deathscripts/players.src:128-148`
  — `CheckResist(ghost, FORTITUDE, RESIST_*)`, difficulty rises as remaining
  DP (×10 vital) falls: `<50→MEDIUM, <40→HARD, <30→VERY_HARD,
  <=19→EXTREMELY_HARD`.
- **Hunger debuff gate**: `pkg/items/food/include/hunger.inc:51-90` — note
  it uses a **flat `GetResist(FORTITUDE) >= RESIST_MEDIUM` compare, not a
  `CheckResist` roll**. ≥19 Fortitude fully negates hunger penalties.
- Spells/traps/poison call `CheckResist` with their own `dif`; the
  pass→full-avoid vs half-effect outcome is decided per caller, not here.
- NB: spell *casting skill* (`MAGERY`/`ARCANA`/`THEOLOGY`,
  `cast_spell.src`) is a separate concept — not a resist.

## Gotchas for agents

- **×10 scale.** Internal intrinsic values are tenths. If you read raw
  `GetAttributeIntrinsicMod` you get the ×10 number; use `GetResist` /
  `AP_GetStat` for the human value, or `* INTRINSIC_TENTHS` when returning
  from an `IntrinsicCalc*`.
- **NPCs return 0.** Any `npctemplate` or missing `chardata` → resist 0.
  Don't design NPC mechanics expecting NPC resist values from this system.
- **Never `SetObjProperty` a resist directly.** Push via the documented
  knobs: the `#FortitudeMod`/`#ReflexosMod`/`#VontadeMod` ObjProperties, or
  a `TemporaryMod` on STR/DEX/INT through `AP_*`. Direct writes desync.
- **`#FortitudeMod` etc. are the generic "other bonus" channel** — kits,
  one-off effects, etc. should use these, not new ad-hoc terms in the calc.
- **Don't add per-class branches.** Class effect is already
  `Guerreiro/Ladino/Sabio +5/+2`; new class flavor goes through
  ability/trait checks, not a new `classe ==` line.
- Recomputed on read via the intrinsic hook; `AP_SetStatMod` calls
  `RecalcVitals`. You don't cache resist — read it when needed.
- Return of `CheckResist` is tri-state (`1/0/-1`); branch on the exact
  value, never `if (CheckResist(...))`.
