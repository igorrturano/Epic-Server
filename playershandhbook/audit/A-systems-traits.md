# Audit A — Sistemas & Características

Domain files:
- `src/10-parte1-sistemas.md` (Desmaio/Morte, Ficha RP, Fome, Pintura, Testes de Resistência)
- `src/71-caracteristicas.md` (Características / chardata.trait)

Codebase truth verified against `pkg/` (eScript, case-insensitive grep). Read-only audit.

---

## SISTEMAS — Desmaio e Morte

### Morte causada por jogador NÃO perde DP — falso
- **Handbook claim:** "No caso de sua morte ter sido causada por algum jogador, você não perderá Pontos de Morte." — src/10-parte1-sistemas.md:61
- **Codebase truth:** O bloco de perda de DP é explicitamente comentado "Perda de DP para TODAS as mortes (NPC e PvP)" e só isenta duelo, proteção de morte (`#phylactery`/`protecaomorte`/`savegame`) e GM. PvP comum perde DP igual a PvE. — pkg/mobiles/death/deathscripts/players.src:109-125
- **Mismatch:** A regra de "morte por jogador não custa DP" não existe no código atual; todo desmaio por outro player consome DP normalmente (10, ou 20 se golpe no pescoço/Neck).
- **Severity:** HIGH (mudança direta de risco em PvP; jogador acredita que pode desmaiar em PvP sem custo).
- **Cross-links:** none

### Pontos de Morte: máximo "fixado em quatro" — não corresponde
- **Handbook claim:** "A quantidade máxima desses pontos é fixada em quatro, salvo exceções..." e tabela DP 1–5 (src/10-parte1-sistemas.md:42-49) — src/10-parte1-sistemas.md:38
- **Codebase truth:** DP é um *vital* (`AP_GetVital(who,"DP")`), exibido como `vital/10`, com `GetDPMaximumValue = 50*100 + dpmod*100` — teto interno bem acima de 4/5. A descrição da Medicine vai de 0 a 8+ DP ("extremamente preparado (8 DP)") e a perda padrão por desmaio é 10 (1 DP exibido), 20 (2) em golpe no pescoço. — pkg/systems/attributes/hooks/vitalInit.src:233-235; pkg/skills/medicine/include/anatomy.inc:355-366; pkg/mobiles/death/deathscripts/players.src:119-125
- **Mismatch:** O máximo não é 4. A tabela da Medicine bate parcialmente (1=quase morto, 2=mal, 3=relativamente bem, 4=bastante disposto, 5=preparado) mas o livro afirma teto 4 e a linha "O alvo está mais do que preparado = 5+", quando o código segue até 8+ ("mais do que preparado"=6, "bem mais do que preparado"=7, "extremamente preparado"=8+). DP inicial e modificadores (dpmod ±30 via kit/morte) também extrapolam 4.
- **Severity:** MED (a tabela Medicine é majoritariamente correta; o "teto 4" e o "5+" induzem leitura errada do recurso).
- **Cross-links:** none

### Pontos de Morte legacy (counter incremental) é código morto
- **Handbook claim:** "a cada desmaio ele perderá um DP" (modelo de pool que decrementa) — src/10-parte1-sistemas.md:61
- **Codebase truth:** Existe um sistema antigo (`CheckDeathPoints`/`RegenerateDeathPoints`) que *incrementa* `deathpoints` a cada desmaio e penaliza stats em `deathpoints >= 4` — mas o sistema vivo de morte usa o vital "DP" que *decrementa*. O legacy não tem caller real além de `RegenerateDeathPoints` em dpCheck.src. — pkg/mobiles/death/include/death.inc:62-138; pkg/mobiles/death/dpCheck.src:9
- **Mismatch:** A direção descrita (perde 1 DP por desmaio) está correta para o sistema vivo (vital DP cai), mas há um sistema legacy contraditório no código. Não é mismatch de regra para o jogador; registrado para evitar confusão de leitura de código numa futura correção.
- **Severity:** LOW (não engana o jogador; nota de manutenção).
- **Cross-links:** none

