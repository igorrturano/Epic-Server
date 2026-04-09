use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Sinfonia Final (Final Symphony) - ABILITY_ID::FINAL_SYMPHONY (1712)
 *
 * Active (ON_USE) - 5 minute cooldown:
 * - Next cantico/composition has DOUBLED effect
 * - Can mix scales in same composition
 * - If entropy triggers: bard CHOOSES event from table
 */
const FINAL_SYMPHONY_COOLDOWN := 300; // 5 minutes in seconds

class FinalSymphony(CombatAbility)

	function FinalSymphony(this)
		Super(ABILITY_ID::FINAL_SYMPHONY);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		var cooldownEnd := CInt(who.GetProp("#bard_final_symphony_cd"));
		if (cooldownEnd > ReadGameClock())
			var remaining := cooldownEnd - ReadGameClock();
			Message($"Sinfonia Final em recarga. Aguarde {remaining}s.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		who.SetProp("#bard_final_symphony_active", 1);
		who.SetProp("#bard_final_symphony_cd", ReadGameClock() + FINAL_SYMPHONY_COOLDOWN);

		Message("*A realidade vibra ao ritmo da Sinfonia Final...*").SendAbove(who);
		Message("Proximo cantico ou composicao tera efeito dobrado!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[FINAL_SYMPHONY] Ativada. Proximo cantico dobrado.", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass