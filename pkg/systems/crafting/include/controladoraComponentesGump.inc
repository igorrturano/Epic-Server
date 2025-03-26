//Author: Deiro || Rafael Santos

use Math;

include "/include/epicConstants";
include "/include/epicPropNames";
include "/include/math";
include "include/epicUtils";
include ":crafting:craftingUtil";
include ":megacliloc:/include/descricaoObjetoTTip";
include ":crafting:gumps/itensComplexosGump";
include ":gumps:/include/dropDownGump";
include ":crafting:/craftingDFEsquemas";
include ":crafting:/include/fabricacaoUtil";
include "include/epicValidAction";

var itemsConfig := ReadConfigFile(":*:itemdesc");

function EnviaGumpComponentes(quem)
	var retorno := 1;
	var configRecursos := ReadConfigFile(":crafting:config/resourceList");
	var configProfissao := ReadConfigFile(":woodworking:carpentry");//TODO isto tem que virar um parâmetro, porque virá do script da ferramente\estação.
	var pericia := "Wood_Working";//TODO isto tem que virar um parâmetro, porque virá do script da ferramente\estação.
	var chavesOpcoesMateriais := GetConfigStringKeys(configRecursos);
	
	var sessao := struct;
	var parametros := PreparaComponentesGump(quem,configProfissao,sessao, pericia);
	var gerenciador := sessao.gerenciadorDropDown;
	var gump;

	while(retorno)
		FechaTodosDropdowns(quem,gerenciador.dropDowns);

		if(sessao.componenteSelecionado && sessao.devePopularDropDown)
			PreparaDropDownsComponentes(quem,sessao,gerenciador,configRecursos,pericia);
			sessao.devePopularDropDown := false;
		endif

		MontaComponentesGump(gump, parametros,sessao);
		retorno := GFSendGump(quem,gump);

		if(find(retorno[COMPONENTES_INPT_QTD],":",1))
			sessao.quantidade := GFExtractData(retorno,COMPONENTES_INPT_QTD)[1,3];
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
			
			RT_BTN_ESFORCO_MAIS:
				sessao.picoPericia += 1;
				if(sessao.picoPericia > parametros.picoMaximoPericia)
					sessao.picoPericia:= parametros.picoMaximoPericia;
				endif
				break;
				
			RT_BTN_ESFORCO_MENOS:
				sessao.picoPericia -= 1;
				if(sessao.picoPericia < 0)
					sessao.picoPericia := 0;
				endif
				break;

			RT_BTN_AVALIAR:
				var alvo :=  Target(quem);
				if(alvo)
					var materialDetalhes := configRecursos[alvo.objtype];
					if(materialDetalhes)
						var isTrabalhavel := materialDetalhes.Skill <= AP_GetSkill(quem,pericia);
						if(isTrabalhavel)
							SendSysMessageEX(quem,"Você tem conhecimento para trabalhar este material.",SSM_HELPFU);
						else
							SendSysMessageEX(quem,"Você não pode trabalhar este material.",SSM_HARMFUL);
						endif
					else
						SendSysMessageEX(quem,"Você não pode trabalhar este material.",SSM_HARMFUL);
					endif
				else
					SendSysMessageEX(quem,"Alvo cancelado.",SSM_FAIL);
				endif
				
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
					
					var quantidade := CInt(sessao.quantidade) ;
					if(quantidade <= 0)
						SendSysMessageEX(quem, "A quantidade não pode ser menor que 1.", SSM_FAIL);
						break;
					endif

					var quantidadePorLote := 20;
					var lotes := Ceil(CDBL(quantidade) / quantidadePorLote);
					var resto := quantidade % quantidadePorLote;
					lotes := lotes < 1 ? 1 : lotes;
					var posicaoInicial :=  CriaObjetoPosicao(quem);
					var vidaInicial := AP_GetVital(quem, "Hits");

					for i := 1 to lotes

						var interrompido := CheckCanWork(quem, vidaInicial, posicaoInicial);
						if(!interrompido)
							break;
						endif

						Sleep(2);
						ProduzComponentes(quem,
							configRecursos,
							sessao.componenteSelecionado.selecione.objtype,
							sessao.materiaisEncontrados,
							materiaisEscolhidos,
							i == lotes && resto > 0 ? resto : quantidadePorLote,
							pericia,
							sessao.componenteSelecionado.encaixe,
							sessao.picoPericia);
					endfor
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

