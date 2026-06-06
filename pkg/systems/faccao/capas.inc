// capas.inc — Capas de facção (lista configurável + venda no mural)
//
// Persistência (props da guild):
//   guild.cape_color : int  — hue único da facção
//   guild.capes      : array de struct{ nome, descmod }
//                      descmod aceita até 3 linhas separadas por "<br>"
//
// Helpers públicos:
//   _CriarCapaItem(player, guild, capa)   — instancia 0x1515 com cor/nome/descmod/hp
//   _CriarCapasHandler(who, guild)        — gump GM (configurar cor + CRUD lista)
//   _ComprarCapaHandler(who, guild)       — gump comprador (listar + comprar 25 cobres)

use uo;
use os;
use guilds;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:yesNo";
include ":tn:tngumps";
include ":ligafaccao:include/coins";

const CAPA_GRAPHIC := 0x1515;
const CAPA_HP      := 400;
const CAPA_PRECO   := 25;

// Retorna só a primeira linha de uma descmod (até o primeiro "<br>"), ou a string
// inteira se não houver "<br>".
function _PrimeiraLinha(texto)
    if (!texto || texto == error) return ""; endif
    var n := Len(texto);
    if (n < 4) return texto; endif
    var i := 1;
    while (i <= n - 3)
        if (texto[i, 4] == "<br>")
            if (i == 1) return ""; endif
            return texto[1, i - 1];
        endif
        i := i + 1;
    endwhile
    return texto;
endfunction

// Coleta até 3 linhas via RequestInput. Linha 1 é obrigatória (esc cancela tudo).
// Linhas 2 e 3 são opcionais — esc só pula a próxima.
// Retorna a string concatenada com "<br>" entre linhas, ou 0 se cancelado.
function _CapaColetarDescmod(who)
    var linha1 := RequestInput(who, who.backpack, "Linha 1 da descricao (esc cancela):");
    if (!linha1 || linha1 == "" || linha1 == error)
        return 0;
    endif
    var resultado := linha1;

    var linha2 := RequestInput(who, who.backpack, "Linha 2 (esc pula):");
    if (linha2 && linha2 != "" && linha2 != error)
        resultado := resultado + "<br>" + linha2;

        var linha3 := RequestInput(who, who.backpack, "Linha 3 (esc pula):");
        if (linha3 && linha3 != "" && linha3 != error)
            resultado := resultado + "<br>" + linha3;
        endif
    endif
    return resultado;
endfunction

// Cria a capa no backpack do comprador com cor/nome/descmod da entrada e HP 400.
// Retorna o item criado, ou 0 se falhou.
function _CriarCapaItem(player, guild, capa)
    var item := CreateItemInBackpack(player, CAPA_GRAPHIC, 1);
    if (!item) return 0; endif
    var hue := CInt(guild.GetProp("cape_color"));
    if (hue > 0)
        item.color := hue;
    endif
    SetName(item, capa.nome);
    SetObjProperty(item, "descmod", capa.descmod + "<br>");
    item.hp    := CAPA_HP;
    item.maxhp := CAPA_HP;
    return item;
endfunction

const CAPAS_POR_PAGINA := 4;

