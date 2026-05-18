// invocacoes/cat5_cobradores.src
// Categoria 5 - Que Vieram Para Levar



function _InvocarCobrador(who, template, nome, inv_id, frase_abertura,
                           frases_idle, expira_ticks, intervalo)
 var npc := _InvocarPadrao(who, template, nome, inv_id, "cat5");
    if (!npc) return; endif
    PrintTextAbove(npc, frase_abertura);
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + expira_ticks, frases_idle, intervalo});
endfunction

function InvocarColetorAnos(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:ghost", "O Coletor de Anos", "coletor_anos", "cat5");
    if (!npc) return; endif

    var valor := 10000;
    if (nivel == 4) valor := 50000; endif
    if (nivel == 5) valor := 100000; endif

 PrintTextAbove(npc, "Pague-me " + valor + " moedas. Ou pague de outra forma.");

    var expira_col := ReadGameClock() + 600;
    if (nivel == 4) expira_col := ReadGameClock() + 1200; endif
    if (nivel == 5) expira_col := ReadGameClock() + 1800; endif
 var frases_col := array{"*conta algo nos dedos*", "*observa voce*", "*aguarda*"};
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, expira_col, frases_col, 30});
endfunction

function InvocarPedeSangue(who, book, nivel, afinidade, lugar)
 _InvocarCobrador(who, ":epicAI:quasit", "O Que Pede Sangue", "pede_sangue",
 "Trinta pontos de sangue. Por sua vontade, ou nao.",
 array{"*observa voce*", "*estende a mao*", "*espera*"}, 60, 15);
endfunction

function InvocarQuerNome(who, book, nivel, afinidade, lugar)
 _InvocarCobrador(who, ":epicAI:nevoida", "A Que Quer Um Nome", "quer_nome",
 "Um nome.",
 array{"*aguarda em silencio*", "*olha para voce*"}, 60, 20);
endfunction

function InvocarVeioMemoria(who, book, nivel, afinidade, lugar)
 _InvocarCobrador(who, ":epicAI:ghost", "O Que Veio Por Memoria", "veio_memoria",
 "*aguarda uma memoria*",
 array{"*espera pacientemente*", "*olha para voce*", "*nao se move*"}, 300, 30);
endfunction

function InvocarCobrador(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:Bodak", "O Cobrador da Divida Antiga", "cobrador", "cat5");
    if (!npc) return; endif

    var alvo := GerarNomeProc();
 PrintTextAbove(npc, "Onde esta " + alvo + "?");

 var frases_cob := array{"*procura em volta*", "*murmura um nome*", "*caminha sem descanso*"};
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + 120, frases_cob, 30});
endfunction

function GerarNomeProc()
 var silabas := array{"Vel", "Tor", "Khar", "Brel", "Dur", "Ash", "Mal", "Ves"};
 var sufixos := array{"an", "us", "en", "ar", "is"};
    return silabas[RandomInt(8)+1] + sufixos[RandomInt(5)+1];
endfunction
