# Religion System — Agent Reference

Canonical, code-grounded reference for the seven in-game religions and how
they wire into abilities, spells and the handbook. Attach this doc as
context whenever you add, modify, or document a religion-keyed mechanic.

## TL;DR

- Religion is a **string** stored on `chardata.religion` for players,
  `npc_religion` ObjProperty for NPCs, read via
  `MobileWrapper(...).GetReligion()`.
- Seven religions, two writing systems — **the `LORE` constant uses a
  Cyrillic homoglyph**, see "Footguns" below. Always use the constant,
  never the literal string.
- Two code patterns:
  1. `case (religiao)` with up to seven branches — different effect per
     religion (Sentença Profana style).
  2. `has_banner && memberWrapper.GetReligion() == caster_religion` —
     amplifies an effect when caster carries the matching banner.
- All `case (religiao)` blocks live in
  `pkg/systems/abilities/include/classes/` (9 abilities today). Spell-side
  religion use is rare (2 files).
- Handbook convention: per-religion bullet list (`Lóre / Veneratio / Bokk
  Bharaz / Ush'krar / Halleuad / Eluith'orth / Kai`), use **"Nenhum efeito
  adicional"** for religions that have no `case` — never invent.

## The religion enum

```escript
// scripts/include/epicConstants.inc:40-48
enum RELIGIAO
    VENERATIO  := "Veneratio",
    ELUITHORTH := "Eluith'orth",
    KAI        := "Kai",
    HALLEUAD   := "Halleuad",
    LORE       := "Lórё",       // ⚠ Cyrillic ё (U+0451), see Footguns
    BOKK_BHARAZ:= "Bokk Bharaz",
    USHKRAR    := "Ushkr'ar"
endenum
```

The line below the enum (`ARADALORE`, `AHTALELORETH`, `KAIJIN`, …) is a
**different concept** — spell schools / lores, not religions. Don't confuse
them.

## Storage & access

**Players:** religion is picked at character creation and stored as
`chardata.religion` (an ObjProperty struct field).
`epicgumpcriacao.src:598` writes it; `:883` sets the racial-default `LORE`
fallback.

**NPCs:** the religion is set at brain init by `makeReligion()` and stored
in the `npc_religion` ObjProperty. Only some NPC archetypes (cleric,
arqueiro arcano) have one — others return the `IsNPCException`.

