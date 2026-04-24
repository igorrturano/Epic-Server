include ":abilities:models/CombatAbility";

/**
 * Metabolic Boost (Impulso Metabólico) - ABILITY_ID::IMPULSO_METABOLICO (902)
 *
 * Passive: All effects checked inline via HasAbility:
 * - Potion tier +2 (whitePotion.src, bluePotion.src, intPotion.src)
 * - PotionSpecialistBonus for BUFF type (alchemy.inc)
 * - Negates vicio/addiction penalty (alchemy.inc VicioSTR/DEX/INT)
 */
class MetabolicBoost(CombatAbility)

	function MetabolicBoost(this)
		Super(ABILITY_ID::IMPULSO_METABOLICO);
	endfunction

endclass
