use Math;

include "/include/epicConstants";
include "include/epicUtils";
include ":crafting:craftingUtil";
include ":megacliloc:/include/descricaoObjetoTTip";
include ":crafting:gumps/itensComplexosGump";
include ":gumps:/include/dropDownGump";

//var fabricacaoGump := DFOpenDataFile(":crafting:CraftGumps", DF_CREATE);
var fabricacaoConfig := ReadConfigFile( ":crafting:config/craftConfig" );
var itemsConfig := ReadConfigFile(":*:itemdesc");

//Itens Complexos Inicio
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
	desc +=  "<BASEFONT COLOR=#7bf8fe> Tempo de Fabricação</BASEFONT><br>" + PegaDuracaoFabricacao() + "<br>";

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
//Itens Complexos Fim

//Componentes Inicio
function EnviaGumpComponentes(quem)
	var retorno := 1;
	var configRecursos := ReadConfigFile(":crafting:config/resourceList");
	var configDetalhes := ReadConfigFile(":woodworking:carpentry");//TODO isto tem que virar um parâmetro, porque virá do script da ferramente\estação.
	var chavesOpcoesMateriais := GetConfigStringKeys(configRecursos);
	
	var navegacao := struct;
	var parametros := PreparaCompoenentesGump(quem,configDetalhes,navegacao);
	var gerenciador := navegacao.gerenciadorDropDown;
	var gump;

	while(retorno)
		Print("navegacao.componenteSelecionado: " + navegacao.componenteSelecionado);
		Print("retorno[0]: " + retorno[0]);
		if(navegacao.componenteSelecionado && navegacao.devePopularDropDown)
			PreparaDropDowns(navegacao,gerenciador,chavesOpcoesMateriais,configRecursos);
			navegacao.devePopularDropDown := false;
		endif

		MontaComponentesGump(gump, parametros,navegacao);
		retorno := GFSendGump(quem,gump);
		
		Print(gerenciador.dropDowns);

		if(GerenciadorDropDown::IsIdDentroDropDowns(gerenciador,retorno[0]))
			var indice := retorno[0] - gerenciador.adicionalIdDropDown;
			var objetoDropDown := gerenciador.dropDowns[indice];

			FechaTodosDropdowns(quem,gerenciador.dropDowns);
			var process := SendDropDownGump(quem, GetPid(),objetoDropDown);
			//TODO Fazer a função para verificar os materiais da bag do jogador e popular o dropdown de acordo.
			if(process)
				objetoDropDown.pid := process.pid;
			endif
		elseif(GerenciadorDropDown::IsIdDentroOpcoes(gerenciador,retorno[0]))
			GerenciadorDropDown::DefineValorEscolhido(gerenciador,retorno[0]);
			Print("valor Escolhido: " + gerenciador.dropDowns[1].valorEscolhido + " indice: " + retorno[0] + " tamanho array." + gerenciador.dropDowns[1].valores.size());
		elseif(retorno[0])
			GerenciadorDropDown::LimpaDropDowns(gerenciador);
			navegacao.devePopularDropDown := true;
			navegacao.componenteSelecionado := parametros.componentes[retorno[0]];
		elseif(!retorno)
			FechaTodosDropdowns(quem,gerenciador.dropDowns);
		endif
	endwhile
endfunction

function PreparaDropDowns(byref navegacao,byref gerenciador, chavesOpcoesMateriais, configRecursos)
	var materiais := SplitWords( navegacao.componenteSelecionado.materiais,",");

	foreach material in materiais
		var dropDown := GerenciadorDropDown::RegistraDropDown(gerenciador,_material_iter);
		var indiceOpcoes := 1;

		foreach chave in chavesOpcoesMateriais
			Sleepms(2);
			var retorno := DropDown::AdicionaOpcao(dropDown,
			configRecursos[chave].name,
			indiceOpcoes,
			struct {vlrChave := chave, nome := configRecursos[chave].name});

			if(!retorno) 
				Print("retorno: " + retorno); 
				continue;
			endif;
			++indiceOpcoes;
		endforeach
		
		Sleepms(2);
	endforeach
endfunction

