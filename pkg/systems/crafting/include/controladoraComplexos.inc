include "/include/epicConstants";
include "/include/epicPropNames";
include "/include/math";
include "include/epicUtils";
include ":crafting:craftingUtil";
include ":crafting:crafting";
include ":megacliloc:/include/descricaoObjetoTTip";
include ":crafting:gumps/fabricacaoGumps";
include ":gumps:/include/dropDownGump";
include ":crafting:/craftingDFEsquemas";
include ":crafting:/include/fabricacaoUtil";
include "include/epicValidAction";

const NR_MAXIMO_ESQUEMAS := 6;
const OBJTYPE_ITEM_MONTADO := 0xe72;
const FIXO_DIF_ADICIONAL_PERICIA := 0; //Adicional no limite mais baixo da dificuldade, o resultado de um somatório faz aparecer no gump.

function InicializaSessaoComplexos(quem, profissaoCraftProp)
	var sessao := struct;

	sessao.+categoriaSelecionada := "";
	NovoObjetoReceitaSelecionada(sessao);
	sessao.+gumpAtual := COMPLEXOS;
	sessao.+acao := ADICIONAR;
	sessao.+quantidade := 1;
	sessao.+deveAtrasarAtualizacao := false;
	sessao.+visualizadas := profissaoCraftProp["visualizadas"];
	sessao.+gerenciadorDropDown := GerenciadorDropDown(60,
										COMPLEXOS_DROPDOWN_INICIAL,
										COMPLEXOS_DROPDOWN_OPCOES_INICIAL,
										COMPLEXOS_DROPDOWN_MAX);
	sessao.+devePopularDropDown := true;

	return sessao;
endfunction


function NovoObjetoReceitaSelecionada(byref sessao, objetoReceita := "", dictEsquemas := dictionary)
	if(!sessao.?receitaSelecionada)
		sessao.+receitaSelecionada := struct;
	else
		sessao.receitaSelecionada := struct;
	endif
	
	sessao.receitaSelecionada.+receita := objetoReceita;
	sessao.receitaSelecionada.+esquemas := dictEsquemas;
	sessao.receitaSelecionada.+esquemaSelecionado := NovoObjetoEsquemaSelecionado();
endfunction

function NovoObjetoEsquemaSelecionado(nomeEsquema := "", dictComponentesEscolhidos := dictionary, corItem := 0, material := "")
	return struct{
				nome := nomeEsquema,
				cor := corItem,
				material := material,
				isCompletamenteComum := 1,
				modificadorHpTotal := 0,
				modificadorPesoTotal := 0,
				qualidade := 0,
				modificadorCache := struct{ peso := dictionary, hp := dictionary, qualidade := dictionary},
				componentesEscolhidos := dictComponentesEscolhidos
			};
endfunction

function SelecionaReceita(byref sessao, quem, parametros , indice)
	var objetoReceita := parametros.visaoReceitas[sessao.categoriaSelecionada][indice];
	var nomeReceitaClassificado := PegaNomeReceitaClassificada(objetoReceita.receitaClassificada);
	NovoObjetoReceitaSelecionada(sessao,objetoReceita,ListaEsquemas(quem,nomeReceitaClassificado));
endfunction

