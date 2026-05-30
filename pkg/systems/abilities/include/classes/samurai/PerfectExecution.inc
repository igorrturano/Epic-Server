use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":hotbar:custombuff";

/**
 * Execucao Perfeita / Perfect Execution - ABILITY_ID::PERFECT_EXECUTION (802)
 *
 * SingleUse BEFORE_DAMAGE: Guarantees next hit lands and forces it as a critical.
 * Sets #GuaranteedHit for HitController, forces critical via attackInfo in OnBeforeDamage.
 */
class PerfectExecution(CombatAbility)

	function PerfectExecution(this)
		Super(ABILITY_ID::PERFECT_EXECUTION);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
		this.SingleUse := true;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(TACTICS) / 10);
		if (duration < 2)
			duration := 2;
		endif
		return duration;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (who.shield)
			Message("Um verdadeiro samurai nao usaria escudo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();

		// Guaranteed hit for HitController (no V3 before-hit hook exists)
		who.SetProperty("#GuaranteedHit", 1);

		Message("*prepara um golpe perfeito*").SendAbove(who);
		Message("Voce prepara um golpe perfeito.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		var duration := this.GetDurationFor(mobile);
		SendAbilityBuff(who, ABILITY_ID::PERFECT_EXECUTION, duration, "Critico garantido");
	endfunction

	function OnBeforeDamage(this, attackInfo)
		// Force critical — CalculateAttackerCriticalHit skips roll since IsCritical() is already true
		attackInfo.GetDamage().SetCritical(true);

		DebugMsg(
			attackInfo.GetAttacker().GetObjRef(),
			DEBUG_HAB,
			Message("[PERFECT_EXECUTION] Critico forcado!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
