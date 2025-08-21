include "Control";

class ImageTiled(Control)
	var _imageId;
	var _width;
	var _height;

	function ImageTiled(this, name, x, y, width, height, imageId, gumpId := 0)
		Super(GUMP_CONTROL::IMAGETILED, name, x, y, gumpId);
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
	endfunction

	function SetWidth(this, width)
		this._width := width;
	endfunction

	function SetHeight(this, height)
		this._height := height;
	endfunction

	function SelfCreate(this, byRef gump)
		GFPicTiled(gump, this.GetX(), this.GetY(), this.GetWidth(), this.GetHeight(), this.GetImageId());
	endfunction

	function ToString(this)
		return "ImageTiled(name=" + this._GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", imageId=" + this.GetImageId()
			+ ", gumpId=" + this.GetGumpId()
			+ ")";
	endfunction
endclass