function EnviaItensComplexosGump(quem,profissao)
	var configRecursos := ReadConfigFile(RECURSOS_CONFIG_PATH);
	var parametros := PreparaItensComplexosGump(quem, profissao);
	var retorno := true;
	var gump;
	var sessao := InicializaSessaoComplexos(quem, parametros.propCraft[parametros.pericia]);
	var gerenciador := sessao.gerenciadorDropDown;
	
	//TODO Fazer opção para reparar
	//TODO criar botão para sair
	//TODO Criar fluxo para itens stackaveis. [Já fiz,mas tem que testar.]
	//TODO implementar componentes opcionais [já fiz os opcionais, tem que testar.] 
	//*** POS MERGE DEVELOP***
	//TODO Solucionar o problema das armaduras estarem sem informações.
	//TODO Criar gumps de ajuda.
	//TODO Criar API para gerar item fabricado.
	//TODO Criar adicionais de dificuldade para estações de tipos diferentes na fabricação.
	//TODO Criar e atribuir componentes a todos os itens.
	//TODO aplicar efeitos do componente em suas respectivas ações.
	//TODO Animações para todas as profissões. [Late]
	//TODO Marcar sistema antigo como depreciado. [Late]
	//TODO Embelezar gumps [Penultima fase]
	//TODO Documentar tudo [Ultima fase]
	while(retorno)
		Sleepms(2);
		if(sessao.gumpAtual == COMPLEXOS)
			MontaItensComplexosGump(gump,parametros,sessao);
		else
			if(sessao.devePopularDropDown)
				FechaTodosDropdowns(quem,gerenciador.dropDowns);
				PreparaDropDownsEsquemas(quem,sessao,gerenciador,configRecursos);
				sessao.devePopularDropDown := false;
			endif
			MontaEsquemaGump(gump,sessao);
		endif

		retorno := GFSendGump(quem,gump);

		if(sessao.deveAtrasarAtualizacao)//booleano para atrasar atualizacao de alguma parte do gump por 1 ciclo. [visualizados, por exemplo]
			sessao.deveAtrasarAtualizacao := false;
		endif

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
				sessao.categoriaSelecionada := parametros.visaoReceitas.keys()[retornoMapeado.indice];
				MarcaCategoriaVisualizada(sessao,parametros);
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
						sessao.gumpAtual := ESQUEMAS;
						sessao.acao := ADICIONAR;
						sessao.devePopularDropDown := true;
						break;

					3: //Fabricar com melhores componentes.
						SelecionaReceita(sessao, quem, parametros, retornoMapeado.indiceItem);
						var melhorEsquema := MontaMelhorEsquema(quem, sessao);

						FabricaItens(quem,melhorEsquema ,sessao.receitaSelecionada.receita, sessao.quantidade);
						sessao.receitaSelecionada.esquemaSelecionado := NovoObjetoEsquemaSelecionado();
						break;

				endcase
				break;

			RT_ESCOLHA_DROPDOWN_CPX:
				var retornoMapeado := MapeiaRetornoGump(retorno[0],gerenciador.adicionalIdDropDown);
				var objetoDropDown := gerenciador.dropDowns[retornoMapeado.indice];

				FechaTodosDropdowns(quem,gerenciador.dropDowns);
				var process := SendDropDownGump(quem, GetPid(),objetoDropDown);
				
				if(process)
					objetoDropDown.pid := process.pid;
				endif
				break;

			RT_ESCOLHA_DD_OPCOES_CPX: 
				var dropdown := GerenciadorDropDown::DefineValorEscolhido(gerenciador,retorno[0]);
				TratarEscolhaComponente(quem, dropdown.valorEscolhido, sessao, configRecursos);
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
				FechaTodosDropdowns(quem,gerenciador.dropDowns);
				break;

			RT_ESQUEMAS:
				var retornoMapeado :=  MapeiaRetornoGump(retorno[0],COMPLEXOS_ESQUEMAS,3);
				var chaveEsquema := sessao.receitaSelecionada.esquemas.keys()[retornoMapeado.indiceItem];
				var esquema := sessao.receitaSelecionada.esquemas[chaveEsquema];
				var nomeReceitaClassificado := PegaNomeReceitaClassificada(sessao.receitaSelecionada.receita.receitaClassificada);

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
						FabricaItens(quem, esquema, sessao.receitaSelecionada.receita, sessao.quantidade);
						break;

				endcase
				break;

			RT_CANCELADO_ESQUEMA_COMPLX:
				sessao.gumpAtual := COMPLEXOS;
				FechaTodosDropdowns(quem,gerenciador.dropDowns);
				retorno := 1;
				break;

			RT_CANCELADO_COMPLX:
				var propCraft := GetObjProperty(quem, "Craft");
				propCraft[parametros.pericia]["visualizadas"] := sessao.visualizadas;
				SetObjProperty(quem, "Craft", propCraft);
				FechaTodosDropdowns(quem,gerenciador.dropDowns);
				retorno := 0;
				break;
		endcase
	endwhile
endfunction

function MarcaCategoriaVisualizada(byref sessao,parametros)
	
	if(!(sessao.categoriaSelecionada in (sessao.visualizadas.categorias)))
		sessao.visualizadas.categorias.append(sessao.categoriaSelecionada);
		if(!sessao.deveAtrasarAtualizacao)
			foreach receita in (parametros.visaoReceitas[sessao.categoriaSelecionada])
				Sleepms(2);
				var nomeClassificado := PegaNomeReceitaClassificada(receita.receitaClassificada);
				if(!(nomeClassificado in sessao.visualizadas.receitas))
					sessao.visualizadas.receitas.append(nomeClassificado); //guardo as que foram visualizadas para atualizar a prop no final;
				endif
			endforeach
		endif
		sessao.deveAtrasarAtualizacao := true;
	endif
endfunction

function MontaMelhorEsquema(quem, sessao)
	var componentesEncontrados := CarregaComponentesEncontrados(quem, sessao.receitaSelecionada.receita, configRecursos);

	foreach componenteEncontrado in componentesEncontrados
		Sleepms(2);
		var componenteNecessario := sessao.receitaSelecionada.receita.componentesNecessarios[componenteEncontrado.indiceEncaixe];
		var quantidadeComponentesNecessaria :=  sessao.receitaSelecionada.receita.componentesNecessarios.size();
		var opcaoEncontrada := MontaObetoOpcaoComponente(componenteEncontrado.melhor, componenteEncontrado.indiceEncaixe);
		var componenteEscolhido := MontaComponenteEscolhido(opcaoEncontrada, componenteNecessario, configRecursos, quantidadeComponentesNecessaria);
		AcrescentaComponenteEscolhido(sessao.receitaSelecionada.esquemaSelecionado,componenteNecessario,componenteEscolhido, opcaoEncontrada.indiceEncaixeComponente);
	endforeach

	return sessao.receitaSelecionada.esquemaSelecionado;
endfunction

function FabricaItens(quem, esquema, receita, quantidade := 1)
	var posicaoInicial := CriaObjetoPosicao(quem);
	var vidaInicial := AP_GetVital(quem, "Hits");

	if(receita.isEmpilhavel)
		Sleep(1);
		var retorno := FabricaItemMontado(quem,esquema, receita, quantidade);
		if(!retorno)
			SendSysMessageEX(quem, "Você parou de fabricar produtos.", SSM_FAIL);
		endif
	else
		for indice := 1 to Cint(quantidade)
			Sleep(1);
			if(!CheckCanWork(quem,vidaInicial, posicaoInicial))
				break;
			endif
			var retorno := FabricaItemMontado(quem,esquema, receita);
			if(!retorno)
				SendSysMessageEX(quem, "Você parou de fabricar produtos.", SSM_FAIL);
				break;
			endif
		endfor
	endif
endfunction

