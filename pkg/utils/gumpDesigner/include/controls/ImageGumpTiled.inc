include "ImageGump";

class ImageGumpTiled(ImageGump)
	function ImageGumpTiled(this, name, x, y, imageId, width, height, color := 0)
		Super(name, x, y, imageId, width, height, color);
	endfunction

	function HowToDraw(this, byRef gump)
		GFPicTiled(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetImageId());
		// ResizableControl::Draw(this, gump);
	endfunction

	function ToString(this)
		return "(ImageGumpTiled) " + Super::ToString(this);
	endfunction
endclass