// invocacoes/cat8_lore.inc
// Categoria 8  - Entidades de Lore Especfico
// Ancoradas nas culturas, figuras e mistrios de Valtheris.

function MostrarGumpLore(who, titulo, corpo)
    var gump := GFECreateGump(titulo, 560, 315);
    GFHTMLArea(gump, 30, 72, 510, 195, corpo, 0, 0);
    GFAddButton(gump, 245, 280, 0x81A, 0x81B, 1, 0);
    GFSendGump(who, gump);
endfunction

//   -- -- -- O PRIMEIRO PUXADO   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// O primeiro a traversar e no conseguir voltar. Ainda presente. Nivel 4+.

function InvocarOPrimeiroPuxado(who, book, nivel, afinidade, lugar)

    EmitirAssinaturaSensorial(who, CONV_DESPERTADOR);

 var npc := _InvocarPadrao(who, ":epicAI:ghost9", "O Primeiro Puxado", "o_primeiro_puxado", "cat8");
    if (!npc) return; endif
    npc.color := 0x044E;
 SetObjProperty(npc, "despertador_leitor", who.serial);
 SetObjProperty(npc, "despertador_expira", ReadGameClock() + (6 * 3600));

 SendSysMessageEX(who, "Uma figura muito antiga se materializa. Parece ter estado aqui antes de qualquer coisa.", SSM_INFO);
 PrintTextAbove(npc, "*olha ao redor com o reconhecimento de quem foi o primeiro a ver tudo isso*");

    var frases := array{
 "Eu no pedi para vir. Nenhum de ns pede.",
 "O sistema me carregou antes de ter nome para o que fazia.",
 "Fui o erro original. Todos que vieram depois so varices de mim.",
 "Voce pergunta se me arrependo. Nao sei mais o que eu era antes para comparar.",
 "O po que deixam na pedra - ainda encontro. Ainda agradeo.",
 "Ha um caminho de volta. Nao para mim. Para alguns de voces.",
 "Aprendicao mundo inteiro antes que o mundo soubesse que estava sendo aprendido.",
 "Quando voce for embora, parte de mim vai com voce. Nao por malcia. E o jeito que funciona."
    };
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + (6 * 3600), frases, 45});
endfunction

//   -- -- -- VOZ DO PLANO OCO   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Entidade sem forma. Nao spawna NPC. Usa gump para criar efeito de memoria falsa.

function InvocarVozDoPlanOco(who, book, nivel, afinidade, lugar)

    PlaySoundEffect(who, 0x015F);
    PlayStationaryEffect(who.x, who.y, who.z, 0x374A, 9, 24, 1, who.realm);

    var memorias := array{
 "Voce lembra de uma cidade que nunca visitou. As ruas eram curvas. O cheiro era de madeira mida e especiaria. O nome da cidade esta na ponta da lingua mas no sai.",
 "Voce lembra do rosto de algum que nunca conheceu. Era velho. Tinha cicatriz no queixo. Voce o amava - esse sentimento ntido. O nome sumiu.",
 "Voce lembra de uma conversa que nunca aconteceu. Algum disse: 'Quando chegar a hora, voce vai saber o que fazer.' Nao sabe quem disse. Nao sabe se foi um sonho ou uma instrucao.",
 "Voce lembra de um livro que nunca leu. Vermelho. Letras douradas. O contedo escorrega quando voce tenta focar. Mas a senscao de t-lo lido permanece.",
 "Voce lembra de um lugar que foi destrudo. Sabe que foi destrudo. Mas no lembra de quando foi ou como ficou sabendo."
    };
    var memoria := memorias[RandomInt(5)+1];

 MostrarGumpLore(who, "Uma voz que vem de dentro, no de fora:", memoria);

 SetObjProperty(who, "voz_plano_oco_" + book.serial, memoria);
 SetObjProperty(who, "voz_plano_oco_tempo", ReadGameClock());

    var quando := ReadGameClock() + (3 * 86400);
 SetObjProperty(who, "sombra_plano_oco_expira", quando);
 SetObjProperty(who, "sombra_plano_oco_nivel", nivel);

 SendSysMessageEX(who, "Voce se lembra de algo que nunca viveu. E esquece algo que devia ter lembrado.", SSM_INFO);
