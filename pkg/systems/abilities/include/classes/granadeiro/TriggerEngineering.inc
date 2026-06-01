use uo;
use os;

include ":abilities:models/CombatAbility";

/**
 * Engenharia de Gatilhos - ABILITY_ID::ENGENHARIA_DE_GATILHOS (2500)
 *
 * Active (OnUse):
 *   - Player: opens the explosive configurator gump. All five tabs are
 *             available from L1 (no tab gating). The gump lives in a standalone
 *             script (:alchemy:explosiveConfig) to keep its strings out of the
 *             AbilityExecutorSabio data segment.
 *   - NPC:    throws one explosive potion from backpack.
 *
 * Passive (no hook, checked inline via HasAbility):
 * - Reduces reagent costs by 50% [alchemybook.src, dyebook.src]
 */
class TriggerEngineering(CombatAbility)

	function TriggerEngineering(this)
		Super(ABILITY_ID::ENGENHARIA_DE_GATILHOS);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		if (mobile.IsNPC())
			return _NpcThrowExplosive(who);
		endif

		start_script(":alchemy:explosiveConfig", { who });
		return true;
	endfunction

endclass

function _NpcThrowExplosive(who)
	var explosive_potions := array{0xbf13, 0xbf14, 0xbf15, 0xef60, 0xef61, 0xef62};
	foreach item in EnumerateItemsInContainer(who.backpack)
		if (item.objtype in explosive_potions)
			UseItem(item, who);
			return true;
		endif
	endforeach
	return false;
endfunction
