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
	var sessao := struct;
	sessao.+categoriaSelecionada := "";

	while(retorno)
		MontaItensComplexosGump(quem,gump,parametros,sessao);
		retorno := GFSendGump(quem,gump);

		if(retorno)
			case(ClassificaRetornoComplexos(retorno[0]))
				RT_CATEGORIAS:
					sessao.categoriaSelecionada := parametros.visaoReceitas.keys()[retorno[0] - COMPLEXOS_CATEGORIAS];
					break;
				RT_RECEITAS:
					var indiceRetorno := retorno[0] - COMPLEXOS_RECEITAS;
					var quantidadeBotoes := 3;
					var indiceBotao := indiceRetorno % quantidadeBotoes;
					indiceBotao := indiceBotao == 0? quantidadeBotoes : indiceBotao;
					var indiceItem := (indiceRetorno - indiceBotao) / 3 + 1;
					Print("indiceRetorno: " + indiceRetorno + " indiceBotao: " + indiceBotao);
					Print("Fabricando Item: " + parametros.visaoReceitas[sessao.categoriaSelecionada][indiceItem].desc);
					sessao.+esquemas := "";
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

	infoItem.+componentes := PegaComponentesNecessarios(objtype);
	infoItem.+textoDescItem := PegaTextoDescItem(quem,objtype,definicaoItem,infoItem.componentes);
	infoItem.+dificuldade := PegaDificuldade();
	infoItem.+objtype := objtype;
	infoItem.+uso := definicaoItem.Attribute;
	infoItem.+desc := definicaoItem.desc;
	infoItem.+graphic := definicaoItem.Graphic ? CInt(definicaoItem.Graphic) : CInt(objtype);

	return infoItem;

endfunction

function PegaTextoDescItem(quem,objtype,definicaoItem,componentes)

	var desc := struct;
	//TODO Lembrar de lidar com as diferenças de nomenclatura para Alchemy.
	desc := "<BASEFONT COLOR=#ffe242>" + TruncatePlural(objtype) + "</BASEFONT><br>";
	desc := MontaDescricaoItem(definicaoItem,quem,desc);
	
	var materiais := PegaMateriaisNecessarios(objtype);
	if(materiais && materiais.size() > 0)
		desc +=  "<br>";
		desc +=  "<BASEFONT COLOR=#7bf8fe> Materiais Necessários</BASEFONT><br>";
		foreach material in (materiais.keys())
			desc +=  materiais[material] + " " + GetMaterialName(material)  + "<br>";
			Sleepms(2);
		endforeach
	endif

	if(componentes  && componentes.size() > 0)
		desc +=  "<br>";
		desc +=  "<BASEFONT COLOR=#7bf8fe> Encaixes de Componente </BASEFONT><br>";
		foreach componente in componentes
			desc +=  componente.qtdNecessaria + " " + componente.encaixeDesc  + "<br>";
			Sleepms(2);
		endforeach
	endif

	desc +=  "<br>";
	desc +=  "<BASEFONT COLOR=#7bf8fe> Tempo de Fabricação</BASEFONT><br>" + PegaDuracaoFabricacao() + "<br>";

	return desc;
endfunction

function PegaComponentesNecessarios(objtype)
	var encaixesItem := GetConfigStringArray(craft_config[objtype],"ComponenteEncaixe");
	var configEncaixes := ReadConfigFile(":*:encaixes");

	var componentes := array;
	foreach encaixe in encaixesItem
		var componente := struct;
		var splitEncaixe := SplitWords(encaixe);

		componente.+caracteristica :=  splitEncaixe[1];
		componente.+encaixe := splitEncaixe[2];
		componente.+qtdNecessaria := splitEncaixe[3];
		componente.+encaixeDesc := GetConfigString(configEncaixes[componente.encaixe],"Desc");
		componente.+itens := GetConfigStringArray(configEncaixes[componente.encaixe],"Item");
		componentes.append(componente);
		Sleepms(2);
	endforeach
	Print("componentes: " + componentes);

	return componentes;
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

