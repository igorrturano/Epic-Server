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