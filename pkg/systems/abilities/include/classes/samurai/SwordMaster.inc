use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Mestre da Espada / Sword Master - ABILITY_ID::SWORD_MASTER (804)
 *
 * Passive ON_PARRY: Counter-attacks whenever the defender successfully parries.
 */
class SwordMaster(CombatAbility)

	function SwordMaster(this)
		Super(ABILITY_ID::SWORD_MASTER);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_PARRY;
	endfunction

	function OnPassiveOnParry(this, attackInfo)
		var defender := attackInfo.GetDefender().GetObjRef();
		var attacker := attackInfo.GetAttacker().GetObjRef();

		defender.attack_once(attacker);

		Message("*contra-ataca!*").SendAbove(defender);

		DebugMsg(
			defender,
			DEBUG_HAB,
			Message("[SWORD_MASTER] Contra-ataque!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
