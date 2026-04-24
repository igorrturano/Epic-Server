use uo;

include ":abilities:models/CombatAbility";

/**
 * Heavy Armor Specialization - ABILITY_ID::HEAVY_ARMOR_SPEC (3202)
 *
 * Passive: Reduces dex penalty by 2 when wearing heavy armor.
 * Effect handled in equipsys/equipitem.src and equipsys/unequipitem.src via ArmorWrapper modifiers.
 * This class is a stub for registry purposes.
 */
class HeavyArmorSpec(CombatAbility)

	function HeavyArmorSpec(this)
		Super(ABILITY_ID::HEAVY_ARMOR_SPEC);
	endfunction

	function OnPassiveOnEquip(this, equip)
		if (!(equip is @ArmorWrapper)) return false; endif
		if (!equip.GetTipoProtecao()["Armadura Pesada"]) return false; endif
		equip.ApplyAbilityBonus(DEXTERITY, 2);
		return true;
	endfunction

endclass
