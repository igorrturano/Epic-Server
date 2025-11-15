include "Control";

class ImageTiled(Control)
	var _imageId := 0x0;
	var _width := 0;
	var _height := 0;

	function ImageTiled(this, name, x, y, width, height, imageId)
		Super(GUMP_CONTROL::IMAGETILED, name, x, y);
		this._imageId := imageId;
		this._width := width;
		this._height := height;
	endfunction

	function GetImageId(this)
		return this._imageId;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function GetHeight(this)
		return this._height;
	endfunction

	function SetImageId(this, imageId)
		this._imageId := imageId;
		return this;
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
		GFPicTiled(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetImageId());
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "ImageTiled(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", imageId=" + this.GetImageId()
			+ ")";
	endfunction
endclass