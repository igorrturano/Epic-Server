# Audit — Caminho da Sabedoria (C2)

Domain: `playershandhbook/src/50-caminho-sabedoria/` (00-intro + 10 class files).
Code resolved via `pkg/systems/abilities/include/AbilityRegistrySabio.inc`,
`AbilityRegistryLadino.inc` (Deslocador das Sombras → `dancarino_das_sombras`),
`models/Ability.inc`, `configs/habilidades.cfg`, and per-class impls under
`pkg/systems/abilities/include/classes/`.

Cost authority: `Ability.inc:58-59` reads `stam`/`mana` from `habilidades.cfg` —
so the cfg fields ARE the real resource cost. Handbook level (2/6/10/14/20) maps
in order to cfg `classe Sabio 1/3/5/7/10`.

Note: this audit follows _FORMAT.md — pure rounded numbers the book abstracts are
NOT flagged. Stamina-vs-mana is flagged because it changes which pool a build
must invest in.

---

## 00-intro

No mechanical mismatches. "Pilha do Heroísmo 75", "Resistência principal Vontade
(20), secundárias Reflexos/Fortitude (10)" and the skill-cost table are config
claims outside the ability code domain — UNVERIFIED here (defer to the
povos/resists auditor), not contradicted by anything read.

---

## 01-arqueiro-arcano

Code: `classes/arqueiro_arcano/*`, cfg hab 2400-2404.

### Disparo Canalizado — described effect is actually a different ability
- **Handbook claim:** "Você canaliza os elementos em um super efeito baseado no seu elemento encantado: Arco Ígneo: Dispara uma bola de fogo explosiva. Arco Gélido: Congela o alvo. Se já estiver congelado, dobra o dano e quebra o gelo. Arco Elétrico: Dispara uma flecha para um local onde você se teleporta." — src/50-caminho-sabedoria/01-arqueiro-arcano.md:82-85
- **Codebase truth:** `CHANNELED_SHOT` (2402) fires a piercing line shot that hits every target in the line and applies the active element's status: fire→`queimar` 10s, cold→`lentidao` 10s, energy→caster `concealed` for ARCANA/20s. No fireball, no freeze-double-damage, no teleport. — pkg/systems/abilities/include/classes/arqueiro_arcano/ChanneledShot.inc:13-149
- **Mismatch:** The book's per-element "super effects" (fireball / freeze→double dmg / teleport) actually describe `PRISMATIC_SHOT` (2404, the level-20 ability), not Disparo Canalizado. The real Disparo Canalizado is a line-pierce + burn/slow/conceal.
- **Severity:** HIGH (player picks the ability expecting a teleport/fireball nuke; gets a different mechanic)
- **Cross-links:** see "Tempestade Elemental" below (same root: L10/L20 effect text is swapped/invented)

### "Tempestade Elemental" (level 20) — ability does not exist
- **Handbook claim:** "### Tempestade Elemental … No ápice do seu caos elemental, você conjura um efeito massivo … Arco Ígneo: chuva de flechas de fogo … Arco Gélido: chuva de flechas gélidas … Arco Elétrico: relâmpagos entre todos os inimigos próximos." — src/50-caminho-sabedoria/01-arqueiro-arcano.md:100-110
- **Codebase truth:** The level-20 ability is `PRISMATIC_SHOT` / "Disparo Prismatico" (cfg hab 2404). It fires 3 shots in sequence (cold, energy, fire); the active element defines the 3rd-shot effect: fire→fireball on target, energy→ethereal jump, cold→freeze. No "rain of arrows", no AoE incinerate, no chain-lightning between enemies. — pkg/systems/abilities/include/classes/arqueiro_arcano/PrismaticShot.inc:15-122; configs/habilidades.cfg:1449-1459
- **Mismatch:** Book invents a level-20 ability name ("Tempestade Elemental") and an AoE-storm effect set; the real L20 is a single-target 3-shot combo with a different per-element finisher (which is what the book wrongly attributes to L10 Disparo Canalizado).
- **Severity:** HIGH (entire capstone is misrepresented; build/PvP expectations wrong)
- **Cross-links:** "Disparo Canalizado" above

