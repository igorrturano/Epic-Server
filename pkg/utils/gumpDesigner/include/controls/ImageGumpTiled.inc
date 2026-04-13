include "ImageGump";

class ImageGumpTiled(ImageGump)
	function ImageGumpTiled(this, name, x, y, width, height, imageId, color := 0)
		Super(name, x, y, imageId, color);
		this._type := GumpControl::ImageGumpTiled;
	endfunction

	function Draw(this, byRef gump)
		GFGumpPicTiled(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetImageId());
		Super::Draw(this, gump);
	endfunction

	function ToString(this)
		return "(ImageGumpTiled) " + Super::ToString(this);
	endfunction
endclass