function FabricaItemMontado(quem,esquema, receita, quantidade := 1)
	var contagem := ContaQuantidadeRequisitos(quem,esquema, receita);
	
	if(isRequisitoSuficiente(quem,contagem, receita, quantidade))
		ConsomeRequisitos(contagem, quantidade);
		CriaItemMontado(quem,esquema,receita, quantidade);
	else 
		return 0;
	endif

	return 1;
endfunction

function hasMovido(quem,posicaoInicial)
	return quem.x != posicaoInicial.x || quem.y != posicaoInicial.y || quem.z != posicaoInicial.z || quem.realm != posicaoInicial.realm;
endfunction

function CriaItemMontado(quem,esquema,receita, quantidade)
	var definicaoItem := itemsConfig[receita.objtype];
	var definicaoTileItem := IU_GetTileInfo( Hex(receita.graphic));

	var pesoFinal := 1;

	var modificador := esquema.modificadorPesoTotal ? pesoFinal + ((esquema.modificadorPesoTotal / 100) + 1) : pesoFinal;
	if(definicaoItem.weight)
		pesoFinal := pesoFinal * (definicaoItem.weight * modificador);
	else
		pesoFinal := pesoFinal * (definicaoTileItem.weight * modificador);
	endif

	var descriptor := GetItemDescriptor(CInt(OBJTYPE_ITEM_MONTADO));
	descriptor.color := esquema.cor;
	descriptor.graphic := receita.graphic;
	descriptor.CProps.insert(PROPNAME_FABRICADO, DefinePropFabricado(esquema,receita));
	descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG, true);

	var item := CreateItemInBackpack(quem,descriptor, cint(quantidade));

	if(item)
		item.weight_multiplier_mod := pesoFinal;
		SendSysMessageEX(quem, "{} criado com sucesso.".format(item.desc), SSM_INFO);
	else
		SendSysMessageEX(quem, "falha ao criar item.".format(item.desc), SSM_FAIL);
	endif

endfunction

function DefinePropFabricado(esquema,receita)
	//Limpa propriedades desnecessárias ou rendundantes.
	receita.-componentesNecessarios;
	receita.-materiaisNecessarios;
	receita.-textoDescItem;
	receita.-dificuldade;
	receita.-isVisualizada;
	receita.-uso;

	esquema.-modificadorCache;

	var propFabricado := struct{ 
		estado := ITEM_MONTADO,
		receita := receita,
		esquema := esquema};

	return propFabricado;
endfunction

function isRequisitoSuficiente(quem, contagem, receita, quantidade := 1)

	var encaixesEncontrados := array;
	foreach elemento in  contagem
		Sleepms(2);
		if(elemento.quantidadeEncontrada < elemento.quantidadeNecessaria * quantidade * elemento.lote )
			var mensagem; 
			if (elemento.?encaixe)
				mensagem := "Você não tem {} [{}] suficiente.".format(elemento.nomeItem,elemento.tierDesc);
			else
				mensagem := "Você não tem {} suficiente.".format(elemento.nomeItem);
			endif
			SendSysMessageEX(quem,mensagem,SSM_FAIL);
			return false;
		endif

		if(elemento.?encaixe)
			encaixesEncontrados.append(elemento.encaixe);
		endif
	endforeach

	foreach necessario in (receita.componentesNecessarios)
		Sleepms(2);
		if(!(necessario.encaixe in encaixesEncontrados) && !necessario.isOpcional)
			SendSysMessageEX(quem,"O encaixe {} é obrigatório e não foi preenchido.".format(necessario.encaixe),SSM_FAIL);
			return false;
		endif
	endforeach

	if(IsMateriaisNaoEncontrados(receita, contagem))
		SendSysMessageEX(quem,"Você não tem todos materiais necessários.",SSM_FAIL);
		return false;
	endif

	return true;
endfunction

function IsMateriaisNaoEncontrados(receita, contagem)
	return receita.materiaisNecessarios.size() - receita.componentesNecessarios.size() > contagem.size();
endfunction

function ContaQuantidadeRequisitos(quem, esquema, receita)
	var contagemFinal := dictionary;

	foreach item in EnumerateItemsInContainer(quem.backpack)
		Sleepms(2);
		foreach componenteEscolhido in (esquema.componentesEscolhidos)
			Sleepms(2);
			
			if(isItemEscolhido(item,componenteEscolhido))
				var necessario := componenteEscolhido.componenteNecessario;
				RegistraInformacoesContagem(contagemFinal,
											item,
											necessario.qtdNecessaria,
											receita.lote,
											necessario.encaixe,
											componenteEscolhido.tier.desc
											);

			endif
		endforeach

		foreach ingrediente in (receita.materiaisNecessarios)
			Sleepms(2);
			var ingredienteInfo := SplitWords(ingrediente);

			if(isMaterialNecessario(item, ingredienteInfo[1]))
				var qtdNecessaria := cint(ingredienteInfo[2]);
				RegistraInformacoesContagem(contagemFinal,
											item,
											qtdNecessaria,
											receita.lote
											);
			endif
		endforeach

	endforeach 
	return contagemFinal;
endfunction

function ConsomeRequisitos(contagem, quantidade := 1)
	foreach contado in contagem
		Sleepms(2);
		var quantidadeRestante := contado.quantidadeNecessaria * contado.lote * quantidade;
		foreach itemEncontrado in (contado.itemRefArray)
			Sleepms(2);
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
		endforeach	
	endforeach
endfunction

