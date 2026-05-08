use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";

/**
 * Tiro Poderoso (Power Shot) - ABILITY_ID::TIRO_PODEROSO (1)
 *
 * OnUse: Guaranteed ranged shot that paralyzes the target for Ranged/10 seconds.
 */
class PowerShot(CombatAbility)

	function PowerShot(this)
		Super(ABILITY_ID::TIRO_PODEROSO);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		var weapon := mobile.GetEquipBySlot(WEAPON);

		if (!weapon || weapon.GetAttribute() != RANGED)
			Message("Voce precisa estar usando uma arma de longo alcance.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var weapon := mobile.GetEquipBySlot(WEAPON);

		SetScriptController(who);

		var targ;
		if (mobile.IsNPC())
			targ := who.opponent;
			if (!targ)
				targ := GetRandomNearbyHostile(who);
			endif
		else
			Message("Escolha o alvo do seu tiro.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			targ := Target(who);
		endif

		if (!targ)
			Message("Alvo cancelado.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			return false;
		endif

		if (!targ.isA(POLCLASS_MOBILE))
			Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (CoordinateDistance(who.x, who.y, targ.x, targ.y) > weapon.GetMaxRange())
			Message("O alvo esta muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (!CheckLineOfSight(who, targ))
			Message("O alvo nao esta visivel para voce.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		var duration := CInt(mobile.GetSkill(RANGED) / 10);
		if (duration < 1)
			duration := 1;
		endif

		SetObjProperty(who, "#GuaranteedHit", 1);
		who.attack_once(targ);

		TS_StartTimer(targ, "paralysis", duration);
		Message("*tiro poderoso*").SendAbove(who);
		Message("*paralisado*").SendAbove(targ);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[TIRO_PODEROSO] Paralisou por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
