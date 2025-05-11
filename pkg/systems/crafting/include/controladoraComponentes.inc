//Author: Deiro || Rafael Santos

use Math;

include "/include/epicConstants";
include "/include/epicPropNames";
include "/include/math";
include "include/epicUtils";
include ":crafting:craftingUtil";
include ":megacliloc:/include/descricaoObjetoTTip";
include ":crafting:gumps/fabricacaoGumps";
include ":gumps:/include/dropDownGump";
include ":crafting:/craftingDFEsquemas";
include ":crafting:/include/fabricacaoUtil";
include "include/epicValidAction";

function PegaPericiaPeloMaterial(tipoMaterial)
	case (tipoMaterial)
		"metal":
		"gems":
			return "Metal_Working";
		"cloth":
		"ossos":
		"leather":
			return "Domestic_Work";
		"wood":
			return "Wood_Working";
		"ingredient": //TODO pedir pro l4nc1s definir um tipo pros materiais de alquimia 
			return "Alchemy";
		default:
			return "Wood_Working";
			break;
	endcase
endfunction

function DecideConfigComponentes(pericia)
	var retorno;
	case(pericia)
		"Wood_Working": retorno :=  ":woodworking:"; break;
		"Metal_Working": retorno :=  ":metalworking:"; break;
		"Alchemy":  retorno :=  ":alchemy:"; break;
		"Domestic_Work": retorno :=  ":DomesticWork:"; break;
		default:
			retorno := ":*:"; break;
	endcase

	retorno += "componentes";
	return retorno;
endfunction

function EnviaGumpComponentes(quem, pericia := "todas")
	var retorno := 1;
	var configRecursos := ReadConfigFile(RECURSOS_CONFIG_PATH);
	var configComponentes := ReadConfigFile(DecideConfigComponentes(pericia));
	var chavesOpcoesMateriais := GetConfigStringKeys(configRecursos);
	
	var sessao := struct;
	var parametros := PreparaComponentesGump(quem,configComponentes,sessao, pericia);
	var gerenciador := sessao.gerenciadorDropDown;
	var gump;

	while(retorno)
		FechaTodosDropdowns(quem,gerenciador.dropDowns);

		if(sessao.componenteSelecionado && sessao.devePopularDropDown)
			PreparaDropDownsComponentes(quem,sessao,gerenciador,configRecursos);
			sessao.devePopularDropDown := false;
		endif

		MontaComponentesGump(gump, parametros,sessao);
		retorno := GFSendGump(quem,gump);

		
		if(find(retorno[COMPONENTES_INPT_QTD],":",1))
			sessao.quantidade := GFExtractData(retorno,COMPONENTES_INPT_QTD)[1,3];
		endif
		sessao.filtros.encaixe := ExtraiStringEntradaOuVazio(retorno,COMPONENTES_INPT_FILTRO_ENCAIXE);
		sessao.filtros.nome := ExtraiStringEntradaOuVazio(retorno,COMPONENTES_INPT_FILTRO_NOME);

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
				sessao.componenteSelecionado := sessao.componentes[retorno[0] - COMPONENTES_TABELA_INICIAL];
				sessao.picoPericia :=  CalculaValorPico(quem, configComponentes[sessao.componenteSelecionado.selecione.objtype].pericia);
				sessao.picoMaximoPericia :=  sessao.picoPericia;
				break;
			
			RT_BTN_ESFORCO_MAIS:
				sessao.picoPericia += 1;
				if(sessao.picoPericia > sessao.picoMaximoPericia)
					sessao.picoPericia:= sessao.picoMaximoPericia;
				endif
				break;
				
			RT_BTN_ESFORCO_MENOS:
				sessao.picoPericia -= 1;
				if(sessao.picoPericia < 0)
					sessao.picoPericia := 0;
				endif
				break;

			RT_BTN_FILTRAR:
				AtualizaPesquisaComponentes(configComponentes,sessao);
				break;

			RT_BTN_LIMPA_FILTRO:
				sessao.filtros := struct{ nome := "", encaixe := ""};
				AtualizaPesquisaComponentes(configComponentes,sessao);
				break;

			RT_BTN_AVALIAR:
				var alvo :=  Target(quem);
				if(alvo)
					var materialDetalhes := configRecursos[alvo.objtype];
					if(materialDetalhes)
						var isTrabalhavel := materialDetalhes.Skill <= AP_GetSkill(quem,configComponentes[sessao.componenteSelecionado.selecione.objtype].pericia);
						if(isTrabalhavel)
							SendSysMessageEX(quem,"Você tem conhecimento para trabalhar o material indicado.",SSM_HELPFU);
						else
							SendSysMessageEX(quem,"Você não pode trabalhar o material indicado para este componente .",SSM_HARMFUL);
						endif
					else
						SendSysMessageEX(quem,"O Alvo não é um material trabalhavel.",SSM_HARMFUL);
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

