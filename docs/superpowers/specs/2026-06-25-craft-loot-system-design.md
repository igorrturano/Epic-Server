# Sistema de Craft & Loot — Design

*2026-06-25. Doc de visão de game design. Define o loop central de Craft & Loot
do Epic: pipeline de materiais, sistema de sockets/gems, bosses nomeados,
Awakening e coerência com Entropia, Reinos e as 7 vocações. Companheiro de
[entropy-system-design.md](2026-06-23-entropy-system-design.md) e
[kingdom-system-design.md](2026-06-24-kingdom-system-design.md).*

---

## A ideia em uma frase

Todo guerreiro pode ser crafter e todo crafter precisa do guerreiro — o mapa é
a progressão, o boss é a história, e a gem é a decisão.

---

## 0. O problema que este design resolve

O loop atual concentra poder no crafter (autosu­ficiente, acumula ouro) e deixa
o guerreiro como pagador passivo. Materiais vêm de grind longo e previsível.
Não há surpresa, nem identidade de item, nem dopamina sustentada.

**Três sintomas:**
- Guerreiro não tem motivo para craftar; crafter não precisa do guerreiro.
- Drops são predizíveis; segunda run é tarefa, não aventura.
- Itens não têm história — qualquer Lendária com fogo é igual a outra.

---

## 1. Princípio central: o mapa É a progressão

Qualidade de material é determinada pela **distância/dificuldade do bioma**,
não por RNG de quantidade. O jogador sabe o que esperar quando decide ir mais
fundo. A dopamina vem da escalada geográfica, não da sorte.

| Zona | Tier | Exemplos de material |
|---|---|---|
| Zonas iniciais | T1 — comum | Ferro, couro comum, madeira simples |
| Zonas intermediárias | T2 — refinado | Aço, couro grosso, carvalho |
| Zonas avançadas | T3 — raro | Adamantita, couro draconiano, salgueira |
| Dungeons profundas / Voidrift | T4 — excepcional | Liga rúnica, couro do Vazio, madeira sombria |

Nós de recurso são **finitos** (veios de minério, clareiras de madeira rara).
Respawnam em dias. Sem mina infinita, sem grind AFK.

---

## 2. Pipeline de materiais: três fontes, três acessos

### 2.1 Inorgânicos (Mining + Lumberjacking)
Nós finitos por bioma. Tier do material = tier da zona. Madeiras raras ficam
em biomas distantes com dificuldade alta de Lumberjacking.

### 2.2 Orgânicos (Survival / skinning)
**O mob é a mina.** `corpseitm` já existe; Survival determina *quanto* se
extrai — nunca punição total, sempre resultado parcial:

- Survival 0–49 → ~50% dos itens do `corpseitm`
- Survival 50–79 → ~75%
- Survival 80–100 → 100%

Qualidade do material é fixa pelo tier do mob/bioma. Matar um dragão draconiano
sempre rende `courodraconiano` — a skill define apenas o aproveitamento.

### 2.3 Drops diretos (exploração)
- **Gear do mob** (raro, roleplay — o mob estava usando): permanece como está.
- **Gems pré-formadas**: drops de boss e dungeon profunda, prontas para encaixar
  em sockets. Não requerem Survival. São a recompensa exclusiva da exploração.
- **Receitas raras**: drops de mobs nomeados e dungeons específicas; também
  disponíveis em Bibliotecas de Baronias.

---

## 3. Crafting: receitas, qualidade e sockets

### 3.1 Receitas
- Algumas conhecidas desde o início.
- Novas: drop raro de mobs, pesquisa em Biblioteca (prédio de Baronia).
- Receitas de bosses nomeados produzem **itens únicos** — ver §4.

### 3.2 Qualidade do item
O sistema de componentes existente (lâmina, cabo, guarda, etc.) já produz tiers
de qualidade. O tier final determina os sockets disponíveis:

| Tier | Sockets |
|---|---|
| Ruim | 0 |
| Comum | 0 |
| Incomum | 1 |
| Épico | 2 |
| Lendário | 3 |

A skill do crafter afeta a **probabilidade de atingir tiers mais altos** — não
o número de sockets diretamente. Crafter com Smithing 100 tem chance real de
produzir Lendários; Smithing 30 raramente passa de Incomum.

### 3.3 Tipos de socket
O crafter escolhe o **tipo de cada socket** no momento do craft. Um item Épico
com 2 sockets pode ter dois sockets de Fogo, ou um de Fogo e um de Sombra — a
decisão é do crafter e fica permanente na estrutura do item.