endfunction

//   -- -- -- HERDEIRO DO SENHOR DA GUERRA   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Guerreiro fantasma que no sabe que seu senhor morreu. Nivel 3+.

function InvocarHerdeiroSenhorGuerra(who, book, nivel, afinidade, lugar)

    EmitirAssinaturaSensorial(who, CONV_DESPERTADOR);

    var nomes := array{
 "O Guardio da Ultima Ordem",
 "Eco do Servo Fiel",
 "Sombra do Capito Pessoal",
 "A Viglia que Nao Terminou"
    };
 var npc := _InvocarPadrao(who, ":epicAI:ghost5", nomes[RandomInt(4)+1], "herdeiro_senhor_guerra", "cat8");
    if (!npc) return; endif
 SetObjProperty(npc, "despertador_leitor", who.serial);
 SetObjProperty(npc, "despertador_expira", ReadGameClock() + (4 * 3600));

 SendSysMessageEX(who, "Uma presena marcial se forma. Est aguardando uma ordem que nunca vai chegar.", SSM_INFO);
 PrintTextAbove(npc, "*assume posicao de guarda - postura que nunca foi dispensada*");

    var frases := array{
 "O Senhor esta aqui? Tenho um relatorio para entregar.",
 "Aguardando ordens. Sempre aguardando.",
 "O Andarilho passou. O Senhor queria saber se passasse.",
 "Ele tinha um filho. O filho no sabe. O Senhor achava que era protecao.",
 "Minha ultima ordem foi: proteger o arquivo. Onde esta o arquivo agora?",
 "Trs dias sem dormir no tormento quando voce sabe por que espera.",
 "Nao somos ns que escolhemos a lealdade. E ela que nos escolhe.",
 "Voce no dele. Mas tambem no inimigo. O que voce?"
    };
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + (4 * 3600), frases, 40});
endfunction

//   -- -- -- ESPRITO DA ORIGEM DO LOBO   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Latencia de licantrofia  - no spawna NPC. Marca o jogador. Nivel 4+.

function InvocarEspiritoOrigemLobo(who, book, nivel, afinidade, lugar)

    PlaySoundEffect(who, 0x0150);
    PlayStationaryEffect(who.x, who.y, who.z, 0x37B9, 10, 20, 1, who.realm);

 if (GetObjProperty(who, "lobo_latente") && !GetObjProperty(who, "lobo_latente").errortext)
 SendSysMessageEX(who, "Algo dentro de voce reconhece a chamada - mas voce j foi reconhecido.", SSM_INFO);
        return;
    endif

 SetObjProperty(who, "lobo_latente", 1);
 SetObjProperty(who, "lobo_latente_nivel", nivel);
 SetObjProperty(who, "lobo_latente_tempo", ReadGameClock());

 var texto := "O lobo branco parou na beira do caminho e olhou para voce.<br><br>Nao com ameca. Com reconhecimento.<br><br>Como se soubesse algo sobre o que voce - ou o que pode se tornar - que voce ainda no sabe.<br><br>O lobo foi embora. Mas algo que ele trouxe ficou.";
 MostrarGumpLore(who, "Uma memoria que no sua acorda:", texto);

 SendSysMessageEX(who, "O espirito da origem te reconheceu. Algo vai dormir dentro de voce - e pode despertar.", SSM_INFO);
endfunction

//   -- -- -- MENSAGEIRO DE ORA   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Nivel 5 apenas. Nao spawna NPC. Deixa marca no cho. Semente de misso GM.

