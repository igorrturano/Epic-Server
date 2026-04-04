use os;
use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":combat:Controller/HitController";
include "include/facings";
include "include/utils";
include "include/client";
include "include/epicCombatUtils";

/**
 * Assassinar (Assassinate) - ABILITY_ID::ASSASSINAR (1200)
 *
 * OnUse: While hidden, leaps behind a target (range = SNEAK/5) and
 * prepares a backstab. Guarantees hit if hidden, critical if has
 * Instinto Assassino.
 */
class Assassinate(CombatAbility)

	function Assassinate(this)
		Super(ABILITY_ID::ASSASSINAR);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		if (!who.hidden)
			Message("Você precisa estar furtivo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		elseif (who.frozen || who.paralyzed)
			Message("Você não consegue se mover.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		if (!CanBackStab(mobile) || !isAssassinWeapon(who))
			Message("Esta arma não é apropriada para um ataque furtivo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		var range := CInt(AP_GetSkill(who, SNEAK) / 5);

		var targ := Target(who);
		if (!targ)
			return false;
		elseif (!targ.isA(POLCLASS_MOBILE))
			Message("Alvo inválido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		elseif (Distance(who, targ) > range || abs(who.z - targ.z) > 19)
			Message("Alvo muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		elseif (!CheckLineOfSight(who, targ))
			Message("Alvo fora da linha de visão.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		// Move in line to target
		var coords := GetCoordsInLine(who.x, who.y, targ.x, targ.y);
		coords.Erase(coords.size());
		foreach coord in coords
			MoveObjectToLocation(who, coord.x, coord.y, targ.z, who.realm, MOVEOBJECT_FORCELOCATION);
			sleepms(50);
		endforeach

		// Position behind target
		var behind := InverseFacing(targ.facing);
		behind := GetModByFacing(behind);
		if (CanWalk("L", who.x, who.y, who.z, targ.x + behind[1], targ.y + behind[2]))
			MoveObjectToLocation(who, targ.x + behind[1], targ.y + behind[2], targ.z, who.realm, MOVEOBJECT_FORCELOCATION);
		else
			var radius := GetPointsInRadius(targ.x, targ.y, 1);
			var random := radius.randomentry();
			while (!MoveObjectToLocation(who, random.x, random.y, targ.z, who.realm))
				random := radius.randomentry();
				sleep(2);
			endwhile
		endif

		who.SetFacing(GetFacing(who.x, who.y, targ.x, targ.y));

		// Prepare backstab
		mobile.SetProperty("backstab", 1);
		mobile.GetEquipBySlot(WEAPON).SetProperty("backstab", 1);

		if (mobile.IsHidden())
			mobile.SetProperty("#GuaranteedHit", 1);
			if (mobile.HasAbility(ABILITY_ID::INSTINTO_ASSASSINO))
				mobile.SetProperty("#nextcritical", 1);
			endif
		endif

		GrantPrivilege(who, "hiddenattack");
		who.enable("hiddenattack");

		return true;
	endfunction

endclass
