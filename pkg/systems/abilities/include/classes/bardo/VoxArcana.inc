use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";

/**
 * Vox Arcana - ABILITY_ID::VOX_ARCANA (1708)
 *
 * Passive (via HasAbility in bard system):
 * - Unlocks bard system (canticos, compositions, drum)
 * - Grants access to C0-C1 canticos
 * - Major scale compositions (buffs)
 * - Drum: Ritmo de Guerra
 */
class VoxArcana(CombatAbility)

	function VoxArcana(this)
		Super(ABILITY_ID::VOX_ARCANA);
	endfunction

endclass
