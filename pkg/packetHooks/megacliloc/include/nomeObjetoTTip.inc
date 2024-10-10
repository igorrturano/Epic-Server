use uo;
include ":attributes:attributes";
include ":commands:/include/r";
include ":custompizza:/itemrp/include/lore_items";
include ":megacliloc:/include/tradutores";
include ":megacliloc:/include/gettersTTipUtils";

function PreencheNome(objeto,quem)
	var nomeObjeto;

	if (objeto.IsA(POLCLASS_MOBILE))
		return PreencheNomeMobile(objeto,quem,nomeObjeto);
	elseif( objeto.IsA(POLCLASS_ITEM) )
		return PreencheNomeITem(objeto);
	else
		nomeObjeto := objeto.name;
	endif

	return nomeObjeto;
endfunction

function PreencheNomeMobile(objeto,quem, nomeObjeto := "")

	//Cor do nome dos NPC conforme skill de combate
	if (objeto.npctemplate)
		nomeObjeto := PreencheNomeNpc(quem,objeto, nomeObjeto);
	else
		nomeObjeto := PreencheNomePersonagem(quem,objeto, nomeObjeto);
	endif

	return nomeObjeto;

endfunction

function PreencheNomeNpc(quem,objeto, nomeObjeto := "")

	if(!objeto.IsA(POLCLASS_MOBILE)) return error{"errortext":="Object passed was not an POLCLASS_MOBILE."}; endif
	
	var desc_npc := GetObjProperty(objeto, "descmodboss");

	if (desc_npc) //Adicionado para mostrar descrição do boss
		nomeObjeto := objeto.name +"<br>"+WHITE+desc_npc;
	endif
	if (quem.weapon.attribute != WRESTLING)
		if ( (AP_GetSkill(objeto, WRESTLING) + 10) > AP_GetSkill(quem, quem.weapon.attribute))
			nomeObjeto :=  AMARELO_CLARO + nomeObjeto;
		endif
	else
		var sk1 := AP_GetSkill(quem, ONEHANDED);
		var sk2 := AP_GetSkill(quem, TWOHANDED);
		var sk3 := AP_GetSkill(quem, RANGED);
		var maxsk := sk1;
		if (sk2 > maxsk)
			maxsk := sk2;
		endif
		if (sk3 > maxsk)
			maxsk := sk3;
		endif
		if (AP_GetSkill(objeto, WRESTLING)+10 > maxsk)
			nomeObjeto :=  AMARELO_CLARO + nomeObjeto;
		endif
	endif
	return nomeObjeto;
endfunction

function PreencheNomePersonagem(quem,objeto, nomeObjeto := "")

	if(!objeto.IsA(POLCLASS_MOBILE)) return error{"errortext":="Object passed was not an POLCLASS_MOBILE."}; endif
	
	//Caso em que tá desconhecido
	if(GetObjProperty(objeto, "realname"))
		nomeObjeto := AMARELO_CLARO + "Incognito";
	else
		//Verifica se o char já foi reconhecido por who
		var conhecidos := dictionary;
		conhecidos := GetObjProperty(quem, "conhecidos"); //Lista de pessoas conhecidadas

		if(conhecidos)
			var ID := verificaDisfarce(objeto);
			nomeObjeto := conhecidos.Exists(ID)? conhecidos[ID] : objeto.name;
		else
			nomeObjeto := objeto.name;
		endif
	endif

	if (quem.cmdlevel > 1) //Staff consegue ver o nome e raça
		nomeObjeto := nomeObjeto + " [" + objeto.acctname + "]";
	endif

	return nomeObjeto;
endfunction

function PreencheNomeItem(objeto)
	if(!objeto.IsA(POLCLASS_ITEM)) return error{"errortext":="Object passed was not an POLCLASS_ITEM."}; endif
	var nomeObjeto := "";
	nomeObjeto := recuperaNomeItemMaterial(objeto);

	if(IsLoreItem(objeto))
		nomeObjeto := AdicionaLore(objeto,nomeObjeto);
	endif
	return nomeObjeto;

endfunction

function recuperaNomeItemMaterial(objeto)
	var resource_config := ReadConfigFile(":crafting:config/resourceList") ;

	if(GetObjProperty(objeto, "material"))
		return objeto.desc + " [" + resource_config[GetObjProperty(objeto, "material")].name + "]";
	else
		return objeto.desc;
	endif
endfunction

function IsLoreItem(objeto)
	var is_lore_item := GetObjProperty(objeto, "IsLoreItem");
	if (!is_lore_item)
		var cfg := ReadConfigFile(":*:itemdesc");
		var elem := FindConfigElem(cfg, objeto.objtype);
		if (elem)
			return GetConfigInt(elem, "IsLoreItem");
		endif
	else
		return 1;
	endif
	return 0;
endfunction

function AdicionaLore(objeto,nomeObjeto)
	var item_info := GetLoreItemInfo(objeto.objtype);
	if (item_info)
		nomeObjeto := nomeObjeto + "<br><BASEFONT COLOR=#FFD700>Item Lendário";
		nomeObjeto := nomeObjeto + "<br><BASEFONT COLOR=#FFFFFF>Povo: " + item_info.culture_name;

		// Verifica se o item já foi avaliado por um especialista
		var evaluated_skill := GetObjProperty(objeto, "evaluated_skill");
		if (evaluated_skill)
			nomeObjeto := nomeObjeto + "<br><BASEFONT COLOR=#d98cd9>Esse item foi avaliado por um " + TraduzirProfissao(item_info.required_skill);
		else
			nomeObjeto := nomeObjeto + "<br><BASEFONT COLOR=#d98cd9>" + GetConsultaEspecialistaTexto(item_info.required_skill);
		endif

		// Verifica se o item já foi examinado por um arqueólogo
		var evaluated_archaeology := GetObjProperty(objeto, "evaluated_archaeology");
		if (evaluated_archaeology)
			nomeObjeto := nomeObjeto + "<br><BASEFONT COLOR=#a1eca7>Esse item foi avaliado por um Arqueólogo";
		else
			nomeObjeto := nomeObjeto + "<br><BASEFONT COLOR=#a1eca7>Consulte um Arqueólogo";
		endif
	endif
	return nomeObjeto;
endfunction