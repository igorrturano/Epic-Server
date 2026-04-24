include ":gumps:gumps";
include ":attributes:attributes";
include ":gumps:gumps_ex";
include ":gumps:requestGump";
include "include/say";
include ":tn:tngumps";
include "include/client";
include ":charactercreation:habilidades";
include ":timedscripts:timedScripts";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";
include "include/epicConstants";

function removeDisguise(who)
	var original := GetObjProperty(who, "disfarce");
//	PrintTextAbove(who, "original " + original);
	if (original == error)
		sendsysmessage(who, "Nao foi possivel achar sua configuracao inicial.");
		return 1;
	endif
	EraseObjProperty(who, "disfarce");
	EraseObjProperty(who, "FakeID");

	if (original.barba)
		var fakebarba := GetEquipmentByLayer(who, LAYER_BEARD);
		var newbarba := CreateItemAtLocation(who.x, who.y, who.z, original.barba, 1, who.realm);
		if (fakebarba)
			DestroyItem(fakebarba);
		endif
		if (newbarba)
			EquipItem(who, newbarba);
		endif
	endif

	if (original.corbarba)
		var barba := GetEquipmentByLayer(who, LAYER_BEARD);
		barba.color := original.corbarba;
	endif

	if (original.cabelo)
		var fakecabelo := GetEquipmentByLayer(who, LAYER_HAIR);
		var newhair := CreateItemAtLocation(who.x, who.y, who.z, original.cabelo, 1, who.realm);
		if (original.corcabelo)
			newhair.color := original.corcabelo;
			sendsysmessage(who, "" + original.corcabelo);
		endif
		if (fakecabelo)
			DestroyItem(fakecabelo);
		endif
		if (newhair)
			EquipItem(who, newhair);
		endif
	else
	//	sendsysmessage(who, " " + original.cabelo);
	endif

	if (original.corcabelo)
		var cabelo := GetEquipmentByLayer(who, LAYER_HAIR);
		cabelo.color := original.corcabelo;
	endif

	if (original.corpele)
		who.color := original.corpele;
		who.truecolor := original.corpele;
	endif

	if (original.sexo)
		who.graphic := original.sexo;
		who.trueobjtype := original.sexo;
	endif

	if (original.raca)
		SetObjProperty(who, "raca", original.raca);
		who.title_race := original.raca;
	endif

	if (original.aparencia != "")
		if (who.name == "Desconhecido")
			SetObjProperty(who, "realname", original.aparencia);
			SetName(who, original.aparencia);
		else
			SetName(who, who.name);
		endif
	else
		SetName(who, who.name);
	endif

	return 1;

endfunction


function removedisfarce(who)

	var original := GetObjProperty(who, "disfarce");
	if (original == error)
		SendSysMessageEx(who, "Voce nao esta disfarcado.", SSM_INFO);
		return 1;
	endif

	if (MobileWrapper(who).HasAbility(ABILITY_ID::NINJA_PATH))
		removeDisguise(who);
	else
		TS_LowerDuration(who, "kitdisfarce", -1);
	endif

	PrintTextAbove(who, "*removendo disfarce*");
	return 1;
endfunction