function RegistraInformacoesContagem(byref catalogoContagem ,byref item , quantidadeNecessaria, lote, encaixe := 0, tierDesc := "")

	if(!catalogoContagem.exists(item.objtype))
		var informacoesContagem := struct;
		informacoesContagem.+quantidadeEncontrada := 0;
		informacoesContagem.+itemRefArray := array;
		informacoesContagem.+nomeItem := item.name;
		informacoesContagem.+tierDesc := tierDesc;
		informacoesContagem.+lote := lote;
		informacoesContagem.+quantidadeNecessaria := cint(quantidadeNecessaria);
		if(encaixe)
			informacoesContagem.+encaixe := encaixe;
		endif
		catalogoContagem.insert(item.objtype, informacoesContagem);
	endif

	catalogoContagem[item.objtype].quantidadeEncontrada += item.amount;
	catalogoContagem[item.objtype].itemRefArray.append(item);
endfunction

function isMaterialNecessario (item, objtypeIngrediente)
	return cint(item.objtype) == cint(objtypeIngrediente);
endfunction

function isItemEscolhido(item,componenteEscolhido)
	return cstr(componenteEscolhido.item.objtype) == cstr(item.objtype) &&
		 cstr(componenteEscolhido.material.chave) == cstr(Hex(GetObjProperty(item, PROPNAME_MATERIAL))) &&
		 cint(componenteEscolhido.tier.valor) ==  PegaComponenteTierDesc(GetObjProperty(item,PROPNAME_TIER_COMPONENTE)).valor;
endfunction

function MapeiaRetornoGump (retorno, indiceInicial, quantidadeBotoes := 0)
	var objetoMapeado :=struct;
	objetoMapeado.+indice := retorno - indiceInicial;
	if(quantidadeBotoes >= 1)
		objetoMapeado.+indiceBotao := objetoMapeado.indice  % quantidadeBotoes;
		objetoMapeado.indiceBotao := objetoMapeado.indiceBotao == 0? quantidadeBotoes : objetoMapeado.indiceBotao;
		objetoMapeado.+indiceItem :=  (objetoMapeado.indice - objetoMapeado.indiceBotao) / quantidadeBotoes + 1;
	endif

	return objetoMapeado;
	
endfunction

function TratarEscolhaComponente(quem, retornoDropDownOpcao,byref sessao, configRecursos)

	var componenteNecessario := sessao.
									receitaSelecionada.
									receita.
									componentesNecessarios[retornoDropDownOpcao.indiceEncaixeComponente];
	
	if(IsAlvoValido(quem,retornoDropDownOpcao.itemRef))
		var quantidadeComponentesNecessaria := sessao.receitaSelecionada.receita.componentesNecessarios.size();
		var componenteEscolhido := MontaComponenteEscolhido(retornoDropDownOpcao, componenteNecessario, configRecursos, quantidadeComponentesNecessaria);
		AcrescentaComponenteEscolhido(sessao.receitaSelecionada.esquemaSelecionado,componenteNecessario,componenteEscolhido, retornoDropDownOpcao.indiceEncaixeComponente);
	endif
endfunction

function AcrescentaComponenteEscolhido (byref esquemaSelecionado, componenteNecessario, componenteEscolhido, indiceEncaixe)
	if(componenteNecessario.isPrincipal)
			esquemaSelecionado.cor := configRecursos[componenteEscolhido.material.chave].color;
			esquemaSelecionado.material := componenteEscolhido.material.chave;
	endif

	DefineTagQualidade(
				esquemaSelecionado,
				esquemaSelecionado.modificadorCache.qualidade,
				indiceEncaixe,
				componenteEscolhido.Tier);

	AtualizaModificadorTotal(
				componenteEscolhido.propriedades.modificadorHp,
				esquemaSelecionado.modificadorHpTotal,
				esquemaSelecionado.modificadorCache.HP,
				indiceEncaixe
				);

	AtualizaModificadorTotal(
				componenteEscolhido.propriedades.modificadorPeso,
				esquemaSelecionado.modificadorPesoTotal,
				esquemaSelecionado.modificadorCache.Peso,
				indiceEncaixe
				);

	if(esquemaSelecionado.componentesEscolhidos.exists(indiceEncaixe))
		esquemaSelecionado.componentesEscolhidos[indiceEncaixe] := componenteEscolhido;
	else
		esquemaSelecionado.componentesEscolhidos.insert(indiceEncaixe, componenteEscolhido);
	endif

	return esquemaSelecionado;
	
endfunction

function DefineTagQualidade(byref esquemaSelecionado, byref cache, indice, tier)
	if(esquemaSelecionado.componenteEscolhido.poderAvaliado.poderContado != 0)
		esquemaSelecionado.isCompletamenteComum := 0; // lida com a flag para ignorar desc na tooltip se tiver só componente comum
		
		var qualidadeAntiga;
		var hasAntigo := cache.exists(indice);
		if(hasAntigo)
			qualidadeAntiga := cache[indice];
		endif

		if(tier.valor < 0)
			esquemaSelecionado.qualidade := -1;
			!hasAntigo ? cache.insert(indice,esquemaSelecionado.qualidade) : cache[indice] := esquemaSelecionado.qualidade;
		else
			if(tier.valor == 3  && qualidadeAntiga != 1) // Se não tem nenhum componente mediocre, mas tem algum exepcional. considere o item excepcional.
				esquemaSelecionado.qualidade := 1;
				!hasAntigo ? cache.insert(indice,esquemaSelecionado.qualidade) : cache[indice] := esquemaSelecionado.qualidade;
			else
				if(tier.valor < 3  && qualidadeAntiga == 1)
					esquemaSelecionado.qualidade := 0;
					!hasAntigo ? cache.insert(indice,esquemaSelecionado.qualidade) : cache[indice] := esquemaSelecionado.qualidade;
				endif
			endif
		endif
	endif
