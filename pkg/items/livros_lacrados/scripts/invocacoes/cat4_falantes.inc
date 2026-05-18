// invocacoes/cat4_falantes.src
// Categoria 4 - Que Vieram Para Falar



function InvocarMensageiro(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:ghost", "O Mensageiro do Que Nao Foi Dito", "mensageiro", "cat4");
    if (!npc) return; endif

    var msg := GerarMensagemRP(who);
    sleep(2);
    PrintTextAbove(npc, msg);
    sleep(3);

    sleep(2);
    if (npc && !npc.errortext) npc.kill(); endif
endfunction

function InvocarVozVelho(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:nevoida", "A Voz do Velho Mundo", "voz_velho", "cat4");
    if (!npc) return; endif

    var emotes := array{
 "*uma nevoa que nao devia existir toma forma*",
 "*o ar esfria sem razao e algo se move*",
 "*o silencio para um instante antes de falar*"
    };
    PrintTextAbove(npc, emotes[RandomInt(3)+1]);
    sleep(2);

    var fragmentos := array{
 "Esta terra foi escolhida porque esquece. O que morre aqui nao parte. Fica.",
 "Houve um acordo entre dois que ja nao existem da mesma forma. A clausula ainda vigora.",
 "Ela dorme ha eras. Nao e repouso. E digestao.",
 "As asas foram dobradas por escolha. Nunca recuperadas. Nunca lamentadas em voz alta.",
 "O arquiteto ainda esta aqui. Nao por vontade. Como consequencia de ter pedido demais.",
 "Ha um plano que nao e plano. Quem entra ja saiu. So nao sabe ainda."
    };
    var frag := fragmentos[RandomInt(6)+1];
    PrintTextAbove(npc, frag);
    sleep(3);

 var magia := AP_GetSkill(who, "Magia");
    if (magia >= 70)
        var segredos := array{
 "...Fea escolheu a imortalidade antes de entender que prisao e eternidade tem o mesmo comprimento...",
 "...Alora nao foi esquecida. Foi enterrada enquanto respirava. O sonho dela ainda move o mundo...",
 "...Vael'shorath e o nome que os que retornam trazem na boca sem saber. O Veneno nao mata. Altera...",
 "...o que os drows chamam de sacrificio, os que viram chamam de recusa. As asas nao cairam. Foram guardadas...",
 "...a torre nao foi terminada por dentro. O que esta la e anterior a magia. Anterior ao nome...",
 "...a ilha recolhe porque foi assim acordado. Por quem ainda tem o que pedir..."
        };
        var seg := segredos[RandomInt(6)+1];
        SendSysMessageEX(who, seg, SSM_INFO);
    else
    endif

    sleep(10);
 PrintTextAbove(npc, "*dissolve como nevoa ao sol*");
    sleep(2);
    if (npc && !npc.errortext) npc.kill(); endif
endfunction

function InvocarAnuncio(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:humanmale", "O Anuncio", "anuncio", "cat4");
    if (!npc) return; endif
    npc.color := 0x0481;
    var robe_an := CreateItemAtLocation(npc.x, npc.y, npc.z, 0x1F03, 1, npc.realm);
    if (robe_an && !robe_an.errortext)
        robe_an.color := 0x0481;
        if (EquipItem(npc, robe_an).errortext) DestroyItem(robe_an); endif
    endif

    var eventos := array{
 "Uma era termina. Outra nao comecou ainda.",
 "O decimo terceiro sinal foi dado. Os outros doze ja passaram.",
 "Algo foi esquecido antes de ser lembrado. Agora lembram.",
 "O nome proibido foi dito. Em algum lugar. Por alguem."
    };
    var evento := eventos[RandomInt(4)+1];

    sleep(2);
    PrintTextAbove(npc, evento);
 Broadcast("[ " + npc.name + " ]: " + evento);

    sleep(5);
    if (npc && !npc.errortext)
        MoveObjectToLocation(npc, 0, 0, 0, npc.realm, MOVEOBJECT_FORCELOCATION);
        sleepms(500);
        npc.kill();
    endif
endfunction

function InvocarConfissor(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:humanmale", "O Confissor Tardio", "confissor", "cat4");
    if (!npc) return; endif
    var robe_cf := CreateItemAtLocation(npc.x, npc.y, npc.z, 0x1F03, 1, npc.realm);
    if (robe_cf && !robe_cf.errortext)
        robe_cf.color := 0;
        if (EquipItem(npc, robe_cf).errortext) DestroyItem(robe_cf); endif
    endif

    npc.frozen := 1;
    sleepms(300);
    PlayNPCBowAnim(npc);
 PrintTextAbove(npc, "*ajoelha*");

    var crimes := array{
 "Enterrei meu irmao vivo. Ele me devia dinheiro.",
 "Incendiei a biblioteca de Aiglano. Os livros me pertenciam.",
 "Matei o mensageiro antes que chegasse. A mensagem nunca foi lida.",
 "Vendi minha filha para pagar uma divida. Ela nunca soube.",
 "Gravei um nome errado na lapide. O verdadeiro esta perdido."
    };

    var i := 1;
    while (i <= 5)
        if (!npc || npc.errortext) return; endif
        sleep(12);
        PrintTextAbove(npc, crimes[RandomInt(5)+1]);
        i += 1;
    endwhile

    if (!npc || npc.errortext) return; endif
    MoveObjectToLocation(npc, 0, 0, 0, npc.realm, MOVEOBJECT_FORCELOCATION);
    sleepms(200);
    npc.kill();
endfunction

function InvocarRecitaNomes(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:ghost6", "A Que Recita Nomes", "recita_nomes", "cat4");
    if (!npc) return; endif

 var frases_rec := array{"*murmura nomes*", "*recita em voz baixa*", "*conta algo*"};
 start_script(":livros_lacrados:scripts/invocacoes/brain_wander",
        array{npc, ReadGameClock() + 120, frases_rec, 15});
endfunction
