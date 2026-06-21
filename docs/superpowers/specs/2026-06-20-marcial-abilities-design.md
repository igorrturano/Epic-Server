# Habilidades do Marcial (Fighter / Guerreiro) — Design

*2026-06-20. Quinta vocação detalhada do rework. Fonte de verdade do sistema:
[2026-06-17-vocation-system-design.md](2026-06-17-vocation-system-design.md).
Irmã das specs de Encantador, Warden, Devoto e
[Ladino](2026-06-20-ladino-abilities-design.md). Esta spec desenha o **kit**;
migração e motor de progressão pertencem ao rework geral.*

---

## O modelo em uma frase

O Marcial (skill-fonte **Tactics**) é o guerreiro mundano — **fora da Radiância**
(sem fonte mágica, sem Casting). 3 troncos honestos Off/Def/Controle, e — a marca
da vocação — **habilidades agnósticas de arma**: a mesma habilidade funciona com
espada, arco ou punho. *Menos é mais.*

| Árvore | Papel | Identidade | `classe` cfg |
|---|---|---|---|
| **Bárbaro** | Ofensivo | fúria, dano, frenesi | Guerreiro |
| **Defensor** | Defensivo | tanque, provocar, postura, contra-ataque | Guerreiro |
| **Combatente** | Controle | desarme, stun, golpes técnicos | Guerreiro |

---

## Fundamentos (o que trava o Marcial)

- **Special ability da classe: Ataques Especiais** — concedida na criação
  (hotbar, não pick de árvore): os golpes especiais de arma.
- **Fonte = Tactics**, fora da Radiância — nunca usa Casting nem trava fontes.
- **Habilidades agnósticas de arma.** A regra: cada habilidade é **agnóstica**
  (funciona com qualquer arma), com **face por arma** só onde a expressão muda de
  verdade, e **específica de arma** só quando é intrínseca. Sem matriz cheia, sem
  habilidade-enchimento (regra do aluguel).
- **Técnica específica de arma mora no Caminho, não no tronco.** Multishot, aimed
  shot, half-sword etc. são **Especialização de Arma** (`CAMINHO - Weapon Specs` /
  `Fight Styles`, já no cfg) — **abertas a todas as vocações**. "Ser arqueiro" =
  qualquer vocação + arco; o Marcial-arqueiro é o kit Ofensivo empunhando arco.
- **Samurai / Monge / Arqueiro dissolvem.** Não são troncos nem kits: são
  **vocação (poder) + arma/spec (skill) + gear (RP)**. Veste oriental + katana (ou
  mãos nuas) e roleplaya samurai/monge **usando o mesmo kit**. É a tese do rework.
- **Recurso = `stam + delay` + textura de Postura.** Os kits atuais já alternam
  *postura ofensiva × defensiva* (Defensor, Combatente) — essa **stance** é a
  textura do Marcial: alternar muda o comportamento das habilidades (decisão, não
  tarefa). Sem medidor/barra.

---

## Progressão

Pick em cada breakpoint **1 / 3 / 5 / 7 / 10**; escolhe a árvore, tier fixado pelo
nível; habilidades autocontidas. Build emerge.

---

## As 3 árvores (reorganizadas dos kits melee, classificadas por arma)

Legenda: **[A]** agnóstica · **[F]** face por arma · **[E]** específica de arma.

### 🪓 Bárbaro — Ofensivo (fúria)
Quase todo o tronco é estado de fúria → naturalmente **agnóstico**.
- Fúria **[A]** (entra em fúria: +dano/duração por Tactics) · Resiliência Bárbara
  **[A]** (ignora dor, +resist físico, cura ativa) · Fúria Indomável **[A]**
  (passiva: aterroriza, imune a crit/paralisia, fortitude como vontade) · Fúria
  Implacável **[A]** (+HP, contra-ataca ao ser acertado) · Destruidor **[A]**
  (passiva: +dano e +crit em fúria).

### 🛡️ Defensor — Defensivo (tanque / postura)
- Provocar **[A]** (taunt; esquiva escala com nº de alvos) · Manobra de Defesa
  **[A]** (postura: defende próximo golpe / +parry) · Manobra Desafiadora **[F]**
  (com escudo: bash+stun; sem: luta melhor contra vários) · Manobra da
  Resiliência **[A]** (off: imune a stun/crit; def: regen) · Manobra da
  Invencibilidade **[A]** (off: contra-ataca; def: foco total em defesa).

### ⚔️ Combatente — Controle (técnico)
- Contra-Empunhadura **[A]** (dobra força ao dano; desarma ao defender) · Golpe
  Mortal **[E]** (half-sword: ignora armadura + stun — melee) · Contragolpe
  Cruzado **[A]** (garante defesa + contra-ataca) · Golpes Furiosos **[A]**
  (golpes extras por TT) · Sentença de Morte **[A]** (críticos garantidos por TT).

> Maioria **[A]** — confirma que o Marcial é onde o agnóstico brilha. As poucas
> **[F]/[E]** são onde a arma realmente importa (escudo, half-sword).

---

## Expressões = gear + Especialização de Arma (não troncos)

- **Arco / desarmado / oriental** não são árvores — são **a arma que você
  empunha** + a **Especialização de Arma** (Caminho, aberta). O kit da vocação é o
  mesmo; a arma dá a face onde houver e a spec dá as técnicas próprias.
- **Reaproveitamento dos kits dissolvidos:**
  - **Arqueiro** (Tiro Poderoso, Disparo Múltiplo, Encontrar Brecha…) → **Weapon
    Spec de Arco** (Caminho), aberto a todos.
  - **Monge** (Artista Marcial, posturas) → **Wrestling spec** + faces desarmadas
    onde fizer sentido; o resto é RP oriental.
  - **Samurai** (Caminho do Andarilho, Fúria do Dragão…) → **flavor cultural Leste
    + gear**, sobre o kit compartilhado.

---

## Fronteiras com outras vocações

- **Marcial-arqueiro vs Warden-Predador (ambos podem usar arco):** a **vocação** dá
  a identidade — Marcial = fúria/técnica (Tactics); Predador = caça/marca (Nature).
  O arco é só a ferramenta, aberta aos dois.
- **Weapon Specs são de todos** (Caminho), não exclusivas do Marcial.

---

## Pendências (fora do escopo desta spec)

1. **Camada de Especialização de Arma (Caminho)** — confirmar que as técnicas
   específicas (Arqueiro/Wrestling/etc.) vivem lá e abertas a todos; mapear o que
   migra dos kits dissolvidos.
2. **Textura de Postura** — detalhar a stance ofensiva×defensiva como mecânica
   limpa (vários kits já dependem dela).
3. **Decompor habilidades sobrecarregadas** — algumas empilham função demais
   (mesmo problema apontado no Ladino); passe de limpeza próprio.
4. **Números** e **migração** (Bárbaro/Defensor/Combatente/Arqueiro/Monge/Samurai
   atuais).
5. **Mapa off/def/controle confirmado:** Bárbaro=Off · Defensor=Def · Combatente=
   Controle. Arqueiro/Monge/Samurai dissolvidos em arma+spec+RP.
