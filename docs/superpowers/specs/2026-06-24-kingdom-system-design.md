# Sistema de Reinos — Design

*2026-06-24. Doc de visão de game design. Define o sistema de Baronias jogáveis
dentro dos Reinos Soberanos NPC do Epic. Companheiro de
[entropy-system-design.md](2026-06-23-entropy-system-design.md),
[vocation-system-design.md](2026-06-17-vocation-system-design.md) e do lore em
`epic-lore/nova-aigla-design.md`. Decisões fechadas via brainstorm dirigido;
números de calibragem ficam marcados como "a sintonizar no playtest".*

---

## A ideia em uma frase

Jogadores fundam **Baronias** dentro de Reinos Soberanos NPC, investem ouro num
**Erário coletivo** que financia construções e corte, e recebem em troca **acesso
exclusivo** a verbos de gameplay — não bônus de planilha.

---

## 0. Propósito central

O sistema de reinos é o **money sink primário** do Epic. Jogadores acumulam ouro
sem destino; o Erário é o buraco que consome esse ouro de forma satisfatória.

- **Ouro do jogador → Erário** (moeda coletiva, não é ouro)
- **Erário → prédios, corte, guarnição, eventos**
- **Prédios → acesso a verbos exclusivos** (crafting de tier alto, rituais,
  rotas comerciais)

Ouro não volta como ouro. Volta como **o que você consegue fazer**.

---

## 1. Hierarquia política

```
Reino Soberano NPC  (Aigla, Gulthrak, Har'oloth…)
    └── Baronia  ← foco do design, controlada por jogadores
            └── Estado / Mansão  (RP solo, não conquistável)
```

Jogadores não criam reinos soberanos — fundam **Baronias** dentro de um reino
NPC já existente no lore. Isso resolve o problema de população: com 50–200
players, 8 reinos soberanos concorrentes seria uma diluição fatal. Baronias
podem ser muitas; o teto é orgânico (território disponível × custo de fundação).

---

## 2. Fundação de uma Baronia

Três requisitos simultâneos:

1. **Território físico** — ocupar e manter controle de uma área no mapa por X
   dias sem ser expulso (a sintonizar no playtest).
2. **Custo de fundação** — depósito inicial em ouro no Erário nascente.
3. **Reconhecimento cultural** — reputação com pelo menos uma das duas facções
   daquela cultura (Aigla: Restauradores ou Pragmáticos; cada cultura tem seu
   par de facções com tensão mútua estilo Fallout — ter reputação com uma tende
   a custar reputação com a outra).

O título vem do **NPC Soberano** da cultura — coerente com o lore de Aigla
("título é favor divino concedido"). Uma Baronia começa com **Legitimidade 100**.

---

## 3. Legitimidade — 0 a 1000

A saúde política do reino, medida em pontos. É o único número que importa pra
determinar vulnerabilidade e prestígio.

### Fontes de ganho

| Evento | Ganho |
|---|---|
| Evento de reino vencido | +15 a +40 (escala pela dificuldade) |
| Prédio construído | +50 (único por prédio) |
| Território reclamado | +30 |
| Entropia global ≥ 0 (mundo em Ordem) | +2/semana passivo |

### Fontes de perda

| Evento | Perda |
|---|---|
| Evento de reino falho | −25 a −60 |
| Cargo vago na corte | −5/semana por cargo vazio |
| Entropia global negativa (mundo em Caos) | −5/semana |
| Sabotagem recebida | −50 a −150 (depende do alvo) |
| Inatividade (30+ dias sem ação) | −10/semana |

### Patamares

| Legitimidade | Estado | Consequência |
|---|---|---|
| 800–1000 | **Lendário** | NPC Soberano pode elevar título (Barone → Marchese) |
| 500–800 | **Consolidado** | Guerra Sancionada muito cara pra iniciar |
| 200–500 | **Estabelecido** | Guerra Sancionada possível |
| 1–200 | **Emergente** | Protegido de guerra, vulnerável a espionagem |
| 0 | **Interregno** | Baronia pode ser absorvida politicamente via Soberano |