endfunction

function AtualizaModificadorTotal(byref modificador,byref modificadorTotal,byref cache, indice)
	if(modificador != 0)
		var modificadorAntigo;
		var hasAntigo := cache.exists(indice);
		if(hasAntigo)
			modificadorTotal -= cache[indice];
			cache[indice] := modificador;
		else
			cache.insert(indice,modificador);
		endif
		modificadorTotal += modificador;
	endif
endfunction

function MontaComponenteEscolhido(componenteEncontrado, componenteNecessario, configRecursos, quantidadeComponentesNecessaria)
	var componenteEscolhido := struct;
		var itemNaBag := componenteEncontrado.itemRef;

		//Não se pode trabalhar com REFs aqui porque não é possivel guarda-las no DataFile.
		componenteEscolhido.+componenteNecessario := componenteNecessario;
		componenteEscolhido.+item := struct{nome := itemNaBag.name, objtype := itemNaBag.objtype};
		componenteEscolhido.+material := struct{nome := componenteEncontrado.nomeMaterial, chave := hex(componenteEncontrado.chaveMaterial)};
		componenteEscolhido.+tier := componenteEncontrado.componenteTier;
		componenteEscolhido.+propriedades := MontaPropriedadesComponente(
														componenteEscolhido.material.chave,
														componenteEscolhido.componenteNecessario.caracteristica,
														componenteEscolhido.tier,
														quantidadeComponentesNecessaria,
														configRecursos
														);
		componenteEscolhido.+poderAvaliado := AvaliaPoderComponente(
														componenteEscolhido.tier,
														componenteEscolhido.material.chave,
														componenteEscolhido.componenteNecessario.caracteristica,
														configRecursos,
														componenteEscolhido.propriedades
														);

	return componenteEscolhido;
endfunction

function PreparaDropDownsEsquemas(quem, byref sessao,byref gerenciador, configRecursos)
	var componentesEncontrados := CarregaComponentesEncontrados(quem, sessao.receitaSelecionada.receita, configRecursos);
	
	foreach componenteNecessario in (sessao.receitaSelecionada.receita.componentesNecessarios)
		var dropDown := GerenciadorDropDown::EncontraDropDown(gerenciador,_componenteNecessario_iter);
		
		if(!dropDown) 
			dropDown := GerenciadorDropDown::RegistraDropDown(gerenciador,_componenteNecessario_iter);
		else
			DropDown::ResetaValores(dropDown);
		endif

		if(componentesEncontrados[componenteNecessario.encaixe])
			var isValorEscolhidoPresente := false;
			var indiceOpcoes := 1;

			foreach componenteEncontrado in (componentesEncontrados[componenteNecessario.encaixe].conjunto)
				Sleepms(2);
				if (dropDown.valorEscolhido.itemRef.objtype == componenteEncontrado.itemRef)
					isValorEscolhidoPresente := true;
				endif
				var quantidadeFormatada :=  componenteEncontrado.quantidade <= 99 ? 
												cstr(componenteEncontrado.quantidade) :
												cstr(componenteEncontrado.quantidade)[1,2] + "..";
				dropDown.largura := 350;
				var nomeOpcao := "{} {} [{}] [{}]".format(
														quantidadeFormatada,
														componenteEncontrado.itemRef.name,
														componenteEncontrado.material.name,
														componenteEncontrado.tier.desc);
				
				var retorno := DropDown::AdicionaOpcao(dropDown,
				nomeOpcao,
				indiceOpcoes,
				MontaObetoOpcaoComponente(componenteEncontrado, _componenteNecessario_iter));
				
				if(!retorno) 
					continue;
				endif;
				indiceOpcoes := indiceOpcoes + 1;
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

function MontaObetoOpcaoComponente(componenteEncontrado, indiceEncaixe)
	return struct{ 
				itemRef := componenteEncontrado.itemRef,
				componenteTier := componenteEncontrado.tier,
				nomeMaterial := componenteEncontrado.material.name,
				chaveMaterial := componenteEncontrado.chaveMaterial,
				indiceEncaixeComponente := indiceEncaixe};
endfunction

function CarregaComponentesEncontrados(quem, receita, configRecursos)
	var retorno := dictionary;
	
	foreach item in EnumerateItemsInContainer(quem.backpack)
		Sleepms(2);
		foreach componenteNecessario in (receita.componentesNecessarios)
			Sleepms(2);
			if( CStr(item.objtype) in componenteNecessario.itens)
				if(!retorno.exists(componenteNecessario.encaixe))
					var envelopeRetorno := struct{ conjunto := array, melhor := 0, indiceEncaixe := _componenteNecessario_iter};
					retorno.insert(componenteNecessario.encaixe, envelopeRetorno);

				endif

				var chaveMat := GetObjProperty(item,PROPNAME_MATERIAL);
				var material := configRecursos[chaveMat];
				var tier := PegaComponenteTierDesc(GetObjProperty(item,PROPNAME_TIER_COMPONENTE));

				var envelope := struct {
					itemRef := item, 
					chaveMaterial := chaveMat,
					material := material,
					tier := tier,
					quantidade := item.amount};

				AtualizaMelhorComponente(retorno[componenteNecessario.encaixe],
										envelope,
										componenteNecessario.caracteristica,
										componenteNecessario.qtdNecessaria);

				retorno[componenteNecessario.encaixe].conjunto.append(envelope);
			endif
		endforeach
	endforeach 

	return retorno;
