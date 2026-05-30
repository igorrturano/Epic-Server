include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":timedScripts:timedScripts";

/**
 * Fantasma (Ghost) - ABILITY_ID::FANTASMA (1004)
 *
 * OnUse: Starts "ghost" timed script for 60 seconds.
 * The timed script sets #fantasma property, making the player invisible to hostile creatures.
 * End script erases the property and notifies the player.
 */
class Ghost(CombatAbility)

	function Ghost(this)
		Super(ABILITY_ID::FANTASMA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		TS_StartTimer(who, "ghost", 60);

		Message("*desaparece nas sombras*").SendAbove(who);
		Message("Você virou um fantasma!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);

		return true;
	endfunction

endclass
