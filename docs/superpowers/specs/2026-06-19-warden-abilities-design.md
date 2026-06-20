# Habilidades do Warden (Guardião) — Design

*2026-06-19. Segunda vocação detalhada do rework. Fonte de verdade do sistema:
[2026-06-17-vocation-system-design.md](2026-06-17-vocation-system-design.md).
Irmã da spec do [Encantador](2026-06-19-encantador-abilities-design.md). Esta spec
só desenha o **kit de habilidades**; migração e motor de progressão pertencem ao
rework geral de vocações.*

---

## O modelo em uma frase

O Warden (skill-fonte **Nature**) tem **3 árvores honestas** — Ofensivo /
Defensivo / Controle — que são **3 playstyles de verdade**, não 3 peles de um kit.
É a vocação "fácil": diferente do Encantador, **não precisa de truque de face**.
Joga como o Marcial: 3 troncos, escolhe 1 por nível, build emerge.

| Árvore | Papel | `classe` cfg | Identidade |
|---|---|---|---|
| **Predador** | Ofensivo | Guerreiro | a caçada — armas, anti-monstro (**agnóstico de arma**) |
| **Naturalista** | Defensivo / sustain | Sábio | comunga com o ambiente — cura, forma, summon-salvaguarda |
| **Armadilheiro** | Controle | Ladino | armadilhas, redes, zoneamento, subterfúgio |

---

## Fundamentos

- **Fonte = Nature** (trava as 5 fontes em ≤1, princípio do doc).
- **Predador é agnóstico de arma.** A caça (marcar/rastrear/+dano no marcado) vale
  igual melee ou arco — **não compete** com o Arqueiro do Marcial, que é sobre *o
  tiro* (técnica de arma); o Predador é sobre *a presa*. Sem faces por arma.
- **O shift é opt-in.** Transformar-se é a habilidade tier-1 do Naturalista (e só
  no dialeto Druida). Quem não quer a vibe drúidica nunca se transforma.
