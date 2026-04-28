include "Control";

/*
 * HtmlText — Controle de área HTML para o gumpDesigner.
 *
 * Renderiza HTML formatado dentro de um GFHTMLArea (gumps clássicos UO).
 * Para HTML/CSS completo via WebView2, use webPanel.inc diretamente.
 *
 * Tags suportadas pelo cliente UO:
 *   <b>, <i>, <u>, <a>, <p>, <big>, <small>, <body>, <basefont>,
 *   <h1>-<h6>, <br>, <bq>, <left>, <center>, <right>, <div>
 *
 * Uso:
 *   var htmlCtrl := HtmlText("descricao", 20, 60, 300, 150,
 *                            HtmlBold("Espada Longa") + HtmlBr() + "Dano: 20-35");
 *   gump.AddControl(htmlCtrl);
 */

class HtmlText(Control)
    var _width       := 200;
    var _height      := 100;
    var _htmlContent := "";
    var _scrollbar   := 0;
    var _background  := 0;

    function HtmlText(this, name, x, y, width, height, htmlContent, scrollbar := 0, background := 0)
        Super(GUMP_CONTROL::HTMLTEXT, name, x, y);
        this._width       := CInt(width);
        this._height      := CInt(height);
        this._htmlContent := CStr(htmlContent);
        this._scrollbar   := scrollbar ? 1 : 0;
        this._background  := background ? 1 : 0;
    endfunction

    function GetWidth(this)
        return this._width;
    endfunction

    function GetHeight(this)
        return this._height;
    endfunction

    function GetHtmlContent(this)
        return this._htmlContent;
    endfunction

    function GetScrollbar(this)
        return this._scrollbar;
    endfunction

    function GetBackground(this)
        return this._background;
    endfunction

    function SetWidth(this, width)
        this._width := CInt(width);
        return this;
    endfunction

    function SetHeight(this, height)
        this._height := CInt(height);
        return this;
    endfunction

    function SetContent(this, htmlContent)
        this._htmlContent := CStr(htmlContent);
        return this;
    endfunction

    function SetScrollbar(this, scrollbar)
        this._scrollbar := scrollbar ? 1 : 0;
        return this;
    endfunction

    function SetBackground(this, background)
        this._background := background ? 1 : 0;
        return this;
    endfunction

    function SelfCreate(this, byRef gump)
        GFHTMLArea(
            gump,
            this.GetX() + this.GetXOffset(),
            this.GetY() + this.GetYOffset(),
            this._width,
            this._height,
            this._htmlContent,
            this._background,
            this._scrollbar
        );
        Super::SelfCreate(this, gump);
    endfunction

    function ToString(this)
        return "HtmlText(gumpId=" + this.GetGumpId()
            + ", name=" + this.GetName()
            + ", x=" + this.GetX()
            + ", y=" + this.GetY()
            + ", w=" + this._width
            + ", h=" + this._height
            + ", scrollbar=" + this._scrollbar
            + ")";
    endfunction
endclass
