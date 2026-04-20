use uo;
use os;
use cfgfile;

include "/include/epicConstants";
include "/include/epicPropNames";
include ":gumps:/include/dropDownGump";
include ":crafting:gumps/fabricacaoGumps";
include ":crafting:include/controladoras/controladoraComplexos";


function InicializaSessaoComplexos(byref profissaoCraftProp, modoGm)
	var sessao := struct;

	sessao.+categoriaSelecionada := "";
	sessao.+categoriaAnterior := "";
	NovoObjetoReceitaSelecionada(sessao);
	sessao.+gumpAtual := COMPLEXOS;
	sessao.+paginador := struct { complexos := struct {pagina := 0}, esquemas := struct {pagina := 0}};
	sessao.+acao := ADICIONAR;
	sessao.+quantidade := 1;
	sessao.+deveAtrasarAtualizacao := false;
	sessao.+visualizadas := profissaoCraftProp["visualizadas"];
	sessao.+gerenciadorDropDown := GerenciadorDropDown(100,
										COMPLEXOS_DROPDOWN_INICIAL,
										COMPLEXOS_DROPDOWN_OPCOES_INICIAL,
										COMPLEXOS_DROPDOWN_MAX);
	sessao.+devePopularDropDown := true;

	if(modoGm)
		sessao.+modoGm := NovoObjetoModoGm();
	endif

	return sessao;
endfunction

