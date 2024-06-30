// //$Id: craftGumps.inc 740 2005-10-31 06:28:43Z muaddiblsd $

include ":crafting:craftingUtil";

var craft_gumps := DFOpenDataFile(":crafting:CraftGumps", DF_CREATE);
var menu_config := ReadConfigFile( ":crafting:config/craftConfig" );
var arinfo   := array;
var arcnt    := 61500;
var craft_settings;
var atc := 1;
var PageNum := 1;
var recipe_skill;
var skillitens := array;
// Gump backgrounds - Used in various functions - easier to use globally.
var BKGND_SILVERTRIM := GFCfgConst("Defaults", "BackGround");
const EL_BUG := "ERROR 666";

// This function is the main backbone for building the Crafting Menu.
function sendCraftGump(who, skill_name)

	unloadconfigfile(":blacksmithy:blacksmithy");
	var menu_file    := menu_config[skill_name].MenuFile;
	var skill_config := ReadConfigFile(menu_file);
	craft_skill      := menu_config[skill_name].Attribute;
	recipe_skill := craft_skill;
	display_skill    := menu_config[skill_name].Name;
	// Craft Cprop is a dictionary storing the information for each crafting skill
	craft_settings := GetObjProperty(who, "Craft");
	if( TypeOf(craft_settings) != "Dictionary" )
		craft_settings := cBaseCraftDict();
		SetObjProperty(who, "Craft", craft_settings);
	endif

	while( 1 )

		craftL := craft_settings[skill_name].last;
		var craft_gump := Craft_IndexGump(who, skill_name);
		if( !craft_gump )
			exit;
		endif

		var res := GFSendGump(who, craft_gump);

		// Exit the menu, return 0 so crafting script knows it was cancled
		if(res[0] == 1)
			return 0;
		endif

		var the_objtype := res[0];

		// Handle setting up of Objtype if they pressed "Last Item". Returns with
		// an error if you haven't crafted anything with the skill yet.
		if( res[61450] )
			the_objtype := craftL;
			if( !the_objtype || (TypeOf(the_objtype) == "Struct") )
				SendSysMessage(who, "Voce nao fez nada antes.");
				return 0;
			endif
		endif

		// This decides if you pressed the Info button, if so, sends info gump
		if( (res[0] >= 61500) && (res[0] <= 62000) )
			the_objtype := arinfo[(res[0] - 61499)];
			var infores := GFCraftInfoPage(who, the_objtype, skill_config);
			case(infores)
			  0: continue;
			  default: the_objtype := infores;
			endcase
		endif

		craft_settings[skill_name].last := the_objtype;
		SetObjProperty(who, "Craft", craft_settings);

		// Sets the amount you had entered into an array to return to crafting
		atc := CInt(GFExtractData(res, atc));
		if(!atc || CInt(atc) < 1)
			atc := 1;
		elseif(atc > CInt(menu_config["Setup"].MaxAmount))
			SendSysMessage(who, "Voce so pode fazer algo entre 1 e " + menu_config["Setup"].MaxAmount + " itens.");
			return 0;
		endif
		craft_settings[skill_name].amount := atc;
		SetObjProperty(who, "Craft", craft_settings);

		// Build the return array and give it to crafting.
		var minfo := array;
		minfo.append(the_objtype);
		minfo.append(atc);
		minfo.append((res[61451]));
		minfo.append((res[61454]));
		return minfo;

	endwhile

endfunction


