# Entropia como Sistema — Semente de Brainstorm

*2026-06-23. **Isto é matéria-prima, NÃO decisão.** Captura das ideias cruas pra
brainstormar entropia como sistema próprio num chat dedicado. Fontes: o gancho
Regência↔entropia ([Encantador](2026-06-19-encantador-abilities-design.md)), o
gancho morte↔entropia ([Devoto](2026-06-20-devoto-abilities-design.md)),
`docs/epic-systems-map.md`, e o sistema já implementado em
`pkg/systems/entropy/entropy.inc` + `pkg/systems/voidrift/`.*

---

## A ideia em uma frase

Entropia deixa de ser um medidor de canto e vira o **eixo Ordem × Caos do mundo
inteiro** — um "jogo dentro do jogo" que conecta vocações, mundo, e facções, e cria
um game-loop de pêndulo (estilo Warhammer + barra de cabo-de-guerra do Battlefield).

---

## O que JÁ existe no código (não inventar do zero)

- **Entropia por jogador** (~**-20.000 a +20.000**) **+ entropia global**
  (`UpdateMobileEntropy` mexe nos dois).
- **Entropia global dá poder a mortos-vivos / demônios / (talvez) extraplanares.**
- **Alta entropia = corrupção** → afeta saves ("sua alma corrupta expurga o veneno").
- **Bônus por marco de entropia ligados a DP** (death points) — `updateDPEntropyBonus`.
- Hoje só alimenta a magia do **Bardo** (`cronica_do_abismo`) e o **voidrift**
  (rituais do vazio que **gastam entropia pra invocar/banir**).

⚠️ **Convenção de sinal a confirmar:** no código `+` (alto) = corrupção/caos,
`-` (baixo) = ordem. O usuário falou "Devoto se beneficia de entropia **positiva**"
— provável que queira dizer o **polo Ordem**; **nomear os polos** é a 1ª decisão.

---

## A moldura de lore (Warhammer-inspirada)

- **Agentes do Caos = aliens** caçando Alora/a Radiância no planeta.
- Entropia alta **abre buracos de minhoca / portais** → atrai mais aliens,
  demônios, criaturas extraplanares.
- Perturbar a ordem **desperta mortos-vivos**; o mundo **mergulha no caos**.
- O **voidrift** já é o "portal do caos" mecânico — escalar isso.
- Eco da **Legião/Capítulo Caído** (que se entregou ao caos) como facção/efeito.

---

## Como conecta com as vocações (ganchos, a refinar)

- **Encantador (Regência):** reger o sem-mente perturba a ordem → **gera
  entropia**. É o **eixo de risco/recompensa** da classe (systems-map já planeja).
  Mortos/armas/feras com pesos diferentes (feras = perturbação leve).
- **Devoto (Fé):** beneficia-se de um polo, prejudica-se do outro **(qual? definir
  sinal)**. O **Caçador de Demônios** beneficia do mesmo polo do Devoto —
  **dropar** o "abraçar as sombras pra lutar contra as sombras" (estilo DH do WoW);
  *menos é mais*, não faz falta.
- **(Outras vocações:** como cada uma se relaciona com Ordem/Caos — TBD.)

---

## As perguntas que o brainstorm tem que responder

1. **Polaridade:** nomear os polos (-20k vs +20k = Ordem vs Caos?), e o que cada
   extremo *significa* e *faz*.
2. **O game-loop de pêndulo:** Caos empurra (abre portais voidrift → atrai
   criaturas → sobe entropia); Ordem empurra de volta. Como a barra oscila
   (Battlefield)? O que **reseta** quando um lado vence?
3. **Recompensa assimétrica:** o que **jogadores do Caos** ganham? o que
   **jogadores da Ordem** ganham? (Sem isso, ninguém escolhe o lado "ruim".)
4. **Os dois extremos como falha:** "ordem demais = tirania; caos demais = ???".
   O que torna *cada* extremo indesejável, pra o pêndulo querer voltar ao meio?
5. **Efeito direto × indireto no jogador:** corrupção/buff/debuff por alinhamento
   (direto) × spawns/portais/inimigos empoderados (indireto/mundo).
6. **Entropia como recurso de vocação** (princípio #7: medidor agindo) vs entropia
   como estado-de-mundo — como as duas leituras coexistem.

---

## Lentes a aplicar (dos docs que já temos)

- **MDA-social:** isto é uma **fábrica de stakes + drama + aresta social** — Ordem
  vs Caos = conflito emergente entre jogadores (o "jogo dentro do jogo"). Construir
  **ferramentas, não roteiros** (jogadores geram o drama).
- **Moeda não-conversível:** o que o Caos dá NÃO pode ser comprável pela Ordem e
  vice-versa, senão colapsa num lado só.
- **Visão:** mundo que evolui (estações/eventos/política) = o pilar de 10 anos;
  entropia pode ser o motor desse mundo-vivo. Mas cuidar pra não virar grind nem
  FOMO ("jornada, não vício").

---

## Inspirações declaradas
Warhammer (Caos) · Battlefield (barra de cabo-de-guerra) · WoW Demon Hunter
(*rejeitado* — abraçar sombra pra lutar sombra).
