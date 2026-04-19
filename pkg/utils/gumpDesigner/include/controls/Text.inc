include "ResizableControl";

class Text(ResizableControl)
	var _align := TextAlign::Left;
	var _color := TEXT_COLOR::COLOR_DEFAULT;
	var _value := "";

	function Text(this, name, x, y, value, align := TextAlign::Left, color := TEXT_COLOR::COLOR_DEFAULT)
		Super(GumpControl::Text, name, x, y, 0, 0);

		this.SetValue(value);
		this.SetAlign(align);
		this.SetColor(color);
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

	function HowToDraw(this, byRef gump)
		if (!this._align)
			GFTextLine(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetColor(), this.GetValue());
		elseif (this._align == TextAlign::Center)
			GFTextMid(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetColor(), this.GetValue());
		elseif (this._align == TextAlign::Right)
			GFTextRight(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetColor(), this.GetValue());
		endif
		
		// Super::Draw(this, gump);
	endfunction

	function ToString(this)
		return "(Text) " + Super::ToString(this)
			+ ", align=" + this.GetAlign()
			+ ", color=" + this.GetColor()
			+ ", value=" + this.GetValue();
	endfunction
endclass

// TODO: Substituir isso aqui por uso de classe Font()
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


// TODO: Substituir isso aqui por uso de classe Color()
enum class TEXT_COLOR
	COLOR_DEFAULT := COLOR_BLUE,

	COLOR_BLACK := 1,
	COLOR_RED := 38,
	COLOR_GREEN := 68,
	COLOR_BLUE := 90,
	COLOR_MAUVE := 30,
	COLOR_LAVENDER := 20,
	COLOR_BROWN := 46,
	COLOR_YELLOW := 53,

	COLOR_ERROR := COLOR_RED,
	COLOR_ALERT := COLOR_YELLOW
endenum

enum class TextAlign
	Left,
	Center,
	Right
endenum