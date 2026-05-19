//Author: Deiro || Rafael Santos

include "/include/epicConstants";
include "/include/epicPropNames";
include "/include/math";
include "include/epicUtils";
include ":megacliloc:/include/descricaoObjetoTTip";
include ":crafting:/craftingDFEsquemas";
include ":crafting:/include/fabricacaoUtil";
include ":crafting:/include/craftingRepair";
include "include/epicValidAction";
include ":crafting:include/util/fabricacaoConstants";
include ":expbar:expbar";

function NovoObjetoModoGm()
	return struct{ 
					encaixeEscolhido := struct { indice := 0, encaixe := 0 },
					componenteEscolhido := 0, 
					qualidadeEscolhida := 0, 
					materialEscolhido := 0
			};
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

function NovoObjetoEsquemaSelecionado(
									nomeEsquema := "", 
									dictComponentesEscolhidos := dictionary, 
									corItem := 0, 
									material := "", 
									classe := 1
								)
	return struct{
				nome := nomeEsquema,
				cor := corItem,
				material := material,
				isCompletamenteComum := 1,
				modificadorHpTotal := 0,
				modificadorPesoTotal := 0,
				qualidade := 0,
				classe := classe,
				modificadorCache := struct{ peso := dictionary, hp := dictionary, qualidade := dictionary},
				componentesEscolhidos := dictComponentesEscolhidos
			};
endfunction

function SelecionaReceita(byref sessao, quem, parametros , indice)
	var objetoReceita := parametros.visaoReceitas[sessao.categoriaSelecionada][indice];
	var nomeReceitaClassificado := PegaNomeReceitaClassificada(objetoReceita.receitaClassificada);
	sessao.paginador.esquemas.pagina := 0;
	NovoObjetoReceitaSelecionada(sessao,objetoReceita,ListaEsquemas(quem,nomeReceitaClassificado));
endfunction



function TrataGMEscolhaComponente(quem, byref sessao)
	var modoGm := sessao.modoGm;

	var encaixeEscolhido := modoGm.encaixeEscolhido;
	var componenteEscolhido := modoGm.componenteEscolhido;
	var qualidadeEscolhida := modoGm.qualidadeEscolhida;
	var materialEscolhido := modoGm.materialEscolhido;

	if(!componenteEscolhido || !qualidadeEscolhida || !materialEscolhido)
		SendSysMessageEX(quem, "Todas as opções são obritatórias.", SSM_FAIL);
		return 0;
	endif

	var componenteNecessario := sessao.
									receitaSelecionada.
									receita.
									componentesNecessarios[encaixeEscolhido.indice];


	var componenteEncontrado := MontaEnvelopeComponenteEncontrado(
														componenteEscolhido, 
														materialEscolhido.chave,
														materialEscolhido,
														qualidadeEscolhida);
	
	var objetoOpcaoEscolhida := MontaObetoOpcaoComponente(componenteEncontrado,encaixeEscolhido.indice);
	
	TrataComponenteEncontrado(
									sessao,
									objetoOpcaoEscolhida,
									componenteNecessario,
									encaixeEscolhido.indice,
									configRecursos
								);

	return 1;
endfunction

function MarcaCategoriaVisualizada(byref sessao,byref parametros)
	if(sessao.categoriaAnterior != "")
		if(!(sessao.categoriaAnterior in (sessao.visualizadas.categorias)))
			sessao.visualizadas.categorias.append(sessao.categoriaAnterior);
			foreach receita in (parametros.visaoReceitas[sessao.categoriaAnterior])
				Sleepms(2);
				var nomeClassificado := PegaNomeReceitaClassificada(receita.receitaClassificada);
				if(!(nomeClassificado in sessao.visualizadas.receitas))
					receita.isVisualizada := true;
					sessao.visualizadas.receitas.append(nomeClassificado); //guardo as que foram visualizadas para atualizar a prop no final;
				endif
			endforeach
		endif
	endif
endfunction

function MontaMelhorEsquema(quem, sessao)
	var componentesEncontrados := CarregaComponentesEncontrados(quem, sessao.receitaSelecionada.receita, configRecursos);

	foreach componenteEncontrado in componentesEncontrados
		Sleepms(2);
		var componenteNecessario := sessao.receitaSelecionada.receita.componentesNecessarios[componenteEncontrado.indiceEncaixe];
		var quantidadeComponentesNecessaria :=  sessao.receitaSelecionada.receita.componentesNecessarios.size();
		var opcaoEncontrada := MontaObetoOpcaoComponente(componenteEncontrado.melhor, componenteEncontrado.indiceEncaixe);
		var componenteEscolhido := MontaComponenteEscolhido(opcaoEncontrada, componenteNecessario, configRecursos, quantidadeComponentesNecessaria, sessao.receitaSelecionada.receita.quantidadeOpicionais);

		AcrescentaComponenteEscolhido(
				sessao.receitaSelecionada.esquemaSelecionado,
				componenteNecessario,
				componenteEscolhido, 
				opcaoEncontrada.indiceEncaixeComponente);

	endforeach

	var classe := sessao.receitaSelecionada.receita.receitaClassificada.classe;
	sessao.receitaSelecionada.esquemaSelecionado.classe := classe ? cint(classe) : CLASSE_PADRAO;

	return sessao.receitaSelecionada.esquemaSelecionado;
