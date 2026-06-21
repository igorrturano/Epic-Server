# Princípios de Design do Epic

*Filtro pra toda decisão de design. Quando bater a dúvida "isso é divertido? tá
enxuto?", consulte aqui em vez de girar. Companheiro do
`superpowers/specs/2026-06-17-vocation-system-design.md`.*

## Filosofia: menos é mais

Cada elemento que você adiciona (classe, recurso, sistema) não **soma**, ele
**multiplica** as combinações a balancear e a explicar. É por isso que o design
"nunca para de crescer" e "sempre tem alguém reclamando" — é a estrutura, não
falta de talento. A cura não é trabalhar mais: é **menos peças que se combinam**
no lugar de mais peças isoladas.

Design enxuto e opinativo que você **defende** ganha de um que tenta agradar
todo mundo. Num sistema enxuto, reclamação é sinal acionável; num sistema
inchado, é ruído.

## A regra do aluguel

> Toda variável nova tem que **pagar aluguel** em diversão ou clareza.
> Se não paga, corta.

Antes de adicionar qualquer coisa: isto deixa o jogo mais divertido **ou** mais
claro? Se for só "mais completo / mais realista / mais flexível", sem ser mais
divertido nem mais claro, corta.

## Decisão vs tarefa

> Se toda vez você **escolhe** algo, é gameplay. Se você sempre aperta o botão
> pra fazer **a mesma coisa**, é tarefa.

Tarefa não paga aluguel — corta ou transforma em decisão. A diferença entre
divertido e cansativo raramente é a quantidade de passos; é **quantos deles são
escolha real**. Para enriquecer sem inchar: não adicione passos — **converta
passo repetitivo em decisão** (escassez, tradeoff, risco).

## A bússola de diversão (4 alavancas)

Toda mecânica de progressão passa por estas quatro:

1. **Ritmo de recompensa.** A barra de XP enche, vem um prêmio. Cada prêmio é um
   **brinquedo novo ou pico de poder**, nunca um "+5% de dano". Antecipação mais
   recompensa é a dopamina. A barra de XP é o motor; nunca a jogue fora.
2. **Escolha que muda o jogo.** Cada prêmio (níveis 2 / 6 / 10 / 14 / 20) é 1 de
   3 (ataque / defesa / controle) que muda **como você joga** — um verbo novo,
   não só números.
3. **Identidade que dá pra contar.** O jogador descreve o personagem em uma
   frase ("Guardião armadilheiro do deserto"). O roleplay nasce da mecânica.
4. **Piso baixo, teto alto.** Escolher 1 de 7 vocações é fácil (piso). Dominar a
   combinação skill + árvore + cultura é profundo (teto).

## Classe ou sem classe: resolvido

O Epic usa o **híbrido**: vocação leve (a fantasia, pro roleplay) + árvore de
habilidades emergente (o build pessoal). Nem UO-planilha (sem fantasia, difícil
de balancear, todo mundo converge no build ótimo), nem D&D-rígido (engessa e
multiplica classes). São 7 vocações, 3 árvores cada, ~5 escolhas marcantes numa
carreira.

## Princípios concretos (já travados nesta reformulação)

- **Recurso é mecânica, não inventário.** Habilidade nunca pede pólvora/poção;
  enche um medidor **agindo** (calor, ki, combo).
- **Cultura é sabor, não jaula.** Cultura pinta e sugere, nunca restringe
  escolha de combate. Exceção: magia enraizada no ambiente (o Naturalista).
- **Especialização é habilidade, não classe.** Escolas, tradições e domínios são
  ramos de árvore, não classes novas.
- **A arma define a face da habilidade.** Uma habilidade, várias faces (Fúria
  melee / Tiro arco). Nunca duplique classes por arma.
- **2 skills-núcleo por build.** Acertar + escalar. MAD paga em atributo, não em
  3ª skill; SAD paga em fragilidade.
- **Penalty de armadura é da fonte, não da classe** (lógica D&D). Arcana paga o
  spell-failure de armadura (arcano precisa de gesto livre); Theology não (fé
  funciona de armadura — o sacerdote-de-guerra). Não se compra com habilidade.
- **Trava generativa: ≤7 vocações × ~3 árvores.** Ideia nova cabe numa célula
  existente ou é flavor dentro de uma árvore.

## Como usar (rode toda decisão por aqui)

1. **Paga aluguel?** (diversão ou clareza)
2. **Passa nas 4 alavancas?** (ritmo · escolha · identidade · piso-teto)
3. **Cabe na trava** (≤7 × ~3) ou é flavor dentro de uma árvore existente?
4. **Soma ou multiplica?** Prefira somar dentro do que já existe a criar peça
   nova isolada.

Passou nos quatro, vai. Não passou, corta ou repensa até caber.
