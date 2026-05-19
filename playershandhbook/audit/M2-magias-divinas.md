# Audit M2 — Magias Divinas (Paladino, Caçador de Demônios, Clérigo)

Domain files:
- `src/80-magias/40-divinas-intro.md` (divider/lore only — nothing to verify)
- `src/80-magias/41-paladino.md`
- `src/80-magias/42-cacador-de-demonios.md`
- `src/80-magias/43-clerigo.md`

Code: `pkg/systems/spells/spell_logic/{paladin,demonhunter,priest}/`,
configs `pkg/systems/spells/config/{paladin,demonhunter,priest}.cfg`.

Method note: per `_FORMAT.md`, the handbook intentionally abstracts
Dificuldade / Custo / Ciclos numbers, so the (very large) divergences
between handbook numbers and `*.cfg` values are **not** flagged. Cleric
spells live in code under `priest/`; that folder naming is internal and
not a handbook claim. No divine spell in this domain branches on the
caster's religion mechanically (`grep -i GetReligion/religiao/VENERATIO…`
across all three folders returns exactly one hit —
`santuario.src:42`, and that branch only swaps a cosmetic FX id, KAI vs
others). The handbook makes no per-religion claims for these spells, so
there are no religion-rider mismatches in this domain.

---

## 40-divinas-intro.md

Pure divider page (background images + logo + index footnote). No
mechanical claims. Nothing to audit.

---

## 41-paladino.md

### Cura Leve do Paladino listada como 1º Círculo, código é Círculo 2
- **Handbook claim:** "## Primeiro Círculo … ### Cura Leve" — src/80-magias/41-paladino.md:5,20
- **Codebase truth:** Spell 803 "Cura Leve" → `Circle 2`, script `paladin/paladinheal` — pkg/systems/spells/config/paladin.cfg (Spell 803, `Circle 2`)
- **Mismatch:** Book places Cura Leve in the 1st circle; the cfg assigns it Circle 2. Circle is an ability-progression gate the book uses to structure the chapter, so the placement is a real structural claim, not an abstracted number.
- **Severity:** MED (wrong circle bucket misleads when a spell unlocks)
- **Cross-links:** none

### Forçar Duelo do Paladino listada como 2º Círculo, código é Círculo 1
- **Handbook claim:** "## Segundo Círculo … ### Forçar Duelo" — src/80-magias/41-paladino.md:33,48
- **Codebase truth:** Spell 804 "Forçar Duelo" → `Circle 1` — pkg/systems/spells/config/paladin.cfg (Spell 804, `Circle 1`)
- **Mismatch:** Book lists it under 2nd circle; cfg has Circle 1. (Conversely Cura Leve is the inverse error — the two appear swapped in the chapter ordering.)
- **Severity:** MED
- **Cross-links:** none

### Sacrifício (Paladino): transfere VITAIS, não atributos
- **Handbook claim:** "O paladino transfere todas as suas forças para um alvo, ficando com 1 em cada atributo." — src/80-magias/41-paladino.md:108
- **Codebase truth:** `paladin/sacrificio.src:32-44` reads `AP_GetVital(who,"Hits"/"Stamina"/"Mana")`, adds them to the target's Hits/Stamina/Mana, then sets the caster's **Hits/Stamina/Mana** to 1. STR/DEX/INT are never touched.
- **Mismatch:** "transfere todas as suas forças … ficando com 1 em cada atributo" describes a STR/DEX/INT (atributos) transfer. The code transfers **vitais** (HP/Stamina/Mana) and drops the caster to 1 HP / 1 Stam / 1 Mana, not 1 in each stat. A paladino reading this expects a stat-buff transfer; in reality it is a near-suicidal vitality dump. Also the cfg `Desc` itself says "ficando com 1 em cada stat" — same wrong description duplicated in the spell config.
- **Severity:** HIGH (completely changes when/why you would cast it)
- **Cross-links:** pkg/systems/spells/config/paladin.cfg Spell 809 Desc (same wrong wording in code-side config, not a src/ file)