endfunction

function FabricaItens(quem, esquema, receita, quantidade := 1, modoGm := false, skill := "", categoria := "")
	var posicaoInicial := BuildPositionStruct(quem);
	var vidaInicial := AP_GetVital(quem, "Hits");

	if(receita.isEmpilhavel)
		Sleep(1);
		var retorno := FabricaItemMontado(quem,esquema, receita, quantidade, modoGm, skill, categoria);
		if(!retorno)
			SendSysMessageEX(quem, "Você parou de fabricar produtos.", SSM_FAIL);
		endif
	else
		for indice := 1 to Cint(quantidade)
			if(modoGm)
				Sleepms(2);
			else
				Sleep(1);
			endif

			if(!CheckCanWork(quem,vidaInicial, posicaoInicial))
				break;
			endif
			var retorno := FabricaItemMontado(quem,esquema, receita, 1, modoGm, skill, categoria);
			if(!retorno)
				SendSysMessageEX(quem, "Você parou de fabricar produtos.", SSM_FAIL);
				break;
			endif
		endfor
	endif
endfunction

function FabricaItemMontado(quem,esquema, receita, quantidade := 1, deveIgnorarRequisitos := false, skill := "", categoria := "")
	var contagem := ContaQuantidadeRequisitos(quem,esquema, receita, deveIgnorarRequisitos);

	if(isRequisitoSuficiente(quem,contagem, receita, esquema, quantidade, deveIgnorarRequisitos))
		if(!deveIgnorarRequisitos)
			ConsomeRequisitos(quem, contagem, quantidade);
		endif

		if(deveIgnorarRequisitos)
			var retorno := CriaItemFabricado(quem,0,quantidade, DefinePropFabricado(esquema,receita, quem));

			if(retorno)
				SendSysMessageEX(quem, "criado com sucesso.", SSM_INFO);
			else
				SendSysMessageEX(quem, "Não foi possivel criar o item.", SSM_FAIL);
			endif
		else
			// Compute now (receita.dificuldade still present) but DO NOT award:
			// stamp it on the montado so the XP is only paid at completion
			// (EV_ITEM_PRODUZIDO). Awarding here would be farmable.
			var xpPendente := ComputeCraftXP(receita, skill, categoria);
			var itemCriado := CriaItemMontado(quem,esquema,receita, quantidade);
			if(itemCriado && xpPendente > 0)
				var propFab := GetObjProperty(itemCriado, PROPNAME_FABRICADO);
				if(propFab)
					propFab.+xpPendente := xpPendente;
					propFab.+xpCategoria := categoria;
					SetObjProperty(itemCriado, PROPNAME_FABRICADO, propFab);
				endif
			endif
		endif
	else
		return 0;
	endif

	return 1;
endfunction

function hasMovido(quem,posicaoInicial)
	return quem.x != posicaoInicial.x 
			|| quem.y != posicaoInicial.y 
			|| quem.z != posicaoInicial.z 
			|| quem.realm != posicaoInicial.realm;
endfunction

function CriaItemMontado(quem,esquema,receita, quantidade)
	var infoAdicional := struct;
	var descriptor := MontaProdutoMontadoDescriptor(quem,esquema,receita, infoAdicional);

	var item := CreateItemInBackpack(quem,descriptor, cint(quantidade));

	if(item)
		item.weight_multiplier_mod := infoAdicional.pesoFinal;
		SendSysMessageEX(quem, "criado com sucesso.".format(item.desc), SSM_INFO);
	else
		SendSysMessageEX(quem, "falha ao criar item. {} " .format(item), SSM_FAIL);
	endif

	return item;

endfunction

function isRequisitoSuficiente(quem, contagem, receita, esquema, quantidade := 1, deveIgnorarRequisitos := false)

	var encaixesEncontrados := array;
	var configEncaixes := ReadConfigFile(":*:encaixes");
	
	if(deveIgnorarRequisitos) return true; endif
	
	foreach elemento in contagem
		Sleepms(2);
		if(elemento.quantidadeEncontrada < cint(elemento.quantidadeNecessaria) * cint(quantidade) * cint(elemento.lote))
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
		if(!(necessario.encaixe in encaixesEncontrados))
			if(necessario.isOpcional)
				foreach componenteEscolhido in (esquema.componentesEscolhidos)
					Sleepms(2);
					if((componenteEscolhido.componenteNecessario.encaixe == necessario.encaixe))//o cara selecionou, mas não tinha nenhum componente. Neste caso acaba não caíndo na contagem.
						SendSysMessageEX(quem,"Você não tem componentes suficientes para o encaixe opcional [{}].".format(configEncaixes[necessario.encaixe].desc),SSM_FAIL);
						return false;
					endif
				endforeach
			else
				SendSysMessageEX(quem,"O encaixe [{}] é obrigatório e não foi preenchido.".format(configEncaixes[necessario.encaixe].desc),SSM_FAIL);
				return false;
			endif
		endif
	
	endforeach

	return true;
endfunction

function IsMateriaisNaoEncontrados(receita, contagem)
	return receita.materiaisNecessarios.size() - receita.componentesNecessarios.size() > contagem.size();
endfunction

