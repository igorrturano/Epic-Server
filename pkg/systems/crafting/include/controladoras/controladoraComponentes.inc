//Author: Deiro || Rafael Santos

use math;

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
	case (Lower(tipoMaterial))
		"metal":
		"gem":
		"glass":
		"stone":
		"brick":
		"arrowhead":
			return "Metal_Working";
		"cloth":
		"ossos":
		"leather":
		"scales":
			return "Domestic_Work";
		"wood":
			return "Wood_Working";
		"catalist":
		"oleo":
		"solvente":
		"alcool":
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
		GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);

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
				var indice := retorno[0] - gerenciador.PegaAdicionalIdDropDown();
				var objetoDropDown := gerenciador.BuscaDropDownPorPosicao(indice);

				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				objetoDropDown.SendGump(quem,GetPid());
				break;

			RT_ESCOLHA_DD_OPCOES:
				GerenciadorDropDown::DefineValorEscolhido(gerenciador,retorno[0]);
				break;
			
			RT_ESCOLHA_COMPONENTE:
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
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

			RT_CHK_BOX_RECICLAR:
				sessao.deveReciclar := !sessao.deveReciclar;
				var propCraft := MontaCraftProp(0,  GetObjProperty(quem, PROPNAME_VELHA_CRAFTPROP));
				propCraft.deveReciclar := sessao.deveReciclar;
				SetObjProperty(quem, PROPNAME_VELHA_CRAFTPROP, propCraft);
				break;	

			RT_BTN_FILTRAR:
				AtualizaPesquisaComponentes(configComponentes,sessao);
				break;

			RT_BTN_LIMPA_FILTRO:
				sessao.filtros := struct{ nome := "", encaixe := ""};
				AtualizaPesquisaComponentes(configComponentes,sessao);
				break;

			RT_LIMPAR_MEMORIA_COMPNT:
				LimparMemoriaEncaixes(quem);
				sessao.encaixesMemorizados := array;
				SendSysMessageEX(quem,"Os encaixes memorizados foram esquecidos.", SSM_INFO);
				break;

			RT_FILTRO_MEMORIA_LIG_DES:
				sessao.isFiltroMemoria := !sessao.isFiltroMemoria;
				var estadoFiltro := sessao.isFiltroMemoria ? "Ligado." : "Desligado.";
				SendSysMessageEX(quem,"Filtro por memória " +  estadoFiltro, SSM_INFO);
				AtualizaPesquisaComponentes(configComponentes,sessao);
				break;

			RT_BTN_AVALIAR:
				var alvo :=  Target(quem);
				if(alvo)
					var materialDetalhes := configRecursos[alvo.objtype];
					if(materialDetalhes)
						var informacaoComponente := configComponentes[sessao.componenteSelecionado.selecione.objtype];
						var isTrabalhavel := materialDetalhes.Skill <= EffectiveSkill(quem, informacaoComponente.pericia);
						if(IsMaterialTrabalhavel(quem, sessao.componenteSelecionado, alvo.objtype))
							SendSysMessageEX(quem,"Você tem conhecimento para trabalhar o material para este componente.",SSM_HELPFU);
						else
							SendSysMessageEX(quem,"Você não pode trabalhar o material indicado para este componente. (Obs: Só há dificuldade para materiais principais.)",SSM_HARMFUL);
						endif
					else
						SendSysMessageEX(quem,"O Alvo não é um material trabalhável.",SSM_HARMFUL);
					endif
				else
					SendSysMessageEX(quem,"Alvo cancelado.",SSM_FAIL);
				endif
				
			RT_BTN_FABRICAR:
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);

				var materiaisEscolhidos := array;
				var isValidado := true;
				foreach dropDown in (gerenciador.PegaDropDowns())
					var escolhido := dropDown.PegaValorEscolhido();
					if(!escolhido || escolhido.size() == 0)
						SendSysMessageEX(quem,"Você não escolheu todos os materiais necessários.",SSM_FAIL);
						isValidado := false;
						break;
					endif

					foreach itemEncontrado in (sessao.materiaisEncontrados[escolhido.tipoMaterial][escolhido.objtype].itensEncontrados)
						if(!itemEncontrado || !Accessible(quem,itemEncontrado, 1) )
							SendSysMessageEX(quem,"Você não esta mais em posse do material {}.".format(itemEncontrado.desc),SSM_FAIL);
							isValidado := false;
							break;
						endif
						Sleepms(2);
					endforeach
					materiaisEscolhidos.append(escolhido);
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
					var posicaoInicial :=  BuildPositionStruct(quem);
					var vidaInicial := AP_GetVital(quem, "Hits");

					for i := 1 to lotes
						
						var interrompido := CheckCanWork(quem, vidaInicial, posicaoInicial);
						
						if(!interrompido)
							break;
						endif
						Sleep(2);
						var isProduzido := ProduzComponentes(quem,
																	configRecursos,
																	sessao.componenteSelecionado.selecione.objtype,
																	sessao.materiaisEncontrados,
																	materiaisEscolhidos,
																	sessao.deveReciclar,
																	i == lotes && resto > 0 ? resto : quantidadePorLote,
																	sessao.componenteSelecionado.selecione.encaixeReal,
																	sessao.picoPericia,
																	Cint(configComponentes[sessao.componenteSelecionado.selecione.objtype].lote));
															
						if(!isProduzido) break; endif
					endfor
				endif

				sessao.devePopularDropDown := true;
				break;

			RT_CANCELADO:
			default:
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
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
	sessao.componentes := MontaVisaoComponentes(configComponentes, sessao.isFiltroMemoria, sessao.encaixesMemorizados, sessao.filtros.encaixe, sessao.filtros.nome);
	sessao.devePopularDropDown := true;
	sessao.componenteSelecionado := 0;