### Imunidade a Veneno (Paladino) — funcional, sem mismatch mecânico
- **Handbook claim:** "O paladino torna-se imune a venenos." Alvo: Auto — src/80-magias/41-paladino.md:122-126
- **Codebase truth:** `paladin/imunidadeveneno.src:20-22` sets `protecaoveneno`=1 on the caster for `theology` seconds, then erases it. Self-only. Matches.
- **Mismatch:** none (note: it is a timed window, not the one-shot SC `poison_ward` used by the Clérigo version — but the book's "imune a venenos" wording is compatible). No finding.
- **Severity:** —
- **Cross-links:** —

### Repelir Magias (Paladino) — funcional
- **Handbook claim:** "O paladino não pode ser alvejado diretamente por magia." Auto — src/80-magias/41-paladino.md:135-139
- **Codebase truth:** `paladin/repelirmagias.src:20-23` sets cooldown `"repeal"` on self for `theology/5` s. Self-only, consistent with "Auto". No finding.

### Comandar / Afastar o Mal / Proteção / Dissipar Magia — consistentes
- `paladin/comandar.src` — forces NPC flee or shoves a player away; blocks `animal` braintype, consistent with "jogador ou NPC não-animal". OK.
- `paladin/afastaromal.src` — AoE flee vs undead/demons gated by tier ≤ theology/20, **Alvo: Self/área** matches "Auto". OK.
- `paladin/protecao.src` → `RechargeRunesOnTarget` — "abençoa equipamentos entalhados" plausible. OK.
- `paladin/dissiparmagia.src` — strips a large list of TS + SC buffs/debuffs and can banish summons (tier ≤ theology/20, chance = theology%). Matches "perde todos os encantamentos … Chance de banir invocações". OK.

---

## 42-cacador-de-demonios.md

### Cura Leve do Caçador listada como 1º Círculo, código é Círculo 2
- **Handbook claim:** "## Primeiro Círculo … ### Cura Leve" — src/80-magias/42-cacador-de-demonios.md:5,7
- **Codebase truth:** Spell 902 "Cura Leve" → `Circle 2`, script `demonhunter/paladinheal` — pkg/systems/spells/config/demonhunter.cfg (Spell 902, `Circle 2`)
- **Mismatch:** Same swap pattern as Paladino: book has Cura Leve in circle 1, cfg has Circle 2; Detectar o Mal is listed 1st-circle and is Circle 1 (OK), while Aprisionar Demônio (2nd in book) is Circle 2 (OK). Cura Leve / circle bucket is wrong.
- **Severity:** MED
- **Cross-links:** Cura Leve do Paladino mismatch above (identical script `paladinheal`, identical circle error).

### Cura Leve do Caçador fere undead/demônios (omitido pelo livro)
- **Handbook claim:** "Canaliza energia vital para curar ferimentos." (no caveat) — src/80-magias/42-cacador-de-demonios.md:7-8
- **Codebase truth:** `demonhunter/paladinheal.src:50-58` — if the target NPC's Environment is `undead` or `demons`, the spell calls `SC_DealDamage(who, targ, …, healed, DMG_FORCED)` instead of healing — it **damages** undead/demons for the heal amount.
- **Mismatch:** Book describes pure healing with no mention that it harms undead/demons. For a demon-hunter this dual-mode (heal allies / smite demons) is a meaningful tactical property the book omits entirely.
- **Severity:** MED (omitted offensive use; same applies to Clérigo Cura Leve only via `paladinheal`, but Clérigo uses `curaleve.src` which has no such branch — see Clérigo section)
- **Cross-links:** Paladino Cura Leve uses the same `paladinheal.src` and has the same undisclosed undead/demon damage branch (src/80-magias/41-paladino.md:20).

### Disparo Divino "fere apenas demônios" — falso, fere qualquer alvo
- **Handbook claim:** "Projeta uma flecha de energia divina que fere apenas demônios." — src/80-magias/42-cacador-de-demonios.md:64
- **Codebase truth:** `demonhunter/disparodivino.src:24-46` — base damage `2d6 + theology/10` is dealt to **any** mobile via `SC_DealDamage(... DMG_FORCED)`; the demon check (`IsDemon`) only **adds bonus** damage (`+1d10 + theology/6`). It also applies a `hitchance_mod := -15` + tint debuff for `theology/2` s to the target regardless of type.
- **Mismatch:** "fere apenas demônios" is wrong — it damages and debuffs any target; demons merely take extra. The book's exclusivity claim would lead a player to think it is useless vs non-demons, when it is a general nuke + accuracy debuff.
- **Severity:** HIGH (misleads core combat usage of a 3rd-circle attack spell)
- **Cross-links:** none

### Transferência Vital — doador não precisa ser demônio
- **Handbook claim:** "Drena a força vital de um demônio e a transfere para um aliado." — src/80-magias/42-cacador-de-demonios.md:108
- **Codebase truth:** `demonhunter/transferenciavital.src:17-81` targets a receptor then a doador via `Target()`. There is **no `IsDemon`/Environment check** on the donor; it works on any mobile that fails the WILL resist (`CheckResist(mobile, WILL, …)`), draining 10 dmg/tick for up to 10 ticks and healing the receiver each tick.
- **Mismatch:** "de um demônio" is an invented restriction — any creature/player can be the donor. The book also omits the WILL resist and the caster-frozen channel.
- **Severity:** HIGH (invented target restriction on a 4th-circle spell)
- **Cross-links:** none

### Pacto Demoníaco — invocação aleatória (1d10), não fixa
- **Handbook claim:** "Invoca uma criatura infernal para lutar ao seu lado." — src/80-magias/42-cacador-de-demonios.md:136
- **Codebase truth:** `demonhunter/pactodemoniaco.src:21-35` rolls `1d10` and summons a different demon per result (imp → succubus → DemonKnight → … → ArchDemon). Duration `120 + 180*theology/100` s, shortened to `theology` s on roll 9 (Abyssal).
- **Mismatch:** Not a contradiction — "uma criatura infernal" is loose enough. The randomized power tier (imp vs ArchDemon) is a significant mechanic the book abstracts away; flagged LOW as imprecise rather than wrong.
- **Severity:** LOW
- **Cross-links:** none

### Banimento / Detectar o Mal / Aprisionar Demônio / Dissipar Magia — consistentes
- `demonhunter/banimento.src` — AoE 9999 dmg to undead/demons within `theology/20` tiles, tier-gated. "Bane todos os mortos-vivos e demônios em uma área" matches (range is small, but book abstracts that). OK.
- `demonhunter/detectaromal.src` — reveals corruption + disguise (disguise needs theology ≥ 50). "Revela a verdadeira forma de demônios disfarçados" — roughly matches; the corruption-aura side is omitted but not contradicted. OK.
- `demonhunter/aprisionardemon.src` — `IsDemon` gate, `opts.frozen := 1` for `5 + theology/10` s. "Aprisiona um único demônio" matches. OK.
- `demonhunter/dissiparmagia.src` — identical script to paladin's. Matches "Remove encantamentos e tem chance de banir invocações". OK.

---

## 43-clerigo.md

### Sacrifício (Clérigo) NÃO troca "todo seu HP"; troca metade do dano que falta
- **Handbook claim:** "O sacerdote troca todo seu HP por uma cura instantânea no alvo." — src/80-magias/43-clerigo.md:134
- **Codebase truth:** `priest/sacrificio.src:48-54` — `healed := (maxHP - currentHP) / 2`; `DamageFLS(who, healed)` self-damages by that amount, then `applyEpicHealing(targ, healed)`. It also **refuses** if the target `IsBloodied` ("O alvo esta muito ferido") or is at full HP.
- **Mismatch:** It does not cost "todo seu HP" — it costs half the caster's *missing* HP (so a full-HP cleric pays 0 and heals 0). And it cannot be cast on a heavily wounded target (the exact case you'd want an emergency heal), which the book does not mention. The "instantânea no alvo" framing hides both the self-damage scaling and the IsBloodied gate.
- **Severity:** HIGH (misrepresents cost and usability of the spell)
- **Cross-links:** Paladino "Sacrifício" is a different spell/script (vitals transfer) — do not conflate; both are mis-described but independently.

