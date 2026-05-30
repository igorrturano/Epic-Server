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
 * Active (OnUse): inicia timer de regen de HP ("recuperacaoaligera") por Tactics
 * segundos. Requer furia ativa.
 *
 * (Pos-Phase 7) A absorcao passiva foi removida — UnarmoredCombat passou a ser
 * a unica fonte de reducao de dano para builds nu/sem armor pesada.
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

endclass
