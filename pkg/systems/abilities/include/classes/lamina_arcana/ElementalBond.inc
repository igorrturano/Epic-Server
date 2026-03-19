use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include "include/epicConstants";

/**
 * Vinculo Elemental (Elemental Bond) - ABILITY_ID::VINCULO_ELEMENTAL (2300)
 *
 * Passive ability (via HasAbility checks):
 * - Allows casting spells while wielding weapons [cast_spell.src]
 * - Prevents disarm [desarmar.src]
 * - Bypasses movement penalties for spellcasting [cast_spell.src]
 * - Increases spiritual weapon pool to 3 slots [absorverarma.src]
 *
 * Spiritual blade summoning is handled by elemental weapon spells
 * (espadaflamejante, espadacongelante, espadaeletrica) via SummonSpiritualBlade().
 */
class ElementalBond(CombatAbility)

	function ElementalBond(this)
		Super(ABILITY_ID::VINCULO_ELEMENTAL);
	endfunction

endclass
