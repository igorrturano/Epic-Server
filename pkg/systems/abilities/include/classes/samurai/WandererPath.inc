use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";

/**
 * Caminho do Andarilho / Wanderer's Path - ABILITY_ID::WANDERER_PATH (800)
 *
 * Active (OnUse): Guarantees a parry defense by setting temp parry bonus
 * and resetting parry delay.
 *
 * Passive (ParryController, unchanged): +30% parry when blocking with weapon
 * instead of shield.
 */
class WandererPath(CombatAbility)

	function WandererPath(this)
		Super(ABILITY_ID::WANDERER_PATH);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (who.shield)
			Message("Um verdadeiro samurai nao usaria escudo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		mobile.SetProperty("#tempparrybonus", 100);
		SetCooldown(who, "parrydelay", 0);

		Message("*prepara a lamina para defender*").SendAbove(who);
		Message("Voce garante uma defesa com sua lamina.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[WANDERER_PATH] Defesa garantida", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
