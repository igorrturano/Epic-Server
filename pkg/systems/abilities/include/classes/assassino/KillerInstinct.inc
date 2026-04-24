include ":abilities:models/CombatAbility";

/**
 * Instinto Assassino (Killer Instinct) - ABILITY_ID::INSTINTO_ASSASSINO (1201)
 *
 * Passive: When landing a sneak attack, guarantees a critical hit.
 *
 * Logic lives in:
 *   - Assassinate.inc ExecuteOn() — checks HasAbility(INSTINTO_ASSASSINO), sets #nextcritical
 *   - DamageController.inc CalculateCriticalChance() — checks #nextcritical, returns 100%
 */
class KillerInstinct(CombatAbility)

	function KillerInstinct(this)
		Super(ABILITY_ID::INSTINTO_ASSASSINO);
	endfunction

endclass