### Regeneração de DP: "1 DP a cada hora"
- **Handbook claim:** "Um DP é recuperado a cada hora que você estiver jogando..." — src/10-parte1-sistemas.md:52
- **Codebase truth:** `GetDPRegenRate` retorna 17 ("1 DP por hora"), e 34 (≈2x) para quem tem a habilidade CONEXAO_DIVINA. — pkg/systems/attributes/hooks/vitalInit.src:225-231
- **Mismatch:** Claim correto na base; omite que CONEXAO_DIVINA dobra a regeneração. Não é regra invertida, apenas pré-requisito/exceção omitida (regen-rate, número abstraído).
- **Severity:** LOW (informação correta; exceção não citada).
- **Cross-links:** none

### Fraturas: dado "1d30 + Fortitude" e tabela 30/40/60/90 — mecânica incorreta
- **Handbook claim:** "O teste de resistência de fortitude tem dificuldade de 1d30 + Fortitude." + tabela: DP<5=30, <4=40, <3=60, <2=90 — src/10-parte1-sistemas.md:70-80
- **Codebase truth:** A fratura é decidida por `CheckResist(ghost, FORTITUDE, <const>)`, que rola **1d20 + valor de resistência** vs dificuldade fixa. Os limiares são por DP restante: DP<50→`RESIST_MEDIUM`(19); DP<40→`RESIST_HARD`(24); DP<30→`RESIST_VERY_HARD`(29); DP≤19→`RESIST_EXTREMELY_HARD`(34). — pkg/mobiles/death/deathscripts/players.src:128-148; pkg/systems/charactercreation/include/resists.inc:16-49; scripts/include/epicConstants.inc:97-103
- **Mismatch:** O dado é **1d20**, não 1d30, e a dificuldade é uma constante fixa contra a qual o jogador soma resistência — não "1d30 + Fortitude". As dificuldades reais (19/24/29/34) e os limiares de DP não correspondem à tabela do livro (30/40/60/90). A direção (mais DP perdido = mais difícil resistir) está certa, a mecânica e os números não.
- **Severity:** MED (a mecânica do teste é estruturalmente diferente; números abstraídos não seriam flag, mas dado e fórmula são regra).
- **Cross-links:** Testes de Resistência (mesma função `CheckResist`, ver abaixo); Sistema de Fome (mesmo `CheckResist` FORTITUDE).

### Assassinato: pré-requisitos 60 dias / avaliação / 2 DPs — não enforçados
- **Handbook claim:** "o assassino deve satisfazer: Possuir 60 dias ou mais; Ter avaliação nula ou positiva; Ter pelo menos dois DPs." — src/10-parte1-sistemas.md:86-92
- **Codebase truth:** O kill do desmaiado (cortar cabeça com arma cortante) só exige: `idadeChar(who) > 30` (30 dias, não 60), não estar em Story Mode, corpo não protegido por `#phylactery`, e confirmação YesNo. Não há checagem de nota/avaliação RP nem de quantidade de DP do assassino. O assassino perde `dpmod -= 10` (1 DP). — pkg/skills/survival/skinning.src:64-108
- **Mismatch:** (1) São **30 dias**, não 60. (2) Não existe gate de "avaliação nula ou positiva". (3) Não existe gate de "pelo menos dois DPs". Apenas a perda de 1 DP do assassino e a arma cortante conferem.
- **Severity:** HIGH (regras de elegibilidade de PK divulgadas como existentes mas inexistentes; afeta decisões e expectativas de jogo).
- **Cross-links:** none

---

## SISTEMAS — Ficha de Interpretação

### Sem ficha não evolui perícias — confirmado
- **Handbook claim:** "Um personagem sem ficha não será capaz de evoluir suas perícias." — src/10-parte1-sistemas.md:143
- **Codebase truth:** `GetSkillCapRegenRate` retorna 0 imediatamente se `!GetObjProperty(mobile,"fichaRP")`. — pkg/systems/attributes/hooks/vitalInit.src:241-271
- **Mismatch:** Nenhum. Claim correto.
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Nota RP influencia dificuldade das perícias — confirmado
- **Handbook claim:** "Essa nota irá influenciar na dificuldade das perícias, quanto melhor sua interpretação mais fácil será para subir suas perícias." — src/10-parte1-sistemas.md:143
- **Codebase truth:** `GetSkillCapRegenRate` retorna `50 + bonusRP*25 + nota*25 + foodbonus*15`; `nota` lido da conta. Nota maior = regen de SkillCap maior. — pkg/systems/attributes/hooks/vitalInit.src:262-270
- **Mismatch:** Nenhum (mecânica é via taxa de regen do vital SkillCap, não "dificuldade" literal, mas o efeito narrado — melhor nota sobe perícia mais fácil — está correto).
- **Severity:** — (verificado correto)
- **Cross-links:** none

