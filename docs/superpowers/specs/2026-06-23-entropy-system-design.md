# Entropia — O Pêndulo de Ordem e Caos (Visão de Core Loop)

*2026-06-23. **Doc de visão de game design**, não spec de implementação. Define o
game core loop da entropia como o eixo Ordem × Caos do mundo de Epic. Substitui e
promove a semente de brainstorm
([2026-06-23-entropy-system-brainstorm-seed.md](2026-06-23-entropy-system-brainstorm-seed.md))
a fonte de verdade. Decisões fechadas via brainstorm dirigido; números de
calibragem ficam marcados como "a sintonizar no playtest".*

---

## A ideia em uma frase

Entropia deixa de ser um medidor de canto e vira o **eixo narrativo do mundo** —
a tensão entre a natureza caótica da realidade e o esforço humano de construir
civilização em cima dela. O Caos não é uma facção: é o estado natural do mundo.
A Ordem não é um time: é o ato contínuo de resistir a esse estado. Alguns jogadores
resistem. Outros abraçam. E o mundo responde.

---

## 0. O que JÁ existe no código (o motor já roda)

Antes de qualquer coisa: **não estamos inventando entropia, estamos construindo o
loop de mundo por cima de um motor que já roda.** A semente subestimava o que está
implementado.

- **Entropia pessoal** (`-20000..+20000`, clampada) +
  **entropia global** (`GlobalEntropy`, **hoje sem cap** — buraco a corrigir).
  `UpdateMobileEntropy` move as duas juntas.
- **Convenção de sinal já fixada e consistente em ~10 sistemas:**
  **negativo = Caos/Corrupção, positivo = Ordem/Virtude.** (O ⚠️ da semente era
  falso alarme.) Evidência: `pkg/items/globoEntropia/globo.src` (`ENT_CAOS_*`
  negativos, `ENT_ORDEM_*` positivos),
  `pkg/systems/spells/spell_logic/priest/detectarcorrupcao.src` (rotula negativo
  como "Corrupção").
- **Efeito direto no jogador já implementado:** cura ±50%
  (`pkg/systems/attributes/include/vitals.inc`), save de veneno com alma
  corrompida ≤ −8000 (`resists.inc`), hit ± por entropia
  (`HitController.inc`), dano vs mortos (`DamageController.inc`), Vontade ±5
  (`intrinsicStats.inc`), bônus de DP por marco (`entropy.inc`).
- **Mundo → monstros já implementado:** entropia global empodera undead (−20k),
  demônio (−40k), void (−60k) em hit/dano (`HitController` / `DamageController`).
- **Voidrift** (`pkg/systems/voidrift/`): rituais que gastam entropia pra invocar
  (Chaos Ritual abre portal, corrompe a área em 8 tiles, spawna 4–20 criaturas) e
  pra banir (Purge Ritual destrói portal, devolve entropia+).
- **Tiers de descrição já codados mas desligados:** Aura Macabra → Olhos Negros →
  (neutro) → Olhos Dourados → Aura Radiante (`entropy.inc`).
- **Globo do mundo** (`globoEntropia`) já exibe o estado global.

**Conclusão:** o que falta é o **loop** (pêndulo, território, stakes,
balanceamento), não o motor de baixo nível.

---

## 1. Polaridade & lore *(fechado)*

- **Caos = entropia negativa. Ordem = entropia positiva.** Não é decisão aberta —
  o código já assume isso em todo lugar.
- **Lore:** a **Radiância / Ordem** (Alora) contra a **Legião Caída / deuses do
  Caos** — os "aliens" estilo Warhammer que caçam a Radiância no planeta. O
  voidrift é o portal mecânico do Caos.
- **Tiers de alma** (religar os que já estão no código): Aura Macabra → Olhos
  Negros → neutro → Olhos Dourados → Aura Radiante.

---

## 2. A espinha: pêndulo de mundo

O **coração do design é a barra global**; as vocações, o craft e o território são
plugues. Isso mantém o escopo focado.

### Física da barra
- A barra global tem **cap** (a corrigir) e uma **inclinação natural pro Caos** —
  sem ação, o mundo deteriora lentamente. Entropia é o estado de repouso da
  realidade; civilização é o esforço que a contraria.
