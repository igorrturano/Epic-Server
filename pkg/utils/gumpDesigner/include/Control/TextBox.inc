include "Control";

class TextBox(Control)
	var _width := 0;
	var _lines := 0;
	var _textColor := 0;
	var _value := "";
	var _maxLength := 0;

	function TextBox(this, name, x, y, width, lines := 1, textColor := 0, value := 0, maxLength := 0)
		Super(GUMP_CONTROL::TEXTBOX, name, x, y);
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
		return this;
	endfunction

	function SetLines(this, lines)
		this._lines := lines;
		return this;
	endfunction

	function SetTextColor(this, textColor)
		this._textColor := textColor;
		return this;
	endfunction

	function SetValue(this, value)
		this._value := value;
		return this;
	endfunction

	function SetMaxLength(this, maxLength)
		this._maxLength := maxLength;
		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		this._value := GFEAddTextBox(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset() - 6, this.GetWidth(), this.GetLines()*16, this.GetTextColor(), this.GetValue(), this.GetGumpId(), this.GetMaxLength());
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "TextBox(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", width=" + this.GetWidth()
			+ ", lines=" + this.GetLines()
			+ ", textColor=" + this.GetTextColor()
			+ ", value=" + this.GetValue()
			+ ", maxLength=" + this.GetMaxLength()
			+ ")";
	endfunction
endclass