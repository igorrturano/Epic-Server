# Habilidades do Alquimista (Ciência) — Design

*2026-06-21. Sétima e última vocação do rework — fecha o sistema de ponta a ponta.
Fonte de verdade: [2026-06-17-vocation-system-design.md](2026-06-17-vocation-system-design.md).
Irmã das specs de Encantador, Warden, Devoto, Ladino, Marcial e Arcanista. Esta
spec desenha o **kit**; migração e motor de progressão pertencem ao rework geral.*

---

## O modelo em uma frase

O Alquimista (skill-fonte **Science**) é o mestre da matéria — **fora da
Radiância** no espírito (razão, não canalização), mas Science é uma das 5 fontes e
trava as outras. 3 troncos honestos Off/Def/Controle, reorganizados dos kits que
já existem; escolhe 1 por nível, build emerge.

| Árvore | Papel | Identidade | Textura | `classe` cfg |
|---|---|---|---|---|
| **Vivisseccionista** | Ofensivo | toxinas fabricadas + cirurgia/sangramento | veneno aplicado | Ladino→Science |
| **Mutante** | Defensivo | auto-aprimoramento químico | **Vícios** (medidor acumula/consome) | Guerreiro→Science |
| **Sapador** | Controle | explosivos / engenhocas / zoneamento | **config de explosivo + cap de minas** | Sabio→Science |

---

## Fundamentos

- **Special ability da classe: Engenhoca** — concedida na criação (hotbar, não pick
  de árvore): o toolkit de Ciência aplicada (poções/concocções + dispositivos).
  *(naming a confirmar — ver Pendências.)*
- **Fonte = Science.** Trava as 5 fontes em ≤1. Lore: dissecar/explorar os
  subprodutos materiais da Radiância pela razão (legado racional anão).
- **Recurso = `stam + delay` + textura por tronco** (Vícios no Mutante; config +
  cap de minas no Sapador). Sem barra global.

---

## Progressão

Pick em cada breakpoint **1 / 3 / 5 / 7 / 10**; escolhe a árvore, tier fixado pelo
nível; habilidades autocontidas. Build emerge.

---

## As 3 árvores (reorganizadas dos kits existentes)

### ⚗️ Vivisseccionista — Ofensivo (toxinas + cirurgia)
Mestre dos Venenos (aplica todos os venenos em comida/arma sem danificar) · Golpe
Sangrento (sangramento cirúrgico) · Veneno Puro (potencializa duração/dosagem/
paralisia) · Precisão Cirúrgica (golpes mais precisos) · Mumificação (imune a
crit e veneno). → mestre da **toxina fabricada** e da lâmina cirúrgica.

### 🧪 Mutante — Defensivo (auto-aprimoramento químico)
Textura = **Vícios** (3 vícios que acumulam e são consumidos).
Hiperestímulo (coquetel: consome Vícios, bônus escala com Cap−Vício) · Blindagem
Química (sobe o Cap; metade do dano com vida baixa) · Síntese Veloz (Vícios
acumulam 3× mais rápido; poções mais fortes) · Mutagênicos (+25 num atributo +
vital) · Sobredose (imune a todo controle e crit por um tempo, depois Letargia).

### 💣 Sapador — Controle (explosivos / engenhocas)
Textura = **configuração de explosivo + cap de minas** (já reworkado).
Engenharia de Gatilhos (configura o explosivo: gatilho/elemento/invólucro/carga/
reforço) · Zona de Pressão (marca área que modifica o próximo explosivo) · Campo
Minado (minas em cadeia + detonação remota) · Torreta Granadeira (torreta que
arremessa explosivo) · Protocolo Mestre (buff dos explosivos; aliados imunes; reset
de CDs). → controle de área por engenhoca.

---

## Fronteiras com outras vocações

- **Veneno — 3 sabores, sem sobreposição:**
  - **Alchemy cívica** = veneno básico fabricado, aberto a todos (ex.: Assassino).
  - **Vivisseccionista (Science)** = *mestria* de toxina fabricada + cirurgia.
  - **Esporo (Nature)** = simbiose viva / contágio (biologia, não frasco).
- **Golem ≠ árvore.** É **feature cultural Badûran** (só anão forja/pilota a
  power-armor a vapor; Blacksmithing constrói; calor mora na manopla). O Sapador
  (torretas/engenhocas) é o vizinho temático, mas golem é cultural, não classe.
- **Sapador vs Marcial-arqueiro etc.:** o Sapador controla por *dispositivo*
  (engenhoca), não por arma; é a fonte Science que dá o kit.

---

## Pendências (fora do escopo desta spec)

1. **Naming da special ability** — "Engenhoca" puxa pro lado Sapador; talvez um
   nome que cubra também poções/química (ex.: "Laboratório" / "Química Aplicada").
2. **Respeitar o rework do Granadeiro em andamento** (`project_granadier_rework`:
   nomes EN + IDs PT, Engatilhar dropado, self-resist→Campo Minado) — não duplicar
   nem reverter; esta spec só reposiciona o Granadeiro como tronco Sapador.
3. **Limite veneno cívico × Vivisseccionista** — definir onde a mestria começa.
4. **Números** e **migração** (Vivisseccionista/Mutante/Granadeiro atuais; o
   Vivisseccionista sai da tag Ladino para Science).
