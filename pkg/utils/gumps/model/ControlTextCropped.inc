include "ControlText";

class ControlTextCropped(ControlText)
	var _width;
	var _height;

	function ControlTextCropped(this, name, x, y, width, height, value, color := 0, gumpId := 0)
		Super(name, x, y, value, color, gumpId);
		this._type := GUMP_CONTROL_TEXTCROPPED;
		
		this._width := width;
		this._height := height;
	endfunction
endclass