---

## SISTEMAS — Sistema de Fome

### Limiar e magnitude das penalidades de fome — incorretos
- **Handbook claim:** "nível 0 e 1 é 'sem fome'... nível 4 a fome está prejudicando... nível 5 'faminto'. Penalidades pela fome 4: -10 em todos os atributos, -10 nos testes de resistência. Fome 5: -20 atributos, -20 testes de resistência." — src/10-parte1-sistemas.md:172-184
- **Codebase truth:** `FomeGetLimiar()` = **3**: o debuff começa em fome **3**, não 4. `FomeCalcDebuff(fome) = fome*10`: fome 3 = **-30**, fome 4 = **-40**, fome 5 = **-50** em STR/DEX/INT (`FomeApplyDebuff` mexe só nesses 3 stats). Não há penalidade flat separada "−X nos testes de resistência"; ao contrário, `ModStatFome` faz um *teste* de resistência Fortitude vs `RESIST_MEDIUM`(19) que, se passado, **anula** o debuff inteiro. Estados de fome: 0=muito bem alimentado, 1=cheio, 2=satisfeito, 3=com fome, 4=muita fome, 5=faminto. — pkg/items/food/include/hunger.inc:9-87; pkg/items/food/hunger.src:29-55; pkg/items/food/textcmd/player/fome.src:11
- **Mismatch:** (1) Debuff começa em fome 3, não 4. (2) Magnitudes erradas: -30/-40/-50 (não -10/-20). (3) "Sem fome" são níveis 0–2 (não só 0 e 1). (4) Não há "-X nos testes de resistência" como penalidade direta — resistências caem só indiretamente porque dependem de STR/DEX (intrinsicStats). (5) O debuff inteiro pode ser anulado por um teste de Fortitude — fato relevante omitido.
- **Severity:** HIGH (números E mecânica de gating divergentes; um jogador planeja alimentação assumindo penalidade leve a partir de fome 4, mas leva -30 já em fome 3, ou nada se passar no teste).
- **Cross-links:** mesma função `CheckResist`/Fortitude de Testes de Resistência e Fraturas.

### Comando .fome — confirmado
- **Handbook claim:** "5 níveis de fome, que podem ser vistas no comando .fome" — src/10-parte1-sistemas.md:172
- **Codebase truth:** `program fome(who)` existe, lê `AP_GetVital(who,"Hunger")/60` e exibe via `MSGFome`. — pkg/items/food/textcmd/player/fome.src:9-25
- **Mismatch:** Comando existe. (O texto exato de cada nível difere do livro, mas isso é prosa.)
- **Severity:** — (verificado correto)
- **Cross-links:** none

---

## SISTEMAS — Sistema de Pintura

### Fluxo de pintura — não verificável por código de sistema
- **Handbook claim:** Cavalete/Pincel/Paleta/Moldura/Tela/Tintas, "3 a 8 tintas", "quanto mais inteligência melhores opções". — src/10-parte1-sistemas.md:194-225
- **Codebase truth:** Não localizado um sistema de pintura/quadros sob `pkg/` no escopo deste audit (sem arquivo de pintura/canvas/cavalete identificado nas buscas de domínio).
- **Mismatch:** UNVERIFIED — não confirmado nem refutado pelo código acessível neste domínio. Não inventar.
- **Severity:** UNVERIFIED
- **Cross-links:** none

---

## SISTEMAS — Testes de Resistência

