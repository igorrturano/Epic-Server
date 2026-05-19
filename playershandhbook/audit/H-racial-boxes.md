# Audit H — Racial perk boxes (`src/21-povos/`)

Scope: the mechanical perk boxes in all 8 povo files — the "Características e
Passado" / "Vantagens e Desvantagens Raciais" boxes plus the mechanical
`> - **Trait**: effect` lines. Pure lore/history prose (Origem, Religião,
Comportamento flavor, the multi-page narrative) is intentionally **not** audited.

Each povo box contains exactly one **Característica** (racial trait) and one
**Passado** (skill-cost background). Verification sources:
- Traits: `chardata.trait` string equality checks scattered across combat /
  skills / nature / attributes; canonical names in
  `pkg/systems/charactercreation/config/traits.cfg`.
- Passados: `pkg/systems/charactercreation/config/backgrounds.cfg` (Desc =
  "Reduz o custo de SKILL para 1"), selected at creation via
  `epicgumpcriacao.src:380` `backgroundSelection`.

All 8 Passado claims were verified accurate (see "Passados — all verified"
below). All findings concern the trait/Característica boxes.

---

## Aiglanos (`src/21-povos/00-aiglanos.md`)

### Força da Legião — accurate
- **Handbook claim:** "Você recebe apenas metade da penalidade usando Scutum" — src/21-povos/00-aiglanos.md:202
- **Codebase truth:** With trait `"Força da Legião"` and a Scutum objtype
  (`0x1B76 / 0x1B77 / 0xFA62`), the shield's DEXTERITY modifier is halved
  (`CInt(dexMod / 2)`) — pkg/systems/equipsys/equipitem.src:36-43 (also
  unequipitem.src:35).
- **Mismatch:** None. The penalty halved is specifically the DEX malus and
  only for the three Scutum objtypes; the book's wording is correct and the
  abstraction is acceptable.
- **Severity:** — (no finding)
- **Cross-links:** src/71-caracteristicas.md:155-158; traits.cfg:176-184.

### Passado: Legionário — accurate
- **Handbook claim:** "Custo 1 para *Parry*." — src/21-povos/00-aiglanos.md:203
- **Codebase truth:** backgrounds.cfg:85-89 — `Legionário` → "Reduz o custo
  de PARRY para 1".
- **Mismatch:** None.
- **Cross-links:** src/70-passados.md:136-140.

---

## Baduran (`src/21-povos/01-baduran.md`)

### Defensor Anão — accurate
- **Handbook claim:** "Você fica imune a críticos e paralisias, mas causa
  -30% de dano." — src/21-povos/01-baduran.md:140
- **Codebase truth:** All three effects implemented:
  - −30% damage dealt: `penalty := 0.7` → damage reduced 30% —
    pkg/systems/combat/include/Controller/DamageController_legacy.inc:1232-1237
  - Crit immunity (as defender): "Você ignora o crítico!" —
    DamageController_legacy.inc:1290-1295
  - Paralysis immunity: early `return` —
    pkg/mobiles/timedScripts/subScripts/paralysis/paralysis.src:16-18,
    .../paralysis/congelamento.src:16-18, and SC pararotempo.src:37-39
- **Mismatch:** None. Fully accurate including the downside.
- **Severity:** — (no finding)
- **Cross-links:** src/71-caracteristicas.md:190-193; traits.cfg:213-221.

### Passado: Filho do Progresso — accurate
- **Handbook claim:** "Reduz o custo de *Alchemy*" — src/21-povos/01-baduran.md:142
- **Codebase truth:** backgrounds.cfg:73-77 — `Filho do progresso` → ALCHEMY
  cost 1.
- **Mismatch:** None (cfg stores name lowercase-p "Filho do progresso" vs
  handbook "Filho do Progresso"; doc-internal cosmetic only — passado is
  selected by gump button, not string-compared at runtime).
- **Cross-links:** src/70-passados.md:120-124.

---

## Bjorske (`src/21-povos/02-bjorske.md`)

### Cavalaria de Escaramuça — false "no penalty" + invented bow rule
- **Handbook claim:** "Você não recebe penalidades no combate montado,
  podendo até usar arcos." — src/21-povos/02-bjorske.md:128
