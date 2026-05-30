// jailAction.inc — JailPlayer flow + supporting helpers.
//
// JailPlayer(jailor, guard, prisoner, severity) is the single entry point.
// Validates the target, then asks the prisoner whether they want to pay the
// penalty in moedacobre debited from their bank (immediate release) or work
// it off in the mine (criminal flag + #jail_quota CProp + teleport).
//
// Suborno is a no-op handled here for symmetry — speech only.

use uo;
use os;
use util;

include "include/say";
include ":fls_core:packets";
include ":brainAI:npcCommands";
include ":gumps:yesNo";
include ":containers:storageAreas";
include ":currency:include/currency";
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

const JAIL_TELEPORT_SOUND	:= 0x1F2;
const JAIL_FX_GRAPHIC		:= 0xD11B;

const JAIL_RELEASE_X		:= 1407;
const JAIL_RELEASE_Y		:= 1062;
const JAIL_RELEASE_Z		:= -4;
const JAIL_RELEASE_REALM	:= "britannia_alt";

const PEDRA_OBJTYPE			:= 0x5001;
const MOEDACOBRE_OBJTYPE	:= 0xBA63;
const MOEDACOBRE_TO_PEDRA	:= 10;
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

	AI_Speak(jailor, prisoner.name + " foi sentenciado(a) a pena " + severity + ".");

	var coinCost := quota / MOEDACOBRE_TO_PEDRA;
	var payCoins := YesNo(prisoner,
		"Voce prefere pagar sua pena com " + coinCost + " moedas (debitado do seu banco) ou trabalhar na mina por " + quota + " pedras?",
		"Pagar moedas",
		"Ir para a mina");

	if (payCoins)
		if (TryPayFromBank(prisoner, coinCost))
			AI_Speak(jailor, "Pena paga em moedas. " + prisoner.name + " esta livre.");
			return 1;
		endif
		AI_Speak(jailor, "Saldo insuficiente no banco. " + prisoner.name + " ira para a mina.");
	endif

	SetObjProperty(prisoner, "#jail_quota",     quota);
	SetObjProperty(prisoner, "#jail_collected", 0);
	prisoner.SetCriminal(1);

	TeleportPrisoner(prisoner, JAIL_MINE_X, JAIL_MINE_Y, JAIL_MINE_Z, JAIL_MINE_REALM);
	RollMeteoriteSpawn();
	return 1;
endfunction

function TryPayFromBank(prisoner, amount)
	var bank := CP_GetStorageContainerForMobile(prisoner, CP_BANK_AREA);
	if (!bank || bank == error)
		return 0;
	endif
	return SpendCoin(prisoner, amount, bank, MOEDACOBRE_OBJTYPE);
endfunction

function ValidatePrisoner(jailor, guard, prisoner)
	if (!prisoner || !prisoner.IsA(POLCLASS_MOBILE))
		SendSysMessage(guard, "Alvo invalido.");
		return 0;
	endif
	if (prisoner.serial == guard.serial)
		SendSysMessage(guard, "Voce nao pode sentenciar a si mesmo.");
		return 0;
	endif
	// if (prisoner.cmdlevel > 0)
	// 	SendSysMessage(guard, "Voce nao pode prender membros da equipe.");
	// 	return 0;
	// endif
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

function TeleportPrisoner(prisoner, x, y, z, realm)
	if (!prisoner)
		return 0;
	endif
	PlayStationaryEffect(prisoner.x, prisoner.y, prisoner.z, JAIL_FX_GRAPHIC, 0, 30, 0, prisoner.realm);
	PlaySoundEffect(prisoner, JAIL_TELEPORT_SOUND);

	PlayStationaryEffect(x, y, z, JAIL_FX_GRAPHIC, 0, 30, 0, realm);
	PlaySoundEffect(prisoner, JAIL_TELEPORT_SOUND);
	sleep(2);

	return MoveObjectToLocation(prisoner, x, y, z, realm, MOVEOBJECT_FORCELOCATION);
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

	EraseObjProperty(prisoner, "#jail_quota");
	EraseObjProperty(prisoner, "#jail_collected");

	DestroyJailPickaxes(prisoner);

	TeleportPrisoner(prisoner, JAIL_RELEASE_X, JAIL_RELEASE_Y, JAIL_RELEASE_Z, JAIL_RELEASE_REALM);

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
	// TEST MODE: 100% spawn rate. Tier split 10/30/60 ouro/prata/cobre.
	// Restore "else return 0;" to gate spawns once testing is done.
	var roll := RandomInt(100);
	var ot := METEOR_COBRE_OBJTYPE;
	if (roll < 10)
		ot := METEOR_OURO_OBJTYPE;
	elseif (roll < 40)
		ot := METEOR_PRATA_OBJTYPE;
	endif

	var x := JAIL_MINE_BOUNDS_X1 + RandomInt(JAIL_MINE_BOUNDS_X2 - JAIL_MINE_BOUNDS_X1 + 1);
	var y := JAIL_MINE_BOUNDS_Y1 + RandomInt(JAIL_MINE_BOUNDS_Y2 - JAIL_MINE_BOUNDS_Y1 + 1);

	var item := CreateItemAtLocation(x, y, JAIL_MINE_Z, ot, 1, JAIL_MINE_REALM);
	if (!item || item == error)
		return 0;
	endif
	return 1;
endfunction
