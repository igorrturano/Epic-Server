include "ResizableControl";

class ImageGump(ResizableControl)
	var _imageId := 0x0;
	var _color := 0;

	function ImageGump(this, name, x, y, imageId, width := 0, height := 0, color := 0)
		Super(GumpControl::ImageGump, name, x, y, width, height);
		this.SetImageId(imageId);
		this.SetColor(color);
	endfunction

	function GetImageId(this)
		return this._imageId;
	endfunction

	function SetImageId(this, imageId)
		this._imageId := imageId;

		return this;
	endfunction

	function GetColor(this)
		return this._color;
	endfunction

	function SetColor(this, color)
		this._color := color;

		return this;
	endfunction

	function HowToDraw(this, byRef gump)
		if (this.GetWidth() > 0 || this.GetHeight() > 0)
			GFResizePic(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetImageId(), this.GetWidth(), this.GetHeight());
		else
			GFGumpPic(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetImageId(), this.GetColor());
		endif

		// Super::Draw(this, gump);
	endfunction

	function ToString(this)
		return "(ImageGump) " + Super::ToString(this)
			+ ", imageId=" + this.GetImageId()
			+ ", color=" + this.GetColor();
	endfunction
endclass