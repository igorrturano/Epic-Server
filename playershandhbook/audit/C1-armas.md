# C1 — Caminho das Armas — Handbook ↔ Codebase Audit

Domain: `playershandhbook/src/40-caminho-armas/` (00-intro + 10 class files).
Mapping resolved via `pkg/systems/abilities/include/models/Ability.inc`
(ABILITY_ID enum) and `AbilityRegistryGuerreiro.inc` (IDs 1–904 = Guerreiro
path). Effects verified against `pkg/systems/abilities/include/classes/<class>/`
plus inline implementations in combat/spells/attributes.

---

## 00-intro

No mechanical mismatches. Skill-cost table, heroism-stack (125) and
resistance values are flavor/abstracted and not contradicted by code read.

---

## 01-aklat-tur

### Tiro Poderoso is the only Arqueiro overlap — n/a here
(Aklat'tur abilities 500–504 are distinct; see below.)

### Sangue por Sangue — lifesteal fraction (informational, not a mismatch)
- **Handbook claim:** "Uma parte do dano causado retorna como cura." — src/40-caminho-armas/01-aklat-tur.md:64
- **Codebase truth:** Heals `CInt(totalDamage * 0.2)` = 20% of damage dealt — pkg/systems/abilities/include/classes/aklattur/BloodForBlood.inc:54
- **Mismatch:** None. Book intentionally abstracts; the in-file doc comment says "50%" but the code is 20% — that is a code/comment drift, not a handbook error.
- **Severity:** LOW
- **Cross-links:** none

### Resistência do Deserto — verified accurate
- **Handbook claim:** "Você ganha mais HP máximo e resistência ao calor." — src/40-caminho-armas/01-aklat-tur.md:89
- **Codebase truth:** +HP via attributes.inc:500; heat resistance via caloreffect.src:118, both gated on `HasAbility(RESISTENCIA_DO_DESERTO)` — pkg/systems/attributes/include/attributes.inc:500, pkg/systems/nature/caloreffect.src:118
- **Mismatch:** None — accurate.
- **Severity:** —

### Carnificina — verified accurate
- **Handbook claim:** "A cada morte, suas habilidades de caçador são reativadas por alguns segundos." — src/40-caminho-armas/01-aklat-tur.md:100
- **Codebase truth:** OnKill re-activates FURIA_DO_DESERTO + SANGUE_POR_SANGUE + MARCA_DA_PRESA for 20s each — pkg/systems/abilities/include/classes/aklattur/Carnage.inc:31-44
- **Mismatch:** None.
- **Severity:** —

---

## 02-arqueiro

### Tiro Poderoso — wrong effect entirely
- **Handbook claim:** "Você dispara uma flecha em linha reta que causa dano em todos no caminho dela." — src/40-caminho-armas/02-arqueiro.md:55
- **Codebase truth:** `PowerShot.ExecuteOn` does a single guaranteed hit on ONE chosen target and **paralyzes** it for `Ranged/10` seconds. There is no line/path AoE anywhere in the file — `SetObjProperty(who,"#GuaranteedHit",1); who.attack_once(targ); TS_StartTimer(targ,"paralysis",duration)` — pkg/systems/abilities/include/classes/arqueiro/PowerShot.inc:71-81
- **Mismatch:** Book describes a line AoE; code is a single-target guaranteed-hit + paralysis. The line-AoE description matches the Samurai Fúria do Dragão / Tiro flavor, not this ability. (The config `desc` at habilidades.cfg hab 1 is also wrong — same false text — but audit scope is handbook vs code.)
- **Severity:** HIGH (a player picks this expecting an AoE; it is a single-target stun)
- **Cross-links:** none

### Tiro Carregado — "destreza ao dano" actually scales with Ranged, not Dex
- **Handbook claim:** "seus disparos adicionam destreza ao dano." — src/40-caminho-armas/02-arqueiro.md:67
- **Codebase truth:** `OnBeforeDamage` adds `CInt(RANGED/2)` piercing damage. The bonus is derived from the RANGED skill, not the Dexterity stat — pkg/systems/abilities/include/classes/arqueiro/ChargedShot.inc:49-51
- **Mismatch:** "destreza" (Dexterity) is misleading; the scaling source is the Ranged skill. Duration TT/5 (Tactics) matches.
- **Severity:** MED (misleads stat investment)
- **Cross-links:** none

### Tiro em Movimento — verified accurate
- **Handbook claim:** "Você consegue atirar enquanto se move." — src/40-caminho-armas/02-arqueiro.md:84
- **Codebase truth:** Pure passive; effect applied inline via `GrantPrivilege(who,"firewhilemoving")` (documented in ShootWhileMoving.inc:8) — pkg/systems/abilities/include/classes/arqueiro/ShootWhileMoving.inc
- **Mismatch:** None.
- **Severity:** —

### Encontrar Brecha / Disparo Múltiplo — verified accurate
FindOpening `SetIgnoreArmor()` for TT/5s (FindOpening.inc:31); MultiShot fires
`TACTICS/40 + 1` rounds (MultiShot.inc:35-42). Both match the book.

---

## 03-barbaro

### Fúria — undocumented CanActivate gate (no heavy armor)
- **Handbook claim:** "Você entra num estado de fúria cujo dano adicional e duração são baseados no seu *Tactics*." — src/40-caminho-armas/03-barbaro.md:52
- **Codebase truth:** `CanActivate` iterates equipped armor and blocks activation entirely if any piece has `TipoProtecao["Armadura"]` (heavy armor) — "Voce nao pode usar Furia com armadura." — pkg/systems/abilities/include/classes/barbaro/Fury.inc:26-39
- **Mismatch:** The ability page never states Fúria is unusable while wearing armor (only the class lore prose mentions "desprovidos de armaduras"). This is a hard activation block the ability text omits.
- **Severity:** MED (build-relevant hard gate)
- **Cross-links:** none

### Resiliência Bárbara — passive physical-resist bonus was removed
- **Handbook claim:** "Você ignora a dor de golpes recebidos, ganhando um bônus na resistência física. Ativamente, recupera HP por um período." — src/40-caminho-armas/03-barbaro.md:63
- **Codebase truth:** Post-Phase 7 the passive absorption/physical-resist was **deleted**; the ability now ONLY starts a `recuperacaoaligera` HP-regen timer and requires Fúria active (`CanActivate` fails without "furia" cooldown) — pkg/systems/abilities/include/classes/barbaro/BarbarianResilience.inc:16-18,27-36
- **Mismatch:** (1) The "bônus na resistência física" passive no longer exists in code. (2) Undocumented prerequisite: must already be in Fúria.
- **Severity:** HIGH (claims a defensive bonus that the code does not grant)
- **Cross-links:** Defensor "Manobra da Resiliência" uses the same `recuperacaoaligera` regen (src/40-caminho-armas/06-defensor.md:96); Monge "Corpo e Mente" too (src/40-caminho-armas/07-monge.md:105)

### Fúria Indomável — half the listed effects are not implemented; wrong direction
- **Handbook claim:** "Você aterroriza seus inimigos, reduzindo a chance de acerto deles. Além disso, fica imune a críticos e paralisia, e passa a usar *Fortitude* para resistir diversas magias." — src/40-caminho-armas/03-barbaro.md:74
- **Codebase truth:** While `GetCooldown(who,"furia")`: sets `vitalidadeinabalavel` (crit immunity) — UntamableFury.inc:24-43. HitController gives the **barbarian's own** attack `+10 hit_chance` (not an enemy accuracy reduction) — pkg/systems/combat/include/Controller/HitController.inc:362-366. Paralysis immunity and "Fortitude as willpower vs magic" are explicitly unimplemented (in-file comment: "Future: paralysis immunity, fortitude as willpower").
- **Mismatch:** (1) "reduzindo a chance de acerto deles" is backwards — code grants the barbarian +10 to his own accuracy, it does not lower enemies' hit chance. (2) "imune a paralisia" — not implemented. (3) "usa Fortitude para resistir magias" — not implemented. (4) Undocumented prerequisite: requires Fúria active.
- **Severity:** HIGH (two of four claimed effects are absent; one is inverted)
- **Cross-links:** none

### Fúria Implacável — undocumented prerequisite
- **Handbook claim:** "Você recebe um bônus em HP e sempre contra-ataca quando for acertado." — src/40-caminho-armas/03-barbaro.md:89
- **Codebase truth:** +30 HP via hitsmod + counter-attack each hit — matches. But `CanActivate` requires Fúria active or it fails ("Voce precisa estar em furia") — pkg/systems/abilities/include/classes/barbaro/RelentlessFury.inc:34-43
- **Mismatch:** Effect correct; the "must be in Fúria" prerequisite is omitted.
- **Severity:** MED
- **Cross-links:** none

### Destruidor — verified accurate
Doubles fury damage (Fury.inc:69-71) and doubles crit chance
(DamageController, per Destroyer.inc:9). Matches "ainda mais dano em fúria e
chance de crítico aumenta."

---

## 04-cacador  ⚠ HANDBOOK CLASS IS OUT OF SYNC WITH CODE

The handbook lists Lv2 **Camuflagem**, Lv6 **Caçador Experiente**, Lv10
**Mestre de Feras**, Lv14 **Espreita Selvagem**, Lv20 **Caçador Implacável**.
The registered Guerreiro 100–104 abilities are **Vínculo de Guerra,
Comando de Feras, Despertar Selvagem, Espreita Selvagem, Caçador Implacável**
(AbilityRegistryGuerreiro.inc:106-110). The first two slots are entirely
different abilities; "Camuflagem" and "Caçador Experiente" are not abilities in
`habilidades.cfg` at all.

### Camuflagem (Lv2) does not exist — slot is Vínculo de Guerra
- **Handbook claim:** "Camuflagem … Ativa … 15 stamina … 120s … Você consegue se esgueirar fora de cidades. Caso tenha um pet, ele se esgueirará junto e, se ele acertar o próximo ataque, será um crítico." — src/40-caminho-armas/04-cacador.md:31,44-51
- **Codebase truth:** ABILITY_ID 100 = `VINCULO_DE_GUERRA` / "Vínculo de Guerra", a **passive** with no stamina/cooldown: +1 war-pet bond and unlocks crafting throwable nets/hunting traps — pkg/systems/abilities/include/classes/cacador/WarBond.inc:3-16; habilidades.cfg hab 100
- **Mismatch:** Wrong ability name, wrong type (Ativa vs Passiva), invented cost/cooldown, and a stealth/pet-crit effect that no code implements for this slot.
- **Severity:** HIGH (entire Lv2 entry is fictional)
- **Cross-links:** none

### Caçador Experiente (Lv6) does not exist — slot is Comando de Feras
- **Handbook claim:** "Caçador Experiente … Ativa … 20 stamina … 180s … Sabe como fazer redes jogáveis usando cordas e instalar armadilhas de caça. Além disso, é capaz de rastrear e proteger um grupo … dos efeitos adversos do clima." — src/40-caminho-armas/04-cacador.md:32,54-61
- **Codebase truth:** ABILITY_ID 101 = `COMANDO_DE_FERAS` / "Comando de Feras", Ativa, delay 120 / stam 20: opens a war-pet battle-command menu (Atacar/Proteger/Emboscar/Recuar/Farejar) gated by Beastmastery thresholds {10,30,50,70,90} — pkg/systems/abilities/include/classes/cacador/BeastCommand.inc:27-103; habilidades.cfg hab 101 (delay 120, stam 20)
- **Mismatch:** Wrong name; wrong cooldown (book 180s, config 120s); the net/trap/group-weather text actually belongs to **Vínculo de Guerra** (nets/traps) and **Despertar Selvagem** (group weather + tracking), not this slot. The real effect (pet command menu) is entirely absent from the book.
- **Severity:** HIGH (entire Lv6 entry is fictional + cooldown wrong)
- **Cross-links:** net/trap crafting is actually Vínculo de Guerra (WarBond.inc:7-10); group weather/tracking is Despertar Selvagem (WildAwakening.inc:8-11)

### Mestre de Feras (Lv10) — name mismatch, effect roughly correct
- **Handbook claim:** "Mestre de Feras … Passiva … despertar a verdadeira natureza do seu animal fiel, alterando seus atributos físicos e despertando um instinto selvagem que aumenta o dano crítico do animal de acordo com o tipo de fera." — src/40-caminho-armas/04-cacador.md:33,64-71
- **Codebase truth:** ABILITY_ID 102 is registered as **"Despertar Selvagem"** (`MESTRE_DE_FERAS` enum, `WildAwakening` class). Effect: awakens pet, alters STR/DEX/HP, instinct-based crit damage by beast type, Beastmastery-scaled HP/damage, group weather resist + Survival tracking — pkg/systems/abilities/include/classes/cacador/WildAwakening.inc:3-12; habilidades.cfg hab 102 ("Despertar Selvagem")
- **Mismatch:** Effect description is accurate; the displayed ability **name** differs (book "Mestre de Feras" vs config "Despertar Selvagem"). The book also omits the group weather-resist + Survival-tracking sub-effects (those are listed under the fictional "Caçador Experiente" instead).
- **Severity:** MED (effect right, name and slot labeling wrong — confusing across the whole class block)
- **Cross-links:** the weather/tracking text is misplaced on src/40-caminho-armas/04-cacador.md:61

### Espreita Selvagem (Lv14) — verified accurate
- **Handbook claim:** "+1 slot de animais domesticados, +1 animal fidelizado, você e seu pet mais furtivos nas sombras." — src/40-caminho-armas/04-cacador.md:78-85
- **Codebase truth:** Passive; inline doubles stealth steps, +1 tame slot, bonded-pet limit 2 — pkg/systems/abilities/include/classes/cacador/WildStalker.inc:6-13
- **Mismatch:** None.
- **Severity:** —

### Caçador Implacável (Lv20) — verified accurate
- **Handbook claim:** "30 stamina, 600s … bônus em fortitude e pontos de vida … ignora as defesas até eliminá-la." — src/40-caminho-armas/04-cacador.md:34,89-96
- **Codebase truth:** ID 104, BeforeDamage `SetIgnoreArmor(true)` for `Survival/5`s; +Fortitude (intrinsicMods), +25 HP, +20 hitsmod on purchase — pkg/systems/abilities/include/classes/cacador/RelentlessHunter.inc:6-42; habilidades.cfg hab 104 (delay 600, stam 30)
- **Mismatch:** None (numbers match config).
- **Severity:** —

---

## 05-combatente

### Contra-Empunhadura — verified accurate
- **Handbook claim:** "empunhando uma arma de duas mãos e defender um golpe, você desarma o oponente … Ativamente, adiciona força ao dano." — src/40-caminho-armas/05-combatente.md:43
- **Codebase truth:** OnParry disarms only when parrying with a two-handed weapon (not shield, not vs unarmed, within 2 tiles), disarm = TT/5s; OnBeforeDamage adds `STR mod ×2` to all physical types — pkg/systems/abilities/include/classes/combatente/Halfsword.inc:21-97
- **Mismatch:** None — accurate.
- **Severity:** —

### Golpe Mortal / Contragolpe Cruzado / Golpes Furiosos / Sentença de Morte — verified accurate
MurderBlow: BeforeDamage `SetIgnoreArmor` + stun TT/10 (min2/max10), OnKill
attacks a nearby enemy (MurderBlow.inc:34-107). CrossCut: guaranteed first
parry + counter-attack for TT/5s (CrossCut.inc:30-66). RageCut: `TT/40`
extra attacks min1/max5 (RageCut.inc:35-41). DeathSentence: forces critical
for TT/5s (DeathSentence.inc:29-33). All match the book.

---

## 06-defensor

### Provocar — evasion scales with nearby hostiles, not "alvos provocados"
- **Handbook claim:** "Seu bônus em esquiva aumenta com a quantidade de alvos provocados." — src/40-caminho-armas/06-defensor.md:57
- **Codebase truth:** Taunt sets `resistenciaheroica` cooldown (PARRY/5s). HitController then reduces incoming hit_chance by a tier based on `ListHostiles(defender,3).size()`: ≥1 → −5, ≥3 → −10, ≥5 → −15 — pkg/systems/abilities/include/classes/defensor/Taunt.inc:72; pkg/systems/combat/include/Controller/HitController.inc:380-390
- **Mismatch:** The evasion tier keys off the count of hostiles within 3 tiles at hit-time, not the number of targets actually taunted. Close in spirit; wording is imprecise.
- **Severity:** LOW
- **Cross-links:** none

### Manobra Desafiadora — defensive branch is DISARM, not "lutar melhor contra vários"
- **Handbook claim:** "Caso sua postura seja ofensiva, golpeará com o escudo, atordoando o alvo … Senão, lutará melhor contra vários oponentes." — src/40-caminho-armas/06-defensor.md:85
- **Codebase truth:** OnParry (SingleUse). Offensive posture: shield-bash paralysis for PARRY/10s. Defensive posture: **disarms the attacker** (moves weapon to ground, `desarmar` timer PARRY/10s) — there is no "fights better vs multiple opponents" logic — pkg/systems/abilities/include/classes/defensor/ChallengerManeuver.inc:57-117
- **Mismatch:** Defensive branch is a disarm, not a multi-opponent buff. Also the ability triggers on a successful parry (OnParry, single-use), which the book's phrasing ("golpeará") does not convey.
- **Severity:** HIGH (defensive half of the ability is described as a different effect)
- **Cross-links:** none

### Manobra de Defesa / Resiliência / Invencibilidade — accurate (minor)
DefenseManeuver: offensive → guaranteed next parry; defensive →
`defesapersistente` 20s hit-reduction (DefenseManeuver.inc:36-52) — matches
"defenderá o próximo golpe / bônus na defesa". ResilienceManeuver: offensive →
crit/stun immune PARRY/3s; defensive → `recuperacaoaligera` regen (fixed 60s,
not PARRY-scaled) (ResilienceManeuver.inc:37-58) — book says "Ambos os efeitos
são baseados no seu Parry"; the defensive HP-regen duration is a flat 60s, not
Parry-scaled (LOW). InvincibilityManeuver: offensive counter for PARRY/3s;
defensive +25 evasion cancel-on-move (InvincibilityManeuver.inc:45-90) —
matches. All four maneuvers also require a posture set via `.postura` or
`CanActivate` fails — the book documents the `.postura` command (line 59) so
this is adequately covered.

### Manobra da Resiliência — defensive regen not Parry-scaled
- **Handbook claim:** "Na defensiva, recupera HP por alguns segundos. Ambos os efeitos são baseados no seu *Parry*." — src/40-caminho-armas/06-defensor.md:96
- **Codebase truth:** Defensive branch is a flat `TS_StartTimer(who,"recuperacaoaligera",60)` — fixed 60s, independent of Parry. Only the offensive crit/stun-immunity uses PARRY/3 — pkg/systems/abilities/include/classes/defensor/ResilienceManeuver.inc:52-55
- **Mismatch:** "Ambos … baseados no seu Parry" is false for the defensive half (fixed duration).
- **Severity:** LOW
- **Cross-links:** shared `recuperacaoaligera` regen with Bárbaro Resiliência Bárbara and Monge Corpo e Mente

---

## 07-monge

### Artista Marcial — multi-attack scales with Wrestling, not Tactics; passive has gate
- **Handbook claim:** "dispara uma sequência de golpes rápidos. Sua esquiva passiva também aumenta. Ambos os efeitos escalam com o seu *Tactics*." — src/40-caminho-armas/07-monge.md:56
- **Codebase truth:** Active extra-attack count = `WRESTLING/30` (min1/max5), not Tactics. The Tactics/20 passive **damage** bonus applies only with a Wrestling-attribute weapon AND no heavy armor equipped. The passive dodge (−Tactics/5 enemy hit) is in HitController — pkg/systems/abilities/include/classes/monge/MartialArtist.inc:39-92
- **Mismatch:** The attack-count scales with Wrestling (the monk's combat skill), not Tactics as stated. The book also omits the "monk weapon + no armor" precondition on the passive damage bonus.
- **Severity:** MED (the book itself notes Monge uses Wrestling at line 47, so partly self-consistent, but the "escalam com Tactics" line is wrong for the attack count)
- **Cross-links:** none

### Estilo do Caranguejo — "proteção física" passive was removed (now evasion)
- **Handbook claim:** "Este estilo concede proteção física e resistência climática passivamente. Ativamente, seu próximo ataque causará sangramento." — src/40-caminho-armas/07-monge.md:81
- **Codebase truth:** Post-Phase 7 the passive physical absorption was replaced by an **evasion** bonus: `OnPassiveBeforeHitDefender` subtracts `Tactics/10` from the attacker's hit chance. Climate resist still lives in coldeffect/caloreffect. Active branch: bleed for Tactics/10s on next hit — pkg/systems/abilities/include/classes/monge/CrabStyle.inc:9-23,69-86
- **Mismatch:** "proteção física" passive no longer exists; it is now an evasion (dodge) passive. Climate resist + active bleed match.
- **Severity:** HIGH (claims damage mitigation that the code does not provide)
- **Cross-links:** same Phase-7 absorption removal as Bárbaro Resiliência Bárbara (src/40-caminho-armas/03-barbaro.md:63) and the Dragon Stance note below

### Postura do Dragão / Postura do Tigre / Corpo e Mente — verified accurate
DragonStance: SingleUse bonus damage Tactics/10 + projectile immunity
("Desviar Disparos") Tactics/5s (DragonStance.inc:45-79) — matches (the
removed passive absorption isn't claimed by the book). TigerStance: guaranteed
hit + disarm + knockback + path AoE Tactics/2 (TigerStance.inc:36-119) —
matches "arremessa para longe, desarmando, todos no caminho atingidos".
BodyAndMind: HP regen `recuperacaoaligera` Tactics s; +5 all resist passive in
intrinsicMods (BodyAndMind.inc:24-34) — matches.

---

## 08-mutante

### Óleo do Caçador — hit bonus scales with Science, book says "alquimia"
- **Handbook claim:** "Adicionalmente, você recebe um bônus de acerto baseado na sua habilidade em alquimia." — src/40-caminho-armas/08-mutante.md:64
- **Codebase truth:** Passive hit bonus is `SCIENCE/5` (HitController:284) and category damage `SCIENCE/20` (DamageController_legacy:558), per HunterOil.inc:15-18; the ability stores an "Óleo" type property rather than crafting a physical oil item — pkg/systems/abilities/include/classes/mutante/HunterOil.inc:8-55
- **Mismatch:** Scaling skill is Science, not Alchemy as the book states (misleads which skill to train). "Criar óleos para armas" is abstracted — code just toggles a damage-type tag.
- **Severity:** MED (stat-investment misdirection)
- **Cross-links:** Forma Adaptável also scales with Science (src/40-caminho-armas/08-mutante.md:53) — same Alchemy/Science confusion

### Forma Adaptável — scales with Science (book abstracts; minor)
- **Handbook claim:** "você consegue aumentar uma resistência à sua escolha." — src/40-caminho-armas/08-mutante.md:53
- **Codebase truth:** Toggle; choose Fortitude/Reflexes/Will, bonus = `AP_GetSkill(who, SCIENCE)/20` (in-file comment says ALCHEMY/20 but code reads SCIENCE) — pkg/systems/abilities/include/classes/mutante/AdaptableForm.inc:24-57
- **Mismatch:** Effect correct; book abstracts the scaling so not a book error. Noted for the cross-class Alchemy↔Science drift.
- **Severity:** LOW
- **Cross-links:** Óleo do Caçador (above)

### Mutagênicos — "Pedra Filosofal" sub-claim has no code
- **Handbook claim:** "Escolha um aprimoramento em destreza, força ou inteligência. Adicionalmente, você aprende a criar e utilizar a Pedra Filosofal." — src/40-caminho-armas/08-mutante.md:100
- **Codebase truth:** `Mutagenic.ExecuteOn` grants +25 stat + +25 matching vital (hits/stam/mana), toggle off on second use. `ABILITY_ID::MUTAGENICO` has **zero** references anywhere outside its own class file; "filosofal" appears only in the unrelated granadeiro `EnhancedInfusion.inc` and in `habilidades.cfg`. No code grants Pedra Filosofal creation/use via this ability — pkg/systems/abilities/include/classes/mutante/Mutagenic.inc:23-108 (grep `MUTAGENICO` / `filosofal` across pkg/)
- **Mismatch:** The stat-buff half is accurate; the "criar e utilizar a Pedra Filosofal" half is unenforced by any code. Cost (5 mana) matches config.
- **Severity:** MED (claims an unlock the code never delivers)
- **Cross-links:** none

### Transmutar — resin crafting OK; "fórmula 90" unverified
- **Handbook claim:** "Você aprende a manusear a fórmula 90, além de aprender a criar resinas elementais para armas." — src/40-caminho-armas/08-mutante.md:89
- **Codebase truth:** `Transmute.ExecuteOn` implements the elemental-resin crafting (Fire/Electricity/Ice; 10×3 reagents + vial). `formulanove.src`'s `formula90` program has no internal ability gate; whether the TRANSMUTAR ability unlocks it is gated (if at all) at the alchemy-menu caller, not verifiable from the files read — pkg/systems/abilities/include/classes/mutante/Transmute.inc:27-113; pkg/skills/alchemy/potions/formulanove.src:15
- **Mismatch:** Resin crafting matches. The "fórmula 90" unlock could not be confirmed from code.
- **Severity:** UNVERIFIED (fórmula 90 linkage); resin half is accurate
- **Cross-links:** none

### Impulso Metabólico — verified accurate
Passive: potion tier +2, PotionSpecialist buff bonus, negates addiction
penalty (MetabolicBoost.inc:6-9). Matches "poções de aprimoramento duram mais
e concedem benefícios maiores."

---

## 09-paladino

### Presença Divina — "imune a medo" is not implemented
- **Handbook claim:** "Você fica imune a medo e inspira sua party com diferentes comandos de fortalecimento." — src/40-caminho-armas/09-paladino.md:93
- **Codebase truth:** `DivinePresence` implements only the 4 party commands. The only engine reference to `PRESENCA_DIVINA` outside its own file is HitController:363, which merely prevents Fúria Indomável's +10 hit bonus from applying against the defender. The fear spell `illusionist/medo.src:26` grants fear-resist to `AURA_DIVINA` (Clérigo), **not** `PRESENCA_DIVINA`; `bardo/lamento_do_covarde.src` has no ability immunity check either — pkg/systems/abilities/include/classes/paladino/DivinePresence.inc; pkg/systems/spells/spell_logic/illusionist/medo.src:26
- **Mismatch:** No code grants fear immunity to a Presença Divina holder. Invented effect.
- **Severity:** HIGH (a stated defensive immunity that does not exist)
- **Cross-links:** none

### Consagração Divina — verified accurate
- **Handbook claim:** "conjurar magias divinas mesmo utilizando armaduras pesadas … vantagem contra mortos-vivos e fortalece sua vontade." — src/40-caminho-armas/09-paladino.md:55
- **Codebase truth:** +damage vs Undead NPCs Theology/20 (DivineConsecration.inc:16-45, DamageController_legacy:575); heavy-armor divine casting allowed via runegem.src:29 / runecarving.src:25 / p.src:27; Will bonus `Theology/20` (intrinsicStats.inc:184) — pkg/systems/attributes/include/intrinsicStats.inc:184
- **Mismatch:** None — all three claims are backed by code.
- **Severity:** —

### Golpe Divino / Aura de Fé / Campeão Divino — verified accurate
DivineSmite: SingleUse divine damage Theology/2 + religion-specific effect
(5 religions: Veneratio/Bokk/Halleuad/Eluithorth/Kai)
(DivineSmite.inc:32-96) — matches "dano adicional e efeito baseado na
religião". AuraOfFaith: 4-command party menu, banner doubles range +50%
(AuraOfFaith.inc:31-137) — matches "diferentes comandos para fortalecer
party". DivineChampion: wing transform, +Theology/2 STR/AGI/INT 20s,
religion-specific rebuke on each hit (DivineChampion.inc:29-148) — matches
"grande bônus em atributos, ira divina em quem te ataca".

---

## 10-samurai

### Caminho do Andarilho — "no penalty" is actually a +30% bonus, with no armor condition
- **Handbook claim:** "não recebe penalidade para defender sem escudo quando sem armadura ou com armaduras leves de madeira. Ao ativar, garante uma defesa." — src/40-caminho-armas/10-samurai.md:57
- **Codebase truth:** ParryController: parrying with a weapon normally costs `parry_chance × 0.7`; with WANDERER_PATH it becomes `parry_chance × 1.3`. This multiplier is applied **unconditionally** — there is no check for "no armor / wood armor only" — pkg/systems/combat/include/Controller/ParryController.inc:191-197. Active branch: guaranteed next parry (WandererPath.inc:33-49)
- **Mismatch:** (1) The armor-type qualifier ("quando sem armadura ou com armaduras leves de madeira") is an invented restriction — code applies it regardless of worn armor. (2) The effect is not merely "no penalty" — it is a net +30% parry chance (vs the −30% others suffer). Stronger than described.
- **Severity:** HIGH (invented restriction + understated power; both mislead build choices)
- **Cross-links:** none

### Samurai active abilities — undocumented shield-block CanActivate
- **Handbook claim:** Fúria do Dragão / Execução Perfeita / Golpe do Vento listed as plain Ativa with no usage restriction. — src/40-caminho-armas/10-samurai.md:61-94
- **Codebase truth:** WandererPath, DragonFury, PerfectExecution, WindStrike all `CanActivate` return false if `who.shield` ("Um verdadeiro samurai nao usaria escudo.") — DragonFury.inc:26-33; PerfectExecution.inc:31-38; WindStrike.inc:30-37; WandererPath.inc:24-31
- **Mismatch:** Every Samurai active is hard-blocked while a shield is equipped; the ability pages never state this.
- **Severity:** MED (lore-consistent but an undocumented hard gate across 4 of 5 abilities)
- **Cross-links:** none

### Fúria do Dragão / Execução Perfeita / Golpe do Vento / Mestre da Espada — effects accurate
DragonFury: charge 5 tiles, path AoE Tactics/2 (DragonFury.inc:35-100) —
matches "avança em linha reta, dano em todos no caminho". PerfectExecution:
SingleUse guaranteed hit + forced critical (PerfectExecution.inc:40-64) —
matches "se acertar, crítico garantido". WindStrike: SingleUse second attack
on next hit (WindStrike.inc:48-62) — matches "dois golpes consecutivos".
SwordMaster: passive counter-attack on every successful parry
(SwordMaster.inc:19-34) — matches "cada golpe que você defende é respondido".

---

## Summary

| Severity   | Count |
|------------|-------|
| HIGH       | 9     |
| MED        | 6     |
| LOW        | 4     |
| INVENTED   | (folded into HIGH where the rule is fictional) |
| UNVERIFIED | 1     |

### Top 5 by impact

1. **Caçador Lv2 & Lv6 are fictional** (04-cacador.md:31-32,44-61) — "Camuflagem" and "Caçador Experiente" do not exist; real slots are passive Vínculo de Guerra and active Comando de Feras. Two whole ability entries are invented; cooldown on Lv6 also wrong. HIGH.
2. **Arqueiro Tiro Poderoso** (02-arqueiro.md:55) — book says line AoE; code is a single-target guaranteed hit + paralysis. Completely wrong effect. HIGH.
3. **Bárbaro Fúria Indomável** (03-barbaro.md:74) — paralysis immunity and Fortitude-vs-magic not implemented; "reduz acerto dos inimigos" is actually +10 to the barbarian's own accuracy. HIGH.
4. **Samurai Caminho do Andarilho** (10-samurai.md:57) — invented "no armor / wood only" restriction; effect is a +30% parry bonus, not just "no penalty". HIGH.
5. **Paladino Presença Divina "imune a medo"** (09-paladino.md:93) — no code grants fear immunity for this ability (medo.src protects AURA_DIVINA, not PRESENCA_DIVINA). HIGH.

Honorable mentions: Bárbaro Resiliência Bárbara & Monge Estilo do Caranguejo
both still advertise a Phase-7-removed passive physical-resistance bonus
(now HP-regen / evasion respectively); Mutante Mutagênicos advertises a
"Pedra Filosofal" unlock with no backing code.