function ContaQuantidadeRequisitos(quem, esquema, receita, deveIgnorarRequisitos)
	var contagemFinal := dictionary;

	if(deveIgnorarRequisitos) return contagemFinal; endif

	foreach item in (EnumerateItemsInContainer(quem.backpack, ENUMERATE_ROOT_ONLY))
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
			var materialNecessarioObjType :=  cint(ingredienteInfo[1]);
			var qtdNecessaria := cint(ingredienteInfo[2]);

			if(!contagemFinal.exists(materialNecessarioObjType))
				var materialDesc := itemsConfig[materialNecessarioObjType];
				InsereInformacoesContagem(
											contagemFinal, 
											struct {
														name := materialDesc.desc, 
														objtype := materialNecessarioObjType
													},
											qtdNecessaria,
											receita.lote);
			endif

			if(isMaterialNecessario(item, materialNecessarioObjType))
				
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

function ConsomeRequisitos(quem, contagem, quantidade := 1)
	foreach contado in contagem
		Sleepms(2);
		var quantidadeRestante := cint(contado.quantidadeNecessaria) * cint(contado.lote) * cint(quantidade);
		foreach itemEncontrado in (contado.itemRefArray)
			Sleepms(2);
			If(quantidadeRestante <= 0)
				break;
			endif;
			var objtypeConsumido := itemEncontrado.objtype;
			var quantidadeConsumida;
			if(itemEncontrado.stackable)
				quantidadeConsumida := cint(itemEncontrado.amount) < quantidadeRestante ? cint(itemEncontrado.amount) : quantidadeRestante;
				var quantidadeSubtraida :=  quantidadeRestante - itemEncontrado.amount;
				SubtractAmount(itemEncontrado, quantidadeRestante);
				quantidadeRestante :=  quantidadeSubtraida;
			else
				quantidadeConsumida := 1;
				quantidadeRestante -= 1;
				DestroyItem(itemEncontrado);
			endif
			DevolveContainerVazio(quem, objtypeConsumido, quantidadeConsumida);
		endforeach
	endforeach
endfunction

function DevolveContainerVazio(quem, objtypeConsumido, quantidadeConsumida)
	if(!quem || quantidadeConsumida <= 0) return; endif
	var descItem := itemsConfig[objtypeConsumido];
	if(!descItem) return; endif
	var emptyObjType := cint(GetConfigInt(descItem, "emptyObjType"));
	if(emptyObjType <= 0) return; endif
	CreateItemInBackpack(quem, emptyObjType, quantidadeConsumida);
endfunction

function RegistraInformacoesContagem(byref catalogoContagem ,byref item , quantidadeNecessaria, lote, encaixe := 0, tierDesc := "")

	if(!catalogoContagem.exists(item.objtype))
		InsereInformacoesContagem(catalogoContagem, item, quantidadeNecessaria, lote, encaixe, tierDesc);
	endif

	catalogoContagem[item.objtype].quantidadeEncontrada += item.amount;
	catalogoContagem[item.objtype].itemRefArray.append(item);
endfunction

function InsereInformacoesContagem(byref catalogoContagem, item, quantidadeNecessaria, lote, encaixe := 0, tierDesc := "")
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
	return informacoesContagem;
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

function TratarEscolhaDropDown(quem, dropdown,byref sessao, configRecursos)

	if(sessao.modoGm)
		var retornoMapeado := MapeiaRetornoGump(
											dropdown.PegaIdBottao(), 
											dropdown.PegaGerenciador().PegaAdicionalIdDropDown(), 
											3
										);
		var valorEscolhido := dropdown.PegaValorEscolhido();

		case(retornoMapeado.indiceBotao)
			1:
				sessao.modoGm.componenteEscolhido := valorEscolhido;
				break;
			2:
				sessao.modoGm.qualidadeEscolhida := valorEscolhido;
				break;
			3:
				sessao.modoGm.materialEscolhido := valorEscolhido;
				break;
		endcase
	else
		var retornoDropDownOpcao := dropdown.PegaValorEscolhido();
		var componenteNecessario := sessao.
									receitaSelecionada.
									receita.
									componentesNecessarios[retornoDropDownOpcao.indiceEncaixeComponente];
	
		if(IsAlvoValido(quem,retornoDropDownOpcao.itemRef))
			TrataComponenteEncontrado(
									sessao,
									retornoDropDownOpcao,
									componenteNecessario,
									retornoDropDownOpcao.indiceEncaixeComponente,
									configRecursos
								);
		endif
	endif
	
endfunction

