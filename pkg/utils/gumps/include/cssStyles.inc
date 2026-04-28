/*
 * CSSSTYLES.INC
 *
 * Sistema de estilos semânticos CSS para gumps EpicUO.
 * Abstrai a formatação HTML em estilos nomeados reutilizáveis.
 *
 * Dois modos de uso:
 *   - WebView2: gera CSS inline completo (color, font-size, font-weight, etc.)
 *   - GFHTMLArea (legado): gera <BASEFONT> + tags inline UO
 *
 * Dependências: htmlBuilder.inc
 *
 * Constantes de estilo:
 *   CSS_TITLE, CSS_HEADING_1, CSS_HEADING_2, CSS_HEADING_3
 *   CSS_BODY, CSS_LABEL, CSS_CAPTION
 *   CSS_ALERT, CSS_ERROR, CSS_SUCCESS, CSS_INFO
 *   CSS_LINK, CSS_NARRATOR, CSS_WHISPER, CSS_EMOTE
 *
 * Funções:
 *   CssApply(style, text)              → string HTML formatada
 *   CssHeading(level, text, style)     → heading HTML com estilo
 *   CssCreateStyle(hex, px, bold, italic, align) → struct de estilo customizado
 *   CssBlock(style, content)           → div com o estilo aplicado
 */

use basic;

include ":gumps:htmlBuilder";

// ─────────────────────────────────────────────
//  Paleta de cores EpicUO (hex HTML)
// ─────────────────────────────────────────────

// Tons dourados (molduras, títulos)
const CSS_HEX_GOLD         := "#C8A040";
const CSS_HEX_GOLD_LIGHT   := "#E8C060";
const CSS_HEX_GOLD_DARK    := "#8B6914";

// Tons de texto
const CSS_HEX_WHITE        := "#EEEEEE";
const CSS_HEX_GRAY         := "#AAAAAA";
const CSS_HEX_GRAY_DARK    := "#666666";

// Estado / feedback
const CSS_HEX_RED          := "#FF4444";
const CSS_HEX_GREEN        := "#44CC77";
const CSS_HEX_BLUE         := "#4499FF";
const CSS_HEX_YELLOW       := "#FFD700";
const CSS_HEX_ORANGE       := "#FF8C00";

// Roleplay / sistema
const CSS_HEX_NARRATOR     := "#DDBB88";
const CSS_HEX_WHISPER      := "#99BBCC";
const CSS_HEX_EMOTE        := "#CC99CC";
const CSS_HEX_SYSTEM       := "#FFDD55";

// ─────────────────────────────────────────────
//  Definição de estilos (structs)
// ─────────────────────────────────────────────

/*
 * _CssMakeStyle(hexColor, sizePx, bold, italic, align)
 * Cria um struct de estilo. Uso interno e para CssCreateStyle().
 */
function _CssMakeStyle(hexColor, sizePx, bold := false, italic := false, align := "left")
    var s := struct;
    s.+hexColor := hexColor;
    s.+sizePx   := sizePx;
    s.+bold     := bold;
    s.+italic   := italic;
    s.+align    := align;
    return s;
endfunction

// Títulos
const CSS_TITLE     := 1;
const CSS_HEADING_1 := 2;
const CSS_HEADING_2 := 3;
const CSS_HEADING_3 := 4;

// Texto geral
const CSS_BODY      := 5;
const CSS_LABEL     := 6;
const CSS_CAPTION   := 7;

// Feedback
const CSS_ALERT     := 8;
const CSS_ERROR     := 9;
const CSS_SUCCESS   := 10;
const CSS_INFO      := 11;

// Comunicação
const CSS_LINK      := 12;
const CSS_NARRATOR  := 13;
const CSS_WHISPER   := 14;
const CSS_EMOTE     := 15;

/*
 * _CssGetStyle(styleConst)
 * Retorna o struct de estilo para a constante dada.
 */