### Mecânica do teste (1d20, dificuldades fixas) e "segunda chance" oculta
- **Handbook claim:** "Cada teste possui uma dificuldade própria, o das magias é definido pelo Arcana do mago..."; "algumas magias completamente evitadas, outras metade; armadilhas sempre metade do dano." — src/10-parte1-sistemas.md:240
- **Codebase truth:** `CheckResist(defender, resist_type, dif, mod)` = **1d20** + `GetResist` (= `AP_GetStat` do resist) + mod vs `dif`. d20=1 falha crítica, d20=20 sucesso crítico. Há uma "segunda chance" não documentada (`ShouldGetSecondChance`): Drow/HAROLOTH+Fortitude, AURA_DIVINA+Fortitude, Anão+Will, qualquer Colhesiana, ou entropia ≤ −8000 + Fortitude rolam o teste duas vezes. O efeito de meio/total evitado é decidido por cada feitiço/armadilha (caller), não por `CheckResist`. — pkg/systems/charactercreation/include/resists.inc:16-78
- **Mismatch:** A descrição do *efeito* (evitar total/metade, dificuldade da magia via Arcana, armadilha metade do dano) não pôde ser confirmada como universal aqui — é regra por-feitiço (fora do domínio estrito; marcar como parcialmente UNVERIFIED para a parte de magia/Arcana). A omissão concreta e relevante: a mecânica de **segunda rolagem** para certas raças/culturas/abilities/entropia, que altera materialmente a chance de resistir e não é mencionada.
- **Severity:** MED (segunda-chance omitida muda probabilidade real de resistir; parte mágica UNVERIFIED).
- **Cross-links:** Fraturas e Sistema de Fome usam o mesmo `CheckResist`.

### Fórmula do Valor da Resistência (Caminho +20/+10, +1 por 4 atributo) — incorreta
- **Handbook claim:** "O bônus do Caminho... é de 20 para a resistência principal e 10 para os demais. Para cada 4 pontos em um atributo o personagem ganha 1 da resistência equivalente." + Exemplo: Caminho das Armas, 100 FOR, 70 DES → 45 Fortitude, 27.5 Reflexos, 20 Vontade. — src/10-parte1-sistemas.md:251-256
- **Codebase truth:** Em `intrinsicStats.inc`: bônus de classe é **+5** (classe casada: Guerreiro→Fortitude, Ladino→Reflexos, Sábio→Vontade) ou **+2** (demais). Atributo contribui `AP_GetStat/20` (1 por **20** pontos), não por 4. Humano dá +2 comum; várias abilities/traits somam. Resultado é exibido como `valor/10` (INTRINSIC_TENTHS) no pacote de status. Ex.: Guerreiro 100 STR ≈ 5(classe) + 5(100/20) + 2(humano) ≈ 12 Fortitude exibido — longe de 45. — pkg/systems/attributes/include/intrinsicStats.inc:104-213; pkg/packetHooks/StatHook/sendStats.src:146-148; scripts/include/epicConstants.inc (INTRINSIC_TENTHS=10 em intrinsicStats.inc:17)
- **Mismatch:** Toda a fórmula está errada: o bônus de "Caminho" é +5/+2 (não +20/+10), o ganho por atributo é 1 por 20 (não por 4), e o exemplo numérico (45/27.5/20) é impossível com o código. Isto não é "número abstraído"; é a fórmula explícita do livro contradita pela fórmula explícita do código.
- **Severity:** HIGH (jogador dimensiona build de resistência com fórmula falsa; valores reais ~1/4 do prometido).
- **Cross-links:** Características Fortitude do Urso / Reflexos de Pantera / Vontade da Águia (todas somam +5 nesta mesma função).

### Script GM para rolar resistência
- **Handbook claim:** "Existe um script que permite o GM rolar a resistência de um personagem, definindo dificuldade e teste." — src/10-parte1-sistemas.md:258-260
- **Codebase truth:** Não localizado comando GM dedicado de rolagem de resistência em `pkg/commands/gm|seer|coun` (nenhum `CheckResist`/rolagem de Fortitude/Reflexos/Vontade nesses comandos).
- **Mismatch:** UNVERIFIED — não encontrado; pode existir sob outro nome/pacote não coberto. Não inventar.
- **Severity:** UNVERIFIED
- **Cross-links:** none

---

## CARACTERÍSTICAS (src/71-caracteristicas.md)

Definidas em `pkg/systems/charactercreation/config/traits.cfg` (texto/UI). Efeitos reais checados nos sites de uso de `chardata.trait`. Nota: traits.cfg permite Corpulento/Ágil/Gênio para Pequilho/Anão etc., mas o livro lista esses três sem pré-requisito — consistente.

