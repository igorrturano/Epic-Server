include "IWeaponDamage";

class WeaponDamage(IWeaponDamage)
	var _slash := ""; // Dice string format, e.g., "1d6+2"
	var _piercing := ""; // Dice string format, e.g., "1d6+2"
	var _blunt := ""; // Dice string format, e.g., "1d6+2"
	var _fire := ""; // Dice string format, e.g., "1d6+2"
	var _cold := ""; // Dice string format, e.g., "1d6+2"
	var _poison := ""; // Dice string format, e.g., "1d6+2"
	var _energy := ""; // Dice string format, e.g., "1d6+2"

	function WeaponDamage(this, slash := "", piercing := "", blunt := "", fire := "", cold := "", poison := "", energy := "")
		this._slash := slash;
		this._piercing := piercing;
		this._blunt := blunt;
		this._fire := fire;
		this._cold := cold;
		this._poison := poison;
		this._energy := energy;
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
		this._slash := value;
	endfunction

	function SetPiercingDamage(this, value)
		this._piercing := value;
	endfunction

	function SetBluntDamage(this, value)
		this._blunt := value;
	endfunction

	function SetFireDamage(this, value)
		this._fire := value;
	endfunction

	function SetColdDamage(this, value)
		this._cold := value;
	endfunction

	function SetPoisonDamage(this, value)
		this._poison := value;
	endfunction

	function SetEnergyDamage(this, value)
		this._energy := value;
	endfunction

	function ToString(this)
		return $"Weapon Damage: Slash={this._slash}, Piercing={this._piercing}, Blunt={this._blunt}, Fire={this._fire}, Cold={this._cold}, Poison={this._poison}, Energy={this._energy}";
	endfunction
endclass