endfunction

function ProduzComponentes(quem,configRecursos,objtype,byref materiaisEncontrados, materiaisEscolhidos, deveReciclar,quantidade, encaixe, picoPericia, lote)

	var maiorQualidade := 1;
	var quantidadeRecicladaTotal := 0;
	var quantidadeMaterialRetornado := 0;
	var chaveMaterialPrincipal; //objtype do material que irá decidir a cor e atributos principais do componente.
	var quantidadeAFazer := CalculaQuantidadePossivel(quantidade,materiaisEscolhidos,materiaisEncontrados);
	
	if(quantidadeAFazer <= 0)
		SendSysMessageEX(quem,"Você não possui material suficiente para produzir este componente.",SSM_FAIL);
		return 0;
	endif

	var isConsumido := ConsomeMateriais(quem,chaveMaterialPrincipal,materiaisEncontrados,materiaisEscolhidos,quantidadeAFazer);
	if(!isConsumido)
		SendSysMessageEX(quem,"Consumo de material interrompido, ação cancelada.",SSM_FAIL);
		return 0;
	endif

	PerformAction( quem, HUMAN_ANIM::HORSE_ATTACK_XBOW ); 
	PlaySoundEffect(quem, SFX_56);

	lote := lote ? lote : 1;
	var qualidades := DistribuiQualidadeComponentes(quantidadeAFazer * lote,picoPericia);
	
	if(deveReciclar)
		maiorQualidade := Max(qualidades.keys());
	endif

	foreach qualidade in (qualidades.keys())
		Sleepms(2);
		if(deveReciclar && qualidade != maiorQualidade)
			var quantidadeTier :=  cint(qualidades[qualidade]);
			quantidadeRecicladaTotal += quantidadeTier;
			quantidadeMaterialRetornado += CriaMaterialReciclado(quem,materiaisEscolhidos,lote,quantidadeTier);
			continue;
		endif
		var descriptor :=  CriaComponenteDescriptor(objtype,chaveMaterialPrincipal,qualidade,encaixe, configRecursos);
		var qtdTier := cint(qualidades[qualidade]);
		for j := 1 to qtdTier
			var item := CreateItemInBackpack(quem,descriptor,1);
			if(!item)
				item := CreateItemAtLocation( quem.x, quem.y, quem.z, descriptor, 1, quem.realm);
			endif
			if(item) item.hp := item.maxhp; endif
			Sleepms(2);
		endfor
	endforeach

	if(quantidadeRecicladaTotal > 0)
		SendSysMessageEX(quem,"Reciclado(s) {}, retornando {} materiai(s).".format(quantidadeRecicladaTotal, quantidadeMaterialRetornado),SSM_INFO_IC);
	endif
	SendSysMessageEX(quem,"{} componete(s) criado(s).".format((quantidadeAFazer * lote) - quantidadeRecicladaTotal),SSM_INFO);
	return 1;
