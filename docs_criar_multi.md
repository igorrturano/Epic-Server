# Como criar um Multi (casa / barco) no POL

Um **Multi** é *um único item* que o cliente desenha como uma estrutura
grande (casa, barco). O servidor guarda um único ObjRef; o cliente
monta o desenho a partir de uma lista de tiles. Hierarquia:

```
Item
 └─ Multi          (footprint, items_decay)
     ├─ House       (components, custom, lockdowns, signserial)
     └─ Boat        (hold, tillerman, planks, sails, pilot)
```

## A cadeia de 3 camadas (a parte que confunde todo mundo)

Colocar um multi NÃO é "passar o id do multi do UOFiddler". O POL
precisa de três coisas, ligadas em cadeia:

```
Deed Item 0x6019        House 0x6060            Multi 0x64
(itemdesc.cfg do pkg)   (itemdesc.cfg do pkg)   (multis.cfg na raiz)
   HouseObjType ──nome──▶  Name + MultiID ──id──▶  static/dynamic ... (geometria)
```

| Camada | Arquivo | O que define | Escopo |
|---|---|---|---|
| **Geometria** | [`multis.cfg`](multis.cfg) (raiz) | `Multi 0x64 { static 0x... dx dy dz }` — a lista de tiles. Exportada do `multi.mul` do cliente. | **Global** (core do POL) |
| **Objtype** | `itemdesc.cfg` de um pkg | `House 0x6060 { Name ...; MultiID 0x64 }` — dá um objtype ao multi e o marca como House/Boat | **Por pkg, mesclado** |
| **Deed** (opcional) | `itemdesc.cfg` de um pkg | `Item 0x6019 { HouseObjType <nome> }` | Por pkg |

Os builtins `TargetMultiPlacement` / `CreateMultiAtLocation` exigem o
**objtype do bloco `House`/`Boat`** (`0x6060`, `0x11000`). NÃO o objtype
do deed, NÃO o `MultiID` cru (`0x64`).

### "Posso criar em qualquer pkg?"

- Os blocos **`House`/`Boat`/deed** podem ficar no `itemdesc.cfg` de
  *qualquer* pacote — o POL mescla todos
  ([house](pkg/multis/house/itemdesc.cfg),
  [boat](pkg/multis/boat/itemdesc.cfg),
  [config/itemdesc.cfg](config/itemdesc.cfg) convivem). Um novo
  `pkg/multis/minhascasas/itemdesc.cfg` funciona.
- A **geometria (`multis.cfg`) é global e única** — config core na raiz
  do repo, não por pkg. Formas novas de multi vão ali, independente de
  qual pkg tem o deed/script.

## Por que um multi do UOFiddler "não dá pra colocar no jogo"

O UOFiddler lê o `multi.mul`/`multi.idx` do **cliente** — mostra todos
os multis da arte. O POL **não** coloca multi por id de arte. Se você vê
`Multi 0xID` no UOFiddler mas:

- não existe bloco `Multi 0xID` na `multis.cfg` da raiz, **ou**
- não existe bloco `House`/`Boat` no `itemdesc.cfg` apontando pra esse `MultiID`,

então `TargetMultiPlacement`/`CreateMultiAtLocation` retornam o erro
**"Object Type is out of range for Multis"**.

Para reproduzir um multi do UOFiddler:

1. Confirme que `Multi 0xID { ... }` existe na [`multis.cfg`](multis.cfg)
   da raiz. Se não existir, exporte a geometria do `multi.mul` e
   adicione o bloco.
2. Adicione um bloco `House`/`Boat` em algum `itemdesc.cfg` com
   `MultiID 0xID`, ganhando um objtype.
3. (Opcional) Crie o deed `Item` que aponta pra esse bloco via
   `HouseObjType`/`ShipObjType`.

## Receita: nova casa

**1. Geometria** — adicione (ou confirme) em [`multis.cfg`](multis.cfg):

```
Multi 0x9999
{
    static  0x0064   -3   -3    0
    static  0x0065   -3   -2    0
    ...
}
```