function TrataComponenteEncontrado(
								byref sessao, 
								componenteEncontrado, 
								componenteNecessario, 
								indiceEncaixeComponente, 
								configRecursos 
							)
	
	var quantidadeComponentesNecessaria := sessao.receitaSelecionada.receita.componentesNecessarios.size();
	var componenteEscolhidoMontado := MontaComponenteEscolhido(
														componenteEncontrado, 
														componenteNecessario, 
														configRecursos, 
														quantidadeComponentesNecessaria, 
														sessao.receitaSelecionada.receita.quantidadeOpicionais
													);
	AcrescentaComponenteEscolhido(
									sessao.receitaSelecionada.esquemaSelecionado,
									componenteNecessario,
									componenteEscolhidoMontado, 
									indiceEncaixeComponente);
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
				componenteEscolhido.tier);

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

		if(tier.valor <= PegaComponenteTierDesc(COMPONENTE_TIERS::MEDIOCRE).valor)
			esquemaSelecionado.qualidade := -1;
			!hasAntigo ? cache.insert(indice,esquemaSelecionado.qualidade) : cache[indice] := esquemaSelecionado.qualidade;
		else
			if(tier.valor >= PegaComponenteTierDesc(COMPONENTE_TIERS::EXCEPCIONAL).valor  && qualidadeAntiga != 1) // Se não tem nenhum componente mediocre, mas tem algum exepcional. considere o item excepcional.
				esquemaSelecionado.qualidade := 1;
				!hasAntigo ? cache.insert(indice,esquemaSelecionado.qualidade) : cache[indice] := esquemaSelecionado.qualidade;
			else
				if(tier.valor < PegaComponenteTierDesc(COMPONENTE_TIERS::EXCEPCIONAL).valorL  && qualidadeAntiga == -1)
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

function MontaComponenteEscolhido(componenteEncontrado, componenteNecessario, configRecursos, quantidadeComponentesNecessaria, quantidadeOpicionais := 0)
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
													quantidadeOpicionais,
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
	
	foreach item in (EnumerateItemsInContainer(quem.backpack, ENUMERATE_ROOT_ONLY))
		Sleepms(2);
		foreach componenteNecessario in (receita.componentesNecessarios)
			Sleepms(2);
			if( CStr(item.objtype) in componenteNecessario.itens)
				if(!retorno.exists(componenteNecessario.encaixe))
					var envelopeRetorno := struct{ conjunto := dictionary, melhor := 0, indiceEncaixe := _componenteNecessario_iter};
					retorno.insert(componenteNecessario.encaixe, envelopeRetorno);

				endif

				if(!GetObjProperty(item,PROPNAME_ENCAIXE_COMPONENTE))
					continue;
				endif
				var chaveMat := GetObjProperty(item,PROPNAME_MATERIAL);
				var material := configRecursos[chaveMat];
				var tier := PegaComponenteTierDesc(GetObjProperty(item,PROPNAME_TIER_COMPONENTE));
				var chaveUnica := "{}{}{}".format(item.objtype, chaveMat, tier.id);
				var envelope ;

				if(!retorno[componenteNecessario.encaixe].conjunto.exists(chaveUnica))
					envelope := MontaEnvelopeComponenteEncontrado(item, chaveMat, material, tier);
					retorno[componenteNecessario.encaixe].conjunto.insert(chaveUnica, envelope);
				else
					envelope := retorno[componenteNecessario.encaixe].conjunto[chaveUnica];
					envelope.quantidade += item.amount;
				endif

				AtualizaMelhorComponente(retorno[componenteNecessario.encaixe],
										envelope,
										componenteNecessario.caracteristica, 
										componenteNecessario.qtdNecessaria);

				retorno[componenteNecessario.encaixe].conjunto.insert(chaveUnica, envelope);
			endif
		endforeach
	endforeach 

	return retorno;
endfunction

function MontaEnvelopeComponenteEncontrado(item, chaveMat, material, tier, quantidade := 0)
	return struct {
					itemRef := item, 
					chaveMaterial := chaveMat,
					material := material,
					tier := tier,
					quantidade := quantidade ? quantidade : item.amount
				};
endfunction

function AtualizaMelhorComponente(byref componentesEncontrados, envelopeNovo, caracteristica, qtdNecessaria)

	//TODO [futuras versoes] Talvez deixar o poder do atual já guardado, pra nao ter que avaliar toda vez.
	var poderAtual := AvaliaPoderComponente(componentesEncontrados.melhor.tier,
											componentesEncontrados.melhor.chaveMaterial,
											caracteristica,
											configRecursos);
											
	var poderNovo := AvaliaPoderComponente(envelopeNovo.tier,envelopeNovo.chaveMaterial, caracteristica, configRecursos);
	
	var envelopeMelhor := ComparaMelhorComponente(poderAtual, poderNovo, envelopeNovo, componentesEncontrados.melhor);

	if(envelopeMelhor.quantidade >= cint(qtdNecessaria))
		componentesEncontrados.melhor := envelopeMelhor;
	endif

endfunction

function ComparaMelhorComponente(poderAtual,poderNovo, envelopeNovo, envelopeVelho)
	if( poderAtual.poderContado < poderNovo.poderContado)
		return envelopeNovo;
	else
		if(poderAtual.poderContado == poderNovo.poderContado) //Empatou
			if(poderAtual.modificadorHp < poderNovo.poderContado) //Prioridade do desempate é o bônus no HP
				return envelopeNovo;
			else
				if(poderAtual.modificadorPeso < poderNovo.modificadorPeso) // Depois o bônus no peso
					return envelopeNovo;
				endif
			endif
		endif
	endif
	return envelopeVelho;
endfunction

function AvaliaPoderComponente(tier, chaveMaterial, caracteristica, configRecursos, propriedades := 0)
	var retorno := struct;

	if(!propriedades)
		propriedades := MontaPropriedadesComponente(
									chaveMaterial,
									caracteristica,
									tier,
									configRecursos);

	endif
								
	retorno.+poderContado := propriedades.valorPrincipal + propriedades.valorTotalSubefeitos;
	retorno.+modPeso := propriedades.modificadorPeso;
	retorno.+modHp := propriedades.modificadorHp;

	return retorno;
