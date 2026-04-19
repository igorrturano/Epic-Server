include "Control";

class ImageTile(Control)
	var _imageId := 0x0;
	var _color := 0;

	function ImageTile(this, name, x, y, imageId, color := 0)
		Super(GumpControl::ImageTile, name, x, y);
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

	function Draw(this, byRef gump)
		GFTilePic(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetImageId(), this.GetColor());

		// Super::Draw(this, gump);
	endfunction

	function ToString(this)
		return "(ImageTile) " + Super::ToString(this)
			+ ", imageId=" + this.GetImageId()
			+ ", color=" + this.GetColor();
	endfunction
endclass