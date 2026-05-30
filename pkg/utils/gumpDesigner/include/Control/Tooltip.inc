include "Control";

class Tooltip(Control)
	var _text := "";

	function Tooltip(this, name, text)
		Super(GUMP_CONTROL::TOOLTIP, name, 0, 0);
		this._text := text;
	endfunction

	function GetText(this)
		return this._text;
	endfunction

	function SetText(this, text)
		this._text := text;
		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		GFTooltipText(gump, this.GetText());
	endfunction

	function ToString(this)
		return "Tooltip(type=" + this.GetType()
			+ ", name=" + this.GetName()
			+ ", text=" + this.GetText()
			+ ")";
	endfunction
endclass