- **A Ordem nunca descansa.** O mundo não "sara sozinho" — ele é segurado pela
  ação contínua dos que resistem. A deriva puxa levemente pro Caos, não pro neutro.
  O piso de repouso fica em torno de −20k a −30k (leve corrupção ambiental: undead
  presentes, noites um pouco mais longas, fronteira sempre tensa).
- **Nada vence permanentemente.** O cap garante que o mundo não afunda além do
  jogável; a resistência humana garante que não fica travado no máximo Caos. O
  conflito perpétuo é emergente.
- **A barra se move por ação — humana ou ambiental** (ver §10, estoque-e-fluxo).
  Portais abertos empurram passivamente. Estruturas da Ordem seguram ativamente.

---

## 3. Alinhamento (híbrido: ação + vocação)

- **Sem menu de facção.** A **entropia pessoal É o alinhamento**, e cada ato
  empurra o jogador e o mundo juntos (`UpdateMobileEntropy` já faz isso).
- **A vocação inclina, não trava:** o **Encantador** (reger o sem-mente) sangra
  Caos por design; o **Devoto / Caçador de Demônios** puxa Ordem. Qualquer um pode
  ir pra qualquer lado — a vocação só muda a *facilidade* de cada polo.
- **Duas relações com o mundo, não dois times:**
  - **Jogador da Ordem:** resiste à natureza do mundo. Sustenta civilização contra
    uma força maior do que qualquer indivíduo. Nunca "venceu" — está sempre lutando.
  - **Jogador do Caos:** abraçou uma verdade sobre o mundo que a maioria recusa.
    Não é um vilão de torcida que "escolheu o lado vermelho" — é alguém que entendeu
    o que o mundo *realmente é* e fez um pacto com isso. Surfa a entropia em vez de
    nadar contra ela. O mundo caótico não é o seu inimigo: é o seu habitat.

---

## 4. Recompensa não-conversível & os dois extremos como falha

A regra de ouro: **o que o Caos dá, a Ordem não compra, e vice-versa.**

| | **Polo CAOS (−)** | **Polo ORDEM (+)** |
|---|---|---|
| **Dá** | Ofensiva bruta (+hit, +dano, imune a veneno) **+ Essência do Vazio** (melhores materiais de craft, só existem com mundo corrompido) **+ exploração rende** (drops) | Sustento/resiliência (+cura, +Vontade, +DP) **+ reinos prosperam** **+ estabilidade** social/econômica |
| **Custa** | Frágil em batalha aberta; caçado em terra ordeira | Sem burst, sem loot raro/Essência |
| **Extremo = falha** | Mundo tomado por monstros empoderados (já no código) **+ pragas/terremotos nos reinos** *(gancho futuro, §13)* | **Estagnação econômica:** sem Essência, sem portais, sem craft de ponta. Seguro porém pobre — crafters/mercadores **pressionam por um caos controlado** |

Duas consequências elegantes:
- **A moeda é não-conversível:** pra forjar o item supremo, *alguém* teve que
  corromper o mundo.
- **A falha da Ordem usa a mesma economia da recompensa do Caos** (a Essência) —
  auto-equilibra sem regra extra. Os dois polos servem *tipos de jogador*
  diferentes: o Caos serve o **explorador/raider**, a Ordem serve o
  **construtor/governante**.

---

## 5. O motor de efeitos: **o mundo pune o OPOSTO, não você** *(correção central)*

O código atual está filosoficamente errado: hoje *sua* entropia negativa nerfa
*sua própria* cura — ou seja, **pune o jogador Evil por ser Evil.** Errado. O
modelo certo:

> **Sua entropia pessoal define de que TIME você é. O estado do MUNDO empodera o
> seu time e enfraquece o time inimigo.**

| Estado do mundo | Time da Ordem (Devotos etc.) | Time do Caos (corrompidos, undead) |
|---|---|---|
| **Mundo em Caos (global −)** | **Sofre:** Devotos curam menos, bênçãos fracas, magia divina enfraquecida | **Prospera:** undead/demônios empoderados (já no código), corrompidos fortes |
| **Mundo em Ordem (global +)** | **Prospera:** cura plena, bênçãos, reinos florescem | **Sofre:** magia profana fraca, lacaios fracos, herege caçado em terra hostil |

Por que isso conserta tudo:
- Ser profundamente corrompido é **ótimo** num mundo corrompido (demônio entre
  demônios) e **péssimo** num mundo ordeiro (herege caçado). Nenhum lado é punido
  por *se comprometer*.
