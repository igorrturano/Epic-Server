use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":timedScripts:timedScripts";

/**
 * Corrida Furtiva (Stealth Run) - ABILITY_ID::STEALTH_RUN (1303)
 *
 * Passive: Allows hiding while being attacked (checked in stealth.src).
 * Active (OnUse): Conceals the caster for Sneak/5 seconds, allowing
 * them to run while hidden.
 */
class StealthRun(CombatAbility)

	function StealthRun(this)
		Super(ABILITY_ID::STEALTH_RUN);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var sneakSkill := CInt(mobile.GetSkill(SNEAK));

		var duration := CInt(sneakSkill / 5);
		if (duration < 2)
			duration := 2;
		endif

		who.hidden := 1;
		who.concealed := 1;

		Message("*Desaparece nas sombras!*").SendEmote(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[STEALTH_RUN] Concealed por {duration}s (Sneak: {sneakSkill})", Color(SYSTEM_COLOR::SUCCESS))
		);

		// Concealment auto-clears when the "conceal" timer expires (fimconceal subscript).
		// ExecuteOn returns immediately so the brain script doesn't block.
		TS_StartTimer(who, "conceal", duration);

		return true;
	endfunction

endclass
