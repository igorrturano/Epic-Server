class Container()
	var _parent;

	var _x;
	var _y;
	var _width;
	var _height;

	var _controls;

	function Container(this, parent, x, y, width, height)
		this._parent := parent;
		this._x := x;
		this._y := y;
		this._width := width;
		this._height := height;

		this._controls := array{};
	endfunction
endclass