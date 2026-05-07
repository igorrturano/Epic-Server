// jailAction.inc — JailPlayer flow + supporting helpers.
//
// JailPlayer(jailor, guard, prisoner, severity) is the single entry point.
// Validates the target, sets criminal flag, attaches a grilhão, hands a
// matching key to the issuing guard, plays teleport FX, drops the prisoner
// into the mine, and rolls a chance to spawn a meteorite there.
//
// Suborno is a no-op handled here for symmetry — speech only.

use uo;
use os;
use util;

include "include/client";
include "include/say";
include ":fls_core:packets";
include ":brainAI:npcCommands";
include ":grilhao:include/grilhao";
include ":jailor:include/jailor";

const JAIL_QUOTA_LEVE		:= 100;
const JAIL_QUOTA_MODERADO	:= 400;
const JAIL_QUOTA_PESADO		:= 1000;

const JAIL_MINE_X			:= 4213;
const JAIL_MINE_Y			:= 2410;
const JAIL_MINE_Z			:= -2;
const JAIL_MINE_REALM		:= "britannia";

const JAIL_MINE_BOUNDS_X1	:= 4200;
const JAIL_MINE_BOUNDS_Y1	:= 2355;
const JAIL_MINE_BOUNDS_X2	:= 4247;
const JAIL_MINE_BOUNDS_Y2	:= 2420;

const JAIL_TELEPORT_SOUND	:= 0x1F0;

const JAIL_RELEASE_X		:= 1407;
const JAIL_RELEASE_Y		:= 1062;
const JAIL_RELEASE_Z		:= -4;
const JAIL_RELEASE_REALM	:= "britannia_alt";

const PEDRA_OBJTYPE			:= 0x5001;
const JAIL_PICKAXE_CPROP	:= "#jail_pickaxe";

const SEVERITY_LEVE			:= "leve";
const SEVERITY_MODERADO		:= "moderado";
const SEVERITY_PESADO		:= "pesado";
const SEVERITY_SUBORNO		:= "suborno";

const METEOR_OURO_OBJTYPE	:= 0xA1D0;
const METEOR_PRATA_OBJTYPE	:= 0xA1D1;
const METEOR_COBRE_OBJTYPE	:= 0xA1D3;

function QuotaForSeverity(severity)
	if (severity == SEVERITY_LEVE)     return JAIL_QUOTA_LEVE;     endif
	if (severity == SEVERITY_MODERADO) return JAIL_QUOTA_MODERADO; endif
	if (severity == SEVERITY_PESADO)   return JAIL_QUOTA_PESADO;   endif
	return 0;
endfunction

function JailPlayer(jailor, guard, prisoner, severity)
	if (severity == SEVERITY_SUBORNO)
		return HandleSuborno(jailor, guard, prisoner);
	endif

	var quota := QuotaForSeverity(severity);
	if (!quota)
		return 0;
	endif

	if (!ValidatePrisoner(jailor, guard, prisoner))
		return 0;
	endif

	SetObjProperty(prisoner, "#jail_quota",     quota);
	SetObjProperty(prisoner, "#jail_collected", 0);
	prisoner.SetCriminal(1);

	var grilhao := AttachGrilhao(prisoner, GRILHAO_DEFAULT_DR);
	if (grilhao == error || !grilhao)
		SendSysMessage(guard, "Falha ao criar o grilhao.");
		return 0;
	endif

	GiveKeyToGuard(guard, grilhao);
	PlayTeleportFx(prisoner);
	MoveObjectToLocation(prisoner, JAIL_MINE_X, JAIL_MINE_Y, JAIL_MINE_Z,
	                     JAIL_MINE_REALM, MOVEOBJECT_FORCELOCATION);
	PlayTeleportFx(prisoner);
	RollMeteoriteSpawn();

	AI_Speak(jailor, prisoner.name + " foi sentenciado(a) a pena " + severity + ".");
	return 1;
endfunction

function ValidatePrisoner(jailor, guard, prisoner)
	if (!prisoner || !prisoner.IsA(POLCLASS_MOBILE))
		SendSysMessage(guard, "Alvo invalido.");
		return 0;
	endif
	if (prisoner.cmdlevel > 0)
		SendSysMessage(guard, "Voce nao pode prender membros da equipe.");
		return 0;
	endif
	if (prisoner.dead)
		SendSysMessage(guard, "Esse alvo esta morto.");
		return 0;
	endif
	if (!prisoner.connected)
		SendSysMessage(guard, "O alvo precisa estar online.");
		return 0;
	endif
	if (IsPrisoner(prisoner))
		SendSysMessage(guard, "Esse alvo ja esta cumprindo pena.");
		return 0;
	endif
	return 1;