### 3.4 Duas forjas

**Forja comum** (cidade, sempre disponível)
- Produz itens até tier Épico.
- Aceita todas as gems elementais e de alma.

**Forja Real** (prédio de Baronia — ver kingdom-system-design.md)
- Única capaz de trabalhar com **Void Essences**.
- Pode produzir tier Lendário.
- Requer Baronia com Legitimidade suficiente para manter o prédio.

**A tensão do design:** Void Essences só dropam quando o mundo está em Caos.
Forja Real exige Baronia saudável, que depende de Ordem. O guerreiro quer Caos
para farmar; o crafter quer Ordem para prosperar. A codependência é estrutural
— eles precisam um do outro exatamente porque querem coisas opostas.

---

## 4. Boss nomeados: itens com história

Mobs genéricos dropam material genérico. **Bosses nomeados** (ex: Chk'Lykk, o
rei-rato) dropam materiais únicos com o nome do boss. Esses materiais permitem
craftar **itens nomeados** — não "espada Lendária com fogo", mas *Fang Blade de
Chk'Lykk*.

O item resultante carrega a identidade do que foi derrotado. Nenhuma outra
fonte produz aquele material. Implementação direta: `corpseitm` especial no
boss + `fabricacaoDesc` própria com o item resultante nomeado.

### Desbloqueio por facção (faction gate)
O boss só aparece após a facção responsável pela região derrotar um threshold
de mobs comuns da dungeon. Isso:
- Dá propósito imediato às facções no mapa.
- Cria eventos sociais naturais ("o boss está prestes a aparecer").
- Conecta o loot loop ao sistema de facções existente.

---

## 5. Sockets e gems

### 5.1 O guerreiro encaixa, o crafter não precisa estar presente
O crafter forja o item com os sockets embutidos. O guerreiro encaixa as gems
**sozinho**, sem crafter. A codependência está em quem cria a estrutura (crafter)
e quem abastece o material (guerreiro), não no ato de encaixar.

### 5.2 Permanência e recuperação
Sockets são **permanentes** no item. Gems são **recuperadas** quando o item
quebra pelo sistema de weardown existente. O guerreiro não perde o que
conquistou; o crafter vende novas armaduras no ritmo natural do desgaste.

*A taxa de desgaste precisará de calibragem no playtest para manter o ciclo
econômico vivo.*

### 5.3 As 6 gems

| Gem | Vocation primária | Em arma | Em armadura | Em item mágico |
|---|---|---|---|---|
| **Fogo** | Arcanista, Fighter | dano de fogo | resist. fogo | amplia magias de fogo |
| **Gelo** | Arcanista, Fighter | dano + lentidão | resist. gelo | amplia magias de gelo |
| **Energia** | Arcanista, Fighter, Ladino | dano + velocidade | resist. energia | amplia magias energia |
| **Sombra** *(nome provisório)* | Encantador, Ladino, Arcanista (ilusão) | dano sombrio + debuff | reduz detecção | amplia invocação e ilusão |
| **Luz** *(nome provisório)* | Guardião, Divino (secundário) | lifesteal no hit | regen de HP | amplia cura e nature |
| **Trevas** *(nome provisório)* | Arcanista (necro-caster), Alquimista/Mutante | necrose + maldição | ward vs. dano de morte | amplia necromancia e debuffs |

**Notas de vocation:**
- **Divino** usa Luz como secundário — o poder primário vem de Theology + Entropia,
  não de gems. A gem potencializa, não é a fonte.
- **Alquimista** (Science) não tem gem própria. Seu papel no sistema de gems é
  criar **poções que amplificam temporariamente** o efeito de uma gem específica
  no equipamento do guerreiro. É a ponte sem precisar de tipo novo.
- Nomes finais das gems são decisão de lore, a afinar posteriormente.

### 5.4 Synergias descobríveis (4 combinações pré-desenhadas)

Sem tutorial, sem dica no jogo. Descobertas por experimentação e transmitidas
entre jogadores.

| Combinação | Nome | Efeito | Arquétipo |
|---|---|---|---|
| Fogo + Gelo + Energia | **Plasma** | Único dano efetivo contra criaturas do Vazio | Todos |
| Sombra + Energia | **Éter** | Dano ignora parcela de armadura (psiônico) | Ladino arcano, Arcanista |
| Luz + Fogo | **Brasa Sagrada** | DoT que cura o portador | Guardião, Divino |
| Sombra + Luz | **Eclipse** | Muito poderoso — corrompe entropia do portador em direção ao Caos | Encantador, os que caem |

