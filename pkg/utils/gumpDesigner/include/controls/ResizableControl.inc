include "Control";

class ResizableControl(Control)
	var _width := 0;
	var _height := 0;

	function ResizableControl(this, type, name, x, y, width, height)
		Super(type, name, x, y);
		this._width := width;
		this._height := height;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function GetHeight(this)
		return this._height;
	endfunction

	function SetWidth(this, width)
		this._width := width;
		return this;
	endfunction

	function SetHeight(this, height)
		this._height := height;
		return this;
	endfunction

	function ToString(this)
		return "(ResizableControl) " + Super::ToString(this)
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight();
	endfunction
endclass