**2. Bloco House** — no `itemdesc.cfg` do pkg (ex.:
[pkg/multis/house/itemdesc.cfg](pkg/multis/house/itemdesc.cfg)):

```
House 0x6099
{
    Name       minhacasanova
    Graphic    0x409E
    MultiID    0x9999
}
```

**3. Deed** — mesmo `itemdesc.cfg`:

```
Item 0x6099D
{
    Name          deed da minha casa nova
    Graphic       0x14F0
    Script        houseDeed
    HouseObjType  minhacasanova
    cprop numlockdowns i25
    cprop numsecure   i1
}
```

O `houseDeed.src` cuida do resto (target fantasma, criação, chaves,
locks). Veja a implementação de referência:
[pkg/multis/house/houseDeed.src](pkg/multis/house/houseDeed.src).

## Receita: novo barco

Igual, mas use `Boat` e `ShipObjType`, e o script `shipDeed`. Barcos
têm `MultiID` próprio em [`multis.cfg`](multis.cfg) (`Boat 0x0`, `Boat
0x1`, ...) e o bloco itemdesc usa objtypes altos (`Boat 0x11000`).
Referência: [pkg/multis/boat/shipDeed.src](pkg/multis/boat/shipDeed.src)
e [pkg/multis/boat/itemdesc.cfg](pkg/multis/boat/itemdesc.cfg).

Barcos precisam de **facing** nos flags; casas usam `0`:

```escript
const CRMULTI_FACING_NORTH := 0x0000;
const CRMULTI_FACING_EAST  := 0x0100;
const CRMULTI_FACING_SOUTH := 0x0200;
const CRMULTI_FACING_WEST  := 0x0300;
```

## O fluxo canônico de criação (do deed)

