include ":abilities:models/CombatAbility";

/**
 * Arcane Mastery (Sintonia Arcana) - ABILITY_ID::ARCANE_MASTERY (2403)
 *
 * Passive ability (no active ExecuteOn):
 * - +hit chance (ARCANA/10) [HitController.inc]
 * - Enhanced bow model 0xF997 [ElementalBow.inc]
 * - Enhanced bow damage multiplier [HitScriptController.inc]
 *
 * All effects are checked inline via HasAbility(ABILITY_ID::ARCANE_MASTERY)
 */
class ArcaneMastery(CombatAbility)

	function ArcaneMastery(this)
		Super(ABILITY_ID::ARCANE_MASTERY);
	endfunction

endclass
