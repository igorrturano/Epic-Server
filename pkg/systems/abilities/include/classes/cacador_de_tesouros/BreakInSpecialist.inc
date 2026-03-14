include ":abilities:models/CombatAbility";

/**
 * Especialista em Invasoes (Break-In Specialist) - ABILITY_ID::ESPECIALISTA_EM_INVASOES (1103)
 *
 * Passive: Never critical fails when lockpicking or disarming traps.
 *
 * Logic lives in:
 *   - lockPicking.src — prevents lockpick breaking on critical fail
 *   - mechanicalapitude.src — prevents trap trigger on critical fail
 */
class BreakInSpecialist(CombatAbility)

	function BreakInSpecialist(this)
		Super(ABILITY_ID::ESPECIALISTA_EM_INVASOES);
	endfunction

endclass
