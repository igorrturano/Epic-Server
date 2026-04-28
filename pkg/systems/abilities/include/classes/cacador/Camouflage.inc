use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Camuflagem (Camouflage) - ABILITY_ID::CAMUFLAGEM (100)
 *
 * OnUse: Hides the player's nearby pet. Pet's next attack will be critical.
 * Requires player to be hidden. Uses Survival skill for stealth steps.
 * If player has Espreita Selvagem, pet stealth steps are doubled.
 */
class Camouflage(CombatAbility)

	function Camouflage(this)
		Super(ABILITY_ID::CAMUFLAGEM);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (!who.hidden)
			Message("Você não está escondido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var nearby := mobile.GetNearbyMobiles(5, LISTEX_FLAG_NORMAL, 1);
		var skill_bonus := CInt(mobile.GetSkill(SURVIVAL) / 5);

		foreach mob in nearby
			var mobWrapper := MobileWrapper(mob);
			var mobOwner := mobWrapper.GetProperty("owner");

			if (mob.master.serial == who.serial || mobOwner == who.serial)
				if (mob.hidden)
					Message("Seu animal já está escondido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif

				GrantPrivilege(mob, "hiddenattack");
				mob.enable("hiddenattack");
				GrantPrivilege(mob, "runwhilestealth");
				mob.enable("runwhilestealth");

				mob.hidden := 1;
				mob.stealthsteps := skill_bonus;
				mobWrapper.SetProperty("#nextcritical", 1);

				if (mobile.HasAbility(ABILITY_ID::ESPREITA_SELVAGEM))
					mob.stealthsteps := CInt(skill_bonus * 2);
				endif

				Message("Você escondeu seu animal.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

				while (mob.hidden)
					_ExibeCharEscondido(who, mob, 2766);
					sleep(1);
				endwhile

				DebugMsg(who, DEBUG_HAB, Message("[CAMUFLAGEM] Pet escondido!", Color(SYSTEM_COLOR::SUCCESS)));
				return true;
			endif
		endforeach

		Message("Nenhum animal encontrado próximo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endfunction

endclass

function _ExibeCharEscondido(who, escondido, cor)
	_DeleteObject(who, escondido);
	_DrawObject(who, escondido, escondido.graphic, cor);
endfunction

function _ConvertIntToHex(astring, alength)
	if (astring < 0)
		astring := (1 << (alength * 8)) + astring;
	endif

	astring := Hex(astring);
	astring := CStr(astring);
	if (astring["0x"])
		astring := astring[3, len(astring)];
	endif
	if (len(astring) > alength * 2)
		return "";
	endif
	while (len(astring) < alength * 2)
		astring := "0" + astring;
	endwhile
	return astring;
endfunction

function _DrawObject(ktowhom, kobj, kgraphic, kcolor)
	var PacketHexStringPart1 := "78";
	var PacketHexStringPart2 := _ConvertIntToHex(kobj.serial, 4) + _ConvertIntToHex(kgraphic, 2);
	PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(kobj.x, 2);
	PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(kobj.y, 2);
	PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(kobj.z, 1);
	PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(kobj.facing, 1);
	PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(kcolor, 2);
	PacketHexStringPart2 := PacketHexStringPart2 + "0000";
	foreach item in ListEquippedItems(kobj)
		PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(item.serial, 4);
		PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(item.graphic, 2);
		PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(item.layer, 1);
		PacketHexStringPart2 := PacketHexStringPart2 + _ConvertIntToHex(kcolor, 2);
	endforeach
	PacketHexStringPart2 := PacketHexStringPart2 + "00000000";
	var FullPacketHexStringLength := (len(PacketHexStringPart1) + len(PacketHexStringPart2)) / 2 + 2;
	var FullPacketHexString := PacketHexStringPart1 + _ConvertIntToHex(FullPacketHexStringLength, 2) + PacketHexStringPart2;
	SendPacket(ktowhom, FullPacketHexString);
endfunction

function _DeleteObject(who, lobj)
	var PacketHexString := "1D" + _ConvertIntToHex(lobj.serial, 4);
	SendPacket(who, PacketHexString);
endfunction