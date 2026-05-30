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
		return this;
	endfunction

	function SetX(this, x)
		this._x := x;
		return this;
	endfunction

	function SetY(this, y)
		this._y := y;
		return this;
	endfunction

	function SetWidth(this, width)
		this._width := width;
		return this;
	endfunction

	function SetHeight(this, height)
		this._height := height;
		return this;
	endfunction

	function SetVisible(this, isVisible)
		this._isVisible := isVisible;
		return this;
	endfunction
	
	function AddControl(this, byRef control)
		control.SetXOffset(this.GetX());
		control.SetYOffset(this.GetY());
		if (control.GetWidth() > this.GetWidth()) control.SetWidth(this.GetWidth()); endif // Ajusta a largura do controle se ela for maior que o container
		if (control.GetHeight() > this.GetHeight()) control.SetHeight(this.GetHeight()); endif // Ajusta a altura do controle se ela for maior que o container
		this._controls.append(control);
		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		if (this._isVisible)
			var bg := array{
				0x5B8, 0xBBC, 0xE14, 0x13C2, 0x1404, 0x1436, 0x9DC, 0x9E6,
				0xA2C, 0xA40, 0xA8E, 0xDB0, 0x24F4, 0x4654, 0x4655, 0x4656,
				0x4657, 0x4658, 0x4659, 0x465A, 0x465B, 0x465C, 0x465D, 0x465E,
				0x465F, 0x4660, 0x4661, 0x4662, 0x4663, 0x4664, 0x4665, 0x4666,
				0x4667, 0x4668, 0x4669, 0x466A, 0x466B, 0x466C, 0x466D, 0x466E,
				0x466F, 0x4670, 0x4671, 0x4672, 0x4673, 0x4674, 0x4675, 0x4676,
				0x4677, 0x4678, 0x4679, 0x467A, 0x467B, 0x467C, 0x467D, 0x467E,
				0x467F, 0x4680, 0x4681, 0x4682, 0x4683, 0x4684, 0x4685, 0x4686,
				0x4687, 0x4688, 0x4689, 0x468A, 0x468B, 0x468C, 0x468D, 0x468E,
				0x468F, 0x4690, 0x4691, 0x4692, 0x4693, 0x4694, 0x4695, 0x4696,
				0x4697, 0x4698, 0x4699, 0x469A, 0x469B, 0x469C, 0x469D, 0x469E,
				0x469F, 0x46A0, 0x46A1, 0x46A2, 0x46A3, 0x46A4, 0x46A5, 0x46A6,
				0x46A7, 0x46A8, 0x46A9, 0x46AA, 0x46AB, 0x46AC, 0x46AD, 0x46AE,
				0x46AF, 0x46B0, 0x46B1, 0x46B2, 0x46B3, 0x46B4, 0x46B5, 0x46B6,
				0x46B7, 0x46B8, 0x46B9, 0x46BA, 0x46BB, 0x46BC, 0x46BD, 0x46BE,
				0x46BF, 0x46C0, 0x46C1, 0x46C2, 0x46C3, 0x46C4, 0x46C5, 0x46C6,
				0x46C7, 0x46C8, 0x46C9, 0x46CA, 0x46CB, 0x46CC
			};
			GFGumpPicTiled(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), bg[RandomInt(bg.size()) + 1]); // Desenha um background aleatório para o container, só para visualização
		endif
	endfunction

	function Dispose(this)
		foreach control in (this._controls)
			control.Dispose();
		endforeach
		this._controls := array{};
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
