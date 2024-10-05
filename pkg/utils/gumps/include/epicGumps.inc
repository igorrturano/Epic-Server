///////////////////////////////////
// Adicionado para o Epic! Shard //
///////////////////////////////////

use uo;
use os;
use basic;

include "include/client";
include "gumps";
include "gumps_ex";


enum GUMP_PARAMS
	HIDE_BACKGROUND := "HIDE_BACKGROUND",
	HIDE_TITLE := "HIDE_TITLE"
endenum
/**
 * Purpose 
 * Criar um gump padronizado com os demais do Shard.
 *
 * Parameters
 * title: O gump que será adicionado a moldura
 * width: Largura
 * height: Altura
 * background: Imagem de fundo (caso queira alterar)
 * params: Array com demais parâmetros (usando as consts acima) para modificar a forma como o gump é criado, se terá barra para título, scroll, etc.
 *
 * Return gump
 * 
 */
function GFECreateGump(title, width, height, background := 39929, params := array{})
	var gump := GFCreateGump(0, 0, width, height);
	var x, y;
	GFClosable(gump, 1);
    GFDisposable(gump, 0);

	if (!(HIDE_BACKGROUND in params))
		GFESetBackground(gump, width - 20, height - 20, background);
	endif
	GFEAddMoldura(gump, width, height);

	x := 20;
	y := 30;
	GFResizePic(gump, x, y, 9350, width - 27, 35);

	if (!(HIDE_TITLE in params))
		GFESetTitle(gump, width/2, title, 53);
	endif
	
	y += 40;
	GFResizePic(gump, x, y, 9350, width - 27, height - y - 8);
	return gump;
endfunction

/**
 * Purpose 
 * Adicionar moldura ao Gump
 *
 * Parameters
 * gump: O gump que será adicionado a moldura
 * width: Largura
 * height: Altura
 */
function GFEAddMoldura(byref gump, width, height)
    var molduras := array{
        struct{ x := 0, y := 0, width := 25, height := 25, img := 39925 },
        struct{ x := width - 25, y := 0, width := 35, height := 25, img := 39927 },
        struct{ x := 25, y := 0, width := width - 41, height := 25, img := 39926 },
        struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 39932 },
        struct{ x := 0, y := 25, width := 10, height := height - 50, img := 39928 },
        struct{ x := width - 25, y := 25, width := 35, height := height - 50, img := 39930 },
        struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 39933 },
        struct{ x := 0, y := height - 25, width := 35, height := 35, img := 39931 }
    };

    foreach moldura in molduras
        GFGumpPicTiled(gump, moldura.x, moldura.y, moldura.width, moldura.height, moldura.img);
    endforeach
endfunction

function GFESetTitle(byref gump, x, title, color := 0)
	GFTextMid(gump, x, 40, Len(title)*2, color, title);
endfunction

/**
 * Parameters
 * gump: Gump para adicionar as linhas
 * arrTextos: Array de frases para adicionar
 * incrY: Valor a ser incrementado em y a cada linha
 * color: Cor do texto
 */

function GFESetBackground(byref gump, width, height, imgID)
	GFGumpPicTiled(gump, 10, 10, width, height, imgID);
endfunction

function GFEAddTextBox(byref gump, x, y, width, height := 16, text_color := COLOR_YELLOW, default_text := 0, id := 0, maxlenght := 0)
	GFResizePic(gump, x, y, GFGetCfgConst("BackGrounds", "GOLDTRIM"), width + 10, height + 15);
	var val := GFTextEntry(gump, x + 6, y + 6, width, height, text_color, default_text, id, maxlenght);
	return val;
endfunction

function GFEAdd(byref gump)

endfunction