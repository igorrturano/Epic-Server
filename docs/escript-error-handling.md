# eScript Error Handling — Convenção Canônica

> Documento gerado a partir da sonda empírica `.diagerr` em 2026-05-24
> (commit `97a3ac125`). Reflete o comportamento atual do POL deste shard.

## TL;DR

| Origem do valor | Como detectar "ausente / inválido" |
|---|---|
| `GetObjProperty`, `.Cprops[]`, `PegaPropObjItemOrTemplate` | `if (!v)` |
| Builtins de mobile/item (`.connected`, `.npctemplate`, etc.) | `if (!v)` |
| Wrapper getters (`MobileWrapper.Race`, etc.) | `if (v is @Exception)` |
| `Wrapper.GetObjRef()` em path inválido | `if (v is @Exception)` |

## Matriz empírica

Resultados do `.diagerr` executado em um player válido equipado com arma e backpack:

| Cenário | TypeOf | `!v` | `v == error` | `v is @Exception` |
|---|---|:---:|:---:|:---:|
| `error` builtin | Error | 1 | 1 | 0 |
| `var x;` (uninit) | Uninit | 1 | 1 | 0 |
| `GetObjProperty(obj, "missing")` | Error (`error{ errortext }`) | 1 | 1 | 0 |
| `PegaPropObjItemOrTemplate(missing)` | Error | 1 | 1 | 0 |
| `error.qualquerCampo` | Uninit | 1 | 1 | 0 |
| `player.npctemplate` | Uninit | 1 | 1 | 0 |
| `player.connected` (online) | Integer | 0 | 0 | 0 |
| Valid `MobileWrapper.Race` (string) | String | 0 | 0 | 0 |
| `MobileWrapper(player).Template` | ClassInstance | **0** | **0** | **1** ← IsPlayerException |
| `ItemWrapper(player).GetObjRef()` | ClassInstance | **0** | **0** | **1** ← InvalidFormatException |
| `MobileWrapper("orc").GetObjRef()` | ClassInstance | 0 | 0 | 1 ← IsTemplateException |
| `MobileWrapper("inexistente").GetObjRef()` | ClassInstance | 0 | 0 | 1 ← InvalidFormatException |
| `ArmorWrapper(weapon).GetObjRef()` | ItemRef ⚠ | 0 | 0 | 0 ← **sem validação de subtipo** |
| `WeaponWrapper(backpack).GetObjRef()` | ItemRef ⚠ | 0 | 0 | 0 ← idem |

## A regra de ouro

**Duas convenções dependendo da camada:**

### 1. POL builtin layer (prop reads, native fields)

POL `error` e `error{...}` são **falsy**. Uninit também. Integer `0` também.
Tudo o que significa "ausente" cai bem em `!v`.

```escript
// CERTO ✓
var disfarce := GetObjProperty(player, "disfarce");
if (disfarce)
    // disfarce existe e não é 0/uninit/error
endif

// REDUNDANTE (funciona mas verboso)
if (disfarce != error && disfarce)

// REDUNDANTE TAMBÉM
if (disfarce == error || !disfarce) return; endif
// equivale a
if (!disfarce) return; endif
```

### 2. Wrapper layer (classes do pkg/systems/wrappers/)

Wrapper getters retornam **class instances** de Exception em paths
inválidos. Class instances são **truthy** (TypeOfInt 42). `!v` e
`== error` falham. Use `is @Exception`.

```escript
// CERTO ✓
var mw := MobileWrapper(target);
var race := mw.Race;
if (race is @Exception)
    return; // target é NPC ou template, sem race
endif

// ERRADO ✗ — Exception class é truthy
if (!race) ... 
// ERRADO ✗ — Exception class não é POL error
if (race == error) ...
```

Para combinar os dois mundos (valor pode ser ausente OU exception):

```escript
function IsUsable(v)
    if (!v) return 0; endif
    if (v is @Exception) return 0; endif
    return 1;
endfunction
```

## Pegadinhas

### `tier < 0` quando tier pode ser error

`error < 0` é comparação não testada. Mantenha o guard:
```escript
var tier := GetObjProperty(obj, "tier");
if (tier != error && tier < 0)  // mantém o guard, comparison é semântica
```

### `who.connected` em ref deletada

Não temos sonda empírica para ref deletada. Por defesa em depth,
o gate de loops longos (ex: char creation) ainda checa `!who || !who.connected`.

### Wrapper subtipo não validado

`ArmorWrapper(weapon_ref)` não throw — retorna o weapon ref como se fosse armor.
Métodos especializados (`GetResists()`) podem retornar dados sem sentido sem
levantar exception. Defenda no chamador:

```escript
if (item.isA(POLCLASS_ARMOR))
    var aw := ArmorWrapper(item);
    // ...
endif
```

Ver [feedback_wrappers_no_subtype.md](../../../home/igorrturano/.claude/projects/-home-igorrturano-epic-epic-shard/memory/feedback_wrappers_no_subtype.md) (memória local).