function _CssGetStyle(styleConst)
    case (styleConst)
        CSS_TITLE:     return _CssMakeStyle(CSS_HEX_GOLD,      28, true,  false, "center");
        CSS_HEADING_1: return _CssMakeStyle(CSS_HEX_GOLD,      22, true,  false, "left");
        CSS_HEADING_2: return _CssMakeStyle(CSS_HEX_GOLD_LIGHT, 18, false, false, "left");
        CSS_HEADING_3: return _CssMakeStyle(CSS_HEX_GRAY,      15, false, false, "left");
        CSS_BODY:      return _CssMakeStyle(CSS_HEX_WHITE,     14, false, false, "left");
        CSS_LABEL:     return _CssMakeStyle(CSS_HEX_GRAY,      13, false, false, "left");
        CSS_CAPTION:   return _CssMakeStyle(CSS_HEX_GRAY_DARK, 11, false, true,  "left");
        CSS_ALERT:     return _CssMakeStyle(CSS_HEX_YELLOW,    14, true,  false, "left");
        CSS_ERROR:     return _CssMakeStyle(CSS_HEX_RED,       14, true,  false, "left");
        CSS_SUCCESS:   return _CssMakeStyle(CSS_HEX_GREEN,     14, false, false, "left");
        CSS_INFO:      return _CssMakeStyle(CSS_HEX_BLUE,      14, false, false, "left");
        CSS_LINK:      return _CssMakeStyle(CSS_HEX_BLUE,      14, false, false, "left");
        CSS_NARRATOR:  return _CssMakeStyle(CSS_HEX_NARRATOR,  14, false, true,  "center");
        CSS_WHISPER:   return _CssMakeStyle(CSS_HEX_WHISPER,   13, false, true,  "left");
        CSS_EMOTE:     return _CssMakeStyle(CSS_HEX_EMOTE,     13, false, true,  "left");
        default:       return _CssMakeStyle(CSS_HEX_WHITE,     14, false, false, "left");
    endcase
endfunction

// ─────────────────────────────────────────────
//  Funções públicas
// ─────────────────────────────────────────────

/*
 * CssApply(styleConst, text)
 * Aplica um estilo pré-definido ao texto e retorna string HTML.
 *
 * Exemplo:
 *   CssApply(CSS_ALERT, "Cuidado! Área de combate.")
 *   → <span style="color:#FFD700;font-size:14px;font-weight:bold">Cuidado! Área de combate.</span>
 */
function CssApply(styleConst, text)
    var s := _CssGetStyle(styleConst);
    return HtmlStyled(text, s.hexColor, s.sizePx, s.bold, s.italic);
endfunction

/*
 * CssHeading(level, text, styleConst)
 * Cria um <h1..h6> formatado com o estilo informado (ou CSS_HEADING_1 por padrão).
 */
function CssHeading(level, text, styleConst := CSS_HEADING_1)
    var s := _CssGetStyle(styleConst);
    var styled := HtmlStyled(text, s.hexColor, s.sizePx, s.bold, s.italic);
    return HtmlHeading(level, styled);
endfunction

/*
 * CssBlock(styleConst, content)
 * Envolve conteúdo HTML em um <div> com alinhamento do estilo.
 */
function CssBlock(styleConst, content)
    var s := _CssGetStyle(styleConst);
    return HtmlDiv(content, $"text-align:{s.align}");
endfunction

/*
 * CssCreateStyle(hexColor, sizePx, bold, italic, align)
 * Cria um estilo customizado (para uso com CssApplyCustom).
 *
 * Exemplo:
 *   var meuEstilo := CssCreateStyle("#FF00FF", 16, true, false, "center");
 */
function CssCreateStyle(hexColor, sizePx := 14, bold := false, italic := false, align := "left")
    return _CssMakeStyle(hexColor, sizePx, bold, italic, align);
endfunction

/*
 * CssApplyCustom(styleStruct, text)
 * Aplica um estilo criado com CssCreateStyle().
 */
function CssApplyCustom(styleStruct, text)
    return HtmlStyled(text, styleStruct.hexColor, styleStruct.sizePx, styleStruct.bold, styleStruct.italic);
endfunction

/*
 * CssToCssString(styleStruct)
 * Converte um estilo em uma string CSS inline (para uso em style="...").
 * Útil ao construir HTML manualmente.
 */
function CssToCssString(styleStruct)
    var css := $"color:{styleStruct.hexColor};font-size:{styleStruct.sizePx}px";
    if (styleStruct.bold)
        css := css + ";font-weight:bold";
    endif
    if (styleStruct.italic)
        css := css + ";font-style:italic";
    endif
    if (styleStruct.align && styleStruct.align != "left")
        css := css + $";text-align:{styleStruct.align}";
    endif
    return css;
endfunction