### Cavalaria de Escaramuça — "não recebe penalidades" é falso (recalibração)
- **Handbook claim:** "Você não recebe penalidades no combate montado, podendo até usar arcos." — src/71-caracteristicas.md:169
- **Codebase truth:** Em `CalculateMountedSkillPenalty` (lógica em HitController): montado, sem trait e sem ability MOUNTED_COMBAT → skill /2 (−50%); exatamente um dos dois → skill ×75/100 (−25%); ambos → sem penalidade. A trait **sozinha** dá só 50→25, não zero. — pkg/systems/combat/include/Controller/HitController.inc:86-99
- **Mismatch:** Trait sozinha NÃO zera a penalidade; reduz de −50% para −25%. Penalidade zero exige também a Habilidade "Combate Montado" (MOUNTED_COMBAT). Confirma a recalibração do _FORMAT.
- **Severity:** HIGH (decisão de build: jogador escolhe a trait achando que zera penalidade montado).
- **Cross-links:** verificar duplicata em src/21-povos (Björska / Povos do Leste) — fora do meu domínio, sinalizar ao auditor de povos.

### Cavalaria de Escaramuça — "podendo até usar arcos" é regra inventada
- **Handbook claim:** "...podendo até usar arcos." — src/71-caracteristicas.md:169
- **Codebase truth:** Só há 2 referências à trait no código: o ladder de penalidade no HitController e `arremessar.src:41` (comando *arremessar*/throw, não arco). Nenhum código bloqueia arcos montado nem concede uso de arco específico à trait; archery montado não é gated por esta trait. — pkg/systems/combat/include/Controller/HitController.inc:88; pkg/systems/spells/commands/player/arremessar.src:41 (grep total de "Cavalaria de Escaramuça" = 2 hits)
- **Mismatch:** Implica que arcos seriam bloqueados montado e que a trait os libera — nenhum código enforça isso. O único gate real da trait via `arremessar.src` é para o comando de *arremesso* (potes/armas de arremesso), não arcos.
- **Severity:** INVENTED
- **Cross-links:** mesma linha do mismatch anterior.

### Cria do Deserto — desvantagem oculta de frio não declarada
- **Handbook claim:** "+10 para resistir climas quentes." / "Desvantagem: Nenhuma" — src/71-caracteristicas.md:215-217
- **Codebase truth:** Calor: dificuldade `dif -= 10` (bônus de fato ~+10). Frio: `dif += 4` — uma **penalidade** de +4 na dificuldade contra frio. — pkg/systems/nature/caloreffect.src:102-104; pkg/systems/nature/coldeffect.src:65-67
- **Mismatch:** "Desvantagem: Nenhuma" é falso — há penalidade de frio (+4 na dificuldade) embutida. O bônus de calor está correto.
- **Severity:** MED (desvantagem real não declarada; afeta escolha de personagem em zonas frias).
- **Cross-links:** none

### Sabe-Tudo — "+5 pontos" e "apenas uma perícia primária" desatualizados
- **Handbook claim:** "Vantagem: +5 pontos para comprar perícias. Desvantagem: Apenas uma perícia primária." — src/71-caracteristicas.md:80-82
- **Codebase truth:** A criação foi reformulada para slots (sem pool de pontos): com Sabe-Tudo, `BuildSkillSlots(path, has_sabe_tudo)` monta **2 Primárias + 5 Secundárias** (vs 3P+3S padrão). Além disso, Sabe-Tudo sobe de nível de habilidade com +3 a menos na soma das primárias (`GetSkillLevelInfo`: `periciaCombinadaAcima += 3`). — pkg/systems/charactercreation/epicgumpcriacao.src:456-467; pkg/systems/charactercreation/include/habilidades.inc:907-919
- **Mismatch:** Não existe "+5 pontos" (sistema de pontos foi removido); o efeito é 2P+5S. "Apenas uma perícia primária" está errado: são **2** primárias (o comentário em habilidades.inc:913 diz "unica primaria", mas BuildSkillSlots dá 2). Bônus real adicional (level-up de perícia mais fácil) não é mencionado.
- **Severity:** HIGH (engana totalmente a expectativa de build; nº de primárias e o "+5 pontos" inexistente).
- **Cross-links:** none