endfunction

function MontaPropriedadesComponente(chaveMaterial, caracteristica ,tierProduto ,quantidadeComponentesNecessaria ,quantidadeOpicionais := 0, configRecursos := 0)

	if(!configRecursos)
		configRecursos := ReadConfigFile(":crafting:config/resourceList");
	endif

	var propriedades := struct {};
	propriedades.+caracteristica := caracteristica;
	var valorEfeitos := InterpretaMaterialEfeitos(caracteristica,chaveMaterial);

	var modificadorHp := GetConfigInt(configRecursos[chaveMaterial],"hpmod");
	var modificadorPeso := GetConfigInt(configRecursos[chaveMaterial],"weightMod");
	
	propriedades.+modificadorHp := modificadorHp ? modificadorHp : 0;

	if(modificadorPeso)
		propriedades.+modificadorPeso := Round(modificadorPeso / (quantidadeComponentesNecessaria - quantidadeOpicionais));
	else
		propriedades.+modificadorPeso := 0;
	endif

	if(valorEfeitos.efeitosAdicionais)
		propriedades.+efeitosAdicionais := valorEfeitos.efeitosAdicionais;
		propriedades.+valorTotalSubefeitos := valorEfeitos.valorTotalSubefeitos;
	endif

	if(TypeOfInt(valorEfeitos.valorPrincipal) != OT_UNINIT || TypeOfInt(valorEfeitos.valorPrincipal) != OT_ERROR)
		propriedades.+valorPrincipal := valorEfeitos.valorPrincipal;
		if(Find(Upper(caracteristica), "VAZIO",1) <= 0)
			propriedades.valorPrincipal := propriedades.valorPrincipal + Cint(tierProduto.valor);
		else
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

function MontaVisaoReceitas(quem,categorias,menuPericiaCfg,nomePericia, receitasConfig, byref propCraft, nomeProfissao, modoGm)

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
							nomeProfissao,
							modoGm))

				var receitaItem := MontaReceitaItem(quem, objtype, nomePericia, receitasConfig, propCraft, nomeProfissao, receitaClassificada);
				if(!receitaItem.isVisualizada) //Tenho que apagar a categoria das visualizadas, porque apareceu uma receita nova.
					var index := categoria in propCraft.pericias[nomePericia]["visualizadas"].categorias;
					propCraft.pericias[nomePericia]["visualizadas"].categorias.erase(index);
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
	var duracao := GetConfigString(receitasConfig[objtype],"DuracaoFabricacao");
	var quantidadeOpicionais := 0;

	if(TypeOf(definicaoItem) != "ConfigElemRef")
		definicaoItem := itemsConfig[("0x0"+(CStr(objtype) - "0x"))];
	endif

	receita.+componentesNecessarios := PegaComponentesNecessarios(objtype, receitasConfig, receitaClassificada.classe, quantidadeOpicionais);
	
	if(!receita.componentesNecessarios)
		return receita.componentesNecessarios;
	endif

	receita.+sucessosNecessarios := PegaDescDuracaoFabricacao(duracao).sucessos;
	receita.+materiaisNecessarios := PegaMateriaisNecessarios(definicaoFabricacao, receitaClassificada.classe);
	receita.+textoDescItem := PegaTextoDescItem(quem,objtype,definicaoItem,receita.componentesNecessarios, receita.materiaisNecessarios, duracao, definicaoFabricacao.lote);
	receita.+dificuldade := PegaDificuldadeFrabicarObjeto(quem,objtype,nomeProfissao,nomePericia, receitaClassificada.classe, receita.sucessosNecessarios);
	receita.+objtype := objtype;
	receita.+pericia := nomePericia;
	receita.+profissao := nomeProfissao;
	receita.+lote := definicaoFabricacao.lote ? definicaoFabricacao.lote : 1;
	receita.+uso := definicaoItem.Attribute;
	receita.+quantidadeOpicionais := quantidadeOpicionais;
	receita.+receitaClassificada := receitaClassificada;
	receita.+desc := definicaoItem.desc ? definicaoItem.desc: definicaoItem.name;
	receita.+graphic := definicaoItem.Graphic ? CInt(definicaoItem.Graphic) : CInt(objtype); //Normalmente o graphic do item é seu objtype, mas a preferência é do atributo graphic no itemdesc.
	receita.+isVisualizada:= PegaNomeReceitaClassificada(receitaClassificada) in (propCraft.pericias[nomePericia]["visualizadas"].receitas);

	if(definicaoFabricacao.color)
		receita.+corObrigatoria := definicaoFabricacao.color;
	endif
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