### Aura Mortis — magia listada no livro não existe em código
- **Handbook claim:** "### Aura Mortis — Parte do dano que você levar volta para quem causou." Segundo Círculo — src/80-magias/43-clerigo.md:70-78
- **Codebase truth:** Spell 517 "Aura Mortis" is **entirely commented out** in pkg/systems/spells/config/priest.cfg (lines ~`# Spell 517 { … }`). No active spell id, no castable script for the cleric. (`necromancer/retribuicao` exists but is not wired to any cleric spell.)
- **Mismatch:** The book advertises a 2nd-circle cleric spell that is disabled in code — a cleric cannot learn or cast it.
- **Severity:** HIGH (book lists an unobtainable spell)
- **Cross-links:** none

### Ressurreição — corpo violado NÃO impede o ress (apenas avisa)
- **Handbook claim:** "Você consegue trazer um herói cujo corpo não foi violado de volta à vida." — src/80-magias/43-clerigo.md:249
- **Codebase truth:** `priest/resurrect.src:36-51` — if `GetObjProperty(targ,"assassinado")` it only `SendSysMessage("O corpo foi violado e voce nao podera ajuda-lo")` and then **continues**: the resurrection logic (`RessPlayer` / `morto_recentemente` cooldown / theology ≥ 90 gate) runs regardless. The "not violated" condition is text only — nothing returns or blocks on it.
- **Mismatch:** The book asserts a hard prerequisite ("cujo corpo não foi violado"); the code does not enforce it. A violated/assassinated body can still be resurrected. Real gating is instead by theology skill (≥ 90 for normal death; any skill within `morto_recentemente` window) — which the book omits.
- **Severity:** HIGH (asserts a restriction no code enforces, and hides the real theology gate)
- **Cross-links:** none