function ClassificaRetornoComplexos(valorRetorno)
	if(valorRetorno > COMPLEXOS_CATEGORIAS && valorRetorno <= COMPLEXOS_RECEITAS)
		return RT_CATEGORIAS;
	elseif(valorRetorno > COMPLEXOS_RECEITAS)
		return RT_RECEITAS;
	else
		return RT_BOTAO_MENU;
	endif
endfunction
//Itens Complexos Fim

//Componentes Inicio
function EnviaGumpComponentes(quem)
	var retorno := 1;
	var configRecursos := ReadConfigFile(":crafting:config/resourceList");
	var configProfissao := ReadConfigFile(":woodworking:carpentry");//TODO isto tem que virar um parâmetro, porque virá do script da ferramente\estação.
	var pericia := "Wood_Working";//TODO isto tem que virar um parâmetro, porque virá do script da ferramente\estação.
	var chavesOpcoesMateriais := GetConfigStringKeys(configRecursos);
	
	var sessao := struct;
	var parametros := PreparaComponentesGump(quem,configProfissao,sessao);
	var gerenciador := sessao.gerenciadorDropDown;
	var gump;

	while(retorno)
		Print("sessao.componenteSelecionado: " + sessao.componenteSelecionado);
		Print("retorno[0]: " + retorno[0]);
		FechaTodosDropdowns(quem,gerenciador.dropDowns);

		if(sessao.componenteSelecionado && sessao.devePopularDropDown)
			PreparaDropDowns(quem,sessao,gerenciador,configRecursos,pericia);
			sessao.devePopularDropDown := false;
		endif

		MontaComponentesGump(gump, parametros,sessao);
		retorno := GFSendGump(quem,gump);

		if(find(retorno[COMPONENTES_BTN_QTD],":",1))
			sessao.quantidade := GFExtractData(retorno,COMPONENTES_BTN_QTD)[1,3];
		endif
		
		case (ClassificaRetornoComponentes(retorno[0], gerenciador))
			RT_ESCOLHA_DROPDOWN:
				var indice := retorno[0] - gerenciador.adicionalIdDropDown;
				var objetoDropDown := gerenciador.dropDowns[indice];

				FechaTodosDropdowns(quem,gerenciador.dropDowns);
				var process := SendDropDownGump(quem, GetPid(),objetoDropDown);
				
				if(process)
					objetoDropDown.pid := process.pid;
				endif
				break;

			RT_ESCOLHA_DD_OPCOES:
				GerenciadorDropDown::DefineValorEscolhido(gerenciador,retorno[0]);
				break;
			
			RT_ESCOLHA_COMPONENTE:
				GerenciadorDropDown::LimpaDropDowns(gerenciador);
				sessao.devePopularDropDown := true;
				sessao.componenteSelecionado := parametros.componentes[retorno[0] - COMPONENTES_TABELA_INICIAL];
				break;
			
			RT_BTN_FABRICAR:
				FechaTodosDropdowns(quem,gerenciador.dropDowns);
				//TODO Animações e emotes de construção.

				var materiaisEscolhidos := array;
				var isValidado := true;
				foreach dropDown in (gerenciador.dropDowns)
					var escolhido := dropDown.valorEscolhido;
					if(!escolhido || escolhido.size() == 0)
						SendSysMessageEX(quem,"Você não escolheu todos os materiais necessários.",SSM_FAIL);
						isValidado := false;
						break;
					endif

					foreach itemEncontrado in (sessao.materiaisEncontrados[dropDown.valorEscolhido.tipoMaterial][dropDown.valorEscolhido.objtype].itensEncontrados)
						if(!itemEncontrado || !Accessible(quem,itemEncontrado, 1) )
							SendSysMessageEX(quem,"Você não esta mais em posse do material {}.".format(itemEncontrado.desc),SSM_FAIL);
							isValidado := false;
							break;
						endif
						Sleepms(2);
					endforeach
					materiaisEscolhidos.append(dropDown.valorEscolhido);
					Sleepms(2);
				endforeach

				
				if(materiaisEscolhidos.size() > 0 && materiaisEscolhidos.size() == SplitWords( sessao.componenteSelecionado.materiais,",").size() && isValidado)
					ProduzComponentes(quem,
						configRecursos,
						sessao.componenteSelecionado.selecione.objtype,
						sessao.materiaisEncontrados,
						materiaisEscolhidos,
						CInt(sessao.quantidade),
						pericia);
				endif

				sessao.devePopularDropDown := true;
				break;

			RT_CANCELADO:
			default:
				FechaTodosDropdowns(quem,gerenciador.dropDowns);
				retorno := 0;
				break;
		endcase
	endwhile
