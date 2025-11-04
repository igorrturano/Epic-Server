include "IDamage";

class Damage(IDamage)
	var _criticalDamage := false;

	var _slashRaw := 0;
	var _slashBase := 0;
	var _piercingRaw := 0;
	var _piercingBase := 0;
	var _bluntRaw := 0;
	var _bluntBase := 0;
	var _fireRaw := 0;
	var _fireBase := 0;
	var _coldRaw := 0;
	var _coldBase := 0;
	var _poisonRaw := 0;
	var _poisonBase := 0;
	var _energyRaw := 0;
	var _energyBase := 0;

	function Damage(this, slash := 0, piercing := 0, blunt := 0, fire := 0, cold := 0, poison := 0, energy := 0)
		this._criticalDamage := false;

		this._slashRaw := CInt(slash);
		this._slashBase := this._slashRaw;
		this._piercingRaw := CInt(piercing);
		this._piercingBase := this._piercingRaw;
		this._bluntRaw := CInt(blunt);
		this._bluntBase := this._bluntRaw;
		this._fireRaw := CInt(fire);
		this._fireBase := this._fireRaw;
		this._coldRaw := CInt(cold);
		this._coldBase := this._coldRaw;
		this._poisonRaw := CInt(poison);
		this._poisonBase := this._poisonRaw;
		this._energyRaw := CInt(energy);
		this._energyBase := this._energyRaw;
	endfunction

	function GetSlashRawDamage(this)
		return this._slashRaw;
	endfunction

	function GetPiercingRawDamage(this)
		return this._piercingRaw;
	endfunction

	function GetBluntRawDamage(this)
		return this._bluntRaw;
	endfunction

	function GetFireRawDamage(this)
		return this._fireRaw;
	endfunction

	function GetColdRawDamage(this)
		return this._coldRaw;
	endfunction

	function GetPoisonRawDamage(this)
		return this._poisonRaw;
	endfunction

	function GetEnergyRawDamage(this)
		return this._energyRaw;
	endfunction

	function GetSlashDamage(this)
		return this._slashBase;
	endfunction

	function GetPiercingDamage(this)
		return this._piercingBase;
	endfunction

	function GetBluntDamage(this)
		return this._bluntBase;
	endfunction

	function GetFireDamage(this)
		return this._fireBase;
	endfunction

	function GetColdDamage(this)
		return this._coldBase;
	endfunction

	function GetPoisonDamage(this)
		return this._poisonBase;
	endfunction

	function GetEnergyDamage(this)
		return this._energyBase;
	endfunction

	function GetTotalRawDamage(this)
		return this._slashRaw + this._piercingRaw + this._bluntRaw + this._fireRaw + this._coldRaw + this._poisonRaw + this._energyRaw;
	endfunction

	function GetTotalBaseDamage(this)
		return this._slashBase + this._piercingBase + this._bluntBase + this._fireBase + this._coldBase + this._poisonBase + this._energyBase;
	endfunction

	function SetSlashRawDamage(this, value)
		this._slashRaw := CInt(value);
		return this;
	endfunction

	function SetPiercingRawDamage(this, value)
		this._piercingRaw := CInt(value);
		return this;
	endfunction

	function SetBluntRawDamage(this, value)
		this._bluntRaw := CInt(value);
		return this;
	endfunction

	function SetFireRawDamage(this, value)
		this._fireRaw := CInt(value);
		return this;
	endfunction

	function SetColdRawDamage(this, value)
		this._coldRaw := CInt(value);
		return this;
	endfunction

	function SetPoisonRawDamage(this, value)
		this._poisonRaw := CInt(value);
		return this;
	endfunction

	function SetEnergyRawDamage(this, value)
		this._energyRaw := CInt(value);
		return this;
	endfunction

	function SetSlashBaseDamage(this, value)
		this._slashBase := CInt(value);
		return this;
	endfunction

	function SetPiercingBaseDamage(this, value)
		this._piercingBase := CInt(value);
		return this;
	endfunction

	function SetBluntBaseDamage(this, value)
		this._bluntBase := CInt(value);
		return this;
	endfunction

	function SetFireBaseDamage(this, value)
		this._fireBase := CInt(value);
		return this;
	endfunction

	function SetColdBaseDamage(this, value)
		this._coldBase := CInt(value);
		return this;
	endfunction

	function SetPoisonBaseDamage(this, value)
		this._poisonBase := CInt(value);
		return this;
	endfunction

	function SetEnergyBaseDamage(this, value)
		this._energyBase := CInt(value);
		return this;
	endfunction

	function SetCritical(this)
		this._criticalDamage := true;
		return this;
	endfunction

	function IsCritical(this)
		return this._criticalDamage;
	endfunction

	function ToString(this)
		return "Damage " + (this._criticalDamage ? "(CRITICAL)" : "") + ":" +
			$"Slash: Raw={this._slashRaw}, Base={this._slashBase}" +
			$"Piercing: Raw={this._piercingRaw}, Base={this._piercingBase}" +
			$"Blunt: Raw={this._bluntRaw}, Base={this._bluntBase}" +
			$"Fire: Raw={this._fireRaw}, Base={this._fireBase}" +
			$"Cold: Raw={this._coldRaw}, Base={this._coldBase}" +
			$"Poison: Raw={this._poisonRaw}, Base={this._poisonBase}" +
			$"Energy: Raw={this._energyRaw}, Base={this._energyBase}";
	endfunction
endclass

enum class DAMAGE_TIER
	DAMAGE_NONE := "DAMAGE_NONE",

	ONEHANDED_LOW := "ONEHANDED_LOW",
	ONEHANDED_MID := "ONEHANDED_MID",
	ONEHANDED_HIGH := "ONEHANDED_HIGH",

	TWOHANDED_LOW := "TWOHANDED_LOW",
	TWOHANDED_MID := "TWOHANDED_MID",
	TWOHANDED_HIGH := "TWOHANDED_HIGH",

	BOW_LOW := "BOW_LOW",
	BOW_MID := "BOW_MID",
	BOW_HIGH := "BOW_HIGH",

	CROSSBOW_LOW := "CROSSBOW_LOW",
	CROSSBOW_MID := "CROSSBOW_MID",
	CROSSBOW_HIGH := "CROSSBOW_HIGH",

	THROWING_LOW := "THROWING_LOW",
	THROWING_MID := "THROWING_MID",
	THROWING_HIGH := "THROWING_HIGH"
endenum

enum class DAMAGE_TYPE
	SLASHDAMAGE := "Cortante",
	PIERCINGDAMAGE := "Perfurante",
	BLUNTDAMAGE := "Contusivo"
endenum