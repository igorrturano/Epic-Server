use uo;

include "include/say";
include ":gumps:yesNo";

function CheckIngredients(master, ritualid)

	var cfg := ReadconfigFile(":tn:rituals");
	if (!cfg)
		SendSysMessageEx(master, "Config file not found: :tn:rituals", SSM_FAIL);
		return 0;
	endif

	var elem := FindConfigElem(cfg, ritualid);
	if (!elem)
		SendSysMessageEx(who, "spellid nao encontrado " + spellinfo.id, SSM_FAIL);
		return 0;
	endif

	var ingredients := GetConfigStringDictionary( element, "materials" );
	var objtypes := dictionary;

	var keys := ingredients.keys();
	var amount;
	foreach reag in keys

		amount := CInt( ingredients[reag] );
		if (!amount)
			amount := 1;
		endif
		var objtype := GetObjTypeByName(reag);
		objtypes[objtype] := amount;
		sleepms(20);
	endforeach

	var objects := ListObjectNearLocation(master.x, master.y, master.z, 2, master.realm);

	var objectsToDestroy := array;
	foreach thing in (objtypes.keys())
		var found := 0;
		foreach item in object
			if (item.objtype ==  thing)
				if ( item.amount == objtypes[thing] )
					found := 1;
					objectsToDestroy(item);
				endif
			endif
		endforeach
		if (!found)
			SendSysMessageEx(master, "Voce nao tem todos os componentes materiais.", SSM_FAIL);
			return 0;
		endif
	endforeach


	var mobs := ListMobilesNearLocationEx( master.x, master.y, master.z, 2, 0, master.realm );
	var maxmages := GetConfigInt(element, "MaxMages");
	var minmages := GetConfigInt(element, "MinMages");
	var mages := 1;

	var ritualmages := array;
	foreach mob in mobs
		if ( mob != master)
			var yes := YesNo(mob, "Voce vai participar do ritual?");
			if (yes)
				mages := mages + 1;
				ritualmages.append(mob);
			endif
		endif
	endforeach

	if ( mages > maxmages )
		SendSysMessageEx(master, "Existem muitos magos participando do ritual.", SSM_FAIL);
		return 0;
	endif

	if ( mages < minmages )
		SendSysMessageEx(master, "Existem poucos magos participando do ritual.", SSM_FAIL);
		return 0;
	endif

	var minskill := GetConfigInt(element, "Skill");
	var minmana := GetConfigInt(element, "Mana");
	var skill := 0;
	var mana := 0;

	foreach mob in ritualmages
		skill := skill + AP_GetSkill(mob, MAGERY);
		mana := mana + AP_GetVital(mob, "Mana");
	endforeach

	if (skill < minskill )
		SendSysMessageEx(master, "Os magos nao possuem o conhecimento arcano necessario para executar o ritual.", SSM_FAIL);
		return 0;
	endif

	if ( mana < minmana )
		SendSysMessageEx(master, "Os magos nao possuem a energia arcana necessaria para executar o ritual.", SSM_FAIL);
		return 0;
	endif

	foreach item in objectsToDestroy
		DestroyItem(item);
	endforeach

	var delay := GetConfigInt(elem, "Delay");
	var loops := GetconfigInt(elem, "Loops");
	var powerwords := GetConfigString(elem, "PowerWords");
	var ritualscript := GetConfigString(elem, "RitualScript");

	foreach loop in loops
		foreach mobile in ritualmages
			PrintText(mobile, powerwords);
			PerformAction(mobile, ANIM_CAST_AREA);
		endforeach
		sleepms(delay);
	endforeach


	Start_Script(ritualscript, {master, ritualid});

endfunction

