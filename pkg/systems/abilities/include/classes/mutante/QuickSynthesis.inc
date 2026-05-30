include ":abilities:models/CombatAbility";

/**
 * Sintese Veloz (Quick Synthesis) - ABILITY_ID::SINTESE_VELOZ (902)
 *
 * Passive, checked inline via HasAbility:
 *   - 3x vicio regen: GetVicioPotRegenRate hook in :attributes:hooks/vitalInit.
 *   - PotionSpecialistBonus for BUFF potions + potion-tier boost / vicio-drain
 *     negation in :alchemy:alchemy and the white/blue/int potion scripts.
 */
class QuickSynthesis(CombatAbility)

	function QuickSynthesis(this)
		Super(ABILITY_ID::SINTESE_VELOZ);
	endfunction

endclass