---

## 4. Território e Materiais

Território controlado gera dois tipos de recurso — nenhum deles é ouro:

- **Materiais comuns em escala** — madeira, minério, tecido em volume superior
  ao que um jogador solo coleta. O reino "amplia" o que já existe no bioma.
- **Materiais exclusivos do bioma** — só existem no território daquele bioma,
  não dropam em coleta avulsa. Forçam comércio e visita entre reinos. Ex.: um
  minério que só existe no território mineiro de uma Baronia Badûran; uma planta
  que só cresce em territórios Bragoneses.

Bioma determina o material exclusivo. Escolher onde fundar a Baronia é uma
decisão de especialização permanente.

---

## 5. Construções — Sistema de Lotes

*Design da Cati, integrado 2026-06-25.*

### Lotes por Baronia

Cada Baronia tem **10 lotes** de três tamanhos:

| Tamanho | Quantidade |
|---|---|
| 30×30 | 2 |
| 20×20 | 3 |
| 15×15 | 5 |

O tamanho do lote determina o que cabe nele — a especialização é espacial, não
só numérica. Com 17 prédios no catálogo e 10 lotes (3 já ocupados pelos prédios
gratuitos), sobram 7 lotes livres para escolher de 14 opções: nunca dá pra ter
tudo.

### Prédios gratuitos (toda Baronia começa com estes)

| Prédio | Função |
|---|---|
| **City Hall** | Interface de gestão do reino — o objeto de acesso à gestão_use |
| **Taverna** | Ponto social, rumores, missões informais; gera renda de serviço |
| **Banco** | Serviços financeiros *(pode ser cortado em feudos — banco da cidade NPC supre)* |

### Catálogo de prédios construíveis

Prédio desbloqueia um **verbo** — uma ação que não existia antes no mundo, não
um bônus passivo. Receita de serviço vai integralmente ao Erário.

| Prédio | Lote | Receita | Verbo desbloqueado |
|---|---|---|---|
| **Quartel** | 20×20 | — | Recrutar e estacionar guardas pagos pelo Erário |
| **Prisão** | 20×20 | — | Prender jogadores e NPCs (sem prisão, guardas atacam mas não encarceram) |
| **Área Residencial** | 20×20 | aluguel | Housing — jogadores têm casa dentro do reino |
| **Área Comercial** | 20×20 | aluguel | Lojas de jogador, Ordens de Compra de reino |
| **Hospital** | ? | — | Tratar eventos de praga (domínio do Medicus); cura de campo |
| **Tribunal** | ? | — | Julgar crimes formalmente — liga ao Lictor e ao sistema de leis |
| **Correios** | ? | por carta | Comunicação formal entre reinos; cartas com taxa |
| **Casa de Leilões** | ? | taxa | Leilão público de itens entre jogadores |
| **Cemitério** | ? | aluguel | Rituais de ressurreição com custo reduzido |
| **Área Rural** | ? | produção | Agricultura e matérias-primas — gera materiais comuns em escala |
| **Templo** | 30×30 | — | Rituais de entropia em escala + cerimônias sociais (casamento, funeral): spawna itens da cerimônia e toca sinos no reino todo |
| **Biblioteca** | ? | assinatura semanal | Acesso a livros do mundo inteiro; card semanal |
| **Docas** | ? | por travessia | Barcos para travessias longas — mobilidade entre reinos costeiros |
| **Teatro** | ? | — | RP atmosférico: apaga luzes, acende palco, abre cortinas — Story Rich sem texto |

*Tamanhos de lote dos prédios ? a definir com a Cati na implementação.*

### Economia dos prédios — Erário bidirecional

O Erário não é só um buraco: tem **dois fluxos**.

