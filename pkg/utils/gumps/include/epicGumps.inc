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
	HIDE_TITLE, HIDE_PAGE, HIDE_BORDER,
	BORDER_GOLDEN, BORDER_BLUE, BORDER_MARBLE,
	NO_CLOSABLE
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
function GFECreateGump(title, width, height, params := array{})
	var gump := GFCreateGump(0, 0, width, height);
	var x, y;
	
	if (NO_CLOSABLE in params)
		GFClosable(gump, 0);
	else
		GFClosable(gump, 1);
	endif
    GFDisposable(gump, 1);

	if (!(HIDE_BORDER in params))
		var border;
		if (BORDER_BLUE in params)
			border := BORDER_BLUE;
		elseif (BORDER_MARBLE in params)
			border := BORDER_MARBLE;
		else 
			border := BORDER_GOLDEN;
		endif
		GFEAddMoldura(gump, width, height, border);
	endif

	x := 20;
	y := 27;
	if (!(HIDE_TITLE in params))
		GFResizePic(gump, x, y, 9350, width - 27, 35);
		GFESetTitle(gump, width/2, title, 53);
		y += 40;
	endif

	if (!(HIDE_PAGE in params))
		GFResizePic(gump, x, y, 9350, width - 27, height - y - 8);
	endif

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
function GFEAddMoldura(byref gump, width, height, border)
	var background, molduras;
	if (border == BORDER_GOLDEN)
		background := 39929;
		molduras := array{
			// SIDES
			struct{ x := 0, y := 15, width := 35, height := height - 15, img := 39928 }, // LEFT
			struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 39930 }, // RIGHT

			// TOP
			struct{ x := 25, y := 0, width := width - 41, height := 35, img := 39926 }, // TOPMIDDLE
			struct{ x := 0, y := 0, width := 35, height := 35, img := 39925 }, // TOPLEFT
			struct{ x := width - 25, y := 0, width := 35, height := 35, img := 39927 }, // TOPRIGHT

			// BOTTOM
			struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 39932 }, // BOTTOMMIDDLE
			struct{ x := 0, y := height - 25, width := 35, height := 35, img := 39931 }, // BOTTOMLEFT
			struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 39933 } // BOTTOMRIGHT
		};
	elseif (border == BORDER_BLUE)
		background := 40004;
		molduras := array{
			// SIDES
			struct{ x := 0, y := 15, width := 35, height := height - 15, img := 40003 }, // LEFT
			struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 40005 }, // RIGHT

			// TOP
			struct{ x := 25, y := 0, width := width - 41, height := 15, img := 40001 }, // TOPMIDDLE
			struct{ x := 0, y := 0, width := 35, height := 15, img := 40000 }, // TOPLEFT
			struct{ x := width - 25, y := 0, width := 35, height := 15, img := 40002 }, // TOPRIGHT

			// BOTTOM
			struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 40007 }, // BOTTOMMIDDLE
			struct{ x := 0, y := height - 25, width := 35, height := 35, img := 40006 }, // BOTTOMLEFT
			struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 40008 } // BOTTOMRIGHT
		};
	elseif (border == BORDER_MARBLE)
		background := 3504;
		molduras := array{
			// SIDES
			struct{ x := 0, y := 15, width := 35, height := height - 15, img := 3503 }, // LEFT
			struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 3505 }, // RIGHT

			// TOP
			struct{ x := 25, y := 0, width := width - 41, height := 15, img := 3501 }, // TOPMIDDLE
			struct{ x := 0, y := 0, width := 35, height := 15, img := 3500 }, // TOPLEFT
			struct{ x := width - 25, y := 0, width := 35, height := 15, img := 3502 }, // TOPRIGHT

			// BOTTOM
			struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 3507 }, // BOTTOMMIDDLE
			struct{ x := 0, y := height - 25, width := 35, height := 35, img := 3506 }, // BOTTOMLEFT
			struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 3508 } // BOTTOMRIGHT
		};
	endif

	GFESetBackground(gump, width - 20, height - 20, background);
    foreach moldura in molduras
        GFGumpPicTiled(gump, moldura.x, moldura.y, moldura.width, moldura.height, moldura.img);
    endforeach
endfunction

function GFESetTitle(byref gump, x, title, color := 0)
	GFTextMid(gump, x, 35, Len(title)*2, color, title);
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