**Eclipse** é a synergy proibida. Na lore, Aigla usou a combinação Vácuo+Luz
para criar os portais de conquista. Um Paladino nunca a tocaria; um Encantador
já escorrega naturalmente pro Caos — para ele, Eclipse é quase inevitável.
Usar Eclipse é um passo concreto em direção ao ponto de não-retorno da Entropia.

Sinergia potencial ainda não desenhada: **Trevas + Luz** (segunda combinação
proibida?) ou **Trevas + Gelo** (glaciação da alma). A definir quando o sistema
elemental estiver implementado e houver dados de playtest.

---

## 6. Awakening

Itens ganham estágios visuais conforme o portador os usa — contagem de kills
com o item equipado, não por tipo específico de mob (evita "matar 10 vacas com
a espada lendária").

| Estágio | Threshold (a sintonizar) | Mudança visual |
|---|---|---|
| Dormente | recém-forjado | aparência padrão |
| Desperto | kills suficientes | gema muda de cor, brilho sutil |
| Lendário pessoal | threshold alto | item ganha segunda linha de nome |

O título "Matador de Dragões" e similares vêm do **sistema de conquistas/títulos**
(separado), não do item. Os dois se complementam — o item acumula história, o
personagem acumula reputação — mas nenhum força o outro.

---

## 7. Coerência com Entropia

- **Void Essences** (socket especial de tier alto) só dropam de mobs de Voidrift
  quando o mundo está em Caos.
- **Sombra/Ausência** como gem conecta à lore de Aigla: o Vácuo que permitiu
  os portais de conquista é o mesmo material que o Encantador canaliza.
- **Eclipse** (Sombra + Luz) é o ato de repetir a conquista de Aigla num item
  pessoal — poderoso, mas com custo entrópico.
- Itens forjados na Forja Real de uma Baronia em mundo ordenado têm proveniência
  narrativa natural (sem tracking complexo — o nome do crafter já existe no
  sistema UO de "crafted by").

---

## 8. Coerência com Reinos

- **Forja Real**: prédio de Baronia, desbloqueado por Erário + construção.
  Fornece acesso exclusivo a Lendários e Void Essences.
- **Biblioteca**: dropa/vende receitas raras, conecta craft ao loop de Baronia.
- **Área Rural**: produz materiais comuns em escala (materiais T1–T2 em volume).
- **Materiais exclusivos de bioma**: cada Baronia tem acesso a materiais que só
  existem naquele território — força comércio e aliança entre reinos.

---

## 9. O loop redesenhado (de ponta a ponta)

> **Ouvir rumor sobre boss** (facção destravou a dungeon) →
> **Planejar a hunt** (grupo, horário, estado de Entropia?) →
> **Matar boss nomeado** → **drop de material único** →
> **Levar à Forja Real** (Baronia aliada) →
> **Crafter forja item nomeado** com sockets configurados →
> **Guerreiro encaixa gems** (coletadas em runs anteriores) →
> **Usar o item até Awakening** (sem pressão de mob específico) →
> **Descobrir synergy** (Plasma, Eclipse, etc.) →
> **Virar alguém que tem aquele item**

Cada passo é uma decisão, não uma tarefa. O item no final não é "Lendária com
3 sockets". É a *Fang Blade de Chk'Lykk, forjada por [nome] na Baronia de [X],
com Eclipse descoberto*.

---

## 10. Pendências (a resolver em specs subsequentes)

- **Nomes finais das 6 gems** (decisão de lore, não de sistema)
- **Synergy Trevas+Luz ou Trevas+Gelo** (segunda proibida ou combo elemental?)
- **Coerência do Alquimista** para materiais ultra-raros de socket (a definir
  após estrutura principal implementada)
- **Lore de Energia** (cyclopes como fonte é incoerente — a corrigir no bestiary)
- **Taxa de desgaste** (calibragem de playtest para ciclo econômico)
- **XP por tipo de item** (Ferreiro que forja 50 espadas desbloqueia versões
  melhores) — incorporar neste sistema ou spec separada?
- **Gems em itens mágicos** (cajados, orbes): lógica de amplificação de spell
  a detalhar

---

## Inspirações

Monster Hunter (mob = item, hunt com alvo claro, carve satisfatório) ·
Dark Souls (Boss Soul weapons, item como trofeu narrativo) ·
Diablo 2 (Runewords como descoberta, items únicos com nome próprio) ·
Path of Exile (camadas de modificação, profundidade administrável)
