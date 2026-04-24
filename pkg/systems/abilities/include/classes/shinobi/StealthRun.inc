use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Corrida Furtiva (Stealth Run) - ABILITY_ID::STEALTH_RUN (1303)
 *
 * Passive: Allows hiding while being attacked (checked in sneak.src).
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

		var expires := ReadGameClock() + duration;
		while (ReadGameClock() < expires)
			if (who.warmode)
				break;
			endif
			sleepms(500);
		endwhile

		if (who.concealed)
			who.concealed := 0;
		endif

		return true;
	endfunction

endclass
