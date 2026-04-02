include ":abilities:models/CombatAbility";

/**
 * Mumificacao (Mummification) - ABILITY_ID::MUMIFICACAO (1404)
 *
 * Passive: Immune to critical hits and poisons.
 *
 * Logic lives in:
 *   - DamageController.inc CalculateCriticalChance() — blocks all crits
 *   - timerControl.src — blocks poison application
 */
class Mummification(CombatAbility)

	function Mummification(this)
		Super(ABILITY_ID::MUMIFICACAO);
	endfunction

endclass