function EnviaItensComplexosGump(quem,profissao, modoGm)
	var configRecursos := ReadConfigFile(RECURSOS_CONFIG_PATH);
	var parametros := PreparaItensComplexosGump(quem, profissao, modoGm);
	var retorno := true;
	var gump;
	var sessao := InicializaSessaoComplexos(parametros.propCraft.pericias[parametros.pericia], modoGm);
	var gerenciador := sessao.gerenciadorDropDown;
	
	while(retorno)
		Sleepms(2);
		if(sessao.gumpAtual == COMPLEXOS)
			MontaItensComplexosGump(gump,parametros,sessao);
		elseif(sessao.gumpAtual == ESQUEMAS)
			if(sessao.devePopularDropDown)
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				PreparaDropDownsEsquemas(quem,sessao,gerenciador,configRecursos);
				sessao.devePopularDropDown := false;
			endif
			MontaEsquemaGump(gump,sessao, parametros);
		elseif(sessao.gumpAtual == GM_COMPONENTE)
			GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
			PreparaDropDownsEsquemas(quem,sessao,gerenciador,configRecursos);
			MontaEscolhaComponenteModoGmGump(gump, sessao);
		endif

		retorno := GFSendGump(quem,gump);

		if(find(retorno[COMPLEXOS_ESQUEMAS_INPT_NOME],":",1))
			var entradaNome := GFExtractData(retorno,COMPLEXOS_ESQUEMAS_INPT_NOME);
			if(entradaNome)
				sessao.receitaSelecionada.esquemaSelecionado.nome := entradaNome;
			endif
		elseif(find(retorno[COMPLEXOS_INPT_QTD],":",1))
			var quantidade := GFExtractData(retorno,COMPLEXOS_INPT_QTD)[1,3];
			if(quantidade)
				sessao.quantidade := quantidade;
			endif
		endif

		case(ClassificaRetornoComplexos(retorno,sessao))
			RT_CATEGORIAS:
				var retornoMapeado :=  MapeiaRetornoGump(retorno[0],COMPLEXOS_CATEGORIAS);
				sessao.categoriaAnterior := sessao.categoriaSelecionada;
				sessao.categoriaSelecionada := parametros.visaoReceitas.keys()[retornoMapeado.indice];
				sessao.paginador.complexos.pagina := 0;
				MarcaCategoriaVisualizada(sessao,parametros);// marca a categoria anterior como visualizada.
				NovoObjetoReceitaSelecionada(sessao);
				break;

			RT_RECEITAS:
				var retornoMapeado :=  MapeiaRetornoGump(retorno[0],COMPLEXOS_RECEITAS,3);
				case (retornoMapeado.indiceBotao)
					1: //Seleciona receita
						SelecionaReceita(sessao, quem, parametros, retornoMapeado.indiceItem);
						
						break;

					2: // Novo Esquema.
						var objetoReceita := parametros.visaoReceitas[sessao.categoriaSelecionada][retornoMapeado.indiceItem];
						var esquemas := ListaEsquemas(quem,objetoReceita.objtype);
						if(esquemas.size() >= NR_MAXIMO_ESQUEMAS)
							SendSysMessageEX(quem, "Limite de esquemas alcançado para esta receita.",SSM_FAIL);
							break;
						endif

						NovoObjetoReceitaSelecionada(sessao,objetoReceita,esquemas);
						sessao.paginador.esquemas.pagina := 0;
						sessao.gumpAtual := ESQUEMAS;
						sessao.acao := ADICIONAR;
						sessao.devePopularDropDown := true;
						break;

					3: //Fabricar com melhores componentes.
						SelecionaReceita(sessao, quem, parametros, retornoMapeado.indiceItem);
						var melhorEsquema := MontaMelhorEsquema(quem, sessao);

						FabricaItens(quem,melhorEsquema ,sessao.receitaSelecionada.receita, sessao.quantidade, sessao.modoGm, parametros.nomeProfissao, sessao.categoriaSelecionada);
						sessao.receitaSelecionada.esquemaSelecionado := NovoObjetoEsquemaSelecionado();
						break;

				endcase
				break;

			RT_ABRE_GM_COMPONENT:
				var retornoMapeado := MapeiaRetornoGump(retorno[0],COMPLEXOS_ESQUEMAS_ESCOLHA_GM_CPNT);

				var componenteEscolhido := sessao.receitaSelecionada.receita.componentesNecessarios[retornoMapeado.indice];
				var modoGm := NovoObjetoModoGm();
				modoGm.encaixeEscolhido.encaixe := componenteEscolhido.encaixe;
				modoGm.encaixeEscolhido.indice := retornoMapeado.indice;

				sessao.modoGm := modoGm;
				sessao.gumpAtual := GM_COMPONENTE;
				//TODO criar o gump de escolha de componete pra gm
				break;

			RT_GM_COMPONENTE_SELECIONADO:
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				TrataGMEscolhaComponente(quem, sessao);
				sessao.gumpAtual := ESQUEMAS;
				break;

			RT_ESCOLHA_DROPDOWN_CPX:
				var retornoMapeado := MapeiaRetornoGump(retorno[0],gerenciador.PegaAdicionalIdDropDown());
				var objetoDropDown := gerenciador.BuscaDropDownPorPosicao(retornoMapeado.indice);
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				objetoDropDown.SendGump(quem, GetPid());
				break;

			RT_ESCOLHA_DD_OPCOES_CPX: 
				var dropdown := GerenciadorDropDown::DefineValorEscolhido(gerenciador,retorno[0]);
				TratarEscolhaDropDown(quem, dropdown, sessao, configRecursos);
				break;
			
			RT_AJUDA_CPX: 
				Start_Script( ":crafting:gumpAjuda", array{quem, parametros.profissao, FABRICACAO_AJUDA_ABA_MONTAGEM});
				break;

			RT_ESQUEMAS_SALVAR:
				if(sessao.receitaSelecionada.esquemaSelecionado.nome == "")
					SendSysMessageEX(quem,"O nome do esquema é obrigatório.", SSM_FAIL);
					break;
				endif
				var nomeReceitaClassificado := PegaNomeReceitaClassificada(sessao.receitaSelecionada.receita.receitaClassificada);

				case(sessao.acao)
					ADICIONAR:
						var isAdicionado := AdicionaEsquema(quem,
										nomeReceitaClassificado,
										sessao.receitaSelecionada.esquemaSelecionado);
						sessao.receitaSelecionada.esquemas := ListaEsquemas(quem,nomeReceitaClassificado);
						if(!isAdicionado)
							SendSysMessageEX(quem,isAdicionado.errortext,SSM_FAIL);
						endif
						break;

					EDITAR:
						EditaEsquema(quem,
									nomeReceitaClassificado,
									sessao.receitaSelecionada.esquemaSelecionado
									);
						sessao.receitaSelecionada.esquemas := ListaEsquemas(quem,nomeReceitaClassificado);
						sessao.devePopularDropDown := true;
						break;

				endcase

				sessao.receitaSelecionada.esquemaSelecionado := NovoObjetoEsquemaSelecionado();//limpa esquema selecionado.
				sessao.gumpAtual := COMPLEXOS;
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				break;

			RT_ESQUEMAS:
				var retornoMapeado :=  MapeiaRetornoGump(retorno[0],COMPLEXOS_ESQUEMAS,3);
				var chaveEsquema := sessao.receitaSelecionada.esquemas.keys()[retornoMapeado.indiceItem];
				var esquema := sessao.receitaSelecionada.esquemas[chaveEsquema];
				var receitaClassificada := sessao.receitaSelecionada.receita.receitaClassificada;
				esquema.classe := receitaClassificada.classe ? cint(receitaClassificada.classe) : CLASSE_PADRAO;
				var nomeReceitaClassificado := PegaNomeReceitaClassificada(receitaClassificada);

				case (retornoMapeado.indiceBotao)
					1: //Editar
						sessao.receitaSelecionada.esquemaSelecionado := esquema;
						sessao.gumpAtual := ESQUEMAS;
						sessao.acao := EDITAR;
						break;

					2: //Excluir
						sessao.receitaSelecionada.esquemas := RemoveEsquema(quem, nomeReceitaClassificado, esquema);
						break;

					3: //Fabricar
						//TODO Animações e sons.
						FabricaItens(quem, esquema, sessao.receitaSelecionada.receita, sessao.quantidade, sessao.modoGm, parametros.nomeProfissao, sessao.categoriaSelecionada);
						break;

				endcase
				break;

			RT_RCT_PROX_PAGINA:
					sessao.paginador.complexos.pagina += 1;
				break;

			RT_RCT_ANT_PAGINA:
					sessao.paginador.complexos.pagina -= 1;
				break;

			RT_ESQ_PROX_PAGINA:
					sessao.paginador.esquemas.pagina += 1;
				break;

			RT_ESQ_ANT_PAGINA:
					sessao.paginador.esquemas.pagina -= 1;
				break;

			RT_LIMPAR_MEMORIA:
				LimparMemoriaEncaixes(quem);
				SendSysMessageEX(quem,"Os encaixes memorizados foram esquecidos.", SSM_INFO);
				break;

			RT_REPARAR:
				var alvo := 1;
				SendSysMessageEX(quem,"Escolha o item que deseja reparar", SSM_INFO);
				While(alvo)
					alvo := Target(quem);
					sleepms(2);

					if(alvo)
						repair_item(quem, alvo,parametros.pericia);
					endif
				endwhile

				if(!alvo)
					SendSysMessageEX(quem,"Nenhum item selecionado para reparo.", SSM_FAIL);
				endif
				break;

			RT_MEMORIZAR_COMPONENTES:
				var propCraft := GetObjProperty(quem, "Craft");

				if(propCraft && sessao.receitaSelecionada.receita)
					foreach componente in (sessao.receitaSelecionada.receita.componentesNecessarios)
					Sleepms(2);
						if(!(componente in propCraft.encaixesMemorizados)) //Se já não foram memorizados, memorize.
							propCraft.encaixesMemorizados.append(componente.encaixe);
						endif
					endforeach
				
					SendSysMessageEX(quem,"Os encaixes para esta receita foram memorizados.", SSM_INFO);
					SetObjProperty(quem, "Craft", propCraft);
				else
					SendSysMessageEX(quem,"É preciso uma receita selecionada para memorizar componentes.", SSM_FAIL);
				endif
				break;

			RT_CANCELADO_ESQUEMA_COMPLX:
				sessao.gumpAtual := COMPLEXOS;
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				retorno := 1;
				break;
			
			RT_CANCELADO_GUMP_GM_COMPONENTE:
				sessao.gumpAtual := ESQUEMAS;
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				retorno := 1;
				break;

			RT_GM_ESQUEMA_FABRICAR:
				FabricaItens(
								quem,
								sessao.receitaSelecionada.esquemaSelecionado,
								sessao.receitaSelecionada.receita,
								sessao.quantidade,
								sessao.modoGm,
								parametros.nomeProfissao,
								sessao.categoriaSelecionada
							);
				sessao.gumpAtual := COMPLEXOS;
				sessao.receitaSelecionada.esquemaSelecionado := NovoObjetoEsquemaSelecionado();
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				break;


			RT_CANCELADO_COMPLX:
				sessao.categoriaAnterior := sessao.categoriaSelecionada;
				MarcaCategoriaVisualizada(sessao,parametros);

				var propCraft := GetObjProperty(quem, "Craft");
				propCraft.pericias[parametros.pericia]["visualizadas"] := sessao.visualizadas;
				SetObjProperty(quem, "Craft", propCraft);
				GerenciadorDropDown::FechaTodosDropdowns(gerenciador,quem);
				retorno := 0;
				break;
		endcase
	endwhile