### Escudo de Fé — bônus extra para alvos sem armadura (omitido)
- **Handbook claim:** "Concede redução de dano ao alvo por alguns segundos." — src/80-magias/43-clerigo.md:57
- **Codebase truth:** `priest/escudodefe.src:37-48` — `bonus := theology/3` slash/piercing/blunt resist, **+10 extra** if `!HasArmorProtection(targ)` (no armor / wildshape counts as armored). `buff_category "armor_protection"` (mutually exclusive with barkskin), `maintainable`.
- **Mismatch:** The naked-target +10 rider is a meaningful build interaction (robe clerics get more than plate wearers) the book does not mention. Not a contradiction of "redução de dano", so MED not HIGH.
- **Severity:** MED
- **Cross-links:** none

### Imunidade a Veneno (Clérigo) — "próximo veneno" correto; "Alvo: Único" correto
- **Handbook claim:** "O alvo fica imune ao próximo veneno que levar." Alvo: Único — src/80-magias/43-clerigo.md:172-175
- **Codebase truth:** `priest/imunidadeveneno.src:33-42` — sets one-shot `protecaoveneno` flag, `buff_category "poison_ward"`, duration `max(theology,30)` s, maintainable. Target is any mobile (Único). Matches.
- **Mismatch:** none. No finding (recorded as verified).

### Prece Curativa — regen condicionado a "região calma" é real
- **Handbook claim:** "Enquanto a região permanecer calma, todos ao redor do sacerdote ganham regeneração." — src/80-magias/43-clerigo.md:122
- **Codebase truth:** `priest/descansotranquilo.src` + `descansotranquiloTick.src` — tick HoT (`theology/20` HP / 2 s, min 1) around the caster, duration `theology` s; recast blocked while active. Matches the described behavior. (Tick reads area; "calma" enforced by tick logic.) OK.
- **Mismatch:** none.

### Campo Anti-Magia — Alvo "Auto" no livro, é self-centrado no código (OK)
- **Handbook claim:** "Cria um campo que silencia todas as criaturas ao redor do alvo." Alvo: Auto — src/80-magias/43-clerigo.md:262-266
- **Codebase truth:** `priest/antimagicfield.src:38` — `SC_ApplyEffect(who, who, …)` self-centered, ticks `antimagicfieldTick` silencing in-range mobiles, duration `max(theology,10)` s. Self/Auto consistent. OK.
- **Mismatch:** none.

### Repelir Magias (Clérigo) — Alvo Único correto, comportamento consistente
- `priest/repelirmagias.src:33-35` sets `"repeal"` cooldown on the chosen target (NPC casters self-target) for `theology` s + buff icon. "O alvo não pode ser alvejado por outras magias" consistent. OK.

