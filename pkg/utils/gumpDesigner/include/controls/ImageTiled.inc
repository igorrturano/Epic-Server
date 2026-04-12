include "ResizableControl";

class ImageTiled(ResizableControl)
	var _imageId := 0x0;

	function ImageTiled(this, name, x, y, width, height, imageId)
		Super(GumpControl::ImageTiled, name, x, y, width, height);
		this._imageId := imageId;
	endfunction

	function GetImageId(this)
		return this._imageId;
	endfunction

	function SetImageId(this, imageId)
		this._imageId := imageId;
		return this;
	endfunction

	function Draw(this, byRef gump)
		GFPicTiled(gump.GetGump(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetImageId());
		Super::Draw(this, gump);
	endfunction

	function ToString(this)
		return "(ImageTiled) " + Super::ToString(this)
			+ ", imageId=" + this.GetImageId();
	endfunction
endclass