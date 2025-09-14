include "Control";

class Text(Control)
	var _color := GUMP_TEXT_COLOR::COLOR_DEFAULT;
	var _value := "";

	function Text(this, name, x, y, value, color := GUMP_TEXT_COLOR::COLOR_DEFAULT)
		Super(GUMP_CONTROL::TEXT, name, x, y);
		
		this._color := color;
		this._value := value;
	endfunction

	function GetColor(this)
		return this._color;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function SetColor(this, color)
		this._color := color;
	endfunction

	function SetValue(this, value)
		this._value := value;
	endfunction

	function SelfCreate(this, byRef gump)
		GFTextLine(gump, this.GetX(), this.GetY(), this.GetColor(), this.GetValue());
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

enum class GUMP_TEXT_COLOR
	COLOR_DEFAULT := 1,

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