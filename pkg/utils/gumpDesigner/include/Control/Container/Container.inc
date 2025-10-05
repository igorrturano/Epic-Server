use util;

include ":gumpDesigner:Control/Control";

class Container(Control)
	var _name := "";

	var _x := 0;
	var _y := 0;
	var _width := 0;
	var _height := 0;

	var _isVisible := false;

	var _controls := array{}; // Array de Controls

	function Container(this, name, x, y, width, height)
		Super(GUMP_CONTROL::CONTAINER, name, x, y);
		
		this._name := name;
		this._x := x;
		this._y := y;
		this._width := width;
		this._height := height;

		this._isVisible := false;

		this._controls := array{};
	endfunction

	function GetName(this)
		return this._name;
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

	function _SetName(this, name)
		this._name := name;
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

	function SetVisible(this, isVisible)
		this._isVisible := isVisible;
	endfunction
	
	function AddControl(this, byRef control)
		control.SetX(control.GetX() + this.GetX());
		control.SetY(control.GetY() + this.GetY());
		if (control.GetWidth() > this.GetWidth()) control.SetWidth(this.GetWidth()); endif // Ajusta a largura do controle se ela for maior que o container
		if (control.GetHeight() > this.GetHeight()) control.SetHeight(this.GetHeight()); endif // Ajusta a altura do controle se ela for maior que o container
		this._controls.append(control);
	endfunction

	function SelfCreate(this, byRef gump)
		if (this._isVisible)
			var bg := array{0x5B8, 0xBBC, 0xE14, 0x13C2, 0x1414, 0x1436};
			GFGumpPicTiled(gump, this.GetX(), this.GetY(), this.GetWidth(), this.GetHeight(), bg[RandomInt(bg.size() + 1)]); // Desenha um background aleatório para o container, só para visualização
		endif

		foreach control in (this._controls)
			control.SelfCreate(gump);
		endforeach
	endfunction

	function ToString(this)
		return "Container (name=" + this.GetName()
			+ ", x=" + CStr(this.GetX())
			+ ", y=" + CStr(this.GetY())
			+ ", width=" + CStr(this.GetWidth())
			+ ", height=" + CStr(this.GetHeight())
			+ ", controlsCount=" + CStr(this.GetControls().size())
			+ ", isVisible=" + CStr(this._isVisible)
			+ ")";
	endfunction
endclass