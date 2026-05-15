include "Control";

class Text(Control)
	var _width := 0;
	var _align := TEXT_ALIGN::ALIGN_LEFT;
	var _color := TEXT_COLOR::COLOR_DEFAULT;
	var _value := "";

	function Text(this, name, x, y, value, color := TEXT_COLOR::COLOR_DEFAULT, width := 0, align := TEXT_ALIGN::ALIGN_LEFT)
		Super(GUMP_CONTROL::TEXT, name, x, y);

		this._width := width;
		this._align := align;
		this._color := color;
		this._value := value;
	endfunction

	function GetWidth(this)
		return this._width;	
	endfunction

	function GetAlign(this)
		return this._align;
	endfunction
	
	function GetColor(this)
		return this._color;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function SetWidth(this, width)
		this._width := width;
		return this;
	endfunction

	function SetAlign(this, align)
		this._align := align;
		return this;
	endfunction

	function SetColor(this, color)
		this._color := color;
		return this;
	endfunction

	function SetValue(this, value)
		this._value := value;
		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		if (!this._align)
			GFTextLine(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetColor(), this.GetValue());
		elseif (this._align == TEXT_ALIGN::ALIGN_CENTER)
			GFTextMid(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetColor(), this.GetValue());
		elseif (this._align == TEXT_ALIGN::ALIGN_RIGHT)
			GFTextRight(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetColor(), this.GetValue());
		endif
		
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "Text(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", color=" + this.GetColor()
			+ ", value=" + this.GetValue()
			+ ")";
	endfunction
endclass

enum class TEXT_FONT
	FONT_UO := 0,

	FONT_ARIAL := 2,
	FONT_ARIAL_BOLD := 1,
	
	FONT_FIXEDSYS := 3,
	FONT_FIXEDSYS_BIG := 4,

	FONT_THIN := 5,
	FONT_THIN_BIG := 6,
	
	FONT_SYMBOLS := 7,
	FONT_SYMBOLS_BOLD := 8,
	FONT_SYMBOLS_BOLD_BIG := 9,

	FONT_RUNE := 10,
	FONT_RUNE_BOLD := 11,
	FONT_RUNE_BOLD_BIG := 12,

	FONT_DEFAULT := FONT_UO
endenum

enum class TEXT_COLOR
	COLOR_BLACK := 1,
	COLOR_RED := 38,
	COLOR_GREEN := 68,
	COLOR_BLUE := 90,
	COLOR_MAUVE := 30,
	COLOR_LAVENDER := 20,
	COLOR_BROWN := 46,
	COLOR_YELLOW := 53,

	COLOR_DEFAULT := 90,
	COLOR_ERROR := 38,
	COLOR_ALERT := 53
endenum

enum class TEXT_ALIGN
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
endenum