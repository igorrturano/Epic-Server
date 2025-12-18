include "ImageGump";

class ImageGumpTiled(ImageGump)
	var _width := 0;
	var _height := 0;

	function ImageGumpTiled(this, name, x, y, width, height, imageId, color := 0)
		Super(name, x, y, imageId, color);
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
		return this;
	endfunction

	function SetHeight(this, height)
		this._height := height;
		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		GFGumpPicTiled(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetImageId());
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "ImageGump(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", imageId=" + this.GetImageId()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", color=" + this.GetColor()
			+ ")";
	endfunction
endclass