### Descanso / Cura Leve / Cura Grave / Juízo Flamejante / Dissipar Magia / Remover Maldição / Proteção Contra a Morte / Detectar* / Imbuir Proteção / Cura em Massa / Destruir o Mal / Círculo de Proteção / Santuário — consistentes
- `priest/descanso.src` — restores stamina (`theology/4`), rejects non-fatigued target. "Recupera o alvo do cansaço" OK.
- `priest/curaleve.src` — heals `max(theology/2,20)`, ×2 if caster has `AURA_DIVINA` ability and target bloodied. (Cleric's own Cura Leve has **no** undead-damage branch, unlike paladin/demonhunter `paladinheal`.) "Cura ferimentos leves" OK.
- `priest/curagrave.src` — heals `theology` (×2 AURA_DIVINA + bloodied). "Cura ferimentos graves" OK.
- `priest/flamestrike.src` — `8d6` DMG_FIRE. "grande dano de fogo" OK.
- `priest/dissiparmagia.src` — same broad strip as paladin's. "Remove as magias … buff ou debuff" OK.
- `priest/removermaldicao.src` — clears curse/debuff TS timers + SC debuffs + uncurses weapon + special FarolEspiritos handling (Magery ≥ 60). "Remove a maldição de um equipamento" OK (book omits the debuff-cleanse and Farol case, but no contradiction).
- `mage/protecaomorte.src` (Spell 519) — sets `protecaomorte` cooldown `90 + theology*1.5` s on a *connected player*. "não corre risco de morrer caso desmaie" OK; only works on players (book doesn't claim otherwise).
- `priest/detectarimpurezas.src` — diseases on mobiles / poison on items. Matches.
- `priest/detectarmagia.src` — magic prop on items; on mobiles reports arcane/divine affinity. Matches "detecta se um item possui propriedades mágicas" (+undisclosed mobile mode, not contradictory).
- `priest/detectarcorrupcao.src` — entropy tiers + WILL resist + 1h cooldown. "Detecta se o alvo possui corrupção" OK (resist/cooldown omitted, not contradicted).
- `priest/imbuirprotecao.src` → `RechargeRunesOnTarget`. "Recarrega as runas sagradas" OK.
- `mage/massheal.src` (Spell 528) — heals non-hostiles in `theology/10` range, `theology/5 + 6d8` each (×2 AURA_DIVINA). "Cura todos os aliados próximos" OK.
- `priest/destruiromal.src` — 1000 dmg to undead/demons within 6 tiles, count cap `theology/10`. Matches "Destrói todas as criaturas malignas na área" (cap omitted, not contradictory).
- `priest/protectioncircle.src` — glyph ring, pushes back undead/demons within 5 tiles while caster channels. "afastar criaturas malignas na área" OK.
- `priest/santuario.src` — self disengage; only religion touch in domain is a **cosmetic** FX (KAI → fire engulf vs energy burst, line 42). "impedindo-o de atacar e ser atacado" OK; book correctly does not claim any religion effect.

---

## Summary

| Severity | Count |
|----------|-------|
| HIGH     | 6 |
| MED      | 4 |
| LOW      | 1 |
| INVENTED | 0 (the invented-restriction findings folded into HIGH) |

Top 5 by impact:

1. **HIGH — Disparo Divino "fere apenas demônios" is false** (42:64). Damages and debuffs any target; demons only take bonus. Misrepresents a core attack spell.
2. **HIGH — Transferência Vital donor need not be a demon** (42:108). No demon/Environment check on the donor; invented restriction; WILL resist + frozen channel omitted.
3. **HIGH — Paladino Sacrifício transfers vitals, not stats** (41:108). Code dumps caster Hits/Stamina/Mana to 1, not "1 em cada atributo"; near-suicidal, opposite of a stat buff.
4. **HIGH — Clérigo Sacrifício does not cost "todo seu HP"** (43:134). Costs half the *missing* HP and refuses bloodied/full-HP targets; full-HP cleric heals 0.
5. **HIGH — Aura Mortis listed but commented out** (43:70) / **Ressurreição "corpo não violado" not enforced** (43:249) — a non-existent cleric spell advertised, and a hard prerequisite the code only warns about while resurrecting anyway (real gate is theology ≥ 90).
