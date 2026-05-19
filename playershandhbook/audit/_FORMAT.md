# Handbook ↔ Codebase Audit — shared format

You are auditing the **Livro do Jogador** (`playershandhbook/src/`) against the
**actual game code** in `/home/igorrturano/epic/epic-shard/pkg/`. Report only —
**do not edit any `src/` file.** Write findings to your assigned
`audit/<domain>.md`.

## What counts as a mismatch

A claim in the handbook that the code contradicts or does not implement.
The book deliberately avoids exact numbers, so **do not flag a missing/rounded
number as a mismatch**. Flag when the *reality* is wrong:

- An effect that works differently than described (gating, stacking, tiers).
- A prerequisite the book omits (e.g. "no penalty" that actually needs a
  second ability — see calibration below).
- A rule the book asserts that **no code enforces** (invented restriction).
- An ability/skill/trait named in the book that does not exist in code, or
  exists under a different trigger/condition.
- A duplicated claim: the same mechanic stated in 2+ places — cross-link them
  so a fix hits every copy.

NOT a mismatch: flavor/lore prose, tone, exact damage/cooldown numbers the
book intentionally abstracts, GM-discretion text.

## Calibration example (the quality bar)

> **Mounted combat.** Truth: `HitController.inc:87-97` +
> `MountedCombat.inc`. Mounted attacker's combat skill is cut on a 3-tier
> ladder gated by TWO independent sources: Característica "Cavalaria de
> Escaramuça" and Habilidade "Combate Montado". Neither → −50%; exactly one
> → −25%; both → none.
> Mismatch 1 — `src/71-caracteristicas.md:169`: claims the trait alone gives
> "não recebe penalidades". False: trait alone is only 50→25; zero penalty
> needs the ability too. Severity: HIGH (misleads build choices).
> Mismatch 2 — `src/21-povos/02-bjorske.md:128`: duplicates the same false
> claim in a racial box. Cross-link of Mismatch 1.
> Mismatch 3 — "podendo até usar arcos" implies bows are blocked while
> mounted; no code enforces that. Invented rule. Severity: MED.

## Finding entry shape (use this exactly)

```
### <short title>
- **Handbook claim:** "<quote>" — src/<file>:<line>
- **Codebase truth:** <what the code actually does> — pkg/<file>:<line>
- **Mismatch:** <the precise delta>
- **Severity:** HIGH (misleads a build/combat decision) | MED (misleading but
  low-stakes) | LOW (imprecise wording) | INVENTED (rule with no code)
- **Cross-links:** other src/ lines stating the same claim (or "none")
```

End your file with a `## Summary` table: count by severity + the top 5 by
impact.

## Codebase entry points

- Ability list & IDs: `pkg/systems/abilities/include/models/Ability.inc`
  (ABILITY_ID enum + display names)
- Registries: `pkg/systems/abilities/include/AbilityRegistry*.inc`
- Per-class ability code: `pkg/systems/abilities/include/classes/<class>/`
  (folder names mirror handbook class files)
- Combat math: `pkg/systems/combat/include/Controller/` (HitController,
  DamageController), `pkg/systems/combat/weaponHitScripts/`
- Spells: `pkg/systems/spells/spell_logic/`
- Skills/perícias, fome, morte: grep `pkg/systems/` and `pkg/fls/`
- Wrappers (entity contracts): `pkg/systems/wrappers/include/Objects/Model/`
- Skills available to you: `combat-guide`, `spells-guide` (navigation aids)

## Rules

- eScript identifiers are case-insensitive — always `grep -i` when checking
  whether a symbol exists.
- Verify against code you actually read; cite `file:line`. No guessing. If a
  claim can't be verified from code, mark it `UNVERIFIED`, don't invent.
- Read-only. Never modify `src/`. Only write your `audit/<domain>.md`.
