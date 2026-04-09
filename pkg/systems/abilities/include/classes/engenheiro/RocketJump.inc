use uo;
use os;
use vitals;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include "include/utils";
include "include/epicCombatUtils";

class RocketJump(CombatAbility)

	function RocketJump(this)
		Super(ABILITY_ID::STEAM_JUMP);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (!GetObjProperty(who, "pilotando"))
			var legs := GetEquipmentByLayer(who, LAYER_PANTS);
			if (!legs || legs.objtype != 0xFFC2)
				Message("Você precisa estar usando as Grevas de Vapor para ativar os Propulsores!").SendSys(who);
				return false;
			endif
		endif
		if (GetObjProperty(who, "pilotando"))
			var golem := _RJ_GetGolemRef(who);
			if (!golem || CInt(GetObjProperty(golem, "golem_coal")) < 2)
				Message("O golem precisa de pelo menos 2 carvoes!").SendSys(who);
				return false;
			endif
		else
			var coal := FindObjtypeInContainer(who.backpack, 0x1bde);
			if (!coal || coal.amount < 2)
				Message("Voce precisa de pelo menos 2 carvoes para acionar os propulsores!").SendSys(who);
				return false;
			endif
		endif
		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var is_piloting := GetObjProperty(who, "pilotando");

		// Consume coal
		if (is_piloting)
			var golem := _RJ_GetGolemRef(who);
			var stored_coal := CInt(GetObjProperty(golem, "golem_coal"));
			SetObjProperty(golem, "golem_coal", stored_coal - 2);
		else
			var coal := FindObjtypeInContainer(who.backpack, 0x1bde);
			SubtractAmount(coal, 2);
		endif

		// Launch movement as separate script so network packets flush properly
		var pid := Start_Script(":abilities:scripts/Ativa/OnUse/rocketjump", {who});
		sleepms(1500);

		// Powder check
		var has_powder := 0;
		if (is_piloting)
			var golem := _RJ_GetGolemRef(who);
			var stored_powder := CInt(GetObjProperty(golem, "golem_powder"));
			if (golem && stored_powder >= 5)
				SetObjProperty(golem, "golem_powder", stored_powder - 5);
				has_powder := 1;
			endif
		else
			var powder := FindObjtypeInContainer(who.backpack, 0x9092);
			if (powder && powder.amount >= 5)
				SubtractAmount(powder, 5);
				has_powder := 1;
			endif
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

		return true;
	endfunction

endclass

function _RJ_GetGolemRef(who)
	var serial := GetObjProperty(who, "lastpossessedserial");
	if (!serial)
		return 0;
	endif
	return SystemFindObjectBySerial(serial);
endfunction