- **Codebase truth:** Mounted combat skill is cut on a 3-tier ladder gated by
  TWO independent sources — the trait `"Cavalaria de Escaramuça"` AND the
  ability `MOUNTED_COMBAT` —
  pkg/systems/combat/include/Controller/HitController.inc:87-97:
  neither → skill / 2 (−50%); exactly one → skill * 75/100 (−25%); BOTH →
  no penalty. Trait alone is **−25%, not zero**.
- **Mismatch:** "não recebe penalidades" is false for the trait alone — zero
  penalty additionally requires the Habilidade "Combate Montado"
  (`MOUNTED_COMBAT`). This is the exact calibration case from `_FORMAT.md`.
- **Severity:** HIGH (misleads a build/combat decision)
- **Cross-links:** src/71-caracteristicas.md:169 (same false claim, the
  primary copy — calibration Mismatch 1); src/21-povos/07-povos-do-leste.md:164
  (restates it); traits.cfg:195 is the upstream origin of the wrong string.

### "podendo até usar arcos" — invented rule
- **Handbook claim:** "podendo até usar arcos" — src/21-povos/02-bjorske.md:128
- **Codebase truth:** No code blocks bows/ranged attacks while mounted. No
  mounted restriction exists in any archery/ranged-attack path; only the
  HitController skill penalty applies (same for any weapon). The only
  trait-gated mounted block is `arremessar.src:41` — that gates the
  *throwing* spell command, not bows, and is bypassed by the trait there.
- **Mismatch:** The phrasing implies bows are otherwise forbidden while
  mounted and the trait lifts that — no such restriction is enforced anywhere
  (same as `_FORMAT.md` calibration Mismatch 3).
- **Severity:** INVENTED (rule with no code)
- **Cross-links:** src/71-caracteristicas.md:169; src/21-povos/07-povos-do-leste.md:164
  ("até mesmo com arcos" — same invented framing); traits.cfg:195.

### Passado: Cavalaria Nórdica — accurate
- **Handbook claim:** "Reduz o custo de *Handle Animal* para 1" — src/21-povos/02-bjorske.md:129
- **Codebase truth:** backgrounds.cfg:91-95 — `Cavalaria Nórdica` →
  HANDLE_ANIMAL cost 1.
- **Mismatch:** None.
- **Cross-links:** src/70-passados.md:144-148.

---

## Haroloth (`src/21-povos/03-haroloth.md`)

### Oito olhos — accurate
- **Handbook claim:** "Recebe um bônus em Awareness." — src/21-povos/03-haroloth.md:135
- **Codebase truth:** trait `"Oito olhos"` adds `base_skill += 10` when the
  queried skill is AWARENESS —
  pkg/systems/attributes/include/skillCheck.inc:115-118. traits.cfg:164/170
  defines it as "Recebe um bônus em awareness (+10)".
- **Mismatch:** None. (Note: 71-caracteristicas.md:144 header is "Oito Olhos"
  with capital O; the runtime/stored string and povo box use "Oito olhos".
  Cosmetic doc inconsistency only — not load-bearing, gameplay compares
  against the traits.cfg `Nome` which is "Oito olhos".)
- **Severity:** — (no finding)
- **Cross-links:** src/71-caracteristicas.md:144-147; traits.cfg:162-172.

### Passado Prole Abissal — accurate
- **Handbook claim:** "Reduz o custo de *Awareness* para 1" — src/21-povos/03-haroloth.md:137
- **Codebase truth:** backgrounds.cfg:49-53 — `Prole Abissal` → AWARENESS
  cost 1.
- **Mismatch:** None.
- **Cross-links:** src/70-passados.md:87-92.

---

## Aluriel (`src/21-povos/04-aluriel.md`)

### Conexão com Alora — accurate
- **Handbook claim:** "Você recebe +10 mana." — src/21-povos/04-aluriel.md:224
- **Codebase truth:** trait `"Conexão com Alora"` adds `vital_points + 1000`
  to max mana; mana is stored ×100, so +1000 = +10 displayed mana —
  pkg/systems/attributes/hooks/vitalInit.src:101-103.
- **Mismatch:** None.
- **Severity:** — (no finding)
- **Cross-links:** src/71-caracteristicas.md:177-180; traits.cfg:201-209.

### Guardião — accurate
- **Handbook claim:** "Reduz o custo de *Ranged* para 1." — src/21-povos/04-aluriel.md:226
- **Codebase truth:** backgrounds.cfg:61-65 — `Guardião` → RANGED cost 1.
- **Mismatch:** None.
- **Cross-links:** src/70-passados.md:104-108.

