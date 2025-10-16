include "IDamageDice";
include ":diceroll:model/Dice";

include ":equipsys:utils/consts";

class DamageDice(IDamageDice)
	var _slash := Dice("1d6+2");
	var _piercing := Dice("1d6+2");
	var _blunt := Dice("1d6+2");
	var _fire := Dice("1d6+2");
	var _cold := Dice("1d6+2");
	var _poison := Dice("1d6+2");
	var _energy := Dice("1d6+2");

	function DamageDice(this, slash := DAMAGE_NONE, piercing := DAMAGE_NONE, blunt := DAMAGE_NONE, fire := DAMAGE_NONE, cold := DAMAGE_NONE, poison := DAMAGE_NONE, energy := DAMAGE_NONE)
		this._slash := slash == DAMAGE_NONE ? Dice(DAMAGE_NONE) : slash;
		this._piercing := piercing == DAMAGE_NONE ? Dice(DAMAGE_NONE) : piercing;
		this._blunt := blunt == DAMAGE_NONE ? Dice(DAMAGE_NONE) : blunt;
		this._fire := fire == DAMAGE_NONE ? Dice(DAMAGE_NONE) : fire;
		this._cold := cold == DAMAGE_NONE ? Dice(DAMAGE_NONE) : cold;
		this._poison := poison == DAMAGE_NONE ? Dice(DAMAGE_NONE) : poison;
		this._energy := energy == DAMAGE_NONE ? Dice(DAMAGE_NONE) : energy;
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
		return this;
	endfunction

	function SetPiercingDamage(this, value)
		this._piercing := value;
		return this;
	endfunction

	function SetBluntDamage(this, value)
		this._blunt := value;
		return this;
	endfunction

	function SetFireDamage(this, value)
		this._fire := value;
		return this;
	endfunction

	function SetColdDamage(this, value)
		this._cold := value;
		return this;
	endfunction

	function SetPoisonDamage(this, value)
		this._poison := value;
		return this;
	endfunction

	function SetEnergyDamage(this, value)
		this._energy := value;
		return this;
	endfunction

	function ToString(this)
		return $"Weapon Damage: Slash={this._slash}, Piercing={this._piercing}, Blunt={this._blunt}, Fire={this._fire}, Cold={this._cold}, Poison={this._poison}, Energy={this._energy}";
	endfunction
endclass