include "IArmorResist";

class ArmorResist(IArmorResist)
	var _slash := 0;
	var _piercing := 0;
	var _blunt := 0;

	var _fire := 0;
	var _cold := 0;
	var _poison := 0;
	var _energy := 0;

	function ArmorResist(this, slash := 0, piercing := 0, blunt := 0, fire := 0, cold := 0, poison := 0, energy := 0)
		this._slash := CInt(slash);
		this._piercing := CInt(piercing);
		this._blunt := CInt(blunt);

		this._fire := CInt(fire);
		this._cold := CInt(cold);
		this._poison := CInt(poison);
		this._energy := CInt(energy);
	endfunction

	function GetSlashResist(this)
		return this._slash;
	endfunction

	function GetPiercingResist(this)
		return this._piercing;
	endfunction

	function GetBluntResist(this)
		return this._blunt;
	endfunction

	function GetFireResist(this)
		return this._fire;
	endfunction

	function GetColdResist(this)
		return this._cold;
	endfunction

	function GetPoisonResist(this)
		return this._poison;
	endfunction

	function GetEnergyResist(this)
		return this._energy;
	endfunction

	function ToString(this)
		return "Slash: " + this._slash + ", Piercing: " + this._piercing + ", Blunt: " + this._blunt + ", Fire: " + this._fire + ", Cold: " + this._cold + ", Poison: " + this._poison + ", Energy: " + this._energy;
	endfunction
endclass

enum class PROTECTION_TYPE
	ARMOR_LOWLOW := "ARMADURA LEVE FRACA",
	ARMOR_LOWMID := "ARMADURA LEVE MEDIA",
	ARMOR_LOWHIGH := "ARMADURA LEVE FORTE",

	ARMOR_MIDLOW := "ARMADURA MEDIA FRACA",
	ARMOR_MIDMID := "ARMADURA MEDIA MEDIA",
	ARMOR_MIDHIGH := "ARMADURA MEDIA FORTE",

	ARMOR_HIGHLOW := "ARMADURA PESADA FRACA",
	ARMOR_HIGHMID := "ARMADURA PESADA MEDIA",
	ARMOR_HIGHHIGH := "ARMADURA PESADA FORTE",

	SHIELD_BUCKLER := "ESCUDO BROQUEL",
	SHIELD_SMALL := "ESCUDO PEQUENO",
	SHIELD_MEDIUM := "ESCUDO MEDIO",
	SHIELD_LARGE := "ESCUDO GRANDE",
	SHIELD_TOWER := "ESCUDO TORRE"
endenum