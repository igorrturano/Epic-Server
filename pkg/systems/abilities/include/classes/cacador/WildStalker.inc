include ":abilities:models/CombatAbility";

/**
 * Espreita Selvagem (Wild Stalking) - ABILITY_ID::ESPREITA_SELVAGEM (103)
 *
 * Purely passive ability - no combat hooks.
 * Effects are handled inline across multiple systems:
 *   - stealth.src: Doubles stealth steps (baseSteps * 2.0)
 *   - Vínculo de Guerra: Doubles pet stealth steps
 *   - taming.src: Allows 2 bonded pets instead of 1
 *   - taming.inc: +1 pet slot, bonded pet limit 2 for resurrect
 *   - tamed.src: Bonded pet limit 2 during feeding/fidelization
 */
class WildStalker(CombatAbility)

	function WildStalker(this)
		Super(ABILITY_ID::ESPREITA_SELVAGEM);
	endfunction

endclass