function PegaTextoDescItem(quem,objtype,definicaoItem,componentes, materiaisNecessarios, duracao, lote)

	var desc := struct;
	var labelQuantidade := "";
	
	if(lote > 0)
		labelQuantidade := lote + "x ";
	endif

	var nomeReceita := definicaoItem.desc == "pocao" ? definicaoItem.type : TruncatePlural(objtype);// adaptação pro sistema de poções antigo.
	
	desc := "<BASEFONT COLOR=#ffe242>" + labelQuantidade + nomeReceita + "</BASEFONT><br>";
	desc := MontaDescricaoItem(definicaoItem,quem,desc);
	
	if(materiaisNecessarios && materiaisNecessarios.size() > 0)
		desc +=  "<br>";
		desc +=  "<BASEFONT COLOR=#7bf8fe> Materiais Necessários</BASEFONT><br>";
		foreach material in materiaisNecessarios
			var materialInfo := SplitWords(material);
			desc +=  cint(materialInfo[2]) * lote + " " + GetMaterialName( materialInfo[1])  + "<br>";
			Sleepms(2);
		endforeach
	endif

	if(componentes  && componentes.size() > 0)
		desc +=  "<br>";
		desc +=  "<BASEFONT COLOR=#7bf8fe> Encaixes de Componente </BASEFONT><br>";
		foreach componente in componentes
			var descTemp := cint(componente.qtdNecessaria) * lote+ " " + componente.encaixeDesc;
			if(componente.isOpcional)
				descTemp := "[" + descTemp + "]";
			endif
			desc +=  descTemp  + "<br>";
			Sleepms(2);
		endforeach
	endif

	desc +=  "<br>";
	desc +=  "<BASEFONT COLOR=#7bf8fe> Tempo de Fabricação</BASEFONT><br>" + PegaDescDuracaoFabricacao(duracao).desc + "<br>";

	return desc;
endfunction

function PegaComponentesNecessarios(objtype, receitasConfig, classe, byref quantidadeOpicionais :=0)

	var receitaCfgElemento:= receitasConfig[objtype];

	if(TypeOfInt(receitaCfgElemento) == OT_ERROR)
		return  error{errortext := "Fabricação não encontrada para o item: " + CStr(objtype)};
	endif

	var encaixesItem := GetConfigStringArray(receitaCfgElemento,"ComponenteEncaixe");
	var configEncaixes := ReadConfigFile(":*:encaixes");
	classe := classe ? classe : 1;
	var quantidadePrincipal := 0;

	var componentes := array;
	foreach encaixe in encaixesItem
		Sleepms(2);
		var componente := struct;
		var splitEncaixe := SplitWords(encaixe);
		componente.+classe := Cint(splitEncaixe[DESC_ENCAIXE_CLASSE]);
		if(Cint(componente.classe) != cint(classe)) continue; endif

		componente.+caracteristica :=  splitEncaixe[DESC_ENCAIXE_CARACTERISTICA];
		componente.+encaixe := splitEncaixe[DESC_ENCAIXE_ENCAIXE];
		componente.+qtdNecessaria := splitEncaixe[DESC_ENCAIXE_QTD_NECESSARIA];

		case (splitEncaixe[DESC_ENCAIXE_CONDICIONAL])
			"principal": componente.+isPrincipal := splitEncaixe[DESC_ENCAIXE_CONDICIONAL] == "principal"; 
						quantidadePrincipal += 1;
						break;
			"opcional": componente.+isOpcional := splitEncaixe[DESC_ENCAIXE_CONDICIONAL] == "opcional"; 
						quantidadeOpicionais += 1; 
						break;
		endcase

		componente.+encaixeDesc := GetConfigString(configEncaixes[componente.encaixe],"Desc");
		componente.+itens := GetConfigStringArray(configEncaixes[componente.encaixe],"Item");
		componentes.append(componente);
	endforeach

	if(quantidadePrincipal == 0 && componentes.size() > 0)
		return Print("Erro [ " + objtype +  " ] Para receitas com componentes, o componente principal é obrigatório e não foi definido  " + CStr(Hex(objtype)));
	endif

	return componentes;
endfunction

function PegaMateriaisNecessarios(descritorFabricacao, classe)
	var materials := GetConfigStringArray(descritorFabricacao, "materials");
	var arrayRetorno := array;
	
	foreach material in materials
		Sleepms(2);
		var splitMaterial := SplitWords(material);
		var material, quantidade;

		if(splitMaterial.size() > 2)
			if(cint(splitMaterial[1]) != cint(classe)) continue; endif
			material :=  splitMaterial[2];
			quantidade := splitMaterial[3];
		else
			if(CLASSE_PADRAO != cint(classe)) continue; endif
			material :=  splitMaterial[1];
			quantidade := splitMaterial[2];
		endif

		arrayRetorno.append(material + " " + quantidade);
	endforeach

	return arrayRetorno;
endfunction

function DeveAparecerReceita(
								quem,
								povo,
								itemOrigem,
								receitaClassificada,
								objtype,
								nomePericia,
								receitasAprendidas,
								isDuplaOrigem,
								nomeProfissao,
								modoGm
							)
	
	var isOrigemValida := false;
	var isValorPericiaSuficiente := TemValorPericiaNecessaria(quem, objtype, receitaClassificada.classe, nomePericia, nomeProfissao );
	
	if(isDuplaOrigem)
		var nomeReceita1 := PegaNomeReceitaClassificada(receitaClassificada);
		isOrigemValida := nomeReceita1 in receitasAprendidas || povo == itemOrigem;
		if (!isOrigemValida && povo != itemOrigem)
			foreach aprendida1 in receitasAprendidas
				if (StrReplace(Lower(aprendida1), " ", "") + "-1" == nomeReceita1)
					isOrigemValida := true;
					break;
				endif
			endforeach
		endif
	else
		if(itemOrigem == "Recipe")
			var nomeReceita2 := PegaNomeReceitaClassificada(receitaClassificada);
			isOrigemValida := nomeReceita2 in receitasAprendidas;
			if (!isOrigemValida)
				foreach aprendida2 in receitasAprendidas
					if (StrReplace(Lower(aprendida2), " ", "") + "-1" == nomeReceita2)
						isOrigemValida := true;
						break;
					endif
				endforeach
			endif
		elseif(itemOrigem == "itemComum")
			isOrigemValida := true;
		else
			isOrigemValida := povo == itemOrigem;
		endif
	endif

	return (isOrigemValida && isValorPericiaSuficiente) || modoGm;