function escolherCabelo(who, byref disfarce, raca := 0, preview := 0)
	UnloadConfigFile(":fls_core:config/hairtypes");
	var config := ReadConfigFile(":fls_core:config/hairtypes");
	var hair := config["todos"];
	var hairStyles := GetConfigStringArray(hair, "style");

	var currentPage := 1;
	var selectedGump := "";

	while (1)
		var gump := GfCreateGump();
		GFClosable(gump, 0);
		GFGumpPic(gump, 17, 31, 0x9E6);
		GFResizePic(gump, 298, 100, 2620, 250, 400);
		GFGumpPic(gump, 328, 120, DisguisePaperdollBg(who.graphic));
		GFTextMid(gump, 15, 60, 770, 1153, "Escolha o cabelo");

		var HAIRS_PER_PAGE := 6;
		var HAIRS_PER_ROW := 2;
		var totalPages := ((hairStyles.size() - 1) / HAIRS_PER_PAGE) + 1;
		var start := (currentPage - 1) * HAIRS_PER_PAGE + 1;
		var end_idx := start + HAIRS_PER_PAGE - 1;
		if (end_idx > hairStyles.size())
			end_idx := hairStyles.size();
		endif

		for i := start to end_idx
			var style := SplitWords(hairStyles[i]);
			var hairname := style[1];
			var hairgraphic := CInt(style[2]);
			var gumppic := CInt(style[3]);

			var col := (i - start) % HAIRS_PER_ROW;
			var row := (i - start) / HAIRS_PER_ROW;
			var x_pos := 63 + (col * 119);
			var y_pos := 100 + (row * 141);

			GFResizePic(gump, x_pos, y_pos, 83, 100, 100);
			while (hairname["_"])
				hairname["_"] := " ";
			endwhile
			GFTextEntry(gump, x_pos + 10, y_pos + 12, 85, 20, 1153, hairname);

			if (selectedGump == gumppic || (i == 1 && selectedGump == ""))
				GFAddButton(gump, x_pos + 34, y_pos + 101, 2153, 2151, GF_CLOSE_BTN, hairgraphic);
			else
				GFAddButton(gump, x_pos + 34, y_pos + 101, 2151, 2153, GF_CLOSE_BTN, hairgraphic);
			endif

			GFGumpPic(gump, x_pos - 40, y_pos - 10, gumppic);
		endfor

		if (selectedGump != "")
			GFGumpPic(gump, 63 + 265, 120, selectedGump);
		endif

		if (totalPages > 1)
			if (currentPage < totalPages)
				GFAddButton(gump, 185, 520, 0x15E1, 0x15E5, GF_CLOSE_BTN, 1001);
			endif
			if (currentPage > 1)
				GFAddButton(gump, 125, 520, 0x15E3, 0x15E7, GF_CLOSE_BTN, 1002);
			endif
			GFTextMid(gump, 400, 536, 248, 1153, "Pagina " + currentPage + " de " + totalPages);
		endif

		GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 500);

		var input := GFSendGump(who, gump);
		var btn_value := input[0];

		case (btn_value)
			1001: currentPage += 1;
			1002: currentPage -= 1;
			500:
				SendSysMessageEx(who, "Voce escolheu o cabelo.", SSM_INFO);
				return 1;
			82:
				disfarce.cabelo := 82;
				selectedGump := "";
				if (preview)
					DestroyItem(GetEquipmentByLayer(who, LAYER_HAIR));
				endif
			default:
				if (btn_value)
					disfarce.cabelo := btn_value;
					selectedGump := DisguiseGetGumpForHair(hairStyles, btn_value);
					if (preview)
						DestroyItem(GetEquipmentByLayer(who, LAYER_HAIR));
						var newHair := CreateItemAtLocation(who.x, who.y, who.z, btn_value, 1, who.realm);
						if (disfarce.corcabelo && disfarce.corcabelo != -1)
							newHair.color := disfarce.corcabelo;
						endif
						EquipItem(who, newHair);
					endif
				else
					SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
					return 0;
				endif
		endcase
	endwhile
endfunction

function DisguisePaperdollBg(graphic)
	case (graphic)
		400:  return 0xC;
		401:  return 0xD;
		1828: return 54562;
		1829: return 54563;
		1830: return 0xE;
		1831: return 0xF;
		1832: return 54596;
		1833: return 54595;
		1834: return 54533;
		1835: return 54534;
		default: return 0xC;
	endcase
endfunction

function DisguiseGetGumpForHair(hairStyles, hairgraphic)
	foreach style in hairStyles
		var parts := SplitWords(style);
		if (CInt(parts[2]) == hairgraphic)
			return CInt(parts[3]);
		endif
	endforeach
	return "";
endfunction