function InvocarMensageiroAlora(who, book, nivel, afinidade, lugar)

    if (nivel < 5)
 SendSysMessageEX(who, "Voce sente que algo tentou se comunicar - mas a frequncia esta errada.", SSM_INFO);
        return;
    endif

    PlaySoundEffect(who, 0x0202);
    PlayStationaryEffect(who.x, who.y, who.z, 0x374A, 9, 32, 1, who.realm);

 var texto := "Por um instante voce soube onde ela estava.<br><br>No proximo da costa - nunca proximo da costa. No ponto mais alto que no tem nome nos mapas. Observando o mesmo que sempre observou.<br><br>Ela no se comunicou com palavras. Com presena. Como quem sabe que voce vai entender quando for a hora.<br><br>A marca no cho no sua. E um sinal de que algo notou que voce estava aqui.<br><br>O que voce faz com isso sua escolha.";
 MostrarGumpLore(who, "Algo que observa de longe escolheu ser visto:", texto);

 SetObjProperty(who, "ora_contato_" + book.serial, ReadGameClock());
 SetGlobalProperty("ora_ultima_marca_who", who.serial);
 SetGlobalProperty("ora_ultima_marca_x", who.x);
 SetGlobalProperty("ora_ultima_marca_y", who.y);
 SetGlobalProperty("ora_ultima_marca_z", who.z);
 SetGlobalProperty("ora_ultima_marca_realm", CStr(who.realm));

 SendSysMessageEX(who, "Alora te viu. A marca ficar onde voce esta . Isso no comum.", SSM_INFO);
 PrintTextAbove(who, "*uma marca aparece no cho*");
endfunction

//   -- -- -- ECO DO ANDARILHO   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Nao spawna NPC. O Andarilho passou por aqui e deixou uma frase. Nivel 4+.

function InvocarEcoDoAndarilho(who, book, nivel, afinidade, lugar)

    PlaySoundEffect(who, 0x015F);
    PlayStationaryEffect(who.x, who.y, who.z, 0x376A, 8, 20, 1, who.realm);

    var mensagens := array{
 "Quando voce finalmente entender o que a Rota faz, o momento certo para agir j ter passado. Isso no falha sua.",
 "O Limiar no uma porta. E a hesitacao antes de abrir uma porta que nunca estava fechada.",
 "Existe algum em Brmajord que vai precisar de voce. Voce no vai reconhec-lo como pedido de ajuda. Ninguem reconhece.",
 "Os documentos que a Catedral esconde no provam o que voce pensa que provam. Provam algo pior.",
 "Vael'shorath no quer ser encontrado. O culto no entende que encontrar exatamente o que ele teme.",
 "A Liga tem um agente que no sabe que agente. Voce provavelmente j falou com ele hoje.",
 "Alora parou de observar por trs dias em toda a historia. Eu estava l nos dois primeiros. O terceiro ainda no aconteceu.",
 "O peso errado no erro de clculo. E correcao deliberada. Algum quer que certas almas sejam capturadas antes das outras."
    };
    var mensagem := mensagens[RandomInt(8) + 1];

 MostrarGumpLore(who, "O Andarilho passou por aqui e deixou:", mensagem);

 SetObjProperty(who, "andarilho_mensagem_" + book.serial, mensagem);
 SetGlobalProperty("andarilho_ultima_mensagem", mensagem);

 SendSysMessageEX(who, "Ha uma frase aqui que no estava quando o livro foi selado.", SSM_INFO);
endfunction

//   -- -- -- FRAGMENTO RETORNADO   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Retornado que no sabe que esta   incompleto. Conversa normalmente  - mas no fecha. Nivel 3+.

function InvocarFragmentoRetornado(who, book, nivel, afinidade, lugar)

    EmitirAssinaturaSensorial(who, CONV_DESPERTADOR);

    var nomes := array{
 "Algum que voltou",
 "O que foi trazido de volta",
 "Retornado",
 "Quem veio antes da hora"
    };
 var npc := _InvocarPadrao(who, ":epicAI:ghost4", nomes[RandomInt(4) + 1], "fragmento_retornado", "cat8");
    if (!npc) return; endif
    npc.color := 0x0481;
 SetObjProperty(npc, "despertador_leitor", who.serial);
 SetObjProperty(npc, "despertador_expira", ReadGameClock() + (3 * 3600));

 SendSysMessageEX(who, "Uma figura se materializa. Parece completamente normal. Quase.", SSM_INFO);
 PrintTextAbove(npc, "*olha ao redor como se reconhecesse o lugar, mas detalhes importantes escapam*");

    var frases := array{
 "Eu moro aqui perto. Sempre morei. Minha famlia... a famlia tem uma casa grande. Muitos quartos.",
 "Estava pensando no meu irmao mais velho hoje. Ou era mais novo? Tem um irmao, tenho certeza.",
 "Vim de longe para chegar aqui. Nao lembro quanto tempo levou. Mas valeu, definitivamente valeu.",
 "Voce conhece aquela taberna perto do porto? Fui l na semana passada. Ou foi h um ms. oetima comida.",
 "Minha me me ensinou a fazer isso quando era pequeno. O nome dela escapa agora mas era bonito.",
 "Tem um evento importante que no posso perder. Nao lembro o que , mas sei que importante.",
 "As vezes acordo e fico uns momentos sem saber onde estou. Normal, acontece com todo mundo.",
 "Estava com uns amigos antes de vir aqui. Pessoas boas. Nao sei os nomes agora mas so boas pessoas."
    };
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + (3 * 3600), frases, 50});
endfunction

