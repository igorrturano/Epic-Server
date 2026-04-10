include "ResizableControl";

class ImageTiled(ResizableControl)
	var _imageId := 0x0;

	function ImageTiled(this, name, x, y, width, height, imageId)
		Super(GUMP_CONTROL::IMAGETILED, name, x, y, width, height);
		this._imageId := imageId;
	endfunction

	function GetImageId(this)
		return this._imageId;
	endfunction

	function SetImageId(this, imageId)
		this._imageId := imageId;
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