include "Control";

class ControlImageGump(Control)
	var _imageId;
	var _width;
	var _height;
	var _color;
	var _isResizable;

	function ControlImageGump(this, name, x, y, imageId, width := 0, height := 0, color := 0, gumpId := 0)
		Super(GUMP_CONTROL::IMAGEGUMP, name, x, y, gumpId);
		this._imageId := imageId;
		this._width := width;
		this._height := height;
		this._color := color;
	endfunction

	function GetImageId(this)
		return this._imageId;
	endfunction

	function GetColor(this)
		return this._color;
	endfunction

	function SetImageId(this, imageId)
		this._imageId := imageId;
	endfunction

	function SetColor(this, color)
		this._color := color;
	endfunction

	function Resize(this, width, height)
		this._width := width;
		this._height := height;
	endfunction

	function SelfCreate(this, byRef gump)
		if (this._width == 0 || this._height == 0)
			GFGumpPic(gump, this.GetX(), this.GetY(), this.GetImageId(), this.GetColor());
		else
			GFResizePic(gump, this.GetX(), this.GetY(), this.GetImageId(), this.GetWidth(), this.GetHeight());
		endif
	endfunction

	function ToString(this)
		return "ControlImageGump(name=" + this._GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", imageId=" + this.GetImageId()
			+ ", color=" + this.GetColor()
			+ ", gumpId=" + this.GetGumpId()
			+ ")";
	endfunction
endclass