---

## Gulthrak (`src/21-povos/05-gulthrak.md`)

### Cria do Deserto — undisclosed cold-resistance penalty
- **Handbook claim:** "Bônus para resistir climas quentes (+10)" with the
  Característica explicitly stating "**Desvantagem:** Nenhuma" —
  src/21-povos/05-gulthrak.md:127 (box) / src/71-caracteristicas.md:212-217.
- **Codebase truth:**
  - Heat: trait `"Cria do Deserto"` does `dif := dif - 10` (easier to resist
    heat) — pkg/systems/nature/caloreffect.src:102-104. **Matches** the
    stated +10 heat resistance.
  - Cold: the SAME trait does `dif := dif + 4` (HARDER to resist cold) —
    pkg/systems/nature/coldeffect.src:65-67. This penalty is **not disclosed**
    anywhere in the handbook or traits.cfg.
- **Mismatch:** The trait carries a hidden cold-resistance drawback (+4 cold
  difficulty) that the handbook omits while explicitly asserting
  "Desvantagem: Nenhuma". The book asserts a no-downside rule the code
  contradicts.
- **Severity:** MED (real omitted drawback, but low-stakes environmental;
  affects survival planning in cold regions, not core combat builds)
- **Cross-links:** src/71-caracteristicas.md:212-217 ("Desvantagem: Nenhuma"
  — the primary copy of the false no-downside claim); traits.cfg:237-245
  ("Desvantagens Nenhuma" — upstream origin).

### Passado: Flagelo das Areias — accurate
- **Handbook claim:** "Reduz o custo de *Two Handed* para 1" — src/21-povos/05-gulthrak.md:129
- **Codebase truth:** backgrounds.cfg:67-71 — `Flagelo das Areias` →
  TWO_HANDED cost 1.
- **Mismatch:** None.
- **Cross-links:** src/70-passados.md:112-116.

---

## Colhesiana (`src/21-povos/06-colhesiana.md`)

### Desapercebido — accurate
- **Handbook claim:** "Você aumenta em 50% os seus passos enquanto
  escondido." — src/21-povos/06-colhesiana.md:168
- **Codebase truth:** trait `"Desapercebido"` does
  `baseSteps := CInt(baseSteps * 1.5)` for stealth steps —
  pkg/skills/sneak/stealth.src:25-27.
- **Mismatch:** None. (Stacks multiplicatively with ESPREITA_SELVAGEM ×2 and
  SMOKE_SCREEN ×2 — not a contradiction of the book, just unstated synergy;
  no finding.)
- **Severity:** — (no finding)
- **Cross-links:** src/71-caracteristicas.md:201-204; traits.cfg:225-232.

### Passado: Armadilheiro — accurate
- **Handbook claim:** "Reduz o custo de *Mechanics* para 1" — src/21-povos/06-colhesiana.md:169
- **Codebase truth:** backgrounds.cfg:55-59 — `Armadilheiro` → MECHANICS
  cost 1.
- **Mismatch:** None.
- **Cross-links:** src/70-passados.md:95-99.

---

## Povos do Leste (`src/21-povos/07-povos-do-leste.md`)

### Cavalaria de Escaramuça — false "no penalty" + invented bow rule
- **Handbook claim:** "Pode combater montado, até mesmo com arcos." —
  src/21-povos/07-povos-do-leste.md:164
- **Codebase truth:** Identical to the Bjorske finding above —
  HitController.inc:87-97. The trait alone gives only the −25% tier, not zero
  penalty (zero requires the `MOUNTED_COMBAT` ability too); and no code
  blocks bows while mounted, so "até mesmo com arcos" implies a restriction
  that does not exist.
- **Mismatch:** (a) implies the trait removes the mounted penalty without
  disclosing the required second source (Habilidade "Combate Montado");
  (b) "até mesmo com arcos" is an invented restriction with no code.
- **Severity:** HIGH for the penalty implication; INVENTED for the bow clause.
- **Cross-links:** Direct duplicate of src/21-povos/02-bjorske.md:128 and
  src/71-caracteristicas.md:169 (the primary copy); traits.cfg:188-197.

### Passado: Discípulo das Lâminas — accurate
- **Handbook claim:** "Reduz o custo de *Dual Wield* para 1." — src/21-povos/07-povos-do-leste.md:166
- **Codebase truth:** backgrounds.cfg:79-83 — `Discípulo das Lâminas` →
  DUAL_WIELD cost 1.
