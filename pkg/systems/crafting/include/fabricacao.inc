include "/include/epicConstants";
include ":crafting:craftingUtil";
include ":megacliloc:/include/descricaoObjetoTTip";
include ":crafting:gumps/itensComplexosGump";

//var fabricacaoGump := DFOpenDataFile(":crafting:CraftGumps", DF_CREATE);
var fabricacaoConfig := ReadConfigFile( ":crafting:config/craftConfig" );
var itemsConfig := ReadConfigFile(":*:itemdesc");

function FabricaItensComplexos(quem,profissao)
	EnviaItensComplexosGump(quem,profissao);
endfunction

function EnviaItensComplexosGump(quem,profissao)
	var parametros := PreparaItensComplexosGump(quem, profissao);
	var retorno := true;
	var gump;
	var navegacao := struct;
	navegacao.+categoriaSelecionada := "";

	while(retorno)
		MontaItensComplexosGump(quem,gump,parametros,navegacao);
		retorno := GFSendGump(quem,gump);

		if(retorno)
			case(ClassificaRetornoComplexos(retorno[0]))
				RT_CATEGORIAS:
					navegacao.categoriaSelecionada := parametros.visaoReceitas.keys()[retorno[0] - COMPLEXOS_CATEGORIAS];
					break;
				RT_RECEITAS:
					var indiceRetorno := retorno[0] - COMPLEXOS_RECEITAS;
					var quantidadeBotoes := 3;
					var indiceBotao := indiceRetorno % quantidadeBotoes;
					indiceBotao := indiceBotao == 0? quantidadeBotoes : indiceBotao;
					var indiceItem := (indiceRetorno - indiceBotao) / 3 + 1;
					Print("indiceRetorno: " + indiceRetorno + " indiceBotao: " + indiceBotao);
					Print("Fabricando Item: " + parametros.visaoReceitas[navegacao.categoriaSelecionada][indiceItem].desc);
					navegacao.+esquemas := "";
					break;
				RT_BOTAO_MENU:
					break;
				default:
			endcase
		endif
	endwhile
endfunction

function PreparaItensComplexosGump(quem, profissao)

	var parametrosGump := struct;
	var menuFile    := fabricacaoConfig[profissao].MenuFile;
	var menuPericiaCfg := ReadConfigFile(menuFile);

	var propCraft := GetObjProperty(quem, "Craft");
	if( TypeOf(propCraft) != "Dictionary" )
		propCraft := MontaCraftProp(fabricacaoConfig);
		SetObjProperty(quem, "Craft", propCraft);
	endif

	var categorias := GetConfigStringArray(menuPericiaCfg["Index"], "Group");
	parametrosGump.+profissao := profissao;
	parametrosGump.+propCraft := propCraft;
	parametrosGump.+periciaCfg := fabricacaoConfig[profissao];
	parametrosGump.+visaoReceitas := MontaVisaoReceitas(quem,categorias,menuPericiaCfg,parametrosGump.periciaCfg.Attribute);
	
	return parametrosGump;
endfunction

function MontaVisaoReceitas(quem,categorias,menuPericiaCfg,nomePericia)

	var retornoVisao := Dictionary;
	
	var chardata := GetObjProperty(quem, "chardata");
	var receitasAprendidas := GetObjProperty(quem, "recipes");
	var povo := chardata.crafting;

	if (povo != error)
		if (povo == "")
			povo := "Aiglana";
		endif
	endif

	foreach categoria in categorias
		//TODO Modificar as Configs pra colocar o elemento "item" onde não tiver.
		var itens := GetConfigStringArray(menuPericiaCfg[categoria], "Item");
		var receitas := Array;

		foreach item in itens
			var itemStringArr := SplitWords(item);
			var receita := Struct;

			var itemOrigem := ""; 
			var nomeItem := ""; 
			var isDuplaOrigem := false;

			case (itemStringArr.size())
				1:
					nomeItem := itemStringArr[1]; // nome
					break;
				2:
					nomeItem := itemStringArr[2]; // nome
					itemOrigem := itemStringArr[1]; // povo ou recipe
					break;
				3:
					nomeItem := itemStringArr[3]; //nome
					itemOrigem := itemStringArr[2]; //povo
					isDuplaOrigem := true; // Neste caso, devo verificar se conheço por cultura ou receita.
					break;
			endcase

			var objtype   := GetObjTypeByName(nomeItem);
			if(TypeOf(objtype) == "Struct")
        			continue;
      		endif

			if(DeveAparecerReceita(quem,povo,itemOrigem,nomeItem,objtype,nomePericia,receitasAprendidas,isDuplaOrigem))
				receitas.append(MontaInfoItem(quem,objtype));
			endif
			Sleepms(2);
		endforeach
		retornoVisao.insert(categoria,receitas);
		Sleepms(2);
	endforeach

	return retornoVisao;