endfunction

function AtualizaMelhorComponente(byref componentesEncontrados, envelopeNovo, caracteristica, quantidadeNecessaria)

	if(TypeOfInt(envelopeNovo.quantidade) != OT_UNINIT && envelopeNovo.quantidade < CInt(quantidadeNecessaria))
		return;
	endif

	if(componentesEncontrados.melhor == 0)
		componentesEncontrados.melhor := envelopeNovo;
		return;
	endif
	
	//TODO [futuras versoes] Talvez deixar o poder do atual já guardado, pra nao ter que avaliar toda vez.
	var poderAtual := AvaliaPoderComponente(componentesEncontrados.melhor.tier,
											componentesEncontrados.melhor.chaveMaterial,
											caracteristica,
											configRecursos);
											
	var poderNovo := AvaliaPoderComponente(envelopeNovo.tier,envelopeNovo.chaveMaterial, caracteristica, configRecursos);

	if( poderAtual.poderContado < poderNovo.poderContado)
		componentesEncontrados.melhor := envelopeNovo;
	else
		if(poderAtual.poderContado == poderNovo.poderContado) //Empatou
			if(poderAtual.modificadorHp < poderNovo.poderContado) //Prioridade do desempate é o bônus no HP
				componentesEncontrados.melhor := envelopeNovo;
			else
				if(poderAtual.modificadorPeso < poderNovo.modificadorPeso) // Depois o bônus no peso
					componentesEncontrados.melhor := envelopeNovo;
				endif
			endif
		endif
	endif


endfunction

function AvaliaPoderComponente(tier, chaveMaterial, caracteristica, configRecursos, propriedades := 0)
	var poderContado := 0;
	var retorno := struct;

	if(!propriedades)
		propriedades := MontaPropriedadesComponente(
									chaveMaterial,
									caracteristica,
									tier,
									configRecursos);

	endif
								
	poderContado += propriedades.valorPrincipal;

	foreach efeitoSecundario in (propriedades.efeitosAdicionais)
		poderContado += efeitoSecundario.valor;
	endforeach

	retorno.+poderContado := poderContado;
	retorno.+modPeso := propriedades.modificadorPeso;
	retorno.+modHp := propriedades.modificadorHp;

	return retorno;
endfunction

function MontaPropriedadesComponente(chaveMaterial, caracteristica ,tier ,quantidadeComponentesNecessaria ,configRecursos := 0)

	if(!configRecursos)
		configRecursos := ReadConfigFile(":crafting:config/resourceList");
	endif

	var propriedades := struct {};
	propriedades.+caracteristica := caracteristica;

	var efeitosMaterial := GetConfigStringDictionary(configRecursos[chaveMaterial],"ComponenteEfeito");
	var modificadorHp := GetConfigInt(configRecursos[chaveMaterial],"hpmod");
	var modificadorPeso := GetConfigInt(configRecursos[chaveMaterial],"weightMod");
	var efeitoEscolhido := efeitosMaterial[propriedades.caracteristica];
	var subEfeitos;
	var valorEfeito := 0;
	
	if(modificadorHp)
		propriedades.+modificadorHp := modificadorHp;
	else
		propriedades.modificadorHp := 0;
	endif
	if(modificadorPeso)
		propriedades.+modificadorPeso := Round(modificadorPeso / quantidadeComponentesNecessaria);
	else
		propriedades.modificadorHp := 0;
	endif

	
	if(efeitoEscolhido)
		subEfeitos := SplitWords(efeitoEscolhido);
		valorEfeito := CInt(subEfeitos[subEfeitos.size()]);

		if(subEfeitos.size() > 1)
			subEfeitos.shrink(subEfeitos.size() - 1); // Remove o valorPrincipal e deixa só os subefeitos.
			var arraySubEfeitos := array{};

			for indice := 1 to subEfeitos.size()
				arraySubEfeitos.append(
						struct {
							efeito := subEfeitos[indice],
							valor := Cint(subEfeitos[indice + 1])
						}
					);
				++indice;// dessa forma o indice sobe de 2 em dois, pois o for já adiciona +1.
				Sleepms(2);
			endfor
			propriedades.+efeitosAdicionais := arraySubEfeitos;
			
		endif 
	endif
	
	if(Upper(caracteristica) != "VAZIO")
		propriedades.+valorPrincipal := valorEfeito + Cint(tier.valor);
	endif
	
	return propriedades; 
endfunction

function IsAlvoValido(quem, alvo)

	if( !Accessible(quem,alvo))
		SendSysMessageEX(quem,"Você não alcança o alvo.", SSM_FAIL);
		return 0;
	endif

	return 1;
endfunction

function PreparaItensComplexosGump(quem, profissao)

	var parametrosGump := struct;
	var profissaoConfig := ReadConfigFile(PROFISSAO_CONFIG_PATH);
	var menuFile    := profissaoConfig[profissao].MenuFile;
	var menuPericiaCfg := ReadConfigFile(menuFile);

	var propCraft := GetObjProperty(quem, "Craft");
	if( TypeOf(propCraft) != "Dictionary" )
		propCraft := MontaCraftProp(profissaoConfig);
		SetObjProperty(quem, "Craft", propCraft);
	endif

	var categorias := GetConfigStringArray(menuPericiaCfg["Index"], "Group");
	parametrosGump.+profissao := profissao;
	parametrosGump.+propCraft := propCraft;
	parametrosGump.+receitasConfig := ReadConfigFile(profissaoConfig[profissao].InfoFabricacao);
	parametrosGump.+pericia := profissaoConfig[profissao].Attribute;
	parametrosGump.+visaoReceitas := MontaVisaoReceitas(
											quem,
											categorias,
											menuPericiaCfg,
											parametrosGump.pericia,
											parametrosGump.receitasConfig, 
											parametrosGump.propCraft,
											profissao);
	
	return parametrosGump;
