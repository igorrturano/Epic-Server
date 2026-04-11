use util;

include ":gumpDesigner:controls/ResizableControl";

class Container(ResizableControl)
	var _isVisible := false;
	var _controls := array{}; // Array de Controls

	function Container(this, name, x, y, width, height)
		Super(GumpControl::Container, name, x, y, width, height);
		
		this._isVisible := false;

		this._controls := array{};
	endfunction

	function GetControls(this)
		return this._controls;
	endfunction

	function SetVisible(this, isVisible)
		this._isVisible := isVisible;

		return this;
	endfunction
	
	function AddControl(this, byRef control)
		control.SetXOffset(this.GetX());
		control.SetYOffset(this.GetY());

		if (control.GetWidth() > this.GetWidth())
			control.SetWidth(this.GetWidth()); // Ajusta a largura do controle se ela for maior que o container
		endif

		if (control.GetHeight() > this.GetHeight())
			control.SetHeight(this.GetHeight()); // Ajusta a altura do controle se ela for maior que o container
		endif

		this._controls.append(control);

		return this;
	endfunction

	function Draw(this, byRef gump)
		if (this._isVisible)
			GFGumpPicTiled(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), RandomIntMinMax(0x4657, 0x46CC + 1)); // Desenha um background aleatório para o container, só para visualização
		endif
	endfunction

	function Dispose(this)
		foreach control in (this._controls)
			control.Dispose();
		endforeach
		
		this._controls := array{};
	endfunction

	function ToString(this)
		return "(Container) " + Super::ToString(this)
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", isVisible=" + CStr(this._isVisible)
			+ ", controls=" + this.GetControls().size();
	endfunction
endclass
