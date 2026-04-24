include "IModificator";

class ResistModBase(IModificator)
	function ResistModBase(this, source := "", armorResist := 0)
		this._armorResist := this.MapResist(armorResist);
		this._source := source;
	endfunction

	uninit function MapResist(this, armorResist);

	function GetSource(this)
		return this._source;
	endfunction

	function GetMod(this)
		return this._armorResist;
	endfunction

	function SetMod(this, armorResist)
		this._armorResist := armorResist;
	endfunction
endclass