endfunction

function GiveKeyToGuard(guard, grilhao)
	var lockid := GetObjProperty(grilhao, "LockId");
	if (!lockid)
		return 0;
	endif
	var key := CreateItemInBackpack(guard, "ironkey", 1);
	if (!key || key == error)
		return 0;
	endif
	SetObjProperty(key, "LockId", lockid);
	key.name := "Chave do Grilhao";
	return 1;
endfunction

function PlayTeleportFx(who)
	if (!who)
		return 0;
	endif
	PlaySoundEffect(who, JAIL_TELEPORT_SOUND);
	PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, FX_ELETRICIDADE, 8, 8, 0, 10);
	return 1;
endfunction

function HandleSuborno(jailor, guard, prisoner)
	if (!prisoner || !prisoner.IsA(POLCLASS_MOBILE))
		SendSysMessage(guard, "Alvo invalido.");
		return 0;
	endif
	AI_Speak(jailor, "Tudo certo, parceiro. Acertem entre voces.");
	return 1;
endfunction

function ReceivePedras(jailor, prisoner, item)
	if (!item || item == error)
		return 0;
	endif
	if (Cint(item.objtype) != PEDRA_OBJTYPE)
		return 0;
	endif
	if (!IsPrisoner(prisoner))
		return 0;
	endif

	var quota     := Cint(GetObjProperty(prisoner, "#jail_quota"));
	var collected := Cint(GetObjProperty(prisoner, "#jail_collected"));
	collected := collected + Cint(item.amount);

	DestroyItem(item);
	SetObjProperty(prisoner, "#jail_collected", collected);

	if (collected >= quota)
		AI_Speak(jailor, "Pena cumprida. Voce esta livre.");
		ReleasePrisoner(jailor, prisoner);
		return 1;
	endif

	var remaining := quota - collected;
	AI_Speak(jailor, "Faltam " + remaining + " pedras, " + prisoner.name + ".");
	return 1;
endfunction

function ReleasePrisoner(jailor, prisoner)
	if (!prisoner || !prisoner.IsA(POLCLASS_MOBILE))
		return 0;
	endif

	prisoner.SetCriminal(0);

	RemoveGrilhao(prisoner);

	EraseObjProperty(prisoner, "#jail_quota");
	EraseObjProperty(prisoner, "#jail_collected");

	DestroyJailPickaxes(prisoner);

	PlayTeleportFx(prisoner);
	MoveObjectToLocation(prisoner, JAIL_RELEASE_X, JAIL_RELEASE_Y, JAIL_RELEASE_Z,
	                     JAIL_RELEASE_REALM, MOVEOBJECT_FORCELOCATION);
	PlayTeleportFx(prisoner);

	return 1;
endfunction

function DestroyJailPickaxes(prisoner)
	if (!prisoner.backpack)
		return 0;
	endif
	var destroyed := 0;
	foreach item in (EnumerateItemsInContainer(prisoner.backpack))
		if (Cint(GetObjProperty(item, JAIL_PICKAXE_CPROP)))
			DestroyItem(item);
			destroyed := destroyed + 1;
		endif
		sleepms(2);
	endforeach
	return destroyed;
endfunction

function RollMeteoriteSpawn()
	var roll := RandomInt(100);
	var ot := 0;
	if (roll < 1)
		ot := METEOR_OURO_OBJTYPE;
	elseif (roll < 6)
		ot := METEOR_PRATA_OBJTYPE;
	elseif (roll < 16)
		ot := METEOR_COBRE_OBJTYPE;
	else
		return 0;
	endif

	var x := JAIL_MINE_BOUNDS_X1 + RandomInt(JAIL_MINE_BOUNDS_X2 - JAIL_MINE_BOUNDS_X1 + 1);
	var y := JAIL_MINE_BOUNDS_Y1 + RandomInt(JAIL_MINE_BOUNDS_Y2 - JAIL_MINE_BOUNDS_Y1 + 1);

	var item := CreateItemAtLocation(x, y, JAIL_MINE_Z, ot, 1, JAIL_MINE_REALM);
	if (!item || item == error)
		return 0;
	endif
	return 1;
endfunction
