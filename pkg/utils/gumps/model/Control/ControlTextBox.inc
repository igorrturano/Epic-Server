include "Control";

class ControlTextBox(Control)
	var _width;
	var _height;
	var _textColor;
	var _value;
	var _maxLength;

	function ControlTextBox(this, name, x, y, width, height := 0, textColor := 0, value := 0, maxLength := 0, gumpId := 0)
		Super(GUMP_CONTROL::TEXTBOX, name, x, y, gumpId);
		this._width := width;
		this._height := height;
		this._textColor := textColor;
		this._value := value;
		this._maxLength := maxLength;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function GetHeight(this)
		return this._height;
	endfunction

	function GetTextColor(this)
		return this._textColor;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function GetMaxLength(this)
		return this._maxLength;
	endfunction

	function SetWidth(this, width)
		this._width := width;
	endfunction

	function SetHeight(this, height)
		this._height := height;
	endfunction

	function SetTextColor(this, textColor)
		this._textColor := textColor;
	endfunction

	function SetValue(this, value)
		this._value := value;
	endfunction

	function SetMaxLength(this, maxLength)
		this._maxLength := maxLength;
	endfunction

	function SelfCreate(this, byRef gump)
		GFEAddTextBox(gump, this.GetX(), this.GetY()-6, this.GetWidth(), this.GetHeight(), this.GetTextColor(), this.GetValue(), this.GetGumpId(), this.GetMaxLength());
	endfunction

	function ToString(this)
		return "ControlTextBox(name=" + this._GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", textColor=" + this.GetTextColor()
			+ ", value=" + this.GetValue()
			+ ", maxLength=" + this.GetMaxLength()
			+ ", gumpId=" + this.GetGumpId()
			+ ")";
	endfunction
endclass