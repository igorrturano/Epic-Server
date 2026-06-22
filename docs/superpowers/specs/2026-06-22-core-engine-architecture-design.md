# Arquitetura Core ↔ Engine — Fundação da migração POL → ModernUO

> **Status:** Design v1. Pré-implementação. A ser revisado pelo EvilLink contra o `Fractura.Core`.
> **Escopo:** SÓ a arquitetura de fundação (a fronteira Core/Engine). NÃO cobre a estratégia de
> conversão dos sistemas, nem as Skills do Claude que guiam a conversão, nem a conversão em si —
> esses são specs separados, que dependem deste.

---

## 1. Objetivo e contexto

Migrar o shard (hoje em POL/eScript) para **ModernUO** (.NET 10, C# 14, serialização gerada por
código, persistência de alta performance). Motivos: engine reescrita e sólida (não o legado do
RunUO), OOP de verdade, fim dos bugs herdados, e o cliente custom (epicuo/ClassicUO) continua
intacto.

Decisão estratégica de longo prazo: deixar a **regra de jogo portável** para que, no futuro, o
mesmo núcleo rode num **standalone CRPG em Unity** (o projeto `Fractura` do dev, já prototipado).
A construção é **sequencial**: servidor primeiro, Core "redondo" para copiar/referenciar depois.
Não há desenvolvimento em paralelo.

O design abaixo é **alinhado com o `Fractura.Core` existente** de propósito — a consistência entre
os dois Cores é o que torna o port futuro mecânico.

---

## 2. As três camadas

ModernUO já separa `Server` (engine) de `UOContent` (conteúdo), mas o `UOContent` é acoplado ao
`Server`. Adicionamos uma **terceira camada** e invertemos a dependência:

```
Server/            ENGINE — plataforma ModernUO (rede, mundo, serialização). Não tocamos.
                   ↑
Game.Core/         CORE — regra pura. C#/BCL baunilha, ZERO referência de engine.
                   Se tentar usar Mobile/packet/tile aqui, NÃO COMPILA.
                   ↑
UOContent/         COLA/ADAPTER — código nosso casado com o ModernUO. Referencia Core E Server.
                   Subclasses de Mobile/Item, packet handlers, hooks. Traduz e aplica.
```

**A regra de dependência (o coração de tudo):** a seta aponta numa direção só —
`UOContent → Game.Core`. O `Game.Core` **nunca** conhece `UOContent` nem `Server`. O compilador
*força* isso: regra de jogo que toca num `Mobile` não compila. A disciplina deixa de ser "lembrar
de fazer certo" e vira "é impossível fazer errado".

**Bônus imediato (antes do standalone):** `Game.Core.Tests` roda fórmulas de combate e resolução
de skill em milissegundos, **sem subir servidor, sem mundo, sem cliente**.

---

## 3. O teste da fronteira

Para decidir em qual camada algo vive:

> **"Se eu apagar o ModernUO e plugar o jogo numa engine diferente (o standalone Unity), esse
> código sobrevive sem mudar?"**
> - Sobrevive → **Core** (regra pura).
> - Quebra porque menciona `Mobile`, packet, tile, save, animação → **Engine/cola**.

| Exemplo | Camada |
|---|---|
| "dado atacante + defensor, calcule o dano" | Core |
| "ler resist do `Mobile`, tocar animação, mandar packet" | Cola/Engine |
| "budget de stealth zerou e o re-roll falhou → revelar o ator" | Core |
| "fazer screen shake, decrementar `stealthsteps`, avisar NPCs da guilda" | Cola/Engine |

**O Core decide o quê acontece. A cola aplica e mostra.**

---

## 4. MVVM — espelhando a estrutura do Fractura

Adotamos **MVVM** no Core, replicando a estrutura do `Fractura.Core` do dev. Justificativa: (a)
consistência com o standalone = port mecânico; (b) MVVM é um misto bom de facilidade/desempenho
e segue o grão OOP do ModernUO (ECS brigaria com a herança Mobile/Item).

Reframe importante que destrava o MVVM no servidor: **num jogo cliente-servidor, o cliente É a
View.** O epicuo/ClassicUO é o renderizador; o servidor não *é* a tela, ele *fala* com a tela por
packets. Logo o MVVM transfere:

| Camada | No Fractura (Unity) | No shard (ModernUO) |
|---|---|---|
| `EnemyModel` (Core) | dado | dado — idêntico |
| `EnemyViewModel` (Core) | onde o cálculo acontece | **idêntico** — mesma lógica |
| `IEnemyView` (Core) | port | port — idêntico |
| `EnemyView` (impl, Engine) | MonoBehaviour que renderiza | **adapter que faz broadcast de packet** pros clientes no alcance |

Mesmo Core, mesma ViewModel, duas implementações de View (Unity renderiza; servidor manda packet).

### Estrutura de pastas do Core (seguindo o Fractura)

```
Game.Core/
  Mobile/
    Enemy/
      AI/
        Context/        (EnemyAIContext)
        Helpers/        (VisionChecker)
        Interfaces/     (IAIContext, IEnemyStrategy, IState, IStateMachine)
        State/          (Idle, Patrol, Chase, ... + EnemyStateMachine)
        Strategies/     (Attack, Chase, Idle, Run, Walk, ...)
      Interfaces/        (IEnemyModel, IEnemyView, IGroundEnemy, IFlyingEnemy)
      Models/            (EnemyModel)
      ViewModels/        (EnemyViewModel)
  ... (outros domínios na mesma forma)
```

**A AI (State machine + Strategies) é diretamente o que os brains de NPC do shard precisam**
(alinha com o projeto Brain V2). Lógica pura no Core, reaproveitável nos dois mundos.

