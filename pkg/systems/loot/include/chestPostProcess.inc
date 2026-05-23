use uo;

include "include/epicPropNames";

// ============================================================
// ChestLoot_PostProcess(chest)
//
// Runs after Loot_Generate fills an epic chest. Enforces the
// chest-loot economy rules that should not bleed into NPC drops:
//
//   1. Mundane weapons/armor are forced to baseline iron so chest
//      drops never outshine what crafters can make.
//   2. Jewelry never carries stat mods (strmod/dexmod/intmod);
//      vital mods on jewelry are deliberate (Phase 2b magic loot).
//
// Magic-loot items (carrying epic_magic_loot) are exempt from the
// iron force — they're meant to be magical objects, not metalwork.
// ============================================================

const CHEST_IRON_OBJTYPE := 0x6309;  // FerroTemperadoIngot

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
		LAYER_RING:     return 1;
		LAYER_BRACELET: return 1;
		LAYER_EARRINGS: return 1;
		LAYER_NECK:     return 1;
	endcase
	return 0;
endfunction

function ChestLoot_PostProcess(chest)
	if (!chest || chest == error)
		return 0;
	endif

	foreach item in EnumerateItemsInContainer(chest)
		// Skip our own magic loot — it's intentionally not iron.
		if (GetObjProperty(item, "epic_magic_loot"))
			if (ChestLoot_IsJewelry(item))
				ChestLoot_StripJewelryStatMods(item);
			endif
			continue;
		endif

		if (item.IsA(POLCLASS_WEAPON) || item.IsA(POLCLASS_ARMOR))
			SetObjProperty(item, PROPNAME_MATERIAL, CHEST_IRON_OBJTYPE);
		endif

		if (ChestLoot_IsJewelry(item))
			ChestLoot_StripJewelryStatMods(item);
		endif
		sleepms(2);
	endforeach
	return 1;
endfunction
