use uo;

include "DamageModBase";
include "Damage";

// Persistent damage mod source for the swordmage Encantar Objeto spell
// (and the alchemy potion variant). Reads the elemental flat-damage struct
// from the EncantarDamage CProp written by setBonus in encantarObjeto.inc.
// Mirrors ResistModRuna for the weapon side.
class DamageModEncantar(DamageModBase)

	function DamageModEncantar(this, byRef item)
		this._item := item;
		Super(MOD_SOURCES::ENCHANT, 0);
	endfunction

	function MapDamage(this, param)
		var damage := param is @Damage ? param : Damage();

		if (!this._item) return damage; endif

		var bonus := GetObjProperty(this._item, "EncantarDamage");
		if (!bonus) return damage; endif

		damage.SetFireRawDamage(CInt(bonus.fire));
		damage.SetColdRawDamage(CInt(bonus.cold));
		damage.SetPoisonRawDamage(CInt(bonus.poison));
		damage.SetEnergyRawDamage(CInt(bonus.energy));

		return damage;
	endfunction

endclass