- **Empurrar a barra vira estratégia de guerra:** corromper o mundo enfraquece
  *todos* os Devotos do servidor. Esse é o cabo-de-guerra de verdade.

**Ação concreta de calibragem:** remover o auto-debuff de cura da própria entropia
em `vitals.inc`; trocar o eixo de "minha entropia" para "entropia global × meu
time".

---

## 6. Stakes: A Regra do Batman *(o coração emocional)*

- **Não existe morte limpa.** Todo assassinato de jogador **empurra o matador pro
  Caos** — mesmo matando um caótico, mesmo "em nome da Ordem". Assassinato *é*
  entropia.
- **Caminhos assimétricos sem regra nova:**
  - **Caos = indulgência.** Matar é coerente com seu rumo; o custo de entropia te
    empurra pra onde você já ia.
  - **Ordem = contenção.** O paladino que assassina **perde as bênçãos** (a
    cura/Vontade/DP que o código já tira de entropia negativa). A disciplina do
    herói **é não matar** — ele lida com o Caos por captura, exílio, purga, banir
    o voidrift.
- **Drama emergente (Righteous Fury):** o Coringa **provoca** o Batman a matar,
  porque cada morte corrompe o justiceiro. A "fúria justa" é a queda lenta de quem
  se acha certo. Ambiguidade moral é o ponto.
- **A morte é uma jornada de volta, não respawn:** o submundo, a alma que ressurge
  — um mini-game que custa tempo/DP e tem sabor. A morte dói de verdade mas é uma
  *experiência*, não um delete (o jogador ama o personagem).
- **Detecção como gato-e-rato:** a corrupção do assassino é **detectável** pela
  magia `detectarcorrupcao`, **mas pode ser disfarçada com magia.** A marca não é
  letreiro automático — é um jogo de **ocultar × revelar**: o herege esconde a
  alma, o inquisidor fareja. Vira intriga social, não foragido-com-placa.
- **Letalidade modula com a barra:** mundo caótico = morte barata (mortos
  ressurgem, volta corrompido); mundo ordeiro = morte rara mas com ressurreição
  acessível.

---

## 7. Ponto de não-retorno: **só o Caos condena**

- Passado um **X de entropia negativa, acabou a redenção** — você mergulhou no
  Caos, é permanente. Assimétrico de propósito.
- **A Ordem é sempre revogável.** Razão de game design: um santo blindado é um
  personagem *resolvido* (sem stakes, chato); um herói **eternamente tentado**
  está sempre a uma noite ruim de cair — tensão permanente = retenção. *"Você
  nunca ascende, está sempre sendo tentado pelo Caos."*
- **Perdão finito é uma mecânica de tensão:** você peca e se redime *até não poder
  mais*. A aproximação do limiar é angustiante ("ainda dá pra voltar?"). O
  Paladino Caído é trágico porque houve um último momento em que ainda dava.
- **O limiar é um Rubicão legível e apavorante:** a alma escurece (os tiers Olhos
  Negros → Aura Macabra), NPCs recuam, a corrupção fica difícil de disfarçar.
  Atravessar é **decisão consciente**, não susto.
- **Cruzar DÁ poder, não só tira** (pacto fáustico): no não-retorno você consuma o
  pacto com o deus do Caos e ganha poder real — afinidade com o Vazio, habilidades
  novas, a Legião Caída. Virar condenado é **escolha de poder pela alma** (o arco
  do Campeão do Caos), não um estado-de-fracasso.
- **Criaturas do Caos param de te atacar.** Undead, demônios e void reconhecem a
  alma condenada como kin — não como presa. Você atravessa áreas corrompidas, portais
  ativos e zonas de voidrift sem ser hostilizado. Não é magia nem habilidade: é
  reconhecimento. O mundo caótico te aceita como parte de si. A troca é permanente
  — a Ordem também te reconhece: NPCs recuam, guardiões hostilem, bênçãos rejeitam.

---

## 8. Território: Templos × Portais *(a guerra simétrica)*

O par simétrico do voidrift. Estruturas erguidas por jogadores são o **motor
concreto** da barra (ver §10) e o **enclave local** (ver §11).