### Ability costs are stamina, not mana
- **Handbook claim:** Gwaenedh "10 mana"; Disparo Canalizado "20 mana"; Tempestade Elemental "30 mana". — src/50-caminho-sabedoria/01-arqueiro-arcano.md:49,79,104
- **Codebase truth:** cfg hab 2400/2402/2404 use `stam 10 / stam 20 / stam 30`, no `mana` field. `Ability.inc:58-59` consumes `stam`. — configs/habilidades.cfg:1410-1458
- **Mismatch:** Resource type is stamina, not mana. A mana-build Arqueiro Arcano cannot pay these costs as the book implies.
- **Severity:** MED (misleads stat investment; not a combat-rule lie)
- **Cross-links:** Druida same pattern (see 05-druida)

NOTE — Gwaenedh (Arco Elemental, 2400): real effect = conjure a temporary elemental bow (permanent with Tirothan/`ARCANE_MASTERY`), requires devotion to Lore. Matches. Ainalaith (Afinidade Elemental, 2401): every 3rd consecutive same-element hit → fire incinerate / cold slow / energy bonus damage; miss resets counter. Matches handbook. Tirothan (`ARCANE_MASTERY`, 2403): +hit, +damage, upgraded bow model. Matches "aumenta dano, alcance e precisão" (ElementalBow.inc:18-19, ArcaneMastery.inc:1-11).

---

## 02-clerigo

Code: `classes/cleric/*`, cfg hab 2800-2804.

### Aura Divina — disease/plague immunity is invented
- **Handbook claim:** "Você se torna imune a doenças e pragas, mais resistente a venenos e não sente mais medo. Além disso, manifesta uma aura de proteção baseada na sua religião." — src/50-caminho-sabedoria/02-clerigo.md:76
- **Codebase truth:** `AURA_DIVINA` (2802) grants: fear immunity (`medo.src:26`, `InflictFear.inc:48`), a FORTITUDE resist second-chance — which covers poison saves (`resists.inc:56,67`), and religion-specific targeted buffs/summons in `DivineAura.inc`. There is NO disease/plague immunity anywhere keyed on `AURA_DIVINA`; disease immunity comes only from a plague mask + medicine (`pkg/skills/medicine/include/disease.inc:48`). — pkg/systems/abilities/include/classes/cleric/DivineAura.inc:29-147; pkg/systems/spells/spell_logic/illusionist/medo.src:26; pkg/systems/charactercreation/include/resists.inc:56,67
- **Mismatch:** "imune a doenças e pragas" has no implementing code. The fear-immunity and poison-resistance parts ARE real.
- **Severity:** MED (one invented sub-clause inside an otherwise-correct ability; could mislead a player into walking into a plague zone)
- **Cross-links:** none

NOTE — Conexão Divina "O uso de armaduras leves e médias não mais limitam seus gestos sagrados": the code grants a *full* hands-free / weapon casting concession via `HasAbility(CONEXAO_DIVINA)` (cast_spell.src:155), not a light/medium-armor-only lift. The book understates/mis-scopes a real concession; LOW imprecision, not flagged as a hard mismatch. Golpe Radiante, Autoridade Divina, Apoteose all match the book's deliberately-abstract "cada religião manifesta efeitos únicos" + costs/types (RadiantSmite.inc, DivineAuthority.inc, Apotheosis.inc; cfg 2801/2803/2804).

---

## 03-construtor-de-golems

Code: `classes/engenheiro/*` (resolved via registry, not folder-name), cfg hab 2700-2704.

