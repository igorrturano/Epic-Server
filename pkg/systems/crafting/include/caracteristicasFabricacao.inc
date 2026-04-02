use uo;
use cfgfile;
include "/include/epicPropNames";
include ":crafting:include/util/fabricacaoConstants";
include ":wrappers:include/Objects/Model/UObjectWrapper";

function PegaValorFabricacao(item, caracteristica)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	var retorno := propFabricadoMontado.componentes[caracteristica].propriedades.valorPrincipal; 
	return TypeOfInt(retorno) != OT_ERROR ? Cint(retorno): 0;
endfunction

function PegaSubValorFabricacao(item, caracteristica, subEfeitoAlvo)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	var retorno := propFabricadoMontado.componentes[caracteristica].propriedades.efeitosAdicionais[subEfeitoAlvo].valor;
	return TypeOfInt(retorno) != OT_ERROR ? Cint(retorno): 0;

endfunction

function PegaSubValoresFabricacao(item, caracteristica)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	return propFabricadoMontado.componentes[caracteristica].propriedades.efeitosAdicionais; 
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
function PegaComponenteFabricacao(item, caracteristica)
	var propFabricadoMontado := PegaPropFabricadoMontado(item);
	var retorno := propFabricadoMontado.componentes[caracteristica]; 
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