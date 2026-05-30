include "IShieldAttributes";

class ShieldAttributes(IShieldAttributes)
	var ParryBonus := 0;
	var ParryDelay := 0;

	function ShieldAttributes(this, parryBonus := 0, parryDelay := 0)
		this.ParryBonus := CInt(parryBonus);
		this.ParryDelay := CInt(parryDelay);
	endfunction

	function GetParryBonus(this)
		return this.ParryBonus;
	endfunction

	function GetParryDelay(this)
		return this.ParryDelay;
	endfunction

	function SetParryBonus(this, value)
		this.ParryBonus := CInt(value);
	endfunction

	function SetParryDelay(this, value)
		this.ParryDelay := CInt(value);
	endfunction

	function ToString(this)
		return $"ParryBonus: {this.GetParryBonus()}, " +
			   $"ParryDelay: {this.GetParryDelay()}";
	endfunction
endclass
