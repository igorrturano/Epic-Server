include ":abilities:models/CombatAbility";

/**
 * Veneno Puro (Pure Poison) - ABILITY_ID::VENENO_PURO (1402)
 *
 * Passive: Maximizes poison potential — increased duration, dosage,
 * and paralyze time.
 *
 * Logic lives in:
 *   - DamageController.inc — doubles paralyze duration
 *   - alchemy.inc — alchemy specialization bonus
 */
class PurePoison(CombatAbility)

	function PurePoison(this)
		Super(ABILITY_ID::VENENO_PURO);
	endfunction

endclass
