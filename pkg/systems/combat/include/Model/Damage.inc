include "IDamage";

class Damage(IDamage)
	var _criticalDamage := false;

	var _slash := struct{
		raw := 0,
		base := 0
	};
	var _piercing := struct{
		raw := 0,
		base := 0
	};
	var _blunt := struct{
		raw := 0,
		base := 0
	};
	var _fire := struct{
		raw := 0,
		base := 0
	};
	var _cold := struct{
		raw := 0,
		base := 0
	};
	var _poison := struct{
		raw := 0,
		base := 0
	};
	var _energy := struct{
		raw := 0,
		base := 0
	};

	function Damage(this, slash := 0, piercing := 0, blunt := 0, fire := 0, cold := 0, poison := 0, energy := 0)
		this._criticalDamage := false;

		this._slash := Damage::_slash;
		this._slash.raw := CInt(slash);
		this._slash.base := this._slash.raw;
		this._piercing := Damage::_piercing;
		this._piercing.raw := CInt(piercing);
		this._piercing.base := this._piercing.raw;
		this._blunt := Damage::_blunt;
		this._blunt.raw := CInt(blunt);
		this._blunt.base := this._blunt.raw;
		this._fire := Damage::_fire;
		this._fire.raw := CInt(fire);
		this._fire.base := this._fire.raw;
		this._cold := Damage::_cold;
		this._cold.raw := CInt(cold);
		this._cold.base := this._cold.raw;
		this._poison := Damage::_poison;
		this._poison.raw := CInt(poison);
		this._poison.base := this._poison.raw;
		this._energy := Damage::_energy;
		this._energy.raw := CInt(energy);
		this._energy.base := this._energy.raw;
	endfunction

	function GetSlashRawDamage(this)
		return this._slash.raw;
	endfunction

	function GetPiercingRawDamage(this)
		return this._piercing.raw;
	endfunction

	function GetBluntRawDamage(this)
		return this._blunt.raw;
	endfunction

	function GetFireRawDamage(this)
		return this._fire.raw;
	endfunction

	function GetColdRawDamage(this)
		return this._cold.raw;
	endfunction

	function GetPoisonRawDamage(this)
		return this._poison.raw;
	endfunction

	function GetEnergyRawDamage(this)
		return this._energy.raw;
	endfunction

	function GetSlashDamage(this)
		return this._slash.base;
	endfunction

	function GetPiercingDamage(this)
		return this._piercing.base;
	endfunction

	function GetBluntDamage(this)
		return this._blunt.base;
	endfunction

	function GetFireDamage(this)
		return this._fire.base;
	endfunction

	function GetColdDamage(this)
		return this._cold.base;
	endfunction

	function GetPoisonDamage(this)
		return this._poison.base;
	endfunction

	function GetEnergyDamage(this)
		return this._energy.base;
	endfunction

	function GetTotalRawDamage(this)
		return this._slash.raw + this._piercing.raw + this._blunt.raw + this._fire.raw + this._cold.raw + this._poison.raw + this._energy.raw;
	endfunction

	function SetSlashRawDamage(this, value)
		this._slash.raw := CInt(value);
		return this;
	endfunction

	function SetPiercingRawDamage(this, value)
		this._piercing.raw := CInt(value);
		return this;
	endfunction

	function SetBluntRawDamage(this, value)
		this._blunt.raw := CInt(value);
		return this;
	endfunction

	function SetFireRawDamage(this, value)
		this._fire.raw := CInt(value);
		return this;
	endfunction

	function SetColdRawDamage(this, value)
		this._cold.raw := CInt(value);
		return this;
	endfunction

	function SetPoisonRawDamage(this, value)
		this._poison.raw := CInt(value);
		return this;
	endfunction

	function SetEnergyRawDamage(this, value)
		this._energy.raw := CInt(value);
		return this;
	endfunction

	function SetSlashDamage(this, value)
		this._slash.base := CInt(value);
		return this;
	endfunction

	function SetPiercingDamage(this, value)
		this._piercing.base := CInt(value);
		return this;
	endfunction

	function SetBluntDamage(this, value)
		this._blunt.base := CInt(value);
		return this;
	endfunction

	function SetFireDamage(this, value)
		this._fire.base := CInt(value);
		return this;
	endfunction

	function SetColdDamage(this, value)
		this._cold.base := CInt(value);
		return this;
	endfunction

	function SetPoisonDamage(this, value)
		this._poison.base := CInt(value);
		return this;
	endfunction

	function SetEnergyDamage(this, value)
		this._energy.base := CInt(value);
		return this;
	endfunction

	function SetCritical(this)
		this._criticalDamage := true;
		return this;
	endfunction

	function DecreaseSlashDamage(this, value)
		this._slash -= CInt(value);
		return this;
	endfunction

	function DecreasePiercingDamage(this, value)
		this._piercing -= CInt(value);
		return this;
	endfunction

	function DecreaseBluntDamage(this, value)
		this._blunt -= CInt(value);
		return this;
	endfunction

	function DecreaseFireDamage(this, value)
		this._fire -= CInt(value);
		return this;
	endfunction

	function DecreaseColdDamage(this, value)
		this._cold -= CInt(value);
		return this;
	endfunction

	function DecreasePoisonDamage(this, value)
		this._poison -= CInt(value);
		return this;
	endfunction

	function DecreaseEnergyDamage(this, value)
		this._energy -= CInt(value);
		return this;
	endfunction

	function IncreaseSlashDamage(this, value)
		this._slash += CInt(value);
		return this;
	endfunction

	function IncreasePiercingDamage(this, value)
		this._piercing += CInt(value);
		return this;
	endfunction

	function IncreaseBluntDamage(this, value)
		this._blunt += CInt(value);
		return this;
	endfunction

	function IncreaseFireDamage(this, value)
		this._fire += CInt(value);
		return this;
	endfunction

	function IncreaseColdDamage(this, value)
		this._cold += CInt(value);
		return this;
	endfunction

	function IncreasePoisonDamage(this, value)
		this._poison += CInt(value);
		return this;
	endfunction

	function IncreaseEnergyDamage(this, value)
		this._energy += CInt(value);
		return this;
	endfunction

	function IsCritical(this)
		return this._criticalDamage;
	endfunction

	function ToString(this)
		return "Damage " + (this._criticalDamage ? "(CRITICAL)" : "") + ":" +
			$"Slash: Raw={this._slash.raw}, Base={this._slash.base}" +
			$"Piercing: Raw={this._piercing.raw}, Base={this._piercing.base}" +
			$"Blunt: Raw={this._blunt.raw}, Base={this._blunt.base}" +
			$"Fire: Raw={this._fire.raw}, Base={this._fire.base}" +
			$"Cold: Raw={this._cold.raw}, Base={this._cold.base}" +
			$"Poison: Raw={this._poison.raw}, Base={this._poison.base}" +
			$"Energy: Raw={this._energy.raw}, Base={this._energy.base}";
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