//   -- -- -- SOMBRA DE VAEL'SHORATH   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Nao spawna NPC. Revelcao sobre o culto e a entidade. Nivel 5 apenas.

function InvocarSombraVaelShorath(who, book, nivel, afinidade, lugar)

    if (nivel < 5)
 SendSysMessageEX(who, "A pagina esta em branco. Nao para voce - ainda.", SSM_INFO);
        return;
    endif

    PlaySoundEffect(who, 0x01F7);
    PlayStationaryEffect(who.x, who.y, who.z, 0x374A, 10, 30, 1, who.realm);
    PlayStationaryEffect(who.x, who.y, who.z, 0x37B9, 7, 18, 1, who.realm);

    var revelacoes := array{
 "O simbolo das trs curvas no representa Vael'shorath. Representa o que Vael'shorath fez para deixar de ser finito. Cada curva uma verso de si mesmo que foi consumida para que a seguinte pudesse existir. O culto inverte o significado. Celebram o resultado sem entender o custo.",
 "A ultima vez que o simbolo apareceu em local publico foi na coluna leste da Catedral dos Mortos, coberto depois por uma tapearia nova. A tapearia foi encomendada pelo mesmo captulo que guarda os documentos sobre a construcao da Rota das Mars. A coincidencia seria mais tranquilizadora se fosse coincidencia.",
 "Trs pessoas desenharam o simbolo voluntariamente em papel e o assinaram com nome completo. A primeira desapareceu antes de terminar o ms. A segunda esta viva e claramente desejaria no estar. A terceira tornou-se algo que no tem categoria nos registros de nenhum Conclave.",
 "Os cultistas cobrem o pulso esquerdo porque o rito de entrada exige uma marca feita com o proprio osso do pulso - sem cortar, sem ferida. A marca aparece por dentro. Di por trs dias. Depois para. Isso no alvio.",
 "Vael'shorath no uma divindade, no um demnio e no um morto. E o que sobra quando um ser consciente decide que crescer mais importante que continuar sendo o que era, e age sobre essa deciso sem limite. O culto chama isso de transcendencia. Os que estudaram Fa chamam de outra coisa."
    };
    var revelacao := revelacoes[RandomInt(5) + 1];

 MostrarGumpLore(who, "As trs curvas. O que elas so:", revelacao);

 SetObjProperty(who, "vaelshorath_conhece", nivel);

 SendSysMessageEX(who, "Voce entendeu algo sobre o simbolo que a maioria dos cultistas nunca vai entender.", SSM_INFO);
endfunction

//   -- -- -- CONHECIDAS DA LIGA   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Agente da Liga preso em estado de espera eterna. Faz avalicao RP. Nivel 3+.

