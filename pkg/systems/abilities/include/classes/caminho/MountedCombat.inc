use uo;

include ":abilities:models/CombatAbility";

/**
 * Mounted Combat - ABILITY_ID::MOUNTED_COMBAT (3301)
 *
 * Passive: Reduces mounted combat penalty by 25%.
 * Effect handled in HitController.inc (_GetSkillForCombat).
 * This class is a stub for registry purposes.
 */
class MountedCombat(CombatAbility)

	function MountedCombat(this)
		Super(ABILITY_ID::MOUNTED_COMBAT);
	endfunction

endclass
