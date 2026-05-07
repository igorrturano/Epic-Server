use uo;
use os;

include ":keys:key";
include ":fls_core:packets";

const GRILHAO_OBJTYPE       := 0xFCD7;
const GRILHAO_DEFAULT_DR    := 60;

function AttachGrilhao(prisoner, lockDR := GRILHAO_DEFAULT_DR)
	if (!prisoner.backpack)
		return error;
	endif

	var grilhao := CreateItemAtContainer(prisoner.backpack, GRILHAO_OBJTYPE, 1);
	if (grilhao == error || !grilhao)
		return error;
	endif

	var lockid := AllocLockId();
	SetObjProperty(grilhao, "LockId",   lockid);
	SetObjProperty(grilhao, "lockDR",   lockDR);
	SetObjProperty(grilhao, "carrier",  prisoner.serial);
	SetObjProperty(prisoner, "#grilhao_serial", grilhao.serial);

	PlayerSpeedSlower(prisoner, 1);
	return grilhao;
endfunction

function RemoveGrilhao(prisoner)
	var grilhaoSerial := GetObjProperty(prisoner, "#grilhao_serial");
	if (grilhaoSerial == error || !grilhaoSerial)
		return 0;
	endif

	var grilhao := SystemFindObjectBySerial(grilhaoSerial);
	if (grilhao && grilhao != error)
		DestroyItem(grilhao);
	endif

	EraseObjProperty(prisoner, "#grilhao_serial");
	PlayerSpeedSlower(prisoner, 0);
	return 1;
endfunction

function HasGrilhao(prisoner)
	var grilhaoSerial := GetObjProperty(prisoner, "#grilhao_serial");
	if (grilhaoSerial == error || !grilhaoSerial)
		return 0;
	endif
	var grilhao := SystemFindObjectBySerial(grilhaoSerial);
	if (grilhao == error || !grilhao)
		return 0;
	endif
	return 1;
endfunction
