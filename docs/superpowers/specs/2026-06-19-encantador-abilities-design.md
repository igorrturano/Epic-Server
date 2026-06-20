# Habilidades do Encantador — Design

*2026-06-19. Primeira vocação detalhada do rework. Fonte de verdade do sistema:
[2026-06-17-vocation-system-design.md](2026-06-17-vocation-system-design.md).
Esta spec só desenha o **kit de habilidades** do Encantador; a migração de
personagens e o motor de progressão (escolher 1 árvore por nível) pertencem ao
rework geral de vocações, não a este documento.*

---

## O modelo em uma frase

O Encantador tem **3 árvores — Ofensivo / Defensivo / Suporte** (o padrão da
casa). A **regência travada na criação** não é a árvore: ela é a **face** de toda
habilidade. Existe **1 kit lógico** (15 conceitos), e cada conceito é escrito em
**3 faces** (Vocal/fera · Musical/arma · Profana/morto). O jogador vive 15
habilidades; existem 45 frases, mas é o mesmo kit reskinado.

### A regra que sustenta tudo

> **1 habilidade = 1 mecânica única.** As 3 faces são só **vibe** (nome +
> descrição), resolvidas pelo tipo de regência do jogador. O efeito —
> dano, duração, alvo, número — é **idêntico** nas três. O lobo, a arma e o
> esqueleto fazem exatamente o mesmo; só a frase que aparece muda.

Isso é o princípio #3 do doc-fonte ("a arma detecta a face") aplicado ao proxy:
**o tipo de proxy detecta a face da habilidade.** Balanceia-se *um* número, com
três peles. O "necrobardo" **emerge** de como o jogador mistura Off/Def/Sup
dentro da sua regência — nunca como combo mecânico (Regência trava em 1, proxy é
1 por vez).

---

## Backbone (concedido pela vocação na criação — todas as regências)

Não é árvore; é a base que existe antes de qualquer pick:

- **1 proxy, sempre 1.** Segunda trava anti-OP, além de Regência ser fonte ≤1.
- **Aquisição por regência:**
  - **Vocal** doma a fera com **Taming** (cívica).
  - **Musical** acha / compra / nasce com a arma regida.
  - **Profana** precisa de um **corpo**: espírito (bicho morto) → esqueleto →
    zumbi, conforme o nível de Regência.
- **Interface de comando** (herda e generaliza a antiga *Comando de Feras*):
  ordens básicas **Atacar / Proteger / Recuar / Emboscar / Farejar**. Vale igual
  pras 3 regências.
- **A força do proxy escala com Regência.** O proxy é troca de stats/flavor; o
  *poder* vem do seu nível de Regência, não do bicho.
- **Regência só rege NPCs, NUNCA Players.**

---

## Recurso — Medidor de Comando

Princípio #7 do doc: recurso é **medidor enchido agindo**, nunca inventário.

- **Enche** enquanto o proxy está engajado (causa/recebe dano) e, devagar,
  passivamente enquanto há um proxy ativo.
- **Decai / não gera** quando você está sem proxy — reforça a fantasia: *sozinho
  você é pouco; tudo passa pelo proxy.*
- **Gasta** nas habilidades **Ativas** das árvores (custo baixo nos tiers 1/3,
  alto nos tiers 7/10). As **Passivas** são de graça.
- **Capacidade e geração escalam com Regência.**
- Capstones (tier 10) também têm `delay` longo como segundo portão, no padrão do
  `habilidades.cfg` atual.

---

## Progressão — escolher 1 das 3 por tier

A vocação dá um pick em cada breakpoint de nível **1 / 3 / 5 / 7 / 10**. Em cada
breakpoint o jogador escolhe **a árvore**; o **tier é fixado pelo nível**. Ao
chegar no 10, você tem **5 habilidades** (uma por tier), na sequência de árvores
que você escolheu. Build = sua sequência de escolhas (3⁵ combinações).

**Decisão de design (muda em relação aos kits antigos):** as habilidades são
**autocontidas** — sem `requisito` cruzando tiers da mesma árvore. O único portão
é o nível. Isso é o que torna o build emergente possível (você pode pegar
Defensivo no nível 1 e Ofensivo no 3 sem travar). Os kits lineares antigos
(1→3→5→7→10 com `requisito` encadeado) são substituídos por este leque.

---

## As 15 habilidades

Notação por slot: **tier (nível) · tipo · mecânica única** seguida das 3 faces
(`nome` — descrição). Números exatos ficam pro passe de balanceamento; o que está
fixado aqui é a **mecânica, o alvo e a intenção**.

### 🗡️ Ofensivo — *o proxy bate / burst*

