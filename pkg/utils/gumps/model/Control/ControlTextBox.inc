include "Control";

class ControlTextBox(Control)
	var _width;
	var _lines;
	var _textColor;
	var _value;
	var _maxLength;

	function ControlTextBox(this, name, x, y, width, lines := 1, textColor := 0, value := 0, maxLength := 0, gumpId := 0)
		Super(GUMP_CONTROL::TEXTBOX, name, x, y, gumpId);
		this._width := width;
		this._lines := lines < 1 ? 1 : lines;
		this._textColor := textColor;
		this._value := value;
		this._maxLength := maxLength;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function GetLines(this)
		return this._lines;
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

	function SetLines(this, lines)
		this._lines := lines;
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
		GFEAddTextBox(gump, this.GetX(), this.GetY()-6, this.GetWidth(), this.GetLines()*16, this.GetTextColor(), this.GetValue(), this.GetGumpId(), this.GetMaxLength());
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