endfunction

function ProduzComponentes(quem,configRecursos,objtype,materiaisEncontrados, materiaisEscolhidos,quantidade, pericia)

	var chaveMaterialPrincipal; //objtype do material que irá decidir a cor e atributos principais do componente.

	var quantidadeAFazer := CalculaQuantidadePossivel(quantidade,materiaisEscolhidos,materiaisEncontrados);
	
	if(quantidadeAFazer <= 0)
		SendSysMessageEX(quem,"Você não possui material suficiente para produzir este componente.",SSM_FAIL);
		return 0;
	endif

	ConsomeMateriais(chaveMaterialPrincipal,materiaisEncontrados,materiaisEscolhidos,quantidadeAFazer);

	var qualidades := DistribuiQualidadeComponentes(quantidadeAFazer,AP_GetSkill(quem,pericia));
	

	foreach qualidade in (qualidades.keys())
		var descriptor := GetItemDescriptor(CInt(objtype));
		descriptor.CProps.insert("material",chaveMaterialPrincipal);
		descriptor.CProps.insert("tierComponente",qualidade); // Usar constante ou cfg vinda do include do craft ao invés de string crua.
		descriptor.color := configRecursos[chaveMaterialPrincipal].Color;
		
		var item := CreateItemInBackpack(quem,descriptor,cint(qualidades[qualidade]));
		
		Sleepms(2);
	endforeach

	SendSysMessageEX(quem,"{} componete(s) criado(s).".format(quantidadeAFazer),SSM_INFO);
endfunction

function CalculaQuantidadePossivel(quantidade,materiaisEscolhidos, materiaisEncontrados)
	
	if(!materiaisEscolhidos || materiaisEscolhidos.size() == 0 )
		return 0;
	endif
	
	var quantidadeAFazer := CInt(quantidade);
	
	foreach materialEscolhido in materiaisEscolhidos

		var qtdPossivel := cInt(materiaisEncontrados[materialEscolhido.tipoMaterial][materialEscolhido.objtype].quantidadeTotal / materialEscolhido.quantidadeNecessaria);
		qtdPossivel := qtdPossivel >= quantidade? cInt(quantidade) : qtdPossivel;
		
		if(qtdPossivel < quantidadeAFazer) // a menor quantidade possivel determina a quantidade final.
			quantidadeAFazer := qtdPossivel;
		endif
		Sleepms(2);
	endforeach
	
	return quantidadeAFazer;
endfunction

function ConsomeMateriais(byref chaveMaterialPrincipal,materiaisEncontrados,materiaisEscolhidos,quantidadeAFazer)

	foreach materialEscolhido in materiaisEscolhidos
	
		if(materialEscolhido.isPrincipal) 
			chaveMaterialPrincipal := materialEscolhido.objtype; 
		endif

		var quantidadeConsumo := cInt(materialEscolhido.quantidadeNecessaria) * quantidadeAFazer;
		var quantidadeRestante := quantidadeConsumo;

		foreach itemEncontrado in (materiaisEncontrados[materialEscolhido.tipoMaterial][materialEscolhido.objtype].itensEncontrados)			

			If(quantidadeRestante <= 0) 
				break; 
			endif;
			
			if(itemEncontrado.stackable)
				SubtractAmount(itemEncontrado, quantidadeRestante);
				quantidadeRestante -= itemEncontrado.amount;
			else
				quantidadeRestante -= 1;
				DestroyItem(itemEncontrado);
			endif
			Sleepms(2);
		endforeach

		Sleepms(2);
	endforeach

