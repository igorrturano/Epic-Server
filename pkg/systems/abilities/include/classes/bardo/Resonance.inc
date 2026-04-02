use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";

/**
 * Ressonancia (Resonance) - ABILITY_ID::RESONANCE (1709)
 *
 * Passive (via HasAbility in bard system):
 * - +Arcana/20 tiles range bonus to canticos/compositions
 * - +Arcana/2% duration bonus
 * - Grants access to C2 canticos
 * - Minor scale compositions (debuffs)
 * - Drum: Batida Ressonante, Cadencia da Marcha
 */
class Resonance(CombatAbility)

	function Resonance(this)
		Super(ABILITY_ID::RESONANCE);
	endfunction

endclass
