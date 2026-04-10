include "ImageGump";

class ImageGumpTiled(ImageGump)
	function ImageGumpTiled(this, name, x, y, width, height, imageId, color := 0)
		Super(name, x, y, imageId, color);
		this._type := GUMP_CONTROL::IMAGEGUMPTILED;
	endfunction

	function SelfCreate(this, byRef gump)
		GFGumpPicTiled(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetImageId());
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "ImageGump(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", imageId=" + this.GetImageId()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", color=" + this.GetColor()
			+ ")";
	endfunction
endclass