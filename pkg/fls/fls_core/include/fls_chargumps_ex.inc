
use uo;
use cfgfile;

include ":gumps:gumps";
include "include/arrays";
include ":fls_core:fls_characters";

function FLHairDye(who, byref new_look)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, 346, 56, 192, 40, "<BASEFONT Size=55 Color=#999999>Escolha a Cor do Cabelo", 0, 0);
    GFHTMLArea(gump, 345, 55, 192, 40, "<BASEFONT Size=55 Color=#ffffff>Escolha a Cor do Cabelo", 0, 0);

    var skin_cfg := ReadConfigFile(":fls_core:config/racas");
    if (!skin_cfg)
        SendSysMessage(who, "Error: Could not open :fls_core:config/racas.cfg -> "+skin_cfg.errortext);
        return;
    endif
    skin_cfg := skin_cfg[cstr(GetObjProperty(who, "raca"))];

    // Adicionar uma imagem central representativa (se disponível)
    GFGumpPic(gump, 298, 100, 0x2050); // Substitua 0x2050 pelo ID da imagem apropriada

    var y_pos := 200;
    var x_pos := 100;
    var place := 1;
    var page_num := 1;
    GFPage(gump, page_num);

    foreach color in GetConfigStringArray(skin_cfg, "HairColor")
        color := CInt(color);
        if (color == new_look.HairColor)
            GFRadioButton(gump, x_pos, y_pos, 0x9C4F, 0x9C4E, 1, color);
        else
            GFRadioButton(gump, x_pos, y_pos, 0x9C4E, 0x9C4F, 0, color);
        endif

        GFTextline(gump, x_pos+35, y_pos, color-1, "Hair");

        if (place % 2 == 0)
            x_pos := 100;
            y_pos += 30;
        else
            x_pos += 300;
        endif
        place += 1;

        if (place > 12 && place % 2 == 1)
            GFAddButton(gump, 543, 300, 0x9903, 0x9904, GF_PAGE_BTN, page_num+1);
            page_num += 1;
            GFPage(gump, page_num);
            GFAddButton(gump, 273, 300, 0x9909, 0x9910, GF_PAGE_BTN, page_num-1);
            y_pos := 200;
            x_pos := 100;
            place := 1;
        endif
    endforeach

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 1);

    var input := GFSendGump(who, gump);

    if (input[0] == 1)
        foreach key in (input.keys)
            if (key != 1 && key > 0)
                new_look.HairColor := key;
                break;
            endif
        endforeach
    endif
endfunction

function FLSkinGump(who, byref new_look)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, 346, 56, 192, 40, "<BASEFONT Size=55 Color=#999999>Escolha a Cor de Pele", 0, 0);
    GFHTMLArea(gump, 345, 55, 192, 40, "<BASEFONT Size=55 Color=#ffffff>Escolha a Cor de Pele", 0, 0);

    var skin_cfg := ReadConfigFile(":fls_core:config/racas");
    if (!skin_cfg)
        SendSysMessage(who, "Error: Could not open :fls_core:config/racas.cfg -> "+skin_cfg.errortext);
        return;
    endif
    skin_cfg := skin_cfg[cstr(GetObjProperty(who, "raca"))];

    // Adicionar uma imagem central representativa (se disponível)
    GFGumpPic(gump, 298, 100, 0x2051); // Substitua 0x2051 pelo ID da imagem apropriada

    var y_pos := 200;
    var x_pos := 100;
    var place := 1;
    var page_num := 1;
    GFPage(gump, page_num);

    foreach color in GetConfigStringArray(skin_cfg, "SkinColor")
        color := CInt(color);
        if (color == new_look.SkinColor)
            GFRadioButton(gump, x_pos, y_pos, 0x9C4F, 0x9C4E, 1, color);
        else
            GFRadioButton(gump, x_pos, y_pos, 0x9C4E, 0x9C4F, 0, color);
        endif

        GFTextline(gump, x_pos+35, y_pos, color-1, "Skin");

        if (place % 2 == 0)
            x_pos := 100;
            y_pos += 30;
        else
            x_pos += 300;
        endif
        place += 1;

        if (place > 12 && place % 2 == 1)
            GFAddButton(gump, 543, 300, 0x9903, 0x9904, GF_PAGE_BTN, page_num+1);
            page_num += 1;
            GFPage(gump, page_num);
            GFAddButton(gump, 273, 300, 0x9909, 0x9910, GF_PAGE_BTN, page_num-1);
            y_pos := 200;
            x_pos := 100;
            place := 1;
        endif
    endforeach

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 1);

    var input := GFSendGump(who, gump);

    if (input[0] == 1)
        foreach key in (input.keys)
            if (key != 1 && key > 0)
                new_look.SkinColor := key;
                break;
            endif
        endforeach
    endif
endfunction

