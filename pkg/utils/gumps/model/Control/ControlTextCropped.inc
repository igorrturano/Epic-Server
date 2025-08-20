include "ControlText";

class ControlTextCropped(ControlText)
	var _width;
	var _height;

	function ControlTextCropped(this, name, x, y, width, height, value, color := 0, gumpId := 0)
		Super(name, x, y, value, color, gumpId);
		this._type := GUMP_CONTROL::TEXTCROPPED;
		
		this._width := width;
		this._height := height;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function GetHeight(this)
		return this._height;
	endfunction

	function SetWidth(this, width)
		this._width := width;
	endfunction

	function SetHeight(this, height)
		this._height := height;
	endfunction

	function SelfCreate(this, byRef gump)
		GFTextCrop(gump, this.GetX(), this.GetY(), this.GetWidth(), this.GetHeight(), this.GetColor(), this.GetValue());
	endfunction

	function ToString(this)
		return "ControlText(name=" + this._GetName()
			+ ", gumpId=" + this.GetGumpId()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", color=" + this._color
			+ ", value=" + this._value
			+ ")";
	endfunction
endclass