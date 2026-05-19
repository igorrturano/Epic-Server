# Handbook ↔ Codebase Audit — Perícias (`src/72-pericias.md`)

Scope: every skill description + mechanical claim in the Perícias chapter,
verified against `pkg/skills/`, `pkg/systems/`, char-creation config and the
spell/alchemy scaling code.

## Context the chapter depends on

Two skill-tier models coexist in `classes.cfg`:

- **Legacy archetype tiers** (`classes.cfg:1-110`): `Guerreiro / Ladino /
  Sabio / Artifice` each list `P/S/T/Q` skills. These are read only by
  `creationUtils.inc::skillGump` (line 743) — a function that is **not called**
  by the active creation flow (`epicgumpcriacao.src:60` calls
  `skillSelection`, not `skillGump`). Effectively dead for new chars.
- **Active path-bucket model** (Phase 2 rework): char creation picks
  3 Primary + 3 Secondary skills from `MILITARY_SKILLS` / `CIVIC_SKILLS`
  buckets (`epicConstants.inc:83-93`), gated only by the char's *path*
  (Militar/Civil), via `creationUtils.inc::BuildSkillSlots` (912-940) →
  `SkillsForBucket` (946-961). The 29 specific subclasses (`classes.cfg:127-340`)
  carry only `archetype`, `dice`, `description` — **no per-subclass skill
  list anywhere in code.**

Net consequence: **no code enforces any "skill X is required/necessary for
class Y" rule.** Skills are bought/raised freely; the only gating is path
bucket at creation and the per-tier cap. The handbook's pervasive
`[Necessário para classe: …]` / `[Necessário para classes: …]` tags assert a
restriction the code does not implement. Where a tag also mis-states the
underlying mechanic (e.g. which skill a class actually rolls to cast) it is
flagged individually below; the blanket "invented requirement" pattern is
flagged once as Finding 1.

All skill *names* in the chapter exist as real Attributes
(`attributes.cfg:65-279`): One_Handed, Two_Handed, Dual_Wield, Ranged,
Tactics, Parry, Magery, Arcana, Theology, Mechanics, Awareness,
Handle_Animal, Pickpocketing, Survival, Medicine, Sneak, Gathering,
Metal_Working, Wood_Working, Domestic_Work, Alchemy, Science, Wrestling.
No skill named in the chapter is missing or misnamed at the attribute level.

---

## Findings

### 1. "[Necessário para classe X]" tags assert an unenforced requirement
- **Handbook claim:** Repeated across the chapter, e.g. *"**[Necessário para classes: Combatente, Duelista e Monge]**"* — src/72-pericias.md:44; *"**[Necessário para classe: Defensor e Samurai]**"* — :48; *"**[Necessário para classe: Caçador de tesouros]**"* — :67; *"**[Necessário para classes: Ladrão]**"* — :79; *"**[Necessário para classes: Assassino]**"* — :93; *"**[Necessário para classes: Construtor de Golems]**"* — :105; *"**[Necessário para classes: Granadeiro, Vivisseccionista, Mutante]**"* — :116.
- **Codebase truth:** Char creation assigns skill tiers from path buckets only (`creationUtils.inc:BuildSkillSlots` 912-940, `SkillsForBucket` 946-961, `epicConstants.inc:83-93`); subclasses in `classes.cfg:127-340` declare no skills. The legacy per-archetype P/S/T/Q reader (`skillGump`, creationUtils.inc:743) is dead (not in the `epicgumpcriacao.src:60` flow). No code keys any skill to a specific subclass.
- **Mismatch:** The book frames many skills as gated to/required by named subclasses. No mechanic enforces "this skill is necessary for class Y" — any class can take any skill subject only to path bucket + tier cap. The list of class names is not derived from code.
- **Severity:** MED (misleads build planning, but the skills themselves work as described; not a combat-math error)
- **Cross-links:** src/72-pericias.md:28, 32, 36, 40, 44, 48, 52, 59, 62, 67, 71, 75, 79, 83, 88, 93, 105, 109, 116, 119 — every bracketed class-requirement tag in the chapter.

