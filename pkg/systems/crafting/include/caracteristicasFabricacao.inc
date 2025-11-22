use uo;
use cfgfile;
include "/include/epicPropNames";
include ":crafting:include/util/fabricacaoConstants";

function PegaValorFabricacao(item, caracteristica)
	var propFabricadoMontado := GetObjProperty(item, PROPNAME_FABRICADO);
	var retorno := propFabricadoMontado.componentes[caracteristica].propriedades.valorPrincipal; 
	return TypeOfInt(retorno) != OT_ERROR ? Cint(retorno): 0;
endfunction

function PegaSubValorFabricacao(item, caracteristica, subEfeitoAlvo)
	var propFabricadoMontado := GetObjProperty(item, PROPNAME_FABRICADO);
	var retorno := propFabricadoMontado.componentes[caracteristica].propriedades.efeitosAdicionais[subEfeitoAlvo].valor;
	return TypeOfInt(retorno) != OT_ERROR ? Cint(retorno): 0;

endfunction

function PegaSubValoresFabricacao(item, caracteristica)
	var propFabricadoMontado := GetObjProperty(item, PROPNAME_FABRICADO);
	return propFabricadoMontado.componentes[caracteristica].propriedades.efeitosAdicionais; 
endfunction