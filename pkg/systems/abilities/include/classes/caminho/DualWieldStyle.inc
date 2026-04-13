use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Dual Wield Style - ABILITY_ID::DUAL_WIELD_STYLE (3103)
 *
 * Passive: When dual wielding: +3 hit chance, -2 enemy hit chance.
 * Logic handled in HitController::CalculateAbilityHitModifier.
 */
class DualWieldStyle(CombatAbility)

	function DualWieldStyle(this)
		Super(ABILITY_ID::DUAL_WIELD_STYLE);
	endfunction

endclass
