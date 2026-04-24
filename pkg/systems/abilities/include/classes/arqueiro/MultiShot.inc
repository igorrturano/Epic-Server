use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Disparo Multiplo (Multi Shot) - ABILITY_ID::DISPARO_MULTIPLO (5)
 *
 * OnUse: Fire TT/40 + 1 rapid shots at current opponent.
 * Requires ranged weapon.
 */
class MultiShot(CombatAbility)

	function MultiShot(this)
		Super(ABILITY_ID::DISPARO_MULTIPLO);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		var weapon := mobile.GetEquipBySlot(WEAPON);

		if (!weapon || weapon.GetAttribute() != RANGED)
			Message("Voce nao pode usar essa habilidade com essa arma.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var shotRounds := CInt(mobile.GetSkill(TACTICS) / 40);

		Message("*seus dedos deslizam pelo arco*").SendAbove(who);

		for i := 0 to shotRounds
			who.attack_once();
			sleepms(50);
		endfor

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[DISPARO_MULTIPLO] Executou {shotRounds + 1} disparos!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
