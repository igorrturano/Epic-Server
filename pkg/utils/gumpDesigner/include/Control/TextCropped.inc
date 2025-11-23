include "Text";

class TextCropped(Text)
	var _width := 0;
	var _height := 0;

	function TextCropped(this, name, x, y, width, height, value, align := TEXT_ALIGN::ALIGN_LEFT, color := TEXT_COLOR::COLOR_DEFAULT)
		Super(name, x, y, value, align, color);
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
		return this;
	endfunction

	function SetHeight(this, height)
		this._height := height;
		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		GFTextCrop(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetColor(), this.GetValue());
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "TextCropped(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
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