function Craft_IndexGump(who, skill_name)

	var menu_file    := menu_config[skill_name].MenuFile;
	var skill_config := ReadConfigFile(menu_file);

	// Now let's get the amount, based on the menu file) from the Cprop.
	atc := craft_settings[skill_name].amount;
	// If it == nothing, or is an error, set it to 1. Error should never occur, but, just in case.
	if( !atc || TypeOf(atc) != "Integer" )
		atc := 1;
	endif

	// Resets the array for objtypes within the menu. Used with
	// The info page and it's buttons.
	arinfo  := array;
	// Button counter for the Info buttons for items. This way, we can seperate craft now buttons
	// from the info buttons. Just remember to make sure any item's button # should NOT be within
	// the 61500 to 62000 range. If you have 50 items, button range will be 61500 to 61550. So please
	// take care with your items.
	arcnt   := 61500;

	var gump := GFCreateGump(50, 25);
	GFResizePic(gump, 2, 0, BKGND_SILVERTRIM, 522, 440);
	GFGumpPic(gump, 10, 10, 2624);
	GFGumpPic(gump, 10, 205, 2624);
	GFGumpPic(gump, 10, 225, 2624);
	GFGumpPic(gump, 250, 10, 2624);
	GFGumpPic(gump, 250, 205, 2624);
	GFGumpPic(gump, 250, 225, 2624);
	GFAddAlphaRegion(gump, 10, 10, 505, 420);
	GFGumpPic(gump, 10, 33, 5055);
	GFGumpPic(gump, 90, 33, 5055);
	GFGumpPic(gump, 10, 43, 5061);
	GFGumpPic(gump, 90, 43, 5061);
	GFGumpPic(gump, 10, 285, 5055);
	GFGumpPic(gump, 90, 285, 5055);
	GFGumpPic(gump, 10, 330, 5055);
	GFGumpPic(gump, 90, 330, 5055);

	GFHTMLArea(gump, 10, 12, 510, 20, "<basefont color=#ffffff>"+CStr(GetConfigString(skill_config["Index"], "Title")));
	GFHTMLArea(gump, 50, 405, 150, 18, "<basefont color=#ffffff>Fechar");
	GFAddButton(gump, 15, 402, 4017, 4019, GF_CLOSE_BTN, 1);

	if( menu_config["Setup"].MakeLast == 1 )
		GFHTMLArea(gump, 305, 405, 150, 18, "<basefont color=#ffffff>Fazer Ultimo");
		GFAddButton(gump, 270, 402, 4005, 4006, GF_CLOSE_BTN, 61450);
	endif

	GFHTMLArea(gump, 410, 405, 75, 20, "<basefont color=#ffffff>Amount");
	GFGumpPic(gump, 455, 402, 2443);
	atc := GFTextEntry(gump, 465, 405, 100, 28, 1152, atc);

	GFHTMLArea(gump, 10, 302, 150, 20, "<basefont color=#ffffff><center>Noticias</center>");
	GFHTMLArea(gump, 10, 37, 200, 20, "<basefont color=#ffffff><center>Categorias</center>");
	GFHTMLArea(gump, 215, 37, 305, 20, "<basefont color=#ffffff><center>Selecoes</center>");

	if( menu_config[skill_name].Repair == 1 )
		GFHTMLArea(gump, 305, 385, 150, 18, "<basefont color=#ffffff>Reparar Item");
		GFAddButton(gump, 270, 382, 4005, 4006, GF_CLOSE_BTN, 61451);
	endif

	// This foreach builds all the pages for Categories and Selections
	// IndexY = the Y position for lines in the Categories.
	var IndexY := 63;
	// Page Number counter to keep track of pages while building the gump
	PageNum := 1;
  	var indexes := GetConfigStringArray(skill_config["Index"], "Group");
  	indexes.append("Recipes");
	foreach MenuName in indexes
		// Add the Category to page 0 for background
		GFPage(gump, "0");
		GFAddButton(gump, 15, (IndexY-3), 4005, 4006, GF_PAGE_BTN, PageNum);
		while (MenuName["_"])
			MenuName["_"] := " ";
			sleepms(1);
		endwhile
		GFHTMLArea(gump, 50, IndexY, 150, 18, ("<basefont color=#ffffff>"+MenuName));
		// Now let's build the page for this Category
		GFPage(gump, PageNum);
		GFCraftNewPage(who, gump, skill_config, MenuName, skill_name);
		// Cycle all the variables up accordingly
		if( PageNum >= 30 )
			syslog("Crafting: Max Amount of menus exceeded! Reduce amount of Groups in " + menu_file);
			SendSysMessage(who, "Error! Max Amount of Menus Exceeded. Please report to shard staff.");
			return 0;
		endif
		IndexY := IndexY + 20;
		PageNum := PageNum+1;
	endforeach

	return gump;

