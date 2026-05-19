// invocacoes/cat3_animais.src
// Categoria 3 - Animais Que Nao Existem


function InvocarCoisaQuadrupede(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:spikedbeast", "A Coisa Quadrupede", "coisa_quadrupede", "cat3", 3);
    if (!npc) return; endif
    npc.opponent := who;
endfunction

function InvocarPequenosLimiar(who, book, nivel, afinidade, lugar)
    var qtd := RandomInt(4) + 5; // 5-8
    var expira := ReadGameClock() + (30 * 24 * 3600);
    var i := 0;
    var criados := 0;
    while (i < qtd)
 var npc := CreateNpcFromTemplate(":epicAI:cat", who.x + RandomInt(6)-3, who.y + RandomInt(6)-3, who.z, 0, who.realm);
        if (npc && !npc.errortext)
            npc.color := 0x0482;
 SetName(npc, "Pequeno do Limiar");
 SetObjProperty(npc, "falso_lacrado", 1);
 SetObjProperty(npc, "invulnerable", 1);
 SetObjProperty(npc, "expira_em", expira);
 start_script(":livros_lacrados:scripts/invocacoes/brain_scatter_wander",
                array{npc, expira, 25});
            criados += 1;
        else
        endif
        i += 1;
        sleepms(100);
    endwhile
 SendSysMessageEX(who, "Criaturas pequenas correm para se esconder. Voce as vera novamente.", SSM_INFO);
endfunction

function InvocarCantante(who, book, nivel, afinidade, lugar)
 var exp_atual := CInt(GetGlobalProperty("cantante_ativo"));
    if (exp_atual > ReadGameClock())
 SendSysMessageEX(who, "O Cantante ainda esta presente no mundo.", SSM_INFO);
        return;
    endif

    var expira := ReadGameClock() + (7 * 24 * 3600);
 SetGlobalProperty("cantante_ativo", expira);
 start_script(":livros_lacrados:scripts/invocacoes/brain_cantante_watcher", array{expira});

    PlaySoundEffect(who, 0x683);
    sleep(1);
 SendSysMessageEX(who, "O livro treme em sua mao.", SSM_INFO);
    sleep(3);
 SendSysMessageEX(who, "O som nao vem de dentro do livro. Vem de baixo.", SSM_INFO);
    sleep(3);
 SendSysMessageEX(who, "Muito abaixo.", SSM_INFO);
    sleep(2);
 SendSysMessageEX(who, "O Cantante nao tem nome porque nomes foram inventados depois.", SSM_INFO);
    sleep(3);
 SendSysMessageEX(who, "Ele cantou antes que houvesse ouvidos para ouvir.", SSM_INFO);
    sleep(3);
 SendSysMessageEX(who, "Ainda canta.", SSM_INFO);
    sleep(2);
 SendSysMessageEX(who, "Um som enorme emerge de algum lugar profundo. Entao silencia.", SSM_INFO);


 var corvo := CreateNpcFromTemplate(":epicAI:crow", who.x + RandomInt(4) - 2, who.y + RandomInt(4) - 2, who.z, 0, who.realm);
    if (corvo && !corvo.errortext)
 SetName(corvo, "O Cantante");
 SetObjProperty(corvo, "invocacao_id", "cantante");
 SetObjProperty(corvo, "falso_lacrado", 1);
 SetObjProperty(corvo, "expira_em", expira);
 start_script(":livros_lacrados:scripts/invocacoes/brain_scatter_wander",
            array{corvo, expira, 30});
    else
    endif
endfunction

function InvocarCoisaImita(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:humanmale", who.name, "coisa_imita", "cat3");
    if (!npc) return; endif

    // Remove tudo que o template colocou (cabelo, roupas, backpack do NPC)
    // para evitar conflito de layer ao equipar os itens do jogador
    var itens_template := ListEquippedItems(npc);
    foreach item_t in itens_template
        DestroyItem(item_t);
    endforeach

    // Copia aparencia do corpo (raca, cor de pele, genero)
    npc.graphic     := who.graphic;
    npc.trueobjtype := who.trueobjtype;
    npc.color       := who.color;
    npc.gender      := who.gender;

    // Copia todos os itens equipados: roupa, armadura, cabelo, barba, etc.
    // Exclui apenas a backpack (layer 0x15)
    var itens_equip := ListEquippedItems(who);
    foreach item_eq in itens_equip
        if (item_eq.layer != 0x15)
            var copia := CreateItemCopyAtLocation(npc.x, npc.y, npc.z, item_eq);
            if (copia && !copia.errortext)
                copia.color := item_eq.color;
                var eq_result := EquipItem(npc, copia);
                if (eq_result.errortext)
                    DestroyItem(copia);
                endif
            else
            endif
        endif
    endforeach

    // Forca re-render completo: tira o NPC do alcance de visao do cliente (>18 tiles)
 // e traz de volta - o cliente recebe um "mobile incoming" novo com o graphic correto
    var rx := npc.x;
    var ry := npc.y;
    MoveObjectToLocation(npc, rx + 30, ry + 30, npc.z, npc.realm, MOVEOBJECT_FORCELOCATION);
    sleepms(200);
    MoveObjectToLocation(npc, rx, ry, npc.z, npc.realm, MOVEOBJECT_FORCELOCATION);

 SetObjProperty(npc, "imita_leitor", who.serial);

 SendSysMessageEX(who, "Algo com sua forma aparece. Com um delay inquietante, copia cada movimento seu.", SSM_INFO);
 start_script(":livros_lacrados:scripts/invocacoes/brain_coisa_imita",
        array{npc, who, ReadGameClock() + 120});
endfunction

function InvocarOvo(who, book, nivel, afinidade, lugar)
    var mochila := who.backpack;
    if (!mochila)
        return;
    endif

    var ovo := CreateItemInContainer(mochila, 0xFF8, 1);
    if (!ovo || ovo.errortext)
        return;
    endif

 SetName(ovo, "Ovo Lacrado");
 SetObjProperty(ovo, "ovo_criado_em", ReadGameClock());
 SetObjProperty(ovo, "ovo_leitor", who.serial);
 SetObjProperty(ovo, "ovo_nivel", nivel);

 SendSysMessageEX(who, "Um ovo cai das paginas e pousa em sua mochila.", SSM_INFO);
 SendSysMessageEX(who, "Carregue-o por 30 dias. Nao o quebre.", SSM_INFO);
endfunction
