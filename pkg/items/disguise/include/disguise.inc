include ":gumps:gumps";
include ":attributes:attributes";
include ":gumps:gumps_ex";
include ":gumps:requestGump";
include "include/say";
include ":tn:tngumps";
include "include/client";
include ":charactercreation:habilidades";
include ":timedscripts:timedScripts";

function removeDisguise(who)
	var original := GetObjProperty(who, "disfarce");
//	printtextabove(who, "original " + original);
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

	if (TemHabilidade(who, "Dissimulacao"))
		removeDisguise(who);
	else
		TS_LowerDuration(who, "kitdisfarce", -1);
	endif

	PrintText(who, "*removendo disfarce*");
	return 1;
endfunction

function escolherCabelo(who, byref disfarce, raca := 0)
	UnloadConfigFile(":fls_core:config/hairtypes");
	var config := ReadConfigFile(":fls_core:config/hairtypes");
	var hair;
	var sizey1 := 670;
	var sizey2 := 640;
	var sizey3 := 600;
	var sizeh := 270;
	var grande;
	// if (raca == "Aquilrano" || raca == "Orc")
	// 	hair := config["Aquilrano"];
	// 	sizey1 := 350;
	// 	sizey2 := 320;
	// 	sizey3 := 200;
	// elseif (raca =="Vanirio")
	// 	hair := config["Vanirio"];
	// 	sizey1 := 670;
	// 	sizey2 := 640;
	// 	sizey3 := 600;
	// 	sizeh := 600;
	// 	grande := 1;
	// elseif (raca == "Numajene")
	// 	hair := config["Numajene"];
	// 	sizey1 := 670;
	// 	sizey2 := 640;
	// 	sizey3 := 600;
	// 	sizeh := 600;
	// 	grande := 1;
	// elseif (raca == "Qualquer")
		hair := config["todos"];
		sizey1 := 670;
		sizey2 := 640;
		sizey3 := 600;
		sizeh := 600;
		grande := 1;
	// else
	// 	hair := config["head"];
	// 	sizeh := 600;
	// 	grande := 1;
	// endif
	var hairgumps := config["hairgump"];

	var gump := GfCreateGump();
	GFResizePic(gump, 0, 0, 3600, 300, sizey1);
	GFResizePic(gump, 15, 15, 0x13be, sizeh, sizey2);
	GFTextMid(gump, 15, 25, 270, 1153, "Escolha o cabelo");
	GFResizePic(gump, 220, 55, 2620, 50, sizey3);

	if (grande == 1)
		GFResizePic(gump, 520, 55, 2620, 50, sizey3-50);
	endif

	GFSetRadioGroup(gump, 1);
	var y_pos := 65;
	var x_pos := 20;
	var numColumn := 0;
	var hairnames := array;
	foreach style in GetConfigStringArray(hair, "style")
		style := splitwords(style);
		var hairname := style[1];
		var hairgraphic := cint(style[2]);
		while (hairname["_"])															hairname["_"] := " ";
		endwhile
		numColumn ++;
		if (numColumn > 19)
			x_pos := 320;
			if(numColumn == 20)
				y_pos := 65;
			endif
		endif
		if (hairgraphic == disfarce.barba)
			GFRadioButton(gump, x_pos, y_pos, 2151, 2154, 1, hairgraphic);
		else
			GFRadioButton(gump, x_pos, y_pos, 2151, 2154, 0, hairgraphic);
		endif
		hairnames.append(hairname);
		GFTextline(gump, x_pos + 55, y_pos+5, 1153, hairname);
		var gumppic := GetConfigString(hairgumps, style[1]);
		GFGumpPic(gump, x_pos+135, y_pos-45, gumppic);
		y_pos := y_pos + 30;

	endforeach

	if (!disfarce.barba || disfarce.barba == -1)
		GFRadioButton(gump, x_pos, y_pos, 2151, 2154, 1, 1);
	else
		GFRadioButton(gump, x_pos, y_pos, 2151, 2154, 0, 1);
	endif
	GFTextline(gump, x_pos+55, y_pos+5, 1153, "Bald");

	GfTextLine(gump, x_pos+45, y_pos+45, 1153, "Pronto");
	GfAddButton(gump, x_pos+25, y_pos+47, 2117, 2118, 1, 10);
	var input := GFSendGump(who, gump);
	if (input[0] == 10)
		SendSysMessageEx(who, "Voce escolheu o cabelo. ", SSM_INFO);
		foreach key in (input.keys)
			if (key != 10)
				disfarce.cabelo := key;
			endif
		endforeach
	else
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
	endif
endfunction