| | **Nó do Caos** | **Nó da Ordem** |
|---|---|---|
| Estrutura | Portal voidrift *(já no código)* | **Templo** *(novo)* |
| Erguer | Pergaminho do Caos → portal | Construir/consagrar templo |
| Efeito local | Corrompe a área (8 tiles), spawna monstros | Aura santa: buff de entropia+ aos fiéis perto |
| Efeito global | Pressão pró-Caos enquanto de pé | Pressão pró-Ordem enquanto de pé |
| Verbo do inimigo | **Purgar** o portal *(ritual já existe)* → ganha entropia+ | **Destruir** o templo → ganha entropia− |

- **Ordem = construir/proteger; Caos = corromper/destruir.** Mesmo split de
  Ordem-edifica (reinos prosperam) vs Caos-explora (saques).
- **Cerco emergente:** derrubar um templo é objetivo grande, visível, cercável →
  guerra de cerco sem roteiro (tools not scripts).
- Erguer/manter estrutura do seu lado é ato alinhado (rende entropia do seu polo);
  destruir a do inimigo idem.

---

## 9. Ciclo Dia/Noite & Atmosfera Grim-Dark

A entropia do mundo **enverga o relógio**: o céu inteiro vira mostrador do pêndulo,
e a noite traz horrores que o amanhecer dissolve.

- **A proporção dia/noite escorrega com a barra** — ciclo total fixo em 4h (hoje
  2h+2h em [daynight.cfg](pkg/systems/nature/config/daynight.cfg)). Escala:
  **6 min deslocados por 10k de entropia**, teto de ±60 min.

| Estado global | Noite | Dia |
|---|---|---|
| Caos máximo (−100k) | **3h** | 1h |
| Neutro (0) | 2h | 2h |
| Ordem máxima (+100k) | 1h | **3h** |

- **Piso de 1h sagrado nos dois sentidos:** nunca há noite eterna nem dia eterno.
  Mesmo no mundo mais santo sobra 1h de noite (a janela do insurgente do Caos); no
  mais sombrio nasce 1h de sol (o respiro dos heróis). É a filosofia dos enclaves
  (§11) levada pro relógio — ninguém é trancado fora do jogo.
- **A noite pare sombras.** Com entropia negativa, além dos portais, **criaturas
  sombrias ambientes spawnam à noite e somem ao amanhecer** (dia = Ordem/Luz, nunca
  spawnam de dia). Quanto mais negativo o mundo, **mais numerosas e mais fortes**
  (reusa os limiares de empoderamento global, §5/§13); mundo ordeiro: raras e
  fracas.
- **Auto-correção, não bola-de-neve:** os horrores da noite são exatamente o que a
  Ordem caça pra empurrar o mundo de volta — matar undead/demônio/void já rende
  entropia **positiva**. *A noite é quando a guerra se trava; o amanhecer é a
  recompensa.* O pêndulo pulsa num ciclo diário.
- **Você lê o mundo olhando pro céu:** noites longas e cheias de sombra = o mundo
  está doente. Legibilidade total, atmosfera grim-dark.

**Ganchos de código:** o `daynight.src` **já modula a duração do dia
sazonalmente** (verão +20%, inverno −20%) — a entropia entra como o mesmo tipo de
modificador, dirigido por `GlobalEntropy` (hoje a `nature` **não lê** entropia). Os
spawners (`pkg/systems/spawnpoint`) ainda **não têm gate de horário** — o spawn
noturno de sombras é o pedaço novo; há precedente no padrão "shift" dia/noite dos
mercadores em [nature.inc](pkg/systems/nature/include/nature.inc).

---

## 10. Como a barra enche: **estoque e fluxo, nunca instantâneo**

A pior versão é "cada ato soma um tanto pra sempre" (spammável, espetado, acumula
infinito). A versão certa trata a barra como **reservatório alimentado por
fluxos**:

- **As ESTRUTURAS de pé são o motor do movimento**, não os atos avulsos. Cada
  portal ativo emite corrente pró-Caos *por tick*; cada templo, pró-Ordem. A barra
  **integra** isso ao longo do tempo. O jogador *lê* o mundo (o **globo** já mostra
  o estado): "3 portais, 1 templo → escorregando pro Caos".
- **Atos avulsos são impulsos**, não o sustento. Um assassinato, um ritual único
  dão empurrão **imediato** (feedback na hora), mas não seguram o mundo. Quem
  segura o mundo é **quem segura território.**
- **A deriva ao centro é mais um fluxo** — corrente de fundo constante puxando pro
  0, que as estruturas têm que vencer. Sem estrutura de pé, o mundo cicatriza.