endfunction

function CriaMaterialReciclado(quem, materiaisEscolhidos, lote, quantidade)
	var profissaoConfig := ReadConfigFile(PROFISSAO_CONFIG_PATH);
	var Percentuais := SplitWords(profissaoConfig["Setup"].ReciclagemPercentual);
	var percentualFinal := RandomIntMinMax(cint(Percentuais[1]), cint(Percentuais[2]));
	var percentualReciclagem := percentualFinal / 100.0;
	var materialRecicladoTotal := 0;

	foreach materialEscolhido in materiaisEscolhidos
		Sleepms(2);

		var quantidadeReciclada := cint(cint(materialEscolhido.quantidadeNecessaria) * (quantidade / lote) * percentualReciclagem);
		if(quantidadeReciclada <= 1) 
			if(RandomInt(100) <= (percentualFinal + 1))
				quantidadeReciclada := 1;
			else
				continue;
			endif
		endif

		materialRecicladoTotal += quantidadeReciclada;
		var descriptor := GetItemDescriptor(materialEscolhido.objtype);
		descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,true);

		var item := CreateItemInBackpack(quem,descriptor, quantidadeReciclada);
		if(!item)
			item := CreateItemAtLocation( quem.x, quem.y, quem.z, descriptor, quantidadeReciclada, quem.realm);
		endif
	endforeach

	return materialRecicladoTotal;
endfunction

function CalculaQuantidadePossivel(quantidade, materiaisEscolhidos,byref materiaisEncontrados)
	
	if(!materiaisEscolhidos || materiaisEscolhidos.size() == 0 )
		return 0;
	endif

	var quantidadeAFazer := quantidade;
	
	foreach materialEscolhido in materiaisEscolhidos
		var total := materiaisEncontrados[Lower(materialEscolhido.tipoMaterial)][materialEscolhido.objtype].quantidadeTotal;
		var qtdPossivel := cInt(cint(total) / cint(materialEscolhido.quantidadeNecessaria) );
		qtdPossivel := qtdPossivel >= cint(quantidade)? cInt(quantidade) : qtdPossivel;
	
		if(qtdPossivel < quantidadeAFazer) // a menor quantidade possivel determina a quantidade final.
			quantidadeAFazer := qtdPossivel;
		endif
		Sleepms(2);
	endforeach
	return quantidadeAFazer;
endfunction

function ConsomeMateriais(quem,byref chaveMaterialPrincipal,byref materiaisEncontrados,materiaisEscolhidos,quantidadeAFazer)
	foreach materialEscolhido in materiaisEscolhidos
			
	
		if(materialEscolhido.isPrincipal) 
			chaveMaterialPrincipal := materialEscolhido.objtype; 
		endif

		var quantidadeConsumo := cInt(materialEscolhido.quantidadeNecessaria) * quantidadeAFazer;
		var quantidadeRestante := quantidadeConsumo;
		
		foreach itemEncontrado in (
							materiaisEncontrados
								[Lower(materialEscolhido.tipoMaterial)]
								[materialEscolhido.objtype]
								.itensEncontrados
							)
			if(!itemEncontrado)
				continue;
			endif			
			If(quantidadeRestante <= 0) 
				break; 
			endif;
			
			var objtypeConsumido := itemEncontrado.objtype;
			if(itemEncontrado.stackable)
				var quantidadeASubtrair := quantidadeRestante;
				var quantidadeItem := itemEncontrado.amount;
				var redutor;
				var isSucesso := SubtractAmount(itemEncontrado, quantidadeRestante);
				quantidadeRestante := quantidadeRestante  - cint(quantidadeItem);

				if(quantidadeRestante < 0)
					redutor := quantidadeASubtrair;
				else
					redutor := quantidadeItem;
				endif

				if(isSucesso)
					SubtraiTotal(
						materiaisEncontrados,
						Lower(materialEscolhido.tipoMaterial),
						materialEscolhido.objtype,
						redutor
					);
					DevolveContainerVazio(quem, objtypeConsumido, redutor);
				else
					return 0;
				endif

			else

				quantidadeRestante -= 1;
				DestroyItem(itemEncontrado);
				SubtraiTotal(materiaisEncontrados, Lower(materialEscolhido.tipoMaterial), materialEscolhido.objtype, 1);
				DevolveContainerVazio(quem, objtypeConsumido, 1);
			endif
			Sleepms(2);
		endforeach

		Sleepms(2);
	endforeach

	return 1;
