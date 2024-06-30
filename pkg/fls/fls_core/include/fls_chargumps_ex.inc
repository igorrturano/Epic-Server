
use uo;
use cfgfile;

include ":gumps:gumps";
include "include/arrays";
include ":fls_core:fls_characters";

function FLHairDye(who, byref new_look)

	var skingump := GFCreateGump();
	GFResizePic(skingump, 0, 0, 3600, 300, 420);
	GFResizePic(skingump, 15, 15, 0x13BE, 270, 390);

	GFTextMid(skingump, 15, 25, 270, 1890, "Escolha a Cor do Cabelo");
	GFAddButton(skingump, 220, 370, 2128, 2129, 1, OK_BUTTON);

	var skin_cfg := ReadConfigFile(":fls_core:config/racas");
	if ( !skin_cfg )
		SendSysMessage(who, "Error: Could not open :fls_core:config/racas.cfg -> "+skin_cfg.errortext);
	endif
	skin_cfg := skin_cfg[cstr(GetObjProperty(who, "raca"))];

	GFPage(skingump, 1);

	var y_pos := 60;
	var x_pos := 20;
	var place := 1;
	var page_num := 1;
	var line := 1;
	foreach color in GetConfigStringArray(skin_cfg, "HairColor");
		color := CInt(color);
		if (color == new_look.SkinColor)
			GFRadioButton(skingump, x_pos, y_pos, 9727, 9730, 1, color);
		else
			GFRadioButton(skingump, x_pos, y_pos, 9727, 9730, 0, color);
		endif

		GFTextline(skingump, x_pos+35, y_pos+5, color-1, "Hair");

		if (place < 3)
			x_pos := x_pos+80;
			place := place+1;
		else
			place := 1;
			x_pos := 20;
			y_pos := y_pos+30;

			line := line+1;

			if (line > 10)
				GFAddButton(skingump, 265, 340, 2648, 2649, GF_PAGE_BTN, page_num+1);
				page_num := page_num+1;
				GFPage(skingump, page_num);
				GFAddButton(skingump, 265, 60, 2650, 2651, GF_PAGE_BTN, page_num-1);

				line := 1;
				y_pos := 60;
			endif
		endif
	endforeach

	var input := GFSendGump(who, skingump);

	if (input[0] == OK_BUTTON)
		foreach key in (input.keys)
			if ( (key != OK_BUTTON) && (key > 0))
				new_look.HairColor := key;
				break;
			endif
		endforeach
	endif

endfunction

function FLSkinGump(who, byref new_look)

	var skingump := GFCreateGump();
	GFResizePic(skingump, 0, 0, 3600, 300, 420);
	GFResizePic(skingump, 15, 15, 0x13BE, 270, 390);

	GFTextMid(skingump, 15, 25, 270, 1890, "Escolha a Cor de Pele");
	GFAddButton(skingump, 220, 370, 2128, 2129, 1, OK_BUTTON);

	var skin_cfg := ReadConfigFile(":fls_core:config/racas");
	if ( !skin_cfg )
		SendSysMessage(who, "Error: Could not open :fls_core:config/racas.cfg -> "+skin_cfg.errortext);
	endif
	skin_cfg := skin_cfg[cstr(GetObjProperty(who, "raca"))];

	GFPage(skingump, 1);

	var y_pos := 60;
	var x_pos := 20;
	var place := 1;
	var page_num := 1;
	var line := 1;
	foreach color in GetConfigStringArray(skin_cfg, "SkinColor");
		color := CInt(color);
		if (color == new_look.SkinColor)
			GFRadioButton(skingump, x_pos, y_pos, 9727, 9730, 1, color);
		else
			GFRadioButton(skingump, x_pos, y_pos, 9727, 9730, 0, color);
		endif

		GFTextline(skingump, x_pos+35, y_pos+5, color-1, "Skin");

		if (place < 3)
			x_pos := x_pos+80;
			place := place+1;
		else
			place := 1;
			x_pos := 20;
			y_pos := y_pos+30;

			line := line+1;

			if (line > 10)
				GFAddButton(skingump, 265, 340, 2648, 2649, GF_PAGE_BTN, page_num+1);
				page_num := page_num+1;
				GFPage(skingump, page_num);
				GFAddButton(skingump, 265, 60, 2650, 2651, GF_PAGE_BTN, page_num-1);

				line := 1;
				y_pos := 60;
			endif
		endif
	endforeach

	var input := GFSendGump(who, skingump);

	if (input[0] == OK_BUTTON)
		foreach key in (input.keys)
			if ( (key != OK_BUTTON) && (key > 0))
				new_look.SkinColor := key;
				break;
			endif
		endforeach
	endif

