use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";

/**
 * Maestria do Caos (Chaos Mastery) - ABILITY_ID::CHAOS_MASTERY (1711)
 *
 * Passive (via HasAbility in bard system):
 * - 2 simultaneous canticos
 * - Grants access to C4 canticos
 * - 3rd octave unlocked for compositions
 * - Surge Entropico chance: 5% -> 20% (rolls 81-100 instead of 96-100)
 */
class ChaosMastery(CombatAbility)

	function ChaosMastery(this)
		Super(ABILITY_ID::CHAOS_MASTERY);
	endfunction

endclass
