include "Control";

class ControlText(Control)
	var _color;
	var _value;

	function ControlText(this, name, x, y, value, color := 0, gumpId := 0)
		Super(GUMP_CONTROL::TEXT, name, x, y, gumpId);
		
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

	function SelfCreate(this, byref gump)
		GFTextLine(gump, this.GetX(), this.GetY(), this.GetColor(), this.GetValue());
	endfunction

	function ToString(this)
		return "ControlText(name=" + this._GetName()
			+ ", gumpId=" + this.GetGumpId()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", color=" + this._color
			+ ", value=" + this._value
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