**Always read via the wrapper.** `MobileWrapper(mob).GetReligion()`
([`MobileWrapper.inc:136-…`](pkg/systems/wrappers/include/Objects/Model/MobileWrapper.inc#L136))
handles both paths (player `chardata.religion` + NPC `npc_religion` lazy
load) and caches the result on `this.Religion`. Never `GetObjProperty(...,
"chardata").religion` directly — the wrapper is the contract.

The interface declares it: `IMobileWrapper.inc:29`.

## The banner mechanic

Several abilities gate or amplify their effect on **the caster carrying a
religious banner**. `MobileWrapper.HasReligiousBannerEquipped()`
([`MobileWrapper.inc:1380`](pkg/systems/wrappers/include/Objects/Model/MobileWrapper.inc#L1380))
checks LAYER_HAND1 / LAYER_HAND2 for these objtypes:

```escript
var banners := { 0xF091, 0xF092, 0xF090, 0xECEB, 0xECEA, 0xECE9 };
```

The recurring pattern in code:

```escript
var has_banner       := mobile.HasReligiousBannerEquipped();
var caster_religion  := mobile.GetReligion();
…
if (has_banner && memberWrapper.GetReligion() == caster_religion)
    // amplified effect (extra blessing, increased duration, etc.)
endif
```

This is **not** a per-religion `case` — it's a single condition that fires
only when banner-equipped and ally religion matches caster religion. Used
in `paladino/DivinePresence.inc`, `paladino/AuraOfFaith.inc`.

## The two code patterns

### Pattern A — `case (religiao)` (per-religion distinct effect)

```escript
var religiao := mobile.GetReligion();
var theology_bonus := CInt(mobile.GetSkill(THEOLOGY) / 10);
…
case (religiao)
    VENERATIO:
        // silence / mana drain / corpofechado / etc.
    BOKK_BHARAZ:
        // slow / knockdown / shield barrier / etc.
    HALLEUAD:
        // blind / heal allies / Tactics buff / etc.
    ELUITHORTH:
        // darkness / true_darkness / nightvision off / etc.
    KAI:
        // ignore armor / crit immunity / guaranteed crit / etc.
endcase
```

Important: `Lóre` and `Ush'krar` rarely have a `case` — they fall through
the `endcase` with only the base effect. **This is intentional.** The
handbook bullet for those religions should be **"Nenhum efeito adicional
(apenas o efeito base)"**, not an invented effect.

### Pattern B — Banner+religion amplification

Already described above. Used for party-wide buffs where the *quantity* or
*quality* of the buff scales with banner+religion-match, rather than
branching on the caster's religion choice.

## Catalog of religion-keyed code (today)

Every `case (religiao)` block in the codebase, by class folder:

| File | Ability | Effects (per religion) |
|---|---|---|
| `cacador_de_demonios/ProfaneSentence.inc:55` | Sentença Profana | base divine dmg + religion rider (Veneratio silence, Bokk Bharaz slow, Halleuad blind, Eluith'orth darkness, Kai bleed; **Lóre/Ush'krar = no rider**) |
| `cacador_de_demonios/AbyssalForm.inc:87` | Forma Abissal | per-religion rebuke on attacker |
| `paladino/DivineSmite.inc:49` | Golpe Divino | per-religion debuff on target |
| `paladino/DivineChampion.inc:87` | Campeão Divino | per-religion rebuke on attacker |
| `cleric/DivineConnection.inc:36` | Conexão Divina | per-religion targeted effect (target gets crit-immunity / truesight / silence+dmg / shield / Tactics-or-Awareness buff) |
| `cleric/RadiantSmite.inc:34` | Golpe Radiante | per-religion **completely different mini-ability** (chicote dominante / contusiva à distância / …) |
| `cleric/DivineAura.inc:37` | Aura Divina | per-religion targeted buffs / summons |
| `cleric/DivineAuthority.inc:52` | Autoridade Divina | per-religion AoE field effect (silence all / crit immunity / dmg reduction / blind / true_darkness) |
| `cleric/Apotheosis.inc:41` | Apoteose | per-religion ultimate (rot / stone skin / heal+destroy undead / torment / guaranteed crits) |

**Banner+religion-match** (Pattern B) currently in:
- `paladino/DivinePresence.inc` (4 commands amplified)
- `paladino/AuraOfFaith.inc`

**Spell-side** religion use is intentionally minimal (the M2 audit
verified it): only `pkg/systems/spells/spell_logic/priest/santuario.src`
(KAI swaps a cosmetic FX — fire vs energy) and `pkg/systems/spells/
priestaltar.src` (altar identity). Spells, as a system, deliberately avoid
per-religion branching — gameplay distinctness lives in the ability layer.

The Will resist also reads religion indirectly via the
`CONSAGRACAO_DIVINA` ability (`+THEOLOGY/20` bonus) — see
[`docs/resist-system.md`](resist-system.md).

## Handbook documentation convention

The handbook's Caçador de Demônios chapter (`src/60-caminho-subterfugio/
03-cacador-de-demonios.md` Sentença Profana, ~line 66) is the **canonical
template** for documenting a religion-keyed ability:

```markdown
Seu próximo ataque desfere dano divino adicional. Conforme a sua religião,
ele também aplica um efeito extra no alvo:

- **Lóre**: Nenhum efeito adicional (apenas o dano divino)
- **Veneratio**: Silencia o alvo
- **Bokk Bharaz**: Deixa o alvo mais lento
- **Ush'krar**: Nenhum efeito adicional (apenas o dano divino)
- **Halleuad**: Cega o alvo
- **Eluith'orth**: Envolve o alvo em trevas
- **Kai**: Causa sangramento no alvo
```

Rules:
- Always list **all seven** religions in the bullet list, even when most
  don't `case` — write "Nenhum efeito adicional" for those. This prevents
  player confusion about whether their religion just isn't listed or
  truly does nothing extra.
- Keep numbers abstract ("por um tempo", "consideravelmente") — never
  expose `theology_bonus` math. Numeric truth lives in the code; the
  handbook conveys the *shape* of the effect.
- Spell `Tipo` / `Custo` / `Cooldown` / `Alvo` rows stay untouched —
  the religion section replaces only the "depends on your religion"
  one-liner with the descriptive lead-sentence + bullets.
- For Pattern B (banner amplification), prose-form note is enough —
  e.g. *"Se você portar o estandarte da sua religião, os efeitos sobre
  aliados que compartilham essa fé são amplificados."* — not a 7-row
  table.

The full audit's "Deferred — Religion-keyed ability documentation" entry
in `playershandhbook/audit/MASTER.md` tracks which abilities still need
this treatment (8 abilities + 2 banner notes as of 2026-05-19).

## Footguns for agents

- **`LORE` is `"Lórё"` with Cyrillic `ё` (U+0451), not Latin.** A literal
  `"Lórё"` you type with a normal keyboard will NOT equal the constant.
  Always reference `LORE` (the enum constant). If you must hardcode the
  string for any reason, copy-paste from `epicConstants.inc`, never retype.
  Equality compares as strings — a Latin `é` instead of Cyrillic `ё`
  silently mismatches.
- **NPCs without a religion return the `IsNPCException`.** Treat
  `GetReligion()` as potentially `is @Exception` — never `case` an
  exception value or assume a string came back. Especially for ability
  hooks that run from NPC-side too.
- **Don't invent rider effects.** If a religion has no `case` in code,
  it has no extra effect. Document it as "Nenhum efeito adicional" — the
  audit (`audit/MASTER.md`) explicitly flags inventing riders as a
  recurring failure pattern.
- **Don't add a new `case` without updating the handbook.** If you add a
  religion branch to an ability, also update the corresponding handbook
  entry's bullet list, or you reproduce the very gap the audit just
  closed.
- **Don't `GetObjProperty(mob, "chardata").religion` directly.** Use
  `MobileWrapper(mob).GetReligion()`. The wrapper handles the player vs
  NPC paths and caches.
- **`ARADALORE` / `AHTALELORETH` / `KAIJIN` are spell schools, not
  religions.** They live in the same `epicConstants.inc` block immediately
  below `RELIGIAO` but belong to a different concept (magic lores). Don't
  cross them.
- **Spell layer deliberately avoids religion branching** (except cosmetic
  FX in santuario). If you find yourself wanting a per-religion effect on
  a spell, ask whether it belongs in the ability layer instead — the
  established pattern is "spells are religion-agnostic; abilities carry
  the religion flavor".
- **Banner objtype list is closed (6 entries).** If you add a new banner
  object, add its objtype to the `banners` array in
  `MobileWrapper.HasReligiousBannerEquipped`, or it won't be detected.

## Related references

- [`docs/resist-system.md`](resist-system.md) — `CONSAGRACAO_DIVINA` (Will
  bonus from Theology) and the resist test mechanic that several
  religion-keyed abilities call against.
- `playershandhbook/audit/MASTER.md` — pending religion-documentation
  work (8 abilities + 2 banner notes); the canonical Sentença Profana
  rewrite lives in `src/60-caminho-subterfugio/03-cacador-de-demonios.md`.
- `pkg/systems/wrappers/include/Objects/Model/MobileWrapper.inc` —
  authoritative source for `GetReligion` / `HasReligiousBannerEquipped`.
