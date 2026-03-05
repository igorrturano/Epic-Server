use uo;

include ":abilities:models/CombatAbility";

/**
 * Quiet Mind - ABILITY_ID::QUIET_MIND (3400)
 *
 * Passive: Doubles mana regeneration rate.
 * Effect handled in vitalInit.src (GetManaRegenRate).
 * This class is a stub for registry purposes.
 */
class QuietMind(CombatAbility)

	function QuietMind(this)
		Super(ABILITY_ID::QUIET_MIND);
	endfunction

endclass
