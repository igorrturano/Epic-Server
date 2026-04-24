use uo;
use os;

include ":attributes:attributes";
include ":timedscripts:timedScripts";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";
include ":tn:cooldown";
include "include/say";
include "include/client";

function KataWakeUp(ghost, corpse)
	var serialC := ghost.serial;
	setcooldown(ghost, "deathtime", 25);

	if (corpse.graphic == 0x10DA) // Web Cocconut
		corpse.graphic := corpse.objtype;
	endif

	if (!MoveObjectToLocation(ghost, corpse.x, corpse.y, corpse.z, corpse.realm, MOVEOBJECT_FORCELOCATION))
		SendSysMessage(ghost, "Ocorreu algum problema ao retornar seu personagem para o corpo. Solicite ajude da Staff");

		if (!CInt(GetObjProperty(corpse, "morto")) && !CInt(GetObjProperty(ghost, "morto")))
			SetObjProperty(corpse, "erro_retornar_corpo", 1);
			corpse.decayat := 0;

			foreach item in EnumerateItemsInContainer(corpse)
				if (item.container == corpse)
					item.decayat := 0;
				endif
			endforeach

			return;
		endif
	endif

	if (ghost.dead)
		Resurrect(ghost);
	endif
	SetCooldown(ghost, "res_trap_protection", 10);
	var ressrobe := GetEquipmentByLayer(ghost, 0x16);
	DestroyItem(ressrobe);

	foreach item in EnumerateItemsInContainer(corpse)
		if (item.container == corpse)
			MoveObjectToLocation(item, ghost.x, ghost.y, ghost.z, ghost.realm);
			MoveItemToContainer(item, ghost.backpack);
			if (CInt(GetObjProperty(item, "#EquippedOn")) == serialC)
				if (!EquipItem(ghost, item))
					EraseObjProperty(item, "#EquippedOn");
				endif
			endif
		endif
	endforeach

	if (MobileWrapper(ghost).HasAbility(ABILITY_ID::RECOVERY))
		SetVital(ghost, "hits", CInt(GetVitalMaximumValue(ghost, "hits") * 0.9));
	else
		SetVital(ghost, "hits", CInt(GetVitalMaximumValue(ghost, "hits") * 0.20));
	endif
	SetVital(ghost, "stamina", CInt(GetVitalMaximumValue(ghost, "stamina") * 0.5));
	SetVital(ghost, "mana", CInt(GetVitalMaximumValue(ghost, "mana") * 0.5));
	RecalcVitals(ghost);

var deferred_fraturas := GetObjProperty(ghost, "#deferred_fratura");
if (deferred_fraturas != error)
	foreach fratura in deferred_fraturas

		if (CInt(fratura.tempo) > 0)
			TS_StartTimer(ghost, fratura.part, CInt(fratura.tempo));
		endif

	endforeach

	EraseObjProperty(ghost, "#deferred_fratura");
endif

	DestroyItem(corpse);

	if (GetObjProperty(ghost, "corda"))
		PrintTextAbove(ghost, "*amarrado*");
		SendSysMessage(ghost, "Para tentar se soltar utilize o comando .soltar");
		var item;
		item := GetEquipmentByLayer(ghost, LAYER_HAND1);
		if (item)
			MoveItemToContainer(item, ghost.backpack);
		endif
		item := GetEquipmentByLayer(ghost, LAYER_HAND2);
		if (item)
			MoveItemToContainer(item, ghost.backpack);
		endif
		ghost.frozen := 1;
	endif

	var save := GetObjProperty(ghost, "savegame");
	if (save != error)
		MoveObjectToLocation(ghost, save.x, save.y, save.z, save.realm, MOVEOBJECT_FORCELOCATION);
		AP_SetVital(ghost, STAMINA, cint(save.stamina));
		AP_SetVital(ghost, MANA, cint(save.mana));
		AP_SetVital(ghost, HITS, cint(save.hits));
		eraseobjproperty(ghost, "savegame");
	endif

	if (GetObjProperty(ghost, "metamorfose"))
		TS_LowerDuration(ghost, "metamorfosedruida", -1);
	endif

	EraseObjProperty(ghost, "death_remaining");
	EraseObjProperty(ghost, "death_corpse_serial");