endfunction

function TemValorPericiaNecessaria(quem, objtype, classe, nomePericia, nomeProfissao)
	var settings := AP_GetSettingsCfgElem("Skills");

	return (  PegaSkillNecessaria(objtype, nomeProfissao, classe) - Cdbl(settings.TooHard) + FIXO_DIF_ADICIONAL_PERICIA) <= EffectiveSkill(quem, nomePericia);
endfunction

function CriaPropFabricadoGerada(
									objtype, 
									tier, 
									deveGerarComponentesOpcionais,
									listaDeMateriaisIgnorados, 
									modoEscolhaMaterial, 
									classe := 1
								)
	
	var esquemaAleatorio := GeraEsquemaAleatorio(objtype, 
									tier, 
									deveGerarComponentesOpcionais,
									listaDeMateriaisIgnorados, 
									modoEscolhaMaterial, 
									classe);

	return CriaNovaPropFabricado(struct{esquema := esquemaAleatorio.esquema}, ITEM_GERADO);

endfunction

function MontaProdutoMontadoDescriptor(quem,esquema,receita, byref structRetorno)
	var definicaoItem := itemsConfig[receita.objtype];
	var definicaoTileItem := IU_GetTileInfo( Hex(receita.graphic));

	structRetorno.+pesoFinal := 1;

	var modificador := esquema.modificadorPesoTotal ? (esquema.modificadorPesoTotal / 100) + 1 : structRetorno.pesoFinal;
	if(definicaoItem.weight)
		structRetorno.pesoFinal := structRetorno.pesoFinal * (definicaoItem.weight * modificador);
	else
		structRetorno.pesoFinal := structRetorno.pesoFinal * (definicaoTileItem.weight * modificador);
	endif

	var descriptor := GetItemDescriptor(CInt(OBJTYPE_ITEM_MONTADO));
	descriptor.color := receita.corObrigatoria ? receita.corObrigatoria : esquema.cor;
	descriptor.graphic := receita.graphic;
	descriptor.CProps.insert(PROPNAME_FABRICADO, DefinePropFabricado(esquema,receita, quem));
	descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG, true);

	return descriptor;
endfunction

function MontaProdutoMontadoAleatorio(
										objtype, 
										tier, 
										deveGerarComponentesOpcionais,
										listaDeMateriaisIgnorados, 
										modoEscolhaMaterial, 
										classe := 1
									)

	var esquemaInfo :=  GeraEsquemaAleatorio(objtype, 
										tier, 
										deveGerarComponentesOpcionais,
										listaDeMateriaisIgnorados, 
										modoEscolhaMaterial, 
										classe);

	return MontaProdutoMontadoDescriptor(0,esquemaInfo.esquema, esquemaInfo.receita, struct { serial := "Gerado"});
endfunction

function EncontraProfissao(objtype)
	var profissoes := GetConfigStringKeys( ReadConfigFile(PROFISSAO_CONFIG_PATH) );
	foreach profissaoChave in profissoes
		Sleepms(2);
		var profissaoConfig := ReadConfigFile(PROFISSAO_CONFIG_PATH)[profissaoChave];
		var fabricacaoDesc := ReadConfigFile(profissaoConfig.InfoFabricacao);
		if(fabricacaoDesc[objtype])
			return struct{ nome := profissaoChave, pericia := profissaoConfig.Attribute};
		endif
	endforeach

	return 0;
endfunction

