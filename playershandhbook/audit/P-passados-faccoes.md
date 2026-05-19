# Audit P — Passados (src/70) + Facções e Guildas (src/23)

Domain: `src/70-passados.md`, `src/23-faccoes-guildas.md`
Auditor pass: mechanical claims vs `pkg/` code. Read-only.

## Scope summary

**Passados (src/70):** 8 Passados Gerais + 8 Passados Culturais. All 16 map
1:1 to `pkg/systems/charactercreation/config/backgrounds.cfg`. The "custo 1"
mechanic is real and correctly wired: `backgroundSelection` stores
`chardata.bgBonus` (the cfg `Bonus` skill token), and
`BuildSkillCostMap` (`pkg/mobiles/merchants/pcmerchant/trainerMenu.src:435-438`)
forces that skill to per-level cost 1, treated as primary. Skill tokens in
`backgrounds.cfg` (`Domestic_Work`, `Metal_Working`, `Pickpocketing`,
`Wood_Working`, `Handle_Animal`, `Dual_Wield`, `Two_Handed`, …) match the
canonical skill tokens in `config/classes.cfg`, so the override lands. No
effect-level (skill / "custo 1") mismatch found on any Passado.

**Facções e Guildas (src/23):** The entire chapter (Augures + Tribunal
Arcano) is lore: religion history, hierarchy titles, beliefs, rites,
appearance, external perception. It makes **no mechanical claim** — no
joining requirement, no perk/ability granted, no faction-gated player
feature. Per `_FORMAT.md` ("NOT a mismatch: flavor/lore prose"), there is
nothing in this chapter to flag. Code context recorded below for
completeness; it does not contradict the book because the book asserts
nothing mechanical.

---

## Findings — Passados

### Cultural prerequisite is gated by *culture*, named by *povo* — "Pequilho" vs code "Colhesiana"
- **Handbook claim:** "**Vantagem**: A perícia Mechanics passa a ter custo 1.  **Pré-requisito:** Pequilho" — src/70-passados.md:99-100 (Armadilheiro)
- **Codebase truth:** The creation gump only ever offers ONE cultural background, resolved by `backgroundByCulture(who)` → `CULTURAL_BACKGROUNDS[chardata.culture]` — `scripts/include/creationUtils.inc:547-552`. The map key for Armadilheiro is the **culture** `"Colhesiana"`, not the povo `"Pequilho"` — `scripts/include/creationUtils.inc:47`. The Colhesiana culture maps to exactly one race, `PEQUILHO` — `scripts/include/creationUtils.inc:196-197`. So functionally the gate IS "you must be a Pequilho" (Colhesiana ⇔ Pequilho is strictly 1:1), but the book names the gate by the race while the code keys it on the culture string.
- **Mismatch:** Terminology only. The prerequisite is real and behaves exactly as the book intends (only a Pequilho/Colhesiana character is offered Armadilheiro). No player-visible behavioral delta.
- **Severity:** LOW (imprecise wording; mechanic is correct)
- **Cross-links:** none (06-colhesiana.md povo box does not restate this Passado)

### Cultural prerequisite "Bjorske" vs code culture "Björska"
- **Handbook claim:** "**Vantagem**: A perícia Handle Animal passa a ter custo 1.  **Pré-requisito:** Bjorske" — src/70-passados.md:148-149 (Cavalaria Nórdica)
- **Codebase truth:** `CULTURAL_BACKGROUNDS` keys Cavalaria Nórdica on culture `"Björska"` (`scripts/include/creationUtils.inc:47`), and `chardata.culture` for that culture is set to the constant `BJORSKA := "Björska"` (`scripts/include/epicConstants.inc:24`). Björska culture maps to race `HUMANO` (`scripts/include/creationUtils.inc:202-203`). The povo handbook itself defines the terms: "se autoproclamam como Björska (singular) e Björske (plural)" — src/21-povos/02-bjorske.md:24. "Bjorske" (no umlaut) is the book's own anglicized plural.
- **Mismatch:** Terminology/spelling only ("Bjorske" vs "Björska"). The gate works; a Björska/Björske character is the only one offered Cavalaria Nórdica. No behavioral delta.
- **Severity:** LOW (imprecise wording; mechanic is correct)
- **Cross-links:** src/21-povos/02-bjorske.md:129 ("**Passado: Cavalaria Nórdica**: Reduz o custo de *Handle Animal* para 1") — restates the same Passado; consistent with backgrounds.cfg:91-94. No mismatch in the cross-link itself.