endfunction

// Used to draw each additional page to the overall gump
function GFCraftNewPage(who, byref gump, skill_config, MenuName, skill_name)

	var ItemX    := 255;
	var ButtonX  := 220;
	var ItemY    := 63;
	var ButtonX2 := 480;
  	var itens := GetConfigStringArray(skill_config[MenuName], "Item");
	var chardata := GetObjProperty(who, "chardata");
	var povo := chardata.crafting;
	if (povo != error)
		if (povo == "")
			povo := "Aiglana";
		endif
		var racialitems := GetConfigStringArray(skill_config[MenuName], povo);
		if (racialitems != error)
			itens := itens + racialitems;
		endif
	endif
		var menurecipe := GetConfigStringArray(skill_config[MenuName], "recipe" );

		var myrecipes := GetObjProperty(who, "recipes");


		foreach recipe in menurecipe
			if (recipe in myrecipes)
				itens.append(recipe);
			endif
		endforeach

    		foreach item in itens
	      		var objtype   := GetObjTypeByName(item);
      			if(TypeOf(objtype) == "Struct")
        			continue;
      			endif
      			var itemdesc  := TruncatePlural(objtype);

      			if(TypeOf(itemdesc) == "Struct")
        			continue;
      			endif
      			itemdesc := lower(itemdesc);
      			if(skill_name =="Alchemy")
      				var config := ReadConfigFile(":*:itemdesc");
        			config := config[objtype];
        			if(lower(config.desc) == "pocao")
      					itemdesc := config.Type;
      				elseif(lower(config.desc) == "tinta")
      					itemdesc := config.name;
      				endif
      			endif

			if ((craft_config[objtype].skill) <= AP_GetSkill(who, craft_skill)) //PARA O ITEM APARECER NA LISTA DO MENU DE CRAFT
				if(ItemY >= 243)
					ItemY := 63;
					GFAddButton(gump, 370, 260, 4005, 4006, GF_PAGE_BTN, (PageNum+1));
					GFHTMLArea(gump, 405, 263, 100, 18, "<basefont color=#ffffff>Proxima Pagina");
					PageNum := PageNum+1;
					GFPage(gump, PageNum);
					GFAddButton(gump, 220, 260, 4014, 4015, GF_PAGE_BTN, (PageNum-1));
					GFHTMLArea(gump, 255, 263, 100, 20, "<basefont color=#ffffff>Pagina Anterior");
				endif
				GFHTMLArea(gump, ItemX, ItemY, 220, 18, ("<basefont color=#ffffff>"+itemdesc));
				GFAddButton(gump, ButtonX, (ItemY-3), 4005, 4006, GF_CLOSE_BTN, objtype);
				if( menu_config["Setup"].Info == 1 )
					GFAddButton(gump, ButtonX2, (ItemY-3), 4011, 4012, GF_CLOSE_BTN, arcnt);
					arinfo.append(objtype);
					arcnt := arcnt + 1;
				endif
				ItemY := ItemY + 20;
				sleepms(1);
			endif
    		endforeach

	return 1;

endfunction

