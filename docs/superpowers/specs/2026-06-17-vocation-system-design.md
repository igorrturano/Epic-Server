# Sistema de Vocações — Design

*2026-06-17. Documento vivo. Evolui o catálogo de 30 classes do
`docs/path-rework-specs/`. Escrito pra ler de cima a baixo e pensar em cima.*

## O modelo em uma frase

Você escolhe **1 vocação** (a fonte do seu poder). Estilo de arma é **skill**,
não classe. Cada vocação tem **3 árvores de habilidade**; a cada nível você
**escolhe 1 das 3** e seu build emerge. A **cultura pinta o flavor**, sem
engaiolar a escolha de combate. São **7 vocações** — o picker mostra 7, fim.

---

## Vocabulário

- **Caminho** = Militar / Civil (o `path` que já existe).
- **Vocação** = a "classe": a fonte de poder. São 7. É o que se escolhe na
  criação. Nome funcional → **nome oficial (classe)** + skill-fonte:
  - Marcial → **Guerreiro** (Tactics)
  - Furtivo → **Ladino** (Stealth)
  - Natureza → **Guardião** (Nature)
  - Fé → **Devoto** (Theology)
  - Arcana → **Arcanista** (Magery + Arcana)
  - Ciência → **Alquimista** (Science)
  - Comando → **Encantador** (skill **Regência** — controla feras/armas/mortos;
    evita "comandar", que confunde com liderança)
- **Árvore** = as 3 trilhas de habilidade de cada vocação. Escolhe 1 por nível.
- **Expressão** = a "mão" (como você acerta): arma-melee / arco / desarmado /
  conjuração / comando. Vem da arma/skill que você empunha.
- **Lean / build** = o personagem que emerge das suas escolhas de árvore.

---

## O problema que isso resolve

"30 classes" afoga o jogador porque "classe" empilhava **três perguntas**:
COMO você luta (arma), DE ONDE vem seu poder (fonte), DE QUAL povo (cultura).
Fundidas numa lista chapada = explosão combinatória. A saída: **separar os
eixos e deixá-los compor.**

---

## Princípios (as regras do sistema)

1. **Vocação = fonte de poder. Estilo de arma = skill.** "Naturalista que usa
   machado 2H" é o Predador — sem classe pra isso.

2. **3 árvores por vocação; escolhe 1 por nível; build emerge.** O *tema* das 3
   varia por vocação (às vezes Ofensivo/Defensivo/Controle, às vezes papel) —
   caminhos são orgânicos, não um trio fixo forçado.

3. **A arma detecta a FACE da habilidade.** Uma habilidade, várias faces:
   Ofensivo melee = **Fúria**; Ofensivo arco = **Tiro Poderoso**; Ofensivo
   desarmado = **flurry de ki**. Mata o absurdo "fúria + arco". Mesmo princípio
   do `if arco → kit ranged; else → melee`.

4. **2 skills-núcleo: Acertar + Escalar.**
   | | Acertar | Escalar |
   | --- | --- | --- |
   | Arma | Wrestling / 1H / 2H / Dual / Ranged | **Tactics** |
   | Magia | **Magery** | Arcana / Theology / Nature / Science |
   - **Caster pleno** (magia é a identidade): 2 skills mágicas (Magery+Arcana), sem arma.
   - **Híbrido** (magia apoia arma): 1 fonte + 1 arma; lista pequena escala na própria fonte.
   - **MAD** (Paladino: STR+DEX+mana) paga em **atributo espalhado**, nunca 3ª skill.
   - **SAD** (Mago) paga em **fragilidade** (HP die, armadura travada).
   - Magery aparece só quando conjurar é o trabalho inteiro (lista grande). É a
     skill de "acertar" da magia (toda magia acerta; a dificuldade é castar).

5. **Cultura = flavor separado, nunca engaiola combate.** Fúria é de quem
   *escolhe* inclinar Ofensivo — não de quem nasce nórdico. Um gladiador
   aiglano pode entrar em fúria; um Björske pode ser cavaleiro. A cultura dá
   trait/estética/uma assinatura, e **liberta a lore** (Björske cavaleiro OU
   bárbaro: mecanicamente tanto faz).
   - **Linha:** o que é *enraizado no ambiente* a cultura determina (o
     Naturalista aprende magia da sua terra → druida/xamã/esporo por cultura).
     O que é *auto-treinado* é escolha (combate). Regra: cultura dá opção/sabor,
     nunca restringe escolha de combate.

