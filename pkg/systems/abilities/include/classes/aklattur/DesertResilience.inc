include ":abilities:models/CombatAbility";

/**
 * Resistencia do Deserto (Desert Resilience) - ABILITY_ID::RESISTENCIA_DO_DESERTO (503)
 *
 * Purely passive ability - no combat hooks.
 * Effects are handled inline:
 *   - habilidades.inc: +20 hitsmod + RecalcVitals()
 */
class DesertResilience(CombatAbility)

	function DesertResilience(this)
		Super(ABILITY_ID::RESISTENCIA_DO_DESERTO);
	endfunction

endclass