function escolheCorCabelo(who, byref disfarce, cordocabelo :=  0, raca :=0, retorna := 0)

	var gump2 := BuildHairDyeGump(cordocabelo, raca, who);
	var input2 := GFSendGump(who, gump2);

	if ( input2[0] == 0 || input2[0xA00001] )
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
		if (retorna)
			disfarce.corcabelo := gump2.colors[1];
		endif
		return 0;
	endif

  var selected := 0;
	foreach key in ( input2.keys )
		if ( key > 2 )
			disfarce.corcabelo := key;
      selected := 1;
			//SendSysMessageEx(who, "Voce escolheu a cor do cabelo.", SSM_INFO);
			break;
		endif
		sleepms(2);
	endforeach

  if (retorna)
  if (!selected)
    disfarce.corcabelo := gump2.colors[1];
  endif
  endif

endfunction

function BuildHairDyeGump(cur_color := 0, raca := 0, who)
	var gump := GFCreateGump();

	GFPage(gump, 0);
	//GFClosable(gump, 0);
	if (raca == RACA.HUMANO)
		GFResizePic(gump, 0, 0, GFCfgConst("Defaults", "BackGround"), 420, 550 * 2);
		GFResizePic(gump, 15, 15, GFCfgConst("Defaults", "ForeGround"), 120, 520 * 2);
		GFResizePic(gump, 140, 15, GFCfgConst("Defaults", "ForeGround"), 265, 486 * 2);
	else
		GFResizePic(gump, 0, 0, GFCfgConst("Defaults", "BackGround"), 420, 550);
		GFResizePic(gump, 15, 15, GFCfgConst("Defaults", "ForeGround"), 120, 520);
		GFResizePic(gump, 140, 15, GFCfgConst("Defaults", "ForeGround"), 265, 486);
	endif

	GFAddButton(gump, 190, 509, 2128, 2129, GF_CLOSE_BTN, 1);
	GFAddButton(gump, 265, 509, 2119, 2120, GF_CLOSE_BTN, 2);

	var chardata := GetObjProperty(who, "chardata");
	var cultura := chardata.culture;
	var color_cfg := ReadConfigFile(":hair:config/hairColors");
	var index_elem;
	if (cultura == "Björska")
	  index_elem := color_cfg["Bjorska"]; // Antigos thurioes
    elseif (raca == "Orc")
        index_elem := color_cfg["Orc"];
    elseif (raca == "Drow")
        index_elem := color_cfg["Drow"];
	elseif (raca == "Elfo")
        index_elem := color_cfg["Elfo"];
	elseif (raca == "Anão")
        index_elem := color_cfg["Anao"];
	elseif (raca == "Qualquer")
	  index_elem := color_cfg["Index"]; // Controls order colors appear.
	else
	  index_elem := color_cfg["Imperial"]; // Antigos zharkianos
	endif

	var categories := GetConfigStringArray(index_elem, "Category");

	// Build category buttons on the left first
	var index_y := 25;
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
		var color_y := 20;
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

function escolherBarba(who, byref disfarce)

	var gump := GfCreateGump();
	GFResizePic(gump, 0, 0, 3600, 300, 550);
	GFResizePic(gump, 15, 15, 0x13be, 270, 525);
	GFTextMid(gump, 15, 25, 270, 1153, "Escolha a barba");
	GFResizePic(gump, 220, 55, 2620, 50, 350);

	var config := ReadConfigFile(":fls_core:config/hairtypes");
	var hair := config["facialhair"];
	var hairgumps := config["hairgump"];

	GFSetRadioGroup(gump, 1);
	var y_pos := 65;

	var hairnames := array;
	foreach style in GetConfigStringArray(hair, "style")
		style := splitwords(style);
		var hairname := style[1];
		var hairgraphic := cint(style[2]);
		while (hairname["_"])															hairname["_"] := " ";
		endwhile
		if (hairgraphic == disfarce.barba)
			GFRadioButton(gump, 20, y_pos, 2151, 2154, 1, hairgraphic);
		else
			GFRadioButton(gump, 20, y_pos, 2151, 2154, 0, hairgraphic);
		endif
		hairnames.append(hairname);
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
	GFTextline(gump, 55, y_pos+5, 1153, "Bald");

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

function SkinGump(who, byref new_look, raca := 0, sempreretorna := 0)

	var gump := GFCreateGump();
	GFResizePic(gump, 0, 0, 3600, 300, 420);
	GFResizePic(gump, 15, 15, 0x13be, 270, 390);
	GFTextMid(gump, 15, 25, 270, 1153, "Cor da Pele");
	GfTextLine(gump, 45, 380, 1153, "Pronto");
	GfAddButton(gump, 25, 382, 2117, 2118, 1, 10);
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
	foreach color in colors;
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
			if (line > 10)
				GFAddButton(gump, 265, 340, 2648, 2649, GF_PAGE_BTN, page_num+1);
				page_num := page_num+1;
				GFPage(gump, page_num);
				GFAddButton(gump, 265, 60, 2650, 2651, GF_PAGE_BTN, page_num-1);
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
				//sendsysmessage(who, "cor de pele " + key);
			endif
		endforeach
    if (!selected)
      if (sempreretorna)
          new_look.corpele :=  (cint(colors[1]));
      endif
    endif
	else
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
     if (sempreretorna)
      new_look.corpele :=  (cint(colors[1]));
    endif
	endif
endfunction