function ProduzComponentes(quem,configRecursos,objtype,materiaisEncontrados, materiaisEscolhidos,quantidade, pericia, encaixe, picoPericia)

	var chaveMaterialPrincipal; //objtype do material que irá decidir a cor e atributos principais do componente.

	var quantidadeAFazer := CalculaQuantidadePossivel(quantidade,materiaisEscolhidos,materiaisEncontrados);
	
	if(quantidadeAFazer <= 0)
		SendSysMessageEX(quem,"Você não possui material suficiente para produzir este componente.",SSM_FAIL);
		return 0;
	endif

	ConsomeMateriais(chaveMaterialPrincipal,materiaisEncontrados,materiaisEscolhidos,quantidadeAFazer);

	var qualidades := DistribuiQualidadeComponentes(quantidadeAFazer,picoPericia);
	
	foreach qualidade in (qualidades.keys())
		var descriptor := GetItemDescriptor(CInt(objtype));
		descriptor.CProps.insert(PROPNAME_MATERIAL,chaveMaterialPrincipal);
		descriptor.CProps.insert(PROPNAME_TIER_COMPONENTE,qualidade);
		descriptor.CProps.insert(PROPNAME_ENCAIXE_COMPONENTE,encaixe);
		descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,true);
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

function PreparaDropDownsComponentes(quem, byref sessao,byref gerenciador, configRecursos,pericia)
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
				MontaValorOpcaoComponente(chave,qtd,tipo,sessao));

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

function MontaValorOpcaoComponente(chave,qtd,tipo,sessao)

	return struct {
					objtype := chave,
					nome := configRecursos[chave].name,
					quantidadeNecessaria := qtd,
					tipoMaterial := tipo,
					isPrincipal := tipo == Trim(Lower(sessao.componenteSelecionado.materialPrincipal))
					};
endfunction

function CarregaMateriaisEncontrados(quem,configRecursos, pericia, materialPrincipal)
	var retorno := dictionary;

	foreach item in EnumerateItemsInContainer(quem.backpack)
		Sleepms(2);
		var materialDetalhe := configRecursos[item.objtype];
		if(materialDetalhe)
			var tipoMaterial := Lower(materialDetalhe.type);
			if(materialPrincipal == tipoMaterial &&  materialDetalhe.Skill > AP_GetSkill(quem,pericia)) continue; endif
			MontaMaterialEncontrado(retorno,tipoMaterial,item);
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

function PreparaComponentesGump(quem, configProfissao,byref sessao, pericia)

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
	sessao.+picoPericia := AP_GetSkill(quem,pericia) / 20;

	foreach chave in chavesComponentes
		Sleepms(2);
		if(!configProfissao[chave].encaixe)
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
		componente.+encaixe := configProfissao[chave].encaixe;
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
	parametrosGump.+picoMaximoPericia := sessao.picoPericia;
	
	return parametrosGump;
endfunction

// TODO [futuras versões] pensar em um modo de aplicar uma dificuldade a mais dependendo do componente.
function DistribuiQualidadeComponentes(quantidade,valorPico, deslocamentoPico := -1) 
	
	if(quantidade == error || quantidade.size() == 0)
		return error{"errortext":="O parâmetro quantidade é obrigatório."};
	endif

	var	chancesPorTier := DistribuicaoGaussianaNormalizada({1,2,3,4,5},valorPico + deslocamentoPico,1.4);
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
	elseif(valorRetorno == COMPONENTES_BTN_ESFORCO_MAIS)
		return RT_BTN_ESFORCO_MAIS;
	elseif(valorRetorno == COMPONENTES_BTN_ESFORCO_MENOS)
		return RT_BTN_ESFORCO_MENOS;
	elseif(valorRetorno == COMPONENTES_BTN_AVALIAR)
		return RT_BTN_AVALIAR;
	elseif(valorRetorno == 1)
		return RT_CANCELADO;
	else
		return RT_CANCELADO;
	endif
endfunction