### Prerequisite enforcement is implicit, not a hard check — book wording is fine
- **Handbook claim:** Every Passado Cultural lists "**Pré-requisito:** <povo/cultura>" (src/70-passados.md:93,100,109,117,125,133,141,149).
- **Codebase truth:** There is no explicit "requisito" validation for backgrounds (unlike abilities, which use `TemRequisito` in `pkg/systems/charactercreation/include/habilidades.inc:23`). Instead, the gump structurally offers only the single `backgroundByCulture(who)` cultural option alongside the 8 general ones (`scripts/include/creationUtils.inc:525-539`), and `backgroundSelection` lets the player pick exactly one (general OR the one cultural) — `pkg/systems/charactercreation/epicgumpcriacao.src:395-418`. A player can never reach a cultural Passado outside their culture because it is never rendered.
- **Mismatch:** None. The prerequisite is honored by construction. Recorded only to confirm the book's prerequisite claims are TRUE (not invented), just enforced by gump gating rather than a validation function.
- **Severity:** (no mismatch — verification note)
- **Cross-links:** none

### All Passado skill effects verified — no invented or wrong-skill effects
- **Handbook claim:** Each Passado: "A perícia <SKILL> passa a ter custo 1" (src/70-passados.md:29,36,43,50,57,64,71,78,92,99,108,116,124,132,140,148).
- **Codebase truth:** 1:1 with `pkg/systems/charactercreation/config/backgrounds.cfg:1-95`:
  Mestre do Lar→Domestic_Work, Ferreiro Aprendiz→Metal_Working,
  Coletor→Gathering, Rato de Rua→Pickpocketing, Aventureiro→Sneak,
  Caçador→Survival, Curandeiro→Medicine, Marceneiro→Wood_Working,
  Prole Abissal→Awareness, Armadilheiro→Mechanics, Guardião→Ranged,
  Flagelo das Areias→Two_Handed, Filho do Progresso→Alchemy,
  Discípulo das Lâminas→Dual_Wield, Legionário→Parry,
  Cavalaria Nórdica→Handle_Animal. Cost override applied at
  `trainerMenu.src:435-438` (`costs[bg] := 1`).
- **Mismatch:** None. Full chapter (general + cultural) verified clean.
- **Severity:** (no mismatch — verification note)
- **Cross-links:** The 8 povo boxes (src/21-povos/*) restate the cultural Passados; the combat-pass H audit already confirmed those 8 map 1:1 to backgrounds.cfg "Reduz o custo de SKILL para 1". Cross-check holds; bjorske box (02-bjorske.md:129) re-verified above.

---

## Findings — Facções e Guildas

### Chapter 23 is lore-only — no mechanical claims to audit
- **Handbook claim:** Entire src/23-faccoes-guildas.md (Augures: religion, Conglomeratio, Sanctus, Evocatio, ritos; Tribunal Arcano: Magistratus/Praetores/Inquisitores hierarchy, Inquisitoris Pacti, Conceptus Arcanum, símbolo, aparência, visão externa).
- **Codebase truth:** Read in full — the chapter contains zero mechanical assertions (no "to join you must…", no "members gain ability X", no faction-gated player feature). The actual mechanical faction system (`pkg/systems/faccao/include/faccao.inc:43-49`) exposes 5 staff factions — "Regentes de Valtheris", "Sinodo", "Rufioes", "Conselho Sofocratico", "A Liga do Pacto Comum" — none of which is "Augures" or "Tribunal Arcano". There IS a thematically-related staff/GM mechanic, "Praetor Arcano" (`pkg/systems/faccao/tribunal.src:7-57` toggles a global `"Praetors"` serial list; `pkg/systems/faccao/praetorAlert.src` notifies enrolled Praetors when someone casts magic in Valtheris — currently commented out at `pkg/packetHooks/RequestAttack/requestAttack.src:44-45`), but the handbook does not describe it or claim it as a player perk.
- **Mismatch:** None per `_FORMAT.md` (lore/flavor is explicitly NOT a mismatch). The book neither claims a mechanic that's missing nor contradicts one that exists.
- **Severity:** (no mismatch — lore chapter)
- **Cross-links:** none

---

## Summary

| Severity | Count |
|---|---|
| HIGH | 0 |
| MED | 0 |
| LOW | 2 |
| INVENTED | 0 |
| Verification notes (no mismatch) | 4 |

**Top issues by impact:**

1. **(LOW) Armadilheiro prerequisite named "Pequilho", code keys culture "Colhesiana"** — src/70-passados.md:100. Functionally identical (1:1 culture↔povo); wording only.
2. **(LOW) Cavalaria Nórdica prerequisite "Bjorske" vs code culture "Björska"** — src/70-passados.md:149. Spelling/anglicization; mechanic correct. Cross-linked in src/21-povos/02-bjorske.md:129 (that copy is consistent).
3. *(No further mismatches.)* All 16 Passado skill effects verified 1:1 against backgrounds.cfg and the active cost-override path (trainerMenu.src:435-438).
4. *(No further mismatches.)* Cultural prerequisites are real and enforced by gump construction, not invented.
5. *(No further mismatches.)* Facções/Guildas chapter is pure lore — no mechanical claim exists to contradict.

**Overall:** This domain is in very good shape. No HIGH/MED/INVENTED issues.
The only nits are two cosmetic prerequisite-naming choices in the cultural
Passados (book uses povo/anglicized names, code keys on the culture
constant). No build-affecting deltas. The faction chapter makes no
mechanical promises, so there is nothing to break.
