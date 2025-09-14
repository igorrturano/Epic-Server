include ":gumpDesigner:Control/Control";

class Container(Control)
	var _parent; // Gump

	var _x;
	var _y;
	var _width;
	var _height;

	var _controls;

	function Container(this, parent, x, y, width, height)
		Super(GUMP_CONTROL::CONTAINER, "Container", x, y);
		
		this._parent := parent;
		this._x := x;
		this._y := y;
		this._width := width;
		this._height := height;

		this._controls := array{};
	endfunction

	function GetParent(this)
		return this._parent;
	endfunction
	
	function GetX(this)
		return this._x;
	endfunction

	function GetY(this)
		return this._y;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function GetHeight(this)
		return this._height;
	endfunction

	function GetControls(this)
		return this._controls;
	endfunction

	function SetParent(this, parent)
		this._parent := parent;
	endfunction

	function SetX(this, x)
		this._x := x;
	endfunction

	function SetY(this, y)
		this._y := y;
	endfunction

	function SetWidth(this, width)
		this._width := width;
	endfunction

	function SetHeight(this, height)
		this._height := height;
	endfunction
	
	function AddControl(this, byRef control)
		this._controls.append(control);
	endfunction

	function SelfCreate(this, byRef gump)
		print("Creating controls in Container...");
		foreach control in (this._controls)
			print("Creating control in Container: " + control.ToString());
			control.SetX(control.GetX() + this._x);
			control.SetY(control.GetY() + this._y);
			control.SelfCreate(gump);
		endforeach
	endfunction

	function ToString(this)
	return "Container (parent=" + this.GetParent().GetName()
			+ ", x=" + CStr(this.GetX())
			+ ", y=" + CStr(this.GetY())
			+ ", width=" + CStr(this.GetWidth())
			+ ", height=" + CStr(this.GetHeight())
			+ ", controlsCount=" + CStr(this.GetControls().count());
			+ ")";
	endfunction
endclass