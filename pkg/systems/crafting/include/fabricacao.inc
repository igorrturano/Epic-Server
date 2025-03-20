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

//var fabricacaoGump := DFOpenDataFile(":crafting:CraftGumps", DF_CREATE);
var fabricacaoConfig := ReadConfigFile( ":crafting:config/craftConfig" );
var itemsConfig := ReadConfigFile(":*:itemdesc");

const NR_MAXIMO_ESQUEMAS := 6;
const OBJTYPE_ITEM_MONTADO := 0xe72;
const FIXO_DIF_ADICIONAL_PERICIA := 0; //Adicional no limite mais baixo da dificuldade, o resultado de um somatório faz aparecer no gump.

//Itens Complexos Inicio
function FabricaItensComplexos(quem,profissao)
	EnviaItensComplexosGump(quem,profissao);
endfunction

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
				componentesEscolhidos := dictComponentesEscolhidos
			};
endfunction

function SelecionaReceita(byref sessao, quem, parametros , indice)
	var objetoReceita := parametros.visaoReceitas[sessao.categoriaSelecionada][indice];
	NovoObjetoReceitaSelecionada(sessao,objetoReceita,ListaEsquemas(quem,objetoReceita.objtype));
endfunction

function EnviaItensComplexosGump(quem,profissao)
	var configRecursos := ReadConfigFile(":crafting:config/resourceList");
	var parametros := PreparaItensComplexosGump(quem, profissao);
	var retorno := true;
	var gump;
	var sessao := InicializaSessaoComplexos(quem, parametros.propCraft[parametros.pericia]);
	var gerenciador := sessao.gerenciadorDropDown;
	
	//TODO Fazer opção para reparar
	//TODO criar botão para sair
	//TODO Criar fluxo para itens stackaveis. [Já fiz,mas tem que testar.]
	//TODO implementar componentes opcionais [já fiz os opcionais, tem que testar.] 
	//TODO Decidir se item complexos podem ser usado como componente. (tranca, por exemplo)
	//TODO Solucionar o problema das armaduras estarem sem informações.
	//TODO Deixar o cara escolher se quer fazer um componente de qualidade mais baixa.
	//TODO verificar porque os componentes não estão estacando na criação..
	//TODO Na hora de montar as propriedades, Passar o modificador de HP e peso de acordo com o material.
	//TODO contabilizar HP e Peso no cálculo para avaliar o poder, talvez como critério de desempate?
	//TODO Munições precisam ser fabricadas de tantas em tantas. [ 10 em 10 ]?
	//TODO Definir um fluxo para Excepcionais.
	//TODO implementar lógica de adicionais de dificuldade. (Exemplo: componentes melhores deixam mais dificil.)
	//TODO implementar a chamada tanto do gump de componentes quanto gump de complexos usando as pericias corretas e alterar a forma de leitura da cfg de craft.
	//TODO Modificar as Configs das receitas pra colocar o elemento "item" onde não tiver.
	//TODO Criar gumps de ajuda.
	//TODO Criar API para gerar item fabricado.
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
				//Print("Sessao info:   " + sessao.receitaSelecionada);
				if(sessao.receitaSelecionada.esquemaSelecionado.nome == "")
					SendSysMessageEX(quem,"O nome do esquema é obrigatório.", SSM_FAIL);
					break;
				endif
				//Print("Salvei Esquema {}!".format(sessao.receitaSelecionada.esquemaSelecionado.nome));
				case(sessao.acao)
					ADICIONAR:
						var isAdicionado := AdicionaEsquema(quem,
										sessao.receitaSelecionada.receita.objtype,
										sessao.receitaSelecionada.esquemaSelecionado);
						sessao.receitaSelecionada.esquemas := ListaEsquemas(quem,sessao.receitaSelecionada.receita.objtype);
						if(!isAdicionado)
							SendSysMessageEX(quem,isAdicionado.errortext,SSM_FAIL);
						endif
						break;

					EDITAR:
						EditaEsquema(quem,
									sessao.receitaSelecionada.receita.objtype,
									sessao.receitaSelecionada.esquemaSelecionado
									);
						sessao.receitaSelecionada.esquemas := ListaEsquemas(quem,sessao.receitaSelecionada.receita.objtype);
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

				case (retornoMapeado.indiceBotao)
					1: //Editar
						sessao.receitaSelecionada.esquemaSelecionado := esquema;
						sessao.gumpAtual := ESQUEMAS;
						sessao.acao := EDITAR;
						break;

					2: //Excluir
						sessao.receitaSelecionada.esquemas := RemoveEsquema(quem, sessao.receitaSelecionada.receita.objtype, esquema);
						break;

					3: //Fabricar
						//TODO Animações e sons.
						FabricaItens(quem, esquema, sessao.receitaSelecionada.receita, sessao.quantidade);
						break;

				endcase
				break;

			RT_BOTAO_MENU:
				sessao.gumpAtual := ESQUEMAS;
				sessao.devePopularDropDown := true;
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
				if(!(receita.objtype in sessao.visualizadas.receitas))
					sessao.visualizadas.receitas.append(receita.objtype); //guardo as que foram visualizadas para atualizar a prop no final;
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
		var opcaoEncontrada := MontaObetoOpcaoComponente(componenteEncontrado.melhor, componenteEncontrado.indiceEncaixe);
		var componenteEscolhido := MontaComponenteEscolhido(opcaoEncontrada, componenteNecessario, configRecursos);
		AcrescentaComponenteEscolhido(sessao.receitaSelecionada.esquemaSelecionado,componenteNecessario,componenteEscolhido, opcaoEncontrada.indiceEncaixeComponente);
	endforeach

	return sessao.receitaSelecionada.esquemaSelecionado;