endfunction

function SubtraiTotal(byref materiaisEncontrados, tipoMaterial, objtype, quantidade)
		var dictMateriais := materiaisEncontrados[tipoMaterial];
		dictMateriais[objtype].quantidadeTotal -= quantidade;
		materiaisEncontrados[tipoMaterial] := dictMateriais;
endfunction

function PreparaDropDownsComponentes(quem, byref sessao,byref gerenciador, configRecursos)
	var materiaisNecessarios := sessao.componenteSelecionado.selecione.materiais;
	sessao.materiaisEncontrados := CarregaMateriaisEncontrados(
																quem,
																configRecursos,
																sessao.componenteSelecionado);
	
	var materiaisNecessariosInvertidos :=  materiaisNecessarios.keys();
	//materiaisNecessariosInvertidos.reverse();//Foi preciso inverter para ficar de acordo com a ordem na string da tabela.

	foreach materialNecessario in (materiaisNecessariosInvertidos)
		
		var dropDown := GerenciadorDropDown::ObtemDropDown(gerenciador,_materialNecessario_iter);

		var tipo := materiaisNecessariosInvertidos[_materialNecessario_iter];
		var qtd := materiaisNecessarios[materialNecessario].quantidade;
		var indiceOpcoes := 1;
		var materiaisEncontrados := sessao.materiaisEncontrados[Lower(materialNecessario)].keys();

		if(materiaisEncontrados)
			var isValorEscolhidoPresente := false;

			foreach chave in (materiaisEncontrados)
				Sleepms(2); 
				if (dropDown.PegaValorEscolhido().objtype == chave)
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
		endif
		
		Sleepms(2);
	endforeach
endfunction

function MontaValorOpcaoComponente(chave,qtd,tipo,sessao)

	return struct {
					objtype := chave,
					nome := configRecursos[chave].name,
					quantidadeNecessaria := qtd,
					tipoMaterial := Lower(tipo),
					isPrincipal := Lower(tipo) == Trim(Lower(sessao.componenteSelecionado.selecione.materialPrincipal))
					};
endfunction

function CarregaMateriaisEncontrados(quem,configRecursos, componente)
	var retorno := dictionary;
	var materiaisComponente;

	foreach item in (EnumerateItemsInContainer(quem.backpack, ENUMERATE_ROOT_ONLY))
		Sleepms(2);
		var materialDetalhe := configRecursos[item.objtype];
		if(materialDetalhe)
			if(!IsMaterialTrabalhavel(quem, componente, item.objtype)) continue; endif //Só importa skill para o material principal.
			MontaMaterialEncontrado(retorno,Lower(materialDetalhe.type),item);
		endif
	endforeach 

	return retorno;
endfunction

function IsMaterialTrabalhavel(quem, componente, chaveMaterial)

	var materialDetalhe := configRecursos[chaveMaterial];

	var tipoMaterial := Lower(materialDetalhe.type);
	var materialPrincipal := Lower(componente.selecione.materialPrincipal);
	var pericia := PegaPericiaPeloMaterial(materialPrincipal);
	
	if(lower(materialPrincipal) == lower(tipoMaterial))
		return cDbl(materialDetalhe.Skill) <= EffectiveSkill(quem, pericia);
	else
		return tipoMaterial in (componente.selecione.materiais.keys());
	endif

endfunction

