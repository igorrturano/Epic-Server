use uo;

include "include/client";

// ============================================================
// ChestLoot_PostProcess(chest)
//
// Runs after Loot_Generate fills an epic chest. Today the only job
// left here is jewelry stat-mod sanitation — the iron-lock on
// mundane gear is enforced at creation time inside CreateItemProcess
// (see lootParser.inc), which actually writes the fabricado prop
// that combat reads. A flat "material" ObjProperty stamp wouldn't
// move combat math.
//
// Jewelry rule: never carries strmod / dexmod / intmod. Vital mods
// on jewelry are deliberate (Phase 2b magic loot, applied via
// ApplyEpicProperties).
// ============================================================

function ChestLoot_StripJewelryStatMods(item)
	var equip := GetObjProperty(item, "equip");
	if (!equip || TypeOfInt(equip) != OT_DICTIONARY)
		return;
	endif
	var dirty := 0;
	if (equip.Exists("strmod"))
		equip.Erase("strmod");
		dirty := 1;
	endif
	if (equip.Exists("dexmod"))
		equip.Erase("dexmod");
		dirty := 1;
	endif
	if (equip.Exists("intmod"))
		equip.Erase("intmod");
		dirty := 1;
	endif
	if (dirty)
		SetObjProperty(item, "equip", equip);
	endif
endfunction

function ChestLoot_IsJewelry(item)
	case (item.layer)
		LAYER_RING:  return 1;
		LAYER_WRIST: return 1;
		LAYER_EARS:  return 1;
		LAYER_NECK:  return 1;
	endcase
	return 0;
endfunction

function ChestLoot_PostProcess(chest)
	if (!chest || chest == error)
		return 0;
	endif

	foreach item in EnumerateItemsInContainer(chest)
		if (ChestLoot_IsJewelry(item))
			ChestLoot_StripJewelryStatMods(item);
		endif
		sleepms(2);
	endforeach
	return 1;
endfunction
