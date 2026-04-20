use util;

include ":gumpDesigner:controls/ResizableControl";

include "system/exception/ParameterException/InvalidParameterException";

class Container(ResizableControl)
	var _isVisible := false;
	var _controls := array{}; // Array de Controls

	function Container(this, name, x, y, width, height)
		Super(GumpControl::Container, name, x, y, width, height);
		
		this._isVisible := false;

		this._controls := array{};
	endfunction

	function SetVisible(this, isVisible)
		this._isVisible := isVisible;

		return this;
	endfunction
	
	function GetControls(this)
		return this._controls;
	endfunction

	function AddControl(this, byRef control)
		if (!(control is @Control))
			return InvalidParameterException("Control must be of type 'Control'.");
		endif
		
		control.SetParent(this);
		// control.SetXOffset(this.GetX() + this.GetXOffset());
		// control.SetYOffset(this.GetY() + this.GetYOffset());

		// if ((control.GetX() + control.GetWidth()) > this.GetWidth())
		// 	control.SetWidth(this.GetWidth() - control.GetX()); // Ajusta a largura do controle se ela for maior que o container
		// endif

		// if ((control.GetY() + control.GetHeight()) > this.GetHeight())
		// 	control.SetHeight(this.GetHeight() - control.GetY()); // Ajusta a altura do controle se ela for maior que o container
		// endif

		this._controls.append(control);

		return this;
	endfunction

	function HowToDraw(this, byRef gump)
		if (this._isVisible)
			GFGumpPicTiled(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetWidth(), this.GetHeight(), RandomIntMinMax(0x4657, 0x46CC + 1)); // Desenha um background aleatório para o container, só para visualização
		endif

		foreach control in (this.GetControls())
			control.Draw(gump);
			SleepMS(2);
		endforeach
	endfunction

	function Dispose(this)
		foreach control in (this.GetControls())
			control.Dispose();
			SleepMS(2);
		endforeach
		
		this._controls := Container::_controls;
	endfunction

	function ToString(this)
		return "(Container) " + Super::ToString(this)
			+ ", isVisible=" + CStr(this._isVisible)
			+ ", controls=" + this.GetControls().size();
	endfunction
endclass
