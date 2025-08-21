include "ImageGump";

class ControlImageGumpTiled(ControlImageGump)
	var _width;
	var _height;

	function ControlImageGumpTiled(this, name, x, y, width, height, imageId, color := 0, gumpId := 0)
		Super(name, x, y, imageId, color, gumpId);
		this._type := GUMP_CONTROL::IMAGEGUMPTILED;
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
		GFGumpPicTiled(gump, this.GetX(), this.GetY(), this.GetWidth(), this.GetHeight(), this.GetImageId());
	endfunction

	function ToString(this)
		return "ControlImageGumpTiled(name=" + this._GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", imageId=" + this.GetImageId()
			+ ", color=" + this.GetColor()
			+ ", gumpId=" + this.GetGumpId()
			+ ")";
	endfunction
endclass