1. **Resolver objtype** do deed via itemdesc
   ([houseDeed.src:47-52](pkg/multis/house/houseDeed.src#L47-L52))
2. **Target fantasma**: `TargetMultiPlacement(char, objtype [, flags])`
   — envia preview translúcido, devolve `{x,y,z}` do cursor
   ([houseDeed.src:53](pkg/multis/house/houseDeed.src#L53))
3. **Validações**: distância do jogador, não em cima do deed
   ([houseDeed.src:59](pkg/multis/house/houseDeed.src#L59))
4. **Materializar**:
   `CreateMultiAtLocation(x, y, z, objtype, flags, realm)`
   ([houseDeed.src:117](pkg/multis/house/houseDeed.src#L117))
5. **Propriedades/chaves**: `ownerserial`, `owneracct`, `decayat`, um
   `AllocLockId()` compartilhado por chave + componentes travados
   ([houseDeed.src:69-95](pkg/multis/house/houseDeed.src#L69-L95))
6. **Rollback**: se `DestroyItem(deed)` falhar, destrua o multi/chaves
   ([houseDeed.src:96-104](pkg/multis/house/houseDeed.src#L96-L104))

O passo 6 é o idioma importante: a destruição do deed é o *commit* da
transação — o deed só some se todo o resto deu certo.

## Componentes: dois modelos de acesso

- **Casa**: itere `house.components` (array) e ramifique por
  `item.objtype` — portas `0x675-0x6f4`, placas `0xbd0/1/2`, baú de
  tenda `0xe43`. Cada um recebe o `LockId` + `houseserial`; o serial da
  placa volta como `house.signserial`
  ([houseDeed.src:82-95](pkg/multis/house/houseDeed.src#L82-L95)).
- **Barco**: acessores nomeados — `ship.hull`, `.wheel`, `.sails`,
  `.storage`, `.weaponslot` (arrays); `.hold`, `.tillerman`,
  `.starboardplank`, `.portplank` (refs únicos)
  ([shipDeed.src:59-97](pkg/multis/boat/shipDeed.src#L59-L97)). O
  tillerman (NPC) carrega o estado de controle (`owner`, `shipserial`,
  `course`, `decay`) e seu `usescript := ":boat:tillerman"`.

Movimento de barco é módulo POL à parte
([modules/boat.em](modules/boat.em)): `MoveBoatRelative`, `TurnBoat`,
`SystemFindBoatBySerial`, `BoatFromItem`; lógica em
[pkg/multis/boat/boat.src](pkg/multis/boat/boat.src).

## Gotcha: coordenada do offset

`TargetMultiPlacement` devolve a posição do **cursor**, não a âncora do
multi. Se passar `xoffset/yoffset` para deslocar o fantasma, subtraia o
mesmo offset no `CreateMultiAtLocation`:

```escript
var dims := GetMultiDimensions(0x6071);
if (!dims) return; endif
var loc := TargetMultiPlacement(who, 0x6071, 0, 0, dims.ymax);
if (!loc) return; endif
var house := CreateMultiAtLocation(loc.x, loc.y - dims.ymax, loc.z,
                                   0x6071, 0, who.realm);
```

Offsets positivos movem o cursor pra esquerda/baixo em relação ao
fantasma. Os scripts de deed evitam isso passando offset zero.

## Gotcha: erro NÃO é falsy em eScript

`TargetMultiPlacement`/`CreateMultiAtLocation` retornam um objeto `error`
tipado em caso de falha. `if (!ship)` **não** captura isso de forma
confiável. Use `if (x.errortext)` ou `if (x == error)` — nunca `if
(!x)`. (Foi exatamente esse bug que fez o `.createmulti` antigo passar
`Uninit` pro `CreateMultiAtLocation`.)

## Comando GM

`.createmulti <objtype|nome> [facing]` —
[pkg/commands/gm/createmulti.src](pkg/commands/gm/createmulti.src).
Aceita o objtype do bloco House/Boat (`0x6060`) ou o nome
(`smallstonehouse`/`SmallBoat`). Resolve via `GetObjtypeByName`, faz o
target fantasma e cria, mostrando o `errortext` real se falhar.
Destruir: `.destroymulti`
([destroymulti.src](pkg/commands/gm/destroymulti.src)).

## Catálogo global de multis (GM)

O `multis.cfg` tem ~440 designs de casa e 12 tipos de barco com
geometria completa mas sem objtype em nenhum `itemdesc.cfg` de deed —
inacessíveis no jogo. O pacote
[pkg/multis/catalog](pkg/multis/catalog/itemdesc.cfg) registra todos
eles para uso de GM/builder (sem deed, sem script, sem decay).

Esquema determinístico e reversível:

- `house_<MultiID>` → objtype `0x200000 + MultiID` (ex.: `house_13ec`
  → `0x2013EC`)
- `boat_<MultiID>`  → objtype `0x210000 + MultiID` (ex.: `boat_18`
  → `0x210018`)

```
.createmulti house_13ec          // casa AOS de mármore/arenito
.createmulti boat_18             // navio orc, virado pro norte
.createmulti boat_18 0x200       // navio orc, virado pro sul
.createmulti 0x2013ec            // pelo objtype cru, também funciona
```

Os 12 barcos são os navios High Seas (`boat_18` = orc, mais
gárgula/Tokuno/britânico/bote). Facing vai no arg opcional de flags;
casas usam `0`.

Notas:

- O catálogo NÃO inclui os ~300 stubs de fundação vazia
  (`0x1f41`–`0x211b`) — esses são slots reservados ao sistema de casa
  customizada (`setcustom`/`addhousepart`), não designs prontos.
- Os blocos `House` do catálogo têm só `Name` + `MultiID` (sem
  `Graphic`, como os blocos `Boat` do distro). Se o POL reclamar de
  `Graphic` faltando num `House`, adicione a linha `Graphic` — o
  desenho do multi vem da geometria em `multis.cfg` de qualquer forma.
- O POL precisa reiniciar para carregar o pacote novo (cfg é lido no
  boot, não compilado).
- Regenerar o catálogo: a lógica de diff (geometria real vs. bindings
  existentes, ignorando stubs e variantes de facing) está descrita em
  [Por que um multi do UOFiddler...](#por-que-um-multi-do-uofiddler-não-dá-pra-colocar-no-jogo).
