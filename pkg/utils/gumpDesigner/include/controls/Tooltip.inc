include "Control";

class Tooltip(Control)
	var _text := "";

	function Tooltip(this, name, text)
		Super(GumpControl::Tooltip, name, 0, 0);
		this._text := text;
	endfunction

	function GetText(this)
		return this._text;
	endfunction

	function SetText(this, text)
		this._text := text;
		return this;
	endfunction

	function HowToDraw(this, byRef gump)
		GFTooltipText(gump.GetGumpString(), this.GetText());
	endfunction

	function ToString(this)
		return "(Tooltip) " + Super::ToString(this)
			+ ", text=" + this.GetText();
	endfunction
endclass