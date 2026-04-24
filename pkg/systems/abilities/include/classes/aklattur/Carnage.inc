use uo;

include ":abilities:models/CombatAbility";
include ":abilities:AbilityRegistry";
include ":abilities:controllers/AbilityMonitor";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Carnificina (Carnage) - ABILITY_ID::CARNIFICINA (504)
 *
 * OnKill: Each kill activates/refreshes Desert Fury, Blood for Blood,
 * and Prey Mark for 20 seconds each. Keep killing to maintain the chain!
 * Duration: 60s window.
 */
class Carnage(CombatAbility)

	function Carnage(this)
		Super(ABILITY_ID::CARNIFICINA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_KILL;
	endfunction

	function GetDurationFor(this, mobile)
		return 60;
	endfunction

	function OnKill(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		var abilitiesToActivate := array{
			ABILITY_ID::FURIA_DO_DESERTO,
			ABILITY_ID::SANGUE_POR_SANGUE,
			ABILITY_ID::MARCA_DA_PRESA
		};

		foreach abilityId in (abilitiesToActivate)
			var ability := CreateAbility(abilityId);
			if (!ability.errortext)
				ability.SetDuration(20);
				var monitor := AbilityMonitor(ability);
				monitor.ActivateOn(attacker);
			endif
		endforeach

		Message("*CARNIFICINA!*").SendAbove(who);
		PlaySoundEffect(who, 0x1F7);
		PlayObjectCenteredEffect(who, 0x3728, 1, 13);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[CARNIFICINA] Ativou Furia + Sangue + Marca por 20s!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