endfunction

function MontaVisaoReceitas(quem,categorias,menuPericiaCfg,nomePericia, receitasConfig, byref propCraft, nomeProfissao)

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
		var itens := GetConfigStringArray(menuPericiaCfg[categoria], "Item");
		var receitas := Array;

		foreach item in itens
			Sleepms(2);
			var itemStringArr := SplitWords(item);
			var receita := Struct;

			var itemOrigem := ""; 
			var nomeItem := ""; 
			var isDuplaOrigem := false;

			case (itemStringArr.size())
				1:
					nomeItem := itemStringArr[1]; // nome
					itemOrigem := "itemComum";
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
			
			var receitaClassificada := ClassificaNomeReceita(nomeItem);
			var objtype   := GetObjTypeByName(receitaClassificada.nome);
			if(TypeOf(objtype) == "Struct")
        		continue;
      		endif

			if(DeveAparecerReceita(
							quem,
							povo,
							itemOrigem,
							receitaClassificada,
							objtype,
							nomePericia,
							receitasAprendidas,
							isDuplaOrigem,
							nomeProfissao))

				var receitaItem := MontaReceitaItem(quem, objtype, nomePericia, receitasConfig, propCraft, nomeProfissao, receitaClassificada);
				if(!receitaItem.isVisualizada)
					var index := categoria in propCraft[nomePericia]["visualizadas"].categorias;
					propCraft[nomePericia]["visualizadas"].categorias.erase(index);
				endif
				receitas.append(receitaItem);
			endif
		endforeach
		retornoVisao.insert(categoria,receitas);
		Sleepms(2);
	endforeach

	return retornoVisao;

endfunction

function MontaReceitaItem(quem, objtype, nomePericia, receitasConfig, byref propCraft, nomeProfissao, receitaClassificada)
	var receita := struct;
	var definicaoItem := itemsConfig[objtype];
	var definicaoFabricacao := receitasConfig[objtype];
	var isComponente := GetConfigString(definicaoFabricacao, "encaixe");
	var sucessos := GetConfigInt(receitasConfig[objtype],"SucessosFabricacao");

	if(TypeOf(definicaoItem) != "ConfigElemRef")
		definicaoItem := itemsConfig[("0x0"+(CStr(objtype) - "0x"))];
	endif

	receita.+componentesNecessarios := PegaComponentesNecessarios(objtype, receitasConfig, receitaClassificada.classe);
	receita.+sucessosNecessarios := sucessos;
	receita.+materiaisNecessarios := PegaMateriaisNecessarios(objtype, definicaoFabricacao);
	receita.+textoDescItem := PegaTextoDescItem(quem,objtype,definicaoItem,receita.componentesNecessarios, receita.materiaisNecessarios, sucessos, definicaoFabricacao.lote);
	receita.+dificuldade := PegaDificuldadeFrabicarObjeto(quem,objtype,nomeProfissao,nomePericia, receitaClassificada.classe);
	receita.+objtype := objtype;
	receita.+pericia := nomePericia;
	receita.+profissao := nomeProfissao;
	receita.+lote := definicaoFabricacao.lote ? definicaoFabricacao.lote : 1;
	receita.+uso := definicaoItem.Attribute;
	receita.+receitaClassificada := receitaClassificada;
	receita.+desc := definicaoItem.desc;
	receita.+graphic := definicaoItem.Graphic ? CInt(definicaoItem.Graphic) : CInt(objtype); //Normalmente o graphic do item é seu objtype, mas a preferência é do atributo graphic no itemdesc.
	receita.+isVisualizada:= PegaNomeReceitaClassificada(receitaClassificada) in (propCraft[nomePericia]["visualizadas"].receitas);

	if(isComponente)
		receita.+complexoComponente := MontaComplexoComponente(definicaoFabricacao); //o item é um complexo considerado componente também.
	endif

	var definicaoTileItem := IU_GetTileInfo( Hex(receita.graphic));
	receita.+isEmpilhavel := definicaoTileItem.Stackable;
	return receita;
endfunction

function MontaComplexoComponente(descritorFabricacao);
	return struct {
		encaixe :=  GetConfigString(descritorFabricacao, "encaixe")
	};
endfunction 