### "Gancho de Agarrar" (level 10) — no such standalone ability
- **Handbook claim:** "### Gancho de Agarrar *Habilidade Nível 10* Tipo: Ativa Custo: 25 stamina Cooldown: 120s — Você dispara um gancho mecânico em um alvo. Se ele falhar em esquivar, fica paralisado e é puxado até você." — src/50-caminho-sabedoria/03-construtor-de-golems.md:73-81
- **Codebase truth:** The level-10 ability (cfg hab 2702, `classe Sabio 5`) is "Ombreira Tática" / `TACTICAL_SHOULDER`. It fires whichever gadget is installed on the Steam Arms: `handcannon` (AoE shot), `harpoon` (the grappling hook — reflex save, paralysis = distance, pull along line), or `powdertrap` (mine). The hook is ONE of three gadget modes of Ombreira Tática, not a dedicated ability. — pkg/systems/abilities/include/classes/engenheiro/TacticalShoulder.inc:19-348; configs/habilidades.cfg:1681-1692
- **Mismatch:** Book names a non-existent ability "Gancho de Agarrar" at L10 and omits the real L10 ability "Ombreira Tática" entirely (including its handcannon and powder-trap modes). Cost/cooldown numbers (25 stam / 120s) happen to match hab 2702, but the ability identity and 2 of its 3 modes are missing.
- **Severity:** HIGH (a whole ability is misnamed and its multi-mode nature is hidden; players can't make an informed pick)
- **Cross-links:** "Visor de Mira" below

### Visor de Mira — "seu Gancho de Agarrar nunca erra" overstated/misattributed
- **Handbook claim:** "Você ativa o visor de mira do capacete, ignorando a armadura do alvo nos próximos disparos. Passivamente, seu Gancho de Agarrar nunca erra." — src/50-caminho-sabedoria/03-construtor-de-golems.md:91
- **Codebase truth:** `TARGETING_VISOR` (2703) is BEFORE_DAMAGE single-use: next hit ignores armor (`attackInfo.SetIgnoreArmor()`), duration METALWORKING/5. Its passive interaction with the harpoon: in `TacticalShoulder.inc`, `has_visor` skips the reflex save and raises hook range 10→14 — so the hook can't be dodged only while you have Visor, and it's the Ombreira-Tática harpoon, not a separate "Gancho de Agarrar". — pkg/systems/abilities/include/classes/engenheiro/TargetingVisor.inc:9-44; TacticalShoulder.inc:278-312
- **Mismatch:** The "nunca erra" passive is real but is bound to the (mis-named) Ombreira-Tática harpoon, and "ignorando a armadura nos próximos disparos" is actually a single next-hit window, not a sustained "next shots" buff.
- **Severity:** MED (cross-contaminated by the L10 naming error; mechanic itself roughly exists)
- **Cross-links:** "Gancho de Agarrar" above

### Montagem do Golem — cost/cooldown contradict cfg
- **Handbook claim:** "Tipo: Ativa Custo: 40 stamina Cooldown: 900s" — src/50-caminho-sabedoria/03-construtor-de-golems.md:98-100
- **Codebase truth:** cfg hab 2704: `stam 1`, `delay 5`. — configs/habilidades.cfg:1707-1718
- **Mismatch:** Real cost is 1 stamina with a 5s delay, not 40 stamina / 900s. (Flagged despite the "abstract numbers" rule because this is not rounding — it's a different order of magnitude that changes whether the capstone is spammable.)
- **Severity:** MED
- **Cross-links:** none

NOTE — handbook L6 "Salto a Vapor" correctly maps to `STEAM_JUMP`/RocketJump (cfg hab 2701, `classe Sabio 3`); jump + explosion consuming powder, passive carry capacity is plausible (RocketJump.inc). Punho Explosivo (2700) matches: melee consumes powder, +fire damage, pushback, makes powder without alchemy (ExplosiveFist.inc:25-186).

---

## 04-deslocador-das-sombras

Code: `classes/dancarino_das_sombras/*` (Ladino registry IDs 1800-1804), cfg hab 1800-1804.

### Valdruk — "forma borrada ao sair na escuridão" is invented
- **Handbook claim:** "Você se funde com as sombras, tornando-se completamente invisível por algum tempo. Ao sair da invisibilidade na escuridão, assume uma forma borrada, dificultando que os inimigos te acertem." — src/50-caminho-sabedoria/04-deslocador-das-sombras.md:79
- **Codebase truth:** `VALDRUK` (1802) only starts a `conceal` timed script for ARCANA/5 seconds (plus passive enhancement of illusionist/mage/necro spells). There is no post-conceal "blurred form", no defensive miss-chance buff on exiting concealment, no darkness condition. — pkg/systems/abilities/include/classes/dancarino_das_sombras/Valdruk.inc:24-44 (grep of all VALDRUK refs shows only spell-enhancement use)
- **Mismatch:** The "forma borrada / inimigos te acertam menos ao sair na escuridão" clause has no code.
- **Severity:** INVENTED (rule with no code; also misleads a survivability decision)
- **Cross-links:** none

NOTE — Dança das Sombras (1800): +ARCANA/5 hit chance, passive — matches "acerta mais golpes escalando com Arcana" (ShadowDance.inc:1-12). Lâmina da Escuridão (1801): next hit = mlore/2 forced magic damage + silence + **bleed**; book omits the bleed only (minor). Dança da Morte (1803): cooldown window, each hit teleports behind defender — matches. Forma Umbral (1804): attack while concealed + stay concealed in warmode — matches "ao usar Valdruk pode atacar enquanto invisível". All costs/types match cfg (hab 1801/1802/1803 stam+mana+delay).

---

## 05-druida

Code: `classes/druida/*`, cfg hab 2100-2104.

### Coração da Matilha — described as a summon/pet buff; code is a wildshape buff
- **Handbook claim:** "Os animais que atenderam ao seu chamado permanecem por mais tempo e ganham bônus em suas resistências. Ativamente, um dos animais escolhidos por você é abençoado como o exemplo máximo de sua espécie." — src/50-caminho-sabedoria/05-druida.md:90
- **Codebase truth:** `CORACAO_DA_MATILHA` (2103) is a pure passive (PackHeart.inc has no ExecuteOn): doubles max wildshape forms and doubles polymorph duration (`NaturalBond.inc:299,553`), boosts polymorph stats, and extends druid spell durations (barkskin/haste/encantarpessoa/etc.). `WildSummoning` summon duration is fixed `60 + Theology*3` with NO `CORACAO_DA_MATILHA` check, and summoned creatures get no resist bonus. There is no "blessed exemplary animal" mechanic anywhere. — pkg/systems/abilities/include/classes/druida/PackHeart.inc:1-21; NaturalBond.inc:297-303; WildSummoning.inc:196-204
- **Mismatch:** Book describes a Convocação-Selvagem/pet enhancer (longer-lived summons + resist bonus + a champion animal); the real ability is a self wildshape/spell-duration passive that doesn't touch summons at all. Also book labels it "Ativa" but code has no active component (cfg says `uso Ativa` too, but PackHeart has no ExecuteOn).
- **Severity:** HIGH (a summoner-druid will pick this expecting pet buffs and get none)
- **Cross-links:** none

### Druida active costs are stamina, not mana
- **Handbook claim:** Elo Natural "5 mana", Convocação Selvagem "10 mana", Coração da Matilha "25 mana", Chamado da Forma Primordial "40 mana". — src/50-caminho-sabedoria/05-druida.md:51,62,87,98
- **Codebase truth:** cfg hab 2100/2101/2103/2104 use `stam 15 / stam 10 / stam 25 / stam 40` respectively, no `mana` field. — configs/habilidades.cfg:1234-1289
- **Mismatch:** All Druida active abilities cost stamina, not mana (and Elo Natural's value is 15, not 5).
- **Severity:** MED (misleads stat investment)
- **Cross-links:** Arqueiro Arcano same pattern (01-arqueiro-arcano)

NOTE — Elo Natural (2100): cast/speak/keep jewelry while polymorphed + study wild forms; book says "usar itens" but code only preserves jewelry (LOW imprecision). Instinto de Sobrevivência (2102): passive HP-gated polymorph rage IS implemented (`DamageController_legacy.inc:612`, fires when HITS ≤ THEOLOGY/2 while polymorphed) — matches the book's passive description; the code also has an extra active AoE-heal the book doesn't mention (omission, not a false claim). Chamado da Forma Primordial (2104): elemental transform via `formaprimordial.src` — matches.

---

## 06-granadeiro

Code: `classes/granadeiro/*`, cfg hab 2500-2504.

### Infusão Química — book says Ativa/15 mana/20s; code is a passive unlock
- **Handbook claim:** "### Infusão Química *Habilidade Nível 6* Tipo: Ativa Custo: 15 mana Cooldown: 20s — Você é capaz de alterar as propriedades químicas de uma poção explosiva, modificando seu efeito." — src/50-caminho-sabedoria/06-granadeiro.md:57-64
- **Codebase truth:** `INFUSAO_QUIMICA` (2501) is a passive gate — `ExecuteOn` just `return true`; its only function is to unlock the "Elemento" tab inside the Alquimista-Experiente explosive-config gump. cfg hab 2501 = `uso Passiva`, no `stam`/`mana`/`delay`. — pkg/systems/abilities/include/classes/granadeiro/ChemicalInfusion.inc:1-18; configs/habilidades.cfg:1473-1479
- **Mismatch:** Type (Ativa→Passiva), cost (15 mana → none), cooldown (20s → none), and framing ("modify a potion on use" → "passively unlocks an element-modification menu tab") are all wrong.
- **Severity:** HIGH (every stated stat for this ability is false)
- **Cross-links:** none

### Alquimista Experiente — "próxima poção dispensa o processo alquímico" not implemented
- **Handbook claim:** "A próxima poção não precisará de todo o processo alquímico. Adicionalmente, suas poções consomem menos reagentes." — src/50-caminho-sabedoria/06-granadeiro.md:53
- **Codebase truth:** `ALQUIMISTA_EXPERIENTE` (2500) active opens the explosive-modification gump `menuExplosivos2` (player) / throws an explosive (NPC). Its passive is a flat −50% reagent cost (alchemybook.src/dyebook.src). There is no "skip the brewing process for the next potion" effect. — pkg/systems/abilities/include/classes/granadeiro/ExperiencedAlchemist.inc:1-31
- **Mismatch:** The "próxima poção não precisará de todo o processo alquímico" active is invented; the real active is an explosive-config gump. Reagent reduction part is correct.
- **Severity:** MED (the marquee active of the level-2 ability is misdescribed)
- **Cross-links:** none

NOTE — Bombardeiro Eficiente (2502) "produz o dobro de explosivos" matches code (doubles bomb production, EfficientBombardier.inc). Essência Explosiva (2503): code grants explosion + walk-on-potion resistance — matches "maior resistência aos efeitos de poções explosivas"; the "modificar em estação de alquimia" crafting clause is not in the ability class — UNVERIFIED (may live in an alchemy station file). Infusão Aprimorada (2504): creates Pedra Filosofal (death protection) — matches; type/cost (Ativa, stam 35, 900s) matches cfg.

---

## 07-lamina-arcana

Code: `classes/lamina_arcana/*`, cfg hab 2300-2304.

### Armadura Arcana — "Raio: ganha velocidade de movimento" is false
- **Handbook claim:** "Se for acertado à curta distância, sua armadura reage: … **Raio**: Você ganha velocidade de movimento." — src/50-caminho-sabedoria/07-lamina-arcana.md:89-92
- **Codebase truth:** `ARMADURA_ARCANA` (2303) passive-on-hit: fire weapon → `queimar` on attacker, ice → `congelamento` on attacker, energy → `lentidao` (slow) on the **attacker**. There is no self movement-speed buff for the energy element; energy slows the attacker. — pkg/systems/abilities/include/classes/lamina_arcana/ArcaneArmor.inc:75-85
- **Mismatch:** Book says the wearer gains move speed on Raio; code instead slows the attacker. Wrong beneficiary and wrong effect.
- **Severity:** HIGH (a kiting Lâmina Arcana picks Raio expecting personal speed and gets an attacker-slow instead — different playstyle)
- **Cross-links:** none

NOTE — Vínculo Elemental (2300): cast with weapon in hand, no movement/combat casting penalty, prevents disarm, 3-slot spiritual blade pool — matches "uno com sua arma, conjura sem penalidades, barreira a cada golpe" (ElementalBond.inc:1-19; the "barreira a cada golpe recebido" is plausibly the spiritual-blade/elemental-weapon system — UNVERIFIED in detail but not contradicted). Fluência (2301): drains target elemental resist, grants it to self, guaranteed next hit — matches. Domínio Vital (2302): pull-if-far / push-if-adjacent + passive enchant-object bonus — matches "bônus em magias + arremessa/traz alvo"; book's "bônus em todas as magias de mago de batalha" maps to the enchant-object boost (acceptable abstraction). Vórtice Prismático (2304): 10s accumulate-then-AoE-explode by weapon element — matches. Costs all stamina, matching cfg.

---

## 08-magus

Code: `classes/magus/*`, cfg hab 2200-2204. **No mismatches.**

- Determinação Arcana (2200): passive, spend stamina when out of mana — matches (the code also bypasses reagents, an unmentioned bonus, not a false claim). ArcaneDetermination.inc:1-18.
- Salto Etéreo (2201): teleport within ARCANA/5, LOS/non-water/walkable — matches. EtherealJump.inc. Cost stam 15 / 240s = cfg.
- Evocator (2202): passive, Evocatium spells harder to resist (+ range/damage/duration) — matches "mais difíceis de resistir". Evocator.inc:1-23.
- Prisão Dimensional (2203): banish a target to a personal prison then return them — matches "enviar para sua prisão pessoal e libertá-lo". Cost stam 25 / 600s = cfg.
- Abrir Portais (2204): mark up to ARCANA/33 spots, portal lasts ARCANA/10s — matches "quantidade de locais e duração escalam com seu poder". Cost stam 35 / 300s = cfg.

---

## 09-necromante

Code: `classes/necromante/*`, cfg hab 2000-2004. **No mismatches.**

- Sacrifício da Carne (2000): self-bleed, restore mana (ARCANA*2), self-damage; passive — Kaijin spells 1.5x when bloodied — matches "recupera mana ao se infligir dor; magias mais poderosas quando gravemente ferido". FleshSacrifice.inc:8-40. Cost stam 15 / 120s = cfg.
- Infligir Medo (2001): applies `temor` (≈ARCANA/1.5), Aura Divina / low-entropy immune — matches "muito mais suscetível a maldições / temor incontrolável". InflictFear.inc:11-18. Cost stam 25 / 300s = cfg.
- Cetro da Morte (2002): passive, Kaijin curses harder to resist — matches. DeathScepter.inc:1-16.
- Controlar Mortos (2003): target undead, control 120 + 1.8*ARCANA s — matches. ControlUndead.inc:11-17. Cost stam 10 / 10s = cfg.
- Lich (2004): phylactery item, death protection while charged — matches "não perde DP ao desmaiar"; the book's "recarregado periodicamente" aligns with cfg's "reenergizado a cada 8h". Lich.inc:11-19. Cost stam 35 / 900s = cfg.

---

## 10-xama

Code: `classes/shaman/*`, cfg hab 2600-2604.

### Sede de Sangue — active is a one-shot burst heal, not per-hit lifesteal
- **Handbook claim:** "Ativamente, recupera vida a cada golpe desferido por um período baseado nas almas coletadas." — src/50-caminho-sabedoria/10-xama.md:68
- **Codebase truth:** `BLOODTHIRST` (2601) `ExecuteOn` does an immediate single heal = `shaman_souls * (Theology/25)`, consumes all souls, sets a `bloodlust` cooldown for `shaman_souls` seconds — but `bloodlust` is referenced nowhere else in the codebase (grep across pkg returns only this file), so there is no per-hit lifesteal window. — pkg/systems/abilities/include/classes/shaman/Bloodthirst.inc:26-46
- **Mismatch:** The described active ("life-on-hit for a duration scaled by souls") does not exist; the real active is a burst heal proportional to stored souls. The passive ("recupera HP a cada inimigo abatido") IS real (npcdeath hook, Bloodthirst.inc:8-9).
- **Severity:** HIGH (sustain expectation in extended fights is wrong — it's burst, not lifesteal)
- **Cross-links:** none

NOTE — Colheita de Almas (2600): passive soul storage on NPC death, cast with weapon in hand, animals/undead have no soul — matches (SoulHarvest.inc + death.inc ShamanSoulHarvest; SOUL_HARVEST in cast_spell.src:154 grants armed casting). Irmãos de Areia (2602): allies guaranteed next crit, enemies `apodrecer` for (Theology/10)*souls — matches "aliados crítico garantido / oponentes mais dano". Ancestralidade Elemental (2603): channeled storm, AoE ally heal, souls→lightning on enemies — matches; cancels on movement (unmentioned, not false). Arena de Sangue (2604): paralyze + `apodrecer` (defense penalty) + poison damage scaled by souls — matches "atordoa, dano de veneno, terror, penalidade de defesa". All Shaman costs (stam+mana+delay) match cfg.

---

## Summary

| Severity | Count |
|----------|-------|
| HIGH     | 8     |
| MED      | 6     |
| LOW      | 0 (noted inline, not flagged) |
| INVENTED | 1     |
| **Total flagged** | **15** |

### Top 5 by impact

1. **Construtor de Golems "Gancho de Agarrar" (L10)** — invents a non-existent ability; the real L10 "Ombreira Tática" (handcannon/harpoon/powder-trap) is entirely missing from the book. (HIGH) — 03:73-81
2. **Arqueiro Arcano "Tempestade Elemental" (L20)** — invented capstone name + AoE-storm effects; real L20 is "Disparo Prismatico" (3-shot combo). (HIGH) — 01:100-110
3. **Arqueiro Arcano "Disparo Canalizado" (L10)** — described effects belong to the L20 ability; real L10 is a line-pierce + burn/slow/conceal. (HIGH) — 01:82-85
4. **Druida "Coração da Matilha"** — described as a summon/pet buff (longer summons, resist bonus, blessed champion animal); real ability is a self wildshape/spell-duration passive that never touches summons. (HIGH) — 05:90
5. **Granadeiro "Infusão Química"** — book states Ativa / 15 mana / 20s cd / "modify a potion"; code is a no-cost passive menu-tab unlock. Every stat is wrong. (HIGH) — 06:57-64

Also HIGH: Lâmina Arcana "Armadura Arcana" Raio = self move-speed (false; code slows attacker); Shaman "Sede de Sangue" active = burst heal not per-hit lifesteal; Cleric/Arqueiro stamina-vs-mana (MED) and the INVENTED Valdruk "forma borrada" clause.
