include "Control";

class ControlImageGump(Control)
	var _imageId;
	var _color;

	function ControlImageGump(this, name, x, y, imageId, color := 0, gumpId := 0)
		Super(GUMP_CONTROL::IMAGEGUMP, name, x, y, gumpId);
		this._imageId := imageId;
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

	function SelfCreate(this, byRef gump)
		GFGumpPic(gump, this.GetX(), this.GetY(), this.GetImageId(), this.GetColor());
	endfunction

	function ToString(this)
		return "ControlImageGump(name=" + this._GetName()
			+ ", gumpId=" + this.GetGumpId()
			+ ", imageId=" + this.GetImageId()
			+ ", color=" + this.GetColor()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ")";
	endfunction
endclass