endfunction

function CarregaDropDownGmMode(byref sessao,byref gerenciador)
	var configEncaixes := ReadConfigFile(":*:encaixes");
	var configComponentes := ReadConfigFile(":*:componentes");

	var quantidadeEscolhasParaAcharComponente := 3;
	
	for indice := 1 to quantidadeEscolhasParaAcharComponente

		var dropDown := GerenciadorDropDown::ObtemDropDown(gerenciador,indice);

		case(indice)
			1: //componentes
				var elementoEcaixe := configEncaixes[sessao.modoGm.encaixeEscolhido.encaixe];
				var componentes := GetConfigIntArray(elementoEcaixe,"Item");

				foreach item in componentes
					Sleepms(2);
					var infoComponente := configComponentes[item];
					
					DropDown::AdicionaOpcao(
												dropDown,
												infoComponente.name,
												_item_iter, 
												struct{ 
													objtype := item ,
													name := infoComponente.name,
													principalMaterial := infoComponente.principalMaterial
												}
											);
				endforeach
				break;
			2: //qualidade
				var tiers := PegaComponenteTiers();
				foreach tier in tiers
					Sleepms(2);
					var tierInfo := PegaComponenteTierDesc(tier);
					DropDown::AdicionaOpcao(dropDown,tierInfo.desc,_tier_iter,tierInfo);
				endforeach
				break;
			3: //material
				var componenteEscolhido := sessao.modoGm.componenteEscolhido;
				var tipo := componenteEscolhido.principalMaterial;

				var materiais := LISTA_TIPO_MATERIAL[UPPER(tipo)];
				foreach material in materiais
					DropDown::AdicionaOpcao(dropDown,material.name,_material_iter,material);
				endforeach
				break;
		endcase
	endfor
