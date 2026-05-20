// invocacoes/cat7_unicos.src
// Categoria 7  - AAnicos (uma vez no servidor)



function InvocarPrimeiroLacrador(who, book, nivel)
 if (GetGlobalProperty("unico_usado_primeiro_lacrador"))
        return;
    endif
 SetGlobalProperty("unico_usado_primeiro_lacrador", 1);

    EmitirAssinaturaSensorial(who, CONV_UNICO);

 var npc := _InvocarPadrao(who, ":epicAI:Lich", "O Primeiro Lacrador", "primeiro_lacrador", "cat7", 3);
    if (!npc) return; endif
    npc.color := 0x0481;

 PrintTextAbove(npc, "Voce encontrou o que selei. Agora prove que merece carreg -lo.");
    npc.opponent := who;
endfunction

function InvocarAntessenhora(who, book, nivel)
 if (GetGlobalProperty("unico_usado_antessenhora"))
        return;
    endif
 SetGlobalProperty("unico_usado_antessenhora", 1);

    EmitirAssinaturaSensorial(who, CONV_UNICO);

 var npc := _InvocarPadrao(who, ":epicAI:ghost8", "A Antessenhora", "antessenhora", "cat7");
    if (!npc) return; endif
    npc.color := 0;
 SetObjProperty(npc, "expira_em", ReadGameClock() + (24 * 3600));

 Broadcast("Uma presena antiga caminha pelo mundo. Procure-a.");
 var frases_ante := array{"*observacao horizonte*", "*fala em lingua esquecida*", "*existe sem pressa*"};
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + (24 * 3600), frases_ante, 60});
endfunction

function InvocarSemNome(who, book, nivel)
 if (GetGlobalProperty("unico_usado_sem_nome"))
        return;
    endif
 SetGlobalProperty("unico_usado_sem_nome", 1);

    EmitirAssinaturaSensorial(who, CONV_UNICO);

 var npc := _InvocarPadrao(who, ":epicAI:MonstrodaPoeira", "", "sem_nome", "cat7", 3);
    if (!npc) return; endif
 SetObjProperty(npc, "invulnerable", 1);
 SetGlobalProperty("sem_nome_expira", polcore().systime + 3600);
 SetObjProperty(npc, "expira_em", ReadGameClock() + 3600);

 Broadcast("Algo que no tem nome esta presente. Os animais sabem.");
 var frases_sn := array{"*no reage a nada*", "*existe*"};
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + 3600, frases_sn, 120});
endfunction

function InvocarFilhoServidor(who, book, nivel)
 if (GetGlobalProperty("unico_usado_filho_servidor"))
        return;
    endif
 SetGlobalProperty("unico_usado_filho_servidor", 1);

    EmitirAssinaturaSensorial(who, CONV_UNICO);

 var npc := _InvocarPadrao(who, ":epicAI:ghost", "O Filho do Servidor", "filho_servidor", "cat7");
    if (!npc) return; endif

 Broadcast("Uma crianca fantasmagrica vaga pelo mundo procura de seu pai.");
 var frases_filho := array{"Pai?", "*olha ao redor com os olhos muito abertos*", "*chama em silencio*"};
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + (24 * 3600), frases_filho, 45});
endfunction

function InvocarCoisaErrada(who, book, nivel)
 if (GetGlobalProperty("unico_usado_coisa_errada"))
        return;
    endif
 SetGlobalProperty("unico_usado_coisa_errada", 1);

    EmitirAssinaturaSensorial(who, CONV_UNICO);

 SetGlobalProperty("coisa_errada_presente", 1);
 SetGlobalProperty("coisa_errada_expira", ReadGameClock() + 3600);

 Broadcast("Algo que no deveria estar aqui esta . Por uma hora.");
 SendSysMessageEX(who, "O mundo parece levemente errado. Magias falham mais. Animais se perdem.", SSM_INFO);

 start_script(":livros_lacrados:scripts/invocacoes/brain_coisa_errada", array{who});
endfunction
