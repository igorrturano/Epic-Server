# M1 — Magias Base (intro / truques / cânticos Bardo / Ninjutsu)

Audit of `playershandhbook/src/80-magias/00-intro.md`, `10-truques.md`,
`20-canticos-bardo.md`, `30-ninjutsu.md` against the live code in
`pkg/systems/spells/`.

Method: each entry cross-checked against its config block
(`pkg/systems/spells/config/*.cfg` — source of truth for circle/mana/type/
power words) and its `SpellScript` (`spell_logic/`), plus the SC framework
and `spell_executor.inc` / `cast_spell.src` for resource & gating behavior.
The book intentionally abstracts numbers; only wrong mechanics, wrong
scaling/source, invented effects, missing prerequisites, and
non-existent/mis-triggered abilities are flagged.

---

## 00-intro.md

No mechanical mismatches. The school list (Evocatium, Kaijin, Velkyn,
Aradalore, Ahtalelóreth) and alternative sources (Xamanismo, Divinismo)
are lore framing that matches the `Type`/`School` strings used across the
configs (`Evocatium`, `Kaijin`/`KAIJIN`, `Velkyn`, `Aradalore`,
`Ahtaleloreth`, `Divine`, shaman line). The conjuration-element list
(Dificuldade, Custo, Círculo, Alvo, Alcance, Palavra, Ciclos) maps to real
config keys (`Difficulty`, `Mana`, `Circle`, `Target`, `Range`,
`PowerWords`, `Cycles`). Flavor only — nothing to flag.

---

## 10-truques.md

### "Truque" cost stated once, but each caster class has its own spell at a different mana
- **Handbook claim:** Common truques listed at "**Custo:** 10 mana"
  (Criar Comida etc.) with class footnotes like "*Xamãs conjuram este
  truque com a palavra de ativação **Ymuk***" — src/80-magias/10-truques.md:13,15
- **Codebase truth:** Cantrips exist as separate per-class spells.
  `cantrips.cfg` (mage path) Criar Comida = `Mana 10`
  (pkg/systems/spells/config/cantrips.cfg:29); the Xamã equivalent is a
  *different* spell, `shaman.cfg` SpellId 600 Criar comida =
  `Mana 5`, `PowerWords Ymuk`
  (pkg/systems/spells/config/shaman.cfg:48-63). Every shaman cantrip
  (601 Dardo Ígneo, 602 Levitar, 603 Manter Fogueira, 604 Raio de Gelo,
  605 Visão Noturna, 606 Voz Fantasma) is `Mana 5`, half the listed cost.
- **Mismatch:** The book presents one cost per truque, but the Xamã
  versions cost ~half. Power words themselves are correct (Ymuk/Khor/Kald/
  Surdk/Don/Ghor/Ehk all verified in shaman.cfg). This is the book's
  deliberate one-line-per-truque abstraction over class-specific spell IDs,
  so the *power words and effect* are right; only the single cost number
  is class-dependent.
- **Severity:** LOW (abstracted number; mechanics & power words correct)
- **Cross-links:** Druida footnote costs (`5 mana` lines 134,148,...) are
  consistent with `druid.cfg` 700-708 = `Mana 5`; no separate flag needed.

### "Manter Fogueira" common cost (15) vs Druida/Xamã (5)
- **Handbook claim:** "Manter Fogueira ... **Custo:** 15 mana" —
  src/80-magias/10-truques.md:38
- **Codebase truth:** `cantrips.cfg` SpellId 5 Manter Fogueira =
  `Mana 15` (pkg/systems/spells/config/cantrips.cfg:115). Druida 704 =
  `Mana 5` (druid.cfg:149), Xamã 603 = `Mana 5` (shaman.cfg:124).
- **Mismatch:** Same class-specific-cost abstraction as above; the generic
  number quoted (15) is the mage value, not universal.
- **Severity:** LOW
- **Cross-links:** Same root cause as the previous finding.

