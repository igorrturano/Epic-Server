include "Text";

class TextCropped(Text)
	function TextCropped(this, name, x, y, width, height, value)
		Super(name, x, y, value);
		this._type := GumpControl::TextCropped;

		this.SetWidth(width);
		this.SetHeight(height);
	endfunction

	function SelfCreate(this, byRef gump)
		GFTextCrop(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), this.GetColor(), this.GetValue());
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "(TextCropped) " + Super::ToString(this)
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight();
	endfunction
endclass