### Natureza Arcana — confirmado
- **Handbook claim:** "Você é capaz de conjurar truques mágicos. Pré-requisito: Elfo ou Drow." — src/71-caracteristicas.md:89-93
- **Codebase truth:** Acesso a cantrips ("truques") liberado se `chardata.trait == "Natureza Arcana"` (além de spellcaster/arcane_archer). — pkg/systems/spells/commands/player/t.src:33; pkg/systems/spells/include/spell_executor.inc:217-225. traits.cfg gate: Raca Elfo/Drow + Classe Guerreiro/Ladino/Sabio (traits.cfg:100-111).
- **Mismatch:** Efeito correto. (Gate Elfo/Drow conforme cfg; livro só cita raça — ok.)
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Defensor Anão — confirmado
- **Handbook claim:** "Você fica imune a críticos e paralisias, mas causa -30% de dano." — src/71-caracteristicas.md:193
- **Codebase truth:** Dano: `-30%` no dano causado (`penalty := 0.7; damage -= damage*(1-penalty)`). Crítico: defensor com a trait ignora o crítico. Paralisia: pular/imune em `pararotempo`, `paralysis.src`, `congelamento.src`. — pkg/systems/combat/include/Controller/DamageController_legacy.inc:1232-1240,1290-1296; pkg/systems/spells/spell_logic/mage/pararotempo.src:36-38; pkg/mobiles/timedScripts/subScripts/paralysis/paralysis.src:16; .../congelamento.src:16
- **Mismatch:** Nenhum. Claim correto.
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Força da Legião — confirmado
- **Handbook claim:** "Você recebe apenas metade da penalidade usando Scutum." — src/71-caracteristicas.md:158
- **Codebase truth:** Ao equipar Scutum (objtypes 0x1B76/0x1B77/0xFA62) com a trait, o modificador de DEXTERITY do escudo é dividido por 2. — pkg/systems/equipsys/equipitem.src:36-42; pkg/systems/equipsys/unequipitem.src:33-38
- **Mismatch:** Nenhum (penalidade modelada como modificador de DEX; o livro chama de "penalidade" — efeito coerente).
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Conexão com Alora — confirmado
- **Handbook claim:** "+10 Mana. Pré-requisito: Alüriel." — src/71-caracteristicas.md:179-184
- **Codebase truth:** `GetManaMaximumValue` soma +1000 interno (= +10 mana exibido, escala ×100) se trait == "Conexão com Alora". — pkg/systems/attributes/hooks/vitalInit.src:96-105
- **Mismatch:** Nenhum. (Gate Alüriel via traits.cfg:199-209.)
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Cavar Buracos — confirmado (com nota de não-stack)
- **Handbook claim:** "Extrai +2 minérios a mais a cada extração. Pré-requisito: Anão." — src/71-caracteristicas.md:111-115
- **Codebase truth:** `amt := amt + 2` na mineração e em meteoritos se trait == "Cavar Buracos" OU ability TALENTED_MINER (não soma com a ability — é OR). — pkg/skills/gathering/oremining/mining.src:180-183; pkg/skills/gathering/include/meteoritos.inc:81-84
- **Mismatch:** Efeito correto. Omissão menor: não acumula com a habilidade TALENTED_MINER (mesmo +2).
- **Severity:** LOW (efeito certo; non-stack não citado mas baixo impacto).
- **Cross-links:** none

