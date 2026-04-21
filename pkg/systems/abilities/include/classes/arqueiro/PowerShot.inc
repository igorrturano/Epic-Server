use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";

/**
 * Tiro Poderoso (Power Shot) - ABILITY_ID::TIRO_PODEROSO (1)
 *
 * OnUse: Player chooses a direction, fires a piercing shot that hits
 * all valid targets along a line within weapon range.
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
			var opponent := who.opponent;
			if (!opponent)
				opponent := GetRandomNearbyHostile(who);
			endif
			if (!opponent)
				return false;
			endif
			targ := struct{
				x := opponent.x,
				y := opponent.y,
				z := opponent.z,
				realm := opponent.realm
			};
		else
			Message("Escolha a direcao que ira disparar a flecha", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			targ := TargetCoordinates(who);
		endif

		if (!targ)
			Message("Alvo cancelado.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			return false;
		endif

		if (!CheckLOSAt(who, targ.x, targ.y, targ.z))
			Message("O alvo nao esta visivel para voce.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (CoordinateDistance(who.x, who.y, targ.x, targ.y) > weapon.GetMaxRange())
			Message("O alvo esta muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		var targetsInLine := _GetTargetsInLine(who, targ.x, targ.y);
		if (targetsInLine.size() == 0)
			Message("Nao ha alvos na linha de tiro.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			return false;
		endif

		var dmg := CInt(mobile.GetSkill(RANGED) / 5);

		foreach target in targetsInLine
			sleepms(50);
			ApplyRawDamage(target, dmg);
		endforeach

		Message($"*tiro poderoso em linha - {targetsInLine.size()} alvos*").SendAbove(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[TIRO_PODEROSO] Atingiu {targetsInLine.size()} alvos em linha", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _GetTargetsInLine(who, targetX, targetY)
	var validTargets := array{};
	var alreadyTargeted := array{};

	var lineCoords := GetCoordsInLine(who.x, who.y, targetX, targetY);

	foreach point in lineCoords
		var mobiles := ListMobilesNearLocation(point.x, point.y, who.z, 1, who.realm);

		foreach mobile in mobiles
			if (mobile.serial in alreadyTargeted)
				continue;
			endif

			if (mobile == who)
				continue;
			endif

			if (who.party && mobile in who.party.members)
				continue;
			endif

			if (GetObjProperty(mobile, "owner") == who.serial)
				continue;
			endif

			if (who.party)
				var skipPet := 0;
				foreach member in (who.party.members)
					if (GetObjProperty(mobile, "owner") == member.serial)
						skipPet := 1;
						break;
					endif
				endforeach
				if (skipPet)
					continue;
				endif
			endif

			validTargets.append(mobile);
			alreadyTargeted.append(mobile.serial);
		endforeach
	endforeach

	return validTargets;
endfunction
