# Craft Pipeline — Agent Reference

Canonical, code-grounded map of the crafting system: how raw materials +
components become a finished item, how a component's material and tier
turn into the numeric bonuses combat reads off the item, and the two
distinct production flows. Companion to
[`combat-pipeline.md`](combat-pipeline.md) (crafted weapon/armor values
feed `PegaValorFabricacao` consumed in the damage/resist math).

The **value-math spine, both assembly controllers, and the production
lifecycle were traced firsthand, line by line.** Peripheral mechanics
(gump rendering, repair, tool wear, animation) were mapped via sub-agents
and are marked *mapped* — verify their exact line numbers before relying
on them. Prior craft memories (`project_craft_xp_rework`,
`feedback_pega_componente_tier_desc_dual_use`,
`project_wood_matrix_invariant`) were re-checked against live code and are
all still accurate. **If you change craft math or lifecycle, update this
doc.**

## TL;DR

- Two pipelines, same value model:
  1. **Component fabrication** ("Mesa de Componentes") — raw materials →
     *components* (items carrying material + tier props). Driven by
     `controladoraComponentes.inc`.
  2. **Complex assembly + production** ("Mesa de Ferreiro/Carpinteiro/
     Flecheiro/…") — pick a recipe, slot components into *encaixes*,
     **montagem** produces a placeholder "montado" item (objtype
     `0xe72`); double-clicking it at a valid station runs the
     **production cycle** (success/fail rolls) that yields the real item.
     Driven by `controladoraComplexos.inc` + `gerenteTratamentoProduto`.
- A finished item carries one ObjProperty, **`fabricado`**
  (`PROPNAME_FABRICADO`), holding every chosen component keyed by
  *characteristic*. Combat/tooltip reads values out of it via
  `PegaValorFabricacao(item, caracteristica)`.
- A component's contributed value =
  **`material's ComponenteEfeito value` (from `resourceList.cfg`,
  `t-N` → tier ordinal 0-4) + `the component's own tier valor` (0-4)**,
  then transformed at read-time by a per-characteristic operator in
  `caracteristicas.cfg`.
- Skill gates recipe visibility and drives the production success
  chance; XP is computed at montagem but **only paid at completion**
  (farm-safe, and suppressed by default).

## Config path constants (`scripts/include/epicConstants.inc:3-4`)

- `PROFISSAO_CONFIG_PATH := ":crafting:config/craftConfig"`
- `RECURSOS_CONFIG_PATH := ":crafting:config/resourceList"`

ObjProperty keys (`scripts/include/epicPropNames.inc:2-10`):
`fabricado` (`PROPNAME_FABRICADO`), `fabricadoTratamento`
(`PROPNAME_FABRICADO_TRATAMENTO`, the cycle counters),
`material`, `tierComponente`, `encaixeComponente` (the 3 props stamped on
a component item), `crafterserial` (`PROPNAME_ARTIFICE_SERIAL`),
`isEstacaoOcupada` (station lock), `Craft` (`PROPNAME_VELHA_CRAFTPROP`,
per-player craft prefs/memorised encaixes).

## File map (firsthand-verified line numbers unless marked *mapped*)

| Concern | File:line | Role |
|---|---|---|
| Value math | `pkg/systems/crafting/include/fabricacaoUtil.inc` | tier desc, material→value, item factory, difficulty |
| Char read | `pkg/systems/crafting/include/caracteristicasFabricacao.inc` | `PegaValorFabricacao` + operator transform + prop accessors |
| Constants | `pkg/systems/crafting/include/util/fabricacaoConstants.inc` | enums: tiers, characteristics, events, states |
| Component fab | `pkg/systems/crafting/include/controladoras/controladoraComponentes.inc` | "Mesa de Componentes" gump + `ProduzComponentes` |
| Complex assembly | `pkg/systems/crafting/include/controladoras/controladoraComplexos.inc` | recipe → encaixe → **montagem**, value computation |
| Production mgr | `pkg/systems/crafting/gerenteTratamentoProduto.src` | 3 sub-processes + event loop + **XP grant** |
| Production cycle | `pkg/systems/crafting/cicloTratamentoProduto.src` | success/fail roll engine |
| Montado entry | `pkg/systems/crafting/useItemMontado.src` | double-click montado → pick station → start manager |
| Materials cfg | `pkg/systems/crafting/config/resourceList.cfg` | per-material bonuses (see schema below) |
| Char operators | `pkg/systems/crafting/config/caracteristicas.cfg` | tooltip labels + read-time operator/valor |
| Encaixes cfg | `pkg/systems/crafting/config/encaixes.cfg` | slot Desc + which component objtypes fit |
| Profession cfg | `pkg/systems/crafting/config/craftConfig.cfg` | skills, attributes, `EstacoesValidas`, `InfoFabricacao`, recycle % |
| Recipes cfg | `:<skill>:fabricacaoDesc` / `:<skill>:componentes` | per-objtype recipe + component defs |
| XP | `pkg/mobiles/expbar/expbar.inc:442` `ComputeCraftXP`, `:475` `GrantCraftXP` | farm-safe deferred XP |
| Repair *(mapped)* | `pkg/systems/crafting/include/craftingRepair.inc` | repair vs quality |
| Tool wear *(mapped)* | `pkg/systems/crafting/include/toolWearMethod.inc` (+ legacy `toolWear.inc`) | tool durability/break |
| Wood validator | `pkg/systems/crafting/validaMatrizMadeiras.src` | boot-time 15-char wood matrix check |

## Config schemas

### `resourceList.cfg` — per-material bonuses (the heart of the value model)

Keyed `Resource 0x<materialObjtype> { … }`. Verified element (Ferro):

```
Resource 0x6309
{
    Name        Ferro
    Color       0
    Skill       50            # EffectiveSkill needed to work this material (principal only)
    Type        Metal         # Metal/Wood/Leather/Cloth/… → maps to a skill (PegaPericiaPeloMaterial)
    hpmod       0             # flat maxhp delta the material contributes
    weightMod   0             # % weight delta (no % sign)
    tier        3             # expected tier when filling a VAZIO (empty) encaixe
    inflamavel  1             # flammability (wood)
    ComponenteEfeito reducaoCritico t-2
    ComponenteEfeito defesa t-2
    ComponenteEfeito dano t-2
    …
    ComponenteEfeito eficiencia t-2
}
```

`ComponenteEfeito` grammar (documented verbatim in the cfg header,
implemented in `InterpretaMaterialEfeitos` `fabricacaoUtil.inc:372-411`):

- **Primary:** `ComponenteEfeito <caracteristica> <"t-x" | n>` — the
  single trailing token is the **principal value**.
- **Secondary (sub-effects):**
  `ComponenteEfeito <caracteristica> [<subCaracteristica> <"t-x"|n>]… <"t-x"|n>`
  e.g. `defesa cortante t-2 perfurante t-2 contusao t-2 t-2`. Tokens are
  read by `GetConfigStringDictionary` (keyed by **first** token), then
  `SplitWords`: the **last** token = `valorPrincipal`; the remaining
  name/value pairs become `efeitosAdicionais` (a `{sub: int}` dict) and
  sum into `valorTotalSubefeitos`.
- `t-x` resolves via `ResolveEfeitoCaracteristica`
  (`fabricacaoUtil.inc:360-370`) → `PegaComponenteTierDesc(x).valor`
  (**0/1/2/3/4**). A literal `n` stays literal (immune to tier rescale).

> **Wood matrix invariant** (`project_wood_matrix_invariant`, verified):
> every `Type Wood` resource must declare all **15** matrix
> characteristics — `reducaoCritico, defesa, mobilidade, bonusAparar,
> atrasoAparar, dano, precisao, critico, prestigio, resistencia,
> durabilidade, tranca, resistenciaClimaFrio, resistenciaClimaCalor,
> intimidacao`. The 4 extras (`manaMaxima, manaRegen, sacredWood,
> bonusEscolaVelkyn`) are intentionally per-wood. Boot validator
> `validaMatrizMadeiras.src:15-20` logs `[MADEIRAS] ALERTA` for any wood
> missing one. Never write two `ComponenteEfeito` lines with the same
> first token — the dict silently drops duplicates; encode subtypes in
> one line.

### `caracteristicas.cfg` — read-time transform

- `caracteristicas descricaoToolTip { <carac> <PosLabel>:<NegLabel> }`
  — tooltip word pairs (e.g. `dano Poderosa:Fraca`).
- `avancado <carac> { operador <soma|subtracao|divisao|multiplicacao>
  valor <n> }` — applied by `ExecutaOperadorCaracteristica`
  (`caracteristicasFabricacao.inc:61-85`) every time the value is read.
  e.g. `avancado doses { operador soma valor 1 }`. A characteristic with
  no `avancado` block passes through unchanged.

### `encaixes.cfg` — slots

`Encaixe <key> { Desc <label> Item <objtype>… }`. `Desc` is the slot's
display name; `Item` is the list of component objtypes that physically
fit the slot. **Objtypes here use `0xaaa`, not `0x0aaa`.**

### `craftConfig.cfg` (= `PROFISSAO_CONFIG_PATH`)

`Setup` global block (`ReciclagemPercentual` min/max recycle %, quality
dmg/resist bonus caps), `Skills`/`Skill` list, and per-profession blocks
with `Attribute` (the skill), `InfoFabricacao` (path to the recipe cfg),
`EstacoesValidas` (valid station objtypes, or `superficie`), animation/
sound/delay.

## The value-math spine (read this before touching craft numbers)

### Tiers (`fabricacaoUtil.inc:139-149`)

`PegaComponenteTierDesc(tier)` → `{id, desc, valor}`:

| tier (`COMPONENTE_TIERS`) | desc | **valor** |
|---|---|---|
| MEDIOCRE 1 | Normal | 0 |
| COMUM 2 | Melhorado | 1 |
| NOTAVEL 3 | Notável | 2 |
| SUPERIOR 4 | Superior | 3 |
| EXCEPCIONAL 5 | Excepcional | 4 |
| (invalid) | Erro! | 0 |

> **`valor` is dual-use** (`feedback_pega_componente_tier_desc_dual_use`,
> verified still live): it is both a *resist/value contribution* AND an
> *ordinal dict key* in `GeraEsquemaAleatorio`
> (`controladoraComplexos.inc:1176-1222`, `materiaisPossiveisPorTier`
> keyed by `.valor`, retrieved at `:1215/1218/1221`, clamped at
> `tiers[tiers.size()]`). Changing the `0/1/2/3/4` scale silently breaks
> random/loot material selection even though item-read combat math keeps
> working. Refactor the dict-key callers first.

### Component value (`controladoraComplexos.inc:714-749`)

`MontaPropriedadesComponente(chaveMaterial, caracteristica, tierProduto,
…)` builds `propriedades`:

```
valorEfeitos     = InterpretaMaterialEfeitos(caracteristica, chaveMaterial)  # material→value
modificadorHp    = resourceList[material].hpmod
modificadorPeso  = round(resourceList[material].weightMod / (nComponentes - nOpcionais))
valorPrincipal   = valorEfeitos.valorPrincipal + tierProduto.valor           # UNLESS caracteristica contains "VAZIO"
efeitosAdicionais, valorTotalSubefeitos = valorEfeitos.{efeitosAdicionais, valorTotalSubefeitos}
```

So the **canonical formula**:

```
component contribution = material ComponenteEfeito value (t-N → 0..4)
                       + component tier valor (0..4)
                       (+ sub-effect values, per damage/defense subtype)
```

`AvaliaPoderComponente` (`:695-712`) condenses this to
`poderContado = valorPrincipal + valorTotalSubefeitos` — used to rank the
"best" component for auto-fill and, negated, as the **crafting difficulty
modifier** (`PegaDificuldade` `fabricacaoUtil.inc:190-200`: harder recipe
the more powerful the components).

### Read-time (combat/tooltip)

`PegaValorFabricacao(item, caracteristica)`
(`caracteristicasFabricacao.inc:7-15`): reads
`fabricado.componentes[caracteristica].propriedades.valorPrincipal`, runs
it through `ExecutaOperadorCaracteristica` (the `caracteristicas.cfg`
operator), returns the int combat uses. Sub-values:
`PegaSubValorFabricacao` / `PegaSubValoresFabricacao` /
`PegaFabricacaoBundle` (hot-path: one prop read, principal + subs dict).
Characteristics with no encaixe (`manaRegen`, `manaMaxima`) derive from
the **principal component's material** via
`PegaValorMaterialPrincipalFabricacao` (`fabricacaoUtil.inc:418-441`) —
guarded so a material that doesn't declare the characteristic contributes
0 (the historical Nogueira +maxmana bug).

## The `fabricado` property structure

`CriaObjetoPropFabricado` (`fabricacaoUtil.inc:487-510`) /
`DefinePropFabricado` (`:640-661`). On a finished item:

```
fabricado = {
  estado,                       # ITEM_PRONTO | ITEM_MONTADO | ITEM_GERADO
  receita,                      # recipe (dificuldade/componentesNecessarios/… stripped at montagem)
  esquema | componentes,        # see below
  isCompletamenteComum, qualidade, modificadorHpTotal, modificadorPesoTotal,
  cor, classe, fabricante, materialPrincipal
}
```

`CriaNovaPropFabricado` (`:460-485`) **re-keys** `componentesEscolhidos`
by **characteristic** (so combat can jump straight to the component
giving a given characteristic). Each component entry:
`{ componenteNecessario, item:{nome,objtype}, material:{nome,chave},
tier:{id,desc,valor}, propriedades:{caracteristica, valorPrincipal,
efeitosAdicionais, valorTotalSubefeitos, modificadorHp, modificadorPeso},
poderAvaliado }`.

> `receita.dificuldade` is stripped by `DefinePropFabricado` — tier/XP
> **must** be computed at montagem, not completion
> (`project_craft_xp_rework`).

## Pipeline A — Component fabrication ("Mesa de Componentes")

Entry: `FabricaComponentes(quem, pericia)` → `EnviaGumpComponentes`
(`controladoraComponentes.inc:59-245`). Player picks a component
definition, picks the material(s) per required slot, sets quantity and
an "esforço" (skill pico, `CalculaValorPico = EffectiveSkill/20`),
optional recycle toggle, then `RT_BTN_FABRICAR` →
`ProduzComponentes` (`:265-319`):

1. `CalculaQuantidadePossivel` / `ConsomeMateriais` — consume raw mats
   (stackable subtract / non-stackable destroy).
2. `DistribuiQualidadeComponentes(qtd, picoPericia)`
   (`fabricacaoUtil.inc:92-137`) — Gaussian distribution of output
   **tiers** around the skill pico (failure floor = worst tier).
3. Per tier bucket: `CriaComponenteDescriptor` stamps `material`,
   `tierComponente`, `encaixeComponente`, dispensável flag, material
   color; `CreateItemInBackpack`. If recycle on, non-top tiers are
   converted back to a % of raw material (`CriaMaterialReciclado`,
   `Setup.ReciclagemPercentual`).

Output components are ordinary items carrying the 3 props; they are the
input to Pipeline B.

## Pipeline B — Complex assembly → montagem → production

Entry: skill-specific use scripts (`pkg/skills/.../make*Items.src`,
`bowcraft.src`, `carpentry.src`, `tinkering.src`, `makeAlchemyItems.src`,
`makeClothItems.src`, `makeDyeItems.src`; GM: `commands/gm/fabricar.src`)
→ `FabricaItensComplexos(quem, "<Profession>", modoGm)` → the recipe gump
(`controladoraGumpComplexos.inc` / `fabricacaoGumps.inc`, *mapped*).

1. **Recipe view** — `MontaVisaoReceitas`
   (`controladoraComplexos.inc:761-836`). A recipe shows only if
   `DeveAparecerReceita` (`:1003-1050`): origin valid (itemComum / a
   learned `recipes` entry / matching culture `chardata.crafting`) **and**
   `TemValorPericiaNecessaria` (`:1052-1056`: `skillNecessaria −
   settings.TooHard ≤ EffectiveSkill`), or GM mode.
2. **Recipe build** — `MontaReceitaItem` (`:838-881`):
   `componentesNecessarios` (`PegaComponentesNecessarios:932-976`, parses
   `ComponenteEncaixe` lines: classe / caracteristica / encaixe /
   qtdNecessaria / `principal|opcional`), `materiaisNecessarios`,
   `sucessosNecessarios` (from `DuracaoFabricacao` tag,
   `PegaDescDuracaoFabricacao` 2…14), `dificuldade`, `lote`,
   `isEmpilhavel`, `corObrigatoria`.
3. **Slot components** — for each encaixe the player (or "best"
   auto-fill `MontaMelhorEsquema:120-142`) picks a component;
   `MontaComponenteEscolhido` (`:568-595`) computes its `propriedades`
   (the value spine) and `AcrescentaComponenteEscolhido` (`:490-525`)
   stores it, updating the item's overall `qualidade`
   (`DefineTagQualidade`: −1 if any mediocre, +1 if any excepcional and
   none mediocre, else 0), HP and weight totals.
4. **Montagem** — `FabricaItens` → `FabricaItemMontado`
   (`:144-210`): `ContaQuantidadeRequisitos` →
   `isRequisitoSuficiente` → `ConsomeRequisitos` (consumes components +
   raw materials; `DevolveContainerVazio` returns empty containers).
   - **GM / `deveIgnorarRequisitos`**: `CriaItemFabricado` directly —
     skips the production cycle, item is final.
   - **Normal**: `ComputeCraftXP` is computed now (recipe difficulty
     still present) but **stamped** as `xpPendente`/`xpCategoria` on the
     montado, not awarded. `CriaItemMontado` (`:219-234`) creates the
     placeholder item objtype **`0xe72`** (`OBJTYPE_ITEM_MONTADO`) with
     the full `fabricado` prop, graphic = recipe graphic.
5. **Production** — double-click the montado → `useItemMontado.src:12`:
   resolve `EstacoesValidas` from `craftConfig`, `TargetCoordinates` a
   station, `Start_Script(:crafting:gerenteTratamentoProduto,
   {quem, item, alvo})`.

### Production lifecycle (`gerenteTratamentoProduto.src` + `cicloTratamentoProduto.src`)

`GerenteTratamentoProduto` starts **3 managed sub-processes**
(`IniciaProcessosGerenciados:139-156`): `tratamentoProdutoGump`
(interactive), `cicloTratamentoProduto` (rolls), `animacaoTratamentoProduto`
(visuals). Event loop on `EV_TRATAMENTO`:

- **`cicloTratamentoProduto.src`** loops while the manager lives:
  `PegaDificuldadeFrabicarObjeto` → `rolagem = RandomIntMinMax(0,100)`;
  `CheckSkillAdvance(quem, pericia, skill+15, chancePelaSkill)`;
  `isSucesso = rolagem ≤ chanceTotal`. Counters live on the item as
  `PROPNAME_FABRICADO_TRATAMENTO` `{sucessos, falhas, ciclos,
  ultimoSucesso, isTratando}`.
  - `falhas ≥ PegaMaximoFalhas` (= `sucessosNecessarios ×
    MULTIPLICADOR_FALHAS(2)`) → **`EV_ITEM_QUEBROU`** (item breaks).
  - `sucessos ≥ receita.sucessosNecessarios` → **`EV_ITEM_PRODUZIDO`**.
- **`EV_ITEM_PRODUZIDO`** (`gerenteTratamentoProduto.src:52-62`):
  `GrantCraftXP(quem, fabProp.xpPendente, fabProp.xpCategoria)` — the
  **only** XP grant point — then `CriaItemFabricado(quem, produto)` turns
  the montado into the real item(s) (`fabricacaoUtil.inc:263-333`:
  applies maxhp/weight mods, alchemy pote info, lock via `AplicaFechadura`
  if it has `tranca`, batch `lote`, stamps `crafterserial` for
  non-stackables).
- `EV_FIM_CICLO` keep cycling · `EV_TODOS_ALTERADO` batch "do all
  similar" · `EV_CANCELADO` restore + return montado. After success the
  manager hunts the backpack for another identical montado
  (`EncontraNovoProduto`, by `0xe72` + same `receita.objtype`) to batch.

## Skills & difficulty math (`fabricacaoUtil.inc`)

- `PegaSkillNecessaria` (`:76-89`) — required skill from the recipe's
  `InfoFabricacao`, indexed by recipe class.
- `PegaChanceFabricacaoBase` (`:238-250`) — `CalcSuccessPercent`
  vs effective skill, + `PegaDeslocamentoFacilitador` (a flat bonus that
  grows with `sucessosNecessarios`: 0→46), capped 100.
- `PegaDificuldade` (`:170-207`) — base chance minus each component's
  `poderAvaliado.poderContado` (powerful components = harder craft).
- `PegaChanceTratamentoFinal` (`:209-222`) — binomial probability of
  reaching `sucessosNecessarios` successes before `2×` failures.
- `EffectiveSkill/20` is the "pico" feeding the Gaussian tier
  distribution of fabricated components.

## XP (`project_craft_xp_rework`, verified)

`ComputeCraftXP` (`expbar.inc:442`) at montagem → stamped
`xpPendente`/`xpCategoria` on the montado → `GrantCraftXP`
(`expbar.inc:475`) only at `EV_ITEM_PRODUZIDO`. **Suppressed by default**
(logged `CRAFT_SUPPRESSED` unless GlobalProperty `CraftXpEnabled=1`); GM
direct-craft path awards nothing (no cycle). The rate-limiter/fatigue
system is deliberately **not built** — needs real `xp.log` distribution
data first; don't add it blind, don't re-enable globally without the
analysis.

## Random / loot generation

`CriaPropFabricadoGerada` → `GeraEsquemaAleatorio`
(`controladoraComplexos.inc:1133-1258`): builds an `ITEM_GERADO` esquema
for an objtype at a tier window, picking materials by
`FABRICACAO_MODO_MATERIAL` (`MESMO_TIER` / `MESMO_TIER_OU_MENOR` /
`MENOR_QUE_TIER`). This is the path the dual-use `.valor` pitfall lives
in — see the tier table note above.

## Repair & tool wear *(mapped — verify lines before relying)*

- **Repair** (`craftingRepair.inc`): `repair_item` — gated by skill vs
  item skill and a finite `reparos` budget; each attempt erodes
  `item.quality` (so repeated repair degrades the item); success restores
  ≈ `(skill+1)/2` hp; failure can damage or destroy. Repair quality is
  bounded by the item's crafted `quality`, which traces back to the
  component tiers (`DefineTagQualidade`).
- **Tool wear** (`toolWearMethod.inc`, newer; `toolWear.inc` legacy):
  `swings` initialised `≈ 50 × tool.quality + eficiencia×10`; each
  qualifying use (roll > clamped skill) decrements `swings` and `hp`;
  `ABILITY_ID::TOOL_PRESERVATION` = 50% skip; tool destroyed at 0.
  `eficiencia` is itself a crafted characteristic — better tools last
  longer.

## Pitfalls (verified)

1. **`PegaComponenteTierDesc.valor` is dual-use** (resist contribution
   *and* ordinal dict key in `GeraEsquemaAleatorio`). Don't rescale
   `0/1/2/3/4` without refactoring `controladoraComplexos.inc:1176-1222`
   first — combat keeps working (reads stamped values) while loot/random
   material selection silently breaks. Same trap in
   `ResolveEfeitoCaracteristica` (`fabricacaoUtil.inc:360-370`).
2. **Wood matrix must be complete** — every `Type Wood` needs all 15
   matrix characteristics or components requesting a missing one get a
   silent empty buff. Boot validator + the explicit-0 init in
   `InterpretaMaterialEfeitos` guard it; never duplicate a
   `ComponenteEfeito` first token.
3. **XP is farm-shaped** — only ever award at `EV_ITEM_PRODUZIDO`. The
   montagem step is repeatable/cancellable; awarding there is farmable
   (the original bug). `receita.dificuldade` is gone post-montagem, so
   compute XP/tier at montagem and carry it on the prop.
4. **GM/`deveIgnorarRequisitos` skips the production cycle entirely** —
   `FabricaItemMontado` calls `CriaItemFabricado` directly, so no rolls,
   no XP, no tool wear. Don't use it to reason about the normal flow.
5. **The `fabricado.componentes` dict is re-keyed by characteristic** at
   `CriaNovaPropFabricado`, not by encaixe index. Read by characteristic
   (`PegaValorFabricacao`), and remember `manaRegen`/`manaMaxima` have no
   encaixe — they derive from the principal component's material.
6. eScript identifiers are case-insensitive — verify renames with
   `grep -ri`. Heavy includes: keep `controladoraComplexos.inc` off hot
   combat paths (read values via the small `caracteristicasFabricacao.inc`
   accessors instead).

## Connection to combat-pipeline.md

Crafted weapon/armor stats are not stored as plain fields — combat reads
them live from the `fabricado` prop:
[`combat-pipeline.md`](combat-pipeline.md) §5a `CalculateWeaponDamage` /
armor resist consume `PegaValorFabricacao(item, "dano"|"defesa"|…)` (+
sub-values per damage type). A component/material/tier change here moves
real combat numbers — model the value with the spine formula above before
assuming a balance impact.

## When you change something here

- New characteristic: add to `COMPONENTES_CARACTERISTICA`
  (`fabricacaoConstants.inc`), give it a `caracteristicas.cfg` tooltip
  pair (+ optional `avancado` operator), wire a reader via
  `PegaValorFabricacao`, and decide if it needs an encaixe or derives
  from the principal material.
- New material: add a `Resource 0x…` block with **all** matrix
  characteristics (15 for wood), `Type`, `Skill`, `tier`. Run the boot
  validator.
- Touching tier `valor`: refactor the `materiaisPossiveisPorTier`
  dict-key callers first (pitfall 1).
- Touching XP: only at `EV_ITEM_PRODUZIDO`; keep suppressed-by-default;
  pull `xp.log` before any rate-limiter.
- New station/profession: add to `craftConfig.cfg`
  (`EstacoesValidas`, `InfoFabricacao`, `Attribute`); the entry use
  script just calls `FabricaItensComplexos(who, "<Profession>")`.
