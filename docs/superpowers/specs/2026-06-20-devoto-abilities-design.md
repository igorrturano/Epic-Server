# Habilidades do Devoto (Fé) — Design

*2026-06-20. Terceira vocação detalhada do rework. Fonte de verdade do sistema:
[2026-06-17-vocation-system-design.md](2026-06-17-vocation-system-design.md).
Irmã das specs do [Encantador](2026-06-19-encantador-abilities-design.md) e do
[Warden](2026-06-19-warden-abilities-design.md). Esta spec desenha o **kit**;
migração e motor de progressão pertencem ao rework geral.*

---

## O modelo em uma frase

O Devoto (skill-fonte **Theology**) é uma vocação mágica de **dois andares**:

1. **3 árvores honestas** — Ofensivo / Defensivo / Suporte — reorganizadas dos
   kits que já existem (Demon Hunter / Paladino / Cleric). Escolhe 1 por nível,
   build emerge. *(igual Warden)*
2. **Uma spellbook divina unificada**, graduada por **Casting**, que **todo
   Devoto tem inteira** (não trava magia por habilidade — isso confunde).

A **religião** pinta a face de tudo (como a cultura faz no Warden).

| Árvore | Papel | Kit de origem | `classe` cfg | Armadura natural |
|---|---|---|---|---|
| **Demon Hunter** | Ofensivo | DH (anti-demônio, móvel) | Ladino | leve |
| **Paladino** | Defensivo | Paladino (tank, protege party) | Guerreiro | pesada |
| **Cleric** | Suporte / Controle | Cleric (zona, sustain) | Sábio | média |

---

## Fundamentos (a fundação travada)

- **Fonte = Theology.** Trava as 5 fontes em ≤1.
- **Casting gradua o círculo.** É a skill de *acertar* da magia (ex-`Magery` —
  **renomear pra `Casting`** é desejável, é neutro de fonte; rename é refactor
  grande, fica pra tarefa dedicada). Casting baixa = círculos baixos; Casting
  cheia = a lista toda.
- **Paladino ↔ Cleric é espectro, não duas classes.** Onde você cai depende de
  quanto Casting investe. O **custo de skill (MAD) É o balanceamento**: curar/
  conjurar forte rouba de STR/DEX/arma. Frontline gish (Casting secundária) num
  extremo, conjurador de backline (Casting primária) no outro.
- **Penalty de armadura é da FONTE, não da classe** (lógica D&D): **Arcana paga**
  o spell-failure de armadura (mantém o Arcanista honesto); **Theology não paga**
  — castar de armadura é a identidade do sacerdote-de-guerra. O Devoto **não
  compra** habilidade pra isso; vem de ser divino. *(Sem imposto no Paladino, que
  já é kit caro; e pagar imposto pelo óbvio seria tarefa, não decisão.)*
- **O único freio de armadura é na cura:** **armadura pesada reduz eficiência de
  cura** (auto-cura / cura recebida). Mata o "tanque-imortal de plate + auto-heal"
  sem matar o "divino casta de armadura". O MAD sozinho deixaria de pé um tanque
  inkillável-mas-inofensivo (trava objetivo); este knob fecha isso.
- **Revive = círculo alto** na spellbook. O espectro trava sozinho: só quem
  investe pesado em Casting alcança (um Paladino só revive se virou praticamente
  Cleric).
- **Devoto criado na criação nunca paga penalty de vocação** (é conjurador divino
  desde o nível 1). Esse penalty é outro, separado do de armadura.

---

## Recurso

`stam + delay` nas habilidades de árvore (padrão do `habilidades.cfg`); as
magias da spellbook custam **mana + delay** e são graduadas por **círculo de
Casting**. Sem medidor próprio. *(Barra de combate universal segue parkada.)*

---

## Progressão

Pick em cada breakpoint **1 / 3 / 5 / 7 / 10**; escolhe a árvore, tier fixado
pelo nível; habilidades autocontidas (sem `requisito` cruzando tiers). **A
spellbook é à parte** — todo Devoto a tem inteira, limitada só pelo círculo de
Casting. Os picks de árvore são as habilidades *não-mágicas* (técnicas, auras,
passivas, comandos); a magia de círculo mora na spellbook.

---

## As 3 árvores (reorganizadas dos kits existentes)

