include "IDamage";

class Damage(IDamage)
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

	function IncreaseSlashDamage(this, value)
		this._slash += CInt(value);
	endfunction

	function IncreasePiercingDamage(this, value)
		this._piercing += CInt(value);
	endfunction

	function IncreaseBluntDamage(this, value)
		this._blunt += CInt(value);
	endfunction

	function IncreaseFireDamage(this, value)
		this._fire += CInt(value);
	endfunction

	function IncreaseColdDamage(this, value)
		this._cold += CInt(value);
	endfunction

	function IncreasePoisonDamage(this, value)
		this._poison += CInt(value);
	endfunction

	function IncreaseEnergyDamage(this, value)
		this._energy += CInt(value);
	endfunction

	function DecreaseSlashDamage(this, value)
		if ( this._slash == 0 ) return; endif
		this._slash -= CInt(value);
	endfunction

	function DecreasePiercingDamage(this, value)
		if ( this._piercing == 0 ) return; endif
		this._piercing -= CInt(value);
	endfunction

	function DecreaseBluntDamage(this, value)
		if ( this._blunt == 0 ) return; endif
		this._blunt -= CInt(value);
	endfunction

	function DecreaseFireDamage(this, value)
		if ( this._fire == 0 ) return; endif
		this._fire -= CInt(value);
	endfunction

	function DecreaseColdDamage(this, value)
		if ( this._cold == 0 ) return; endif
		this._cold -= CInt(value);
	endfunction

	function DecreasePoisonDamage(this, value)
		if ( this._poison == 0 ) return; endif
		this._poison -= CInt(value);
	endfunction

	function DecreaseEnergyDamage(this, value)
		if ( this._energy == 0 ) return; endif
		this._energy -= CInt(value);
	endfunction

	function ToString(this)
		return $"Damage: Slash={this._slash}, Piercing={this._piercing}, Blunt={this._blunt}, Fire={this._fire}, Cold={this._cold}, Poison={this._poison}, Energy={this._energy}";
	endfunction
endclass

enum class DAMAGETIER
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

enum class DAMAGETYPE
	SLASHDAMAGE := "Cortante",
	PIERCINGDAMAGE := "Perfurante",
	BLUNTDAMAGE := "Contusivo"
endenum