function BuildHairDyeGump(cur_color := 0, raca := 0, who)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 0, 0, 0x9E6); // Fundo do CreationUtils
    GFGumpPic(gump, 298, 100, 0xC); // Imagem central

	GFAddButton(gump, 190, 680, 2128, 2129, GF_CLOSE_BTN, 1);
	GFAddButton(gump, 265, 680, 2119, 2120, GF_CLOSE_BTN, 2);

	var chardata := GetObjProperty(who, "chardata");
	var cultura := chardata.culture;
	var color_cfg := ReadConfigFile(":hair:config/hairColors");
	var index_elem;
	if (cultura == "Björska")
		index_elem := color_cfg["Bjorska"];
	elseif (raca == "Orc")
		index_elem := color_cfg["Orc"];
	elseif (raca == "Drow")
		index_elem := color_cfg["Drow"];
	elseif (raca == "Elfo")
		index_elem := color_cfg["Elfo"];
	elseif (raca == "Anão")
		index_elem := color_cfg["Anao"];
	elseif (raca == "Qualquer")
		index_elem := color_cfg["Index"];
	else
		index_elem := color_cfg["Imperial"];
	endif

	var categories := GetConfigStringArray(index_elem, "Category");

	// Build category buttons on the left first
	var index_y := 65;
	foreach category in ( categories )
		GFAddButton(gump, 20, index_y+3, 5224, 5224, GF_PAGE_BTN, _category_iter);
		var color_elem := color_cfg[category];
		GFTextLine(gump, 42, index_y, color_elem.TitleColor, category);

		index_y := index_y + 20;
		sleepms(2);
	endforeach

	// Build colors inside each category
	foreach category in ( categories )
		GFPage(gump, _category_iter);
		var color_elem := color_cfg[category];

		var color_x := 150;
		var color_y := 60;
		var side := 1;
		var colors := GetConfigStringArray(color_elem, "Color");
		gump.+colors := colors;
		foreach color in colors
			color := CInt(color);
			GFRadioButton(gump, color_x, color_y, 210, 211, (color == cur_color), color);
			GFTextLine(gump, color_x+20, color_y, color-1, category);
			if ( side == 1 )
				color_x := 290;
				side := 0;
			else
				color_x := 150;
				color_y := color_y+20;
				side := 1;
			endif

			sleepms(2);
		endforeach
		sleepms(2);
	endforeach

	return gump;
endfunction

function escolherBarba(who, byref disfarce, preview := 0)
	UnloadConfigFile(":fls_core:config/hairtypes");
	var config := ReadConfigFile(":fls_core:config/hairtypes");
	var hair := (who.getRace() == ANAO) ? config["dwarwen_beard"] : config["beard"];
	var beardStyles := GetConfigStringArray(hair, "style");

	if (preview)
		return escolherBarbaPreview(who, disfarce, beardStyles);
	endif

	var hairgumps := config["hairgump"];
    var gump := GfCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 0, 0, 0x9E6);
    GFResizePic(gump, 15, 15, 0x13be, 770, 740);
    GFGumpPic(gump, 298, 100, 0xC);
    GFTextMid(gump, 15, 25, 770, 1153, "Escolha a barba");

	GFSetRadioGroup(gump, 1);
	var y_pos := 65;

	foreach style in beardStyles
		style := splitwords(style);
		var hairname := style[1];
		var hairgraphic := cint(style[2]);
		while (hairname["_"])
			hairname["_"] := " ";
		endwhile
		if (hairgraphic == disfarce.barba)
			GFRadioButton(gump, 20, y_pos, 2151, 2154, 1, hairgraphic);
		else
			GFRadioButton(gump, 20, y_pos, 2151, 2154, 0, hairgraphic);
		endif
		GFTextline(gump, 55, y_pos+5, 1153, hairname);
		var gumppic := GetConfigString(hairgumps, style[1]);
		GFGumpPic(gump, 155, y_pos-65, gumppic);
		y_pos := y_pos + 30;
	endforeach

	if (!disfarce.barba || disfarce.barba == -1)
		GFRadioButton(gump, 20, y_pos, 2151, 2154, 1, 1);
	else
		GFRadioButton(gump, 20, y_pos, 2151, 2154, 0, 1);
	endif
	GFTextline(gump, 55, y_pos+5, 1153, "Sem barba");

	GfTextLine(gump, 45, y_pos+85, 1153, "Pronto");
	GfAddButton(gump, 25, y_pos+87, 2117, 2118, 1, 10);
	var input := GFSendGump(who, gump);
	if (input[0] == 10)
		SendSysMessageEx(who, "Voce escolheu a barba. ", SSM_INFO);
		foreach key in (input.keys)
			if (key != 10)
				disfarce.barba := key;
			endif
		endforeach
	else
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
	endif
