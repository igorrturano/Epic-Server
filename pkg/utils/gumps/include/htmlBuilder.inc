/*
 * HTMLBUILDER.INC
 *
 * Biblioteca para composição de strings HTML destinadas a dois contextos:
 *   1. GFHTMLArea (gumps clássicos UO) — usa <BASEFONT>, <b>, <i>, <center>, etc.
 *   2. WebView2 (painel moderno) — usa CSS inline completo (HTML5/CSS3).
 *
 * Todas as funções retornam strings HTML. Nenhuma delas chama GFHTMLArea.
 * Use webPanel.inc para enviar conteúdo ao painel WebView2.
 *
 * Função List
 *   HtmlBold(text)
 *   HtmlItalic(text)
 *   HtmlUnderline(text)
 *   HtmlColor(text, hexColor)
 *   HtmlSize(text, sizePx)
 *   HtmlStyled(text, hexColor, sizePx, bold, italic)
 *   HtmlCenter(text)
 *   HtmlLeft(text)
 *   HtmlRight(text)
 *   HtmlHeading(level, text)
 *   HtmlParagraph(text)
 *   HtmlBr()
 *   HtmlLink(text, url)
 *   HtmlTable(rows, headers)
 *   HtmlList(items, ordered)
 *   HtmlImage(src, width, height, alt)
 *   HtmlDiv(text, cssStyle)
 *   HtmlSpan(text, cssStyle)
 *   HtmlShadow(text, hexColor, sizePx)
 *   HtmlSeparator()
 *   HtmlJoin(parts)
 *
 * Classe HtmlBuilder (fluent builder)
 *   Append(text)
 *   Bold(text)
 *   Italic(text)
 *   Underline(text)
 *   Color(text, hexColor)
 *   Size(text, sizePx)
 *   Styled(text, hexColor, sizePx, bold, italic)
 *   Heading(level, text)
 *   Paragraph(text)
 *   Center(text)
 *   Br()
 *   Link(text, url)
 *   Table(rows, headers)
 *   List(items, ordered)
 *   Separator()
 *   Build()
 *   Clear()
 */

use basic;

// ─────────────────────────────────────────────
//  Funções de inline tags
// ─────────────────────────────────────────────

function HtmlBold(text)
    return "<b>" + CStr(text) + "</b>";
endfunction

function HtmlItalic(text)
    return "<i>" + CStr(text) + "</i>";
endfunction

function HtmlUnderline(text)
    return "<u>" + CStr(text) + "</u>";
endfunction

/*
 * HtmlColor(text, hexColor)
 * hexColor: "#RRGGBB" ou "#RRGGBBAA"
 * Compatível com GFHTMLArea (via <BASEFONT COLOR=>) e WebView2 (via style=).
 */
function HtmlColor(text, hexColor)
    return $"<span style=\"color:{hexColor}\">{text}</span>";
endfunction

/*
 * HtmlSize(text, sizePx)
 * sizePx: tamanho em pixels (ex: 14, 18, 24)
 */
function HtmlSize(text, sizePx)
    return $"<span style=\"font-size:{sizePx}px\">{text}</span>";
endfunction

/*
 * HtmlStyled(text, hexColor, sizePx, bold, italic)
 * Combina cor, tamanho, negrito e itálico em um único <span>.
 */
function HtmlStyled(text, hexColor, sizePx := 14, bold := false, italic := false)
    var styleStr := $"color:{hexColor};font-size:{sizePx}px";
    if (bold)
        styleStr := styleStr + ";font-weight:bold";
    endif
    if (italic)
        styleStr := styleStr + ";font-style:italic";
    endif
    return $"<span style=\"{styleStr}\">{text}</span>";
endfunction

// ─────────────────────────────────────────────
//  Funções de alinhamento
// ─────────────────────────────────────────────

function HtmlCenter(text)
    return $"<div style=\"text-align:center\">{text}</div>";
endfunction

function HtmlLeft(text)
    return $"<div style=\"text-align:left\">{text}</div>";
endfunction

function HtmlRight(text)
    return $"<div style=\"text-align:right\">{text}</div>";
endfunction

// ─────────────────────────────────────────────
//  Funções de headings e parágrafos
// ─────────────────────────────────────────────

/*
 * HtmlHeading(level, text)
 * level: 1–6
 */
function HtmlHeading(level, text)
    var lvl := CInt(level);
    if (lvl < 1) lvl := 1; endif
    if (lvl > 6) lvl := 6; endif
    return $"<h{lvl}>{text}</h{lvl}>";
endfunction

function HtmlParagraph(text)
    return $"<p>{text}</p>";
endfunction

function HtmlBr()
    return "<br>";
endfunction

function HtmlSeparator()
    return "<hr style=\"border-color:#5a4010;margin:8px 0\">";
endfunction

// ─────────────────────────────────────────────
//  Links
// ─────────────────────────────────────────────

function HtmlLink(text, url)
    return $"<a href=\"{url}\">{text}</a>";
endfunction

// ─────────────────────────────────────────────
//  Tabelas
// ─────────────────────────────────────────────

/*
 * HtmlTable(rows, headers)
 * rows: array de arrays, onde cada sub-array é uma linha de células.
 * headers: array de strings para o cabeçalho (opcional, passa array{} para omitir).
 *
 * Exemplo:
 *   HtmlTable(array{ array{"Espada", "20 dano"}, array{"Escudo", "10 def"} },
 *             array{"Item", "Atributo"})
 */