- **Entrada passiva:** aluguel (residências, comércio, cemitério) + taxas de
  serviço (correios, biblioteca, docas, taverna)
- **Saída recorrente:** manutenção de prédios + salários da corte + guarnição

Money sink na **construção** (custo alto único); semi-autossustentável depois
disso. Reino bem construído e ativo se paga; reino negligenciado sangra o Erário
e a Legitimidade juntos.

---

## 6. Corte — 9 cargos com atribuição ativa

O líder da Baronia (o Barão) **nomeia jogadores** para cargos. A skill do
jogador nomeado entra nas rolagens de eventos semanais do cargo correspondente.
Cargo vago = −5 Legitimidade/semana + evento daquele domínio tem dificuldade
aumentada.

Nomes do lore de Aigla (cada cultura pode adaptar o vocabulário):

| Cargo | Domínio | Skill principal |
|---|---|---|
| **Quaestor** | Economia / tesouro | Smithing, Mining, Provisioning, Tailoring, Carpentry |
| **Legatus** | Militar | Tactics + skills de combate |
| **Aedilis** | Obras e infraestrutura | Smithing, Carpentry, Mechanics |
| **Lictor** | Lei e ordem / contra-espionagem | One_Handed, Parry, Perception |
| **Augur** | Sagrado / entropia | Theology |
| **Medicus** | Saúde e saneamento | Medicine, Science |
| **Orator** | Diplomacia | (qualidade do jogador — sem skill única) |
| **Vilicus** | Administração diária | Provisioning, Survival, Nature |
| **Speculator** | Espionagem | Stealth, Theft, Perception |

*Arcana/Magery não entram na corte diretamente — magia é domínio regulado pelo
Tribunal Arcano no lore de Aigla.*

---

## 7. Eventos Semanais — Motor de Conteúdo

### Princípio: Story Rich ≠ Text Rich

O mundo conta a história — o jogador não lê sobre um problema, ele *encontra*
o problema no mundo. Sem telas de burocracia, sem notificações de stat.

### Fluxo passivo (padrão)

A cada semana o reino resolve **1–3 eventos** automaticamente. O cargo relevante
faz a rolagem silenciosa: `skill do conselheiro + d100 vs DC do evento`. O
jogador nunca vê essa rolagem acontecer.

- **Rolagem bem-sucedida:** nada muda. O mundo continua estável.
- **Rolagem falha:** o mundo muda. O jogador *descobre* o problema explorando.

| Cargo que falhou | Conteúdo que spawna no mundo |
|---|---|
| Lictor | Acampamento de bandidos no território da Baronia |
| Medicus | NPC portador de praga aparece na cidade |
| Augur | Rift não-purgado começa a corromper área do território |
| Legatus | Deserção de guarnição, posição de fronteira exposta |
| Speculator | Agente inimigo infiltrado, evento de sabotagem iminente |
| Quaestor | Crise de recursos — produção cai por X dias |

**Os jogadores resolvem o conteúdo no mundo → problema some. Ignoram →
Legitimidade cai.**

### Modo narrativo (opt-in)

O Barão pode ativar **modo narrativo** para qualquer cargo. Quando um evento
daquele domínio é sorteado, uma notificação curta aparece *antes* da rolagem
com uma situação e 2–3 escolhas:

> *"Um viajante encapuzado chegou às muralhas pedindo abrigo. Diz ser mercador
> de Aigla."*
>
> **[ Dar abrigo ]  [ Barrar a entrada ]  [ Interrogar primeiro ]**

A escolha **não substitui a rolagem** — ela define o *perfil da consequência*:

| Escolha | Sucesso na rolagem | Falha na rolagem |
|---|---|---|
| Dar abrigo | Espião entrega intel sobre reino rival | Espião rouba segredos do reino |
| Barrar entrada | Risco evitado, nada acontece | Incidente diplomático com origem do espião |
| Interrogar | Identidade revelada, vira aliado | Ele foge e avisa o rival |

