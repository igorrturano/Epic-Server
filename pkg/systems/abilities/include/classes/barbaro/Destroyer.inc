include ":abilities:models/CombatAbility";

/**
 * Destruidor (Destroyer) - ABILITY_ID::DESTRUIDOR (604)
 *
 * Purely passive ability - no combat hooks.
 * Effects are handled inline across multiple systems:
 *   - Fury.inc: Doubles fury damage bonus (Tactics/20 * 2) when Destruidor is learned
 *   - DamageController.inc: Doubles critical hit chance (CalculateCriticalChance)
 */
class Destroyer(CombatAbility)

	function Destroyer(this)
		Super(ABILITY_ID::DESTRUIDOR);
	endfunction

endclass