### 2. Magery "necessário" for Clérigo, Xamã e Druida — those cast off Theology
- **Handbook claim:** *"Magery (Magia) … **[Necessário para classes: Magus, Lâmina Arcana, Necromante, Deslocador das Sombras, Clérigo, Xamã e Druida]**"* — src/72-pericias.md:52
- **Codebase truth:** Casting success rolls a skill chosen by spell *Type*, not class: `cast_spell.src:558-563` — default `MAGERY`; if arcane-sensitive/`VOX_ARCANA` → `ARCANA`; **if `spellinfo.Type == "Divine"` → `THEOLOGY`**. Clérigo (priest), Xamã (shaman) and Druida (druid) spell trees are Divine-type and consume Theology (52 druid/priest/shaman spell files reference THEOLOGY). The book's own Theology entry (src:62) lists *"Clérigo, … Xamã e Druida"* as scaling off Theology — directly contradicting putting them under Magery.
- **Mismatch:** Clérigo/Xamã/Druida do not need/use Magery; their casting skill is Theology. The Magery class list wrongly includes three Theology classes (internally contradicted by the chapter's own Theology entry).
- **Severity:** HIGH (a Clérigo/Druida/Xamã player who raises Magery instead of Theology will fail casts — directly misleads a build decision)
- **Cross-links:** src/72-pericias.md:62 (Theology entry correctly lists Clérigo/Xamã/Druida — the contradictory copy).

### 3. Arcana "escalam diretamente o dano/efeito de magias" — true but conditional
- **Handbook claim:** *"Arcana (Arcanismo) … Além disso, escalam diretamente o dano/efeito de magias."* — src/72-pericias.md:59
- **Codebase truth:** Arcane spells scale off Arcana (e.g. `fireball.src:32` `AP_GetSkill(caster, ARCANA)/20`, `:58` radius `ARCANA/25`; `getSupportingSkill` cast_spell.src:602-615 keeps Arcana as supporting skill only for Mago / spellcaster magia / arcane-sensitive). Divine spells scale off **Theology** instead (`luzdivina.src:19-22` `AP_GetSkill(who, THEOLOGY)`, `armasagrada.src:21`). So Arcana scales *arcane* magic damage/effect, not all magic.
- **Mismatch:** "diretamente o dano/efeito de magias" reads as universal; it only governs arcane-school magic. Divine magic ignores Arcana and scales on Theology. Low-stakes because the chapter separately documents Theology's scaling.
- **Severity:** LOW (imprecise scope wording; the Theology entry covers the other half)
- **Cross-links:** src/72-pericias.md:62 (Theology scaling), :119 (Science scaling — parallel "potência" wording).

### 4. Mechanics — "armadilhas, fechaduras" claim verified, no mismatch
- **Handbook claim:** *"construir, reparar e desarmar dispositivos mecânicos, como armadilhas, fechaduras e máquinas"* — src/72-pericias.md:67
- **Codebase truth:** `mechanicalapitude.src:14 program skillremoveTrap` (disarm traps) and `lockPicking.src` (`:mechanicalapitude:lockpick_minigame`, key/traps includes) confirm trap-disarm + lockpicking under the Mechanics attribute.
- **Mismatch:** None on function. (The `[Necessário para classe: Caçador de tesouros]` tag is covered by Finding 1.)
- **Severity:** — (no finding; recorded as verified)
- **Cross-links:** none

### 5. Science — potion potency scaling verified
- **Handbook claim:** *"Science representa a potência dos seus alquímicos … podem potencializar ainda mais poções de cura, venenos, soluções explosivas"* — src/72-pericias.md:119
- **Codebase truth:** `explosionPotion.src:131` `var lvl := CInt(AP_GetSkill(who, SCIENCE)/10)` drives explosive potion level; `alchemy.inc:344 PotionSpecialistBonus` and `science.src` (skill_science identifies/empowers potions) confirm Science governs potion potency tier. Matches description.
- **Mismatch:** None on function. (Class-requirement tag covered by Finding 1.)
- **Severity:** — (no finding; recorded as verified)
- **Cross-links:** src/72-pericias.md:115 (Alchemy = recipe/"saber fazer"), correctly distinguished from Science.

### 6. Sneak / Survival / Medicine / Awareness / Theology functions verified
- **Handbook claim:** Sneak = move silently/unseen (:93); Survival = wilderness, tracking, skinning implied (:83); Medicine = heal wounds/first aid (:88); Awareness = perceive surroundings/avoid surprise (:71); Theology = divine casting + altar interaction + religious lore (:62).
- **Codebase truth:** Sneak → `sneak.src` hide/stealth, `SkillCheck(who, SNEAK)` + Stealth, range table 190-211. Survival → `skinning.src:126/167/173` `AP_GetSkill(who, SURVIVAL)`, plus camping/bedroll. Medicine → `healing.src:36-214` `SkillCheck(mobile, MEDICINE, …)`. Awareness → `awareness.src skill_Alertness` LOS detection. Theology → `theology.src` sacred/cursed item ID; `cast_spell.src:561-562` divine casting roll; `luzdivina.src:19-22` duration scales with THEOLOGY.
- **Mismatch:** None — descriptions match implemented behavior.
- **Severity:** — (no finding; recorded as verified)
- **Cross-links:** none

### 7. Gathering described as ore + wood + stone; engine splits the work
- **Handbook claim:** *"Gathering (Coleta) … reunir materiais brutos … como toras de árvore, minérios e pedras."* — src/72-pericias.md:96
- **Codebase truth:** Gathering exists as a single Attribute (`attributes.cfg:235`) and the legacy archetype tiers reference `Gathering` (`classes.cfg:18` etc.). But the active path-bucket creation model has **no `Gathering` skill** — `CIVIC_SKILLS` (`epicConstants.inc:90-92`) splits it into separate `Mining` and `Lumberjacking` skills. `habilidades.inc:589` maps legacy `Gathering → "Civil"` as a compat alias. So a new char never picks "Gathering"; it picks Mining and/or Lumberjacking, which are the live extraction skills.
- **Mismatch:** The chapter presents "Gathering" as one acquirable skill covering wood+ore+stone. Under the active model it is two distinct picks (Mining, Lumberjacking); "Gathering" survives only as a legacy umbrella attribute. Imprecise but not build-breaking (a player still ends up with the extraction capability).
- **Severity:** LOW (naming/granularity drift between legacy attribute and active bucket)
- **Cross-links:** Finding 1 (path-bucket model), Finding 8.

### 8. Sneak / Handle_Animal / Pickpocketing / Metal_Working / Wood_Working / Domestic_Work names drift from active bucket names
- **Handbook claim:** Chapter uses Sneak (:93), Handle Animal (:75), Pickpocketing (:79), Metal Working (:105), Wood Working (:109), Domestic Work (:111) as the acquirable skill names.
- **Codebase truth:** Active creation buckets (`epicConstants.inc:83-93`) use `Stealth`, `Taming`, `Theft`, `Smithing`, `Carpentry`, `Tailoring`. `habilidades.inc:583-592` BuildSkillPathLookup maps the handbook's legacy names as compat aliases (`Sneak`, `Handle_Animal`, `Pickpocketing`, `Metal_Working`, `Wood_Working`, `Domestic_Work`). Both name sets resolve to the same underlying capability, but a new player choosing skills sees the bucket names, not the handbook's.
- **Mismatch:** Handbook skill labels lag the active path-rework names. Same mechanic, divergent surface naming — a player won't find "Sneak"/"Metal Working" in the creation picker (they see Stealth/Smithing).
- **Severity:** LOW (cosmetic/terminology lag, not a mechanical contradiction)
- **Cross-links:** Finding 1, Finding 7.

### 9. Survival "[Necessário para classes: Aklat'tur e Caçador]" — Aklat'tur not a class
- **Handbook claim:** *"Survival (Sobrevivência) … **[Necessário para classes: Aklat'tur e Caçador]**"* — src/72-pericias.md:83
- **Codebase truth:** The subclass catalog (`classes.cfg:127-340`) has `Aklattur` (Guerreiro archetype, dice d10) as a player class — that name is valid. (Per project memory, the *Aklat'tur monster templates* are environmental, but the playable class `Aklattur` does exist here.) The requirement itself is still unenforced per Finding 1; the class name is otherwise legitimate.
- **Mismatch:** No standalone mismatch beyond Finding 1 — recorded to pre-empt a false "Aklat'tur isn't a class" flag. The `Aklattur` class exists; only the "necessário" framing is the issue (Finding 1).
- **Severity:** — (no separate finding; clarification)
- **Cross-links:** Finding 1.

---

## Summary

| Severity | Count | Findings |
|----------|-------|----------|
| HIGH     | 1     | #2 (Magery list wrongly includes Theology classes Clérigo/Xamã/Druida) |
| MED      | 1     | #1 (blanket unenforced "necessário para classe X" requirement, ~20 sites) |
| LOW      | 3     | #3 (Arcana scaling scope), #7 (Gathering granularity), #8 (skill-name drift) |
| INVENTED | 0     | (Finding 1 is the invented-rule pattern, scored MED for low stakes) |
| Verified (no mismatch) | 4 | #4 Mechanics, #5 Science, #6 Sneak/Survival/Medicine/Awareness/Theology, #9 Aklattur exists |

### Top 5 by impact

1. **#2 — Magery class list (HIGH).** Clérigo/Xamã/Druida cast Divine-type
   spells that roll vs **Theology** (`cast_spell.src:561-562`), not Magery.
   A player of those classes raising Magery instead of Theology will fail
   casts. The chapter even contradicts itself (Theology entry, src:62).
2. **#1 — Unenforced per-class skill requirements (MED).** ~20 bracketed
   `[Necessário para classe …]` tags assert restrictions no code enforces;
   the live model is path-bucket + tier cap, subclass-agnostic.
3. **#3 — Arcana "escala dano/efeito de magias" (LOW).** True only for
   arcane-school spells; divine magic scales on Theology
   (`luzdivina.src:19-22`).
4. **#7 — Gathering (LOW).** Presented as one wood+ore+stone skill; active
   model splits it into Mining + Lumberjacking; "Gathering" is a legacy
   compat alias only.
5. **#8 — Skill-name drift (LOW).** Sneak/Handle Animal/Pickpocketing/Metal
   Working/Wood Working/Domestic Work are legacy aliases; the creation
   picker shows Stealth/Taming/Theft/Smithing/Carpentry/Tailoring.
