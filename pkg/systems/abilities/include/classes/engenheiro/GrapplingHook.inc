use uo;
use os;
use util;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":fls_core:packets";

class GrapplingHook(CombatAbility)

	function GrapplingHook(this)
		Super(ABILITY_ID::GRAPPLING_HOOK);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var targ := Target(who);
		if (!targ)
			Message("Cancelado.").SendSys(who);
			return false;
		endif

		if (!targ.isA(POLCLASS_MOBILE))
			Message("Alvo invalido.").SendSys(who);
			return false;
		endif

		var distance := _ChebyshevDist(who.x, who.y, targ.x, targ.y);
		if (distance > 10)
			Message("Alvo muito distante! Alcance maximo: 10 tiles.").SendSys(who);
			return false;
		endif

		if (distance < 2)
			Message("Alvo muito proximo!").SendSys(who);
			return false;
		endif

		var auto_hit := mobile.HasAbility(ABILITY_ID::TARGETING_VISOR);

		if (!auto_hit)
			var difficulty := CInt(mobile.GetSkill(METALWORKING) / 5);
			if (difficulty < 1)
				difficulty := 1;
			endif
			var resisted := _ReflexSave(targ, difficulty);
			if (resisted == 1)
				Message("*O alvo desvia do gancho!*").SendAbove(who);
				PlaySoundEffect(who, 569);
				DebugMsg(
					who,
					DEBUG_HAB,
					Message($"[GRAPPLING_HOOK] Alvo resistiu (dif: {difficulty})", Color(SYSTEM_COLOR::FAIL))
				);
				return true;
			endif
		endif

		Message("*Dispara o gancho mecanico!*").SendAbove(who);
		PlayMovingEffect(who, targ, 0x55A7, 5, 1, 0);
		sleep(1);

		TS_StartTimer(targ, "paralysis", distance);

		Message("*Enganchado!*").SendAbove(targ);
		PlaySoundEffect(who, 0x523);

		var coords := GetCoordsInLine(targ.x, targ.y, who.x, who.y);
		foreach coord in coords
			if (coord.x == targ.x && coord.y == targ.y)
				continue;
			endif
			if (coord.x == who.x && coord.y == who.y)
				break;
			endif
			var err := MoveObjectToLocation(targ, coord.x, coord.y, targ.z, targ.realm, MOVEOBJECT_FORCELOCATION);
			if (err == error)
				Message("*O alvo bate em um obstaculo!*").SendAbove(targ);
				break;
			endif
			sleepms(150);
		endforeach

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[GRAPPLING_HOOK] Alvo puxado! Distancia: {distance}, Paralisia: {distance}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _ReflexSave(defender, difficulty)
	var dado := RandomDiceRoll("1d20");
	var valor := AP_GetStat(defender, REFLEXES);
	var chance := dado + valor;

	if (dado == 1)
		return -1;
	elseif (dado == 20)
		return 1;
	endif

	if (chance >= difficulty)
		return 1;
	endif

	return 0;
endfunction

function _ChebyshevDist(x1, y1, x2, y2)
	var xd := x1 - x2;
	var yd := y1 - y2;
	if (xd < 0)
		xd := -xd;
	endif
	if (yd < 0)
		yd := -yd;
	endif
	if (xd > yd)
		return xd;
	else
		return yd;
	endif
endfunction