function FLHairGump(who, byref new_look)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, 346, 56, 192, 40, "<BASEFONT Size=55 Color=#999999>Escolha seu Cabelo", 0, 0);
    GFHTMLArea(gump, 345, 55, 192, 40, "<BASEFONT Size=55 Color=#ffffff>Escolha seu Cabelo", 0, 0);

    var config := ReadConfigFile(":fls_core:config/hairtypes");
    var hair := (GetObjProperty(who, "raca") == "anao" || GetObjProperty(who, "raca") == "hobbit") ? config["headdwarf"] : config["head"];
    var hairgumps := config["hairgump"];
    GFSetRadioGroup(gump, 1);

    var y_pos := 200;
    var page_num := 1;
    GFPage(gump, page_num);

    foreach style in GetConfigStringArray(hair, "style")
        style := splitwords(style);
        var hairname := style[1];
        var hairgraphic := cint(style[2]);
        hairname["_"] := " ";

        if (hairgraphic == new_look.HairStyle)
            GFRadioButton(gump, 100, y_pos, 0x9C4F, 0x9C4E, 1, hairgraphic);
        else
            GFRadioButton(gump, 100, y_pos, 0x9C4E, 0x9C4F, 0, hairgraphic);
        endif

        GFTextline(gump, 135, y_pos, 1890, hairname);

        var gumppic := GetConfigStringArray(hairgumps, style[1]);
        gumppic := splitwords(gumppic);
        GFGumpPic(gump, 298, 100, gumppic[new_look.Gender+2]);

        y_pos += 40;

        if (y_pos > 500)
            GFAddButton(gump, 543, 300, 0x9903, 0x9904, GF_PAGE_BTN, page_num+1);
            page_num += 1;
            GFPage(gump, page_num);
            GFAddButton(gump, 273, 300, 0x9909, 0x9910, GF_PAGE_BTN, page_num-1);
            y_pos := 200;
        endif
    endforeach

    if (!new_look.HairStyle || new_look.HairStyle == 1)
        GFRadioButton(gump, 100, y_pos, 0x9C4F, 0x9C4E, 1, 1);
    else
        GFRadioButton(gump, 100, y_pos, 0x9C4E, 0x9C4F, 0, 1);
    endif
    GFTextline(gump, 135, y_pos, 1890, "Bald");

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 1);

    var input := GFSendGump(who, gump);

    if (input[0] == 1)
        foreach key in (input.keys)
            if (key != 1 && key > 0)
                new_look.HairStyle := key;
                break;
            endif
        endforeach
    endif
endfunction

function FLBeardGump(who, byref new_look)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, 346, 56, 192, 40, "<BASEFONT Size=55 Color=#999999>Escolha sua Barba", 0, 0);
    GFHTMLArea(gump, 345, 55, 192, 40, "<BASEFONT Size=55 Color=#ffffff>Escolha sua Barba", 0, 0);

    var config := ReadConfigFile(":fls_core:config/hairtypes");
    var hair := (GetObjProperty(who, "raca") == "anao") ? config["facialhairdwarf"] : config["facialhair"];
    var hairgumps := config["hairgump"];
    GFSetRadioGroup(gump, 1);

    var y_pos := 200;
    var page_num := 1;
    GFPage(gump, page_num);

    foreach style in GetConfigStringArray(hair, "style")
        style := splitwords(style);
        var hairname := style[1];
        var hairgraphic := cint(style[2]);
        hairname["_"] := " ";

        if (hairgraphic == new_look.BeardStyle)
            GFRadioButton(gump, 100, y_pos, 0x9C4F, 0x9C4E, 1, hairgraphic);
        else
            GFRadioButton(gump, 100, y_pos, 0x9C4E, 0x9C4F, 0, hairgraphic);
        endif

        GFTextline(gump, 135, y_pos, 1890, hairname);

        var gumppic := GetConfigString(hairgumps, style[1]);
        GFGumpPic(gump, 298, 100, gumppic);

        y_pos += 40;

        if (y_pos > 500)
            GFAddButton(gump, 543, 300, 0x9903, 0x9904, GF_PAGE_BTN, page_num+1);
            page_num += 1;
            GFPage(gump, page_num);
            GFAddButton(gump, 273, 300, 0x9909, 0x9910, GF_PAGE_BTN, page_num-1);
            y_pos := 200;
        endif
    endforeach

    if (!new_look.BeardStyle || new_look.BeardStyle == 1)
        GFRadioButton(gump, 100, y_pos, 0x9C4F, 0x9C4E, 1, 1);
    else
        GFRadioButton(gump, 100, y_pos, 0x9C4E, 0x9C4F, 0, 1);
    endif
    GFTextline(gump, 135, y_pos, 1890, "Bald");

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 1);

    var input := GFSendGump(who, gump);

    if (input[0] == 1)
        foreach key in (input.keys)
            if (key != 1 && key > 0)
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