- **Limitado nos dois sentidos:** cap global (corrigir o buraco; teto perto de
  ±60k/±100k, onde moram os limiares de monstro) + deriva + retornos decrescentes
  perto do próprio polo (o rubber-band, §11). **Nunca "pontos pra sempre"** — é um
  equilíbrio vivo.

**Separação limpa de leituras** (responde a pergunta #6 da semente):
- **Entropia pessoal** acumula por *deeds* (com o piso de não-retorno do Caos).
- **Barra global** integra o *balanço de poder territorial* do momento.

---

## 11. Balanceamento: por que não precisa de times iguais

O modelo de cabo-de-guerra pressupõe população equilibrada. O Epic não tem isso
(~90% Ordem / ~10% Caos) — e **não precisa ter**. O equilíbrio não é demográfico;
é estrutural.

**O ambiente É o time do Caos.** Portais abrem passivamente, criaturas spawnam à
noite, a entropia drena sem supervisão. Os 10% de jogadores Caos não são um exército
rival — são **amplificadores** de uma força que já existe independente deles. O
confronto real é: **90% de humanos vs o mundo inteiro que quer afundar no Caos**,
com 10% de humanos que escolheram surfar a onda em vez de nadar contra ela.

Quatro engrenagens mantêm o sistema vivo:

1. **Rubber-band — o lado escasso pesa mais.** Quanto mais o mundo já pende pro seu
   lado, **menos** cada ato seu move a barra; quanto mais hostil, **mais** você
   move. Um jogador Caos solitário num mundo ordeiro move a barra mais do que
   dez num mundo já corrompido. Os extremos resistem sozinhos; o comeback é sempre
   viável.
2. **O debuff muda seu MODO, não seu chão.** O caótico num mundo ordeiro não é "50%
   mais fraco em tudo" — ele vai pra **clandestinidade/guerrilha/sombra** (onde o
   jogo de detectar × disfarçar é o gameplay). Fraco em batalha aberta, perigoso na
   subversão. Underdog continua *divertido*.
3. **O tilt é polegar na balança, não condição de vitória.** Modificador de mundo
   com **teto baixo (~±20%)**. Quem decide a luta é skill/build; a entropia é a
   inclinação que faz valer a pena empurrar a barra, sem virar o perdedor em kill
   grátis.
4. **Enclaves locais — o underdog sempre tem um pé.** Mesmo num mundo ordeiro, o
   Caos **derruba o templo da vila** e abre seu bolsão de poder local; a Ordem
   **purga** o portal ou ergue **santuário** em terra caótica. A briga vira
   territorial; ninguém fica de mãos atadas.

Resultado: **não existe bola-de-neve porque os lados não são simétricos.** A Ordem
pode "vencer" e o mundo ainda vai deteriorar à noite, ainda vai ter portais abrindo,
ainda vai exigir manutenção ativa. A vitória da Ordem não é um estado — é um esforço
contínuo. O Caos não precisa "ganhar": precisa só existir, porque o mundo já puxa
nessa direção.

---

## 12. Anti-exploit (farmar Caos → correr pra Ordem)

O risco: derrubar o mundo no Caos pra farmar Essência, depois deixar tudo Ordem pra
governar reinos — o melhor dos dois sem consequência. Duas travas:

1. **Exclusividade temporal (estrutural):** prosperidade de reino exige **Ordem
   sustentada** (acumula no tempo); Essência exige **Caos sustentado**. Com a
   deriva lenta + cap + estoque-e-fluxo, **não dá pra segurar os dois** — alternar
   custa dias e tanca o bônus do outro lado.
2. **Ponto de não-retorno (pessoal):** o corruptor que mergulha no Caos fica
   **trancado fora da Ordem** (não governa reino, não recebe bênção, é detectável).
   Não pode virar cidadão-modelo depois.

Resíduo honesto: uma guild pode **dividir papéis** (condenados farmam Essência,
limpos governam). Mas os condenados viram **párias permanentes detectáveis** — é
drama emergente bom, não exploit silencioso.

---

## 13. As camadas de efeito (existe / novo / gancho futuro)

- **Direto no jogador — JÁ EXISTE, recalibrar:** trocar o eixo pra "mundo × meu
  time" (§5), remover auto-debuff de cura, baixar o tilt pra ~±20%, religar os
  tiers de alma.
