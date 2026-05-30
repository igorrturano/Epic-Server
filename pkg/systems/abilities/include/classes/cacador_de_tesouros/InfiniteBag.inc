include ":abilities:models/CombatAbility";

/**
 * Bolsa Infinita (Infinite Bag) - ABILITY_ID::BOLSA_INFINITA (1104)
 *
 * Passive: +120 carrying capacity (set on ability purchase).
 *
 * Logic lives in:
 *   - charactercreation/include/habilidades.inc — carrying_capacity_mod = 120
 */
class InfiniteBag(CombatAbility)

	function InfiniteBag(this)
		Super(ABILITY_ID::BOLSA_INFINITA);
	endfunction

endclass