6. **Especialização = árvore/habilidade, nunca classe.** Escolas de magia,
   tradições de natureza, domínios de fé: no level-up você troca uma habilidade
   genérica por uma de especialização e ganha os bônus. Assim o Naturalista vira
   druida/xamã/esporo pela cultura, e o Magus escolhe sua escola
   (Evocação/Ilusão/Necromancia/Trapaceiro), sem virar classes separadas. A
   fantasia vive, o slot morre.

7. **Recurso é mecânica, não inventário.** Habilidade nunca se prende a pólvora/
   poção — vira tarefa, não gameplay. O recurso é um **medidor enchido agindo**:
   calor (golem), ki (monge), sangue (xamã), combo, carga, cooldown.

8. **Trava generativa: ≤7 vocações × ~3 árvores.** Ideia nova cabe numa célula
   ou é flavor (habilidade/magia dentro de uma árvore) — não classe.

9. **Conjurar ≠ comandar.** Qualquer fonte mágica (Arcana/Theology/Nature/…)
   pode **conjurar, transformar ou explodir** — invocações transientes, mágicas,
   regidas pela IA, que expiram. Só a **Regência** **comanda** um proxy leal, que
   escala com você e persiste. O minion-master é exclusividade do Encantador;
   toda outra fonte só ganha a versão temporária. Resolve Necro↔Magus e
   Druida↔Caçador de Feras: o Druida *invoca/vira* fera mas não a comanda; o
   Magus *ergue/estraçalha* mortos mas não os rege.

---

## Modelo de skills (3 camadas)

A skill responde "o que sei fazer"; a vocação responde "qual poder me move".
São eixos separados — é o que mata os combos OP sem engaiolar profissões.

1. **Fontes de poder — trava mútua, escolhe ≤1.** Na lore são *relações
   diferentes com a mesma Radiância*: você canaliza UMA, nunca duas. São as 5
   skills-fonte:

   > **Arcana · Theology · Nature · Science · Regência**

   Pegar Regência anula Arcana/Theology/Nature/Science, e vice-versa. É o
   anti-OP central (sem necro-druida, sem paladino-mago). `Magery` é a skill de
   *acertar* da magia e anda junto da Arcana no caster pleno (princípio 4).

2. **Skills cruas — abertas a todos.** Stat bruto, qualquer vocação treina o
   baseline: armas (Wrestling / 1H / 2H / Dual / Ranged / Parry), **Tactics**
   (escala dano de arma), **Stealth** (posicionamento) e **toda a coluna
   Civic**. *Tactics e Stealth NÃO são fontes* — Marcial e Furtivo ficam fora
   da Radiância. Por isso o Assassino pega Stealth + Tactics + arma + veneno
   (Alchemy cívica) sem nunca tocar numa fonte mágica.

3. **Árvores de habilidade — travadas pela sua 1 vocação.** É aqui que
   Guerreiro ≠ Ladino mesmo ambos usando Tactics: o Guerreiro destrava Fúria, o
   Ladino destrava o burst de assassinato. Um Mago pode treinar Stealth pra se
   esgueirar, mas nunca ganha a árvore do Assassino.

> Resumo: **skill = músculo bruto (aberto)** · **árvore = o que a vocação ensina
> (travado)** · **fonte = qual Radiância você canaliza (≤1)**.

**As skills (sketch, 2 colunas):**
- **Military — fontes (≤1):** Arcana · Theology · Nature · Science · **Regência**
  *(+ Magery, o "acertar" da magia)*
- **Military — abertas:** Wrestling · One_Handed · Two_Handed · Dual_Wield ·
  Parry · Ranged · **Tactics** · **Stealth**
- **Civic — todas abertas:** Smithing · Carpentry · Tailoring · Provisioning ·
  Alchemy · Mining · Lumberjacking · Survival · Taming · Medicine · Mechanics ·
  Perception · Theft

