# Audit B — `src/30-caminhos-classes.md` (Caminhos, Classes, Habilidades de Caminho)

Scope: Os Três Caminhos, Classes, Habilidades de Caminho, Especializações em
Armas / Armaduras, Estilos de Combate, Escudo e Montaria, Conjuração, Extração,
Agropecuária, Habilidades Diversas.

Code entry points verified:
`pkg/systems/abilities/include/AbilityRegistryCaminho.inc`,
`.../models/Ability.inc`, `.../classes/caminho/*.inc`,
`pkg/systems/combat/include/Controller/HitController.inc` &
`ParryController.inc`, `DamageController_legacy.inc`, `cast_spell.src`,
`mantainSpell.src`, `scripts/include/epicMageryUtils.inc`,
`pkg/systems/attributes/hooks/vitalInit.src`, gathering scripts under
`pkg/skills/gathering/`, `pkg/tn/colheReagente.src`, crafting toolWear,
`pkg/mobiles/death/`, `pkg/systems/abilities/configs/habilidades.cfg`.

The path/ability model ("Os Três Caminhos" / "Habilidades de Caminho") matches
code: every ability the file names exists in `ABILITY_ID` (3000-3628) and is
registered in `AbilityRegistryCaminho.inc`, **with one exception** (Corte
Limpo, below). The "(Acumulável)" markings match `IsPool := true` for Weapon
Specs and Elemental Specs; School Specs and Combat Styles are correctly NOT
accumulable in both book and code.

---

### Estilo de Arma Única — wrong effect (no crit, undocumented damage)
- **Handbook claim:** "Ao usar uma arma de uma mão sem escudo: +5 de chance de crítico e -2 na chance de acerto do inimigo." — src/30-caminhos-classes.md:249
- **Codebase truth:** `SingleWeaponStyle.inc` (3100) grants **+3 damage**
  (`OnPassiveBeforeDamage`, IncreaseSlash/Piercing/BluntRawDamage(3)) and **-2
  enemy hit** (`OnPassiveBeforeHitDefender`). There is **no critical-chance
  modifier anywhere in the class**. — pkg/systems/abilities/include/classes/caminho/SingleWeaponStyle.inc:38-55
- **Mismatch:** The "+5 de chance de crítico" does not exist. The real bonus is
  +3 flat damage, which the book never mentions. Only the "-2 acerto inimigo"
  is correct.
- **Severity:** HIGH (misleads a build decision — players pick this for crit; it gives none)
- **Cross-links:** none

### Estilo de Espada e Escudo — wrong effect (no damage, undocumented parry)
- **Handbook claim:** "Ao usar uma arma de uma mão com escudo: +2 de dano e -2 na chance de acerto do inimigo." — src/30-caminhos-classes.md:260
- **Codebase truth:** `SwordAndBoard.inc` (3101) grants only **-2 enemy hit**
  (`OnPassiveBeforeHitDefender`). The actual offensive payoff is **+5 parry
  chance**, applied inline in `ParryController.CalculateParryChance` when the
  defender holds 1H + shield. There is **no damage modifier**. —
  pkg/systems/abilities/include/classes/caminho/SwordAndBoard.inc:32-40;
  pkg/systems/combat/include/Controller/ParryController.inc:161-172
- **Mismatch:** "+2 de dano" does not exist; the real second bonus is +5 parry
  chance, undocumented. Only "-2 acerto inimigo" is correct.
- **Severity:** HIGH (misleads a build decision — sold as a damage style, is actually a defensive parry style)
- **Cross-links:** none

### Combate Montado — omits the two-source 50/25/0 ladder
- **Handbook claim:** "Reduz a penalidade de combate montado em 25%." — src/30-caminhos-classes.md:368
- **Codebase truth:** `HitController.inc:_GetSkillForCombat` cuts the mounted
  attacker's combat skill on a 3-tier ladder gated by TWO independent sources —
  Característica "Cavalaria de Escaramuça" AND Habilidade "Combate Montado":
  neither → skill / 2 (−50%); exactly one → skill * 75/100 (−25%); both → no
  penalty. — pkg/systems/combat/include/Controller/HitController.inc:87-97
- **Mismatch:** The penalty is not a flat "−25% of the penalty". With the
  ability alone the penalty goes 50% → 25% (one of two gates). It only reaches
  zero if the player ALSO has the trait "Cavalaria de Escaramuça". The book
  presents Combate Montado as a standalone partial reducer and never mentions
  the trait dependency or that full removal requires both.
- **Severity:** HIGH (misleads a build/combat decision; this is the calibration case from _FORMAT.md applied to the ability side)
- **Cross-links:** `src/71-caracteristicas.md:169` and
  `src/21-povos/02-bjorske.md:128` and `src/21-povos/07-povos-do-leste.md:164`
  state the trait alone gives "não recebe penalidades" — the inverse half of
  the same false model (trait alone is only 50→25, not zero). Audit of those
  files owns the trait-side finding; this is the ability-side cross-link.

