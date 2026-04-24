use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":timedScripts:timedScripts";
include ":attributes:attributes";
include ":hotbar:custombuff";

/**
 * Resiliencia Barbara (Barbarian Resilience) - ABILITY_ID::RESILIENCIA_BARBARA (601)
 *
 * Active (OnUse): Starts HP regen timed script ("recuperacaoaligera") for Tactics seconds.
 * Requires fury active.
 *
 * Passive (DefenderBeforeDamage): Reduces incoming physical damage by Tactics/10,
 * but only when in fury AND no armor covers the hit body region (barbarian fights unarmored).
 */
class BarbarianResilience(CombatAbility)

	function BarbarianResilience(this)
		Super(ABILITY_ID::RESILIENCIA_BARBARA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (!GetCooldown(who, "furia"))
			Message("Voce precisa estar em furia.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var duration := CInt(mobile.GetSkill(TACTICS));
		if (duration < 5)
			duration := 5;
		endif

		TS_StartTimer(who, "recuperacaoaligera", duration);
		SendAbilityBuff(who, ABILITY_ID::RESILIENCIA_BARBARA, duration, "Recuperacao de HP");
		Message($"Resiliencia Barbara ativada por {duration}s.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[RESILIENCIA_BARBARA] Recuperacao HP por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var defender := attackInfo.GetDefender();
		var who := defender.GetObjRef();

		if (!GetCooldown(who, "furia"))
			return false;
		endif

		var reduction := CInt(defender.GetSkill(TACTICS) / 20);
		if (reduction <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.DecreaseSlashBaseDamage(reduction)
			.DecreasePiercingBaseDamage(reduction)
			.DecreaseBluntBaseDamage(reduction);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[RESILIENCIA_BARBARA] Reducao passiva: -{reduction}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
