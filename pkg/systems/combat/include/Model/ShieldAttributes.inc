include "IShieldAttributes";

class ShieldAttributes(IShieldAttributes)
	var _parryBonus := 0;
	var _parryDelay := 0;

	function ShieldAttributes(this, parryBonus := 0, parryDelay := 0)
		this._parryBonus := CInt(parryBonus);
		this._parryDelay := CInt(parryDelay);
	endfunction

	function GetParryBonus(this)
		return this._parryBonus;
	endfunction

	function GetParryDelay(this)
		return this._parryDelay;
	endfunction

	function SetParryBonus(this, value)
		this._parryBonus := CInt(value);
	endfunction

	function SetParryDelay(this, value)
		this._parryDelay := CInt(value);
	endfunction

	function ToString(this)
		return $"ParryBonus: {this._parryBonus}, " +
			   $"ParryDelay: {this._parryDelay}";
	endfunction
endclass
