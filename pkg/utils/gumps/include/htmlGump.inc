/*
 * HTMLGUMP.INC
 *
 * Funções de alto nível para exibir gumps com conteúdo HTML formatado.
 * Usa htmlBuilder.inc e cssStyles.inc para composição de conteúdo.
 *
 * Para HTML/CSS completo (tabelas, flexbox, imagens) use webPanel.inc.
 * Estas funções usam GFHTMLArea e são compatíveis com todos os clientes UO.
 *
 * Funções:
 *   SendHTMLGump(mobile, title, text)
 *   SendStyledHTMLGump(mobile, title, bodyParts, width, height, options)
 */

use uo;
use os;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:htmlBuilder";
include ":gumps:cssStyles";

/*
 * SendHTMLGump(mobile, title, text)
 *
 * Exibe um gump simples com área HTML rolável.
 * Compatível com todos os clientes UO (usa GFHTMLArea).
 *
 * Parâmetros:
 *   mobile: O personagem que receberá o gump
 *   title:  Título exibido no topo
 *   text:   Conteúdo HTML (use htmlBuilder.inc para compor)
 *
 * Retorno: resultado do gump
 *
 * Exemplo:
 *   var corpo := HtmlBold("Bem-vindo!") + HtmlBr() + CssApply(CSS_BODY, "Texto normal.");
 *   SendHTMLGump(who, "Aviso", corpo);
 */
function SendHTMLGump(mobile, title, text)
    var gump := GFCreateGump(220, 20);
    GFResizePic(gump, 0, 0, GFGetCfgConst("Defaults", "BackGround"), 360, 480);
    GFResizePic(gump, 15, 12, GFGetCfgConst("Defaults", "ForeGround"), 330, 30);
    GFResizePic(gump, 15, 438, GFGetCfgConst("Defaults", "ForeGround"), 330, 30);
    GFResizePic(gump, 15, 45, GFGetCfgConst("Backgrounds", "GOLDTRIM"), 330, 390);

    GFAddAlphaRegion(gump, 0, 0, 360, 480);

    GFTextMid(gump, 15, 17, 370, 1720, title);

    GFHTMLArea(gump, 22, 53, 322, 375, text, 0, 1);

    GFAddButton(gump, 150, 443, 2128, 2129, GF_CLOSE_BTN, 0xA1);

    return GFSendGump(mobile, gump);
endfunction

/*
 * SendStyledHTMLGump(mobile, title, bodyParts, width, height, options)
 *
 * Gump HTML com tamanho e conteúdo customizáveis.
 *
 * Parâmetros:
 *   mobile:    O personagem
 *   title:     Título do gump
 *   bodyParts: Array de strings HTML — concatenadas na ordem
 *   width:     Largura total (padrão 400)
 *   height:    Altura total (padrão 500)
 *   options:   Array de opções (usa enum GUMP_OPTIONS de epicGumps.inc)
 *
 * Retorno: resultado do gump
 *
 * Exemplo:
 *   var partes := array{
 *       CssApply(CSS_HEADING_1, "Relatório"),
 *       HtmlBr(),
 *       HtmlTable(dados, array{"Nome", "Valor"})
 *   };
 *   SendStyledHTMLGump(who, "Relatório de Batalha", partes);
 */
function SendStyledHTMLGump(mobile, title, bodyParts, width := 400, height := 500, options := array{})
    var gump := GFECreateGump(title, width, height, options);

    var innerX := 27;
    var innerY := 75;
    var innerW := width - 54;
    var innerH := height - 110;

    var htmlContent := HtmlJoin(bodyParts);

    GFHTMLArea(gump, innerX, innerY, innerW, innerH, htmlContent, 0, 1);

    GFAddButton(gump, CInt(width / 2) - 15, height - 27, 2128, 2129, GF_CLOSE_BTN, 0xA1);

    return GFSendGump(mobile, gump);
endfunction
