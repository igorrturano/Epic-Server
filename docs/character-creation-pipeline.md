# Character Creation Pipeline

End-to-end map of how a new player goes from "press Play" to standing in
`britannia_alt` at `(1407, 1062, -4)`. Source of truth is
[epicgumpcriacao.src](../pkg/systems/charactercreation/epicgumpcriacao.src);
the heavy lifting lives in
[scripts/include/creationUtils.inc](../scripts/include/creationUtils.inc).
HP math, dice parsing, and per-level scaling live in
[pkg/systems/attributes/include/attributes.inc](../pkg/systems/attributes/include/attributes.inc).

> Phase notes: the pipeline is mid-rework. Phase 2 reworked skills into
> slot buckets, Phase 3 reworked classes into 29 specific classes, Phase 4
> reworked HP into dice × odd-level grants. Comments in
> [epicgumpcriacao.src:328-335](../pkg/systems/charactercreation/epicgumpcriacao.src#L328-L335)
> and [attributes.inc:206-247](../pkg/systems/attributes/include/attributes.inc#L206-L247)
> reference these phases.

---

## 1. Orchestrator order

`program epicCreationGump(who)` in
[epicgumpcriacao.src:42-122](../pkg/systems/charactercreation/epicgumpcriacao.src#L42-L122)
runs steps strictly in this order. `chardata` is created at the very start
(line 50) and acts as both the in-progress flag and the accumulator for
every selection. Each gump loop sleeps 2 ms and validates via
`IsGumpValid()` to avoid tight-loop spam.

| # | Function                       | Writes to `chardata`                                              |
| - | ------------------------------ | ----------------------------------------------------------------- |
| 1 | `culturalSelection`            | `culture`, `crafting`                                             |
| 2 | `racialSelection`              | `raca`; also mutates `who.graphic`/`who.trueobjtype`/`who.race`   |
| 3 | `pathSelection`                | `path` (`MILITAR` or `CIVIL`)                                     |
| 4 | `classSelection`               | `subclasse`, `classe` (archetype), `dice`                         |
| 5 | `traitSelection`               | `trait`                                                           |
| 6 | `skillSelection`               | `Skills` dict (skill → tier); also writes base skill values       |
| 7 | `attributeSelection`           | `str`, `dex`, `int`, `attribute_points`                           |
| 8 | `religionSelection`            | `religion`                                                        |
| 9 | `gameModeSelection`            | `game_mode`                                                       |
| 10 | `applyClassStarter`           | `magia` (casting type); grants spellbook/scrolls/items per class  |
| 11 | `itemInheritance`             | (no chardata write) — grants coins, outfit, weapons, tools         |
| 12 | `languageSelection`           | (writes `linguas_conhecidas`, `lingua_escolhida` on `who`)         |
| 13 | `signSelection`               | `sign` (one of 12 constellations)                                  |
| 14 | `birthDaySelection`           | `birthdate.day`                                                    |
| 15 | `RPcharacteristicsSelection`  | `caracteristicas_rp`, `idade_escolhida`, `idade_aparente`, `idade_real` |
| 16 | `setBirthdate`                | finalizes `birthdate` (year/month/day), `initial_age`, `current_age` |
| 17 | `looksSelection`              | `hair`, `hair_item`, `hair_color`, `beard`, `beard_color`, `skin_color` |
| 18 | `setCitizenship`              | `citizenship_title` + nobility/professional title arrays           |
| 19 | `startAdventure`              | `Level := 1`; moves character into the world                       |

After step 19, `creation_completed` is set on `who`
([epicgumpcriacao.src:91](../pkg/systems/charactercreation/epicgumpcriacao.src#L91))
and (for non-GMs) the census is incremented.

The migration handoff block
([epicgumpcriacao.src:104-121](../pkg/systems/charactercreation/epicgumpcriacao.src#L104-L121))
runs only when `migrationStone.src` triggered the rerun — it restores the
player's pre-migration XP, Level, createdate, and name.

---

## 2. Cultures (8)

Selected by cycling pictures `0x9E7..0x9EE` in
[`culturalSelection`](../pkg/systems/charactercreation/epicgumpcriacao.src#L124-L156).
`getCultureByPictureId` and `getCraftCultureByPictureId` in
creationUtils.inc map the picture to a culture string + crafting culture.

| Culture     | Pic    | Default race        | Default language | Notes                               |
| ----------- | ------ | ------------------- | ---------------- | ----------------------------------- |
| AIGLANA     | 0x9E7  | HUMANO (any/hybrid) | Imperialis       | Multicultural; all 6 races allowed  |
| ALURIEL     | 0x9E8  | ELFO                | Ahruwën          | Elven kingdom                       |
| BADURAN     | 0x9E9  | ANAO                | Aglâb            | Dwarven kingdom; **+10 max HP**     |
| BJORSKA     | 0x9EA  | HUMANO              | Björ             | Northern human culture              |
| GULTHRAK    | 0x9EB  | ORC                 | Grumnak          | Orc culture                         |
| HAROLOTH    | 0x9EC  | DROW                | Idrith'har       | Drow culture                        |
| COLHESIANA  | 0x9ED  | PEQUILHO            | Comum            | Halfling culture                    |
| LESTE       | 0x9EE  | HUMANO (any/hybrid) | Phalk            | Eastern; all races allowed          |

`BADURAN` characters get a **+1000 vital-points (10 HP) bonus** in
[vitalInit.src:54-56](../pkg/systems/attributes/hooks/vitalInit.src#L54-L56).

---

## 3. Races (6)

Race constants defined in `epicConstants.inc`. Body graphics are stamped
on `who.graphic` AND `who.trueobjtype` so any revert (polymorph end,
resurrection, metamorfose dispel, ironmaiden release) restores to the
racial body, not the 400/401 human template
([epicgumpcriacao.src:200-227](../pkg/systems/charactercreation/epicgumpcriacao.src#L200-L227)).

| Race      | Body M | Body F | STR | DEX | INT | Cultural restriction                       |
| --------- | ------ | ------ | --- | --- | --- | ------------------------------------------ |
| HUMANO    | 400    | 401    | 0   | 0   | 0   | Free (Aiglana/Leste/Bjorska)               |
| ANAO      | 1828   | 1829   | +10 | 0   | 0   | Free in Aiglana/Leste; native to Baduran   |
| ELFO      | 1830   | 1831   | 0   | 0   | +10 | Free in Aiglana/Leste; native to Aluriel   |
| DROW      | 1830   | 1831   | 0   | +5  | +5  | Native to Haroloth                         |
| ORC       | 1832   | 1833   | +5  | 0   | +5  | Native to Gulthrak                         |
| PEQUILHO  | 1835   | 1834   | 0   | +10 | 0   | Native to Colhesiana                       |

Stat bonuses applied by `attributeBonusByRaceAndTraits` in
creationUtils.inc — runs **after** the player commits attribute spend, so
the bonuses sit on top of the 40/40/40 + 80-point pool.

In Aiglana/Leste a player must pick one of buttons 201-206; in single-race
cultures the race is auto-set via `getRaceByCulturalId`
([epicgumpcriacao.src:189-194](../pkg/systems/charactercreation/epicgumpcriacao.src#L189-L194)).

---

## 4. Paths (2) and skill slots

Path is a binary fork (`MILITAR` button 201 / `CIVIL` button 202)
controlling which skill bucket each slot draws from.

`BuildSkillSlots(path, has_sabe_tudo)` in creationUtils.inc returns the
slot layout. Skill values are written in
[epicgumpcriacao.src:506-517](../pkg/systems/charactercreation/epicgumpcriacao.src#L506-L517).

**Standard layout (6 slots):**

| Slot | Tier | Bucket    | Start |
| ---- | ---- | --------- | ----- |
| 1    | 3 (P) | path     | 40    |
| 2    | 3 (P) | path     | 40    |
| 3    | 3 (P) | cross    | 40    |
| 4    | 2 (S) | path     | 20    |
| 5    | 2 (S) | cross    | 20    |
| 6    | 2 (S) | either   | 20    |

**Sabe-Tudo trait (7 slots, 2P + 5S):**

| Slot | Tier | Bucket   |
| ---- | ---- | -------- |
| 1    | 3 (P) | path    |
| 2    | 3 (P) | cross   |
| 3-7  | 2 (S) | mix     |

Buckets (`epicConstants.inc`):

- **MILITARY_SKILLS** — Wrestling, One_Handed, Two_Handed, Dual_Wield, Parry, Ranged, Tactics, Stealth, Beastmastery, Magery, Arcana, Theology, Science.
- **CIVIC_SKILLS** — Smithing, Carpentry, Tailoring, Provisioning, Alchemy, Mining, Lumberjacking, Survival, Taming, Medicine, Mechanics, Perception, Theft.

> ⚠️ Discrepancy: the comment on
> [epicgumpcriacao.src:507](../pkg/systems/charactercreation/epicgumpcriacao.src#L507)
> says Secondary starts at 10, but
> [line 512](../pkg/systems/charactercreation/epicgumpcriacao.src#L512)
> sets it to 20. **20 is the actual value.**

---

## 5. Classes (29)

Defined in
[classes.cfg](../pkg/systems/charactercreation/config/classes.cfg). Each
entry carries `archetype` (`Guerreiro` / `Ladino` / `Sabio`) and `dice`
(`d4..d12`). The archetype is what legacy code keying off
`chardata.classe` still sees; the new specific class is `chardata.subclasse`.

### Guerreiro
| Class      | Dice | Notes |
| ---------- | ---- | ----- |
| Arqueiro    | d10 | Ranged specialist |
| Caçador     | d10 | Tracker / stealthy |
| Combatente  | d10 | Close-range brawler |
| Defensor    | d12 | Guardian / taunt |
| Paladino    | d10 | Holy warrior + magic |
| Aklattur    | d10 | Monster hunter, life steal |
| Bárbaro     | d12 | Primal fury |
| Monge       | d12 | Unarmed |
| Samurai     | d10 | Swordmaster |
| Mutante     | d8  | Alchemical metamorph |

### Ladino
| Class               | Dice | Notes |
| ------------------- | ---- | ----- |
| Malandro_Arcano      | d6 | Arcane thief |
| Cacador_de_Tesouros  | d8 | Treasure hunter |
| Assassino            | d8 | Stealth + bleed |
| Shinobi              | d8 | Silent pressure |
| Vivisseccionista     | d8 | Poison + surgery |
| Esgrimista           | d8 | Graceful swordmaster |
| Ladrao               | d8 | Street thief |
| Bardo                | d6 | Mystical performer |
| Dancarino_das_Sombras | d8 | Shadow dancer (martial, no spellbook) |
| Cacador_de_Demonios   | d8 | Demon hunter |

### Sabio
| Class                   | Dice | Notes |
| ----------------------- | ---- | ----- |
| Necromante               | d6 | Undead summoner — spellbook |
| Druida                   | d8 | Nature summoner — auto Sylovir language |
| Magus                    | d6 | Dimensional mage — spellbook |
| Lamina_Arcana            | d8 | Elemental sword — spellbook |
| Deslocador_das_Sombras    | d8 | Shadow Velkyn mage — spellbook |
| Arqueiro_Arcano           | d6 | Auto LORE religion + ARCANE_BOW ability |
| Granadeiro               | d6 | Alchemical engineer |
| Shaman                   | d8 | Spirit binder |
| Engenheiro               | d8 | Gunpowder; gets 2 pistols + powder + ammo |
| Cleric                   | d8 | Sacred caster; chisels + random raw gem |

### Class-driven starter grants

[applyClassStarter](../pkg/systems/charactercreation/epicgumpcriacao.src#L808-L903)
dispatches by `chardata.subclasse`:

- **Spellcasters** (Magus / Necromante / Lamina_Arcana / Deslocador_das_Sombras):
  `chardata.magia := "spellcaster"`, grimoire bound to player serial,
  newbie + recolored to 0x0EFA, plus spell choice scrolls.
- **Cleric**: `magia := "Sacerdote"`; if culture is HAROLOTH, gets a whip
  (via `CreateItemInicial` to mark it newbie); chisels + 2 random raw gems.
- **Druida**: `magia := "Druid"`.
- **Shaman**: `magia := "Shaman"`.
- **Engenheiro**: 20 ammo, 20 powder, powderfixer, bandoleira (0xC892),
  pistol1, pistol2 (both bound by `serialid`).
- **Arqueiro_Arcano**: `magia := "arcane_archer"`, religion locked to
  `LORE`, and `ABILITY_ID::ARCANE_BOW` slotted into `myhabs[2]`/`buyhab[2]`.
  This is the only ability granted at creation — every other class earns
  abilities through level-ups.

---

## 6. Traits

Read from
[traits.cfg](../pkg/systems/charactercreation/config/traits.cfg);
filtered per character by `listTraits(who, chardata)` in creationUtils.inc.
Filtering is race-, culture- and class-aware. Trait struct uses lowercase
`.nome` (see the cautionary note in
[epicgumpcriacao.src:441-447](../pkg/systems/charactercreation/epicgumpcriacao.src#L441-L447)).

### Universal stat-shaping traits

| Trait                | STR | DEX | INT | Extra effect            |
| -------------------- | --- | --- | --- | ----------------------- |
| Corpulento            | +10 | -5  | -5  | —                      |
| Ágil                  | -5  | +10 | -5  | —                      |
| Gênio                 | -5  | -5  | +10 | —                      |
| Fortitude do Urso     | —   | —   | —   | +5 Fortitude save      |
| Reflexos de Pantera   | —   | —   | —   | +5 Reflexos save       |
| Vontade da Águia      | —   | —   | —   | +5 Vontade save        |
| **Sabe-Tudo**         | —   | —   | —   | 2P + 5S skill slots    |

### Racial / cultural traits

| Trait                       | Race / Culture | Effect                                      |
| --------------------------- | -------------- | ------------------------------------------- |
| Natureza Arcana             | Elfo, Drow      | Cantrips                                   |
| Sorrateiro                  | Pequilho        | Sneak bonus                                |
| Cavar Buracos               | Anão            | +2 mining ore                              |
| Robusto                     | Orc             | +15 STR / -5 DEX / -5 INT                  |
| Adaptável                   | Humano          | +2 climate resist                          |
| Oito Olhos                  | Haroloth        | Cultural perception trait                  |
| Força da Legião             | Aiglana         | Cultural                                   |
| Cavalaria de Escaramuça      | Leste, Bjorska  | Cultural mounted skirmish                  |
| Conexão com Alora           | Alüriel         | Cultural                                   |
| Defensor Anão               | Badûran         | Cultural                                   |
| Desapercebido               | Colhesiana      | Cultural stealth                           |
| Cria do Deserto             | Gulthrak        | Cultural                                   |

---

## 7. Attributes

[attributeSelection](../pkg/systems/charactercreation/epicgumpcriacao.src#L520-L570)
opens with `attribute_points := 80` and the three stats pre-loaded at
40/40/40. Buttons 200/201 (STR ±5), 210/211 (DEX ±5), 220/221 (INT ±5)
trade points against the 80-point pool. Confirming with
`attribute_points < 0` hard-disconnects the client (anti-tamper).

After commit, `attributeBonusByRaceAndTraits(who)` layers in:

1. **Race** bonuses from the table above.
2. **Trait** bonuses (Corpulento / Ágil / Gênio / Robusto).
3. Writes the final values via `AP_SetTrueStat`.

---

## 8. HP — the Phase 4 formula

Max HP is `(GetHP + GetPH) × 100` vital-points, computed in
[vitalInit.src GetHitsMaximumValue](../pkg/systems/attributes/hooks/vitalInit.src#L42-L69).

### `GetHP(who)` — STR-derived portion

[attributes.inc:497-507](../pkg/systems/attributes/include/attributes.inc#L497-L507)

```
HP = TrueStr
   + 30 if HasAbility(RESISTENCIA_DO_DESERTO)
   + hitsmod + #hitsmod
```

### `GetPH(who)` — level/class-derived portion

[attributes.inc:518-549](../pkg/systems/attributes/include/attributes.inc#L518-L549)

Path-rework characters (have `chardata.dice` set):

```
PH = die × (1 + OddLevelsReached(Level))
```

where `die` is the numeric die size (`d4=4 ... d12=12`) and
`OddLevelsReached` is `(level - 1) / 2`, saturating at 9 once Level ≥ 19.
So every odd level (3, 5, 7, ..., 19) adds another die-worth of HP, then
gains stop.

PH then adds `+25` if `CACADOR_IMPLACAVEL`, plus `phmod + #phmod`.

#### Class-die → starting PH (Level 1)

| Die | L1 PH | L19+ PH (max odd-level grants) |
| --- | ----- | ------------------------------ |
| d4   |  4    |  40 |
| d6   |  6    |  60 |
| d8   |  8    |  80 |
| d10  | 10    | 100 |
| d12  | 12    | 120 |

#### Race + class L1 max HP (no traits, no mods, base STR before purchases)

Starting STR is the player's chosen base (after race bonus). Sample:
race-default 40 STR + race STR bonus, picking a representative die per
archetype.

| Race      | Race STR bonus | Min L1 HP (d4) | (d6) | (d8) | (d10) | (d12) |
| --------- | -------------- | -------------- | ---- | ---- | ----- | ----- |
| Humano     |  0  | 44 | 46 | 48 | 50 | 52 |
| Anão       | +10 | 54 | 56 | 58 | 60 | 62 |
| Elfo       |  0  | 44 | 46 | 48 | 50 | 52 |
| Drow       |  0  | 44 | 46 | 48 | 50 | 52 |
| Orc        | +5  | 49 | 51 | 53 | 55 | 57 |
| Pequilho   |  0  | 44 | 46 | 48 | 50 | 52 |

> Numbers assume base STR = 40 (no points spent into STR). Add +1 HP per
> +1 STR spent at creation. Culture `BADURAN` adds a flat **+10 HP** on
> top via [vitalInit.src:54-56](../pkg/systems/attributes/hooks/vitalInit.src#L54-L56).

Legacy (pre-Phase-3) characters fall back to the old archetype lookup
([attributes.inc:529-540](../pkg/systems/attributes/include/attributes.inc#L529-L540)):
Guerreiro 25, Artifice 105, Ladino 20, else 15.

`RecalcVitals(who)` is called once at the end of creation
([epicgumpcriacao.src:76](../pkg/systems/charactercreation/epicgumpcriacao.src#L76))
and again after migration restore (line 119).

---

## 9. Religions (8)

Cycled in `religionSelection` over picture IDs `0x9EF..0x9F6`.
`getReligionByPictureId` in creationUtils.inc:

| Pic    | Religion      |
| ------ | ------------- |
| 0x9EF  | Bokk Bharaz   |
| 0x9F0  | Eluith'orth   |
| 0x9F1  | Halleuad      |
| 0x9F2  | Kai           |
| 0x9F3  | Lórё          |
| 0x9F4  | Ushkr'ar      |
| 0x9F5  | Veneratio     |
| 0x9F6  | Céticos       |

Arqueiro_Arcano forces `LORE` regardless of the player's pick
([epicgumpcriacao.src:883](../pkg/systems/charactercreation/epicgumpcriacao.src#L883)).
For the broader religion mechanic see
[docs/religion-system.md](religion-system.md).

---

## 10. Game modes (2)

> **Hotfix note (2026-05-29):** the `gameModeSelection(who)` call is
> currently commented out in the orchestrator
> ([epicgumpcriacao.src:70](../pkg/systems/charactercreation/epicgumpcriacao.src#L70)).
> New characters skip the selection and never get `chardata.game_mode`
> written — they fall through to Story Mode behavior everywhere
> `chardata.game_mode == "True Mode"` is checked. Legacy True Mode
> characters keep the field set and continue using the level-based
> skill cap (`TrueModeSkillCap`) and level-based buyhab slot opening
> in `SetPlayerSkillLevel`. The function and helpers stay in source
> so a future re-enable is a single uncomment.

`gameModeSelection` cycles `0x9FB..0x9FC`. `gameModeByPictureId`:

- `0x9FB` → STORY_MODE — softer, age-based skill caps; the only mode that
  preserves the `createdate` invariant
  ([epicgumpcriacao.src:77-85](../pkg/systems/charactercreation/epicgumpcriacao.src#L77-L85)).
- `0x9FC` → TRUE_MODE — full ruleset, harder.

---

## 11. Resists

Per-race / per-culture resists are applied by intrinsic mods elsewhere,
not by the gump flow. The character-creation package's contribution is
[resists.inc](../pkg/systems/charactercreation/include/resists.inc),
which adds **second-chance "salvation" hooks** on top of the normal save:

- **HAROLOTH + Fortitude** — "Seu organismo drow expurga o veneno."
- **ANAO + Will** — "Sua constituição anã o salvou da magia."
- **COLHESIANA (any save)** — "Um pequilho sempre pode contar com a própria sorte."
- **High entropy + Fortitude** — "Sua alma corrupta expurga o veneno."

For the full resist pipeline (Fortitude / Reflexos / Vontade, DCs,
modifiers) see [docs/resist-system.md](resist-system.md) and the spec
[docs/spec_resist_saves.md](spec_resist_saves.md).

---

## 12. Languages

`linguasOpt = [Imperialis, Aglâb, Ahruwën, Haiei, Phalk, Grumnak, Björ,
Idrith'har, Comum]`
([epicgumpcriacao.src:933-934](../pkg/systems/charactercreation/epicgumpcriacao.src#L933-L934)).

`parseLanguageByCulture(who, linguasOpt)` returns the cultural default
which is always known. INT ≥ 40 grants extras:

```
total_amount = 1 + ceil((INT - 30) / 30) + 1   # if INT >= 40
```

Plus an implicit "Comum" appended when INT ≥ 40. Druida class auto-adds
**Sylovir**
([epicgumpcriacao.src:970-972](../pkg/systems/charactercreation/epicgumpcriacao.src#L970-L972)).

---

## 13. RP traits — height / weight / eyes / voice / age

[RPTraits.inc](../pkg/systems/charactercreation/include/RPTraits.inc)
defines `GetRacialTraits(raca)` → race-specific category sets. Required
categories at creation are Altura, Peso, Olhos and Voz (Idade is a free
textbox). All four required plus a non-empty age must be entered before
the gump's "finalize" button (222) accepts.

Common categories (5 / 5 / 5 / 3 options respectively):

- **Altura** — race-specific 5 options
- **Peso** — race-specific 5 options
- **Olhos** — Castanho, Verde, Azul, Preto, Mel
- **Voz** — Aguda, Média, Grave

`ConvertAge(raca, idade)` maps the textbox value to an apparent age
category bucketed by race, then computes a real age (capped):

| Race      | Max age |
| --------- | ------- |
| Humano    |  75     |
| Orc       |  75     |
| Anão      | 150     |
| Pequilho  | 150     |
| Elfo      | 225     |
| Drow      | 225     |

Sample apparent-age categories for ANAO: Jovem adulto (~45), Adulto
(~65), Meia-idade (~85), Maduro (~105), Ancião (~125+).

---

## 14. Item inheritance

`itemInheritance(who)` in creationUtils.inc fans out into four functions:

1. **`genericItems`** — coins (1d4 Sabio / 1d8 Guerreiro / 1d12 Ladino),
   cultural outfit (robe + tunic + pants + footwear), torch, lantern,
   polish oil.
2. **`armorsByCulture`** — culture-specific armor pieces.
3. **`weaponsByClassAndCulture`** — weapon set picked from culture + the
   player's selected weapon skill (e.g. Gulthrak: Adaga + 1H Scepter +
   Shield + 2H Scythe/Grishnak + axe for Dual_Wield).
4. **`workingItems`** — crafting tools picked by culture/class.

These run **after** `applyClassStarter`, so class-specific grants
(grimoires, pistols, chisels…) are already on the player.

---

## 15. Sign and birthday

`signSelection` cycles 12 constellations (`0x152..0x15D`): Cuidador,
Governador, Herói, Bobo, Amante, Pessoa Comum, Criador, Mago, Explorador,
Fora da Lei, Inocente, Sábio.

`birthDaySelection` picks a day inside the month linked to the sign
(`getMonthBySign` maps sign → month index into the `MONTHS` array).
`setBirthdate` then runs `calculateBirthdate`
([epicgumpcriacao.src:979-1005](../pkg/systems/charactercreation/epicgumpcriacao.src#L979-L1005))
to fill the struct:

```
year  = current_year - idade_real
month = MONTHS[getMonthBySign(sign)].name
day   = chardata.birthdate.day
```

Initial age and current age are stamped here for the aging system.

---

## 16. Looks

[looksSelection](../pkg/systems/charactercreation/epicgumpcriacao.src#L638-L688)
sequences:

1. `resetHairAndBeard` — destroys any existing hair/beard layers.
2. Hair gump (with page nav 1001/1002, confirm 500, "no hair" 82).
3. Beard gump — gated on male gender (`who.gender == 0`) AND non-Elfo/non-Drow.
4. Hair & beard color gump — skipped entirely if both came back empty.
5. Skin color gump.

Hair item is created at the player's location with
`CreateItemAtLocation` and equipped via `EquipItem` so it is a real
LAYER_HAIR object, not just a CProp.

---

## 17. Citizenship

[setCitizenship](../pkg/systems/charactercreation/epicgumpcriacao.src#L1007-L1035)
stamps:

- `citizenship_title := "Plebeu"` (default).
- `available_citizenship_titles := [Banido, Indigente, Plebeu, Cidadão, Nobre]`.
- `nobility_titles` — gendered: male {Rei, Príncipe, Duque, Marquês, Conde,
  Visconde, Barão, Cavaleiro, Lorde, Escudeiro}; female {Rainha, Princesa,
  Duquesa, Marquesa, Condessa, Viscondessa, Baronesa, Dama, Lady, Escudeira}.
- `current_nobility_title := "Nenhum"`.
- `available_professional_positions := [Imperador, Mão do Rei,
  Conselheiro Nobre, Arcebispo, Senador, Serviçal]`.
- `current_professional_position := "Nenhum"`.

These are intended to be promoted later by social/political systems, not
chosen at creation.

---

## 18. World entry

[startAdventure](../pkg/systems/charactercreation/epicgumpcriacao.src#L905-L930)
hard-sets `Level := 1`, teleports the player to
`(1407, 1062, -4, britannia_alt)` via `MOVEOBJECT_FORCELOCATION`, grants
3 vision-adaptation potions (0xDCE6), plays a stationary FX +
sound (0x1F2), sleeps 2 s, then opens the start-adventure gump.

`creation_completed` is set immediately after this returns
([epicgumpcriacao.src:91](../pkg/systems/charactercreation/epicgumpcriacao.src#L91)).
Until that flag is set, **any logon will reopen `epicCreationGump`** —
`chardata` alone is the in-progress sentinel, not a "done" marker.

---

## 19. Known cross-cutting quirks

- **Skill secondary value comment is wrong** — code at
  [line 512](../pkg/systems/charactercreation/epicgumpcriacao.src#L512)
  sets 20, not the 10 the nearby comment claims.
- **Trait struct uses lowercase `.nome`** — reading `.Nome` silently
  returns nothing and breaks Sabe-Tudo's 2P+5S slot rebuild
  (see [comment at line 441](../pkg/systems/charactercreation/epicgumpcriacao.src#L441)).
- **`createdate` is conditional**, not unconditional — Story Mode's
  age-based skill caps depend on it, and the migration stone preserves it
  ([lines 77-85](../pkg/systems/charactercreation/epicgumpcriacao.src#L77-L85)).
- **Legacy `chardata.classe`** is still written
  ([line 376](../pkg/systems/charactercreation/epicgumpcriacao.src#L376))
  so old ability registries and `pathOfWisdom`-era checks keep working.
  New code should read `chardata.subclasse`.
- **`Dancarino_das_Sombras`** shares the 1800-1804 ability tree with
  Deslocador_das_Sombras but is **NOT** a caster — only the Sabio variant
  gets the spellbook
  ([lines 819-825](../pkg/systems/charactercreation/epicgumpcriacao.src#L819-L825)).

---

## See also

- [docs/ability-pipeline.md](ability-pipeline.md) — abilities granted via
  level-up after creation
- [docs/resist-system.md](resist-system.md) — Fortitude / Reflexos / Vontade
- [docs/religion-system.md](religion-system.md)
- [docs/spells-pipeline.md](spells-pipeline.md) — spell flow that Phase-3
  caster classes plug into via `chardata.magia`