### "Voar" cost: book says 5 mana; Paladino version is 0
- **Handbook claim:** "Voar ... *Exclusivo: Paladino, Caçador de
  Demônios* ... **Custo:** 5 mana" — src/80-magias/10-truques.md:362-371
- **Codebase truth:** Caçador de Demônios spell 901 Voar = `Mana 5`
  (pkg/systems/spells/config/demonhunter.cfg:36-52). Paladino spell 801
  Voar = `Mana 0` (pkg/systems/spells/config/paladin.cfg:40-55). Both
  `Difficulty 80` — matches the book's stated "Dificuldade: 80".
- **Mismatch:** Cost 5 is correct for Caçador de Demônios but the Paladino
  version is free (0 mana). One number can't be right for both exclusive
  classes.
- **Severity:** LOW (free is strictly better for the player; not a
  build-misleading trap)
- **Cross-links:** none

### "Despertar" (Clérigo truque) targets a corpse, not a fainted mobile
- **Handbook claim:** "Despertar — *Exclusivo: Clérigo* — Recupera o alvo
  de um desmaio. **Alvo:** Único" — src/80-magias/10-truques.md:166-175
- **Codebase truth:** `priest.cfg` 501 Despertar `Target Unico`
  (pkg/systems/spells/config/priest.cfg:67-86); the script only accepts a
  corpse: `if (!targ.isa(POLCLASS_CORPSE)) ... return;`
  (pkg/systems/spells/spell_logic/priest/despertar.src:14-17). A fainted
  ("desmaiado") character in this engine is represented by a corpse with
  no `morto` prop, so targeting works — but only if you click the corpse,
  not the standing/fallen mobile.
- **Mismatch:** "Alvo: Único" + "recupera o alvo de um desmaio" implies you
  target the person; in practice you must target their corpse object. This
  is an interaction nuance the book glosses, not a wrong effect.
- **Severity:** LOW (wording imprecision; effect is real)
- **Cross-links:** Ninjutsu "Despertar" (30-ninjutsu.md:61) is the same
  mechanic via `iryo_ninjutsu.src` — same corpse-target nuance. See M1
  Ninjutsu section.

### "Orientar" is a sustained hit buff, not strictly "o próximo ataque"
- **Handbook claim:** "Orientar — concede uma bênção divina que guia o
  **próximo ataque** do alvo." — src/80-magias/10-truques.md:294-296
- **Codebase truth:** `orientar.src` applies `opts.hitchance_mod := bonus`
  (THEOLOGY/20, min 1) for `duration = THEOLOGY/2` seconds (min 15s),
  `buff_category "hit_bonus"`, `maintainable`
  (pkg/systems/spells/spell_logic/priest/orientar.src:29-46). It is a
  durational accuracy buff via SC, not a one-shot "next attack" charge.
- **Mismatch:** "guia o próximo ataque" reads as a single-swing buff; it is
  actually a timed +hit buff lasting ≥15s and renewable (maintain).
- **Severity:** MED (a player relying on "one attack then gone" will
  misjudge its value; it is substantially stronger than described)
- **Cross-links:** none

### Truques the book does not mark exclusive but that have only class-specific implementations
- **Handbook claim:** "Globo de Luz" is listed under **Truques Comuns**
  with no class footnote — src/80-magias/10-truques.md:87-94
- **Codebase truth:** It exists as `cantrips.cfg` SpellId 3 Globo de luz
  (pkg/systems/spells/config/cantrips.cfg:57-78, script
  `mage/lightglobe`). It is a real common cantrip — summons a
  `:epicai:globodeluz` follower for `60 + skill*3`s
  (pkg/systems/spells/spell_logic/mage/lightglobe.src:14-27). The Druida
  equivalent "Invocar Wisp" (702) and Clérigo "Luz Divina" (500) reuse the
  same script. No mismatch — entry verified.
