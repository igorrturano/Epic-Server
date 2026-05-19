# Handbook ↔ Codebase Audit — Master Report (Combat-facing pass)

Report only. Per-domain detail in the sibling files:
`A-systems-traits.md`, `B-caminhos-overview.md`, `C1-armas.md`,
`C2-sabedoria.md`, `C3-subterfugio.md`, `H-racial-boxes.md`.

Scope of this pass: Parte I Sistemas, Características, Caminhos overview,
all 30 class ability lists, racial perk boxes. **Not yet audited:** Passados
(70), Perícias (72), all Magias (80-magias/*), Faccoes/Guildas (23).

## Tally

| Domain | HIGH | MED | LOW | INVENTED | UNVERIFIED |
|---|---|---|---|---|---|
| A — Sistemas + Características | 6 | 2 | 0 | 1 | 2 |
| B — Caminhos overview | 3 | 2 | 1 | 1 | 0 |
| C1 — Armas (10 classes) | 9 | 6 | 4 | 0 | 1 |
| C2 — Sabedoria (10 classes) | 8 | 6 | 0 | 1 | 0 |
| C3 — Subterfúgio (10 classes) | 3 | 4 | 4 | 1 | 3 |
| H — Racial boxes | 2 | 1 | 0 | 2 | 0 |
| **Total** | **31** | **21** | **9** | **6** | **6** |

## Recurring failure patterns (fix the pattern, not just the line)

1. **Phantom abilities** — book lists abilities that have no code:
   Caçador "Camuflagem"/"Caçador Experiente", Assassino "Salto das
   Sombras"/"Ataque Furtivo" (one real ability split into two), Construtor
   "Gancho de Agarrar", Arqueiro Arcano "Tempestade Elemental", Extração
   "Corte Limpo". Often the *real* ability at that slot is missing from the
   book (e.g. Construtor's real L10 "Ombreira Tática", Assassino's real L20
   "Manto das Sombras").
2. **Phase-7-removed passives still advertised** — Bárbaro "Resiliência
   Bárbara", Monge "Estilo do Caranguejo" still promise a physical-resist
   that was deleted; now HP-regen / evasion respectively.
3. **Stat-investment misdirection** — book tells players to invest the wrong
   attribute: Mutante Óleo do Caçador / Tiro Carregado scale Science/Ranged
   not alquimia/destreza; Duelista Reflexo Estratégico is stamina not dex.
   This actively wastes player builds — treat as HIGH.
4. **Sold-as-offense, is-defense (and vice-versa)** — Estilo de Arma Única
   ("+5 crit" → actually +3 flat dmg, zero crit), Espada e Escudo ("+2 dmg"
   → actually +5 parry).
5. **Unstated prerequisite / gating** — the mounted-combat ladder family;
   Samurai actives hard-blocked with a shield; Duelista parry needs a
   buckler; Especialização em Escudo needs an actual shield-parry.
6. **Invented restrictions** — "podendo até usar arcos" (no mounted-bow
   block exists); Deslocador/Valdruk "forma borrada ao sair na escuridão".
7. **Duplicate claims** — a wrong mechanic restated in Característica +
   path-ability + racial box. One source fix must cascade (clusters below).

## Duplicate-claim clusters (one decision → many lines)

- **Cluster: Mounted combat** — `71-caracteristicas.md:169`,
  `30-caminhos-classes.md:368`, `21-povos/02-bjorske.md:128`,
  `21-povos/07-povos-do-leste.md:164`. Truth: 50/25/0 ladder gated on trait
  *and* "Combate Montado" ability; book never conveys the ladder. Upstream
  string: `traits.cfg:195`.
- **Cluster: Cria do Deserto "no downside"** — `71-caracteristicas.md:215`
  + Gulthrak racial box. Code adds +4 cold-resist penalty
  (`coldeffect.src:65`).

## Top fixes by player impact (HIGH/INVENTED, the short list)

1. **Resistance formula (Sistemas)** — book's worked example is
   mathematically impossible; real values ≈¼ of stated. Foundational; every
   "Teste de Resistência" reference inherits this.
2. **Caçador class** — two of its listed abilities are fiction; the two real
   ones (Vínculo de Guerra, Comando de Feras) are absent. Whole block.
3. **Assassino class** — one ability presented as two, wrong tier order,
   real L20 missing.
4. **Construtor de Golems** — invented L10, real L10 missing, costs wrong.
5. **Arqueiro Arcano** — L10/L20 effects swapped/invented.
6. **Stat-misdirection set** (pattern 3) — wastes builds silently.
7. **Mounted-combat cluster** — 4 lines, your original example.
8. **Hunger / PvP-DP / Assassination-prereqs (Sistemas)** — wrong
   thresholds and wrong rules players plan around.

## Recommended next steps (your call — no edits made)

- Spot-check the HIGH/INVENTED items against the cited `pkg/` lines before
  rewriting (agent findings, player-facing text).
- Decide fix mode: I rewrite per-cluster with you approving each, or you
  triage which classes to touch first.
- Run the second pass (Passados, Perícias, all Magias, Faccoes) — Magias is
  the largest single domain still unaudited.

## Code-side issues surfaced during fixes (not handbook bugs)

- **Presença Divina fear immunity unimplemented.** `habilidades.cfg:323`
  desc promises "Você é imune a medo", but no code grants fear-resist to
  `PRESENCA_DIVINA` — `medo.src:24` only checks `AURA_DIVINA`. Handbook was
  corrected to match the engine (2026-05-19); the cfg desc and the missing
  implementation remain a code TODO for the dev to reconcile (wire the
  immunity, or fix the cfg desc).
