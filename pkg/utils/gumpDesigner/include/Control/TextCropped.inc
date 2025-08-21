include "Text";

class TextCropped(Text)
	var _width;
	var _height;

	function TextCropped(this, name, x, y, width, height, value, color := 0)
		Super(name, x, y, value, color);
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
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "TextCropped(gumpId=" + this.GetGumpId()
			+ ", name=" + this._GetName()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", color=" + this.GetColor()
			+ ", value=" + this.GetValue()
			+ ")";
	endfunction
endclass