endfunction

function MontaInfoItem(quem,objtype)
	var infoItem := struct;
	var definicaoItem := itemsConfig[objtype];
	if(TypeOf(definicaoItem) != "ConfigElemRef")
		definicaoItem := itemsConfig[("0x0"+(CStr(objtype) - "0x"))];
	endif

	infoItem.+textoDescItem := PegaTextoDescItem(quem,objtype,definicaoItem);
	infoItem.+dificuldade := PegaDificuldade();
	infoItem.+objtype := objtype;
	infoItem.+uso := definicaoItem.Attribute;
	infoItem.+desc := definicaoItem.desc;
	infoItem.+graphic := definicaoItem.Graphic ? CInt(definicaoItem.Graphic) : CInt(objtype);

	return infoItem;

endfunction

function PegaTextoDescItem(quem,objtype,definicaoItem)

	var desc := struct;
	//TODO Lembrar de lidar com as diferenças de nomenclatura para Alchemy.
	desc := "<BASEFONT COLOR=#ffe242>" + TruncatePlural(objtype) + "</BASEFONT><br>";
	desc := MontaDescricaoItem(definicaoItem,quem,desc);
	
	var materiais := PegaMateriaisNecessarios(objtype);
	if(materiais)
		desc +=  "<br>";
		desc +=  "<BASEFONT COLOR=#7bf8fe> Materiais Necessários</BASEFONT><br>";
		foreach material in (materiais.keys())
			desc +=  materiais[material] + " " + GetMaterialName(material)  + "<br>";
			Sleepms(2);
		endforeach
	endif

	desc +=  "<br>";
	desc +=  "<BASEFONT COLOR=#7bf8fe> Tempo de fabricação</BASEFONT><br>" + PegaDuracaoFabricacao() + "<br>";

	return desc;
endfunction

function PegaMateriaisNecessarios(objtype)
	var descritorFabricacao := craft_config[objtype];
	var materials := GetConfigStringDictionary(descritorFabricacao, "materials");
	return materials;
endfunction

function PegaDuracaoFabricacao()
	return "Longo";//TODO Implementar esta função.
endfunction

function PegaDificuldade()
	//TODO Implementar esta função.
endfunction

function DeveAparecerReceita(quem,povo,itemOrigem,nomeItem,objtype,nomePericia,receitasAprendidas,isDuplaOrigem)
	
	var isOrigemValida := false;
	var isValorPericiaSuficiente := (craft_config[objtype].skill) <= AP_GetSkill(quem, nomePericia);

	if(isDuplaOrigem)
		isOrigemValida := nomeItem in receitasAprendidas || povo == itemOrigem;
	else
		if(itemOrigem == "Recipe")
			isOrigemValida := nomeItem in receitasAprendidas;
		else
			isOrigemValida := povo == itemOrigem;
		endif
	endif

	return isOrigemValida && isValorPericiaSuficiente;
endfunction

function MontaCraftProp(fabricacaoConfig)

	var baseDict  := dictionary;
	foreach pericia in GetConfigStringArray(fabricacaoConfig["Skills"], "Skill")
		var temp := dictionary;
		temp.+skill  := fabricacaoConfig[pericia].Attribute;
		temp.+amount := CInt(fabricacaoConfig["Setup"].DefaultAmount);
		temp.+retain := CInt(fabricacaoConfig[pericia].RetainColor);
		temp.+last   := 0;
		baseDict[temp.skill] := temp;
		Sleepms(2);
	endforeach

	return baseDict;

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