- **Mundo → monstros — JÁ EXISTE:** limiares globais empoderam undead/demônio/void.
- **Mundo → reinos — GANCHO FUTURO:** pragas, terremotos, rolagens. **Depende de
  construir a governança de reino** — hoje só existe a casca de UI em
  `pkg/systems/faccao/gestao_use.src` (abas Reino/Leis/Guerra são stubs TODO) +
  folha de pagamento (soldo). Não é núcleo desta visão; é o gancho que justifica a
  governança quando ela for desenhada.
- **Loot/craft — NOVO:** Essência do Vazio (dreno do voidrift → material de craft
  de elite, só existe com mundo corrompido).
- **Vocações — casam com specs já escritas:** Encantador (Regência: undead pesa,
  arma médio, fera leve →
  [2026-06-19-encantador-abilities-design.md](2026-06-19-encantador-abilities-design.md));
  Devoto (custo cósmico do assassinato →
  [2026-06-20-devoto-abilities-design.md](2026-06-20-devoto-abilities-design.md)).

---

## 14. O core loop, de ponta a ponta

> **O mundo deriva pro Caos por natureza** → a Ordem o segura com templos, rituais
> e presença ativa → **estruturas** (templos × portais) **movem a barra** → a barra
> **inclina o campo** (tilt ~20%) **e enverga o céu** (noites longas no Caos, §9)
> → Essência/reinos motivam a Ordem a agir; jogadores Caos amplificam o que o mundo
> já faz → **rubber-band + enclaves** impedem o travamento → a **Regra do Batman**
> e o **não-retorno** (com o kin das criaturas) dão o peso emocional → **o mundo
> nunca descansa, e a briga é eterna.**

Sobre esse loop de longo prazo, o **ciclo dia/noite (§9)** faz o pêndulo **pulsar
diariamente:** a noite caótica pare sombras, os heróis as caçam até o amanhecer. A
guerra se trava à noite; a recompensa é o sol.

---

## 15. Escopo & fatiamento *(roadmap, não compromisso de implementar agora)*

Esta é uma **visão**; o sistema é grande demais pra um plano só. Quando for
implementar, fatiar (cada fatia = sua própria spec → plano):

- **Fatia 1 — motor:** cap + deriva ao centro na barra global; recalibrar efeitos
  pro modelo time-vs-mundo (§5); religar tiers; tilt ~±20%.
- **Fatia 2 — território & stakes:** Templos (§8); Essência do Vazio; a Regra do
  Batman (custo de assassinato + detecção gato-e-rato + jornada de morte); ponto de
  não-retorno + pacto fáustico.
- **Fatia 2b — atmosfera (§9):** `GlobalEntropy` enverga o ciclo dia/noite em
  `nature` (reusa o modificador sazonal de `daynight.src`); spawn noturno de sombras
  com gate de horário + escala por entropia nos spawners.
- **Fatia 3+ — ganchos:** efeitos nos reinos (depende de governança); religação
  fina com Encantador/Devoto.

Números (pontos/tick por estrutura, taxa de deriva, X do não-retorno, cap global,
% do tilt) são **calibragem de playtest** — não travam a visão.

---

## Perguntas da semente — status

| # | Pergunta | Resposta |
|---|---|---|
| 1 | Polaridade / nomear polos | §1 — Caos(−)/Ordem(+), fixado pelo código + lore Radiância × Legião Caída |
| 2 | Game-loop de pêndulo / o que reseta | §2, §10 — deriva ao centro É o reset; estruturas movem a barra |
| 3 | Recompensa assimétrica | §4 — Caos: ofensiva+Essência+exploração; Ordem: sustento+reinos+estabilidade |
| 4 | Dois extremos como falha | §4 — Caos: monstros+pragas; Ordem: estagnação econômica |
| 5 | Efeito direto × indireto | §5, §13 — modelo time-vs-mundo; direto já existe, recalibrar |
| 6 | Recurso de vocação × estado de mundo | §3, §10 — pessoal=deeds, global=território; alinhamento híbrido |

---

## Inspirações

Warhammer (Caos, Campeão do Caos) · Battlefield (barra de cabo-de-guerra) · Batman
× Coringa (conflito perpétuo, a Regra: o herói que mata se trai) · WoW Demon Hunter
(*rejeitado* — abraçar sombra pra lutar sombra).