endfunction

function FabricaItens(quem, esquema, receita, quantidade := 1)
	var posicaoInicial := CriaObjetoPosicao(quem);
	var vidaInicial := AP_GetVital(quem, "Hits");

	if(receita.isEmpilhavel)
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
			var retorno := FabricaItemMontado(quem,posicaoInicial,esquema, receita);
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

	if(definicaoItem.weight)
		pesoFinal := pesoFinal * definicaoItem.weight;
	else
		pesoFinal := pesoFinal * definicaoTileItem.weight;
	endif

	var descriptor := GetItemDescriptor(CInt(OBJTYPE_ITEM_MONTADO));
	descriptor.color := esquema.cor;
	descriptor.graphic := receita.graphic;
	descriptor.CProps.insert(PROPNAME_FABRICADO, DefinePropFabricado(esquema,receita));
	descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG, true);
	
	var item := CreateItemInBackpack(quem,descriptor, quantidade);
	item.weight_multiplier_mod := pesoFinal;
	SendSysMessageEX(quem, "{} criado com sucesso.".format(item.desc), SSM_INFO);

endfunction

function DefinePropFabricado(esquema,receita)
	//Limpa propriedades desnecessárias ou rendundantes.
	receita.-componentesNecessarios;
	receita.-materiaisNecessarios;
	receita.-textoDescItem;
	receita.-dificuldade;
	receita.-isVisualizada;
	receita.-uso;

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
		if(elemento.quantidadeEncontrada < elemento.quantidadeNecessaria * quantidade )
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
											qtdNecessaria
											);
			endif
		endforeach

	endforeach 
	return contagemFinal;
endfunction

function ConsomeRequisitos(contagem, quantidade := 1)
	foreach contado in contagem
		Sleepms(2);
		var quantidadeRestante := contado.quantidadeNecessaria * quantidade;
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

function RegistraInformacoesContagem(byref catalogoContagem ,byref item , quantidadeNecessaria, encaixe := 0, tierDesc := "")

	if(!catalogoContagem.exists(item.objtype))
		var informacoesContagem := struct;
		informacoesContagem.+quantidadeEncontrada := 0;
		informacoesContagem.+itemRefArray := array;
		informacoesContagem.+nomeItem := item.name;
		informacoesContagem.+tierDesc := tierDesc;
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
		var componenteEscolhido := MontaComponenteEscolhido(retornoDropDownOpcao, componenteNecessario, configRecursos);
		AcrescentaComponenteEscolhido(sessao.receitaSelecionada.esquemaSelecionado,componenteNecessario,componenteEscolhido, retornoDropDownOpcao.indiceEncaixeComponente);
	endif
endfunction

function AcrescentaComponenteEscolhido (byref esquemaSelecionado, componenteNecessario, componenteEscolhido, indiceEncaixe)
	if(componenteNecessario.isPrincipal)
			esquemaSelecionado.cor := configRecursos[componenteEscolhido.material.chave].color;
			esquemaSelecionado.material := componenteEscolhido.material.chave;
	endif

	if(componenteEscolhido.poderAvaliado != 0)// lida com a flag para ignorar desc na tooltip se tiver só componente comum
		esquemaSelecionado.isCompletamenteComum := 0;
	endif

	if(esquemaSelecionado.componentesEscolhidos.exists(indiceEncaixe))
		esquemaSelecionado.componentesEscolhidos[indiceEncaixe] := componenteEscolhido;
	else
		esquemaSelecionado.componentesEscolhidos.insert(indiceEncaixe, componenteEscolhido);
	endif

	return esquemaSelecionado;
	