O resultado não é memorizável: a escolha define o *risco*, não o *resultado*.
Escolher "Interrogar" é sempre mais arriscado e mais recompensador que "Barrar"
— é tolerância ao risco, não decoreba da resposta certa.

### Auto-management

Barões que não querem gerenciar eventos manualmente deixam tudo no **modo
passivo**. O sistema rola silenciosamente e o mundo muda sem interrupção. O
Barão presente em modo narrativo tem vantagem marginal sobre o Barão ausente —
não uma punição ao absenteísmo, mas um bônus ao engajamento.

A dificuldade dos eventos escala com o **estado de entropia global**:
- Mundo em Ordem → DCs menores, eventos mais manejáveis
- Mundo em Caos → DCs maiores, eventos mais severos e frequentes

---

## 8. Sabotagem — Espionagem Ativa

Ação executada pelo **Speculator** (ou qualquer jogador com Stealth/Theft alto)
que vai fisicamente ao reino-alvo e executa a missão. É um **money sink
secundário** — custo alto em ouro proporcional ao alvo.

### Alvos e custos (a calibrar no playtest)

| Alvo | Efeito | Custo relativo |
|---|---|---|
| Prédio específico | Desativa o verbo por X dias | Médio |
| Erário | Drena ouro do tesouro coletivo | Alto |
| Evento provocado | Força um evento negativo na próxima semana do alvo | Médio |
| Legitimidade direta | Ataque à reputação junto ao Soberano NPC | Muito alto |

### Risco e contraplay

- O **Lictor** do reino-alvo pode detectar o Speculator se sua Perception for
  suficientemente alta → cria o loop **espião × contra-espionagem**
- Se detectado: incidente diplomático com o NPC Soberano, perda de reputação
  com a facção, possível prisão do personagem
- O investimento em Lictor forte é o trade-off direto contra sabotagem

---

## 9. Conflito — Escada, Nunca Abrupto

O **NPC Soberano é o árbitro**. Barões brigando desordenadamente envergonha o
reino soberano — conflito exige escalada dentro das regras.

### Três níveis de conflito

1. **Espionagem** — sempre permitida, sem declaração. Sabotagem, roubo de
   informação, infiltração.
2. **Saque** — permitido em territórios não-protegidos (fronteira, ruínas,
   postos não-reclamados). Nunca na cidade principal de uma Baronia ativa.
3. **Guerra Sancionada** — requer petição ao NPC Soberano. Custos:
   - Ouro (custo diplomático)
   - Reputação com a facção
   - Ambos os lados precisam ter Legitimidade ≥ 200

### Fações com tensão Fallout

Duas facções por cultura têm tensão mútua. Isso modula o custo de guerra:
- **Baronias de facções opostas** → Guerra Sancionada mais fácil e barata de
  aprovar (o Soberano enxerga como conflito ideológico legítimo)
- **Baronias da mesma facção** → Guerra Sancionada muito cara; a facção penaliza
  o agressor

### Resolução gradual — nunca abrupta

Guerra Sancionada corrói Legitimidade ao longo de semanas. Chegar a 0:
- → **Vassalagem**, não deleção. O derrotado mantém corte, prédios, identidade,
  mas paga tributo ao vencedor
- → Rebelião pra recuperar independência é o próximo arco de gameplay

**Reinos abandonados** (inatividade 30+ dias): Legitimidade drena
automaticamente. Abaixo de 100, qualquer Barão pode peticionar absorção ao
Soberano NPC via processo político — sem guerra necessária.

### Anti-troll embutido

O Barão que falha todos os eventos intencionalmente drena a própria Legitimidade.
Se a Baronia permanecer **abaixo de 100 por 4 semanas consecutivas sem nenhum
evento vencido**, o Soberano NPC revoga o título por incompetência → território
entra em interregno. A janela de 4 semanas protege o jogador legítimo que sofreu
sabotagem pesada; pune só negligência crônica. O troll perde o reino pra si
mesmo, não pra outro jogador.