// Used for creating the Info Page gump for an item selected from the
// crafting gump. This gump will display the information about item such
// as skills required, their amt required, materials and their required
// amount per peice, success chances and exceptional chances based on
// skill the character has, show a picture of the item (Non-colored).
// Also says if the item can be exceptionally made and if can have the
// name of it's maker put on the item at GM skill. Can either return to
// the Crafting gump, or create the item immediately from this page.
function GFCraftInfoPage(who, objtype, skill_config)

  	var i_config, thisitem, elem, graphic, itemdesc, tooltip, skillreq, diff, skillreqb, aid_skill, matname, material, color;
  	color := 0;
  	if (objtype > 0)
		i_config := ReadConfigFile(":*:itemdesc");
		thisitem := i_config[objtype];
		if(TypeOf(thisitem) != "ConfigElemRef")
  			thisitem := i_config[("0x0"+(CStr(objtype) - "0x"))];
		endif
		elem     := craft_config[objtype];
		graphic  := CInt(thisitem.graphic);
		color	 := CInt(thisitem.color);
		itemdesc := TruncatePlural(objtype);
		itemdesc := lower(itemdesc);

		if(!graphic || TypeOf(graphic) == "Struct")
  			graphic := CInt(objtype);
		endif

		tooltip := getConfigString(elem, "text");;
		if(TypeOf(tooltip) != "String")
  			tooltip := "";
		endif

		// Set the variables for skill required levels
		skillreq  := elem.skill;
		diff      := skillreq + 10;
		skillreqb := elem.skill2;
		/* DEBUGS: 
		PrintTextAbove(who, "DEBUG: SKILL ITEM: " +skillreq);
		PrintTextAbove(who, "DEBUG: DIFICULDADE: " +diff);
		*/
		if(!skillreqb || skillreqb.errortext)
  			skillreqb  := 0;
		endif

		matname := "";
		material := "";
		var materials := GetConfigStringDictionary(elem, "materials");

		foreach thing in (materials.keys())
			matname := matname + "\n" + GetMaterialName(thing);
			material := material + "\n" + materials[thing];
		endforeach
  	endif
	// Get their xxx.x skill amount
	var temp1 := CDbl(AP_GetSkill(who, craft_skill));
	// Get their xxx.x skill amount of the secondary skill entry
	var temp2 := 0;

	// If there was no secondary skill in the cfg entry, set it to main skill
	if(temp2 == error)
		temp2 := temp1;
	endif
	// Figure the sickening odd Success Chance percentages.
	var schance   := 0.0;
	if((TypeOf(aid_skill) == "Integer") && (skillreqb != 0))
		if(TypeOf(skillreqb) == "Double" || TypeOf(skillreqb) == "Integer")
			// Although CraftCheckSkill() would return 0%, let's not run all that code
			// since it is a given anyway.
			if(skillreqb > temp2)
				schance := 0.0;
			else
				schance := CraftCheckSkill(who, craft_skill, diff, 0, 1);
			endif
		endif
	else
		if(skillreq > temp1)
			schance := 0;
		else
			schance := CraftCheckSkill(who, craft_skill, diff, 0, 1);
		endif
	endif
	if(elem.placeholder == 1)
		schance := 100;
	endif
	if(TypeOf(aid_skill) != "String")
		aid_skill := 0;
	endif

	if (TypeOf(elem.skill) == "String")
		PrintTextAbove(who, EL_BUG); //Hehehe
		schance := EL_BUG;
		skillreq := 200; //Para nunca fazer
	endif
	/*	DEBUGS:

	SendSysMessage(who, "DEBUG: CHANCE:" +schance);
	SendSysMessage(who, "DEBUG: " +TypeOf(schance));
	SendSysMessage(who, "DEBUG: REQUIRED:" +skillreq);
	*/
	// Below added for checking for parts. If parts are present, then create the part list
	// instead.
	var tmatname  := 0;
	var tmpmat    := "";
	if(TypeOf(elem.parts) == "String")
		tmpmat := SplitWords(elem.parts);
		matname  := "";
		material := "";
		if((len(tmpmat)/2) < 5)
			for j := 1 to (len(tmpmat)/2)
				tmatname := TruncatePlural(tmpmat[j]);
				case(j)
					1:
					2:
					3: matname := matname + tmatname + "\n";
					4: matname := matname + tmatname;
					default: matname := matname + tmatname + "\n";
				endcase
				material := material + (tmpmat[j+(len(tmpmat)/2)]) + "\n";
			endfor
		else
			material := 0;
			for j := 1 to (len(tmpmat)/2)
				tmatname := TruncatePlural(tmpmat[j]);
				if(j == (len(tmpmat)/2))
					matname := matname + "& " + (tmpmat[j+(len(tmpmat)/2)]) + " " + tmatname + " requirido";
				else
					matname := matname + (tmpmat[j+(len(tmpmat)/2)]) + " " + tmatname + ", ";
				endif
			endfor
		endif
	endif

	// Exception % chance wording.
	var echance   := 0;
	if(elem.Exceptional == 1)
		echance := ExceptionalCraftChance(who, craft_skill, temp1, diff, thisitem);
	endif

	var gump := GFCreateGump(50, 25);
	GFResizePic(gump, 2, 0, BKGND_SILVERTRIM, 521, 470);
	GFGumpPic(gump, 10, 10, 2624);
	GFGumpPic(gump, 10, 205, 2624);
	GFGumpPic(gump, 250, 10, 2624);
	GFGumpPic(gump, 250, 205, 2624);
	GFAddAlphaRegion(gump, 10, 10, 505, 450);
	GFGumpPic(gump, 10, 30, 5055);
	GFGumpPic(gump, 90, 30, 5055);
	GFResizePic(gump, 160, 120, 5054, 356, 13);
	GFGumpPic(gump, 10, 210, 5055);
	GFGumpPic(gump, 90, 210, 5055);
	GFGumpPic(gump, 10, 320, 5055);
	GFGumpPic(gump, 90, 320, 5055);
	GFGumpPic(gump, 10, 420, 5055);
	GFGumpPic(gump, 90, 420, 5055);
	GFHTMLArea(gump, 10, 27, 150, 18, "<basefont color=#ffffff>ITEM");
	GFHTMLArea(gump, 170, 117, 150, 18, "<basefont color=#ffffff>SKILLS");
	GFHTMLArea(gump, 10, 208, 150, 18, "<basefont color=#ffffff>MATERIAIS");
	GFHTMLArea(gump, 10, 318, 150, 18, "<basefont color=#ffffff>DESCRICAO");
	GFAddButton(gump, 15, 435, 4014, 4016, GF_CLOSE_BTN, 1);
	GFHTMLArea(gump, 50, 438, 150, 18, "<basefont color=#ffffff>VOLTAR");
	GFAddButton(gump, 270, 435, 4005, 4006, GF_CLOSE_BTN, CInt(objtype) );
	GFHTMLArea(gump, 305, 438, 150, 18, "<basefont color=#ffffff>FAZER");
	GFTilePic(gump, 20, 50, CInt(graphic), color);
	//GFHTMLArea(gump, 430, 132, 150, 18, ("<basefont color=#ffffff>" + skillreq));

	// Tooltip info from itemdesc.cfg appended in.
	if(tooltip != "")
		GFHTMLArea(gump, 170, 333, 340, 85, ("<basefont color=#ffffff>" + tooltip), 0, 1);
	endif
	// Material Required
	GFHTMLArea(gump, 170, 205, 300, 90, ("<basefont color=#ffffff>" + matname));
	if(material != 0)
		GFHTMLArea(gump, 430, 205, 20, 90, ("<basefont color=#ffffff>" + material));
	endif
	// Success Chance % text
	GFTextLine(gump, 170, 80, 1153, "Chance de Sucesso:");
	var schanceString := GetChanceString(schance);
	GFTextLine(gump, 430, 80, 1153, schanceString);
	// Excpetional Chance % text
	if(echance != 0)
		GFTextLine(gump, 170, 100, 1153, "Chance de Excepcional:");
		//var echanceString := GetChanceString(echance);
		GFTextLine(gump, 430, 100, 1153, CStr(echance) + "%");
	endif
	// Title of the menu
	GFHTMLArea(gump, 10, 12, 510, 20, ("<basefont color=#ffffff>"+CStr(GetConfigString(skill_config["Index"], "Title"))));
	// Item Name
	GFHTMLArea(gump, 330, 40, 150, 20, ("<basefont color=#ffffff>"+itemdesc));
	// Skill name
	GFHTMLArea(gump, 170, 132, 150, 20, ("<basefont color=#ffffff>"+display_skill + " requirido"));

	if((skillreqb > 0) && (skillreqb != error))
		// Skill 2 name
		GFHTMLArea(gump, 170, 152, 150, 20, ("<basefont color=#ffffff>"+lower(aid_skill)+" requirido"));
		GFHTMLArea(gump, 430, 152, 150, 20, ("<basefont color=#ffffff>"+skillreqb));
	endif

	var ifres := GFSendGump(who, gump);

	if(ifres[0] == 1)
		return 0;
	else
		return ifres[0];
	endif
