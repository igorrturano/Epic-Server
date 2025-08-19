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

enum GUMP_TEXT_COLOR
	GUMP_TEXT_COLOR_DEFAULT := 1,

	GUMP_TEXT_COLOR_BLACK := 1,
	GUMP_TEXT_COLOR_RED := 38,
	GUMP_TEXT_COLOR_GREEN := 68,
	GUMP_TEXT_COLOR_BLUE := 90,
	GUMP_TEXT_COLOR_MAUVE := 30,
	GUMP_TEXT_COLOR_LAVENDER := 20,
	GUMP_TEXT_COLOR_BROWN := 46,
	GUMP_TEXT_COLOR_YELLOW := 53,

	GUMP_TEXT_COLOR_ERROR := 38,
	GUMP_TEXT_COLOR_ALERT := 53
endenum