- **Conjurar ≠ comandar** (princípio #9 do doc). O summon do Naturalista é
  **transiente** (salvaguarda que aparece, tanka e some) — não um proxy leal. O
  proxy leal é exclusividade do Encantador.

---

## Recurso — `stam + delay` + textura leve por tronco

*Menos é mais.* Sem medidor global, sem barra de regência. As Ativas custam
`stam + delay` (padrão do `habilidades.cfg`); Passivas são de graça. Cada tronco
tem uma **textura mecânica leve** (não é inventário, princípio #7):

- **Predador** — *Marca de Caça* no alvo (acumula benefício contra a presa).
- **Naturalista** — a *Forma/dialeto* é uma **stance** com duração.
- **Armadilheiro** — *limite de armadilhas plantadas* simultâneas.

Uma eventual **barra de combate universal** (vibe roguelike, igual pra todos) está
parkada como ideia de sistema; se vier, encaixa por cima sem retrabalho.

---

## Progressão — escolher 1 das 3 por tier

Idêntico ao Encantador: pick em cada breakpoint **1 / 3 / 5 / 7 / 10**; o tier é
fixado pelo nível, o jogador escolhe **a árvore**. 5 habilidades ao chegar no 10,
na sequência de troncos que escolheu. Habilidades **autocontidas** (sem
`requisito` cruzando tiers) — é o que permite o build emergente.

---

## As 15 habilidades

### 🗡️ Predador — *Ofensivo · a caçada* (agnóstico de arma)

Mina o **Aklat'tur** (hoje caçador de monstros Gulthrak-only) e o generaliza.

| Tier | Tipo | Mecânica única | Origem |
|---|---|---|---|
| 1 | Ativa | **Marca da Presa**: marca o alvo; contra ele +dano, ignora parte da armadura, e você o rastreia (textura *Marca de Caça*) | *Marca da Presa* |
| 3 | Ativa (modo) | **Sangue por Sangue**: % do dano causado vira cura por X seg | *Sangue por Sangue* |
| 5 | Passiva | **Instinto Selvagem**: +dano anti-monstro; em HP baixo entra em ira (escala Nature) | *Fúria do Deserto* + *Resistência do Deserto* |
| 7 | Ativa | **Cleave da Caça**: seus ataques atingem inimigos próximos por X seg | *Fúria do Deserto* (cleave) |
| 10 | Ativa · capstone | **Carnificina**: cada abate renova suas habilidades de caça — "continue matando" | *Carnificina* |

### 🌿 Naturalista — *Defensivo / sustain* (dialeto travado pela cultura)

**É UMA Natureza só.** O núcleo são os **5 papéis** (a gramática); a cultura é o
**dialeto** (o vocabulário) — *qual ambiente é o seu*, travado na criação. Druida
e Xamã já vêm ~80% dos kits existentes (nomes em itálico); **Esporo é greenfield
(TBD-lore)**.

| Papel (núcleo) | Druida (superfície / fera) | Xamã (deserto / ancestral·sangue) | Esporo (subterrâneo / fungo) — *TBD-lore* |
|---|---|---|---|
| **N1 — forma/presença** (stance) | shapeshift em fera *(Elo Natural)* | ergue totem ancestral *(Colheita de Almas)* | floresce nuvem de esporos |
| **N3 — cura** | regeneração / seiva | lifesteal de sangue *(Sede de Sangue)* | cura por simbiose / decomposição |
| **N5 — salvaguarda** (summon transiente) | espírito da mata *(Convocação Selvagem)* | ancestral guerreiro *(Ancestralidade Elemental)* | enxame subterrâneo |
| **N7 — sustain** (passiva) | comunhão com a mata *(Instinto de Sobrevivência)* | resiliência do deserto | resistência fúngica |
| **N10 — avatar** (capstone) | forma primordial *(Chamado da forma primordial)* | avatar totêmico *(Arena de Sangue / Ancestralidade)* | mente-colônia |

Notas de dialeto:
- **Druida** é o único que **shifta**. Xamã e Esporo expressam o mesmo papel sem
  transformação (totem; floração) — shiftar nunca foi o núcleo, comungar com o
  ambiente é.
- **Xamã** tem a textura leve de **almas** (coleta em abates, gasta na
  assinatura) — stack por tronco, não barra global.
- **Esporo** fica **TBD-lore**: o subterrâneo encosta no tema drow/sombras
  (Velkrath); merece passe de lore próprio antes de fixar.

### 🪤 Armadilheiro — *Controle* (greenfield — não há kit hoje)

| Tier | Tipo | Mecânica única |
|---|---|---|
| 1 | Ativa | **Armadilha**: planta; **root + dano** ao ser pisada (sujeita ao limite de plantadas) |
| 3 | Ativa | **Rede**: imobiliza / slow o alvo à distância |
| 5 | Passiva | **Armadilhas Ocultas**: suas armadilhas ficam camufladas + camuflagem própria na mata (sinergia Stealth) |
| 7 | Ativa | **Zona de Caça**: área de atoleiro / espinhos — lentidão + dano (area denial) |
| 10 | Ativa · capstone | **Cerco**: prende pesado + impede fuga / cura / teleporte (combo enabler) |

---

## Cultura — dialeto travado + afinidade de casa (opcional)

- **Dialeto travado na criação**, igual à regência do Encantador. Não é
  ambiente-reativo (virar esporo na caverna seria swingy de balancear e mataria a
  identidade). É **herança**: como você foi criado, não onde você pisa.
- **Predador e Armadilheiro são combate auto-treinado → abertos a todos**, cultura
  só pinta assinatura/estética (orc = caçador icônico; pequilho = armadilheiro
  icônico). Só o **dialeto do Naturalista** é preso à cultura (magia enraizada na
  terra, princípio #5).
- **Gancho parkado — afinidade de casa:** bônus leve quando o Naturalista age no
  seu ambiente nativo (druida na mata, xamã no deserto). Flavor, não swap de kit.
  Pega ou não.

---

## Fronteiras com outras vocações

- **Predador vs Arqueiro (Marcial):** Predador = *a presa* (agnóstico de arma);
  Arqueiro = *o tiro* (técnica de arma). Sem sobreposição.
- **Esporo vs Cientista (Alquimista):** Cientista = toxina **fabricada**
  (química, frascos, Alchemy). Esporo = simbiose **viva** (contágio que se
  espalha, esporo que controla a mente, cura por decomposição). Biologia, não
  frasco.
- **Naturalista vs Encantador (conjurar ≠ comandar):** o summon do Naturalista é
  transiente e some; o Encantador comanda 1 proxy leal e persistente.
- **Ideia transferível:** o "agnóstico de arma" do Predador também pode servir ao
  **Combatente** (controle do Marcial) — avaliar quando chegar lá.

---

## Pendências (fora do escopo desta spec)

1. **Esporo (dialeto subterrâneo)** — precisa de passe de lore (tema drow/sombras
   de Haroloth/Velkrath) antes de fixar mecânica.
2. **Números de balanceamento** — dano, durações, custos, escalas com Nature.
3. **Textura de almas do Xamã** — confirmar como stack leve (não barra).
4. **Migração** de personagens (Aklat'tur / Druida / Xamã atuais → Warden).
5. **Motor de progressão** (pick 1 de 3 por tier) — pertence ao rework geral.
6. **Afinidade de casa** — decidir se entra como flavor ou fica fora.