Correções pendentes no sketch: incluir `Nature` na coluna Military (hoje falta,
e o Guardião depende dela); renomear `Beastmastery` → `Regência`.

---

## As 7 vocações (e o destino das 30 antigas)

*(6 detalhadas abaixo; a 7ª — Encantador — tem seção própria logo após.)*

### 🗡️ Marcial — *Tactics + arma, sem magia*
- **Árvores:** Ofensivo (**Bárbaro** / Fúria) · Defensivo (**Tanque** / guarda
  proativa: Interceptar, medidor de Guarda → Escudada, Muralha, Provocar
  grudento) · Controle (**Combatente** / Half-sword, desarme)
- **Expressões:** melee · arco (**Arqueiro**) · desarmado (**Monge** / Wrestling+ki)
- **Dissolve:** Samurai → habilidade cultural Leste/Dragão · Esgrimista → vai
  pro **Furtivo** (duelista de finesse) · Arqueiro → expressão arco · tank →
  lean Defensivo.

### 🌑 Furtivo — *Stealth (furtividade mundana)*
- **Árvores:** Ofensivo (**Assassino** / burst+bleed) · Controle (**Shinobi** /
  pressão+CC) · finesse/duelo (**Esgrimista**, DEX) *(mapa exato TBD)*
- **Dissolve:** Ladrão → skill cívica Theft · Caçador de Tesouros →
  Perception/Survival · Bardo → Encantador · Dançarino das Sombras → Arcana
  (a furtividade de sombra é arcana, não mundana).

### 🌿 Natureza — *Nature*
- **Árvores:** **Predador** (armas — caçador de monstros) · **Armadilheiro**
  (subterfúgio/armadilhas) · **Naturalista** (conjuração)
- **Naturalista, especialização por cultura:** Druida (superfície) · Xamã
  (deserto/Gulthrak) · Esporo (subterrâneo/Haroloth). Núcleo compartilhado +
  assinatura cultural.
- O **Caçador/Mestre de Feras NÃO fica aqui** — vai pra vocação Encantador.

### ✝️ Fé — *Theology*
- **Árvores:** **Paladino** (arma) · **Caçador de Demônios** (furtivo) ·
  **Cleric** (conjuração)
- Domínios divinos = especialização (TBD).

### 🔮 Arcana — *Magery + Arcana*
- **Árvores:** **Lâmina Arcana** (arma — melee **e** arco via weapon-detect;
  absorve o Arqueiro Arcano) · **Dançarino das Sombras** (subterfúgio — já usa
  Arcana; o ladino de sombra) · **Magus** (conjuração)
- **4 escolas (especialização do Magus), confirmadas em config:**
  **Evocação** (dano elemental — a que vingou) · **Ilusão**/Velkyn (engano/
  controle) · **Necromancia** (morte/dreno; magias de *invocar mortos* vão pro
  Encantador, *dreno/maldição* ficam na escola) · **Trapaceiro**/Trickster
  (utilidade ardilosa: fechaduras, decifrar, disparo místico — herda o sabor do
  Malandro Arcano dissolvido).
- **Dissolve:** Deslocador das Sombras → deletado (era o Dançarino duplicado) ·
  Arqueiro Arcano → build de arco da Lâmina · Necromante → **vocação Encantador** ·
  Malandro Arcano → dissolvido (scroll quebrado; sabor "trapaceiro arcano" vira
  habilidade do Dançarino).

### ⚗️ Ciência — *Science*
- **Árvores:** Ofensivo (**Vivisseccionista**) · Defensivo (**Mutante**) ·
  Controle (**Sapador**)
- **Sapador:** constrói engenhocas por **medidor de engenharia acumulado
  agindo**, não consumindo poções.
- **Golem / Engenheiro → sai do sistema de classes:** vira **feature cultural
  Badûran** (só anão forja e pilota a power-armor a vapor; Blacksmithing
  constrói; calor mora na manopla). Tira uma classe pesada e impopular sem
  matar a fantasia.

---

## 🎭 Encantador — a 7ª vocação (ex-Comando) *(skill-fonte: `Regência`, ex-`Beastmastery`)*

