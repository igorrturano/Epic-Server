# Handbook ↔ Codebase Audit — Caminho do Subterfúgio (C3)

Domain: `playershandhbook/src/60-caminho-subterfugio/`
Resolution path: `pkg/systems/abilities/include/AbilityRegistryLadino.inc` +
`pkg/systems/abilities/configs/habilidades.cfg` + per-class implementations
under `pkg/systems/abilities/include/classes/<class>/`.

Tier mapping used: handbook levels 2/6/10/14/20 correspond, in order, to
config `classe Ladino 1/3/5/7/10` (the ability IDs xx00..xx04).

---

## 01 — Assassino

The whole Assassino ability table is misaligned with code. Code (registry
`AbilityRegistryLadino.inc:114-118`, enum `Ability.inc:254-258`,
`habilidades.cfg:699-752`) defines exactly five abilities, in tier order:
**Assassinar (1200, Ladino 1)**, **Instinto Assassino (1201, Ladino 3)**,
**Arsenal Sombrio (1202, Ladino 5)**, **Faro por Sangue (1203, Ladino 7)**,
**Manto das Sombras (1204, Ladino 10)**.

### Handbook splits "Assassinar" into two invented abilities; omits "Manto das Sombras"
- **Handbook claim:** lvl 2 "Salto das Sombras" (jump that doesn't consume hidden steps, lands behind the foe) and lvl 6 "Ataque Furtivo" (next hit gets bonus from sneak when hidden) — src/60-caminho-subterfugio/01-assassino.md:31-32, 47-65
- **Codebase truth:** There is no "Salto das Sombras" or "Ataque Furtivo". A single ability `Assassinar` (ID 1200, tier 2) does both: leaps behind a target at range Stealth/2, guarantees the hit if hidden, and `OnPassiveBeforeDamage` adds the sneak bonus damage — pkg/systems/abilities/include/classes/assassino/Assassinate.inc:42-118; habilidades.cfg:701-710. The book's lvl-20 entry "Faro por Sangue" and the absence of "Manto das Sombras" mean the actual tier-20 ability (`Manto das Sombras`, ID 1204 — re-enter stealth after a successful backstab, SNEAK vs AWARENESS) is undocumented — ShadowCloak.inc:15-56; habilidades.cfg:744-752.
- **Mismatch:** Book invents two abilities ("Salto das Sombras", "Ataque Furtivo") for one (`Assassinar`); tier order of the remaining abilities is wrong (book: Arsenal Sombrio@10, Instinto Assassino@14, Faro por Sangue@20; code: Arsenal Sombrio@10, Faro por Sangue@14, Manto das Sombras@20); and `Manto das Sombras` is entirely missing from the book.
- **Severity:** HIGH (a player cannot map the book's progression to what they actually unlock)
- **Cross-links:** none

### "Salto das Sombras: não consome passos escondidos"
- **Handbook claim:** "Você consegue dar um salto furtivo que não consome passos escondidos" — src/60-caminho-subterfugio/01-assassino.md:54
- **Codebase truth:** `Assassinate.ExecuteOn` does `who.setwarmode(1)`, teleports behind, grants the `hiddenattack` privilege and calls `attack_once`. There is no stealth-step accounting at all; the ability is gated on having an assassin weapon and not being frozen/paralyzed, and the original `!who.hidden` guard is commented out — Assassinate.inc:25-85
- **Mismatch:** "não consome passos escondidos" describes a stealth-step economy the code never touches. Invented rule.
- **Severity:** INVENTED
- **Cross-links:** none

### Instinto Assassino "crítico garantido" — only on the Assassinar hit
- **Handbook claim:** "Caso acerte um ataque furtivo, o golpe é sempre um crítico garantido." — src/60-caminho-subterfugio/01-assassino.md:90
- **Codebase truth:** `KillerInstinct` is a stub; the crit is only set inside `Assassinate.ExecuteOn` and only when `mobile.IsHidden()` is true at the moment of `Assassinar` (`#nextcritical` → 100% in `DamageController.CalculateCriticalChance`) — KillerInstinct.inc:1-18; Assassinate.inc:72-77; DamageController.inc:942-949
- **Mismatch:** Effectively consistent with "sneak attack → guaranteed crit", but the trigger is specifically the `Assassinar` activation while hidden, not any furtive hit. Low-stakes wording given the book's own confusion above.
- **Severity:** LOW
- **Cross-links:** none

### Arsenal Sombrio weapon list
- **Handbook claim:** A 13-item weapon list (Adaga, Adagas Duplas, Arco Curto, Espada Curta, Garras, Karda, Kryss, Kukri, Lâminas Élficas, Lança, Picareta de Guerra, Wakizashi) — src/60-caminho-subterfugio/01-assassino.md:105-129
- **Codebase truth:** `ShadowArsenal`/`Assassinate` gate on `WeaponWrapper.IsAssassin()`, driven by `Assassin 1` flags in `itemdesc.cfg` (dozens of graphics, WeaponWrapper.inc:419-421, itemdesc.cfg ~40 `Assassin 1` entries). The flag is per-graphic; the exact set was not enumerated to weapon names.
- **Mismatch:** Could not confirm the printed list exactly matches the `Assassin 1` set.
- **Severity:** UNVERIFIED
- **Cross-links:** none

---

## 02 — Bardo

Maps to V3 Bardo (IDs 1708-1712, `AbilityRegistryLadino.inc:148-153`). The
five abilities are progression-gate passives whose effects live in the
musicianship system and `spell_executor.inc`. All five line up well.

### Bardo abilities — verified consistent
- **Handbook claim:** Vox Arcana unlocks C0-C1 + major scale + Ritmo de Guerra; Ressonância adds C2 + minor scale + 2 drum buffs + range/duration scaling with Arcana; Tear da Realidade adds chromatic comp + C3 + all drums + void portals; Maestria do Caos adds 2 simultaneous canticos + C4 + 3rd octave + boosted Surge Entrópico; Sinfonia Final (ativa, 300s) doubles the next effect — src/60-caminho-subterfugio/02-bardo.md:47-103
- **Codebase truth:** Circle gates `spell_executor.inc:332-346` (Vox=C1, Resonance=C2, RealityTear=C3, ChaosMastery=C4). Composition gates `CompositionEffect.inc:14-25`. Surge threshold 96→81 (5%→20%) gated by `CHAOS_MASTERY` `BardEntropyController.inc:188-202`. Vox required to play `bardmusic.inc:386-398`. `FinalSymphony` ON_USE, 300s cd, sets `#bard_final_symphony_active` to double next cantico/composition — FinalSymphony.inc:15-51. Resonance/ChaosMastery stubs document the same scaling — Resonance.inc:6-15, ChaosMastery.inc:6-14.
- **Mismatch:** None material. Numbers the book abstracts (Arcana/20 range, Arcana/2% duration) match the config.
- **Severity:** (no finding)
- **Cross-links:** none

---

## 03 — Caçador de Demônios

Maps to Demon Hunter (1900-1904, `AbilityRegistryLadino.inc:163-167`).

### Sentença Profana — religion table does not match code
- **Handbook claim:** Seven religion riders: Lóre = enraíza (root); Veneratio = silencia; Bokk Bharaz = invoca uma pistola sagrada com um projétil; Ush'krar = sangramento; Halleuad = cega; Eluith'orth = cria uma ilusão confundindo alvos ao redor; Kai = desarma / monstros recebem penalidade em acerto — src/60-caminho-subterfugio/03-cacador-de-demonios.md:66-74
- **Codebase truth:** `ProfaneSentence.OnBeforeDamage` `case(religiao)` handles only five: VENERATIO→silence; BOKK_BHARAZ→`lentidao` (slowness) + projectile *visual* only; HALLEUAD→`blinded`; ELUITHORTH→`darkness` cooldown on target; KAI→`bleedingstrike`. **Lóre and Ush'krar have no case** (only the flat THEOLOGY/2 energy damage applies) — ProfaneSentence.inc:55-78
- **Mismatch:** (a) Lóre "enraíza" — not implemented. (b) Ush'krar "sangramento" — not implemented. (c) Bokk Bharaz "invoca uma pistola sagrada" — code applies slowness + a moving FX, no summoned weapon. (d) Eluith'orth "cria uma ilusão confundindo alvos ao redor" — code applies single-target `darkness`, no illusion/AoE confuse. (e) Kai "desarma / penalidade de acerto" — code applies bleed, no disarm.
- **Severity:** HIGH (five of seven advertised religion payoffs are wrong or absent — directly misleads religion build choice)
- **Cross-links:** AbyssalForm rebuke uses the same religion mapping with a *different* set of effects (ProfaneSentence.inc:55-78 vs AbyssalForm.inc:87-105) — internal divergence worth noting.

### Mancha da Corrupção — generic hit buff, not demon-specific; "sentir demônios" unverified
- **Handbook claim:** "concedendo bônus contra demônios e a capacidade de senti-los nas proximidades." — src/60-caminho-subterfugio/03-cacador-de-demonios.md:88
- **Codebase truth:** `CorruptionMark` adds +THEOLOGY/10 attacker hit chance / −THEOLOGY/10 defender evasion against **any** target, not demon-gated — CorruptionMark.inc:6-17; HitController.inc:304-310. No demon-sense logic found in the ability file.
- **Mismatch:** "bônus contra demônios" implies a demon-specific bonus; code grants it universally. The "senti-los nas proximidades" (sense demons) effect was not located.
- **Severity:** MED
- **Cross-links:** none

### Armadilha Demoníaca — pentagrama vs hexagrama; damage split unverified
- **Handbook claim:** "Você cria um pentagrama que impede demônios e mortos-vivos de escapar. O alvo principal recebe dano total enquanto os demais na área sofrem dano parcial." — src/60-caminho-subterfugio/03-cacador-de-demonios.md:99
- **Codebase truth:** `DemonicTrap` builds a hexagram / Star-of-David vertex pattern, paralyzes Demon/Undead NPCs 30s, applies `#chainofpain`, `ApplyRawDamage(...,2)` to all + `1` extra to demons/undead — DemonicTrap.inc:14-131. The "100% main / 20% others" split (claimed in habilidades.cfg:1151) is via the chain-of-pain system, not visible in this file.
- **Mismatch:** "pentagrama" vs hexagram is a flavor delta (LOW). The full-vs-partial damage split could not be confirmed from the ability code.
- **Severity:** LOW
- **Cross-links:** none

### Forma Abissal — consistent
- **Handbook claim:** Wings to fly out of combat; in battle an abyssal avatar with big bonuses to all attributes — src/60-caminho-subterfugio/03-cacador-de-demonios.md:110
- **Codebase truth:** Equips wings, +Theology/2 STR/AGI/INT for 20s, plus an undocumented religion-specific rebuke on incoming hits — AbyssalForm.inc:19-148. Flight is a property of the wing item, not in this file.
- **Mismatch:** None material; the rebuke is an extra effect the book omits (not a mismatch).
- **Severity:** (no finding)
- **Cross-links:** none

---

## 04 — Caçador de Tesouros

Maps to 1100-1104 (`AbilityRegistryLadino.inc:107-111`).

### Distrair Oponentes — code paralyzes, book says you go unnoticed
- **Handbook claim:** "Você consegue distrair criaturas de forma convincente, passando despercebido por elas por algum tempo." — src/60-caminho-subterfugio/04-cacador-de-tesouros.md:66
- **Codebase truth:** `DistractOpponents.ExecuteOn` plays an attack FX on every non-party mobile within 1 tile and applies a 5s `paralysis` timer to them. It does not stealth/conceal the caster or make them "unnoticed" — DistractOpponents.inc:15-38. (Config habilidades.cfg:662 also says "passar despercebido", so code diverges from both.)
- **Mismatch:** The advertised effect (you slip by unnoticed) is not what the code does (AoE 5s paralyze of adjacent enemies).
- **Severity:** MED (build/utility expectation is wrong)
- **Cross-links:** none

### Desbravador / Sempre Alerta / Especialista / Bolsa Infinita — consistent
- **Handbook claim:** trap evasion on Reflexes + read treasure maps + open magic locks (lvl 2); immune to surprise + detect hidden bonus (lvl 10); never crit-fail lockpicking/trap disarm (lvl 14); much larger carry capacity (lvl 20) — src/60-caminho-subterfugio/04-cacador-de-tesouros.md:55,77,91,102
- **Codebase truth:** BornExplorer.inc:14-19 (trap/maps/locks), AlwaysAlert.inc:13-18 (surprise immunity + detection), BreakInSpecialist.inc:12-17 (no crit-fail), InfiniteBag.inc:11-16 (+120/+100 carry per habilidades.cfg:694).
- **Mismatch:** None.
- **Severity:** (no finding)
- **Cross-links:** none

---

## 05 — Dançarino das Sombras

Maps to 1800-1804 (`AbilityRegistryLadino.inc:156-160`). Pré-requisito
Drow/Har'oloth confirmed (habilidades.cfg:1052;
charactercreation/include/habilidades.inc:467).

### Valdruk — "forma borrada que dificulta te acertar" is not part of Valdruk
- **Handbook claim:** "Você se funde com as sombras, tornando-se completamente invisível... Ao sair da invisibilidade na escuridão, assume uma forma borrada, dificultando que os inimigos te acertem." — src/60-caminho-subterfugio/05-dancarino-das-sombras.md:79
- **Codebase truth:** `Valdruk.ExecuteOn` only starts a `conceal` timer for ARCANA/5s; `fimconceal.src` clears concealment with no follow-up effect — Valdruk.inc:24-42; subScripts/classes/fimconceal.src:1-13. The "forma borrada / harder to hit" is the separate illusionist spell `blur` (which Valdruk merely empowers), gated on `InDarkness` and cast manually — illusionist/blur.src:14-28.
- **Mismatch:** The book folds the standalone `blur` spell into Valdruk's automatic exit behavior. Valdruk grants no on-exit evasion.
- **Severity:** MED (overstates Valdruk's payoff)
- **Cross-links:** none

### Lâmina da Escuridão — undocumented bleed
- **Handbook claim:** "Seu próximo ataque causa dano mágico adicional e silencia o alvo" — src/60-caminho-subterfugio/05-dancarino-das-sombras.md:67
- **Codebase truth:** `ShadowBlade.OnBeforeDamage` applies mlore/2 forced damage, silence mlore/5s, **and** `MakeBleed(defender)` — ShadowBlade.inc:32-62
- **Mismatch:** Bleed is a real extra debuff the book omits; minor since it doesn't misstate what's there.
- **Severity:** LOW
- **Cross-links:** none

### Dança das Sombras / Dança da Morte / Forma Umbral — consistent
- **Handbook claim:** more hits scaling with Arcana (lvl 2); each hit repositions you behind the foe for some seconds (lvl 14); Forma Umbral lets you attack while invisible via Valdruk (lvl 20) — src/60-caminho-subterfugio/05-dancarino-das-sombras.md:55,93,105
- **Codebase truth:** ShadowDance +ARCANA/5 hit chance HitController.inc:278-280; DeathDance `deathdance` cd ARCANA/5s + `moveToBehind` on AfterDamage DeathDance.inc:21-65; ShadowForm lets you attack while concealed combatHook.src:216 — ShadowForm.inc:8-19.
- **Mismatch:** None.
- **Severity:** (no finding)
- **Cross-links:** none

---

## 06 — Duelista

Resolves to the **esgrimista** code folder (1500-1504,
`AbilityRegistryLadino.inc:135-139`; header comment "ESGRIMISTA -> Duelista
desde 16/04/2026" habilidades.cfg:873). Tier-2 ability is "Movimentação
Graciosa", tier-6 is "Postura de Duelo" (book calls it "Reflexo
Estratégico"), tier-14 is "Alta Guarda"/`SwashbucklerFeint` (book "Finta
Esgrimista").

### Reflexo Estratégico — Tactics-as-Parry requires a buckler; active effect described wrong
- **Handbook claim:** "Passivamente, você usa *Tactics* como *Parry*. Ao ativar, sua destreza aumenta em relação à falta de stamina." — src/60-caminho-subterfugio/06-duelista.md:65
- **Codebase truth:** `POSTURA_DE_DUELO` only substitutes Tactics for Parry when a Buckler (objtype 0x1B73) is in the off-hand; otherwise it falls back to Parry — ParryController.inc:174-184. Other equipment gives different passives (empty off-hand = evasion, dual wield = +hit, pistol = enables pistol) HitController.inc:291-302. The active simply starts the `recuperacaostamina` stamina-regen timer — there is no "destreza aumenta" — DuelStance.inc:28-46; habilidades.cfg:891 confirms ("Ativar: regenera stamina").
- **Mismatch:** Book states the Tactics-as-Parry passive unconditionally (omits the buckler prerequisite) and misdescribes the active (stamina regen, not a dexterity boost).
- **Severity:** HIGH (drives weapon/shield build choice and is simply wrong about the active)
- **Cross-links:** none

### Movimentação Graciosa — the +acerto/+esquiva bonus is a permanent passive, not a temporary buff
- **Handbook claim:** "Você se esquiva do próximo golpe e recebe um bônus temporário em acerto e esquiva." — src/60-caminho-subterfugio/06-duelista.md:54
- **Codebase truth:** Activation only guarantees one dodge (BEFORE_HIT, SingleUse). The +TACTICS/10 attacker hit and −TACTICS/10 defender evasion is a *permanent* passive granted by simply having the ability (HasAbility), not a temporary buff tied to activation — GracefulMovement.inc:19-68; HitController.inc:282-288
- **Mismatch:** "bônus temporário" — the bonus is permanent and unconditional, not temporary.
- **Severity:** MED
- **Cross-links:** none

### Desmoralizar / Finta Esgrimista / Mestre Duelista — consistent
- **Handbook claim:** target gets hit penalties (lvl 10); in stance every received attack is counter-attacked (lvl 14); re-rolls every missed attack for some seconds (lvl 20) — src/60-caminho-subterfugio/06-duelista.md:76,90,101
- **Codebase truth:** Demoralize applies `finta` timer (−hit) Demoralize.inc:23-66; SwashbucklerFeint `SetCounterAttack(true)` on every BEFORE_HIT SwashbucklerFeint.inc:38-50; MasterDuelist `attack_once` OnMiss for Tactics/5s MasterDuelist.inc:39-53.
- **Mismatch:** None.
- **Severity:** (no finding)
- **Cross-links:** none

---

## 07 — Shinobi

Maps to 1300-1304 (`AbilityRegistryLadino.inc:121-125`; Cultura Leste,
habilidades.cfg:754-818). Config names differ from the book (Golpe
Paralisante / Corrida Furtiva) but the mechanics line up.

### Shinobi abilities — verified consistent (stealth/reveal semantics OK)
- **Handbook claim:** Caminho do Ninja: better stealth, hear through walls, throw daggers cheap, kunai-mark + teleport-strike (lvl 2); Bloqueio de Chakra: next hit dmg + paralyze + stam drain scaling Sneak (lvl 6); Bomba de Fumaça: passive +stealth steps & no distance penalty, active smoke hides party and lets the ninja attack without revealing inside it (lvl 10); Névoa Oculta: passive hide-while-attacked, active invisible + can run, revealed on attack or timeout (lvl 14); Postura Fantasma: teleport-strike + paralyze nearby foes (lvl 20) — src/60-caminho-subterfugio/07-shinobi.md:51,62,73,87,99
- **Codebase truth:** NinjaPath two-state mark/teleport NinjaPath.inc:20-180; VitalStrike paralyze Sneak/10 + drain Sneak/3 + bonus dmg VitalStrike.inc:17-72; SmokeScreen `ApplySmokeProtection` on ninja only, party just `hidden:=1` and "reveal normally on attack" SmokeScreen.inc:13-86; StealthRun hidden+concealed Sneak/5s, no `hiddenattack` privilege so attacking reveals normally StealthRun.inc:16-48, and the hide-while-attacked passive is honored in sneak.src:145; PhantomStrike teleport+attack+paralyze, re-hides after PhantomStrike.inc:16-94.
- **Mismatch:** None material. "atacar sem se revelar" correctly applies only to the ninja inside the smoke; "revelado ao atacar" matches (no hiddenattack grant on StealthRun).
- **Severity:** (no finding)
- **Cross-links:** none

---

## 08 — Malandro Arcano

Maps to 1000-1004 (`AbilityRegistryLadino.inc:100-104`).

### Reaproveitar Pergaminho — "+reflexos considerável" is actually +5
- **Handbook claim:** "além de ganhar um bônus considerável em reflexos." — src/60-caminho-subterfugio/08-malandro-arcano.md:90
- **Codebase truth:** `intrinsicStats.inc:145-147` grants exactly **+5** reflexos for `REAPROVEITAR_PERGAMINHO`. (The ability's own doc-comment and habilidades.cfg:630 claim "+40", which is itself wrong vs the +5 in code.)
- **Mismatch:** +5 is not a "considerável" bonus; the book (and the config) overstate it. The book intentionally abstracts numbers, so this is borderline.
- **Severity:** LOW
- **Cross-links:** none

### Ladrão de Magias — "usa reflexos no lugar da vontade" not confirmed
- **Handbook claim:** "devolver as magias conjuradas contra você, usando seus reflexos no lugar da vontade." — src/60-caminho-subterfugio/08-malandro-arcano.md:65
- **Codebase truth:** While `spellthief` cooldown is active, `CastSpell` swaps the spell target back to the caster (full reflection) — tnmagery.inc:17-25; SpellThief sets cd for ARCANA seconds SpellThief.inc:19-29. The reflection swap is unconditional; a Reflexos-for-Will resist substitution was not located.
- **Mismatch:** Core reflection works; the specific "Reflexos instead of Will" resist claim is unverifiable from the reflection path.
- **Severity:** UNVERIFIED
- **Cross-links:** none

### Sensibilidade / Invocação do Caos / Fantasma — consistent
- **Handbook claim:** cast simple tricks + use scrolls (lvl 2); summon a random creature, sheep to demon, fights for you (lvl 10); pass unnoticed by hostile creatures for a while (lvl 20) — src/60-caminho-subterfugio/08-malandro-arcano.md:54,76,101
- **Codebase truth:** ArcaneSensitivity (ARCANA-cast tricks + scrolls) ArcaneSensitivity.inc:11-17; ChaosInvocation 1d10 table incl. SuccubusVermelha (demon, roll 8), summoned as ally ChaosInvocation.inc:22-74 — note "sheep" is only the unreachable default since rolls 1-10 are all assigned (flavor-level, LOW); Ghost `ghost` timer 60s, invisible to hostiles Ghost.inc:12-29.
- **Mismatch:** None material.
- **Severity:** (no finding)
- **Cross-links:** none

---

## 09 — Ladrão

Maps to 1600-1604 (`AbilityRegistryLadino.inc:142-146`). All five align.

### Ladrão abilities — verified consistent
- **Handbook claim:** never fail stealing coins/jewels/keys + speed burst on failure (lvl 2); plant a marked item that returns + copy keys toolless (lvl 6); crowd bonus to theft + active stun (lvl 10); steal weapon on a furtividade check + survive a fatal blow (lvl 14); hide under combat threat + run while hidden (lvl 20) — src/60-caminho-subterfugio/09-ladrao.md:52,64,76,90,102
- **Codebase truth:** Pickpocket passive + 30s `velocidade` Pickpocket.inc:18-38; DirtyTrick plant/mark item return 300s + key copy DirtyTrick.inc:19-99; Bully crowd-steal passive + stun OnBeforeDamage Bully.inc:17-58; ExpertTrickster Pickpocketing-vs-Tactics weapon steal + reduced unconscious time on death ExpertTrickster.inc:19-89; EmergencyEscape 30s conceal + sneak.src:145 bypass EmergencyEscape.inc:17-37.
- **Mismatch:** None material. "teste de furtividade" maps to the PICKPOCKETING skill check (consistent with habilidades.cfg:977).
- **Severity:** (no finding)
- **Cross-links:** Trapaceiro/Fuga both rely on `sneak.src:145` hide-while-threatened bypass (shared with Shinobi's StealthRun).

---

## 10 — Vivisseccionista

Maps to 1400-1404 (`AbilityRegistryLadino.inc:128-132`).

### Precisão Cirúrgica — "mais precisos" not implemented; only +damage
- **Handbook claim:** "Suas mãos são extremamente firmes, fazendo com que seus golpes sejam mais precisos e causem mais dano." — src/60-caminho-subterfugio/10-vivisseccionista.md:92
- **Codebase truth:** `SurgicalPrecision.OnPassiveBeforeDamage` only adds SCIENCE/20 to the three physical damage types — there is no accuracy/hit-chance component anywhere — SurgicalPrecision.inc:19-39. (habilidades.cfg:858 says only "mais precisos", with no damage — i.e. book and config each describe a different half of reality.)
- **Mismatch:** The "mais precisos" (higher accuracy) half of the book's claim is unimplemented; code is pure +damage.
- **Severity:** MED
- **Cross-links:** none

### Mestre dos Venenos / Golpe Sangrento / Veneno Puro / Mumificação — consistent
- **Handbook claim:** apply any poison to food/weapons without damage (lvl 2); next hit causes intense bleed (lvl 6); maximize poison duration/dosage/paralyze (lvl 10); immune to crits and poison (lvl 20) — src/60-caminho-subterfugio/10-vivisseccionista.md:50,62,74,104
- **Codebase truth:** PoisonMaster stub → envenenar.src PoisonMaster.inc:11-17; BloodyStrike `bleed` ALCHEMY/10s BloodyStrike.inc:13-54; PurePoison stub → DamageController doubles paralyze + alchemy spec PurePoison.inc:13-19; Mummification blocks all crits (DamageController.inc:951-954) + poison Mummification.inc:12-18.
- **Mismatch:** None material.
- **Severity:** (no finding)
- **Cross-links:** none

---

## Summary

| Severity   | Count |
|------------|-------|
| HIGH       | 3     |
| MED        | 4     |
| LOW        | 4     |
| INVENTED   | 1     |
| UNVERIFIED | 3     |

### Top 5 by impact

1. **HIGH — Assassino table fully misaligned** (01-assassino.md:31-32,47-65): book invents "Salto das Sombras" + "Ataque Furtivo" from the single `Assassinar`, wrong tier order, and omits `Manto das Sombras` entirely.
2. **HIGH — Sentença Profana religion riders wrong/absent** (03-cacador-de-demonios.md:66-74): Lóre & Ush'krar unimplemented; Bokk Bharaz/Eluith'orth/Kai do different things than advertised — 5 of 7 religion payoffs misstated.
3. **HIGH — Reflexo Estratégico (Postura de Duelo)** (06-duelista.md:65): Tactics-as-Parry needs a buckler (omitted prerequisite); active is stamina regen, not "destreza aumenta".
4. **MED — Distrair Oponentes** (04-cacador-de-tesouros.md:66): advertised as "pass unnoticed", code is a 5s AoE paralyze of adjacent foes.
5. **MED — Valdruk "forma borrada"** (05-dancarino-das-sombras.md:79): the on-exit blur/evasion is the separate `blur` spell, not part of Valdruk; Valdruk only conceals.
