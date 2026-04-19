include "Text";

class TextCropped(Text)
	function TextCropped(this, name, x, y, width, height, value, align := TEXT_ALIGN::ALIGN_LEFT, color := TEXT_COLOR::COLOR_DEFAULT)
		Super(name, x, y, value, align, color);
		this._type := GumpControl::TextCropped;

		this.SetWidth(width);
		this.SetHeight(height);
	endfunction

	function HowToDraw(this, byRef gump)
		GFTextCrop(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetColor(), this.GetValue());
		// Super::Draw(this, gump);
	endfunction

	function ToString(this)
		return "(TextCropped) " + Super::ToString(this)
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight();
	endfunction
endclass