### Corte Limpo — ability does not exist in code (invented; duplicates Lenhador Talentoso)
- **Handbook claim:** "### Corte Limpo … Você corta lenha com tanta precisão que consegue retirar mais toras de madeira por árvore cortada." — src/30-caminhos-classes.md:524-531
- **Codebase truth:** No `CORTE_LIMPO` / `CLEAN_CUT` `ABILITY_ID`, no class, no
  registry entry, no config block. The Extrator block is 8 abilities (IDs
  3560,3561,3563,3565,3566,3567,3568,3569 — `habilidades.cfg:2158-2230`). The
  effect "more wood per tree" is exactly **Lenhador Talentoso /
  TALENTED_LUMBERJACK (3568)**: `lumberjack.src:131 amt := amt + 4`. —
  pkg/systems/abilities/include/models/Ability.inc:427-435;
  pkg/systems/abilities/configs/habilidades.cfg:2158-2230;
  pkg/skills/gathering/chopTrees/lumberjack.src:128-138
- **Mismatch:** "Corte Limpo" is a phantom 9th Extração ability. No code
  enforces it; its described effect is a verbatim duplicate of "Lenhador
  Talentoso" (src/30-caminhos-classes.md:577-584), which is the real ability.
- **Severity:** INVENTED (named ability with no code; duplicate of an existing one)
- **Cross-links:** src/30-caminhos-classes.md:577-584 ("Lenhador Talentoso", the real ability that does this)

### Herbalista Experiente — "dobro" vs +1
- **Handbook claim:** "Ao colher um reagente vegetal, você tem 30% de chance de obter o dobro." — src/30-caminhos-classes.md:698
- **Codebase truth:** `EXPERT_HERBALIST` (3609): 30% roll adds **+1** to the
  amount, not double. `if (RandomInt(100) < 30) amt := amt + 1;` —
  pkg/tn/colheReagente.src:21-25
- **Mismatch:** "o dobro" (double the yield) is wrong; it is a flat +1 on a
  successful 30% roll. Harvesting 4 yields 5, not 8. The 30% chance itself is
  correct.
- **Severity:** MED (misleading magnitude, low-stakes utility ability — not a combat/build pivot)
- **Cross-links:** none — note "Sorte na Coleta" (HARVEST_LUCK, src:676) is the
  one that genuinely doubles harvest/honey/reagent yields; do not conflate.

### Especialização em Escudo — omits the shield-equipped gate
- **Handbook claim:** "Reduz o cooldown de aparar em 20%." — src/30-caminhos-classes.md:357
- **Codebase truth:** `ShieldSpecialization.inc` (3300) only fires
  `OnPassiveOnParry` when `who.shield` is true; otherwise returns false (no
  reduction). The 20% (min 1s) figure is correct. —
  pkg/systems/abilities/include/classes/caminho/ShieldSpecialization.inc:28-44
- **Mismatch:** The reduction is conditional on actively parrying *with a
  shield equipped*. The book states it unconditionally; a player parrying with
  a weapon gets nothing.
- **Severity:** MED (build-relevant prerequisite the book omits, but the ability name implies a shield anyway)
- **Cross-links:** none

### Especialização em Armadura Pesada — flat "2" vs per-piece +1
- **Handbook claim:** "Reduz a penalidade de destreza em 2 ao usar armadura pesada." — src/30-caminhos-classes.md:342
- **Codebase truth:** `HeavyArmorSpec.OnPassiveOnEquip` calls
  `equip.ApplyAbilityBonus(DEXTERITY, 1)` **per heavy piece** equipped (also +1
  parry per heavy piece in ParryController). It is not a single flat "2". —
  pkg/systems/abilities/include/classes/caminho/HeavyArmorSpec.inc:25-29;
  pkg/systems/combat/include/Controller/ParryController.inc:150-158
- **Mismatch:** Structural model differs: the book describes one fixed −2 dex
  offset; code restores +1 dex per heavy piece (can exceed 2 with a full set)
  and additionally grants parry. Severity kept low because the book
  intentionally abstracts numbers and the direction (mitigates heavy-armor dex
  loss) is correct.
- **Severity:** LOW (imprecise model, intentional number abstraction)
- **Cross-links:** src/30-caminhos-classes.md:331 — Medium armor states "Reduz
  a penalidade de destreza em 1"; code is identically +1 dex *per medium piece*
  (`MediumArmorSpec.inc:21-25`), same per-piece-vs-flat imprecision.

---

## Verified accurate (no mismatch — recorded for coverage)

- **Weapon Specializations** (14 categories, src:87-237): `WeaponSpecBase`
  grants +count hit and +count damage scaling with pool stacks; all 14
  categories present and `IsPool`. Firearm correctly matches
  pistol/musket/carabine/handcannon. — WeaponSpecialization.inc:27-178