endfunction

function PreparaDropDowns(quem, byref sessao,byref gerenciador, configRecursos,pericia)
	var materiais := SplitWords( sessao.componenteSelecionado.materiais,",");
	sessao.materiaisEncontrados := CarregaMateriaisEncontrados(quem,configRecursos,pericia,sessao.componenteSelecionado.materialPrincipal);
	
	foreach material in materiais

		var dropDown := GerenciadorDropDown::EncontraDropDown(gerenciador,_material_iter);
		
		if(!dropDown) 
			dropDown := GerenciadorDropDown::RegistraDropDown(gerenciador,_material_iter);
		else
			DropDown::ResetaValores(dropDown);
		endif

		var splitMaterial := SplitWords(material);
		var tipo := Trim(Lower(splitMaterial[1]));
		var qtd := splitMaterial[2];
		var indiceOpcoes := 1;
		var materiaisEncontrados := sessao.materiaisEncontrados[tipo].keys();

		if(materiaisEncontrados)

			var isValorEscolhidoPresente := false;

			foreach chave in (materiaisEncontrados)
				Sleepms(2);
				if (dropDown.valorEscolhido.objtype == chave)
					isValorEscolhidoPresente := true;
				endif

				var retorno := DropDown::AdicionaOpcao(dropDown,
				configRecursos[chave].name,
				indiceOpcoes,
				MontaValorOpcao(chave,qtd,tipo,sessao));

				if(!retorno) 
					Print("retorno: " + retorno); 
					continue;
				endif;
				++indiceOpcoes;
			endforeach

			if(!isValorEscolhidoPresente)
				DropDown::LimpaValorEscolhido(dropDown);
			endif
		else
			DropDown::LimpaValorEscolhido(dropDown);
		endif
		
		Sleepms(2);
	endforeach
endfunction

function MontaValorOpcao(chave,qtd,tipo,sessao)

	var valorOpcao := struct {
					objtype := chave,
					nome := configRecursos[chave].name,
					quantidadeNecessaria := qtd,
					tipoMaterial := tipo,
					isPrincipal := tipo == Trim(Lower(sessao.componenteSelecionado.materialPrincipal))
					};

	return valorOpcao;
endfunction

function CarregaMateriaisEncontrados(quem,configRecursos, pericia, materialPrincipal)
	var retorno := dictionary;

	foreach item in EnumerateItemsInContainer(quem.backpack)
		Sleepms(2);
		var materialDetalhe := configRecursos[item.objtype];
		if(materialDetalhe)
			
			var tipo := Lower(materialDetalhe.type);
			if(materialPrincipal == tipo &&  materialDetalhe.Skill > AP_GetSkill(quem,pericia)) continue; endif
			MontaMaterialEncontrado(retorno,tipo,item);
		else
			var itemCfg := itemsConfig[item.objtype];

			if(itemCfg)
				MontaMaterialEncontrado(retorno,item.objtype,item);
			endif
		endif
	endforeach 

	return retorno;
endfunction