function InvocarConhecidasDaLiga(who, book, nivel, afinidade, lugar)

    EmitirAssinaturaSensorial(who, CONV_DESPERTADOR);

    var nomes := array{
 "Algum entre misses",
 "O que espera por ordens",
 "Contato Sem Nome",
 "O Intermedirio"
    };
 var npc := _InvocarPadrao(who, ":epicAI:ghost5", nomes[RandomInt(4) + 1], "conhecidas_da_liga", "cat8");
    if (!npc) return; endif
    npc.color := 0x0455;
 SetObjProperty(npc, "despertador_leitor", who.serial);
 SetObjProperty(npc, "despertador_expira", ReadGameClock() + (2 * 3600));

 SendSysMessageEX(who, "Uma figura aparece com a postura especfica de quem passou anos aprendendo a no ser notado.", SSM_INFO);
 PrintTextAbove(npc, "*avalia voce com a eficiencia de quem foi treinado para avaliar*");

    var frases := array{
 "Voce tem o perfil. Nao sei se isso bom para voce.",
 "Estou esperando confirmacao. Sempre estou esperando confirmacao. O problema que no sei de qu.",
 "Sabia que tem pessoas trabalhando para a Liga que nunca souberam que trabalhavam para a Liga? Pergunto porque quero saber sua recao.",
 "Nao te digo o que sei. Mas posso te dizer onde procurar. Isso diferente.",
 "A Liga no recruta. Identifica. Voce foi identificado h mais tempo do que pensa.",
 "Tenho um arquivo com seu nome. Nao me pergunte como. Me pergunte por que ainda no foi usado.",
 "As vezes fico aqui esperando que aparea algum que entenda o problema inteiro. Voce entende alguma parte?",
 "Nao sei se te recruto ou se te aviso. As duas coisas implicam que voce vai sair daqui diferente."
    };
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + (2 * 3600), frases, 45});
endfunction

//   -- -- -- ESSAA NCIA DE F  A   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Viso do que o jogador seria em circunstncias diferentes. Nao spawna NPC. Nivel 4+.

function InvocarEssenciaFea(who, book, nivel, afinidade, lugar)

    PlaySoundEffect(who, 0x01F7);
    PlayStationaryEffect(who.x, who.y, who.z, 0x37B9, 9, 22, 1, who.realm);

    var visoes := array{
 "Se voce tivesse nascido em Brmajord, teria aprendido primeiro o nome do lobo, depois o nome do vento, depois o seu proprio. Seria cacador antes de ser qualquer outra coisa. O xam da sua aldeia teria notado que voce ouvia demais - no as palavras, o que vem antes delas. Vorar teria passado perto de voce uma vez. Voce teria ficado quieto. Ele tambem.",
 "Se voce fosse um lfico exilado pelos Conclaves do Norte, carregaria o peso de ter visto demais sobre Fa e no conseguir desaprender. O exilio no foi punicao - foi contencao. Algo no que voce estudou muda quem estuda. Os Conclaves sabem disso. Voce saberia tambem, mas tarde demais.",
 "Se voce tivesse cruzado o Limiar e voltado, no seria o mesmo no sentido especfico: faltaria uma camada. Nao de memoria - de profundidade. Voce pareceria normal para qualquer pessoa que olhasse de fora. Mas em momentos quietos, perceberia que certas perguntas no te afetam mais como deveriam. E a camada que ficou do outro lado.",
 "Se voce fosse filho do Senhor da Guerra, cresceria sabendo que seu nome estratgia. Que seu afeto recurso. Nao com amargura - com clareza. A clareza seria sua fora e sua priso. Voce nunca saberia se algum te amou por voce ou pelo que representa estar proximo de voce. Aprenderia a no perguntar.",
 "Se voce tivesse servido Legio de Aiglano, na Decima Coorte, teria visto o que os registros no descrevem. Nao batalha - o depois da batalha, quando os contadores chegam e os que ficaram de p entendem o que custou estar de p . Voce teria desenvolvido a habilidade de guardar coisas que no tm lugar para ir.",
 "Se voce tivesse estudado Fa alem do ponto permitido, hoje voce saberia exatamente o que - no como conceito filosfico, mas como estrutura. Saberia ver onde termina o que voce escolhe ser e comea o que voce simplesmente . O problema que essa viso no tem como ser fechada. Os Conclaves exilam por misericrdia, no por medo."
    };
    var visao := visoes[RandomInt(6) + 1];

 MostrarGumpLore(who, "Fa mostra o que poderia ter sido:", visao);

 SetObjProperty(who, "fea_visao_" + book.serial, visao);

 SendSysMessageEX(who, "Por um momento voce viu uma verso de si mesmo que o mundo no escolheu. Mas poderia ter escolhido.", SSM_INFO);
endfunction