Alto reaproveitamento: cada kit atual vira uma árvore. Mudanças vs hoje: **somem
os locks de armadura por classe** (armadura é aberta + penalty é por fonte) e o
**"conjurar de armadura X" deixa de ser unlock** (Theology já casta de armadura).

### ⚔️ Demon Hunter — Ofensivo (anti-demônio, móvel)
Sigilo de Proteção (passiva: +vontade, vantagem vs demônios) · Sentença Profana
(sacrifica HP → golpe forte, +dano vs demônio) · Mancha da Corrupção (passiva:
sente demônios) · Armadilha Demoníaca (pentagrama AoE, prende demônio/morto) ·
Forma Abissal (asas: voo fora de combate + stats em batalha).

### 🛡️ Paladino — Defensivo (tank, protege a party)
Consagração Divina (passiva: +vontade, vantagem vs mortos — *sem o lock de
"armadura pesada", que virou regra de fonte*) · Golpe Divino (dano divino +
efeito por religião) · Aura de Fé (comandos de buff de party) · Presença Divina
(imune a medo + buffs de party) · Campeão Divino (stats + dano de retaliação).

### ✨ Cleric — Suporte / Controle (zona, sustain)
Conexão Divina (manifestação por religião) · Golpe Radiante (ataque divino) ·
Aura Divina (imune a doença/medo, resist veneno) · Autoridade Divina (terreno
consagrado: buff aliado / debuff inimigo) · Apoteose (capstone: magias divinas
sem mana/delay; poder supremo por religião).

---

## Religião = face (domínios)

Todo kit já diz *"cada religião manifesta efeitos únicos"* — é o mesmo padrão do
dialeto do Warden. A **religião pinta a face** de habilidades e magias, e é a
**especialização** ("Domínios divinos" do doc-fonte). Mapear as religiões da lore
(`epic-lore/religioes.md`, `deuses-aliens.md`: Veneratio, Bokk Bharaz, Halleuad,
Eluith'orth, Kai…) aos domínios fica como passe próprio (ver Pendências).

---

## Fronteiras com outras vocações

- **Arcanista vs Devoto:** Arcana paga penalty de armadura (arcano precisa de
  gesto livre); Theology não (fé funciona de armadura). É o que diferencia caster
  arcano de divino.
- **Devoto vs Encantador (Profana):** o Devoto recebe poder de Aghar/protetores
  (conjura); o Encantador Profana comanda 1 morto leal (conjurar ≠ comandar,
  princípio #9).

---

## Ganchos futuros (não implementar agora)

- **Morte & Ressurreição Divina (stakes + religião + entropia).** A morte deixa
  de ser game-over e vira **drama** (pilar de stakes do MDA):
  - **Death Points (DP):** o jogador tem ~5; morrer (PvP ou PvM numa dungeon)
    gasta DP.
  - **Matar player custa entropia ao assassino** — custo cósmico do assassinato;
    conversa com a Regência↔entropia já anotada no systems-map.
  - **Em vez de morte definitiva, uma quest de ressurreição:** quando os DP
    acabam, o **Deus do jogador reacende sua "faísca"** ("preciso de você") e
    devolve um ponto — *deus ex machina* divino, conquistar a vida de novo. Dá
    **peso real à religião** (central no Devoto) e é **ferramenta, não roteiro**
    se for sistêmico. Spec própria quando maduro.

---

## Pendências (fora do escopo desta spec)

1. **Curadoria da spellbook** — ela é "várias e muito fortes" hoje; definir
   círculos, potência, custo (revive em especial). É onde mora o balanceamento.
2. **Mapear religiões → domínios** (especialização cultural divina).
3. **Knob cura×armadura** — tunar quanto a armadura pesada reduz eficiência de
   cura.
4. **Rename `Magery` → `Casting`** — refactor mecânico grande (cfg/código/UI).
5. **Refinar princípio #4 do doc-fonte** — some a "listinha pequena que escala na
   fonte sem Magery"; vira spellbook única graduada por Casting (vale checar se
   generaliza pros outros híbridos, ex. Lâmina Arcana).
6. **Spell vs habilidade de árvore** — confirmar a fronteira (alguns ativos dos
   kits, tipo Golpe Divino/Radiante, são técnica de árvore, não magia de círculo).
7. **Números de balanceamento** e **migração** (Paladino/DH/Cleric atuais).
