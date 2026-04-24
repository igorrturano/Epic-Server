use uo;

include ":abilities:models/CombatAbility";

/**
 * Spell Range - ABILITY_ID::SPELL_RANGE (3403)
 *
 * Passive: Increases spell range by 5 tiles.
 * Effect handled in epicMageryUtils.inc (distanceAllowedForSpells).
 * This class is a stub for registry purposes.
 */
class SpellRange(CombatAbility)

	function SpellRange(this)
		Super(ABILITY_ID::SPELL_RANGE);
	endfunction

endclass