- **Mismatch:** None.
- **Cross-links:** src/70-passados.md:128-132.

---

## Passados — all verified accurate

Every Passado named in the 8 boxes maps 1:1 to a `backgrounds.cfg` entry with
"Reduz o custo de <SKILL> para 1". No discrepancies:

| Povo | Passado | Skill | cfg line |
|---|---|---|---|
| Aiglanos | Legionário | PARRY | backgrounds.cfg:85-89 |
| Baduran | Filho do Progresso | ALCHEMY | backgrounds.cfg:73-77 |
| Bjorske | Cavalaria Nórdica | HANDLE_ANIMAL | backgrounds.cfg:91-95 |
| Haroloth | Prole Abissal | AWARENESS | backgrounds.cfg:49-53 |
| Aluriel | Guardião | RANGED | backgrounds.cfg:61-65 |
| Gulthrak | Flagelo das Areias | TWO_HANDED | backgrounds.cfg:67-71 |
| Colhesiana | Armadilheiro | MECHANICS | backgrounds.cfg:55-59 |
| Povos do Leste | Discípulo das Lâminas | DUAL_WIELD | backgrounds.cfg:79-83 |

---

## Summary

| Severity | Count |
|---|---|
| HIGH | 2 |
| MED | 1 |
| INVENTED | 2 |
| LOW | 0 |
| Accurate (no finding) | 6 traits + 8 passados |

Findings are concentrated in just 2 of the 16 mechanical perks (the shared
"Cavalaria de Escaramuça" trait and "Cria do Deserto"); the other 14 are
correct.

### Top 5 by impact

1. **Cavalaria de Escaramuça "não recebe penalidades" — false** (HIGH).
   Trait alone is only the −25% tier; zero penalty needs Habilidade "Combate
   Montado" too. Appears in `bjorske.md:128`, `povos-do-leste.md:164`,
   `71-caracteristicas.md:169`. One fix to the Característica + the two povo
   boxes hits all in-book copies (traits.cfg:195 is the upstream string).
2. **Cavalaria de Escaramuça "arcos" — invented** (INVENTED). No code blocks
   bows while mounted. Same 3 locations.
3. **Cria do Deserto hidden cold penalty** (MED). Trait does `dif += 4` in
   coldeffect.src:65 yet the Característica asserts "Desvantagem: Nenhuma"
   (`71-caracteristicas.md:215`) and the box omits it (`gulthrak.md:127`).
4. (Covered by 1 — povos-do-leste copy.)
5. (Covered by 2 — povos-do-leste copy.)

---

## Duplicate-claim clusters

**Cluster A — Cavalaria de Escaramuça "no mounted penalty + bows"**
(false penalty claim + invented bow rule, same wording, 3 in-book sites + cfg):
- src/21-povos/02-bjorske.md:128 — "não recebe penalidades... podendo até usar arcos"
- src/21-povos/07-povos-do-leste.md:164 — "Pode combater montado, até mesmo com arcos"
- src/71-caracteristicas.md:169 — "não recebe penalidades... podendo até usar arcos" (PRIMARY copy)
- src/21-povos/02-bjorske.md:139 (Comportamento: "natos no uso de montarias" —
  flavor, NOT flagged, listed only to show where the theme recurs)
- Upstream non-handbook source of the wrong string:
  `pkg/systems/charactercreation/config/traits.cfg:195`
→ One correction to `71-caracteristicas.md:169` + the two povo boxes fixes
every player-facing copy. (Per `_FORMAT.md` this is calibration
Mismatch 1/2/3.)

**Cluster B — Cria do Deserto "Desvantagem: Nenhuma"**
(no-downside assertion contradicted by the cold penalty):
- src/21-povos/05-gulthrak.md:127 — box, omits the cold drawback
- src/71-caracteristicas.md:212-217 — "Desvantagem: Nenhuma" (PRIMARY copy)
- Upstream: `traits.cfg:237-245` "Desvantagens Nenhuma"
→ Disclose the cold-resistance penalty in the Característica; the box can stay
abstracted but should not be paired with an explicit "no downside".

**No other duplicates.** Each of the 8 Passados appears once per povo and once
in `70-passados.md`; all verified accurate, so the cross-links are documented
per finding above but require no fix.