endfunction

function escolherBarbaPreview(who, byref disfarce, beardStyles)
	var currentPage := 1;

	while (1)
		var gump := GfCreateGump();
		GFClosable(gump, 0);
		GFGumpPic(gump, 17, 31, 0x9E6);
		GFResizePic(gump, 298, 100, 2620, 250, 400);
		GFGumpPic(gump, 328, 120, DisguisePaperdollBg(who.graphic));
		GFTextMid(gump, 15, 60, 770, 1153, "Escolha a barba");

		var BEARDS_PER_PAGE := 6;
		var BEARDS_PER_ROW := 2;
		var totalPages := ((beardStyles.size() - 1) / BEARDS_PER_PAGE) + 1;
		var start := (currentPage - 1) * BEARDS_PER_PAGE + 1;
		var end_idx := start + BEARDS_PER_PAGE - 1;
		if (end_idx > beardStyles.size())
			end_idx := beardStyles.size();
		endif

		for i := start to end_idx
			var style := SplitWords(beardStyles[i]);
			var beardname := style[1];
			var beardgraphic := CInt(style[2]);
			var gumppic := CInt(style[3]);

			var col := (i - start) % BEARDS_PER_ROW;
			var row := (i - start) / BEARDS_PER_ROW;
			var x_pos := 63 + (col * 119);
			var y_pos := 100 + (row * 141);

			GFResizePic(gump, x_pos, y_pos, 83, 100, 100);
			while (beardname["_"])
				beardname["_"] := " ";
			endwhile
			GFTextEntry(gump, x_pos + 10, y_pos + 12, 85, 20, 1153, beardname);

			if (beardgraphic == disfarce.barba)
				GFAddButton(gump, x_pos + 34, y_pos + 101, 2153, 2151, GF_CLOSE_BTN, beardgraphic);
			else
				GFAddButton(gump, x_pos + 34, y_pos + 101, 2151, 2153, GF_CLOSE_BTN, beardgraphic);
			endif

			GFGumpPic(gump, x_pos - 40, y_pos - 10, gumppic);
		endfor

		if (totalPages > 1)
			if (currentPage < totalPages)
				GFAddButton(gump, 185, 520, 0x15E1, 0x15E5, GF_CLOSE_BTN, 1001);
			endif
			if (currentPage > 1)
				GFAddButton(gump, 125, 520, 0x15E3, 0x15E7, GF_CLOSE_BTN, 1002);
			endif
			GFTextMid(gump, 400, 536, 248, 1153, "Pagina " + currentPage + " de " + totalPages);
		endif

		GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 500);

		var input := GFSendGump(who, gump);
		var btn_value := input[0];

		case (btn_value)
			1001: currentPage += 1;
			1002: currentPage -= 1;
			500:
				SendSysMessageEx(who, "Voce escolheu a barba.", SSM_INFO);
				return 1;
			82:
				disfarce.barba := 82;
				DestroyItem(GetEquipmentByLayer(who, LAYER_BEARD));
			default:
				if (btn_value)
					disfarce.barba := btn_value;
					DestroyItem(GetEquipmentByLayer(who, LAYER_BEARD));
					var newBeard := CreateItemAtLocation(who.x, who.y, who.z, btn_value, 1, who.realm);
					if (disfarce.corcabelo && disfarce.corcabelo != -1)
						newBeard.color := disfarce.corcabelo;
					endif
					EquipItem(who, newBeard);
				else
					SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
					return 0;
				endif
		endcase
	endwhile
endfunction

