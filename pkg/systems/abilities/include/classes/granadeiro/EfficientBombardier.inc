include ":abilities:models/CombatAbility";

/**
 * Efficient Bombardier (Bombardeiro Eficiente) - ABILITY_ID::CAMPO_MINADO (2502)
 *
 * Passive ability (no active ExecuteOn):
 * - Doubles bomb production [alchemybook.src, alchemyAdv.src]
 * - Doubles potion specialist bonus for bombs [alchemy.inc PotionSpecialistBonus]
 *
 * All effects are checked inline via HasAbility(ABILITY_ID::CAMPO_MINADO)
 */
class EfficientBombardier(CombatAbility)

	function EfficientBombardier(this)
		Super(ABILITY_ID::CAMPO_MINADO);
	endfunction

endclass
