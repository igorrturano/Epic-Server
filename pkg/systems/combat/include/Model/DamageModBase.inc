include "IModificator";

class DamageModBase(IModificator)
	function DamageModBase(this, source := "", damage := 0)
		this._damage := this.MapDamage(damage);
		this._source := source;
	endfunction

	uninit function MapDamage(this, damage);

	function GetSource(this)
		return this._source;
	endfunction

	function GetMod(this)
		return this._damage;
	endfunction

	function SetMod(this, damage)
		this._damage := damage;
	endfunction
endclass