function SkinGump(who, byref new_look, raca := 0, sempreretorna := 0)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 0, 0, 0x9E6); // Fundo do CreationUtils
    GFResizePic(gump, 15, 15, 0x13be, 770, 740);
    GFGumpPic(gump, 298, 100, 0xC); // Imagem central
    GFTextMid(gump, 15, 25, 770, 1153, "Cor da Pele");
	GfTextLine(gump, 45, 720, 1153, "Pronto");
	GfAddButton(gump, 25, 722, 2117, 2118, 1, 10);
	var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
	if ( !skin_cfg )
		SendSysMessage(who, "Error: Could not open :fls_core:config/skincolors.cfg -> "+skin_cfg.errortext);
	endif
	if  (raca == "Elfo")
		skin_cfg := skin_cfg["elfo"];
	elseif (raca == "Drow")
		skin_cfg := skin_cfg["drow"];
	elseif (raca == "Orc")
		skin_cfg := skin_cfg["orc"];
	else
		skin_cfg := skin_cfg["list"];
	endif
	GFPage(gump, 1);
	var y_pos := 60;
	var x_pos := 20;
	var place := 1;
	var page_num := 1;
	var line := 1;

	var colors := GetConfigStringArray(skin_cfg, "color");
	foreach color in colors
		color := CInt(color);
		if (color == new_look.corpele)
			GFRadioButton(gump, x_pos, y_pos, 2151, 2154, 1, color);
		else
			GFRadioButton(gump, x_pos, y_pos, 2151, 2154, 0, color);
		endif
		GFTextline(gump, x_pos+35, y_pos+5, color-1, "Skin");

		if (place < 3)
			x_pos := x_pos+80;
			place := place+1;
		else
			place := 1;
			x_pos := 20;
			y_pos := y_pos+30;
			line := line+1;
			if (line > 20)
				GFAddButton(gump, 735, 690, 2648, 2649, GF_PAGE_BTN, page_num+1);
				page_num := page_num+1;
				GFPage(gump, page_num);
				GFAddButton(gump, 735, 60, 2650, 2651, GF_PAGE_BTN, page_num-1);
				line := 1;
				y_pos := 60;
			endif
		endif
	endforeach

	var input := GFSendGump(who, gump);
	if (input[0] == 10)
		SendSysMessageEx(who, "Voce escolheu da pele. " + input.keys, SSM_INFO);
		var selected := 0;
		foreach key in (input.keys)
			if (key != 10 && key != 0 )
				new_look.corpele := key;
				selected := 1;
			endif
		endforeach
		if (!selected && sempreretorna)
			new_look.corpele := (cint(colors[1]));
		endif
	else
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
		if (sempreretorna)
			new_look.corpele := (cint(colors[1]));
		endif
	endif
endfunction

function escolheCorCabelo(who, byref disfarce, cordocabelo := 0, raca := 0, retorna := 0, preview := 0)
    if (preview)
        return escolheCorCabeloPreview(who, disfarce, cordocabelo, raca);
    endif

    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 0, 0, 0x9E6);
    GFGumpPic(gump, 298, 100, 0xC);

    GFTextMid(gump, 15, 25, 770, 1153, "Escolha a Cor do Cabelo");

    var chardata := GetObjProperty(who, "chardata");
    var cultura := chardata.culture;
    var color_cfg := ReadConfigFile(":hair:config/hairColors");
    var index_elem := GetColorConfigElement(color_cfg, cultura, raca);
    var categories := GetConfigStringArray(index_elem, "Category");

    var y_pos := 60;
    var x_pos := 20;
    var page_num := 1;
    GFPage(gump, page_num);

    foreach category in (categories)
        var color_elem := color_cfg[category];
        var colors := GetConfigStringArray(color_elem, "Color");

        foreach color in colors
            color := CInt(color);
            if (color == cordocabelo)
                GFRadioButton(gump, x_pos, y_pos, 0x9C4F, 0x9C4E, 1, color);
            else
                GFRadioButton(gump, x_pos, y_pos, 0x9C4E, 0x9C4F, 0, color);
            endif

            GFTextline(gump, x_pos+35, y_pos, color-1, category);

            y_pos += 30;
            if (y_pos > 650)
                GFAddButton(gump, 735, 640, 2648, 2649, GF_PAGE_BTN, page_num+1);
                page_num += 1;
                GFPage(gump, page_num);
                GFAddButton(gump, 735, 60, 2650, 2651, GF_PAGE_BTN, page_num-1);
                y_pos := 60;
            endif
        endforeach
    endforeach

    GFAddButton(gump, 350, 670, 2128, 2129, GF_CLOSE_BTN, 1);

    var input := GFSendGump(who, gump);

    if (input[0] == 0 || input[0xA00001])
        SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
        if (retorna)
            disfarce.corcabelo := cordocabelo;
        endif
        return 0;
    endif

    var selected := 0;
    foreach key in (input.keys)
        if (key > 1)
            disfarce.corcabelo := key;
            selected := 1;
            break;
        endif
    endforeach

    if (retorna && !selected)
        disfarce.corcabelo := cordocabelo;
    endif

    return 1;
