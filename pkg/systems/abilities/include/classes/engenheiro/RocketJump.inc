use uo;
use os;
use vitals;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include "include/facings";
include "include/utils";
include "include/epicCombatUtils";

class RocketJump(CombatAbility)

	function RocketJump(this)
		Super(ABILITY_ID::STEAM_JUMP);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var coal := FindObjtypeInContainer(who.backpack, 0x1bde);
		if (!coal || coal.amount < 2)
			Message("Voce precisa de pelo menos 2 carvoes para acionar os propulsores!").SendSys(who);
			return false;
		endif
		SubtractAmount(coal, 2);

		who.frozen := 1;
		PerformAction(who, 12);
		Message("*Aciona os propulsores*").SendAbove(who);

		var world_z := GetStandingHeight(who.x, who.y, who.z + 15, who.realm);
		var moved := 0;
		if (world_z.z <= who.z + 1)
			moved := 1;
			MoveObjectToLocation(who, who.x, who.y, who.z + 15, who.realm, MOVEOBJECT_FORCELOCATION);
		endif
		sleepms(100);
		_RocketPushForward(who, 5, 50);

		sleepms(200);
		if (moved)
			MoveObjectToLocation(who, who.x, who.y, who.z - 15, who.realm);
		endif
		sleepms(500);

		var has_powder := 0;
		var powder := FindObjtypeInContainer(who.backpack, 0x9092);
		if (powder && powder.amount >= 5)
			SubtractAmount(powder, 5);
			has_powder := 1;
		endif

		if (has_powder)
			PlayStationaryEffect(who.x + 2, who.y + 2, who.z, 0x6C9B, 0, 10, 0, who.realm);
			PlaySoundEffect(who, 0x221);

			var duration := CInt(mobile.GetSkill(METALWORKING) / 10);
			if (duration < 1)
				duration := 1;
			endif
			var dano := CInt(mobile.GetSkill(METALWORKING) / 2);

			sleepms(500);

			var festa := who.party;
			var allPets := array{};
			if (who.party)
				foreach member in (who.party.members)
					if (member != who)
						var memberObj := SystemFindObjectBySerial(member.serial);
						var petsNear := ListAllPetsAndSummonedPetsNear(memberObj, 8);
						foreach pet in petsNear
							allPets.append(pet);
						endforeach
					endif
				endforeach
			endif

			var nearby := ListMobilesNearLocation(who.x, who.y, who.z, 2, who.realm);
			foreach char in nearby
				var char_owner := GetObjProperty(char, "owner");
				if (!(char in (festa.members) || char == who))
					if (!(char in allPets) && !(char in who.party.members) && !(char_owner == who.serial))
						PlayObjectCenteredEffect(char, 0x6d67, 0, 10);
						TS_StartTimer(char, "tombar", duration);
						TS_StartTimer(char, "lentidao", duration * 2);
						TS_StartTimer(char, "paralysis", duration);
						ApplyRawDamage(char, dano);
					endif
				endif
			endforeach

			DebugMsg(
				who,
				DEBUG_HAB,
				Message($"[STEAM_JUMP] Explosao! Dano: {dano}, Duracao: {duration}s", Color(SYSTEM_COLOR::SUCCESS))
			);
		else
			Message("*Salta sem explosao*").SendAbove(who);
			DebugMsg(
				who,
				DEBUG_HAB,
				Message("[STEAM_JUMP] Salto sem polvora", Color(SYSTEM_COLOR::INFO))
			);
		endif

		who.frozen := 0;
		return true;
	endfunction

endclass

function _RocketPushForward(who, dist, delay)
	var mod := GetModByFacing(who.facing);
	var x := mod[1];
	var y := mod[2];

	var finalx := who.x + (dist * x);
	var finaly := who.y + (dist * y);
	var world_z := GetStandingHeight(finalx, finaly, who.z - 15, who.realm);
	var force := _ifElse(world_z.z != error && world_z.z <= who.z, MOVEOBJECT_FORCELOCATION,
					  MOVEOBJECT_NORMAL);
	for i := 1 to dist
		finalx := (who.x + x);
		finaly := (who.y + y);
		if (!CheckLosAt(who, finalx, finaly, who.z))
			return 0;
		endif
		world_z := GetStandingHeight(who.x, who.y, who.z, who.realm);
		if (world_z.z > who.z)
			return 0;
		endif

		var err := MoveObjectToLocation(who, finalx, finaly, who.z, who.realm, force);
		if (err == error)
			return 0;
		endif
		sleepms(delay);
	endfor

	return 1;
endfunction
