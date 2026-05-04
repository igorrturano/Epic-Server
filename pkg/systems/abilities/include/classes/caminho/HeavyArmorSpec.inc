use uo;

include ":abilities:models/CombatAbility";

/**
 * Heavy Armor Specialization - ABILITY_ID::HEAVY_ARMOR_SPEC (3202)
 *
 * Passive (on equip): reduz penalidade de DEX em 2 ao equipar peca pesada.
 * Aplicado via ApplyAbilityBonus → ArmorWrapper modifier no equip pipeline.
 *
 * Passive (on parry): +1 chance de parry por peca de armadura pesada
 * equipada (max +5 com 5 pecas). Contado ao vivo em
 * ParryController.CalculateParryChance — segue o padrao inline ja usado por
 * POSTURA_DE_DUELO e WANDERER_PATH (parry-side nao tem hook OnPassiveBeforeParry).
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