endfunction

function MontaComponenteEscolhido(componenteEncontrado, componenteNecessario, configRecursos)
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
														configRecursos);
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
	
	if( poderAtual < poderNovo)
		componentesEncontrados.melhor := envelopeNovo;
	endif

endfunction

function AvaliaPoderComponente(tier, chaveMaterial, caracteristica, configRecursos, propriedades := 0)
	var poderContado := 0;

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

	return poderContado;
endfunction

function MontaPropriedadesComponente(chaveMaterial, caracteristica ,tier, configRecursos := 0)

	if(!configRecursos)
		configRecursos := ReadConfigFile(":crafting:config/resourceList");
	endif

	var propriedades := struct {};
	propriedades.+caracteristica := caracteristica;

	var efeitosMaterial := GetConfigStringDictionary(configRecursos[chaveMaterial],"ComponenteEfeito");
	var efeitoEscolhido := efeitosMaterial[propriedades.caracteristica];
	
	if(efeitoEscolhido)
		var subEfeitos := SplitWords(efeitoEscolhido);

		propriedades.+valorPrincipal := CInt(subEfeitos[subEfeitos.size()]) + Cint(tier.valor);

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
	parametrosGump.+pericia := fabricacaoConfig[profissao].Attribute;
	parametrosGump.+visaoReceitas := MontaVisaoReceitas(quem,categorias,menuPericiaCfg,parametrosGump.pericia, parametrosGump.propCraft);
	
	return parametrosGump;
endfunction

function MontaVisaoReceitas(quem,categorias,menuPericiaCfg,nomePericia,byref propCraft)

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

			var objtype   := GetObjTypeByName(nomeItem);
			if(TypeOf(objtype) == "Struct")
        		continue;
      		endif

			if(DeveAparecerReceita(quem,povo,itemOrigem,nomeItem,objtype,nomePericia,receitasAprendidas,isDuplaOrigem))
				var receitaItem := MontaReceitaItem(quem, objtype, nomePericia, propCraft);
				if(!receitaItem.isVisualizada)
					//TODO FIX tá removendo todas as categorias visualizadas, mesmo quando eu já vi elas.
					var index := propCraft[nomePericia]["visualizadas"].categorias.findIndex( @(categoriaArray) {return categoriaArray == categoria;});
					propCraft[nomePericia]["visualizadas"].categorias.erase(index);
				endif
				receitas.append(receitaItem);
			endif
			Sleepms(2);
		endforeach
		retornoVisao.insert(categoria,receitas);
		Sleepms(2);
	endforeach

	return retornoVisao;

endfunction

function MontaReceitaItem(quem, objtype, nomePericia,byref propCraft)
	var receita := struct;
	var definicaoItem := itemsConfig[objtype];
	var descritorFabricacao := craft_config[objtype];
	var sucessos := GetConfigInt(craft_config[objtype],"SucessosFabricacao");

	if(TypeOf(definicaoItem) != "ConfigElemRef")
		definicaoItem := itemsConfig[("0x0"+(CStr(objtype) - "0x"))];
	endif

	receita.+componentesNecessarios := PegaComponentesNecessarios(objtype);
	receita.+sucessosNecessarios := sucessos;
	receita.+materiaisNecessarios := PegaMateriaisNecessarios(objtype, descritorFabricacao);
	receita.+textoDescItem := PegaTextoDescItem(quem,objtype,definicaoItem,receita.componentesNecessarios, receita.materiaisNecessarios, sucessos);
	receita.+dificuldade := PegaDificuldade(quem, descritorFabricacao, nomePericia);
	receita.+objtype := objtype;
	receita.+pericia := nomePericia;
	receita.+uso := definicaoItem.Attribute;
	receita.+desc := definicaoItem.desc;
	receita.+graphic := definicaoItem.Graphic ? CInt(definicaoItem.Graphic) : CInt(objtype); //Normalmente o graphic do item é seu objtype, mas a preferência é do atributo graphic no itemdesc.
	receita.+isVisualizada:=  objtype in (propCraft[nomePericia]["visualizadas"].receitas);

	var definicaoTileItem := IU_GetTileInfo( Hex(receita.graphic));
	receita.+isEmpilhavel := definicaoTileItem.Stackable;
	return receita;

endfunction