A ideia é **reger o sem-mente** — você é a inteligência/direção, seus proxies
lutam por você. Você **escolhe a regência na criação e fica nela** (como o
Drizzt: um ranger, uma pantera — foco, não canivete suíço). As 3 regências
**são** as 3 árvores da vocação:

- **Vocal** → rege feras (**Caçador / Mestre de Feras**, companheiro vivo)
- **Musical** → rege armas (**Bardo**: toca e a arma luta sozinha)
- **Profana/Mental** → rege mortos (**Necromante**)

Regras do sistema:
- **O kit pertence à regência, não ao minion.** São 3 kits no total — as mesmas
  habilidades independem do tipo de proxy (lobo ou urso; esqueleto ou zumbi). O
  proxy é troca de stats/flavor; sua *força* escala com o nível de Regência.
- **1 proxy por vez** (2ª trava anti-OP, além de Regência ser fonte ≤1).
- **Aquisição por regência:** Vocal doma com **Taming** (cívica) · Musical
  acha/compra/nasce com a arma · Profana precisa de um **corpo** — espírito
  (bicho morto) → esqueleto → zumbi conforme o nível de Regência.
- **Regência só rege NPCs, NUNCA Players.**

**Taming** segue **cívica** (montaria/utilidade + aquisição do companheiro
Vocal), **sem scaling de combate e sem exército** — mata o absurdo dos "5 bichos
cinzas" que morrem por você. O "necrobardo" vira só sabor de lore possível,
nunca combo mecânico (Regência trava em 1 e o proxy é 1 por vez).

---

## Lore das Fontes (amarrada à cosmologia)

Quase todo poder de Eora flui da **Radiância** (energia liberada na queda de
Alora, domada no evento das Árvores Sagradas). As fontes mágicas são **relações
diferentes com ela**. Cosmologia completa em `epic-lore/deuses-aliens.md`.

- **Arcana = canalizar a Radiância crua.** As 4 escolas (Evocação/Ilusão/
  Necromancia/Trapaceiro) são **interpretações culturais** do poder de Alora.
- **Fé = receber poder de Aghar** (guardião de Alora) e dos protetores. Aghar é
  a **origem das magias divinas**; os domínios da Fé mapeiam as religiões
  (Domínio de Aghar, de Alora, da Vigília…).
- **Natureza = comungar com a reação viva da Radiância** (mundo mutado: ogros,
  ettins). **Berço do druidismo = os bragoneses** (Vigília, reverência ao mundo
  selvagem); xamã (deserto) e esporo (subterrâneo) são adaptações regionais.
- **Ciência = dissecar e explorar os subprodutos materiais** da Radiância
  (mutação, alquimia, máquina) pela razão — legado do caminho racional anão.
  *(proposta)*
- **Ambição** (Cavaleiro Morte): minério/subterrâneo; anões (Deus-Máquina) e
  drows (Illyra). Lore do Golem-Piloto Badûran.
- **Marcial / Furtivo** = sem fonte mágica; ficam de fora da Radiância.
- **Encantador (Regência)** = arte aprendida de impor vontade ao sem-mente;
  mais disciplina que fonte cósmica. *(a refinar)*

> Through-line: **canalizar** (Arcana) · **receber** (Fé) · **comungar**
> (Natureza) · **dissecar** (Ciência). Quatro relações com a mesma Radiância.

---

## Piloto "começar menor"

Vocação **Natureza**, árvore **Naturalista**: fundir Druida + Xamã + (novo)
Esporo numa coisa só, resolvida por cultura no `applyClassStarter`
(`if gulthrak → xamã; if haroloth → esporo; else → druida`). Prova o framework
inteiro num corte vertical pequeno.

---

## Estacionado / pontas soltas

- Furtivo: confirmar mapa off/def/controle (Assassino/Shinobi/Esgrimista).
- **Encantador:** o proxy mantém ataques inatos (aranha empoçona, urso não) ou
  todo proxy do mesmo tipo bate igual? Decidir na implementação da 1ª regência.
- Fé: detalhar domínios + lore dos deuses-aliens.
- **Migração** de personagens existentes — pedra de migração, `path-rework/07`.
- Lore do Björske (cavaleiro vs bárbaro) — agora é só sabor, não trava mecânica.
