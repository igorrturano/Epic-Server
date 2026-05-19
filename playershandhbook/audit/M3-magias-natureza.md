# M3 — Magias de Natureza e Primordiais — Handbook ↔ Codebase Audit

Domain files audited:
- `src/80-magias/50-naturais-intro.md` (divider page, no spell content)
- `src/80-magias/51-druida.md`
- `src/80-magias/52-xama.md`
- `src/80-magias/60-primordiais.md` (Evocatium school verified here; Kaijin =
  necromancer, Velkyn = illusionist, Ahtalelóreth = swordmage belong to other
  domains and are only spot-checked / cross-noted)

Config → script map: `pkg/systems/spells/config/druid.cfg`,
`config/shaman.cfg`, `config/evocation.cfg`. Spell logic under
`pkg/systems/spells/spell_logic/{druid,shaman,evocation,mage}/`.

Scope note: the `50-naturais-intro.md` file is a pure cover/divider page
(image + logo + page break). No mechanical claims; nothing to verify.

---

## 51-druida.md

### Crescimento Selvagem — "dano contínuo" overstated
- **Handbook claim:** "Videiras espinhosas surgem do chão prendendo todos os
  inimigos em uma área, paralisando-os e causando dano contínuo." —
  src/80-magias/51-druida.md:250 (also config Desc "causando danos contínuos
  durante sua duração", druid.cfg:660)
- **Codebase truth:** `naturezaselvagem.src:29-43` only applies a `frozen := 1`
  root (`opts.+frozen := 1`, buff_category "movement_root") for the duration —
  **no damage during the root**. After the root fully expires
  (`sleep(duration)` at :46), it starts a single `"bleed"` TS timer
  (`naturezaselvagem.src:47-51`). Damage is a post-effect bleed that begins
  *when the vines release*, not continuous damage *while* rooted.
- **Mismatch:** Book says it paralyzes AND damages simultaneously
  ("paralisando-os e causando dano contínuo"). Reality: paralyze first, then a
  separate bleed only after release. The "contínuo durante a duração" framing
  is wrong — there is zero damage during the displayed duration.
- **Severity:** MED (misleads on the spell's combat profile; players expect a
  damage-over-time prison, get a CC-then-bleed sequence)
- **Cross-links:** none

### Corrente Viva — single-target wording vs chain reality
- **Handbook claim:** "Cria um raio que se propaga entre vários alvos."
  Alvo: Auto — src/80-magias/51-druida.md:222-226
- **Codebase truth:** `chainlightning.src` (druid) iterates
  `listAllMobilesNearbyButPetsAndParty(caster, range, targ)` and damages each
  (`druid/chainlightning.src:35-47`). It is genuinely multi-target. Config
  `Target Self` (druid.cfg:631).
- **Mismatch:** None on mechanics — "propaga entre vários alvos" matches the
  chain. The "Alvo: Auto" is consistent with config `Target Self`. No
  mismatch.
- **Severity:** — (verified consistent, listed for completeness)
- **Cross-links:** Xamã has no equivalent; Evocation "Corrente de Raios"
  (mage/chainlightning) has a 20% per-jump falloff the druid version lacks —
  not a handbook claim, informational only.

### Reviver Animais — pet-only restriction omitted
- **Handbook claim:** "Revive um animal morto." — src/80-magias/51-druida.md:236
- **Codebase truth:** `ressanimal.src:33-51` hard-requires the corpse to have
  an `owner` ObjProperty and that owner be the caster (or no living owner):
  "só pode ressuscitar pets domesticados", and refuses other players' pets.
  Wild/unowned animal corpses → "A natureza já reclamou este ser." Also a
  `#resurrected` cooldown gate (:54).
- **Mismatch:** Book implies any dead animal; code only revives the caster's
  own tamed pet. Significant restriction omitted.
- **Severity:** MED (a Druida picking this spell expecting battlefield animal
  revival will find it only works on their own pet)
- **Cross-links:** none

### Manto da Casca de Carvalho — verified (resist buff, not flat armor)
- **Handbook claim:** "A pele do alvo endurece como casca de árvore,
  aumentando sua defesa." — src/80-magias/51-druida.md:103
- **Codebase truth:** `barkskin.src:55-64` applies slash/piercing/blunt
  `_resist` (= % into `#SpellResist`), scaled `Theology/3` (max ~33% at GM),
  +10% if target wears no armor (`HasArmorProtection`, :75-94), +5% & +20%
  duration if caster has `CORACAO_DA_MATILHA`. `buff_category
  "armor_protection"` (replaces escudodefe). Maintainable.
- **Mismatch:** None — "aumenta sua defesa" is accurate (physical resist).
  Abstracted numbers are fine per format rules. Verified.
- **Severity:** — (verified)
- **Cross-links:** none

### Enraizar (Druida C2) / Enxame / Pira Purificadora / Renascimento / Cura
  Leve / Pomo da Vitalidade / Despetrificar / Tempestade de Espinhos /
  Encantar Pessoa / Ventania Selvagem / Caminhada do Vento / Tremor dos
  Antigos / Saltar — verified, mechanics match descriptions
- Enraizar: `enraizar.src` applies `frozen := 1` to mobiles in radius 2 →
  "paralisando-o" matches.
- Enxame: `enxame.src` tick DoT (`enxameTick`), Theology-scaled
  damage/duration → "dano por tempo" matches.
- Pira Purificadora: `pirapurificadora.src` creates a fire-field cone with
  area tick damage → "barreira de fogo" matches (book Alvo "Único", config
  `Target Area` — book wording loose but not a mechanical mismatch).
- Renascimento da Terra: `renascimentodaterra.src` builds a rising stone wall
  → "Conjura uma parede de pedra" matches.
- Cura Leve: `druidheal.src` instant heal + 4-tick HoT (Theology/3) → matches.
- Pomo da Vitalidade: `goodberry.src` converts a food item into a healing
  fruit (HoT on eat) → "fruta mágica que cura ao longo do tempo" matches.
- Despetrificar: `despetrificar.src` removes `#petrified` via
  `TS_LowerDuration(targ,"petrify",-1)` → matches.
- Tempestade de Espinhos: `tempestadeespinhos.src` single-target physical
  damage → matches.
- Encantar Pessoa: `encantarpessoa.src` sets `charmed_cannot_attack`, WILL
  resist, humanoid-only → "impedindo-a de atacar o druida" matches.
- Ventania Selvagem (`rajadavento.src` druid): `dex_mod := -20` + pushback →
  "dificulta o movimento" matches.
- Caminhada do Vento (`haste.src` druid): `speed_boost` → matches.
- Tremor dos Antigos (`earthquake.src` druid): AoE `DMG_FORCED` → matches.
- Saltar (`jump.src`): jump/leap movement → matches.
- **Severity:** — (all verified, no mismatch)

### Canção das Águas Cristalinas — narrower than "remove contaminação"
- **Handbook claim:** "Remove a contaminação da água e dos alimentos." —
  src/80-magias/51-druida.md:8
- **Codebase truth:** `purificaralimentos.src:39-44` only erases the
  `poisonInfo` ObjProperty on a targeted item. No water-specific logic; it
  works on any item that carries `poisonInfo`.
- **Mismatch:** Minor — "água e alimentos" is flavor; mechanically it removes
  item poison. Not a real mechanical contradiction (poisoned water/food carry
  `poisonInfo`). LOW.
- **Severity:** LOW (imprecise but functionally consistent)
- **Cross-links:** none

---

## 52-xama.md

### Sugar Alma — drains mana AND deals nothing to HP-as-damage; lifesteal-style
- **Handbook claim:** "Você converte a mana do alvo em HP para o xamã."
  Alvo: Único — src/80-magias/52-xama.md:217-221
- **Codebase truth:** `sugaralma.src:32-59` is a 10-second channel: each
  second it `ConsumeMana`/`DamageMana(targ, amt)` and `ApplyEpicHealing(who,
  amt)` where `amt = Theology/10`. Channel breaks on caster move/freeze/range.
- **Mismatch:** None — "converte mana do alvo em HP para o xamã" accurately
  describes the per-second mana-to-heal channel. Verified. (Note: it is a
  sustained channel, not instant — book doesn't claim instant, so OK.)
- **Severity:** — (verified)
- **Cross-links:** none

### Rito de Sangue — HP threshold is NOT "seu Theology"
- **Handbook claim:** "Todas as criaturas ao redor com HP abaixo do seu
  *Theology* explodem e concedem uma alma extra ao xamã." —
  src/80-magias/52-xama.md:232
- **Codebase truth:** `bloodbath.src:21-23`: `hp_threshold := (theology / 2) +
  (shaman_souls * 2)` — i.e. **half** Theology plus 2 per stored
  `shaman_souls`. Players additionally need `DP < 10` to be eligible
  (`bloodbath.src:36-39`). Each kill grants `kills` souls; message says
  `kills * 2` ("colheu N almas") (`bloodbath.src:124-140`).
- **Mismatch:** Book says threshold = full Theology. Reality = Theology/2 +
  soul-stack bonus, and players also need very low DP. The "uma alma extra"
  per creature is roughly right (kills→souls) but the gating value is wrong.
- **Severity:** MED (overstates the kill window by ~2x and omits the
  player-only DP<10 gate)
- **Cross-links:** config Desc also says "HP < Theology"
  (shaman.cfg:591) — same wrong claim, in code-adjacent config (informational,
  not a src/ file).

### Encantar Monstros — scales on Arcana, not Theology; monsters only
- **Handbook claim:** "O monstro escolhido não ataca você por um tempo." —
  src/80-magias/52-xama.md:152
- **Codebase truth:** `encantarmonstro.src:46` duration `= AP_GetSkill(who,
  ARCANA)/5` (not Theology like the rest of the Xamã kit), `isMonsterBrain()`
  gate (:32), WILL resist + true-boss immunity (:40), +50% duration with
  `SAND_BROTHERS` ability. Sets `charmed_cannot_attack`.
- **Mismatch:** Mechanic ("não ataca você") matches. The scaling source being
  ARCANA while every other Xamã spell scales on THEOLOGY is a code
  inconsistency, but the book abstracts numbers so this is not a handbook
  mismatch per format rules. No book-level mismatch.
- **Severity:** — (verified consistent with book text; ARCANA scaling flagged
  for the dev team, not a handbook defect)
- **Cross-links:** Druida "Encantar Pessoa" (`encantarpessoa.src:48`) uses
  Theology/5 and is humanoid-only — parallel spell, different skill+target
  gate; both books describe only "impede ataque", both accurate.

### Despertar — corpse-only, does not "wake from a faint"
- **Handbook claim:** "Acorda o alvo de um desmaio." Alvo: Único —
  src/80-magias/52-xama.md:32-35
- **Codebase truth:** `despertar.src:14-28` requires
  `targ.isa(POLCLASS_CORPSE)`; on a non-corpse it fails
  ("Isto nao pode ser alvejado"). For a player corpse it sets
  `curado := 1`. It operates on the *downed/dead body*, not a conscious
  fainted mobile.
- **Mismatch:** "Acorda o alvo de um desmaio" reads as targeting a living
  fainted creature; code only accepts a CORPSE object (the UO representation
  of a downed player). Targeting wording is misleading — players will try to
  cast it on the standing/fainted mobile and get a failure.
- **Severity:** MED (target-type confusion; functional intent — revive a
  downed ally — is right but the "alvo" the player must click is the corpse)
- **Cross-links:** none

### Sacrificar Animal — requires altar kill, not instant blessing
- **Handbook claim:** "Você recebe uma bênção e proteção contra a morte ao
  sacrificar um animal." — src/80-magias/52-xama.md:138
- **Codebase truth:** `sacrificaranimal.src` only *marks* the animal
  (`animal_sacrifice`/`aklat_sacrifice` ObjProperty) and tells the player:
  "O animal foi marcado. Mate-o perto de um altar xamanico para receber a
  proteção contra a morte." The blessing is not granted by this spell — it is
  deferred to killing the marked animal near a shaman altar.
- **Mismatch:** Book implies casting the spell grants the blessing/death
  protection directly. Reality: it only tags the target; the reward requires
  a separate altar-kill step the book never mentions.
- **Severity:** HIGH (a player relying on this for death protection mid-fight
  gets nothing; the actual ritual is multi-step and altar-bound)
- **Cross-links:** none

### Lama — verified (slow terrain)
- **Handbook claim:** "Você cria um terreno que deixa lentos todos que são
  afetados." — src/80-magias/52-xama.md:58
- **Codebase truth:** `grease.src:28-35` spawns immovable "lama" items and
  starts a `"fraturaexposta"` TS timer on each mobile in radius 3. (Note: the
  timer name is `fraturaexposta`, a slow/snare timer per the file's header
  comment.)
- **Mismatch:** None on the described effect (area slow). Verified.
- **Severity:** — (verified)
- **Cross-links:** none

### Outras Xamã verificadas (sem mismatch)
- Cura Leve (`shaman/druidheal.src`) — instant + HoT, matches.
- Enfraquecer (`enfraquecer.src`) — arms next melee hit to apply STR debuff;
  book "deixa o alvo fisicamente mais fraco" / Alvo Auto matches the
  self-buff-then-hit design.
- Força do Touro (`mage/forca`) — STR buff, matches "aumenta a força".
- Nutrir o Corpo (`Ent2.src`) — spawns food items (24h cooldown); book
  "cria alimentos capazes de recuperar seu HP" matches (the HP comes from
  eating, consistent).
- Proteção Contra a Morte (`protecaomorte.src`) — sets `protecaomorte`
  cooldown on a connected player; "não corre risco de morrer caso desmaie"
  matches intent.
- Intimidar (`pacifismo.src`) — `hitchance_mod` negative (Theology/5); book
  "passa a causar menos dano" — code reduces *hit chance*, not damage. See
  next finding.
- Rajada de Vento (`shaman/rajadavento.src`) — `dex_mod := -20` + pushback,
  matches "dificulta o movimento".
- Frenesi (`GrizzlyBear.src`) — `"berserk"` TS timer, matches "estado de
  fúria".
- Montaria das Areias (`invocarmontariadeserto.src`) — summons
  `:epicai:desertostard` mount, matches "invoca uma montaria".
- Terremoto (`shaman/earthquake.src`) — AoE `DMG_FORCED`, matches.
- Trovão (`trovao.src`) — single-target `DMG_ENERGY` (Theology + 4d8),
  matches "poderoso trovão em seu alvo".
- Círculo de Pedras (`circulopedras.src`) — raises a ring of stone items,
  matches.
- Observar (`telepatia.src`) — reports STR/DEX/INT + opens backpack view;
  book "informações precisas sobre o alvo" matches.
- Enraizar (Xamã, `shaman/enraizar.src`) — `frozen := 1`, matches
  "imobilizando-o".

### Intimidar — "menos dano" vs hit-chance penalty
- **Handbook claim:** "O alvo passa a causar menos dano no combate." —
  src/80-magias/52-xama.md:165 (config Desc identical, shaman.cfg:442)
- **Codebase truth:** `pacifismo.src:23-32` applies `opts.+hitchance_mod :=
  -(Theology/5)` (min -1) for 60s. This reduces the target's **chance to
  land hits**, not the damage of hits that do land.
- **Mismatch:** Book says damage is reduced; code reduces accuracy. A target
  that still connects deals full damage. Mechanically different lever
  (whiff-rate vs damage-per-hit).
- **Severity:** MED (a player picking Intimidar to blunt a hard-hitting boss
  expects damage reduction, gets a miss-chance debuff that does nothing on a
  hit that lands)
- **Cross-links:** none

---

## 60-primordiais.md (Evocatium school)

### Mísseis Mágicos — "cada dardo atinge uma criatura de sua escolha" is false
- **Handbook claim:** "Você cria três dardos brilhantes de força mágica. Cada
  dardo atinge uma criatura de sua escolha que você possa ver dentro do
  alcance." Alvo: Único — src/80-magias/60-primordiais.md:24-27
- **Codebase truth:** `mage/magicmissile.src:43-60` fires `mlore` darts
  (`Arcana/25`, min 1, +1 with `EVOCATOR`) — count is **not fixed at 3**, it
  scales with Arcana — and **all darts hit the single chosen `targ`** (loop
  reuses the same `targ`). Only the last dart rolls full circle damage; the
  rest are flat `1d4+1`.
- **Mismatch:** Two claims contradicted: (1) "três dardos" — count is
  Arcana-scaled, not always 3; (2) "cada dardo atinge uma criatura de sua
  escolha" — there is no per-dart target pick; every dart strikes one
  single target (consistent with the book's own "Alvo: Único", which
  contradicts the prose).
- **Severity:** MED (the multi-target prose is the classic D&D Magic Missile
  expectation; this implementation is single-target only — misleads tactical
  use)
- **Cross-links:** none

### Chuva de Meteoros / Chuva de Gelo — "Alvo: Único" but AoE in code
- **Handbook claim:** Both list **Alvo: Único** —
  src/80-magias/60-primordiais.md:165-169 (Chuva de Gelo),
  :177-181 (Chuva de Meteoros)
- **Codebase truth:** `mage/meteorswarm.src` and `mage/icestorm.src` both call
  `listAllMobilesNearbyButPetsAndParty(caster, radius, targ)` and damage every
  mobile in an Arcana-scaled radius, plus `DanificaObjetosProximos` area
  object damage. These are area spells. (config also says `Target Unico`,
  evocation.cfg:275/321 — the config is wrong too.)
- **Mismatch:** Book (and config) say single-target; both are radius AoE
  nukes.
- **Severity:** MED (a Mago choosing between single-target and AoE for the C5
  slot is misled — both are AoE)
- **Cross-links:** Druida "Tremor dos Antigos" / Xamã "Terremoto" correctly
  imply area; only these two Evocation C5 entries mislabel target.

### Corrente de Raios (Evocation C4) — verified chain, label OK
- **Handbook claim:** "Cria um raio que se propaga entre vários alvos." Alvo:
  Único — src/80-magias/60-primordiais.md:127-131
- **Codebase truth:** `mage/chainlightning.src:42-60` chains through nearby
  mobiles with a 20% per-jump damage falloff. Multi-target by design;
  "vários alvos" is accurate. "Alvo: Único" = initial target pick, consistent
  enough.
- **Mismatch:** None mechanically. Verified.
- **Severity:** — (verified)
- **Cross-links:** Druida "Corrente Viva" same engine without falloff.

### Velocidade / Protecao Arcana / Teleportar / Bola de Fogo / Incinerar /
  Congelar / Relâmpago / Mãos Flamejantes / Cone de Gelo (Repulsão) /
  Singularidade — verified
- Velocidade (`evocation/haste.src`) — `speed_boost`, matches.
- Proteção Arcana (`evocation/manashield.src`) — diverts damage to mana at
  2:1 (1:1 if Arcana primary); book "Dano recebido é desviado para a mana
  (2:1)" matches exactly.
- Teleportar (`mage/teleport.src`) — short teleport, Arcana/10 range cap;
  matches "transporta você a um local à vista".
- Bola de Fogo (`mage/fireball.src`) — AoE fire explosion; book Alvo Área
  matches.
- Incinerar (`mage/ignite.src`) — impact + WILL-gated burn DoT; matches
  "incinera seu alvo".
- Congelar (`mage/frostbolt.src`) — token cold dmg + WILL save: fail = hard
  freeze, save = slow; matches config Desc.
- Relâmpago (`mage/lightningbolt.src`) — single-target energy nuke, failed
  REFLEX = ×2 crit; matches config Desc (book prose "eletrocutando tudo em
  seu caminho" is loose flavor, single-target in code — LOW, see below).
- Mãos Flamejantes (`mage/burninghands.src`) — cone fire DoT; matches.
- Singularidade / Repulsão Estática (`evocation/`) — pull/knockback + energy
  dmg with save; match config Desc. (Note: `singularidade.src` ships with
  live `print("[DEBUG singularidade]...")` console spam, :68-110 — not a
  handbook issue, flagged for cleanup.)
- **Severity:** — (verified)

### Relâmpago — prose "eletrocutando tudo em seu caminho" vs single-target
- **Handbook claim:** "Dispara um raio poderoso, eletrocutando tudo em seu
  caminho." Alvo: Único — src/80-magias/60-primordiais.md:97-101
- **Codebase truth:** `mage/lightningbolt.src:44-57` is strictly
  single-target (one `targ`, ×2 on failed REFLEX). Nothing "in the path" is
  hit.
- **Mismatch:** Prose implies a piercing line AoE; it is single-target (book's
  own "Alvo: Único" is correct, prose oversells).
- **Severity:** LOW (flavor overreach; the structured Alvo field is correct)
- **Cross-links:** none

### Kaijin / Velkyn / Ahtalelóreth — out of this domain
The Primordiais file also documents Kaijin (necromancer —
`spell_logic/necromancer/`), Velkyn (illusionist —
`spell_logic/illusionist/`) and Ahtalelóreth (swordmage —
`spell_logic/swordmage/`). These belong to the necromancy / illusion /
swordmage audit domains and are NOT verified here to avoid double-coverage.
Flagged so the M-series owner can route them.

---

## Summary

| Severity | Count |
|---|---|
| HIGH | 1 |
| MED | 7 |
| LOW | 2 |
| INVENTED | 0 |
| Verified (no mismatch) | many (listed inline) |

### Top 5 by impact

1. **HIGH — Sacrificar Animal** (xama.md:138): book says the spell grants the
   blessing/death protection; code only *marks* the animal — the reward
   requires a separate altar-kill step never mentioned. Players relying on it
   mid-fight get nothing.
2. **MED — Mísseis Mágicos** (primordiais.md:24): "três dardos, cada um numa
   criatura de sua escolha" is false — dart count is Arcana-scaled and all
   darts hit one single target.
3. **MED — Rito de Sangue** (xama.md:232): HP threshold is Theology/2 +
   soul-stack (not full Theology) and players also need DP<10; book overstates
   the kill window ~2x and omits the DP gate.
4. **MED — Reviver Animais** (druida.md:236): only revives the *caster's own
   tamed pet*, not any dead animal; major restriction omitted.
5. **MED — Despertar** (xama.md:32): target must be a CORPSE object, not a
   standing fainted creature — target-type confusion that breaks casting in
   practice.

Other MED: Crescimento Selvagem (CC-then-bleed, not continuous damage),
Intimidar (hit-chance debuff sold as damage reduction), Chuva de
Meteoros/Gelo (labeled Alvo: Único, actually AoE). LOW: Canção das Águas
Cristalinas (water/food flavor vs generic poison-strip), Relâmpago prose
("tudo em seu caminho" vs single-target).