endfunction

function escolheCorCabeloPreview(who, byref disfarce, cordocabelo, raca)
    var chardata := GetObjProperty(who, "chardata");
    var cultura := chardata.culture;
    var color_cfg := ReadConfigFile(":hair:config/hairColors");
    var index_elem := GetColorConfigElement(color_cfg, cultura, raca);
    var categories := GetConfigStringArray(index_elem, "Category");

    var selectedColor := cordocabelo;

    while (1)
        var gump := GFCreateGump();
        GFClosable(gump, 0);
        GFGumpPic(gump, 0, 0, 0x9E6);
        GFGumpPic(gump, 298, 100, 0xC);
        GFTextMid(gump, 15, 25, 770, 1153, "Escolha a Cor do Cabelo");

        // Categorias no lado esquerdo (pagina 0 = visivel em todas)
        var index_y := 65;
        foreach category in (categories)
            GFAddButton(gump, 20, index_y+3, 5224, 5224, GF_PAGE_BTN, _category_iter);
            var color_elem := color_cfg[category];
            GFTextLine(gump, 42, index_y, color_elem.TitleColor, category);
            index_y += 20;
        endforeach

        // Confirmar (pagina 0)
        GFAddButton(gump, 350, 670, 4005, 4007, GF_CLOSE_BTN, 500);

        // Cores por categoria
        foreach category in (categories)
            GFPage(gump, _category_iter);
            var color_elem := color_cfg[category];
            var color_x := 150;
            var color_y := 60;
            var side := 1;
            var colors := GetConfigStringArray(color_elem, "Color");
            foreach color in colors
                color := CInt(color);
                if (color == selectedColor)
                    GFAddButton(gump, color_x, color_y, 211, 210, GF_CLOSE_BTN, color);
                else
                    GFAddButton(gump, color_x, color_y, 210, 211, GF_CLOSE_BTN, color);
                endif
                GFTextLine(gump, color_x+20, color_y, color-1, category);
                if (side == 1)
                    color_x := 290;
                    side := 0;
                else
                    color_x := 150;
                    color_y += 20;
                    side := 1;
                endif
            endforeach
        endforeach

        var input := GFSendGump(who, gump);
        var btn := input[0];

        if (btn == 500)
            disfarce.corcabelo := selectedColor;
            return 1;
        elseif (btn == 0)
            // Cancelado - reverter cor original
            var hair := GetEquipmentByLayer(who, LAYER_HAIR);
            if (hair)
                hair.color := cordocabelo;
            endif
            disfarce.corcabelo := cordocabelo;
            return 0;
        else
            // Clicou uma cor - aplicar imediatamente
            selectedColor := btn;
            disfarce.corcabelo := btn;
            var hair := GetEquipmentByLayer(who, LAYER_HAIR);
            if (hair)
                hair.color := btn;
            endif
        endif
    endwhile
endfunction

function GetColorConfigElement(color_cfg, cultura, raca)
    var index_elem;
    if (raca == "Qualquer")
        index_elem := color_cfg["Index"];
    elseif (cultura == "Björska")
        index_elem := color_cfg["Bjorska"];
    elseif (raca == "Orc")
        index_elem := color_cfg["Orc"];
    elseif (raca == "Drow")
        index_elem := color_cfg["Drow"];
    elseif (raca == "Elfo")
        index_elem := color_cfg["Elfo"];
    elseif (raca == "Anão")
        index_elem := color_cfg["Anao"];
    else
        index_elem := color_cfg["Imperial"];
    endif
    return index_elem;
endfunction