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

		this.SetSlashRawDamage(slash);
		this.SetPiercingRawDamage(piercing);
		this.SetBluntRawDamage(blunt);

		this.SetFireRawDamage(fire);
		this.SetColdRawDamage(cold);
		this.SetPoisonRawDamage(poison);
		this.SetEnergyRawDamage(energy);
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

	function GetSlashBaseDamage(this)
		return this._slashBase;
	endfunction

	function GetPiercingBaseDamage(this)
		return this._piercingBase;
	endfunction

	function GetBluntBaseDamage(this)
		return this._bluntBase;
	endfunction

	function GetFireBaseDamage(this)
		return this._fireBase;
	endfunction

	function GetColdBaseDamage(this)
		return this._coldBase;
	endfunction

	function GetPoisonBaseDamage(this)
		return this._poisonBase;
	endfunction

	function GetEnergyBaseDamage(this)
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
		this._slashBase := this._slashRaw;
		return this;
	endfunction

	function SetPiercingRawDamage(this, value)
		this._piercingRaw := CInt(value);
		this._piercingBase := this._piercingRaw;
		return this;
	endfunction

	function SetBluntRawDamage(this, value)
		this._bluntRaw := CInt(value);
		this._bluntBase := this._bluntRaw;
		return this;
	endfunction

	function SetFireRawDamage(this, value)
		this._fireRaw := CInt(value);
		this._fireBase := this._fireRaw;
		return this;
	endfunction

	function SetColdRawDamage(this, value)
		this._coldRaw := CInt(value);
		this._coldBase := this._coldRaw;
		return this;
	endfunction

	function SetPoisonRawDamage(this, value)
		this._poisonRaw := CInt(value);
		this._poisonBase := this._poisonRaw;
		return this;
	endfunction

	function SetEnergyRawDamage(this, value)
		this._energyRaw := CInt(value);
		this._energyBase := this._energyRaw;
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

	function SetCritical(this, isCriticalDamage)
		this._criticalDamage := isCriticalDamage;
		return this;
	endfunction

	function IncreaseSlashRawDamage(this, value)
		this._slashRaw += CInt(value);
		this._slashBase += CInt(value);
		return this;
	endfunction

	function IncreasePiercingRawDamage(this, value)
		this._piercingRaw += CInt(value);
		this._piercingBase += CInt(value);
		return this;
	endfunction

	function IncreaseBluntRawDamage(this, value)
		this._bluntRaw += CInt(value);
		this._bluntBase += CInt(value);
		return this;
	endfunction

	function IncreaseFireRawDamage(this, value)
		this._fireRaw += CInt(value);
		this._fireBase += CInt(value);
		return this;
	endfunction

	function IncreaseColdRawDamage(this, value)
		this._coldRaw += CInt(value);
		this._coldBase += CInt(value);
		return this;
	endfunction

	function IncreasePoisonRawDamage(this, value)
		this._poisonRaw += CInt(value);
		this._poisonBase += CInt(value);
		return this;
	endfunction

	function IncreaseEnergyRawDamage(this, value)
		this._energyRaw += CInt(value);
		this._energyBase += CInt(value);
		return this;
	endfunction

	function IncreaseSlashBaseDamage(this, value)
		this._slashBase += CInt(value);
		return this;
	endfunction

	function IncreasePiercingBaseDamage(this, value)
		this._piercingBase += CInt(value);
		return this;
	endfunction

	function IncreaseBluntBaseDamage(this, value)
		this._bluntBase += CInt(value);
		return this;
	endfunction

	function IncreaseFireBaseDamage(this, value)
		this._fireBase += CInt(value);
		return this;
	endfunction

	function IncreaseColdBaseDamage(this, value)
		this._coldBase += CInt(value);
		return this;
	endfunction

	function IncreasePoisonBaseDamage(this, value)
		this._poisonBase += CInt(value);
		return this;
	endfunction

	function IncreaseEnergyBaseDamage(this, value)
		this._energyBase += CInt(value);
		return this;
	endfunction

	function DecreaseSlashRawDamage(this, value)
		this._slashRaw -= CInt(value);
		return this;
	endfunction

	function DecreasePiercingRawDamage(this, value)
		this._piercingRaw -= CInt(value);
		this._piercingBase -= CInt(value);
		return this;
	endfunction

	function DecreaseBluntRawDamage(this, value)
		this._bluntRaw -= CInt(value);
		this._bluntBase -= CInt(value);
		return this;
	endfunction

	function DecreaseFireRawDamage(this, value)
		this._fireRaw -= CInt(value);
		this._fireBase -= CInt(value);
		return this;
	endfunction

	function DecreaseColdRawDamage(this, value)
		this._coldRaw -= CInt(value);
		this._coldBase -= CInt(value);
		return this;
	endfunction

	function DecreasePoisonRawDamage(this, value)
		this._poisonRaw -= CInt(value);
		this._poisonBase -= CInt(value);
		return this;
	endfunction

	function DecreaseEnergyRawDamage(this, value)
		this._energyRaw -= CInt(value);
		this._energyBase -= CInt(value);
		return this;
	endfunction

	function DecreaseSlashBaseDamage(this, value)
		this._slashBase -= CInt(value);
		return this;
	endfunction

	function DecreasePiercingBaseDamage(this, value)
		this._piercingBase -= CInt(value);
		return this;
	endfunction

	function DecreaseBluntBaseDamage(this, value)
		this._bluntBase -= CInt(value);
		return this;
	endfunction

	function DecreaseFireBaseDamage(this, value)
		this._fireBase -= CInt(value);
		return this;
	endfunction

	function DecreaseColdBaseDamage(this, value)
		this._coldBase -= CInt(value);
		return this;
	endfunction

	function DecreasePoisonBaseDamage(this, value)
		this._poisonBase -= CInt(value);
		return this;
	endfunction

	function DecreaseEnergyBaseDamage(this, value)
		this._energyBase -= CInt(value);
		return this;
	endfunction

	function IsCritical(this)
		return this._criticalDamage;
	endfunction

	function ToString(this)
		return "Damage" + (this._criticalDamage ? " (CRITICAL)" : "") + ": " +
			$"Slash: [Raw={this._slashRaw}, Base={this._slashBase}], " +
			$"Piercing: [Raw={this._piercingRaw}, Base={this._piercingBase}], " +
			$"Blunt: [Raw={this._bluntRaw}, Base={this._bluntBase}], " +
			$"Fire: [Raw={this._fireRaw}, Base={this._fireBase}], " +
			$"Cold: [Raw={this._coldRaw}, Base={this._coldBase}], " +
			$"Poison: [Raw={this._poisonRaw}, Base={this._poisonBase}], " +
			$"Energy: [Raw={this._energyRaw}, Base={this._energyBase}]";
	endfunction
endclass

enum class DAMAGE_TIER
	DAMAGE_NONE := "DAMAGE_NONE",

	DAMAGE_FIST := "DAMAGE_FIST",

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
	SLASH := "Cortante",
	PIERCING := "Perfurante",
	BLUNT := "Contusivo",

	FIRE := "Fogo",
	COLD := "Gelo",
	POISON := "Veneno",
	ENERGY := "Energia"
endenum