endfunction

function CarregaDropDownNormal(quem, byref sessao,byref gerenciador, configRecursos)
	var componentesEncontrados := CarregaComponentesEncontrados(quem, sessao.receitaSelecionada.receita, configRecursos);
	
	foreach componenteNecessario in (sessao.receitaSelecionada.receita.componentesNecessarios)
		var dropDown := GerenciadorDropDown::ObtemDropDown(gerenciador,_componenteNecessario_iter, LARGURA_DROPDOWN);

		if(componentesEncontrados[componenteNecessario.encaixe])
			var indiceOpcoes := 1;
			var envelopesEncontrados := componentesEncontrados[componenteNecessario.encaixe].conjunto;
			foreach chaveEnvelope in (envelopesEncontrados.keys())
				Sleepms(2);
				var quantidadeFormatada :=  envelopesEncontrados[chaveEnvelope].quantidade <= 99 ? 
												cstr(envelopesEncontrados[chaveEnvelope].quantidade) :
												cstr(envelopesEncontrados[chaveEnvelope].quantidade)[1,2] + "..";
				var nomeOpcao := "{} {} [{}] [{}]".format(
														quantidadeFormatada,
														envelopesEncontrados[chaveEnvelope].itemRef.name,
														envelopesEncontrados[chaveEnvelope].material.name,
														envelopesEncontrados[chaveEnvelope].tier.desc);
				
				var retorno := DropDown::AdicionaOpcao(
														dropDown,
														nomeOpcao,
														indiceOpcoes,
														MontaObetoOpcaoComponente(envelopesEncontrados[chaveEnvelope], 
														_componenteNecessario_iter)
													);

				if(!retorno) 
					continue;
				endif;
				indiceOpcoes := indiceOpcoes + 1;
			endforeach
		endif
		
		Sleepms(2);
	endforeach
endfunction

function PreparaDropDownsEsquemas(quem, byref sessao,byref gerenciador, configRecursos)
	if(sessao.modoGm)
		CarregaDropDownGmMode(sessao, gerenciador);
	else
		CarregaDropDownNormal(quem, sessao, gerenciador, configRecursos);
	endif
endfunction


function PreparaItensComplexosGump(quem, profissao, modoGm)

	var parametrosGump := struct;
	var profissaoConfig := ReadConfigFile(PROFISSAO_CONFIG_PATH);
	var menuFile    := profissaoConfig[profissao].MenuFile;
	var menuPericiaCfg := ReadConfigFile(menuFile);

	var propCraft := MontaCraftProp(profissaoConfig,  GetObjProperty(quem, PROPNAME_VELHA_CRAFTPROP));
	SetObjProperty(quem, PROPNAME_VELHA_CRAFTPROP, propCraft);

	var categorias := GetConfigStringArray(menuPericiaCfg["Index"], "Group");
	parametrosGump.+receitasPorPagina := NR_MAXIMO_RECEITAS_POR_PAGINA;
	parametrosGump.+encaixesPorPagina := NR_MAXIMO_ENCAIXES_POR_PAGINA;
	parametrosGump.+profissao := profissaoConfig[profissao].MenuFile;
	parametrosGump.+propCraft := propCraft;
	parametrosGump.+receitasConfig := ReadConfigFile(profissaoConfig[profissao].InfoFabricacao);
	parametrosGump.+gumpIdProfissaoPressed := profissaoConfig[profissao].GumpicPressionado 
													? cint(profissaoConfig[profissao].GumpicPressionado )
													: 0;
	parametrosGump.+gumpIdProfissaoUnpressed := profissaoConfig[profissao].GumpicNaoPressionado 
													? cint(profissaoConfig[profissao].GumpicNaoPressionado)
													: 0;
	parametrosGump.+pericia := profissaoConfig[profissao].Attribute;
	parametrosGump.+nomeProfissao := profissao;
	//TODO Por questão de performace, é melhor carregar as receitas de forma "Lazy" ao clicar na categoria 
	//e não aqui carregando tudo no inicio, alterar isso no futuro. ass: deiro
	parametrosGump.+visaoReceitas := MontaVisaoReceitas(
											quem,
											categorias,
											menuPericiaCfg,
											parametrosGump.pericia,
											parametrosGump.receitasConfig, 
											parametrosGump.propCraft,
											profissao,
											modoGm);
	
	return parametrosGump;
