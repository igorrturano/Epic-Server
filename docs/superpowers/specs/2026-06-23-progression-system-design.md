# Sistema de Progressão (Level / XP / Stats / HP) — Design

*2026-06-23. O "motor" que faz o build das 7 vocações emergir. Fonte de verdade:
[2026-06-17-vocation-system-design.md](2026-06-17-vocation-system-design.md);
filtros em [epic-design-principles.md](../../epic-design-principles.md) e
[epic-vision.md](../../epic-vision.md). Estado atual do código:
[character-creation-pipeline.md](../../character-creation-pipeline.md) (a fórmula
de HP Phase 4 já roda). Esta spec é a camada de **decisão de design**; números
finais e implementação ficam pra depois.*

---

## A régua (a frase que governa tudo)

> **Par = você escolhe quem é · Ímpar = você fica mais forte no que já é ·
> Stats = criação · Diversão = pares + skills.**

Decisão de build vive nos **níveis pares** (habilidades) e na **criação** (stats).
Os **ímpares** são a subida automática — nunca uma escolha fraca (escolha morna é
*tarefa disfarçada de decisão*, pior que bump limpo).

---

## A cadência de níveis (1 → 20)

| Nível | O que dá |
|---|---|
| **1** | **Special ability da classe** (assinatura, na criação, hotbar — ver doc-fonte) |
| **2 · 6 · 10 · 14 · 20** | **Habilidade de classe** — 1 de 3 troncos (Of/Def/Sup). **5 picks; capstone no 20** |
| **4 · 8 · 12 · 16 · 18** | **Habilidade de Caminho** — pool **aberta a todos** (weapon spec, fight style, armadura, cross-class tipo Disfarce) — **5 picks** |
| **3 · 5 · 7 · 9 · 11 · 13 · 15 · 17 · 19** (ímpares) | **Bump automático: HP + vitais** (sem escolha). Em alguns marcos ímpares, **destrava de tier de gear** (ver abaixo) |

= **10 picks marcantes** na carreira (5 classe + 5 caminho), ~1 a cada 2 níveis.
Os 10 picks + o crescimento contínuo de **skill** carregam a diversão; o ímpar é o
tecido conjuntivo.

> **Sem feats no ímpar.** Mais uma habilidade por nível fica forte demais e difícil
> de balancear. O Caminho (pares 4/8/12/16/18) já é a camada "feat" do Epic.

Mapeamento p/ implementação: os 5 tiers de habilidade do `habilidades.cfg`
(hoje `classe X 1/3/5/7/10`) correspondem aos níveis de personagem
**2/6/10/14/20**.

---

## Stats — definidos na criação

Mantém o modelo atual: **40/40/40 + 80 pontos** distribuídos na criação.

**Por que NÃO migrar pontos pro level-up** (a gente considerou e descartou):
- **STR pra equipar arma** exige a STR base já na criação — não dá pra diferir.
- **Pareto:** ponto de late é caro; build refém do stat tardio = grind frustrante.
- **Novato:** alocar stat pingado por 20 níveis = carga cognitiva e cansaço.

O flex de híbrido (guerreiro inclinando paladino/arqueiro via dex/int/mana) é uma
**escolha de criação** — alocar lá — ou via habilidade (ex.: Mutagênicos). Não é
drip de nível. **O `+2 por ímpar` foi deletado** (irrisório; não pagava aluguel).

---

## HP — a fórmula Phase 4 fica

`HP = STR + (die × níveis-ímpares-alcançados)`, die por classe (d4…d12), ímpares
saturam em 9 (nível 19+). Já roda hoje
([detalhes](../../character-creation-pipeline.md#8-hp--the-phase-4-formula)).

- Parte **STR** = a carne do build (alavanca MAD/SAD: caster foca INT → frágil).
- Parte **die** = identidade de classe, automática e escalando (d12 = +12/ímpar).
- Spread de carreira ~**44 no L1** → ~**100 (frágil) a ~220 (tank)**. Saudável.
- **Acoplar HP à STR é proposital** (alavanca de fragilidade limpa). Aceita-se o
  "STR puxa builds não-mágicos" em nome do *menos é mais* — não vale um 4º atributo
  só pra desacoplar.
- Convergência cedo (no L1 o die quase não pesa) é **boa pro onboarding** — todo
  mundo sobrevive a 1ª hora; a fragilidade do mago emerge no late. Manter.

---

## Níveis ímpares — bump + gate de gear

- **Núcleo: HP + vitais automáticos** (o die-HP que já existe + stam/mana). "Ding,
  mais forte", zero pra aprender, zero balanceamento, não toca em STR-equip nem
  Pareto.
- **Em alguns marcos ímpares (ex.: 5 / 11 / 17): destrava de tier de gear.**
  Equipamento **level-gated** — cruzar o nível libera vestir a próxima tier. Faz
  trabalho duplo:
  1. **Marco tangível no ímpar** (+HP *e* "agora posso usar essa tier"), claro pro
     novato, sem virar escolha.
  2. **Anti-twink:** impede o veterano de vestir um personagem novo com top gear
     ("panela").
- **Regras do gate:**
  - Gate por **TIER / salto de poder**, não item a item (muro constante = atrito).
  - **Level-gate ≠ STR-gate:** STR = "arma pesada, precisa de músculo" (build);
    Level = "tier de poder" (progressão). Não muralhar a mesma peça com os dois
    sem intenção.
  - Trava só as **tiers altas**; low/mid acessível pro novato ("gear é o ingresso,
    não o boss").
- **Bônus MDA:** reforça a **moeda não-conversível** — poder de gear é *ganho
  jogando*, não comprado/herdado. O crafter constrói a peça top; só quem subiu de
  nível veste. Amarra a cadeia coletor→crafter→combatente.

---

## Caminho mítico — parkado

Camada de **endgame / Legado** (pós-cap). Régua quando for desenhar: tem que ser
**horizontal** (status, título, marca no mundo), **não "+poder"** — senão fere
"poder não pode ser a única moeda" (visão). TBD.

---

## Pendências (fora do escopo desta spec)

1. **Números:** HP base/curva, quanto de vital por ímpar, em quais ímpares cai o
   gate de gear, as tiers de gear e seus níveis.
2. **Tiers de gear level-gated** — definir as faixas e quais itens entram.
3. **XP → level** — a curva (Pareto: rápido até funcional, lento até mestre;
   "deus" no 20 em ~6 meses, ver visão).
4. **Caminho mítico** — design próprio.
5. **Migração** — personagens legados (níveis/XP) já tratados pelo creation
   pipeline; confirmar que a nova cadência não quebra os existentes.
