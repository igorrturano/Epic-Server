use uo;

include "ResistModBase";
include "ArmorResist";

// Persistent resist mod source for the swordmage Encantar Objeto spell
// (and the alchemy potion variant). Reads the elemental %-reduction struct
// from the EncantarResist CProp written by setBonus in encantarObjeto.inc.
// Encantar buffs only the four elementals — physical resists are left alone.
class ResistModEncantar(ResistModBase)

	function ResistModEncantar(this, byRef item)
		this._item := item;
		Super(MOD_SOURCES::ENCHANT, 0);
	endfunction

	function MapResist(this, param)
		var resist := param is @ArmorResist ? param : ArmorResist();

		if (!this._item) return resist; endif

		var bonus := GetObjProperty(this._item, "EncantarResist");
		if (!bonus) return resist; endif

		resist.SetFireResist(CInt(bonus.fire));
		resist.SetColdResist(CInt(bonus.cold));
		resist.SetPoisonResist(CInt(bonus.poison));
		resist.SetEnergyResist(CInt(bonus.energy));

		return resist;
	endfunction

endclass