function HtmlTable(rows, headers := array{})
    var html := "<table>";

    if (headers.size() > 0)
        html := html + "<thead><tr>";
        foreach h in headers
            html := html + $"<th>{h}</th>";
            SleepMs(1);
        endforeach
        html := html + "</tr></thead>";
    endif

    html := html + "<tbody>";
    foreach row in rows
        html := html + "<tr>";
        foreach cell in row
            html := html + $"<td>{cell}</td>";
            SleepMs(1);
        endforeach
        html := html + "</tr>";
        SleepMs(1);
    endforeach
    html := html + "</tbody></table>";

    return html;
endfunction

// ─────────────────────────────────────────────
//  Listas
// ─────────────────────────────────────────────

/*
 * HtmlList(items, ordered)
 * items: array de strings
 * ordered: true para <ol> (1, 2, 3…), false para <ul> (pontos)
 */
function HtmlList(items, ordered := false)
    var tag := ordered ? "ol" : "ul";
    var html := $"<{tag}>";
    foreach item in items
        html := html + $"<li>{item}</li>";
        SleepMs(1);
    endforeach
    html := html + $"</{tag}>";
    return html;
endfunction

// ─────────────────────────────────────────────
//  Imagens
// ─────────────────────────────────────────────

function HtmlImage(src, width := 0, height := 0, alt := "")
    var styleStr := "";
    if (width > 0 || height > 0)
        var wStr := width > 0 ? $"width:{width}px;" : "";
        var hStr := height > 0 ? $"height:{height}px;" : "";
        styleStr := $" style=\"{wStr}{hStr}\"";
    endif
    return $"<img src=\"{src}\"{styleStr} alt=\"{alt}\">";
endfunction

// ─────────────────────────────────────────────
//  Containers genéricos
// ─────────────────────────────────────────────

function HtmlDiv(text, cssStyle := "")
    if (cssStyle)
        return $"<div style=\"{cssStyle}\">{text}</div>";
    endif
    return $"<div>{text}</div>";
endfunction

function HtmlSpan(text, cssStyle := "")
    if (cssStyle)
        return $"<span style=\"{cssStyle}\">{text}</span>";
    endif
    return $"<span>{text}</span>";
endfunction

// ─────────────────────────────────────────────
//  Efeito sombra (equivalente a GFHtmlShadowed mas retorna string)
// ─────────────────────────────────────────────

/*
 * HtmlShadow(text, hexColor, sizePx)
 * Simula sombra usando text-shadow CSS (para WebView2).
 */
function HtmlShadow(text, hexColor, sizePx := 16)
    var style := $"color:{hexColor};font-size:{sizePx}px;text-shadow:1px 1px 3px #000000,0 0 6px #000000";
    return $"<span style=\"{style}\">{text}</span>";
endfunction

// ─────────────────────────────────────────────
//  Utilitário: juntar partes
// ─────────────────────────────────────────────

/*
 * HtmlJoin(parts)
 * parts: array de strings HTML — concatena sem separador.
 */
function HtmlJoin(parts)
    var result := "";
    foreach part in parts
        result := result + CStr(part);
        SleepMs(1);
    endforeach
    return result;
endfunction

// ─────────────────────────────────────────────
//  Classe HtmlBuilder (fluent)
// ─────────────────────────────────────────────

class HtmlBuilder()
    var _buffer := "";

    function HtmlBuilder(this)
        this._buffer := "";
    endfunction

    function Append(this, text)
        this._buffer := this._buffer + CStr(text);
        return this;
    endfunction

    function Bold(this, text)
        this._buffer := this._buffer + HtmlBold(text);
        return this;
    endfunction

    function Italic(this, text)
        this._buffer := this._buffer + HtmlItalic(text);
        return this;
    endfunction

    function Underline(this, text)
        this._buffer := this._buffer + HtmlUnderline(text);
        return this;
    endfunction

    function Color(this, text, hexColor)
        this._buffer := this._buffer + HtmlColor(text, hexColor);
        return this;
    endfunction

    function Size(this, text, sizePx)
        this._buffer := this._buffer + HtmlSize(text, sizePx);
        return this;
    endfunction

    function Styled(this, text, hexColor, sizePx := 14, bold := false, italic := false)
        this._buffer := this._buffer + HtmlStyled(text, hexColor, sizePx, bold, italic);
        return this;
    endfunction

    function Heading(this, level, text)
        this._buffer := this._buffer + HtmlHeading(level, text);
        return this;
    endfunction

    function Paragraph(this, text)
        this._buffer := this._buffer + HtmlParagraph(text);
        return this;
    endfunction

    function Center(this, text)
        this._buffer := this._buffer + HtmlCenter(text);
        return this;
    endfunction

    function Br(this)
        this._buffer := this._buffer + HtmlBr();
        return this;
    endfunction

    function Link(this, text, url)
        this._buffer := this._buffer + HtmlLink(text, url);
        return this;
    endfunction

    function Table(this, rows, headers := array{})
        this._buffer := this._buffer + HtmlTable(rows, headers);
        return this;
    endfunction

    function List(this, items, ordered := false)
        this._buffer := this._buffer + HtmlList(items, ordered);
        return this;
    endfunction

    function Separator(this)
        this._buffer := this._buffer + HtmlSeparator();
        return this;
    endfunction

    function Shadow(this, text, hexColor, sizePx := 16)
        this._buffer := this._buffer + HtmlShadow(text, hexColor, sizePx);
        return this;
    endfunction

    function Build(this)
        return this._buffer;
    endfunction

    function Clear(this)
        this._buffer := "";
        return this;
    endfunction
endclass
