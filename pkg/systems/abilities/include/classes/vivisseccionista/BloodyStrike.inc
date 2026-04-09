include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";
include ":attributes:attributes";

/**
 * Golpe Sangrento (Bloody Strike) - ABILITY_ID::GOLPE_SANGRENTO (1401)
 *
 * Active (SingleUse AFTER_DAMAGE): Next hit causes bleed for ALCHEMY/10 seconds.
 * Follows CrabStyle bleed pattern.
 */
class BloodyStrike(CombatAbility)

	function BloodyStrike(this)
		Super(ABILITY_ID::GOLPE_SANGRENTO);
		this.ActivationMoment := ABILITY_ACTIVATION::AFTER_DAMAGE;
		this.SingleUse := true;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(ALCHEMY) / 10);
		if (duration < 2)
			duration := 2;
		endif
		return duration;
	endfunction

	function OnActivate(this, mobile)
		Message("*prepara um ataque minucioso*").SendAbove(mobile.GetObjRef());
	endfunction

	function OnAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var defender := attackInfo.GetDefender();

		var bleedDuration := CInt(attacker.GetSkill(ALCHEMY) / 10);
		if (bleedDuration < 3)
			bleedDuration := 3;
		endif

		TS_StartTimer(defender.GetObjRef(), "bleed", bleedDuration);

		Message("*faz um corte profundo!*").SendAbove(attacker.GetObjRef());

		DebugMsg(
			attacker.GetObjRef(),
			DEBUG_HAB,
			Message($"[GOLPE_SANGRENTO] Sangramento: {bleedDuration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