endfunction


function cBaseCraftDict()

	var baseDict  := dictionary;
	foreach skillentry in GetConfigStringArray(menu_config["Skills"], "Skill")
		var temp := dictionary;
		temp.+skill  := menu_config[skillentry].Attribute;
		temp.+amount := CInt(menu_config["Setup"].DefaultAmount);
		temp.+mark   := CInt(menu_config[skillentry].MakersMark);
		temp.+retain := CInt(menu_config[skillentry].RetainColor);
		temp.+last   := 0;
		baseDict[temp.skill] := temp;
	endforeach

	return baseDict;

endfunction

function Craft_RetrieveGump(elem_name, prop_name)

        var elem := DFFindElement(craft_gumps, elem_name);
        return (elem.GetProp(prop_name));

endfunction

function Craft_SaveGump(elem_name, gump, prop_name)

        var elem := DFFindElement(craft_gumps, elem_name, DF_CREATE);
        return (elem.SetProp(prop_name, gump));

endfunction


//
//  TruncatePlural()
//
//  * Returns the name for the item of the objtype passed.
//
function TruncatePlural(obj_type, add_S := 0)
        obj_type := CInt(obj_type);

        var config := ReadConfigFile(":*:itemdesc");
        config := config[obj_type];

        var name;
        if (config.desc)
                name := config.desc;
        elseif (config.name)
                name := config.name;
        else
                return error{"errortext":="Invalid Objtype : " + hex(obj_type) + "."};
        endif

        if ( !name["%"] )
                if ( add_S == 0 )
                        return name;
                endif
                if ( name[len(name)] in array{"s"} )
                        return name;
                elseif ( name[len(name)] in array{"h","j","x","z"} )
                        return name + "es";
                endif
                return name + "s";
        endif

        // Will deal with:
        //
        //      Bread Stick%s
        //      Bread Loa%ves/f%
        //      Lump%s% of Coal
        //      Loa%ves/f% of Bread
        //
        var first := find(name, "%", 1);
        var slash := find(name, "/", first);
        var second := find(name, "%", first + 1);
        if ( slash && !second )
                // Force-append a % if a '/' exists!
                name := name + "%";
                second := len(name);
        elseif ( !slash )
                slash := second;
        endif

        // wwwww%xx[/yy]%[zzzzz]
        // 12345678     9 01234
        // 12345678 901 2 34567

        var s_plur := name[first+1, slash-(first+1)];
        var s_sing := "";
        if ( slash != first )
                s_sing := name[slash+1, second-(slash+1)];
        endif

        var s_first := name[1, first-1];
        var s_middle := s_plur;
        var s_last := "";

        if ( len(name) > second )
                s_last := name[second+1, len(name)-second];
        endif

        if ( add_S == 0 )
                s_middle := s_sing;
        endif

        if ( !s_first )
                s_first := "";
        endif
        if ( !s_middle )
                s_middle := "";
        endif
        if ( !s_last )
                s_last := "";
        endif
        return s_first + s_middle + s_last;

endfunction



