include ":abilities:models/CombatAbility";

/**
 * Sensibilidade Arcana (Arcane Sensitivity) - ABILITY_ID::SENSIBILIDADE_ARCANA (1000)
 *
 * Passive: Allows casting magic tricks (mage hand, mystic bolt) and using scrolls.
 * Uses ARCANA instead of MAGERY for casting checks.
 *
 * Logic lives in cast_spell.src (battlemageHabs check, ARCANA skill substitution).
 */
class ArcaneSensitivity(CombatAbility)

	function ArcaneSensitivity(this)
		Super(ABILITY_ID::SENSIBILIDADE_ARCANA);
	endfunction

endclass
