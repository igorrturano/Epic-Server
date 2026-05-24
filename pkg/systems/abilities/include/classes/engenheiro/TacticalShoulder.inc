use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include "include/utils";
include "include/epicCombatUtils";

const STEAMARMS_OBJTYPE := 0xFFC3;
const HANDCANNON_OBJTYPE := 0xFFC7;
const HARPOON_OBJTYPE := 0xFFC6;
const POWDERTRAP_OBJTYPE := 0x2808;
const CANNONBALL_OBJTYPE := 0x4224;
const GUNPOWDER_OBJTYPE := 0x9092;

class TacticalShoulder(CombatAbility)

	function TacticalShoulder(this)
		Super(ABILITY_ID::TACTICAL_SHOULDER);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		var gadget;

		if (GetObjProperty(who, "pilotando"))
			var golem := _TS_GetGolemRef(who);
			if (!golem)
				Message("Golem nao encontrado!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif
			gadget := GetObjProperty(golem, "InstalledGadget");
			if (!gadget)
				Message("Nenhum gadget instalado no golem. Clique duas vezes no golem para instalar.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif

			case (gadget)
				"handcannon":
					if (CInt(GetObjProperty(golem, "golem_cannonball")) < 1)
						Message("O golem precisa de balas de canhao!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
					if (CInt(GetObjProperty(golem, "golem_powder")) < 5)
						Message("O golem precisa de pelo menos 5 polvoras!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
				"powdertrap":
					if (CInt(GetObjProperty(golem, "golem_powder")) < 5)
						Message("O golem precisa de polvora para a armadilha!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
			endcase
		else
			var steamArms := _GetSteamArms(who);
			if (!steamArms)
				Message("Voce precisa estar usando os Bracos a Vapor.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif

			gadget := GetObjProperty(steamArms, "InstalledGadget");
			if (!gadget)
				Message("Nenhum gadget instalado na ombreira. Clique duas vezes nos Bracos a Vapor para instalar.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif

			case (gadget)
				"handcannon":
					var ball := FindObjtypeInContainer(who.backpack, CANNONBALL_OBJTYPE);
					if (!ball || ball.amount < 1)
						Message("Voce precisa de uma bala de canhao!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
					var powder := FindObjtypeInContainer(who.backpack, GUNPOWDER_OBJTYPE);
					if (!powder || powder.amount < 5)
						Message("Voce precisa de pelo menos 5 polvoras!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
				"powdertrap":
					var trap := FindObjtypeInContainer(who.backpack, POWDERTRAP_OBJTYPE);
					if (!trap)
						Message("Voce precisa de uma armadilha de polvora na mochila!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
						return false;
					endif
			endcase
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		// Equipment check
		if (!GetObjProperty(who, "pilotando"))
			var arms := GetEquipmentByLayer(who, LAYER_ARMS);
			if (!arms || arms.objtype != STEAMARMS_OBJTYPE) // Steam Arms (0xFFC3)
				Message("Você precisa estar usando os Braços a Vapor para ativar a Ombreira Tática!").SendSys(who);
				return false;
			endif
		endif

		var gadget;

		if (GetObjProperty(who, "pilotando"))
			var golem := _TS_GetGolemRef(who);
			gadget := GetObjProperty(golem, "InstalledGadget");
		else
			var steamArms := _GetSteamArms(who);
			gadget := GetObjProperty(steamArms, "InstalledGadget");
		endif

		case (gadget)
			"handcannon":
				return _FireHandcannon(mobile, who);
			"harpoon":
				return _FireHarpoon(mobile, who);
			"powdertrap":
				return _DeployPowderTrap(mobile, who);
			default:
				Message("Gadget desconhecido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
		endcase
	endfunction

endclass

function _FireHandcannon(mobile, who)
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
		Message("Escolha o alvo do disparo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		targ := TargetCoordinates(who);
	endif

	if (!targ)
		Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (!CheckLOSAt(who, targ.x, targ.y, targ.z))
		Message("Alvo nao visivel.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	var dist := _ChebyshevDist(who.x, who.y, targ.x, targ.y);
	if (dist > 8)
		Message("Alvo muito distante! Alcance maximo: 8 tiles.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	// Consume ammo after validation
	if (GetObjProperty(who, "pilotando"))
		var golem := _TS_GetGolemRef(who);
		var stored_balls := CInt(GetObjProperty(golem, "golem_cannonball"));
		SetObjProperty(golem, "golem_cannonball", stored_balls - 1);
		var stored_powder := CInt(GetObjProperty(golem, "golem_powder"));
		SetObjProperty(golem, "golem_powder", stored_powder - 5);
	else
		var ball := FindObjtypeInContainer(who.backpack, CANNONBALL_OBJTYPE);
		SubtractAmount(ball, 1);
		var powder := FindObjtypeInContainer(who.backpack, GUNPOWDER_OBJTYPE);
		SubtractAmount(powder, 5);
	endif

	// Cannon animation
	Message("*Dispara o canhao de mao!*").SendEmote(who);
	PlaySoundEffect(who, 0x667); // fuse
	sleepms(500);
	PlayMovingEffectXYZ(who.x, who.y, who.z + 5, targ.x, targ.y, targ.z, 0x5418, 5, 1, 0, who.realm);
	PlaySoundEffect(who, 0x665); // shot
	sleep(1);

	// Explosion at target
	PlayStationaryEffect(targ.x, targ.y, targ.z, 0x6D0E, 0, 10, 0, who.realm);
	PlaySoundEffect(who, 0x306); // explosion

	// AoE damage
	var science := mobile.GetSkill(SCIENCE);
	var base_damage := CInt(science / 10);
	var dano := RandomDiceRoll($"{base_damage}d12") + CInt((AP_GetStat(who, DEXTERITY) - 50) / 10) + CInt(science / 10);

	if (mobile.HasAbility(ABILITY_ID::TARGETING_VISOR))
		dano := CInt(dano * 1.25);
	endif

	var duration := CInt(science / 10);
	if (duration < 1)
		duration := 1;
	endif

	var festa := who.party;
	var allPets := array{};
	if (festa)
		foreach member in (festa.members)
			if (member != who)
				var petsNear := ListAllPetsAndSummonedPetsNear(member, 8);
				foreach pet in petsNear
					allPets.append(pet);
				endforeach
			endif
		endforeach
	endif

	var nearby := ListMobilesNearLocation(targ.x, targ.y, targ.z, 1, who.realm);
	foreach char in nearby
		if (char == who)
			continue;
		endif
		if (festa && char in festa.members)
			continue;
		endif
		if (char in allPets)
			continue;
		endif
		var char_owner := GetObjProperty(char, "owner");
		if (char_owner == who.serial)
			continue;
		endif

		PlayObjectCenteredEffect(char, 0x6D67, 0, 10);
		ApplyRawDamage(char, dano);
		TS_StartTimer(char, "tombar", duration);
	endforeach

	DebugMsg(
		who,
		DEBUG_HAB,
		Message($"[TACTICAL_SHOULDER] Handcannon! Dano: {dano}, AoE range: 1", Color(SYSTEM_COLOR::SUCCESS))
	);

	return true;
endfunction

function _FireHarpoon(mobile, who)
	SetScriptController(who);

	var targ;
	if (mobile.IsNPC())
		targ := who.opponent;
		if (!targ)
			targ := GetRandomNearbyHostile(who);
		endif
	else
		Message("Escolha o alvo do gancho.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		targ := Target(who);
	endif

	if (!targ)
		Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (!targ.isA(POLCLASS_MOBILE))
		Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	var has_visor := mobile.HasAbility(ABILITY_ID::TARGETING_VISOR);
	var max_range := 10;
	if (has_visor)
		max_range := 14;
	endif

	var dist := _ChebyshevDist(who.x, who.y, targ.x, targ.y);
	if (dist > max_range)
		Message($"Alvo muito distante! Alcance maximo: {max_range} tiles.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (dist < 2)
		Message("Alvo muito proximo!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	// Reflex test (Visor skips it)
	if (!has_visor)
		var difficulty := CInt(mobile.GetSkill(SCIENCE) / 5);
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
				Message($"[TACTICAL_SHOULDER] Gancho resistido (dif: {difficulty})", Color(SYSTEM_COLOR::FAIL))
			);
			return true;
		endif
	endif

	// Hook animation + pull
	Message("*Dispara o gancho mecanico!*").SendEmote(who);
	PlayMovingEffect(who, targ, 0x55A7, 5, 1, 0);
	sleep(1);

	TS_StartTimer(targ, "paralysis", dist);

	Message("*Enganchado!*").SendAbove(targ);
	PlaySoundEffect(who, 0x523);

	// Pull target along line
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
		Message($"[TACTICAL_SHOULDER] Gancho! Distancia: {dist}, Paralisia: {dist}s", Color(SYSTEM_COLOR::SUCCESS))
	);

	return true;
endfunction

function _DeployPowderTrap(mobile, who)
	SetScriptController(who);

	var targ;
	if (mobile.IsNPC())
		// Drop at own position — pursuers stepping in range trigger the trap.
		targ := struct{
			x := who.x,
			y := who.y,
			z := who.z,
			realm := who.realm
		};
	else
		Message("Escolha o local para colocar a armadilha.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		targ := TargetCoordinates(who);
	endif

	if (!targ)
		Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	if (GetObjProperty(who, "pilotando"))
		var golem := _TS_GetGolemRef(who);
		var stored := CInt(GetObjProperty(golem, "golem_powder"));
		SetObjProperty(golem, "golem_powder", stored - 5);
	else
		var trap_item := FindObjtypeInContainer(who.backpack, POWDERTRAP_OBJTYPE);
		SubtractAmount(trap_item, 1);
	endif

	var science := mobile.GetSkill(SCIENCE);
	var dano := CInt(science / 2);

	if (mobile.HasAbility(ABILITY_ID::TARGETING_VISOR))
		dano := CInt(dano * 1.5);
	endif

	var placed := CreateItemAtLocation(targ.x, targ.y, targ.z, POWDERTRAP_OBJTYPE, 1, who.realm);
	if (!placed)
		Message("Erro ao colocar a armadilha.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return false;
	endif

	SetObjProperty(placed, "#OwnerSerial", who.serial);
	SetObjProperty(placed, "Damage", dano);
	placed.invisible := 1;
	placed.movable := 0;

	Message("*Instala uma armadilha de polvora no chao*").SendEmote(who);
	PlaySoundEffect(who, 0x4F);

	DebugMsg(
		who,
		DEBUG_HAB,
		Message($"[TACTICAL_SHOULDER] Mina colocada! Dano: {dano}", Color(SYSTEM_COLOR::SUCCESS))
	);

	return true;
endfunction

function _TS_GetGolemRef(who)
	var serial := GetObjProperty(who, "lastpossessedserial");
	if (!serial)
		return 0;
	endif
	return SystemFindObjectBySerial(serial);
endfunction

function _GetSteamArms(who)
	foreach item in ListEquippedItems(who)
		if (item.objtype == STEAMARMS_OBJTYPE)
			return item;
		endif
	endforeach
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