- **Estilo de Duas Mãos** (src:271): +3 damage, +5 crit — matches
  TwoHandedStyle.inc:32-45.
- **Estilo Duplo** (src:282): +3 hit, -2 enemy hit — matches DualWieldStyle.inc:33-58.
- **Estilo à Distância** (src:293): +2 hit, +2 damage — matches RangedStyle.inc:30-58.
- **Brigueiro de Taverna** (src:304): +4 damage unarmed/wrestling — matches TavernBrawler.inc:32-46.
- **Armadura Leve/Média evasão** (src:320,331): −2 / −1 enemy hit per piece —
  matches Light/MediumArmorSpec `OnPassiveBeforeHitDefender`.
- **Mente Tranquila** (src:383): doubles mana regen (`base_regen + base_regen`)
  — vitalInit.src:85-87.
- **Conjurador de Suporte** (src:394): maintain at half mana
  (mantainSpell.src:81-82) + 50% mana refund on fail
  (cast_spell.src:249-251,431-433,446-448) — matches.
- **Magia de Alcance** (src:405): +5 tiles — epicMageryUtils.inc:97-100.
- **Espec. Elemental Fogo/Gelo/Raio/Veneno** (src:416-449): +2 dmg per pool
  stack — DamageController_legacy.inc:446-451 (`dmg + (count * 2)`), all 4
  elements mapped, file actively included.
- **Espec. Escola Evocatium/Velkyn/Ahtalelorëth/Kaijin** (src:460-493): −10
  cast difficulty — cast_spell.src:574-577, all 4 schools mapped.
- **Extração** Vigor/Minerador de Joias/Minerar Raridades/Tesouro do
  Lixo/Carregador/Minerador Talentoso/Lenhador Talentoso/Prospector — all 8
  exist and effects fire (Pack Mule +60 `MobileWrapper.inc:1147`, Talented
  Miner +2 `oremining/mining.src:180`, Talented Lumberjack +4
  `lumberjack.src:131`, Gem Miner ~8-10% `mining.inc:19`, Prospector/Treasure
  in `smelt.src`/`mining.src`).
- **Agropecuária** Campestre (Ativa, 10 stam, 120s — habilidades.cfg:2232-2241
  exactly matches src:608-611), Dedo Verde, Agricultor Talentoso, Sem
  Desperdício, Replantio Garantido, Pastor, Sorte na Coleta — all registered
  and effect-stubbed to gathering/farming scripts.
- **Habilidades Diversas** Arqueólogo, Recuperação (½ revive time +90% HP vs
  20% — wakeup.inc:56-58, players.src:207-209), Arquiteto, Preservar
  Ferramentas (50% `randomint(2)==0` — toolWear.inc:29-33), Escrever
  Pergaminhos, Ervas Curativas, Formação Acadêmica — all exist and fire.

UNVERIFIED (effect stubbed to scripts not exhaustively traced for exact
magnitude, but ability exists and is consumed): Vigor do Extrator stamina
discount %, Dedo Verde grow-step, Agricultor Talentoso +20, Arqueólogo /
Formação Acadêmica / Escrever Pergaminhos / Ervas Curativas / Arquiteto exact
mechanics. None contradicted by code; flagged only as not numerically
pinned-down.

Note: code ability `FOCO_ARCANO` (3402, −20% mana w/ Staff/SpiritualBlade/
MagicBow) exists and is registered but is **absent from the handbook's
Conjuração section** — a documentation gap, not a handbook mismatch (book makes
no false claim about it).

---

## Summary

| Severity | Count | Findings |
|----------|-------|----------|
| HIGH     | 3 | Estilo de Arma Única (no crit), Estilo de Espada e Escudo (no dmg), Combate Montado (two-source ladder) |
| MED      | 2 | Herbalista Experiente (+1 not double), Especialização em Escudo (shield gate omitted) |
| LOW      | 1 | Armadura Pesada/Média (per-piece +1 vs flat) |
| INVENTED | 1 | Corte Limpo (ability does not exist; duplicates Lenhador Talentoso) |
| **Total**| **7** | |

Top 5 by impact:
1. **Estilo de Arma Única** (HIGH) — advertised crit bonus does not exist; real
   bonus (+3 dmg) undocumented. Direct combat-build deception.
2. **Estilo de Espada e Escudo** (HIGH) — advertised +2 dmg does not exist;
   real payoff is +5 parry. Sold as offense, is defense.
3. **Combate Montado** (HIGH) — hides the trait dependency; ability alone never
   removes the mounted penalty fully. Cross-contaminates 3 other src files.
4. **Corte Limpo** (INVENTED) — phantom Extração ability, duplicate of Lenhador
   Talentoso; players will try to buy a slot that doesn't exist.
5. **Herbalista Experiente** (MED) — "dobro" overstates a flat +1; sets wrong
   farming expectations.