endfunction

function FLHairGump(who, byref new_look)

	var hairgump := GFCreateGump();

	GFResizePic(hairgump, 0, 0, 3600, 300, 420);
	GFResizePic(hairgump, 15, 15, 0x13BE, 270, 390);

	GFTextMid(hairgump, 15, 25, 270, 1890, "Escolha seu Cabelo");
	GFAddButton(hairgump, 220, 370, 2128, 2129, 1, OK_BUTTON);

	GFResizePic(hairgump, 220, 55, 2620, 50, 310);

	var config := ReadConfigFile(":fls_core:config/hairtypes");
	var hair;
	if ( (GetObjProperty(who, "raca") == "anao") || (GetObjProperty(who, "raca") == "hobbit"))
		hair := config["headdwarf"];
	else
		hair := config["head"];
	endif
	var hairgumps := config["hairgump"];
	GFSetRadioGroup(hairgump, 1);

	var y_pos := 65;
	foreach style in GetConfigStringArray(hair, "style")
		style := splitwords(style);
		var hairname := style[1];
		var hairgraphic := cint(style[2]);
		while (hairname["_"])
			hairname["_"] := " ";
		endwhile

		if (hairgraphic == new_look.HairStyle)
			GFRadioButton(hairgump, 20, y_pos, 2151, 2154, 1, hairgraphic);
		else
			GFRadioButton(hairgump, 20, y_pos, 2151, 2154, 0, hairgraphic);
		endif

		GFTextline(hairgump, 55, y_pos+5, 1890, hairname);

		var gumppic := GetConfigStringArray(hairgumps, style[1]);
		gumppic := splitwords(gumppic);
		GFGumpPic(hairgump, 155, y_pos-50, gumppic[new_look.Gender+2]);

		y_pos := y_pos + 30;
	endforeach

	if ( !new_look.HairStyle || new_look.HairStyle == 1 )
		GFRadioButton(hairgump, 20, y_pos, 2151, 2154, 1, 1);
	else
		GFRadioButton(hairgump, 20, y_pos, 2151, 2154, 0, 1);
	endif
	GFTextline(hairgump, 55, y_pos+5, 1890, "Bald");

	var input := GFSendGump(who, hairgump);


	if (input[0] == OK_BUTTON)
		foreach key in (input.keys)
			if ( (key != OK_BUTTON) && (key > 0))
				sendsysmessage(who, " "+ key);
				new_look.HairStyle := key;
				break;
			endif
		endforeach
	endif

endfunction

function FLBeardGump(who, byref new_look)

	var beardgump := GFCreateGump();
	GFResizePic(beardgump, 0, 0, 3600, 300, 420);
	GFResizePic(beardgump, 15, 15, 0x13BE, 270, 390);

	GFTextMid(beardgump, 15, 25, 270, 1890, "Escolha sua barba");
	GFAddButton(beardgump, 220, 370, 2128, 2129, 1, OK_BUTTON);

	GFResizePic(beardgump, 220, 55, 2620, 50, 280);

	var config := ReadConfigFile(":fls_core:config/hairtypes");
	var hair;
	if (GetObjProperty(who, "raca") == "anao")
		if (who.cmdlevel)
			SendSysMessage(who, "lendo barbas de anoes");
		endif
		hair := config["facialhairdwarf"];
	else
		hair := config["facialhair"];
	endif
	var hairgumps := config["hairgump"];
	GFSetRadioGroup(beardgump, 1);

	var y_pos := 65;
	foreach style in GetConfigStringArray(hair, "style")
		style := splitwords(style);
		var hairname := style[1];
		var hairgraphic := cint(style[2]);
		while (hairname["_"])
			hairname["_"] := " ";
		endwhile

		if (hairgraphic == new_look.BeardStyle)
			GFRadioButton(beardgump, 20, y_pos, 2151, 2154, 1, hairgraphic);
		else
			GFRadioButton(beardgump, 20, y_pos, 2151, 2154, 0, hairgraphic);
		endif

		GFTextline(beardgump, 55, y_pos+5, 1890, hairname);

		var gumppic := GetConfigString(hairgumps, style[1]);
		GFGumpPic(beardgump, 155, y_pos-65, gumppic);
		if (who.cmdlevel)
			SendSysMessage(who, "nome do estilo: " + style + " gump: " + gumppic + " id " + hairgraphic);
		endif
		y_pos := y_pos + 30;
	endforeach

	if (!new_look.BeardStyle || new_look.BeardStyle == 1)
		GFRadioButton(beardgump, 20, y_pos, 2151, 2154, 1, 1);
	else
		GFRadioButton(beardgump, 20, y_pos, 2151, 2154, 0, 1);
	endif
	GFTextline(beardgump, 55, y_pos+5, 1890, "Bald");

	var input := GFSendGump(who, beardgump);

	if (input[0] == OK_BUTTON)
		foreach key in (input.keys)
			if ( (key != OK_BUTTON) && (key > 0))
				new_look.BeardStyle := key;
				break;
			endif
		endforeach
	endif

