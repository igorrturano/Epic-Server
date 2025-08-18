include "Control";

class ControlText(Control)
	var _color;
	var _value;

	function ControlText(this, name, x, y, value, color := 0, gumpId := 0)
		Super(GUMP_CONTROL_TEXT, name, x, y, gumpId);
		
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