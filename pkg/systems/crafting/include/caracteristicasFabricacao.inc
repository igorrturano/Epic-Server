use uo;
use cfgfile;
include "/include/epicPropNames";
include ":crafting:include/util/fabricacaoConstants";
include ":wrappers:include/Objects/Model/UObjectWrapper";

function PegaValorFabricacao(item, caracteristica)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	var valorRetorno := ExecutaOperadorCaracteristica(
												caracteristica, 
												propFabricadoMontado.componentes[caracteristica].propriedades.valorPrincipal
											);
	
	return TypeOfInt(valorRetorno) != OT_ERROR ? Cint(valorRetorno): 0;
endfunction

// efeitosAdicionais e um dict {subEfeito: valorInteiro} (ver
// InterpretaMaterialEfeitos em fabricacaoUtil.inc) — indexa direto,
// sem .valor. Componente/efeito ausente -> acesso encadeado vira erro
// -> retorna 0.
function PegaSubValorFabricacao(item, caracteristica, subEfeitoAlvo)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	var retorno := ExecutaOperadorCaracteristica(
									caracteristica,
									propFabricadoMontado.componentes[caracteristica].propriedades.efeitosAdicionais[subEfeitoAlvo]
								);
	
	return TypeOfInt(retorno) != OT_ERROR ? Cint(retorno): 0;

endfunction

function PegaSubValoresFabricacao(item, caracteristica)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	return propFabricadoMontado.componentes[caracteristica].propriedades.efeitosAdicionais;
endfunction

// Le PROPNAME_FABRICADO UMA vez e devolve { principal, subs } da
// caracteristica. principal e cada subs[<sub>] ja vem resolvidos como
// int (0 em ausencia/erro/uninit). Hot path de combate: use isto em vez
// de varias chamadas PegaSubValorFabricacao, que releem a prop por chave.
// Chave de sub ausente -> Cint(uninit) == 0 no consumidor.
function PegaFabricacaoBundle(item, caracteristica)
	var prop := PegaPropFabricadoMontado(item);
	var props := prop.componentes[caracteristica].propriedades;

	var principal := ExecutaOperadorCaracteristica(caracteristica, props.valorPrincipal);
	principal := TypeOfInt(principal) != OT_ERROR ? Cint(principal) : 0;

	var subs := dictionary{};
	var subsRaw := props.efeitosAdicionais;
	if (TypeOfInt(subsRaw) != OT_ERROR && subsRaw)
		foreach valorSub in (subsRaw)
			var resolvido := ExecutaOperadorCaracteristica(caracteristica, valorSub);
			subs[_valorSub_iter] := TypeOfInt(resolvido) != OT_ERROR ? Cint(resolvido) : 0;
		endforeach
	endif

	return struct{ principal := principal, subs := subs };
endfunction

function ExecutaOperadorCaracteristica(caracteristica, valor)
	var caracteristicasConfig := ReadConfigFile(":crafting:config/caracteristicas");
	var elementoConfig := caracteristicasConfig[caracteristica];
	var valorTotal := valor;

	if(elementoConfig)
		var operador := elementoConfig["operador"];
		var valorOperacao := cint(elementoConfig["valor"]);
		
		case (lower(operador))
			"soma":
				valorTotal += valorOperacao;
			"subtracao":
				valorTotal -= valorOperacao;
			"divisao":
				if(valorOperacao != 0)
					valorTotal /= valorOperacao;
				endif
			"multiplicacao":
				valorTotal *= valorOperacao;
		endcase
	endif

	return valorTotal;
endfunction

/*
PegaComponenteFabricacao(item, caracteristica)

Retorno: 
struct{ 
	componenteNecessario: struct{ caracteristica , classe , encaixe, encaixeDesc , isPrincipal , itens , qtdNecessaria },
	item: struct{ nome, objtype}, 
	material: struct{ chave, nome }, //Chave é o objtype do material.
	poderAvaliado: struct{ modHp, modPeso, poderContado},
	propriedades = struct{ caracteristica , modificadorHp , valorPrincipal}, 
	tier = struct{ desc , valor } //Qualidade do componente
}
*/
function PegaComponenteFabricacao(item, caracteristica := 0)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	var retorno;
	if(caracteristica)
		retorno := propFabricadoMontado.componentes[caracteristica];
	else
		foreach chaveCaracteristica in (propFabricadoMontado.componentes.keys())
			Sleepms(2);
			if(propFabricadoMontado.componentes[chaveCaracteristica].componenteNecessario.isPrincipal)
				retorno := propFabricadoMontado.componentes[chaveCaracteristica];
				break;
			endif
		endforeach
	endif
	return retorno;
endfunction

function PegaPropFabricadoMontado(objeto)
	// Discrimina UObjectWrapper -> struct -> Item. Em struct,
	// objeto.isA(POLCLASS_ITEM) retorna error truthy (falsa positiva), por
	// isso OT_STRUCT vem antes do isA.
	if (objeto is @UObjectWrapper)
		return objeto.GetProperty(PROPNAME_FABRICADO);
	elseif (TypeOfInt(objeto) == OT_STRUCT)
		return objeto.CProps[PROPNAME_FABRICADO];
	elseif (objeto.isA(POLCLASS_ITEM))
		return GetObjProperty(objeto, PROPNAME_FABRICADO);
	endif
endfunction