endfunction


function FL_GetSkillID(skillname)

	case (skillname)
		"Alchemy": 		return 1;
		"Anatomy": 		return 2;
		"AnimalLore": 		return 3;
		"Archery": 		return 4;
		"ItemLore": 		return 5;
		"Axe": 			return 6;
		"Backstab": 		return 7;
		"Blacksmithy": 		return 8;
		"BowCraft": 		return 9;
		"Camping": 		return 10;
		"Carpentry":		return 11;
		"Cartography":		return 12;
		"Cooking": 		return 13;
		"DetectingHidden": 	return 14;
		"Disguise": 		return 15;
		"Evidance": 		return 16;
		"Farming": 		return 17;
		"Fishing": 		return 18;
		"Healing": 		return 19;
		"Hiding": 		return 20;
		"Lockpicking": 		return 21;
		"Lumberjacking": 	return 22;
		"Magery": 		return 23;
		"Mining": 		return 24;
		"Musicianship": 	return 25;
		"Poisoning": 		return 26;
		"Removetrap": 		return 27;
		"Skinning": 		return 28;
		"Stealth": 		return 29;
		"Tailoring": 		return 30;
		"AnimalTaming": 	return 31;
		"Tinkering": 		return 32;
		"Tracking": 		return 33;
		"Parry": 		return 34;
		"Fencing": 		return 35;
		"Swordsmanship": 	return 36;
		"MaceFighting": 	return 37;
		"Tactics": 		return 38;
		"Wrestling": 		return 39;
		"PoleArms": 		return 40;
		"Masonry": 		return 41;
		"Staff": 		return 42;
		"ResistingSpells": 	return 43;
	endcase


endfunction

function FL_GetSkillName(skill)

	case (skill)
		1: return "Alchemy";
		2: return "Anatomy";
		3: return "AnimalLore";
		4: return "Archery";
		5: return "ItemLore";
		6: return "Axe";
		7: return "Backstab";
		8: return "Blacksmithy";
		9: return "BowCraft";
		10: return "Camping";
		11: return "Carpentry";
		12: return "Cartography";
		13: return "Cooking";
		14: return "DetectingHidden";
		15: return "Disguise";
		16: return "Evidance";
		17: return "Farming";
		18: return "Fishing";
		19: return "Healing";
		20: return "Hiding";
		21: return "Lockpicking";
		22: return "Lumberjacking";
		23: return "Magery";
		24: return "Mining";
		25: return "Musicianship";
		26: return "Poisoning";
		27: return "Removetrap";
		28: return "Skinning";
		29: return "Stealth";
		30: return "Tailoring";
		31: return "AnimalTaming";
		32: return "Tinkering";
		33: return "Tracking";
		34: return "Parry";
		35: return "Fencing";
		36: return "Swordsmanship";
		37: return "MaceFighting";
		38: return "Tactics";
		39: return "Wrestling";
		40: return "PoleArms";
		41: return "Masonry";
		42: return "Staff";
		43: return "ResistingSpells";
	endcase

endfunction

function GetSkillPrice(skillid, byref primarias, byref secundarias, byref terciarias, byref quaternarias)

	var skillname := FL_GetSkillName(skillid);

	foreach skill in primarias
		if (skill == skillname)
			return 1;
		endif
	endforeach

	foreach skill in secundarias
		if (skill == skillname)
			return 2;
		endif
	endforeach

	foreach skill in terciarias
		if (skill == skillname)
			return 3;
		endif
	endforeach

	foreach skill in quaternarias
		if (skill == skillname)
			return 4;
		endif
	endforeach

	return 4;

endfunction

function GetTrueSkillName(skill)

	if (skill == "Evidance")
		return "EvaluatingIntelligence";
	elseif ( skill == "ItemLore")
		return "ArmsLore";
	elseif ( skill == "BowCraft");
		return "BowcraftFletching";
	else
		return skill;
	endif

endfunction

function GetSkillDescValue(skill)

	if (!skill || skill == error)
		return "";
	elseif (skill == 1)
		return "Ter";
	elseif (skill == 2)
		return "Sec";
	elseif (skill == 3)
		return "Pri";
	endif

endfunction
