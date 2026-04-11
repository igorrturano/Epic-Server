include "ResizableControl";

class ImageGump(ResizableControl)
	var _imageId := 0x0;
	var _color := 0;

	function ImageGump(this, name, x, y, imageId, width := 0, height := 0, color := 0)
		Super(GumpControl::ImageGump, name, x, y, width, height);
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
		return this;
	endfunction

	function SetColor(this, color)
		this._color := color;
		return this;
	endfunction

	function Resize(this, width, height)
		this._width := width;
		this._height := height;
		return this;
	endfunction

	function Draw(this, byRef gump)
		if (this._width == 0 || this._height == 0)
			GFGumpPic(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetImageId(), this.GetColor());
		else
			GFResizePic(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetImageId(), this.GetWidth(), this.GetHeight());
		endif

		Super::Draw(this, gump);
	endfunction

	function ToString(this)
		return "(ImageGump) " + Super::ToString(this)
			+ ", imageId=" + this.GetImageId()
			+ ", color=" + this.GetColor();
	endfunction
endclass