function PegaTextoDescItem(quem,objtype,definicaoItem,componentes, materiaisNecessarios, sucessos, lote)

	var desc := struct;
	var labelQuantidade := "";
	//TODO Lembrar de lidar com as diferenças de nomenclatura para Alchemy.
	if(lote > 0)
		labelQuantidade := lote + "x ";
	endif
	
	desc := "<BASEFONT COLOR=#ffe242>" + labelQuantidade + TruncatePlural(objtype) + "</BASEFONT><br>";
	desc := MontaDescricaoItem(definicaoItem,quem,desc);
	
	if(materiaisNecessarios && materiaisNecessarios.size() > 0)
		desc +=  "<br>";
		desc +=  "<BASEFONT COLOR=#7bf8fe> Materiais Necessários</BASEFONT><br>";
		foreach material in materiaisNecessarios
			var materialInfo := SplitWords(material);
			desc +=  cint(materialInfo[2]) * lote + " " + GetMaterialName(materialInfo[1])  + "<br>";
			Sleepms(2);
		endforeach
	endif

	if(componentes  && componentes.size() > 0)
		desc +=  "<br>";
		desc +=  "<BASEFONT COLOR=#7bf8fe> Encaixes de Componente </BASEFONT><br>";
		foreach componente in componentes
			desc +=  cint(componente.qtdNecessaria) * lote+ " " + componente.encaixeDesc  + "<br>";
			Sleepms(2);
		endforeach
	endif

	desc +=  "<br>";
	desc +=  "<BASEFONT COLOR=#7bf8fe> Tempo de Fabricação</BASEFONT><br>" + PegaDescDuracaoFabricacao(sucessos) + "<br>";

	return desc;
endfunction

function PegaComponentesNecessarios(objtype, receitasConfig, classe)
	var encaixesItem := GetConfigStringArray(receitasConfig[objtype],"ComponenteEncaixe");
	var configEncaixes := ReadConfigFile(":*:encaixes");
	classe := classe ? classe : 1;

	var componentes := array;
	foreach encaixe in encaixesItem
		Sleepms(2);
		var componente := struct;
		var splitEncaixe := SplitWords(encaixe);
		componente.+classe := splitEncaixe[DESC_ENCAIXE_CLASSE];
		if(Cint(componente.classe) != cint(classe)) continue; endif

		componente.+caracteristica :=  splitEncaixe[DESC_ENCAIXE_CARACTERISTICA];
		componente.+encaixe := splitEncaixe[DESC_ENCAIXE_ENCAIXE];
		componente.+qtdNecessaria := splitEncaixe[DESC_ENCAIXE_QTD_NECESSARIA];

		case (splitEncaixe[DESC_ENCAIXE_CONDICIONAL])
			"principal": componente.+isPrincipal := splitEncaixe[DESC_ENCAIXE_CONDICIONAL] == "principal"; break;
			"opcional": componente.+isOpcional := splitEncaixe[DESC_ENCAIXE_CONDICIONAL] == "opcional"; break;
		endcase

		componente.+encaixeDesc := GetConfigString(configEncaixes[componente.encaixe],"Desc");
		componente.+itens := GetConfigStringArray(configEncaixes[componente.encaixe],"Item");
		componentes.append(componente);
	endforeach

	return componentes;
endfunction

function PegaMateriaisNecessarios(objtype, descritorFabricacao)
	var materials := GetConfigStringArray(descritorFabricacao, "materials");
	return materials;
endfunction

function DeveAparecerReceita(quem,povo,itemOrigem,receitaClassificada,objtype,nomePericia,receitasAprendidas,isDuplaOrigem, nomeProfissao)
	
	var isOrigemValida := false;
	var isValorPericiaSuficiente := TemValorPericiaNecessaria(quem, objtype, receitaClassificada.classe, nomePericia, nomeProfissao );
	
	if(isDuplaOrigem)
		isOrigemValida := PegaNomeReceitaClassificada(receitaClassificada) in receitasAprendidas || povo == itemOrigem;
	else
		if(itemOrigem == "Recipe")
			isOrigemValida := PegaNomeReceitaClassificada(receitaClassificada) in receitasAprendidas;
		elseif(itemOrigem == "itemComum")
			isOrigemValida := true;
		else
			isOrigemValida := povo == itemOrigem;
		endif
	endif

	return isOrigemValida && isValorPericiaSuficiente;
endfunction

function TemValorPericiaNecessaria(quem, objtype, classe, nomePericia, nomeProfissao)
	var settings := AP_GetSettingsCfgElem("Skills");

	return (  PegaSkillNecessaria(objtype, nomeProfissao, classe) - Cdbl(settings.TooHard) + FIXO_DIF_ADICIONAL_PERICIA) <= AP_GetSkill(quem, nomePericia);
endfunction

function ClassificaRetornoComplexos(retorno,sessao)
	var valorRetorno := retorno[0];
	if(valorRetorno > COMPLEXOS_CATEGORIAS && valorRetorno <= COMPLEXOS_RECEITAS)
		return RT_CATEGORIAS;
	elseif(valorRetorno > COMPLEXOS_RECEITAS && valorRetorno <= COMPLEXOS_ESQUEMAS)
		return RT_RECEITAS;
	elseif(valorRetorno > COMPLEXOS_ESQUEMAS && valorRetorno <= COMPLEXOS_ESQUEMAS_CPNT)
		return RT_ESQUEMAS;
	elseif(GerenciadorDropDown::IsIdDentroDropDowns(sessao.gerenciadorDropDown,valorRetorno))
		return RT_ESCOLHA_DROPDOWN_CPX;
	elseif(GerenciadorDropDown::IsIdDentroOpcoes(sessao.gerenciadorDropDown,valorRetorno))
		return RT_ESCOLHA_DD_OPCOES_CPX;
	elseif(valorRetorno == COMPLEXOS_ESQUEMAS_BTN_SALVAR)
		return RT_ESQUEMAS_SALVAR;
	elseif((valorRetorno == 1 || valorRetorno == 0) && sessao.gumpAtual == ESQUEMAS)
		return RT_CANCELADO_ESQUEMA_COMPLX;
	elseif(valorRetorno == 1)
		return RT_CANCELADO_COMPLX;
	endif
endfunction