include ":abilities:models/CombatAbility";

/**
 * Sempre Alerta (Always Alert) - ABILITY_ID::SEMPRE_ALERTA (1102)
 *
 * Passive: Immune to surprise attacks, +5 detection range for hidden creatures
 * and secret doors, and scans every 3s instead of 15s.
 *
 * Logic lives in:
 *   - awareness.inc — detection range bonus, counters enhanced stealth
 *   - alert.src — faster scan interval
 */
class AlwaysAlert(CombatAbility)

	function AlwaysAlert(this)
		Super(ABILITY_ID::SEMPRE_ALERTA);
	endfunction

endclass
