# Habilidades do Ladino (Furtivo) — Design

*2026-06-20. Quarta vocação detalhada do rework. Fonte de verdade do sistema:
[2026-06-17-vocation-system-design.md](2026-06-17-vocation-system-design.md).
Irmã das specs do [Encantador](2026-06-19-encantador-abilities-design.md),
[Warden](2026-06-19-warden-abilities-design.md) e
[Devoto](2026-06-20-devoto-abilities-design.md). Esta spec desenha o **kit**;
migração e motor de progressão pertencem ao rework geral.*

---

## O modelo em uma frase

O Ladino (skill-fonte **Stealth**) é o mestre mundano da furtividade — **fora da
Radiância** (sem fonte mágica, sem Casting). 3 troncos honestos Off/Def/Controle,
reorganizados dos kits que já existem; escolhe 1 por nível, build emerge.

| Árvore | Papel | Identidade | `classe` cfg |
|---|---|---|---|
| **Assassino** | Ofensivo | burst furtivo, execução, re-stealth | Ladino |
| **Esgrimista** | Defensivo / finesse | duelista DEX, parry/contra-ataque (weapon-detect) | Ladino |
| **Shinobi** | Controle | truques ninja, CC, pressão | Ladino |

---

## Fundamentos (o que trava o Ladino)

- **Special ability da classe: Evasão** — concedida na criação (hotbar, não pick
  de árvore): a evasão + maestria de furtividade.
- **Fonte = Stealth.** É a "fonte" do Furtivo, mas fica **fora da Radiância** —
  não trava as 5 fontes mágicas e **nunca usa Casting**. Por isso o Ladino
  coexiste com veneno (Alchemy cívica), Theft, etc. sem tocar em magia.
- **Truque mundano ≠ magia de sombra.** O ninja *mágico de verdade* (clone real,
  teleporte de sombra) é build de **Arcanista / escola Velkyn** (o Dançarino das
  Sombras) — princípio #9 e a divisão já no doc-fonte. O Ladino faz a versão
  **mundana** (engodo, fumaça, ferramenta, perícia).
- **Recurso = `stam + delay` + textura de Stealth.** Sem medidor próprio. O
  **Sneak escala e destrava** os truques (substitui o "Circle/Difficulty" que os
  truques tinham como spell). Muitos exigem estar furtivo pra disparar.
- **Disfarce/espionagem = cívico**, aberto a todos. Mantém os 3 troncos focados em
  combate; espião pode ser qualquer vocação (e é RP/social → casa no cívico).

---

## Progressão

Pick em cada breakpoint **1 / 3 / 5 / 7 / 10**; escolhe a árvore, tier fixado pelo
nível; habilidades autocontidas. Build emerge da sequência de escolhas.

---

## As 3 árvores (reorganizadas dos kits existentes)

### 🗡️ Assassino — Ofensivo (burst / execução)
Assassinar (salta às costas, acerto garantido se furtivo) · Instinto Assassino
(Assassinar vira crítico + dobra dano) · Arsenal Sombrio (+sneak/20 de dano com
arma de assassino) · Faro por Sangue (executa feridos, escala com o ferimento) ·
Manto das Sombras (re-stealth após golpe furtivo se vencer sneak vs awareness).

### 🤺 Esgrimista — Defensivo / finesse (duelista DEX) — *weapon-detect*
Movimentação Graciosa (esquiva + tactics/10 acerto/esquiva) · Postura de Duelo
(**adapta ao equipamento**: buckler→parry, mão livre→esquiva, duas armas→acerto,
pistola de duelo→habilita — é a *face por arma*) · Desmoralizar (provoca, −acerto
no alvo) · Alta Guarda (contra-ataca todo golpe recebido) · Mestre Duelista
(re-rola ataque que erra por tt/5s).

### 🥷 Shinobi — Controle (truques ninja)
Base do tronco (reorganizada): Caminho do Ninja · Golpe Paralisante · Bomba de
Fumaça · Corrida Furtiva · Postura Fantasma. **Cada tier também concede uma
Técnica = 2 truques** (o `shinobi.cfg` já agrupa os 10 em 5 Técnicas de 2 — bate
exato com os 5 tiers, e dá ritmo: 2 brinquedos novos por nível).

**Os truques deixam de ser spells e viram habilidades movidas por Stealth** (sem
mana, sem círculo, sem Casting — escala em Sneak). Triagem dos 10:

| Técnica (tier) | Truques | Destino |
|---|---|---|
| 1 | Substituição (vira tora→some) · Avanço (dash) | **Mundano → Ladino** |
| 2 | Distração (troca alvo) · Escapar (solta-se) | **Mundano → Ladino** |
| 3 | Fingir Morte · Despertar (medicina) | **Mundano → Ladino** *(Despertar encosta em Medicine cívica)* |
| 4 | **Disfarce** (Henge) · **Troca** (Irekawari, teleporta) | Disfarce → **cívico** · Troca → **Velkyn/Arcana** |
| 5 | **Clone** (Kage Bunshin, clone real) · Intuição (lê o alvo) | Clone → **Velkyn/Arcana** · Intuição → **Mundano** *(encosta em Perception cívica)* |

Resultado: o Ladino mantém **~7 truques mundanos** (o grosso da diversão); o ninja
mágico (Clone real, teleporte-troca) vira **Arcanista/Velkyn**; disfarce vira
**cívico**. Quem quer o ninja mágico joga Velkyn; quem quer o ninja de ferramenta
e sombra joga Ladino — leve, sem mana.

---

## Fronteiras com outras vocações

- **Ladino vs Arcanista/Velkyn (Dançarino das Sombras):** furtividade *mundana*
  (Stealth) vs furtividade *arcana* (sombra é magia). Os truques mágicos moram lá.
- **Veneno:** é **Alchemy cívica**, não tronco — o Assassino usa, qualquer um pode.
- **Esgrimista** usa **weapon-detect** (Postura de Duelo) — a face muda pela arma,
  igual ao princípio #3. É o único tronco com faces por arma.

---

## Pendências (fora do escopo desta spec)

1. **Balanceamento — habilidades acumulam funções demais.** As habilidades atuais
   empilham várias coisas por slot (ex.: *Caminho do Ninja* = +furtividade + ouvir
   por paredes + adaga sem stam + marca-teleporte-golpe + destrava técnicas). O
   modelo novo quer picks **autocontidos** (1 coisa clara por escolha). **Decompor
   as habilidades sobrecarregadas é uma spec/passe próprio** (apontado pelo user).
2. **Conversão técnica** dos `spell_logic/shinobi/*` de spell → habilidade-Stealth
   (remover mana/círculo/Difficulty; reimplementar escala por Sneak). Trabalho de
   código real.
3. **Mover Clone/Troca pra escola Velkyn** (spec do Arcanista) e **Disfarce pro
   cívico** (skill de disfarce).
4. **Números** e **migração** (Assassino/Shinobi/Esgrimista atuais).
5. **Mapa off/def/controle confirmado** (doc-fonte tinha como TBD): Assassino=Off ·
   Esgrimista=Def/finesse · Shinobi=Controle.