function FechaTodosDropdowns(quem,dropdowns)
	foreach dropdown in dropdowns

		if(GetProcess( dropdown.pid ))
			CloseGump(quem,dropdown.idBottao);
		endif
		Sleepms(2);
	endforeach
endfunction

function PreparaCompoenentesGump(quem, detalhes,byref navegacao)

	var parametrosGump := struct;
	
	var chavesComponentes := GetConfigStringKeys(detalhes);

	var materiais := array;
	var componentes := array;
	var indiceComponentes := 1;

	navegacao.+gerenciadorDropDown := GerenciadorDropDown(30,
										COMPONENTES_DROPDOWN_INICIAL,
										COMPONENTES_DROPDOWN_OPCOES_INICIAL,
										COMPONENTES_DROPDOWN_MAX);
	navegacao.+componenteSelecionado := 0;
	navegacao.+devePopularDropDown := true;

	foreach chave in chavesComponentes
		Sleepms(2);
		if(!detalhes[chave].encaixeComponente)
			continue;
		endif

		var materiaisNecessarios := GetConfigStringArray( detalhes[chave], "materials" );
		var materiaisConcatenados := "";  
		foreach material in materiaisNecessarios
			if(_material_iter != 1)
				materiaisConcatenados += ", ";
			endif

			materiaisConcatenados += material;
			Sleepms(2);
		endforeach

		var componente := struct;
		componente.+nome := detalhes[chave].name;
		componente.+estacao := itemsConfig[detalhes[chave].estacao].desc;
		componente.+materialPrincipal := detalhes[chave].principalMaterial;
		componente.+materiais := materiaisConcatenados;
		componente.+encaixe := detalhes[chave].encaixeComponente;
		componente.+selecione := struct{isColunaBotao := true, valor := indiceComponentes};

		componentes.append(componente);
		indiceComponentes += 1;
	endforeach

	var colCfg := dictionary;
		colCfg["nome"] := struct{titulo := "Nome",ordem := 1, largura := 2};
		colCfg["materiais"] := struct{titulo := "Materiais",ordem := 2,largura := 3};
		colCfg["materialPrincipal"] := struct{titulo := "Principal Material",ordem := 3,largura := 2};
		colCfg["estacao"] := struct{titulo := "Estação",ordem := 5,largura := 2};
		colCfg["encaixe"] := struct{titulo := "Encaixe",ordem := 4,largura := 2};
		colCfg["selecione"] := struct{titulo := "Selecione",ordem := 6,largura := 1};

	parametrosGump.+configColunas := colCfg;
	parametrosGump.+materiais := materiais;
	parametrosGump.+componentes := componentes;
	
	return parametrosGump;
endfunction


// TODO pensar em um modo de aplicar uma dificuldade a mais dependendo do componente.
function DistribuiQualidadeComponentes(quantidade,valorPericia, deslocamentoPico := -1) 
	
	if(quantidade == error || quantidade.size() == 0)
		return error{"errortext":="O parâmetro quantidade é obrigatório."};
	endif

	var	pico := valorPericia / 20;
	var	chancesPorTier := DistribuicaoGaussianaNormalizada({1,2,3,4,5},pico + deslocamentoPico,1.4);
	var retorno := dictionary;

	//Print("Chances: "  + chancesPorTier);
	for i:=1 to quantidade
		var isFalhouTudo := true;
		var rolagem := RandomFloat(100.0);
		var chanceAcumulada := 0;

		foreach chance in chancesPorTier
			chanceAcumulada += chance;
			//Print("Chance Tier " + _chance_iter + ": " + chance);
			//Print("rolagem: " + rolagem + " chance: " + chanceAcumulada);
			if (rolagem <= chanceAcumulada)
				
				isFalhouTudo := false;
				if(retorno.exists(_chance_iter)) // popula o dict de retono com chave=tier e valor=quantidade de itens nesse tier.
					retorno[_chance_iter] += 1;
				else
					retorno.insert(_chance_iter,1);
				endif
				break;
			endif
			Sleepms(2);
		endforeach

		if(isFalhouTudo)
			if(retorno.exists(1))
				retorno[1] += 1;
			else
				retorno.insert(1,1);
			endif
		endif
		Sleepms(2);
	endfor
	return retorno;

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