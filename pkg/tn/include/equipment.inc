use uo;

include ":attributes:attributes";

function coverFace(item)
	case(item.graphic)
		0x1408: 0x1409: 0x1412:
		0x1419: 0x1451: 0x1F0B:
		0x140C: 0x140D: 0x1456:
		0x1F0C: 0x1549: 0x154A:
		0x154B: 0x154C:	0x278E:
		0x278F: 0x1388:
			return 1;
		default:
			return 0;
	endcase
endfunction

function EnumeratePhysicalItems(mobile)
	var EquipList := array;

	var SkipLayers := {11, 16, 21, 25};
	var i;

	for (i:=0; i<=25; i:=i+1)
		if (!(i in SkipLayers))
			var layer := GetEquipmentByLayer(mobile, i);
			if (layer)
				EquipList.append(layer);
			endif
		endif
	endfor

	return EquipList;
endfunction

function UnequipAll(who)
	var equips := EnumeratePhysicalItems(who);

	foreach equip in equips
		MoveItemToContainer(equip, who.backpack);
	endforeach
endfunction

function ApplyEquipObjMods(who, item)
	var mods := GetObjProperty(item, "equip");
	if (!mods)
		return;
	endif
	if (mods.keys().size() < 1)
		return;
	endif

	foreach mod in (mods.keys())
		if (GetObjProperty(item, "magic") == 1)
			continue;
		endif

		case (mod)
			"strmod":
				AP_ModifyStatMod(who, STRENGTH, CInt(mods["strmod"]));
			"dexmod":
				AP_ModifyStatMod(who, DEXTERITY, CInt(mods["dexmod"]));
			"intmod":
				AP_ModifyStatMod(who, INTELLIGENCE, CInt(mods["intmod"]));
			"manamod":
				SetObjProperty(who, "#manamod", CInt(GetObjProperty(who, "#manamod")) + CInt(mods["manamod"]));
				RecalcVitals(who);
			"hitsmod":
				SetObjProperty(who, "#hitsmod", CInt(GetObjProperty(who, "#hitsmod")) + CInt(mods["hitsmod"]));
				RecalcVitals(who);
			"stammod":
				SetObjProperty(who, "#stammod", CInt(GetObjProperty(who, "#stammod")) + CInt(mods["stammod"]));
				RecalcVitals(who);
			"ColdResistMod":
			"FireResistMod":
			"PhysicalResistMod":
			"EnergyResistMod":
			"PoisonResistMod":
				RecalcVitals(who);
			"setprop":
				SetObjProperty(who, "#" + GetObjProperty(item, "propname"), GetObjProperty(item, "propvalue"));
			"sacred":
			"cursed":
				// Flags de categoria, nao sao mods
			default:
				AP_ModifySkillMod(who, mod, CInt(mods[mod]));
		endcase
	endforeach
endfunction

function RemoveEquipObjMods(who, item)
	var mods := GetObjProperty(item, "equip");
	if (!mods)
		return;
	endif
	if (mods.keys().size() < 1)
		return;
	endif

	foreach mod in (mods.keys())
		if (GetObjProperty(item, "magic") == 1)
			continue;
		endif

		case (mod)
			"strmod":
				AP_ModifyStatMod(who, STRENGTH, -CInt(mods["strmod"]));
			"dexmod":
				AP_ModifyStatMod(who, DEXTERITY, -CInt(mods["dexmod"]));
			"intmod":
				AP_ModifyStatMod(who, INTELLIGENCE, -CInt(mods["intmod"]));
			"manamod":
				SetObjProperty(who, "#manamod", CInt(GetObjProperty(who, "#manamod")) - CInt(mods["manamod"]));
				RecalcVitals(who);
			"hitsmod":
				var final := CInt(GetObjProperty(who, "#hitsmod")) - CInt(mods["hitsmod"]);
				if (final <= 0)
					EraseObjProperty(who, "#hitsmod");
				else
					SetObjProperty(who, "#hitsmod", final);
				endif
				RecalcVitals(who);
			"stammod":
				SetObjProperty(who, "#stammod", CInt(GetObjProperty(who, "#stammod")) - CInt(mods["stammod"]));
				RecalcVitals(who);
			"ColdResistMod":
			"FireResistMod":
			"PhysicalResistMod":
			"EnergyResistMod":
			"PoisonResistMod":
				RecalcVitals(who);
			"setprop":
				EraseObjProperty(who, "#" + GetObjProperty(item, "propname"));
			"sacred":
			"cursed":
				// Flags de categoria, nao sao mods
			default:
				AP_ModifySkillMod(who, mod, -CInt(mods[mod]));
		endcase
	endforeach
endfunction