function MontaMaterialEncontrado(byref categorias, tipo, item)
	var dictitemsEncontrados;

	if( !categorias.exists(tipo)) // garante que o dictionary do tipo exista nas categorias de materiais.
		categorias.insert(tipo, dictionary);
	endif

	dictitemsEncontrados := categorias[tipo]; 

	var pacoteItem := struct{ itensEncontrados := array , quantidadeTotal := 0};

	if(!dictitemsEncontrados.exists(item.objtype)) // contabiliza os itens, se nao existir cria novo registro de contabilidade.
		pacoteItem.itensEncontrados.append(item);
		pacoteItem.quantidadeTotal := item.amount;
		dictitemsEncontrados.insert(item.objtype,pacoteItem);

		categorias[tipo] := dictitemsEncontrados;
	else
		dictitemsEncontrados[item.objtype].itensEncontrados.append(item);
		dictitemsEncontrados[item.objtype].quantidadeTotal += item.amount;
	endif
endfunction

function FechaTodosDropdowns(quem,dropdowns)
	foreach dropdown in dropdowns

		if(GetProcess( dropdown.pid ))
			CloseGump(quem,dropdown.idBottao);
		endif
		Sleepms(2);
	endforeach
endfunction

function PreparaComponentesGump(quem, configProfissao,byref sessao)

	var parametrosGump := struct;
	
	var chavesComponentes := GetConfigStringKeys(configProfissao);

	var materiais := array;
	var componentes := array;

	sessao.+gerenciadorDropDown := GerenciadorDropDown(30,
										COMPONENTES_DROPDOWN_INICIAL,
										COMPONENTES_DROPDOWN_OPCOES_INICIAL,
										COMPONENTES_DROPDOWN_MAX);
	sessao.+componenteSelecionado := 0;
	sessao.+materiaisEncontrados := 0;
	sessao.+devePopularDropDown := true;
	sessao.+quantidade := "1";

	foreach chave in chavesComponentes
		Sleepms(2);
		if(!configProfissao[chave].encaixeComponente)
			continue;
		endif

		var materiaisNecessarios := GetConfigStringArray( configProfissao[chave], "materials" );
		var materiaisConcatenados := "";  
		foreach material in materiaisNecessarios
			if(_material_iter != 1)
				materiaisConcatenados += ", ";
			endif

			materiaisConcatenados += material;
			Sleepms(2);
		endforeach

		var componente := struct;
		componente.+nome := configProfissao[chave].name;
		componente.+estacao := itemsConfig[configProfissao[chave].estacao].desc;
		componente.+materialPrincipal := configProfissao[chave].principalMaterial;
		componente.+materiais := materiaisConcatenados;
		componente.+encaixe := configProfissao[chave].encaixeComponente;
		componente.+selecione := struct{isColunaBotao := true, objtype := chave};

		componentes.append(componente);
	endforeach

	var colCfg := dictionary;
		colCfg["nome"] := struct{titulo := "Nome",ordem := 1, largura := 2};
		colCfg["materiais"] := struct{titulo := "Materiais",ordem := 2,largura := 4};
		colCfg["materialPrincipal"] := struct{titulo := "Principal Material",ordem := 3,largura := 2};
		colCfg["estacao"] := struct{titulo := "Estação",ordem := 5,largura := 1.5};
		colCfg["encaixe"] := struct{titulo := "Encaixe",ordem := 4,largura := 1.5};
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

		if(isFalhouTudo) //Se falhou tudo, coloca na pior qualidade.
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

function ClassificaRetornoComponentes(valorRetorno, gerenciador)
	if(GerenciadorDropDown::IsIdDentroDropDowns(gerenciador,valorRetorno))
		return RT_ESCOLHA_DROPDOWN;
	elseif(GerenciadorDropDown::IsIdDentroOpcoes(gerenciador,valorRetorno))
		return RT_ESCOLHA_DD_OPCOES;
	elseif(valorRetorno >= COMPONENTES_TABELA_INICIAL)
		return RT_ESCOLHA_COMPONENTE;
	elseif(valorRetorno == COMPONENTES_BTN_FABRICAR)
		return RT_BTN_FABRICAR;
	elseif(valorRetorno == 1)
		return RT_CANCELADO;
	else
		return RT_CANCELADO;
	endif
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