function GeraEsquemaAleatorio(
									objtype, 
									tier, 
									deveGerarComponentesOpcionais,
									listaDeMateriaisIgnorados, 
									modoEscolhaMaterial, 
									classe := 1
								)
	var itemDescConfig := ReadConfigFile(":*:itemdesc");
	var fabricacaoDescConfig := ReadConfigFile(":*:fabricacaoDesc");
	var componenteDescConfig := ReadConfigFile(":*:componentes");
	var profissao := EncontraProfissao(objtype);
	var recursosConfig := ReadConfigFile(":crafting:config/resourceList");
	var objetoDesc := itemDescConfig[objtype];
	var janelaTier := SplitWords(tier, ">");
	var retorno := struct;
	tier := cint(tier);

	var receita := MontaReceitaItem(0,objtype,profissao.pericia,fabricacaoDescConfig,0,profissao.nome, struct{nome := objetoDesc.name, classe := classe});
	if(!receita)
		return receita;
	endif
	
	var componentesEscolhidos := dictionary;
	var objetoEsquema := NovoObjetoEsquemaSelecionado("",dictionary,0,"", classe);
	var tierInfo := PegaComponenteTierDesc(tier);

	Sleepms(50);
	foreach componenteNecessario in (receita.componentesNecessarios)
		Sleepms(5);
		if(janelaTier.size() > 1)
			tier := RandomIntMinMax(cint(janelaTier[1]), cint(janelaTier[2]) + 1);
			tierInfo := PegaComponenteTierDesc(tier);
		endif

		if(componenteNecessario.isOpcional && !deveGerarComponentesOpcionais) continue; endif

		var materialStruct, valorTier, materialNom, materialComponente;
		var componenteAleatorio := componenteNecessario.itens[RandomIntMinMax(1,componenteNecessario.itens.size() + 1)];
		var arrayMateriais := array;
		var tipoMaterial := componenteDescConfig[hex(componenteAleatorio)].principalMaterial;
		var materiaisPossiveis :=  LISTA_TIPO_MATERIAL[UPPER(tipoMaterial)];
		var materiaisPossiveisPorTier := dictionary;

		foreach materialPossivel in materiaisPossiveis
			sleepms(5);
			if(cint(materialPossivel.chave)  in listaDeMateriaisIgnorados)
				continue;	
			endif
			//Print("Analisando material: " + recursosConfig[hex(materialPossivel.chave)].name + " para componente: " + componenteDescConfig[hex(componenteAleatorio)].name);
			if(Find(UPPER(componenteNecessario.caracteristica),"VAZIO",1) > 0 )
				
				var chaveValorTier := PegaComponenteTierDesc(cint(recursosConfig[materialPossivel.chave].tier)).valor;
				materiaisPossiveisPorTier.insert(
													chaveValorTier,
													struct{
															materialComponente := materialPossivel.chave,
															materialNome := recursosConfig[hex(materialPossivel.chave)].name
														}
												);
			else
				var efeitosmaterial := InterpretaMaterialEfeitos(componenteNecessario.caracteristica, materialPossivel.chave);
				var valorTierMaterial := efeitosmaterial.valorPrincipal + efeitosmaterial.valorTotalSubefeitos;
				var tiers :=  PegaComponenteTiers();
				var chaveValorTier := valorTierMaterial > tiers[tiers.size()] ? tiers[tiers.size()] : valorTierMaterial;
				//Print("Material: " + recursosConfig[hex(materialPossivel.chave)].name + " valorTierMaterial: " + valorTierMaterial + " chaveValorTier: " + chaveValorTier);
				if( DeveEscreverMaterial(materiaisPossiveisPorTier,chaveValorTier))
					materiaisPossiveisPorTier.insert(
														chaveValorTier,
														struct{
																materialComponente := materialPossivel.chave,
																materialNome := recursosConfig[hex(materialPossivel.chave)].name
															}
													);
			
				endif
			endif
			
		endforeach

		if(modoEscolhaMaterial == FABRICACAO_MODO_MATERIAL::MESMO_TIER)
			valorTier := tierInfo.valor;
			materialStruct := materiaisPossiveisPorTier[valorTier];
		elseif(modoEscolhaMaterial == FABRICACAO_MODO_MATERIAL::MESMO_TIER_OU_MENOR)
			valorTier := PegaComponenteTierDesc(RandomIntMinMax(1,tier + 1)).valor;
			materialStruct := materiaisPossiveisPorTier[valorTier];
		elseif(modoEscolhaMaterial == FABRICACAO_MODO_MATERIAL::MENOR_QUE_TIER)
			var tierValidado := tier > 1 ? tier : 2;// achei melhor só reajustar ao invés de mandar msg de erro.
			valorTier := PegaComponenteTierDesc(RandomIntMinMax(1,tierValidado)).valor;
			materialStruct := materiaisPossiveisPorTier[valorTier];
		else
			return error{"errorText" := "parâmetro modoEscolhaMaterial inválido."};
		endif

		var opcaoEncontrada := struct{ 
				itemRef := struct{ objtype := componenteAleatorio, name := componenteDescConfig[hex(componenteAleatorio)].name},
				componenteTier :=  tierInfo,
				nomeMaterial := materialStruct.materialNome,
				chaveMaterial := hex(materialStruct.materialComponente),
				indiceEncaixeComponente := componenteNecessario.caracteristica};
		
		if(TypeOfInt(opcaoEncontrada.chaveMaterial) == OT_ERROR || TypeOfInt(opcaoEncontrada.chaveMaterial) == OT_UNINIT)
			return error{"errorText" := "Não foi possível encontrar material para este valor de tier [{}].".format(valorTier) };
		endif

		Sleepms(50);
		
		var componenteEscolhido := MontaComponenteEscolhido(
														opcaoEncontrada, 
														componenteNecessario, 
														recursosConfig, 
														receita.componentesNecessarios.size(), 
														receita.quantidadeOpicionais);

		AcrescentaComponenteEscolhido(
										objetoEsquema,
										componenteNecessario,
										componenteEscolhido, 
										opcaoEncontrada.indiceEncaixeComponente);
	endforeach

	retorno.+receita := receita;
	retorno.+esquema := objetoEsquema;

	return retorno;
endfunction

function DeveEscreverMaterial(materiaisPossiveisPorTier, tierMaterial)
	return !materiaisPossiveisPorTier.exists(tierMaterial) 
						|| ( materiaisPossiveisPorTier.exists(tierMaterial) 
							&& RandomIntMinMax(1,1) > 0 );
endfunction