---

## 10. Conexão com Entropia

O sistema de reinos é o "gancho futuro" descrito na
[entropy-system-design.md](2026-06-23-entropy-system-design.md) §13, agora
especificado:

| Estado global | Impacto nos Reinos |
|---|---|
| **Mundo em Ordem (+)** | DCs dos eventos caem; Legitimidade ganha +2/sem passivo; reinos prosperam naturalmente |
| **Mundo em Caos (−)** | DCs sobem; Legitimidade perde −5/sem; eventos graves ficam mais frequentes; pragas e rifts mais severos |
| **Caos extremo (< −60k)** | Eventos de reino incluem spawns de void diretamente no território |

Isso cria a motivação dos Barões pela Ordem: um mundo caótico é literalmente
mais caro de governar. Os Barões são aliados naturais dos Devotos no eixo
Ordem×Caos — e inimigos naturais dos Encantadores que se beneficiam do Caos.

---

## 11. Títulos de Nobreza (lore de Aigla)

Progressão pelo número de territórios controlados e Legitimidade acumulada:

| Territórios + Legitimidade | Título (Aigla) | Equivalente |
|---|---|---|
| 1 território | **Dominus / Domina** | Senhor(a) de um domínio |
| 2–3 territórios | **Barone / Baronessa** | Barão(esa) |
| 4–5 territórios + 800+ Legit | **Marchese / Marchesa** | Marquês(a) |
| 6 territórios + 950+ Legit | **Signore / Signora** | Rege uma região inteira |

*Cada cultura adapta o vocabulário mantendo a espinha de progressão. Pequilhos,
Gulthrak, Bragoneses terão seus próprios títulos equivalentes.*

---

## 12. Reuso de código existente

O sistema se apoia em código que já existe em `pkg/systems/faccao/`:

- `gestao_use.src` — UI de gestão já tem abas Reino/Leis/Facções/Construções/
  Guerra (hoje stubs TODO); este spec preenche o conteúdo dessas abas
- `soldocontrol.src` — folha de pagamento já existe; cargos da corte usam esse
  sistema para salários pagos pelo Erário
- `ferramentas_construcao.src` — base para construção de prédios no território
- `architect/areas` — datafile de territórios já usado na aba Reino

Refactor de unificação, **não rewrite do zero**.

---

## 13. Escopo e fatiamento

Este é um doc de visão. Quando for implementar, fatiar em specs menores:

- **Fatia 1 — Legitimidade + Erário:** o medidor e a moeda coletiva; fundação
  de Baronia; cidadania/juramento
- **Fatia 2 — Corte + Eventos:** 9 cargos com atribuição; relógio semanal de
  eventos; spawn de conteúdo por falha
- **Fatia 3 — Construções:** catálogo completo (definir com Cati); sistema de
  slots; verbos desbloqueados
- **Fatia 4 — Território e Materiais:** claims de área; geração de materiais
  comuns e exclusivos por bioma
- **Fatia 5 — Conflito e Sabotagem:** escada espionagem→saque→guerra; loop
  Speculator×Lictor; vassalagem; anti-troll
- **Fatia 6 — Integração com Entropia:** DCs variáveis por estado global;
  eventos de void em território; títulos de nobreza

Números (custo de fundação, DCs dos eventos, pontos de Legitimidade por ação,
custo de sabotagem, tempo de inatividade) são **calibragem de playtest** — não
travam a visão.

---

## Pendências (fora do escopo desta spec)

- Vocabulário de títulos por cultura (Gulthrak, Bragonesa, Har'oloth, etc.)
- Mecânica in-game de facções por cultura (as 2 facções e sua tensão mútua)
- Sistema de "rotas comerciais" entre Baronias (Porto)
- Mecânica de ressurreição via Cemitério
- Integração com sistema de construção física no mapa (architect)
- Migração de personagens com títulos existentes (chardata.current_nobility_title)
