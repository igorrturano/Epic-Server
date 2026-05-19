// invocacoes/cat2_mortos.src
// Categoria 2 - Mortos Que Nao Aceitaram



function InvocarMaeProcura(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:ghost5", "A Mae que Procura", "mae_procura", "cat2");
    if (!npc) return; endif
 PrintTextAbove(npc, "*olha em volta com expressao de desespero*");
 var frases_mae := array{"*chama por um nome*", "*soluca em silencio*", "*olha ao redor perdida*"};
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + 3600, frases_mae, 30});
endfunction

function InvocarSoldado(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:skeletonfighter", "O Soldado de Aiglano", "soldado", "cat2");
    if (!npc) return; endif
 PrintTextAbove(npc, "A guerra... acabou?");
 var frases_sold := array{"A guerra... acabou?", "*olha para as proprias maos*", "Onde esta meu regimento?", "*marcha sem destino*"};
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + 1800, frases_sold, 30});
endfunction

function InvocarLirenne(who, book, nivel, afinidade, lugar)
 if (CInt(GetGlobalProperty("lirenne_expira")) > polcore().systime)
        return;
    endif

 SetGlobalProperty("lirenne_expira", polcore().systime + 3600);

 var npc := _InvocarPadrao(who, ":epicAI:ghost4", "Lirenne", "lirenne", "cat2");
    if (!npc)
 EraseGlobalProperty("lirenne_expira");
        return;
    endif

 SendSysMessageEX(who, "Uma presenca feminina passa por voce.", SSM_INFO);
 start_script(":livros_lacrados:scripts/invocacoes/brain_lirenne",
        array{npc, ReadGameClock() + 3600, 30});
endfunction

function InvocarTresIrmaos(who, book, nivel, afinidade, lugar)
 var npc1 := CreateNpcFromTemplate(":epicAI:ghost3", who.x + 2, who.y, who.z, 0, who.realm);
 var npc2 := CreateNpcFromTemplate(":epicAI:ghost3", who.x - 2, who.y, who.z, 0, who.realm);
 var npc3 := CreateNpcFromTemplate(":epicAI:ghost3", who.x, who.y + 2, who.z, 0, who.realm);

    if (npc1 && !npc1.errortext)
 SetName(npc1, "Irmao Mais Velho");
 SetObjProperty(npc1, "invocacao_id", "tres_irmaos");
    endif
    if (npc2 && !npc2.errortext)
 SetName(npc2, "Irmao do Meio");
 SetObjProperty(npc2, "invocacao_id", "tres_irmaos");
    endif
    if (npc3 && !npc3.errortext)
 SetName(npc3, "Irmao Mais Novo");
 SetObjProperty(npc3, "invocacao_id", "tres_irmaos");
    endif

 start_script(":livros_lacrados:scripts/invocacoes/brain_tres_irmaos",
        array{npc1, npc2, npc3, ReadGameClock() + 1800});
endfunction

function InvocarBibliotecario(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:ghost9", "O Bibliotecario Morto", "bibliotecario", "cat2", 1);
    if (!npc) return; endif
 PrintTextAbove(npc, "*ajusta oculos que nao existem*");
 SendSysMessageEX(who, "Uma figura silenciosa aparece. Fale com ela.", SSM_INFO);
 start_script(":livros_lacrados:scripts/invocacoes/brain_bibliotecario",
        array{npc, who, ReadGameClock() + 3600});
endfunction
