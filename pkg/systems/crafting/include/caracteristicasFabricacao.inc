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

function PegaSubValorFabricacao(item, caracteristica, subEfeitoAlvo)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	var retorno := ExecutaOperadorCaracteristica(
									caracteristica, 
									propFabricadoMontado.componentes[caracteristica].propriedades.efeitosAdicionais[subEfeitoAlvo].valor
								);
	
	return TypeOfInt(retorno) != OT_ERROR ? Cint(retorno): 0;

endfunction

function PegaSubValoresFabricacao(item, caracteristica)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	return propFabricadoMontado.componentes[caracteristica].propriedades.efeitosAdicionais; 
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
	var propFabricadoMontado;
	if( objeto.isA(POLCLASS_ITEM))
		propFabricadoMontado := GetObjProperty(objeto, PROPNAME_FABRICADO);
	elseif(objeto is @UObjectWrapper)
		propFabricadoMontado := objeto.GetProperty(PROPNAME_FABRICADO);
	else
		propFabricadoMontado := objeto.CProps[PROPNAME_FABRICADO];
	endif

	return propFabricadoMontado;
endfunction