- **Mismatch:** none (verification note only).
- **Severity:** —
- **Cross-links:** Druida "Invocar Wisp" line 223, Clérigo/Paladino "Luz
  Divina" line 250 — same `lightglobe`/`luzdivina` summon mechanic; the
  book's three separate descriptions are consistent with code.

UNVERIFIED in 10-truques.md: "Disparo Místico" teleport-after-attack chain
(src:182-185), "Mãos Mágicas" ranged pickpocket (src:264-267), "Lâmina
Estrondosa" bonus magic damage (src:237-240), "Abrir Fechaduras Mágicas"
(src:113-122). These are Malandro Arcano `trickster.cfg` 1000/1001 +
ability-side scripts; trickster.cfg only defines 1000/1001/1002 and the
detailed combat behavior lives in the abilities layer (out of this
domain's spell_logic scope). Effects could not be confirmed from the spell
configs alone — flagged UNVERIFIED, not asserted.

---

## 20-canticos-bardo.md

General note (verified, not a mismatch): cânticos route through
`SPELL_TYPE_BARD` in `spell_executor.inc` and require
`ABILITY_ID::VOX_ARCANA` (pkg/systems/spells/include/spell_executor.inc:
265-268). All 16 cânticos exist in `bardo.cfg` with the circles/mana the
book states (C0 1200-1202, C1 1203-1205, C2 1206-1209, C3 1210-1212,
C4 1213-1215). Circle/mana/Type all match. The book's "apenas a voz, o
instrumento" is flavor — no instrument-object requirement is enforced in
the cast path (not flagged; lore).

### "Requiem do Silêncio" only silences enemy *Bards*, not all casters
- **Handbook claim:** "Requiem do Silêncio — silencia todos os inimigos em
  uma área de três tiles, **impedindo-os de conjurar**." —
  src/80-magias/20-canticos-bardo.md:122-123
- **Codebase truth:** `requiem_do_silencio.src` sets
  `#bard_silenced` on hostiles in 3 tiles
  (pkg/systems/spells/spell_logic/bardo/requiem_do_silencio.src:24-32).
  The only place that property *blocks casting* is the `SPELL_TYPE_BARD`
  branch of `ValidateSpellAccess`
  (pkg/systems/spells/include/spell_executor.inc:270-273). It is **not**
  checked for `SPELL_TYPE_MAGE`, `_PRIEST`, `_DRUID`, `_NECROMANCER`,
  `_DEMONHUNTER`, `_SHINOBI`, `_TRICKSTER`, `_SHAMAN`. Grep confirms the
  only enforcement site is that one bard-only line; everything else just
  erases the flag (dispel scripts).
- **Mismatch:** "impedindo-os de conjurar" implies a universal silence. In
  reality it only stops other Bards from singing; mages, clérigos, druidas,
  necromancers, etc. cast normally while "silenced". Against the vast
  majority of caster enemies the cântico does nothing.
- **Severity:** HIGH (a C2 control cântico the player will pick to shut
  down enemy casters does not work on enemy casters)
- **Cross-links:** none

### "O Sonho do Louco" — hallucination flag is set but never read; no inversion of ally/enemy perception
- **Handbook claim:** "O Sonho do Louco — mergulha um alvo em alucinações
  que **invertem sua percepção de aliados e inimigos**." —
  src/80-magias/20-canticos-bardo.md:110-111
- **Codebase truth:** `sonho_do_louco.src` rolls Arcana vs Will, then only
  `SetObjProperty(targ, "#bard_hallucination", 1)` for `Arc/10`s and a
  sysmessage; no targeting/AI inversion, no `Hallucinate()` call, no
  frozen/scramble (pkg/systems/spells/spell_logic/bardo/
  sonho_do_louco.src:39-48). Grep of the whole pkg shows `#bard_
  hallucination` is consumed *only* by the three `dissiparmagia.src`
  cleanup lists — no combat/AI/targeting code reads it. The real
  hallucination system (`mobiles/timedScripts/subScripts/poison/
  hallucination.src`, `tn/include/hallucination.inc::Hallucinate`) is a
  separate mechanism this cântico never invokes.
- **Mismatch:** The described effect (invert ally/enemy perception) is not
  implemented at all. The cântico costs 10 mana, can be resisted, and on
  success does nothing mechanical beyond a chat line until someone dispels
  it.
- **Severity:** HIGH (invented effect — a C2 cântico with no working
  mechanic)
- **Cross-links:** none

### "O Chamado Irresistível" — charm flag is set but no AI follows it; charmed creatures do not fight for you
- **Handbook claim:** "O Chamado Irresistível — encanta até três criaturas
  próximas, **fazendo-as lutar ao seu lado** temporariamente." —
  src/80-magias/20-canticos-bardo.md:163-164
- **Codebase truth:** `chamado_irresistivel.src` rolls Arcana vs Will on
  up to 3 hostile NPCs and sets `#bard_charmed := who.serial` for
  `Arc/3`s (pkg/systems/spells/spell_logic/bardo/
  chamado_irresistivel.src:24-39). Grep shows `#bard_charmed` is read
  nowhere except the `dissiparmagia.src` cleanup lists. The AI's only
  forced-ally/forced-target hook is `#ForcedTarget`
  (pkg/mobiles/epicAI/include/epicNPCCombat.inc:641) — a *different*
  property the cântico never sets. So a "charmed" NPC keeps its normal
  hostility; it does not switch sides or fight for the bard.
- **Mismatch:** Described pet/charm effect ("lutar ao seu lado") is not
  wired to any AI. The cântico marks creatures and does nothing else.
- **Severity:** HIGH (invented effect — C3 cântico with no working
  mechanic)
- **Cross-links:** Compare Shinobi "Distração" which *does* correctly use
  `#ForcedTarget` + `EVID_TAUNT` (distracao.src:46-50) — the working
  pattern the charm cântico fails to use.

### "Canção de Ninar" / "O Lamento do Covarde" — sleep/fear is a hard freeze, broken only by timer (not by damage)
- **Handbook claim:** "Canção de Ninar — pode adormecer um alvo caso ele
  falhe em resistir." (src:18-19); the config Desc and SC convention imply
  sleep ends on damage. "O Lamento do Covarde — pode fazer um alvo fugir
  de pavor." (src:69-70)
- **Codebase truth:** `cancao_de_ninar.src` sets `targ.frozen := 1` +
  `#bard_sleeping` and just `sleep(duration)`; it only unfreezes when the
  timer elapses — taking damage does **not** wake the target
  (pkg/systems/spells/spell_logic/bardo/cancao_de_ninar.src:39-50).
  `lamento_do_covarde.src` is identical: `targ.frozen := 1` for the
  duration with no flee/movement and no damage-break
  (pkg/systems/spells/spell_logic/bardo/lamento_do_covarde.src:37-48).
- **Mismatch:** (a) Canção de Ninar's in-code comment says "Dura ate
  receber dano" but the implementation has no damage-break — it is an
  unconditional hard CC for the full duration. (b) "O Lamento do Covarde"
  is described as *fear that makes the target flee* ("fugir de pavor");
  the implementation is a stationary freeze (target cannot move at all),
  i.e. a paralyze, not a fear/flee.
- **Severity:** MED (sleep stronger than a typical break-on-damage sleep;
  fear behaves as paralyze not flee — both mislead tactical use, but the
  CC does function)
- **Cross-links:** Same freeze pattern as the C0 sleep; one fix touches the
  shared CC approach across cancao_de_ninar / lamento_do_covarde.

### "Dança das Lâminas" — speed bonus not implemented; hit bonus uses the broken one-shot `#hit_mod` bucket
- **Handbook claim:** "Dança das Lâminas — aumenta a chance de acerto **e
  a velocidade de ataque** da party." —
  src/80-magias/20-canticos-bardo.md:151-152
- **Codebase truth:** `danca_das_laminas.src` only writes a hit bonus and
  has an explicit code comment "bonus de velocidade sera implementado
  quando o sistema de speed estiver disponivel"
  (pkg/systems/spells/spell_logic/bardo/danca_das_laminas.src:10,29-39).
  No attack-speed change is applied. Worse, the hit bonus is stored in
  `#hit_mod` (the one-shot ObjProperty consumed and erased by
  HitController on the first swing — see docs/spells-pipeline.md
  Pitfall #1),
  not the durational `opts.hitchance_mod`. So the +hit lasts exactly one
  attack despite the cântico advertising a duration, and the cleanup at
  :50-61 can corrupt an unrelated `#hit_mod` pushed by another source.
- **Mismatch:** (1) Attack-speed half of the described effect does not
  exist. (2) The accuracy half lasts one swing, not the duration the
  player is told (and the message claims `por {duration}s`).
- **Severity:** HIGH (a C3 party cântico delivering ~one of two stated
  effects, and that one for a single attack)
- **Cross-links:** Same `#hit_mod` anti-pattern flagged in
  docs/spells-pipeline.md (Pitfall #1). Any handbook fix should also stop
  promising the speed component until implemented.

### "Escudo dos Ancestrais" — buffs only 4 of the 7 resist types
- **Handbook claim:** "Escudo dos Ancestrais — invoca a proteção dos
  ancestrais, aumentando **a resistência** de toda a party." —
  src/80-magias/20-canticos-bardo.md:98-99
- **Codebase truth:** `escudo_dos_ancestrais.src` adds the bonus only to
  `fire_resist`, `cold_resist`, `energy_resist`, `poison_resist`
  (pkg/systems/spells/spell_logic/bardo/escudo_dos_ancestrais.src:30-33).
  Physical resists (slash/piercing/blunt) are untouched. It also writes
  the native `*_resist` fields directly rather than via SC's
  `#SpellResist` layer, but functionally the party gains only the four
  elemental/poison resists.
- **Mismatch:** "aumentando a resistência" reads as all-resist; the
  cântico does not protect against physical (slash/pierce/blunt) damage.
- **Severity:** MED (party-defense cântico that is dead against pure
  physical attackers; players will overvalue it)
- **Cross-links:** none

### "Crônica do Abismo" — summon lasts the stated 60s and matches "entidade do void"
- **Handbook claim:** "convoca uma entidade do void como aliada por
  sessenta segundos. Gera entropia negativa." —
  src/80-magias/20-canticos-bardo.md:180-181
- **Codebase truth:** `cronica_do_abismo.src` summons a void creature by
  tier (Arc/20) and `sleep(60)` then `npc.Kill()`
  (pkg/systems/spells/spell_logic/bardo/cronica_do_abismo.src:24-45).
  Entropy is awarded via `_AwardBardCanticoEntropy` in the executor
  (spell_executor.inc:183-203). Verified — no mismatch. Note: the summoned
  NPC is created with no faction/master binding shown here, so whether it
  actually fights *for* the bard depends on the void template's AI; the
  60s duration and entropy are correct.
- **Mismatch:** none (verification note).
- **Severity:** —
- **Cross-links:** none

Verified-correct cânticos (no mismatch): O Sussurro do Vento (private
message ≤20 tiles — sussurro_do_vento.src:17-29), A Chama que Dança
(fire DoT — chama_que_danca.src:30-39), Balada do Herói Ferido (HoT
Arc/10 ×5 — balada_heroi_ferido.src:39-48), Marcha dos Bravos (+STR party
— marcha_dos_bravos.src:28-37), A Queda do Tirano (forced dmg + −DEX —
queda_do_tirano.src:35-41), A Ascensão de Crassus (+STR/DEX/INT party —
ascensao_de_crassus.src:30-35), O Deserto de Kordum (fire burst + DoT AoE
— deserto_de_kordum.src:40-58), Sinfonia do Apocalipse (party + / enemies
− STR/DEX/INT — sinfonia_do_apocalipse.src:34-52). All match their
descriptions.

---

## 30-ninjutsu.md

### "O Ninjutsu não consome mana" — false; every technique deducts mana
- **Handbook claim:** "Diferente das magias convencionais, o Ninjutsu
  **não consome mana**, porém precisa ser intervalado..." —
  src/80-magias/30-ninjutsu.md:3
- **Codebase truth:** Every shinobi technique has a non-zero `Mana` in
  `shinobi.cfg`: Substituição/Avanço `Mana 5`, Distração/Escapar
  `Mana 10`, Tanatose/Despertar `Mana 15`, Disfarce/Troca `Mana 20`,
  Clone/Intuição `Mana 25` (pkg/systems/spells/config/shinobi.cfg:
  41,58,80,98,120,138,160,178,201,221). The `.n` command →
  `ExecuteSpellByName` → `Start_Script(":spells:cast_spell", ...)`
  (pkg/systems/spells/commands/player/n.src:50-51;
  spell_executor.inc:99-105). `cast_spell.src` calls
  `consumeResourcesForSpell` unconditionally
  (pkg/systems/spells/cast_spell.src:51) and that function has **no
  Ninjutsu exemption** — it `AP_ConsumeVital(mobile, MANA, mana_cost)`
  (cast_spell.src:235/246). The only Ninjutsu special-cases in
  `cast_spell.src` are: not blocked while frozen (line 104), no auto-self-
  freeze (line 110), and `checkCastingSuccess` auto-passes (line 554) —
  none of which touch mana.
- **Mismatch:** The book's headline rule for the entire Ninjutsu chapter
  ("não consome mana") is wrong. Techniques cost 5–25 mana exactly like
  spells; the only true distinction is the per-technique `Delay`
  (cooldown) the book calls "Intervalo".
- **Severity:** HIGH (a class-defining resource claim that is false for
  every entry in the chapter; directly misleads Shinobi build/resource
  planning)
- **Cross-links:** none

### Ninjutsu "Despertar" — corpse-target nuance (same as Clérigo Despertar)
- **Handbook claim:** "Despertar — ativa pontos de chakra de um corpo
  desmaiado ... **Alvo:** Único" — src/80-magias/30-ninjutsu.md:61-64
- **Codebase truth:** `shinobi.cfg` 1107 Despertar `Target Unico`
  (pkg/systems/spells/config/shinobi.cfg:126-142); script requires a
  corpse: `if (!targ.isa(POLCLASS_CORPSE)) ... return;`
  (pkg/systems/spells/spell_logic/shinobi/iryo_ninjutsu.src:19-22). Same
  "target the corpse object" nuance as the Clérigo truque.
- **Mismatch:** "Alvo: Único" implies targeting the person; you target
  their corpse. Effect itself (wake a fainted body) is real.
- **Severity:** LOW (wording imprecision)
- **Cross-links:** Clérigo "Despertar" — src/80-magias/10-truques.md:166;
  same mechanic, one fix should cover both.

### "Substituição" — duration is 30s, not the 20s "Intervalo" implies; effect functions
- **Handbook claim:** "Substituição — você fica imóvel e o próximo golpe
  contra você o substitui por uma tora, deixando-o invisível. ...
  **Intervalo:** 20s" — src/80-magias/30-ninjutsu.md:21-29
- **Codebase truth:** `substituicao.src` sets `#substituicao_active` and
  waits up to a hard-coded 30s window for a hit/timeout
  (pkg/systems/spells/spell_logic/shinobi/substituicao.src:23,28-49). The
  config `Delay 500` (5s) is the recast interval, not 20s; the book's
  "Intervalo: 20s" does not match `shinobi.cfg` `Delay` for any technique
  (all are 300–600 = 3–6s).
- **Mismatch:** The book lists every technique's "Intervalo" as 20s, but
  `shinobi.cfg` `Delay` values are 300/500/600/etc. (3–6s). The 20s figure
  appears on all 10 ninjutsu entries and matches none of them. (The
  substituição arm/window is a separate 30s timeout, also not 20s.)
- **Severity:** MED (uniform wrong cooldown across the whole chapter; the
  book intentionally abstracts numbers, but 20s is a concrete, repeated,
  wrong value affecting rotation planning)
- **Cross-links:** Affects all 10 ninjutsu entries
  (src/80-magias/30-ninjutsu.md:16,28,42,54,68,80,93,105,118,131).

Verified-correct ninjutsu (effect matches description; only the
mana/interval header issues above apply): Avanço (dash via PushBack —
passo_rapido.src:24-32), Distração (decoy NPC + `#ForcedTarget` taunt —
distracao.src:31-50), Escapar (clears paralysis/frozen/restringir/tombar —
escapar.src:19-40), Tanatose (forge death, conceal near corpse —
fingir_morte.src:24-57), Disfarce (disguise kit + 20s channel →
`:disguise:disguise` — transformacao.src:20-56), Troca (swap positions —
irekawari.src:54-60), Clone (shadow clone decoy + taunt —
kage_bunshin.src:32-53), Intuição (reads RP-sheet "Comportamento" of a
player — sentir_motivacao.src:36-56). Note Intuição reads from the
`:fls_core:ficharp` datafile and **fails if the target has no RP sheet**
— a prerequisite the book omits (LOW; covered by the next finding).

### "Intuição" silently requires the target to have an RP sheet
- **Handbook claim:** "Intuição — você observa um alvo por algum tempo e
  descobre suas características comportamentais." —
  src/80-magias/30-ninjutsu.md:124-125
- **Codebase truth:** `sentir_motivacao` opens `:fls_core:ficharp` and, if
  there is no element for the target serial, aborts: "Voce nao pode
  descobrir nada sobre um alvo sem ficha de RP."
  (pkg/systems/spells/spell_logic/shinobi/sentir_motivacao.src:36-42). It
  also only works on connected players (not NPCs) and requires facing +
  LoS.
- **Mismatch:** The book presents it as working on any "alvo"; in code it
  only yields anything against a *player who has filled an RP behavior
  sheet*. Undisclosed prerequisite.
- **Severity:** LOW (the technique still "works", just no-ops on most
  targets; niche RP tool)
- **Cross-links:** none

---

## Summary

| Severity | Count |
|---|---|
| HIGH | 5 |
| MED  | 4 |
| LOW  | 6 |
| INVENTED | (counted within HIGH: Sonho do Louco, Chamado Irresistível) |

Total findings: 15 (plus 3 verification-only notes that confirmed
correctness).

### Top 5 by impact

1. **HIGH — Ninjutsu "não consome mana" is false** (30-ninjutsu.md:3).
   Every technique costs 5–25 mana via the standard `cast_spell.src`
   path; no Ninjutsu mana exemption exists. A chapter-wide, class-defining
   resource claim that is wrong for all 10 entries.
2. **HIGH — "O Chamado Irresistível" does nothing** (20-canticos:163).
   `#bard_charmed` is never read by AI; the real forced-ally hook is
   `#ForcedTarget`, which the cântico never sets. C3 cântico with no
   functioning mechanic.
3. **HIGH — "O Sonho do Louco" does nothing** (20-canticos:110).
   `#bard_hallucination` is set but read by no combat/AI/targeting code;
   the perception-inversion effect is unimplemented. C2 cântico with no
   functioning mechanic.
4. **HIGH — "Requiem do Silêncio" only silences enemy Bards**
   (20-canticos:122). `#bard_silenced` is enforced solely in the
   `SPELL_TYPE_BARD` branch of `spell_executor.inc`; mages/clérigos/
   druidas/etc. cast freely while "silenced".
5. **HIGH — "Dança das Lâminas" delivers ~half its effect, for one swing**
   (20-canticos:151). Attack-speed bonus unimplemented (code TODO); hit
   bonus stored in the one-shot `#hit_mod` bucket so it expires after the
   first attack despite the advertised duration.
