
use uo;

/*retorna o total de penalidade em hiding que o char tem devido ao equipamento 
* Cprops: HidingPenalty, StealthPenalty, DetectHiddenPenalty, LockpickingPenalty, RemoveTrapPenalty
* alem das cprops o itemdesc.cfg pode conter esses mesmos campos (sem ser Cprop)
*/

function getHidingEquipMod(who)

	var mod := 0;
	var cfg := ReadConfigFile(":*:itemdesc");
	var entry;
	var items := ListEquippedItems( who );

	foreach item in items

		entry :=  cfg[item.objtype];
		mod := mod +  Cint(GetObjProperty(item, "HidingPenalty"));
		if (entry)
			mod := mod + Cint(entry.HidingPenalty);
		endif

	endforeach
	items := 0;

	return mod;

endfunction

function getStealthEquipMod(who)

	var mod := 0;
	var cfg := ReadConfigFile(":*:itemdesc");
	var entry;

	var items := ListEquippedItems( who );
	foreach item in items

		entry :=  cfg[item.objtype];
		mod := mod +  Cint(GetObjProperty(item, "StealthPenalty"));
		if (entry)
			mod := mod + Cint(entry.StealthPenalty);
		endif

	endforeach

	return mod;

endfunction

function getDetectHiddenEquipMod(who)

	var mod := 0;
	var cfg := ReadConfigFile(":*:itemdesc");
	var entry;

	var items := ListEquippedItems( who );
	foreach item in items

		entry :=  cfg[item.objtype];
		mod := mod +  Cint(GetObjProperty(item, "DetectHiddenPenalty"));
		if (entry)
			mod := mod + Cint(entry.DetectHiddenPenalty);
		endif

	endforeach

	return mod;

endfunction

function getLockpickingEquipMod(who)

	var mod := 0;
	var cfg := ReadConfigFile(":*:itemdesc");
	var entry;

	var items := ListEquippedItems( who );

	foreach item in items

		entry :=  cfg[item.objtype];
		mod := mod +  Cint(GetObjProperty(item, "LockpickingPenalty"));
		if (entry)
			mod := mod + Cint(entry.LockpickingPenalty);
		endif

	endforeach

	return mod;

endfunction

function getRemoveTrapEquipMod(who)

	var mod := 0;
	var cfg := ReadConfigFile(":*:itemdesc");
	var entry;

	var items := ListEquippedItems( who );

	foreach item in items

		entry :=  cfg[item.objtype];
		mod := mod +  Cint(GetObjProperty(item, "RemoveTrapPenalty"));
		if (entry)
			mod := mod + Cint(entry.RemoveTrapPenalty);
		endif

	endforeach

	return mod;

endfunction