---

## 5. Os dois padrões de fronteira

### 5.1 Inversão de Dependência (ports) — para comportamento

O Core **define a interface**, a Engine **fornece a implementação**. Ex.: `IEnemyView` mora no
Core; `EnemyView` (Unity) ou o broadcaster de packet (ModernUO) implementam. O Core comanda/notifica
sem conhecer a classe concreta. A seta de dependência continua apontando só pra dentro do Core.

**Disciplina obrigatória:** as interfaces do Core falam **evento/intenção abstrata**, nunca verbo
de engine.
- ✅ `OnTookDamage(amount)`, `OnMovedTo(pos)`, `OnAppeared()` — Unity e servidor implementam.
- ❌ `FlashRed()`, `PlayUnityDeathAnim()` — vazou apresentação/Unity pro Core.

A ViewModel **notifica** "levei dano"; cada View decide que isso *parece* um flash (Unity) ou *é*
um packet de hit (servidor).

### 5.2 Adapter — para conversão de tipo

Traduz **dados** entre Core e Engine. Ex.: `Position2D` (Core) ↔ `Point3D`/`Vector2` (Engine).

- O adapter mora **no lado da Engine** (precisa conhecer os dois tipos; o Core não pode ver a engine).
- É **`static`** (conversão sem estado, sem alocação no heap, performance). Não precisa de interface.
- Dados que cruzam a fronteira em caminho quente são **`struct`** (value type), não classe.

Referência do dev (`Fractura.Unity.Infrastructure.Adapters.PositionAdapter`):

```csharp
public static class PositionAdapter {
    public static Position2D ToPosition2D(Vector2 u) => new(u.x, u.y);
    public static Vector2   ToVector2(Position2D c) => new(c.X, c.Y);
}
```

No shard, escrevemos o equivalente convertendo os tipos do ModernUO ↔ os tipos do Core. **Mesmo
Core, um conjunto de adapters por engine.**

---

## 6. Estado e serialização (sem acoplar o Core)

**Decisão: Core sem estado autoritativo no servidor.** O estado autoritativo mora na **entidade
da engine** (subclasse de `Mobile`/`Item`); o Core é lógica que recebe um retrato (struct/snapshot),
decide e devolve resultado. Se o Core não guarda estado de longa vida, **não há nada para
serializar no Core** — em engine nenhuma.

Isso encaixa na serialização do ModernUO, que é **gerada por código** (diferente do ServUO, onde
se escreve `Serialize`/`Deserialize` na mão). Anota-se o campo, o gerador faz o resto:

```csharp
[SerializationGenerator(0)]
public partial class EpicPlayer : PlayerMobile {
    [SerializableField(0)] private int    _vocationId;
    [SerializableField(1)] private double _stealthSkill;

    public void Attack(EpicPlayer target) {
        var result = CombatRules.Resolve(this.ToSnapshot(), target.ToSnapshot());
        this.ApplyDamageResult(result); // escreve de volta nos campos serializáveis
    }
}
```

Sem método de serialização escrito à mão, sem Façade, sem bind, **uma cópia só de estado**.

**Sobre o "Façade + Bind" do dev:** é um padrão engine-agnóstico legítimo, mas para o *servidor*
ele introduz estado duplicado + sincronia (colide com "one source of truth", custa perf e memória,
e abre caso de estado rasgado em concorrência). O ModernUO já resolve persistência com o source-gen,
então **não usamos Façade/bind no servidor**. Esse padrão (ou um surrogate só no save/load) volta a
ser relevante no **capítulo Unity**, onde a serialização é outra fera. Nos dois casos o **Core não
serializa nada** — cada engine cuida da persistência do seu jeito.

---

## 7. Plataforma e portabilidade

- **.NET 10 / C# 14 em tudo** (Server, Core, UOContent). Sem `netstandard2.1` — não vale handicapar
  o presente; a aposta é que a Unity estará em .NET moderno quando o standalone chegar.
- **Disciplina barata de portabilidade:** o `Game.Core` usa **C#/BCL baunilha** — evita API exótica
  de runtime. Regra de jogo pura não precisa disso, e isso mantém o copia-e-cola futuro trivial
  independente de onde a Unity aterrissar. (Curiosidade: ModernUO em C#14 está à frente da Unity —
  o port futuro *baixa* o nível, não sobe.)

---

## 8. Fora de escopo (specs futuros)

1. **Design final do sistema de skill/vocação** — SKILLS_REDESIGN.md + specs de vocação. Pré-requisito
   para converter skill; é trabalho de design já em andamento, não de migração.
2. **Skills do Claude Code** (`.claude/skills`) que codificam as regras desta arquitetura + as receitas
   de conversão, para a IA converter de forma consistente. Dependem deste design existir.
3. **Estratégia de conversão** — mapa POL → ModernUO (wrappers/brains/spells/packets) e a receita
   repetível de porte.
4. **A conversão em si** — fatiada por sistema.
5. **Pôr um ModernUO limpo de pé** — tarefa independente (instalar .NET 10, clonar, `publish.sh`,
   conectar com o epicuo). Não bloqueia nada.

---

## 9. A confirmar com o EvilLink

- A estrutura de pastas do Core acima bate com a forma atual do `Fractura.Core`?
- Os nomes dos contratos: manter `IEnemyView` ou renomear o port do Core para algo mais neutro
  (ex.: o contrato de domínio é `IEnemyView`, e ele expõe só eventos abstratos — ok?).
- O `Fractura.Core` tem dependências externas hoje? (Precisa ser zero-deps para o copia-e-cola.)
- Há uma versão do `Fractura.Core` que o shard deve referenciar/copiar como base inicial?
