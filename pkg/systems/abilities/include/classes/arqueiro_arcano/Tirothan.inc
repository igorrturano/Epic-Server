include ":abilities:models/CombatAbility";

/**
 * Tirothan - ABILITY_ID::TIROTHAN (2403)
 *
 * Passive ability (no active ExecuteOn):
 * - +hit chance (ARCANA/10) [HitController.inc]
 * - +spell range for Evocatium school [epicMageryUtils.inc]
 * - Enhanced bow model 0xF997 [arcoelemental.src / ElementalBow.inc]
 * - Enhanced bow damage multiplier [HitScriptController.inc]
 *
 * All effects are checked inline via HasAbility(ABILITY_ID::TIROTHAN)
 */
class Tirothan(CombatAbility)

	function Tirothan(this)
		Super(ABILITY_ID::TIROTHAN);
	endfunction

endclass
