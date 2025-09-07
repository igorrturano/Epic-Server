class Damage()
	var _slash := 0;
	var _piercing := 0;
	var _blunt := 0;
	var _fire := 0;
	var _cold := 0;
	var _poison := 0;
	var _energy := 0;

	function Damage(this, slash := 0, piercing := 0, blunt := 0, fire := 0, cold := 0, poison := 0, energy := 0)
		this._slash := CInt(slash);
		this._piercing := CInt(piercing);
		this._blunt := CInt(blunt);
		this._fire := CInt(fire);
		this._cold := CInt(cold);
		this._poison := CInt(poison);
		this._energy := CInt(energy);
	endfunction

	function GetSlashDamage(this)
		return this._slash;
	endfunction

	function GetPiercingDamage(this)
		return this._piercing;
	endfunction

	function GetBluntDamage(this)
		return this._blunt;
	endfunction

	function GetFireDamage(this)
		return this._fire;
	endfunction

	function GetColdDamage(this)
		return this._cold;
	endfunction

	function GetPoisonDamage(this)
		return this._poison;
	endfunction

	function GetEnergyDamage(this)
		return this._energy;
	endfunction

	function SetSlashDamage(this, value)
		this._slash := CInt(value);
	endfunction

	function SetPiercingDamage(this, value)
		this._piercing := CInt(value);
	endfunction

	function SetBluntDamage(this, value)
		this._blunt := CInt(value);
	endfunction

	function SetFireDamage(this, value)
		this._fire := CInt(value);
	endfunction

	function SetColdDamage(this, value)
		this._cold := CInt(value);
	endfunction

	function SetPoisonDamage(this, value)
		this._poison := CInt(value);
	endfunction

	function SetEnergyDamage(this, value)
		this._energy := CInt(value);
	endfunction

	function ToString(this)
		return $"Damage: Slash={this._slash}, Piercing={this._piercing}, Blunt={this._blunt}, Fire={this._fire}, Cold={this._cold}, Poison={this._poison}, Energy={this._energy}";
	endfunction
endclass