endfunction

function ClassificaRetornoComplexos(retorno,sessao)
	var valorRetorno := retorno[0];
	//FAIXAS
	if(valorRetorno > COMPLEXOS_CATEGORIAS && valorRetorno <= COMPLEXOS_RECEITAS)
		return RT_CATEGORIAS;
	elseif(valorRetorno > COMPLEXOS_RECEITAS && valorRetorno <= COMPLEXOS_ESQUEMAS)
		return RT_RECEITAS;
	elseif(valorRetorno > COMPLEXOS_ESQUEMAS && valorRetorno <= COMPLEXOS_ESQUEMAS_ESCOLHA_GM_CPNT)
		return RT_ESQUEMAS;
	elseif(valorRetorno > COMPLEXOS_ESQUEMAS_ESCOLHA_GM_CPNT && valorRetorno <= COMPLEXOS_DROPDOWN_INICIAL)
		return RT_ABRE_GM_COMPONENT;
	elseif(GerenciadorDropDown::IsIdDentroDropDowns(sessao.gerenciadorDropDown,valorRetorno))
		return RT_ESCOLHA_DROPDOWN_CPX;
	elseif(GerenciadorDropDown::IsIdDentroOpcoes(sessao.gerenciadorDropDown,valorRetorno))
		return RT_ESCOLHA_DD_OPCOES_CPX;
	
	//BOTOES
	elseif(valorRetorno == COMPLEXOS_ESQUEMAS_BTN_SALVAR)
		return RT_ESQUEMAS_SALVAR;
	elseif(valorRetorno == COMPLEXOS_BTN_AJUDA)
		return RT_AJUDA_CPX;
	elseif(valorRetorno == COMPLEXOS_BTN_RCT_PROX_PAGINA)
		return RT_RCT_PROX_PAGINA;
	elseif(valorRetorno == COMPLEXOS_BTN_RCT_ANT_PAGINA)
		return RT_RCT_ANT_PAGINA;
	elseif(valorRetorno == COMPLEXOS_BTN_ESQ_PROX_PAGINA)
		return RT_ESQ_PROX_PAGINA;
	elseif(valorRetorno == COMPLEXOS_BTN_ESQ_ANT_PAGINA)
		return RT_ESQ_ANT_PAGINA;
	elseif(valorRetorno == COMPLEXOS_BTN_MEMORIZAR_COMPONENTES)
		return RT_MEMORIZAR_COMPONENTES;
	elseif(valorRetorno == COMPLEXOS_BTN_LIMPAR_MEMORIA)
		return RT_LIMPAR_MEMORIA;
	elseif(valorRetorno == COMPLEXOS_BTN_OKAY_GM_CMPNT)
		return RT_GM_COMPONENTE_SELECIONADO;
	elseif(valorRetorno == COMPLEXOS_BTN_FABRICAR_GM_CMPNT)
		return RT_GM_ESQUEMA_FABRICAR;
	elseif(valorRetorno == COMPLEXOS_BTN_REPARAR)
		return RT_REPARAR;

	//CANCELAMENTOS
	elseif((valorRetorno == 1 || valorRetorno == 0) && sessao.gumpAtual == ESQUEMAS)
		return RT_CANCELADO_ESQUEMA_COMPLX;
	elseif((valorRetorno == 1 || valorRetorno == 0) && sessao.gumpAtual == GM_COMPONENTE)
		return RT_CANCELADO_GUMP_GM_COMPONENTE;
	elseif(valorRetorno == COMPLEXOS_BTN_CANCELAR)
		return RT_CANCELADO_COMPLX;
	elseif(valorRetorno == 1)
		return RT_CANCELADO_COMPLX;
	endif
endfunction