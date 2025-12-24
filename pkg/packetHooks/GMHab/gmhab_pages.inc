
function ShowMagiasPage(byref gump, who, page_num)
	// Get character's grimoire
	var chardata := GetObjProperty(who, "chardata");
	if (!chardata || !chardata.magia)
		GFTextLine(gump, 50, 100, 1153, "Personagem nao possui magias!");
		return;
	endif
	
	var y := 100;
	GFTextLine(gump, 50, y, 1153, "Magias do Personagem");
	GFTextLine(gump, 50, y + 20, 1150, "Tipo: " + chardata.magia);
	y := y + 50;
	
	// Find grimoire in backpack
	var backpack := who.backpack;
	var grimoire := 0;
	
	foreach item in EnumerateItemsInContainer(backpack)
		if (item.objtype == 0x0EFA || item.objtype == 0x0E3B) // Grimoire graphics
			var booktype := GetObjProperty(item, "Booktype");
			if (booktype == chardata.magia || booktype == "spellcaster")
				grimoire := item;
				break;
			endif
		endif
		sleepms(2);
	endforeach
	
	if (!grimoire)
		GFTextLine(gump, 50, 150, 1153, "Grimorio nao encontrado na mochila!");
		GFTextLine(gump, 50, 180, 1150, "Procurando por: " + chardata.magia);
		return;
	endif
	
	// Get spells from grimoire
	var spells := GetObjProperty(grimoire, "spells");
	if (!spells || !spells.size())
		GFTextLine(gump, 50, 150, 1153, "Nenhuma magia no grimorio!");
		return;
	endif
	
	// Display spells (10 per page, 5 left + 5 right)
	var total_spells := spells.size();
	var spells_per_page := 10;
	var total_pages := CInt((total_spells + spells_per_page - 1) / spells_per_page);
	
	if (page_num < 1)
		page_num := 1;
	elseif (page_num > total_pages)
		page_num := total_pages;
	endif
	
	GFTextLine(gump, 400, 100, 1150, "Pagina " + CStr(page_num) + "/" + CStr(total_pages));
	GFTextLine(gump, 50, 120, 1150, "Total: " + CStr(total_spells) + " magias");
	
	var x_left := 50;
	var x_right := 310;
	var y_left := 150;
	var y_right := 150;
	var count := 0;
	
	var start_index := (page_num - 1) * spells_per_page + 1;
	var end_index := start_index + spells_per_page - 1;
	if (end_index > total_spells)
		end_index := total_spells;
	endif
	
	for i := start_index to end_index
		var spell_id := spells[i];
		
		// Determine position
		count := count + 1;
		var x, y_pos;
		if (count <= 5)
			x := x_left;
			y_pos := y_left;
			y_left := y_left + 40;
		else
			x := x_right;
			y_pos := y_right;
			y_right := y_right + 40;
		endif
		
		// Display spell
		GFTextLine(gump, x, y_pos, 2100, "Magia ID: " + CStr(spell_id));
		
		sleepms(2);
	endfor
	
	// Navigation
	y := 420;
	if (page_num > 1)
		GFAddButton(gump, 50, y, 2223, 2223, 1, 5000 + (page_num - 1));
		GFTextLine(gump, 75, y + 5, 2100, "Anterior");
	endif
	
	if (page_num < total_pages)
		GFAddButton(gump, 500, y, 2224, 2224, 1, 5000 + (page_num + 1));
		GFTextLine(gump, 450, y + 5, 2100, "Proximo");
	endif
endfunction

function ShowComandosPage(byref gump, who)
	var y := 100;
	var x := 50;
	
	GFTextLine(gump, x, y, 1153, "Comandos Uteis para GMs");
	y := y + 30;
	
	// Lista de comandos úteis
	var comandos := array{
		struct{ "cmd" := ".addallhabs", "desc" := "Adiciona todas as habilidades" },
		struct{ "cmd" := ".add pergaminho_X", "desc" := "Cria pergaminho de nivel X" },
		struct{ "cmd" := ".set level X", "desc" := "Define nivel do personagem" },
		struct{ "cmd" := ".props", "desc" := "Abre propriedades do item/mobile" },
		struct{ "cmd" := ".set str/dex/int X", "desc" := "Define atributos" },
		struct{ "cmd" := ".set skill X", "desc" := "Define skill" },
		struct{ "cmd" := ".tele", "desc" := "Teleporta para local" },
		struct{ "cmd" := ".go [nome]", "desc" := "Vai ate jogador" },
		struct{ "cmd" := ".bring [nome]", "desc" := "Traz jogador ate voce" },
		struct{ "cmd" := ".invis", "desc" := "Fica invisivel" }
	};
	
	foreach cmd in comandos
		GFTextLine(gump, x, y, 68, cmd.cmd);
		GFTextLine(gump, x + 200, y, 1150, cmd.desc);
		y := y + 25;
		
		sleepms(2);
	endforeach
	
	y := y + 20;
	GFTextLine(gump, x, y, 1153, "Comandos de Teste:");
	y := y + 25;
	
	var test_cmds := array{
		struct{ "cmd" := ".gmhab", "desc" := "Abre este gump" },
		struct{ "cmd" := ".special", "desc" := "Executa special ability" },
		struct{ "cmd" := ".shab [nome]", "desc" := "Executa habilidade de classe" }
	};
	
	foreach cmd in test_cmds
		GFTextLine(gump, x, y, 88, cmd.cmd);
		GFTextLine(gump, x + 200, y, 1150, cmd.desc);
		y := y + 25;
		
		sleepms(2);
	endforeach
endfunction