// _CriarCapasHandler — gump GM para configurar cor da facção e CRUD da lista de capas.
// Pagina a lista em blocos de CAPAS_POR_PAGINA. Reabre o gump após cada edição.
function _CriarCapasHandler(who, guild)
    var pagina := 1;
    while (who.connected)
        var hue_raw := guild.GetProp("cape_color");
        var hue     := 0;
        if (hue_raw != error) hue := CInt(hue_raw); endif

        var capas := guild.GetProp("capes");
        if (!capas || capas == error) capas := array{}; endif

        // Calcular página atual e limites
        var total := capas.size();
        var total_paginas := 1;
        if (total > 0) total_paginas := CInt((total + CAPAS_POR_PAGINA - 1) / CAPAS_POR_PAGINA); endif
        if (pagina > total_paginas) pagina := total_paginas; endif
        if (pagina < 1) pagina := 1; endif
        var inicio := (pagina - 1) * CAPAS_POR_PAGINA + 1;
        var fim    := inicio + CAPAS_POR_PAGINA - 1;
        if (fim > total) fim := total; endif

        var hue_texto := "(nao definida)";
        var hue_cor   := 2100;
        if (hue > 0)
            hue_texto := CStr(hue);
            hue_cor   := hue;
        endif

        var gump := GFCreateGump();
        GFResizePic(gump, 80, 60, 9270, 460, 380);
        GFHtmlShadowed(gump, 80, 75, 460, 18, "#FFC33C", "<center>Capas da Faccao", 7);

        // Linha da cor
        GFTextLine(gump, 110, 110, 2100, "Cor da faccao:");
        GFTextLine(gump, 230, 110, hue_cor, hue_texto);
        GFAddButton(gump, 360, 112, 0x845, 0x846, GF_CLOSE_BTN, 1); // alterar cor
        GFTextLine(gump, 380, 110, 2100, "Alterar");

        // Cabeçalho da lista + indicador de página
        GFTextLine(gump, 110, 145, 2100, "Capas configuradas:");
        if (total_paginas > 1)
            GFTextLine(gump, 360, 145, 2100,
                       "Pagina " + CStr(pagina) + "/" + CStr(total_paginas));
        endif

        var y := 170;
        var idx := inicio;
        while (idx <= fim)
            sleepms(2);
            var capa := capas[idx];
            var nome_capa := capa.nome;
            if (!nome_capa || nome_capa == error) nome_capa := "(sem nome)"; endif
            GFTextLine(gump, 110, y, hue_cor, CStr(nome_capa));

            var desc_capa := capa.descmod;
            if (!desc_capa || desc_capa == error) desc_capa := ""; endif
            var primeira := _PrimeiraLinha(desc_capa);
            if (Len(primeira) > 38) primeira := primeira[1, 35] + "..."; endif
            GFTextLine(gump, 130, y + 16, 2100, "\"" + primeira + "\"");

            GFAddButton(gump, 470, y + 4, 4017, 4019, GF_CLOSE_BTN, 100 + idx); // remover
            y += 38;
            idx += 1;
        endwhile

        if (total == 0)
            GFTextLine(gump, 110, y, 2100, "(nenhuma capa configurada)");
        endif

        // Navegação de páginas (acima dos botões de ação)
        if (pagina > 1)
            GFAddButton(gump, 110, 335, 0x15E3, 0x15E7, GF_CLOSE_BTN, 3); // anterior
            GFTextLine(gump, 130, 333, 2100, "Anterior");
        endif
        if (pagina < total_paginas)
            GFAddButton(gump, 260, 335, 0x15E1, 0x15E5, GF_CLOSE_BTN, 4); // próxima
            GFTextLine(gump, 280, 333, 2100, "Proxima");
        endif

        // Botões de ação
        GFAddButton(gump, 110, 360, 0x845, 0x846, GF_CLOSE_BTN, 2); // adicionar
        GFTextLine(gump, 130, 358, 2100, "Adicionar Nova Capa");

        GFAddButton(gump, 400, 360, 0x845, 0x846, GF_CLOSE_BTN, 999); // fechar
        GFTextLine(gump, 420, 358, 2100, "Fechar");

        var input := GFSendGump(who, gump);
        var btn := input[0];

        if (btn == 999 || btn == 0)
            return 1;
        elseif (btn == 1)
            // Alterar cor
            var raw := RequestInput(who, who.backpack, "Hue da capa (1 a 3000):");
            if (!raw || raw == error || raw == "") continue; endif
            var novo := CInt(raw);
            if (novo < 1 || novo > 3000)
                SendSysMessageEx(who, "Hue invalido. Use numero entre 1 e 3000.", SSM_FAIL);
                continue;
            endif
            guild.SetProp("cape_color", novo);
            SendSysMessageEx(who, "Cor da faccao: " + CStr(novo), SSM_INFO);
        elseif (btn == 2)
            // Adicionar nova capa
            var nome := RequestInput(who, who.backpack, "Nome da capa (ex: Capa de Recruta):");
            if (!nome || nome == error || nome == "") continue; endif

            var descmod := _CapaColetarDescmod(who);
            if (!descmod) continue; endif

            capas.append(struct{
                "nome"    := nome,
                "descmod" := descmod
            });
            guild.SetProp("capes", capas);
            // Pula direto para a página onde a nova capa apareceu
            pagina := CInt((capas.size() + CAPAS_POR_PAGINA - 1) / CAPAS_POR_PAGINA);
            SendSysMessageEx(who, "Capa '" + nome + "' adicionada.", SSM_INFO);
        elseif (btn == 3)
            // Página anterior
            pagina := pagina - 1;
        elseif (btn == 4)
            // Próxima página
            pagina := pagina + 1;
        elseif (btn >= 101 && btn <= 100 + total)
            // Remover capa (índice global)
            var rem_idx := btn - 100;
            var alvo := capas[rem_idx];
            var nome_alvo := alvo.nome;
            if (!nome_alvo || nome_alvo == error) nome_alvo := "(sem nome)"; endif
            if (!YesNo(who, "Remover a capa '" + nome_alvo + "'?"))
                continue;
            endif
            capas.erase(rem_idx);
            guild.SetProp("capes", capas);
            SendSysMessageEx(who, "Capa '" + nome_alvo + "' removida.", SSM_INFO);
        endif
    endwhile
    return 1;