**O1 (nível 1) · Ativa · custo Comando baixo**
Próximo ataque do proxy: **+dano e derruba** (knockdown curto).
- *Vocal* — **Bote Dilacerante**: seu lobo dá um bote e derruba o alvo.
- *Musical* — **Lâmina Giratória**: sua arma gira num arco e derruba o alvo.
- *Profana* — **Investida Esmagadora**: seu morto avança e esmaga o alvo no chão.

**O3 (nível 3) · Ativa · custo Comando baixo**
**Investida** — o proxy fecha distância até o alvo e bate (gap-closer + dano).
- *Vocal* — **Disparada**: a fera dispara sobre a presa.
- *Musical* — **Arco Cortante**: a arma cruza o ar num talho.
- *Profana* — **Arremesso Profano**: o morto se lança sobre o alvo.

**O5 (nível 5) · Passiva**
O proxy ganha **crítico escalando com Regência** contra alvos **debuffados ou
abaixo de X% de vida**. (Sinergia direta com a árvore Defensiva.)
- *Vocal* — **Instinto da Caça** · *Musical* — **Gume Afiado** · *Profana* —
  **Sede de Carniça**.

**O7 (nível 7) · Ativa · custo Comando alto + delay**
**Frenesi** — o proxy entra em frenesi: +velocidade de ataque e ataques extras
por (Regência / N) segundos.
- *Vocal* — **Fúria da Matilha** · *Musical* — **Staccato de Guerra** · *Profana*
  — **Frenesi Necrótico**.

**O10 (nível 10) · Ativa · capstone · delay longo**
**Caçada Implacável** — marca o alvo; o proxy **ignora as defesas** dele e o
persegue até matá-lo ou expirar. *(Minera a antiga "Caçador Implacável".)*
- *Vocal* — **Presa Marcada** · *Musical* — **Sentença Afinada** · *Profana* —
  **Perseguição Implacável**.

### 🛡️ Defensivo — *debuff = enfraquece o inimigo, logo te protege*

**D1 (nível 1) · Ativa · custo Comando baixo**
Inimigo recebe **medo**: −dano causado + chance de fuga/hesitação por Y seg.
*(Minera a antiga "Inflingir Medo".)*
- *Vocal* — **Rosnado Intimidante** · *Musical* — **Acorde Desmoralizante** ·
  *Profana* — **Inflingir Medo**.

**D3 (nível 3) · Ativa · custo Comando baixo**
**Quebra de Guarda** — alvo com **−resistência** por Y seg (abre pra dano).
- *Vocal* — **Mordida Lacerante** · *Musical* — **Dissonância** · *Profana* —
  **Corrosão Profana**.

**D5 (nível 5) · Passiva**
**Aura** — inimigos adjacentes ao proxy **atacam mais devagar / −precisão**.
- *Vocal* — **Presença Selvagem** · *Musical* — **Réquiem** · *Profana* —
  **Aura de Pavor**.

**D7 (nível 7) · Ativa · custo Comando médio**
**Agrilhoar** — o proxy prende o alvo: **root/slow** por Y seg (CC).
- *Vocal* — **Encurralar** · *Musical* — **Nota Suspensa** · *Profana* —
  **Garras da Cova**.

**D10 (nível 10) · Ativa · capstone · delay longo**
**Colapso** — em área: inimigos com **−resist −dano + terror** por Y seg.
- *Vocal* — **Uivo Aterrorizante** · *Musical* — **Crescendo Sombrio** · *Profana*
  — **Praga do Colapso**.

### ✨ Suporte — *buff no proxy / em si*

**S1 (nível 1) · Ativa · custo Comando baixo**
Proxy ganha **+X% de dano** por Y seg.
- *Vocal* — **Uivo de Guerra** · *Musical* — **Marcha de Bravura** · *Profana* —
  **Aura Profana**.

**S3 (nível 3) · Passiva**
**Elo Vital** — o proxy cura uma fração do dano que causa (lifelink); parte
reverte pra você. *(Minera a antiga "Elo Natural".)*
- *Vocal* — **Vínculo Selvagem** · *Musical* — **Harmonia Vital** · *Profana* —
  **Dreno Compartilhado**.