function PegaTextoDescItem(quem,objtype,definicaoItem,componentes, materiaisNecessarios, sucessos)

	var desc := struct;
	//TODO Lembrar de lidar com as diferenças de nomenclatura para Alchemy.
	desc := "<BASEFONT COLOR=#ffe242>" + TruncatePlural(objtype) + "</BASEFONT><br>";
	desc := MontaDescricaoItem(definicaoItem,quem,desc);
	
	if(materiaisNecessarios && materiaisNecessarios.size() > 0)
		desc +=  "<br>";
		desc +=  "<BASEFONT COLOR=#7bf8fe> Materiais Necessários</BASEFONT><br>";
		foreach material in materiaisNecessarios
			var materialInfo := SplitWords(material);
			desc +=  materialInfo[2] + " " + GetMaterialName(materialInfo[1])  + "<br>";
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
	desc +=  "<BASEFONT COLOR=#7bf8fe> Tempo de Fabricação</BASEFONT><br>" + PegaDescDuracaoFabricacao(sucessos) + "<br>";

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

		case (splitEncaixe[4])
			"principal": componente.+isPrincipal := splitEncaixe[4] == "principal"; break;
			"opcional": componente.+isOpcional := splitEncaixe[4] == "opcional"; break;
		endcase

		componente.+encaixeDesc := GetConfigString(configEncaixes[componente.encaixe],"Desc");
		componente.+itens := GetConfigStringArray(configEncaixes[componente.encaixe],"Item");
		componentes.append(componente);
		Sleepms(2);
	endforeach

	return componentes;
endfunction

function PegaMateriaisNecessarios(objtype, descritorFabricacao)
	var materials := GetConfigStringArray(descritorFabricacao, "materials");
	return materials;
endfunction

function PegaDescDuracaoFabricacao(qtdSucessos)
	if(qtdSucessos > 30)
		return "Extremamente Longo";
	elseif(qtdSucessos <= 30 && qtdSucessos > 25)
		return "Muito Longo";
	elseif(qtdSucessos <= 20 && qtdSucessos > 15)
		return "Longo";
	elseif(qtdSucessos <= 15 && qtdSucessos > 10)
		return "Razoável";
	elseif(qtdSucessos <= 10 && qtdSucessos > 5)
		return "Rápido";
	else
		return "Muito Rápido";
	endif
endfunction

function DeveAparecerReceita(quem,povo,itemOrigem,nomeItem,objtype,nomePericia,receitasAprendidas,isDuplaOrigem)
	
	var isOrigemValida := false;
	var isValorPericiaSuficiente := TemValorPericiaNecessaria(quem, objtype, nomePericia);
	
	if(isDuplaOrigem)
		isOrigemValida := nomeItem in receitasAprendidas || povo == itemOrigem;
	else
		if(itemOrigem == "Recipe")
			isOrigemValida := nomeItem in receitasAprendidas;
		elseif(itemOrigem == "itemComum")
			isOrigemValida := true;
		else
			isOrigemValida := povo == itemOrigem;
		endif
	endif

	return isOrigemValida && isValorPericiaSuficiente;
endfunction

function TemValorPericiaNecessaria(quem, objtype, nomePericia)
	var settings := AP_GetSettingsCfgElem("Skills");
	return (  craft_config[objtype].skill - Cdbl(settings.TooHard) + FIXO_DIF_ADICIONAL_PERICIA) <= AP_GetSkill(quem, nomePericia);
endfunction

function MontaCraftProp(fabricacaoConfig)

	var baseDict  := dictionary;
	foreach pericia in GetConfigStringArray(fabricacaoConfig["Skills"], "Skill")
		var temp := dictionary;
		var visualizadas := struct;
		visualizadas.+receitas := array;
		visualizadas.+categorias := array;

		temp.+skill  := fabricacaoConfig[pericia].Attribute;
		temp.+visualizadas := visualizadas;
		temp.+last   := 0;
		baseDict[temp.skill] := temp;
		Sleepms(2);
	endforeach
	
	return baseDict;

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
	elseif(valorRetorno == 1 && sessao.gumpAtual == ESQUEMAS)
		return RT_CANCELADO_ESQUEMA_COMPLX;
	elseif(valorRetorno == 1)
		return RT_CANCELADO_COMPLX;
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
						pericia,
						sessao.componenteSelecionado.encaixe);
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

function ProduzComponentes(quem,configRecursos,objtype,materiaisEncontrados, materiaisEscolhidos,quantidade, pericia, encaixe)

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

// TODO [futuras versões] pensar em um modo de aplicar uma dificuldade a mais dependendo do componente.
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