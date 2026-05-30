use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";

/**
 * Tear da Realidade (Reality Tear) - ABILITY_ID::REALITY_TEAR (1710)
 *
 * Passive (via HasAbility in bard system):
 * - Chromatic scale compositions (chaos)
 * - Grants access to C3 canticos
 * - All 4 drum buffs unlocked (Rufar dos Trovoes)
 * - Void portal system: open (Arcana/20 capacity pts) and seal
 */
class RealityTear(CombatAbility)

	function RealityTear(this)
		Super(ABILITY_ID::REALITY_TEAR);
	endfunction

endclass