function MontaMaterialEncontrado(byref categorias, tipo, item)
	var dictitemsEncontrados;
	tipo := lower(tipo);
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
		categorias[tipo] := dictitemsEncontrados;
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
	sessao.+isFiltroMemoria := false;
	sessao.+picoPericia := CalculaValorPico(quem,pericia);
	sessao.+picoMaximoPericia := sessao.picoPericia;
	sessao.+filtros := struct{ nome := "", encaixe := ""};
	sessao.+encaixesMemorizados := GetObjProperty(quem, PROPNAME_VELHA_CRAFTPROP).encaixesMemorizados;
	sessao.+componentes := MontaVisaoComponentes(configProfissao, false, array);
	var propCraft := MontaCraftProp(0,  GetObjProperty(quem, PROPNAME_VELHA_CRAFTPROP));
	SetObjProperty(quem, PROPNAME_VELHA_CRAFTPROP, propCraft);
	sessao.+deveReciclar := propCraft.deveReciclar;

	var colCfg := dictionary;
		colCfg["nome"] := struct{titulo := "Nome",ordem := 1, largura := 3};
		colCfg["materiais"] := struct{titulo := "Materiais",ordem := 2,largura := 3};
		colCfg["pericia"] := struct{titulo := "Perícia", ordem := 3,largura := 2};
		colCfg["encaixe"] := struct{titulo := "Encaixe",ordem := 4,largura := 3};
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
	elseif(valorRetorno == COMPONENTES_BTN_LIMPAR_MEMORIA)
		return RT_LIMPAR_MEMORIA_COMPNT;
	elseif(valorRetorno == COMPONENTES_BTN_LIGA_DESL_FILTRO_MEMORIA)
		return RT_FILTRO_MEMORIA_LIG_DES;
	elseif(valorRetorno == COMPONENTES_BTN_LIMPA_FILTRO)
		return RT_BTN_LIMPA_FILTRO;
	elseif(valorRetorno == COMPONENTES_CHK_BOX_RECICLAR)
		return RT_CHK_BOX_RECICLAR;
	elseif(valorRetorno == 1)
		return RT_CANCELADO;
	else
		return RT_CANCELADO;
	endif
endfunction

function MontaVisaoComponentes(configProfissao, isFiltroMemoria, encaixesMemorizados , filtroEncaixe := "", filtroNome := "")
	var componentes := array;
	var chavesComponentes := GetConfigStringKeys(configProfissao);
	var encaixesConfig := ReadConfigFile(":crafting:/config/encaixes");


	foreach chave in chavesComponentes
		Sleepms(2);
		if(!configProfissao[chave].encaixe)
			continue;
		endif

		if(isFiltroMemoria && !(configProfissao[chave].encaixe in encaixesMemorizados))
			var isMemorizadoEncontrado := false;
			foreach encaixeMemorizado in encaixesMemorizados
				Sleepms(2);
				isMemorizadoEncontrado :=  encaixesConfig[configProfissao[chave].encaixe].Desc 
											== encaixesConfig[encaixeMemorizado].Desc;

				if(isMemorizadoEncontrado) break; endif
			endforeach
			if(!isMemorizadoEncontrado) continue; endif
		endif

		if(filtroEncaixe != "" && !Find(UPPER(encaixesConfig[configProfissao[chave].encaixe].Desc),UPPER(filtroEncaixe),1))
			continue;
		endif

		if(filtroNome != "" && !Find(UPPER(configProfissao[chave].name),UPPER(filtroNome),1))
			continue;
		endif

		var materiaisNecessarios := GetConfigStringArray( configProfissao[chave], "materials" );
		var materiaisConcatenados := "";
		var dicionarioMateriais := dictionary;
		var materiaisPossiveis := array;

		foreach material in materiaisNecessarios
			if(_material_iter != 1)
				materiaisConcatenados += ", ";
			endif

			var tipoEQuantidade := SplitWords(material);
			dicionarioMateriais.insert(
					Lower(tipoEQuantidade[1]),
					struct { 
						quantidade := tipoEQuantidade[2], 
						possiveis := LISTA_TIPO_MATERIAL[UPPER(tipoEQuantidade[1])] 
					}
				);

			materiaisConcatenados += tipoEQuantidade[2] + " " + tipoEQuantidade[1];
			Sleepms(2);
		endforeach
		
		var quantidadeLotePrefixo := cint(configProfissao[chave].lote) > 1? configProfissao[chave].lote + "x " : "";

		var componente := struct;
		componente.+nome :=  quantidadeLotePrefixo + configProfissao[chave].name;
		componente.+pericia :=  StrReplace(configProfissao[chave].pericia,"_", " ");
		componente.+materiais := materiaisConcatenados;
		componente.+encaixe := encaixesConfig[configProfissao[chave].encaixe].Desc;
		componente.+selecione := struct{
										isColunaBotao := true, 
										objtype := chave, 
										materiais := dicionarioMateriais, 
										materialPrincipal := configProfissao[chave].principalMaterial,
										encaixeReal := configProfissao[chave].encaixe
									};

		componentes.append(componente);
	endforeach

	return componentes;
endfunction
