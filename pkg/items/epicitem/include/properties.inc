use uo;

// ============================================================
// ApplyEpicProperties / SetOrErase
// Stamps the epic-item ObjProperty schema on an item:
//   - equip dict (sacred/cursed flag, stat/vital/resist mods)
//   - magic flag
//   - epicEntropy threshold (sacred/cursed equip gate)
//   - epiceffect dict (spell or timedscript usescript)
//   - usescript wiring to :epicitem:epicitem_use
//
// Lifted out of pkg/commands/include/epicitemGump.inc so it can
// be reused by loot generators that build magic items without
// invoking the GM gump UI. The GM gump now also includes this file.
// ============================================================

function SetOrErase(byref dict, key, value)
	if (CInt(value) != 0)
		dict[key] := CInt(value);
	else
		dict.Erase(key);
	endif
endfunction

function ApplyEpicProperties(item, state)
	// Build equip dictionary
	var equip := GetObjProperty(item, "equip");
	if (!equip || TypeOfInt(equip) != OT_DICTIONARY)
		equip := dictionary{};
	endif

	// Clear old epic flags
	equip.Erase("sacred");
	equip.Erase("cursed");

	// Set category
	if (state.category == "sacred")
		equip["sacred"] := 1;
	elseif (state.category == "cursed")
		equip["cursed"] := 1;
	endif

	// Set stat mods
	SetOrErase(equip, "strmod", state.strmod);
	SetOrErase(equip, "dexmod", state.dexmod);
	SetOrErase(equip, "intmod", state.intmod);
	SetOrErase(equip, "hitsmod", state.hitsmod);
	SetOrErase(equip, "manamod", state.manamod);
	SetOrErase(equip, "stammod", state.stammod);

	// Set resist mods
	SetOrErase(equip, "ColdResistMod", state.ColdResistMod);
	SetOrErase(equip, "FireResistMod", state.FireResistMod);
	SetOrErase(equip, "PhysicalResistMod", state.PhysicalResistMod);
	SetOrErase(equip, "EnergyResistMod", state.EnergyResistMod);
	SetOrErase(equip, "PoisonResistMod", state.PoisonResistMod);

	SetObjProperty(item, "equip", equip);

	// Magic flag — start unidentified for ALL enchanted categories.
	//   magic == 1 → unidentified (mods skipped on equip, no stamp written)
	//   magic == 2 → identified (Arcana for "magic", Theology for sacred/cursed)
	// Identification scripts re-apply mods immediately if the item is worn.
	// Mod application owned by EquipModStamper
	// (pkg/systems/equipsys/include/Model/EquipModStamper.inc),
	// invoked from POL's global EquipTest/UnequipTest hooks.
	if (state.category == "magic" || state.category == "sacred" || state.category == "cursed")
		SetObjProperty(item, "magic", 1);
	else
		EraseObjProperty(item, "magic");
	endif

	// Entropy threshold
	if (state.entropy_threshold != 0)
		SetObjProperty(item, "epicEntropy", state.entropy_threshold);
	else
		EraseObjProperty(item, "epicEntropy");
	endif

	// Double-click effect
	if (state.effect_type != "none" && state.effect_name != "")
		var epiceffect := struct{};
		epiceffect.+effect_type := state.effect_type;
		epiceffect.+effect_id := state.effect_id;
		epiceffect.+effect_name := state.effect_name;
		epiceffect.+script := state.effect_script;
		epiceffect.+target := state.effect_target;
		epiceffect.+charges_left := CInt(state.charges);
		epiceffect.+cooldown := CInt(state.cooldown);
		epiceffect.+duration := CInt(state.duration);
		epiceffect.+destroy_on_use := CInt(state.destroy_on_use);

		SetObjProperty(item, "epiceffect", epiceffect);
		item.usescript := ":epicitem:epicitem_use";
	else
		EraseObjProperty(item, "epiceffect");
	endif
endfunction
