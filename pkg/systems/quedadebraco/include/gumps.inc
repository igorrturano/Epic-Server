use uo;
use os;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include "quedadebraco";
include ":gumps:epicGumps";

function ShowMoveSelectionGump(who, round_num)
    while (1)
        var gump := GFECreateGump("Escolha seu Movimento", 225, 155, {HIDE_PAGE, BORDER_BLUE});
        var x := 25;
        var y := 70; // Movido mais para baixo

        // Três ícones lado a lado, com o do meio em destaque (vermelho)
        if (GetObjProperty(who, "#LastMove") == CONST_MOVE_HOOK)
            GFGumpPic(gump, x-13, y-23, 30062); // Indicador de seleção
            GFGumpPic(gump, x, y, 125, COLOR_RED); // Gancho em vermelho
            x += 70;
            GFAddButton(gump, x, y, 129, 129, GF_CLOSE_BTN, CONST_MOVE_COVER);
            x += 70;
            GFAddButton(gump, x, y, 126, 126, GF_CLOSE_BTN, CONST_MOVE_PRESSURE);
        elseif (GetObjProperty(who, "#LastMove") == CONST_MOVE_COVER)
            GFAddButton(gump, x, y, 125, 125, GF_CLOSE_BTN, CONST_MOVE_HOOK);
            x += 70;
            GFGumpPic(gump, x-13, y-23, 30062);
            GFGumpPic(gump, x, y, 129, COLOR_RED);
            x += 70;
            GFAddButton(gump, x, y, 126, 126, GF_CLOSE_BTN, CONST_MOVE_PRESSURE);
        elseif (GetObjProperty(who, "#LastMove") == CONST_MOVE_PRESSURE)
            GFAddButton(gump, x, y, 125, 125, GF_CLOSE_BTN, CONST_MOVE_HOOK);
            x += 70;
            GFAddButton(gump, x, y, 129, 129, GF_CLOSE_BTN, CONST_MOVE_COVER);
            x += 70;
            GFGumpPic(gump, x-13, y-23, 30062);
            GFGumpPic(gump, x, y, 126, COLOR_RED);
        else
            // Estado inicial, nenhum selecionado ainda
            GFAddButton(gump, x, y, 125, 125, GF_CLOSE_BTN, CONST_MOVE_HOOK);
            x += 70;
            GFAddButton(gump, x, y, 129, 129, GF_CLOSE_BTN, CONST_MOVE_COVER);
            x += 70;
            GFAddButton(gump, x, y, 126, 126, GF_CLOSE_BTN, CONST_MOVE_PRESSURE);
        endif

        // Nomes dos golpes abaixo dos ícones
        x := 15; // Reset x position
        y += 45; // Mais espaço entre ícones e texto
        GFTextMid(gump, x, y, 70, 2103, "Gancho");
        GFTextMid(gump, x+70, y, 70, 2103, "Alavanca"); 
        GFTextMid(gump, x+140, y, 70, 2103, "Pressão");

        var input := GFSendGump(who, gump);
        
        if (input[0] == 0)
            return CONST_MOVE_NONE;
        endif

        SetObjProperty(who, "#LastMove", input[0]);
        return input[0];
    endwhile
endfunction