**S5 (nível 5) · Passiva**
**Vínculo Reforçado** — o proxy escala **HP e dano com seu nível de Regência** (o
"fica mais forte com você"). *(Minera a parte de stats do antigo "Despertar
Selvagem".)*
- *Vocal* — **Despertar** · *Musical* — **Ressonância** · *Profana* — **Ascensão
  Profana**.

**S7 (nível 7) · Ativa · custo Comando médio + delay**
**Segundo Fôlego** — cura o proxy; se ele estiver morto, **reergue/reinvoca** com
cooldown. É o botão de sobrevivência da pet-class.
- *Vocal* — **Trato Selvagem** · *Musical* — **Refrão Restaurador** · *Profana* —
  **Reanimar**.

**S10 (nível 10) · Ativa · capstone · delay longo**
**Ápice** — o proxy ganha **stats massivos** (tamanho/HP/dano) por Y seg, e você
ganha **mitigação** enquanto durar. O grande momento da classe.
- *Vocal* — **Forma Primordial** · *Musical* — **Sinfonia Final** · *Profana* —
  **Forma Abissal**.

---

## Reaproveitamento dos kits antigos

Os kits velhos eram **3 classes separadas**, logo cheios de profundidade
*específica de uma regência*. O modelo novo quer o oposto (mecânica idêntica nas
3). Por isso quase nada transfere como mecânica — mas servem de **pedreira de
flavor**.

**Levantado pro backbone:** *Comando de Feras* → interface de comando universal.

**Minerado como efeito/face:** *Inflingir Medo* (face Profana de D1) · *Elo
Natural* (S3) · stats do *Despertar Selvagem* (S5) · *Caçador Implacável* (O10) ·
canções de buff/debuff do Bardo (faces Musicais) · nomes de capstone
(*Sinfonia Final*, *Forma Primordial*, *Forma Abissal*).

**Cortado (quebra o modelo ou as regras):** *Cetro da Morte* (stack de maldição)
· *Lich / filactério* (dropado) · multi-pet (*Vínculo de Guerra* extra,
slots da *Espreita Selvagem*) — fere "1 proxy" · *Sacrifício da Carne / Kaijin*
(isso é cast = Magus agora) · *Controlar Mortos* (sequestrar mortos só faz
sentido em Profana, não espelha) · todo o **sistema de cânticos** do Bardo
(profundidade só-Bardo).

Regra: **não force a habilidade velha no slot.** Os 15 slots são desenhados
limpos; o saque de flavor vem depois.

---

## Fronteira com outras vocações — conjurar ≠ comandar

Princípio #9 do doc-fonte. O Encantador é o **único** que comanda um proxy leal e
persistente. As outras fontes só ganham a versão transiente:

- **Magus de Necromancia (Arcana):** lança magia da morte (dreno/maldição/raio
  negro) — *é* a magia. Sem proxy leal.
- **Encantador Profana (Regência):** ergue e **comanda** 1 morto. Sem spellbook.
- **Druida (Nature):** comunga/vira a fera (shifter), invoca espírito **transiente
  que expira**, acalma bestas — mas **não comanda** companheiro leal.
- **Caçador de Feras (Encantador Vocal):** doma e comanda 1 fera leal e
  escalável.

Duas fantasias por par, zero sobreposição mecânica.

---

## Ganchos futuros (não implementar agora)

- **Regência ↔ Entropia.** Reger o sem-mente é impor vontade sobre o
  livre-arbítrio do proxy — manipular a vontade alheia. Isso conversa com o
  sistema de **entropia** do mundo (o systems-map já anota "entropia religada à
  Regência"): usar Regência pode gerar/alterar entropia. A casar com o sistema de
  utopia/entropia num passe dedicado.
- **Druida (Warden) — invocação como salvaguarda.** Reforça o princípio #9: o
  summon do Druida é um chamado de socorro **transiente** — o bicho aparece,
  recebe dano por você e some. Não é proxy comandado. Pertence ao brainstorm do
  Guardião, não a esta spec.

## Pendências (fora do escopo desta spec)

1. **Números de balanceamento** — dano, durações, custos de Comando, escalas com
   Regência. Passe dedicado depois do esqueleto aprovado.
2. **Resolução de face na engine** — `habilidades.cfg` hoje tem 1 `desc` por
   `hab`. Para 3 faces por conceito, decidir entre: (a) 1 conceito + lookup de
   `nome/desc` por regência, ou (b) 3 entradas `hab` que compartilham mecânica.
   Decisão fica pro plano de implementação.
3. **Ataques inatos do proxy** — o proxy mantém ataques próprios (aranha
   empoçona, urso não) ou todo proxy do mesmo tipo bate igual? Doc-fonte já
   parkou; decidir na 1ª regência implementada.
4. **Migração** de personagens (Caçador/Bardo/Necromante atuais → Encantador) —
   pedra de migração, pertence ao `path-rework`.
5. **Motor de progressão** (pick 1 de 3 por tier) — pertence ao rework geral de
   vocações, não a este kit.
