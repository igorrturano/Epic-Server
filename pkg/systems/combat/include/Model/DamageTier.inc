include "Damage";

class DamageTier()
	var _slashDamage := "";
	var _piercingDamage := "";
	var _bluntDamage := "";
	var _fireDamage := "";
	var _coldDamage := "";
	var _poisonDamage := "";
	var _energyDamage := "";

	function DamageTier(this, slashDamage := DAMAGE_TIER::DAMAGE_NONE, piercingDamage := DAMAGE_TIER::DAMAGE_NONE, bluntDamage := DAMAGE_TIER::DAMAGE_NONE, fireDamage := DAMAGE_TIER::DAMAGE_NONE, coldDamage := DAMAGE_TIER::DAMAGE_NONE, poisonDamage := DAMAGE_TIER::DAMAGE_NONE, energyDamage := DAMAGE_TIER::DAMAGE_NONE)
		this._slashDamage := slashDamage;
		this._piercingDamage := piercingDamage;
		this._bluntDamage := bluntDamage;
		this._fireDamage := fireDamage;
		this._coldDamage := coldDamage;
		this._poisonDamage := poisonDamage;
		this._energyDamage := energyDamage;
	endfunction

	function GetSlashDamage(this)
		return this._slashDamage;
	endfunction

	function GetPiercingDamage(this)
		return this._piercingDamage;
	endfunction

	function GetBluntDamage(this)
		return this._bluntDamage;
	endfunction

	function GetFireDamage(this)
		return this._fireDamage;
	endfunction

	function GetColdDamage(this)
		return this._coldDamage;
	endfunction

	function GetPoisonDamage(this)
		return this._poisonDamage;
	endfunction

	function GetEnergyDamage(this)
		return this._energyDamage;
	endfunction

	function SetSlashDamage(this, slashDamage)
		this._slashDamage := slashDamage;
	endfunction

	function SetPiercingDamage(this, piercingDamage)
		this._piercingDamage := piercingDamage;
	endfunction

	function SetBluntDamage(this, bluntDamage)
		this._bluntDamage := bluntDamage;
	endfunction

	function SetFireDamage(this, fireDamage)
		this._fireDamage := fireDamage;
	endfunction

	function SetColdDamage(this, coldDamage)
		this._coldDamage := coldDamage;
	endfunction

	function SetPoisonDamage(this, poisonDamage)
		this._poisonDamage := poisonDamage;
	endfunction

	function SetEnergyDamage(this, energyDamage)
		this._energyDamage := energyDamage;
	endfunction

	function ToString(this)
		return "DamageTier[" +
			$"Slash: {this._slashDamage}, " +
			$"Piercing: {this._piercingDamage}, " +
			$"Blunt: {this._bluntDamage}, " +
			$"Fire: {this._fireDamage}, " +
			$"Cold: {this._coldDamage}, " +
			$"Poison: {this._poisonDamage}, " +
			$"Energy: {this._energyDamage}]";
	endfunction
endclass