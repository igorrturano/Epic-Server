use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";
include ":attributes:attributes";
include ":hotbar:custombuff";

/**
 * Shintai to Kokoro / Corpo e Mente (Body and Mind) - ABILITY_ID::BODY_AND_MIND (704)
 *
 * Active (OnUse): Starts HP recovery timed script for Tactics seconds.
 *
 * Passive +5 all resistances stays in intrinsicMods.src (outside combat).
 */
class BodyAndMind(CombatAbility)

	function BodyAndMind(this)
		Super(ABILITY_ID::BODY_AND_MIND);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var duration := CInt(mobile.GetSkill(TACTICS));
		if (duration < 5)
			duration := 5;
		endif

		TS_StartTimer(who, "recuperacaoaligera", duration);
		Message($"Corpo e mente em harmonia por {duration}s.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		SendAbilityBuff(who, ABILITY_ID::BODY_AND_MIND, duration, "Recuperacao de HP");

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[BODY_AND_MIND] Recuperacao HP por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