endfunction

function DuelWakeUp(ghost, corpse)
	var serialC := ghost.serial;
	setcooldown(ghost, "deathtime", 25);

	if (corpse.graphic == 0x10DA)
		corpse.graphic := corpse.objtype;
	endif

	if (!MoveObjectToLocation(ghost, corpse.x, corpse.y, corpse.z, corpse.realm, MOVEOBJECT_FORCELOCATION))
		SendSysMessage(ghost, "Ocorreu algum problema ao retornar seu personagem para o corpo. Solicite ajude da Staff");
		if (!CInt(GetObjProperty(corpse, "morto")) && !CInt(GetObjProperty(ghost, "morto")))
			SetObjProperty(corpse, "erro_retornar_corpo", 1);
			corpse.decayat := 0;
			foreach item in EnumerateItemsInContainer(corpse)
				if (item.container == corpse)
					item.decayat := 0;
				endif
			endforeach
			return;
		endif
	endif

	if (ghost.dead)
		Resurrect(ghost);
	endif
	SetCooldown(ghost, "res_trap_protection", 10);
	var ressrobe := GetEquipmentByLayer(ghost, 0x16);
	DestroyItem(ressrobe);

	foreach item in EnumerateItemsInContainer(corpse)
		if (item.container == corpse)
			MoveObjectToLocation(item, ghost.x, ghost.y, ghost.z, ghost.realm);
			MoveItemToContainer(item, ghost.backpack);
			if (CInt(GetObjProperty(item, "#EquippedOn")) == serialC)
				if (!EquipItem(ghost, item))
					EraseObjProperty(item, "#EquippedOn");
				endif
			endif
		endif
	endforeach

	SetVital(ghost, "hits", CInt(GetVitalMaximumValue(ghost, "hits") * 0.80));
	SetVital(ghost, "stamina", CInt(GetVitalMaximumValue(ghost, "stamina") * 0.5));
	SetVital(ghost, "mana", CInt(GetVitalMaximumValue(ghost, "mana") * 0.5));
	RecalcVitals(ghost);

	DestroyItem(corpse);

	EraseObjProperty(ghost, "death_remaining");
	EraseObjProperty(ghost, "death_corpse_serial");
endfunction

function FallbackResurrect(ghost)
	if (ghost.dead)
		Resurrect(ghost);
	endif
	SetCooldown(ghost, "res_trap_protection", 10);
	var ressrobe := GetEquipmentByLayer(ghost, 0x16);
	DestroyItem(ressrobe);

	SetVital(ghost, "hits", CInt(GetVitalMaximumValue(ghost, "hits") * 0.20));
	SetVital(ghost, "stamina", CInt(GetVitalMaximumValue(ghost, "stamina") * 0.5));
	SetVital(ghost, "mana", CInt(GetVitalMaximumValue(ghost, "mana") * 0.5));
	RecalcVitals(ghost);

	var deferred_fraturas := GetObjProperty(ghost, "#deferred_fratura");
	if (deferred_fraturas != error)
		foreach fratura in deferred_fraturas
			var cooldown_name := "hurt_" + fratura.part;
			var existing := GetCooldown(ghost, cooldown_name);
			if (existing > 0)
				SetCoolDown(ghost, cooldown_name, existing * 2);
			elseif (CInt(fratura.tempo) > 0)
				SetCoolDown(ghost, cooldown_name, CInt(fratura.tempo));
			endif
			TS_StartTimer(ghost, fratura.part, 1);
		endforeach
		EraseObjProperty(ghost, "#deferred_fratura");
	endif

	SendSysMessage(ghost, "Voce acordou, mas seu corpo nao foi encontrado.");

	EraseObjProperty(ghost, "death_remaining");
	EraseObjProperty(ghost, "death_corpse_serial");
endfunction
