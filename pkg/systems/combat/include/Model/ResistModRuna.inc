use uo;

include "ResistModBase";
include "ArmorResist";

class ResistModRuna(ResistModBase)

	function ResistModRuna(this, byRef item)
		this._item := item;
		Super(MOD_SOURCES::RUNE, 0);
	endfunction

	function MapResist(this, param)
		var resist;
		if (param is @ArmorResist)
			resist := param;
		else
			resist := ArmorResist();
		endif

		if (!this._item) return resist; endif

		var physBonus := CInt(GetObjProperty(this._item, "runePhysBonus"));
		if (physBonus)
			resist.SetSlashResist(physBonus);
			resist.SetPiercingResist(physBonus);
			resist.SetBluntResist(physBonus);
		endif

		var fireBonus := CInt(GetObjProperty(this._item, "runeFireBonus"));
		if (fireBonus) resist.SetFireResist(fireBonus); endif

		var coldBonus := CInt(GetObjProperty(this._item, "runeColdBonus"));
		if (coldBonus) resist.SetColdResist(coldBonus); endif

		var energyBonus := CInt(GetObjProperty(this._item, "runeEnergyBonus"));
		if (energyBonus) resist.SetEnergyResist(energyBonus); endif

		var poisonBonus := CInt(GetObjProperty(this._item, "runePoisonBonus"));
		if (poisonBonus) resist.SetPoisonResist(poisonBonus); endif

		return resist;
	endfunction

endclass