function ExtraiStringEntradaOuVazio(entrada, id)
	if(!find(entrada[id],":",1)) return ""; endif

	var chave := CStr(id)+": ";
	var stringDados := entrada[id];
	if( entrada[id] == chave)
		return "";
	else
		return stringDados[Len(chave)+1,Len(stringDados)];
	endif
endfunction

function AtualizaPesquisaComponentes(configComponentes,byref sessao)
	sessao.componentes := MontaVisaoComponentes(configComponentes, sessao.filtros.encaixe, sessao.filtros.nome);
	sessao.devePopularDropDown := true;
	sessao.componenteSelecionado := 0;
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
		var descriptor :=  CriaComponenteDescriptor(objtype,chaveMaterialPrincipal,qualidade,encaixe, configRecursos);
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

function PreparaDropDownsComponentes(quem, byref sessao,byref gerenciador, configRecursos)
	var materiais := SplitWords( sessao.componenteSelecionado.materiais,",");
	sessao.materiaisEncontrados := CarregaMateriaisEncontrados(quem,configRecursos,sessao.componenteSelecionado.materialPrincipal);
	
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

function CarregaMateriaisEncontrados(quem,configRecursos, materialPrincipal)
	var retorno := dictionary;

	foreach item in EnumerateItemsInContainer(quem.backpack)
		Sleepms(2);
		var materialDetalhe := configRecursos[item.objtype];
		if(materialDetalhe)
			materialPrincipal := Lower(materialPrincipal);
			var tipoMaterial := Lower(materialDetalhe.type);
			var pericia := PegaPericiaPeloMaterial(materialPrincipal);

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
	sessao.+picoPericia := CalculaValorPico(quem,pericia);
	sessao.+picoMaximoPericia := sessao.picoPericia;
	sessao.+filtros := struct{ nome := "", encaixe := ""};
	sessao.+componentes := MontaVisaoComponentes(configProfissao);

	var colCfg := dictionary;
		colCfg["nome"] := struct{titulo := "Nome",ordem := 1, largura := 3};
		colCfg["materiais"] := struct{titulo := "Materiais",ordem := 2,largura := 4.5};
		colCfg["materialPrincipal"] := struct{titulo := "Principal",ordem := 3,largura := 1.5};
		colCfg["encaixe"] := struct{titulo := "Encaixe",ordem := 4,largura := 2};
		colCfg["selecione"] := struct{titulo := "Selecione",ordem := 6,largura := 1};

	parametrosGump.+configColunas := colCfg;
	parametrosGump.+materiais := materiais;
	
	
	return parametrosGump;
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
	elseif(valorRetorno == COMPONENTES_BTN_FILTRAR)
		return RT_BTN_FILTRAR;
	elseif(valorRetorno == COMPONENTES_BTN_LIMPA_FILTRO)
		return RT_BTN_LIMPA_FILTRO;
	elseif(valorRetorno == 1)
		return RT_CANCELADO;
	else
		return RT_CANCELADO;
	endif
endfunction

function MontaVisaoComponentes(configProfissao, filtroEncaixe := "", filtroNome := "")
	var componentes := array;
	var chavesComponentes := GetConfigStringKeys(configProfissao);

	foreach chave in chavesComponentes
		Sleepms(2);
		if(!configProfissao[chave].encaixe)
			continue;
		endif

		if(filtroEncaixe != "" && !Find(UPPER(configProfissao[chave].encaixe),UPPER(filtroEncaixe),1))
			continue;
		endif

		if(filtroNome != "" && !Find(UPPER(configProfissao[chave].name),UPPER(filtroNome),1))
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
		componente.+materialPrincipal := configProfissao[chave].principalMaterial;
		componente.+materiais := materiaisConcatenados;
		componente.+encaixe := configProfissao[chave].encaixe;
		componente.+selecione := struct{isColunaBotao := true, objtype := chave};

		componentes.append(componente);
	endforeach

	return componentes;
endfunction