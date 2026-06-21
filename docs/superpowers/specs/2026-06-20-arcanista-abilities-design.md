# Habilidades do Arcanista (Arcana) — Design

*2026-06-20 (rev. 2026-06-21: troncos Off/Def/Sup + sombra resolvida). Sexta
vocação do rework. Fonte de verdade: [2026-06-17-vocation-system-design.md](2026-06-17-vocation-system-design.md).
Irmã das specs de Encantador, Warden, Devoto, Ladino e Marcial. É a vocação que
**amarra as pontas soltas** das outras (Velkyn, Kaijin). Esta spec desenha o
**kit**; migração e motor de progressão pertencem ao rework geral.*

---

## O modelo em uma frase

O Arcanista (skill-fonte **Arcana + Casting**) é o caster pleno (a magia é o
trabalho inteiro). Como o Devoto, é de **dois andares**:

1. **3 troncos Off/Def/Suporte** (universais): Ofensivo (**mago lutador / nuker**)
   · Defensivo (**mago abjurador**) · Suporte/Controle (**mago de backline**).
2. **Spellbook Arcana unificada**, graduada por **Casting**, organizada em **4
   escolas**. Você **especializa em 1 escola** — e isso É o anti-combo.

---

## Fundamentos

- **Fonte = Arcana + Casting.** Caster pleno (2 skills mágicas, princípio 4).
- **Arcana PAGA o penalty de armadura** (regra source-based): magia arcana precisa
  de gesto livre. O Arcanista é **SAD — paga em fragilidade** (HP die menor,
  armadura leve). É o contraste limpo com o Devoto (que casta de armadura).
- **Especializa em 1 das 4 escolas.** Mais fácil de balancear e identidade clara
  — e mata o combo absurdo de hoje (fireball + dreno + tudo junto): se você é de
  uma escola, **não tem** as magias das outras.
- **Recurso:** magias = **mana + delay** (círculo por Casting); habilidades de
  árvore = **stam + delay**. Sem medidor próprio.

---

## As 4 escolas (especialização — escolhe 1)

| Escola (cfg) | Domínio | Absorve |
|---|---|---|
| **Evocatium** | dano elemental | a escola "que vingou" |
| **Velkyn** | ilusão / sombra / controle | Clone real + Troca/teleporte do ninja (spec do Ladino) |
| **Kaijin** | necromancia: **dreno / maldição** | a magia da morte do Necromante que **não** é comandar minion |
| **Trapaceiro** | utilidade ardilosa (fechaduras, decifrar, disparo místico) | o **Malandro Arcano** dissolvido |

> **Dois necromantes:** Arcanista/**Kaijin** = mago da morte (dreno/maldição, sem
> minion); Encantador/**Profana** = comandante da morte (1 morto leal). Magia
> cria; só a Regência comanda. (Princípio #9.)

---

## A sombra — resolvida (lore-first, sem caso especial)

A magia de sombra estava emaranhada em "classe Drow exclusiva". Desemaranhada em
duas camadas, **as duas abertas**:

- **Magia Velkyn (spells) = aberta.** Qualquer Arcanista estuda — é o **mago de
  sombra**. É a escola Velkyn acima.
- **Lutador de sombra = build de Ladino** (furtividade *mundana*: esgueirar,
  reposicionar, atacar das sombras). Não é Arcanista.
- **Drow = o povo icônico da sombra** (bênção de Idrith) — **flavor/estética, sem
  classe travada**. Joga Arcanista/Velkyn (mago) ou Ladino (lutador), como
  qualquer um. Não força o Drow à sombra (ele pode ser Evocatium/elementalista).
- **Deslocador das Sombras = deletado** (nunca foi feito de verdade; era um
  Dançarino pior, travado pela mana). **Dançarino deixa de ser tronco** —
  vira esses dois builds abertos.

---

## Progressão

Pick em cada breakpoint **1 / 3 / 5 / 7 / 10** (escolhe o tronco); a **spellbook**
é à parte (graduada por Casting), e você **especializa em 1 escola**. Habilidades
autocontidas. Build emerge.

---

## As 3 árvores (reorganizadas dos kits existentes)

### 🗡️ Ofensivo — mago lutador / nuker
Gish de lâmina + evocação agressiva. **Absorve o Arqueiro Arcano** (build de arco).
Vem da **Lâmina Arcana**: Vínculo Elemental (absorve armas 1H como lâminas de
energia; conjura sem penalidade de movimento) · Fluência (drena proteção
elemental, próximo golpe certeiro) · Domínio Vital (puxa/empurra alvo) · Armadura
Arcana (armadura conforme a arma) · Vórtice Prismático (absorve dano → explosão
elemental em área).

### 🛡️ Defensivo — mago abjurador
Wards, escudos, blink, absorção — a sobrevivência do SAD frágil. Minera Armadura
Arcana / Vórtice Prismático / Salto Etéreo (escape) + abjuração nova (greenfield).

### 🔮 Suporte/Controle — mago de backline
CC, ilusão, banir, portais, debuff/buff. Vem do **Magus**: Determinação Arcana
(stam sem mana) · Salto Etéreo (teleporte) · Maestria de Escola (ex-*Evocator*:
+eficácia da **sua** escola) · Prisão Dimensional (bane um alvo) · Abrir Portais
(marca locais e teleporta). + silêncio/controle herdado do antigo Dançarino.

---

## Fronteiras com outras vocações

- **Arcanista vs Devoto:** Arcana paga penalty de armadura (SAD/fragilidade);
  Theology não (war-priest). Caster arcano × divino.
- **Sombra: Arcanista/Velkyn (magia) vs Ladino (lutador mundano).** Magia aberta;
  luta de sombra é furtividade mundana. Drow icônico nos dois, travado em nenhum.
- **Kaijin (Arcana) vs Encantador Profana (Regência):** mago da morte × comandante.
- **Arco:** Ofensivo (Lâmina) × Marcial × Warden-Predador — a vocação dá a
  identidade; o arco é só a ferramenta, aberta a todos.

---

## Pendências (fora do escopo desta spec)

1. **Curadoria da spellbook Arcana** por escola (círculos, potência) — o miolo do
   balanceamento.
2. **Nomenclatura das escolas** — confirmar mapa cfg (Evocatium/Velkyn/Kaijin/
   Ahtalelorëth?) × doc (Evocação/Ilusão/Necromancia/Trapaceiro); resolver
   possível sobreposição Ahtalelorëth×Evocatium na lore.
3. **Receber as migrações** do ninja (Clone/Troca → Velkyn) e do Encantador
   (dreno/maldição → Kaijin); o lutador-de-sombra mundano → Ladino.
4. **Afinidade tronco×escola** — se Ofensivo puxa Evocatium etc., ou é livre.
5. **Números** e **migração** (Magus/Lâmina/Dançarino/Arqueiro Arcano/Malandro
   atuais).