### Oito Olhos — confirmado
- **Handbook claim:** "+10 Awareness. Pré-requisito: Har'oloth." — src/71-caracteristicas.md:147-151
- **Codebase truth:** `base_skill += 10` em checks de AWARENESS se trait == "Oito olhos". — pkg/systems/attributes/include/skillCheck.inc:116-119
- **Mismatch:** Nenhum. (cfg key "Oito olhos"/livro "Oito Olhos" — case-insensitive, ok.)
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Sorrateiro — confirmado
- **Handbook claim:** "Você recebe bônus para se esconder. Pré-requisito: Pequilho." — src/71-caracteristicas.md:100-104
- **Codebase truth:** Ao esconder, `diff := -10` (dificuldade de esconder cai drasticamente) se trait == "Sorrateiro". — pkg/skills/sneak/sneak.src:58-63
- **Mismatch:** Nenhum. Claim correto.
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Desapercebido — confirmado
- **Handbook claim:** "Você aumenta em 50% os seus passos. Pré-requisito: Colhesiana." — src/71-caracteristicas.md:204-208
- **Codebase truth:** `baseSteps := Cint(baseSteps * 1.5)` (passos escondido +50%) se trait == "Desapercebido". — pkg/skills/sneak/stealth.src:24-27
- **Mismatch:** Nenhum. Claim correto.
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Corpulento / Ágil / Gênio / Robusto — confirmados
- **Handbook claim:** Corpulento +10 STR/-5 DEX/-5 INT (src:30-32); Ágil +10 DEX/-5 STR/-5 INT (src:37-40); Gênio +10 INT/-5 STR/-5 DEX (src:47-49); Robusto +15 STR/-5 DEX/-5 INT (src:126-128).
- **Codebase truth:** Bônus de atributo aplicados exatamente assim por trait. — pkg/mobiles/merchants/pcmerchant/trainerMenu.src:245-260
- **Mismatch:** Nenhum. Valores batem.
- **Severity:** — (verificado correto)
- **Cross-links:** none

### Fortitude do Urso / Reflexos de Pantera / Vontade da Águia — confirmados
- **Handbook claim:** +5 Fortitude / +5 Reflexos / +5 Vontade, "Desvantagem: Nenhuma". — src/71-caracteristicas.md:55-73
- **Codebase truth:** `IntrinsicCalcFortitude` +5 se "Fortitude do Urso"; `IntrinsicCalcReflexes` +5 se "Reflexos de Pantera"; `IntrinsicCalcWill` +5 se "Vontade da Aguia". — pkg/systems/attributes/include/intrinsicStats.inc:107-109,141-143,164-166
- **Mismatch:** Nenhum no efeito. Nota: cfg/código usam "Vontade da Aguia" (sem acento); livro "Vontade da Águia" — irrelevante (string vinda da cfg, consistente internamente). Sem desvantagem confirmado.
- **Severity:** — (verificado correto)
- **Cross-links:** estes +5 entram na mesma fórmula cuja descrição global está errada (ver "Fórmula do Valor da Resistência").

### Adaptável — confirmado
- **Handbook claim:** "Bônus +2 para resistir danos climáticos. Pré-requisito: Humano." — src/71-caracteristicas.md:137-140
- **Codebase truth:** Calor `dif -= 2` e frio `dif -= 2` se trait == "Adaptável". — pkg/systems/nature/caloreffect.src:106-108; pkg/systems/nature/coldeffect.src:69-71
- **Mismatch:** Nenhum (bônus simétrico quente/frio, coerente com "qualquer temperatura").
- **Severity:** — (verificado correto)
- **Cross-links:** none

---

## Summary

| Severity   | Count |
|:-----------|:-----:|
| HIGH       | 6     |
| MED        | 4     |
| LOW        | 3     |
| INVENTED   | 1     |
| UNVERIFIED | 2     |
| Verificado correto | 13 |

### Top 5 por impacto

1. **HIGH — Fórmula do Valor da Resistência (src:251-256):** Caminho +20/+10 e "1 por 4 atributo" são falsos; código usa +5/+2 e 1 por 20, exibido /10. Exemplo (45/27.5/20) impossível; valores reais ~¼ do prometido. Engana toda construção de resistência.
2. **HIGH — Sabe-Tudo (src:80-82):** "+5 pontos" não existe (sem pool de pontos); efeito real é 2 Primárias + 5 Secundárias e level-up de perícia mais fácil. "Apenas uma perícia primária" errado (são 2).
3. **HIGH — Penalidades de Fome (src:172-184):** debuff começa em fome 3 (não 4), magnitude -30/-40/-50 (não -10/-20), pode ser totalmente anulado por teste de Fortitude; "−X testes de resistência" não é penalidade flat.
4. **HIGH — Assassinato (src:86-92):** "60 dias" na verdade 30; gates de "avaliação nula/positiva" e "dois DPs" não existem no código.
5. **HIGH — Morte por jogador não custa DP (src:61) + Cavalaria de Escaramuça (src:169):** PvP perde DP igual a PvE (regra do livro inexistente); trait sozinha só reduz penalidade montado de −50% para −25%, zero exige a Habilidade Combate Montado.