endfunction

// _ComprarCapaHandler — gump para o líder/delegado comprar capas da facção.
// Mostra a lista configurada e cobra 25 cobres por compra.
function _ComprarCapaHandler(who, guild)
    var hue_raw := guild.GetProp("cape_color");
    var hue     := 0;
    if (hue_raw != error) hue := CInt(hue_raw); endif

    var capas := guild.GetProp("capes");
    if (!capas || capas == error) capas := array{}; endif

    if (hue <= 0 || capas.size() == 0)
        SendSysMessageEx(who, "Esta faccao ainda nao tem capas configuradas.", SSM_FAIL);
        return 0;
    endif

    var guild_name := guild.GetProp("name");
    if (!guild_name || guild_name == error) guild_name := "Faccao"; endif

    while (who.connected)
        capas := guild.GetProp("capes");
        if (!capas || capas == error) capas := array{}; endif
        if (capas.size() == 0)
            SendSysMessageEx(who, "Esta faccao nao tem mais capas configuradas.", SSM_FAIL);
            return 0;
        endif

        var gump := GFCreateGump();
        GFResizePic(gump, 80, 60, 9270, 460, 400);
        GFHtmlShadowed(gump, 80, 75, 460, 18, "#FFC33C",
                       "<center>Capas - " + CStr(guild_name), 7);

        var y := 115;
        var indice := 0;
        foreach capa in capas
            sleepms(2);
            indice += 1;
            var nome_capa := capa.nome;
            if (!nome_capa || nome_capa == error) nome_capa := "(sem nome)"; endif
            GFTextLine(gump, 110, y, hue, CStr(nome_capa));

            var desc_capa := capa.descmod;
            if (!desc_capa || desc_capa == error) desc_capa := ""; endif
            var primeira := _PrimeiraLinha(desc_capa);
            if (Len(primeira) > 42) primeira := primeira[1, 39] + "..."; endif
            GFTextLine(gump, 130, y + 16, 2100, "\"" + primeira + "\"");

            GFTextLine(gump, 360, y, 2100, CStr(CAPA_PRECO) + " cobres");
            GFAddButton(gump, 460, y + 4, 0x845, 0x846, GF_CLOSE_BTN, indice);
            y += 40;
            if (y > 360) break; endif
        endforeach

        GFAddButton(gump, 400, 380, 0x845, 0x846, GF_CLOSE_BTN, 999); // fechar
        GFTextLine(gump, 420, 378, 2100, "Fechar");

        var input := GFSendGump(who, gump);
        var btn := input[0];

        if (btn == 999 || btn == 0 || btn > capas.size())
            return 1;
        endif

        var capa := capas[btn];
        if (CountCoins(who) < CAPA_PRECO)
            SendSysMessageEx(who, "Cobres insuficientes. Precisa de " + CStr(CAPA_PRECO) + " cobres.",
                             SSM_FAIL);
            continue;
        endif
        if (!TakeCoins(who, CAPA_PRECO))
            SendSysMessageEx(who, "Erro ao processar pagamento.", SSM_FAIL);
            continue;
        endif

        var item := _CriarCapaItem(who, guild, capa);
        if (!item)
            CreateItemInBackpack(who, MOEDA_COBRE, CAPA_PRECO);
            SendSysMessageEx(who, "Nao foi possivel criar a capa (mochila cheia?).", SSM_FAIL);
            continue;
        endif

        AddReport(guild, who.name + " comprou '" + capa.nome + "' por " + CStr(CAPA_PRECO) + "mc");
        SendSysMessageEx(who, "Voce comprou '" + capa.nome + "'.", SSM_INFO);
    endwhile
    return 1;
endfunction
