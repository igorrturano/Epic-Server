use uo;
use cfgFile;
include "/include/epicPropNames";

enum COMPONENTES_CARACTERISTICA
	CARACTERISTICA_FABRICACAO_REDUCAOCRITICO := "reducaoCritico",
	CARACTERISTICA_FABRICACAO_DEFESA := "defesa",
	CARACTERISTICA_FABRICACAO_MOBILIDADE := "mobilidade",
	CARACTERISTICA_FABRICACAO_BONUSAPARAR := "bonusAparar",
	CARACTERISTICA_FABRICACAO_ATRASOAPARAR := "atrasoAparar",
	CARACTERISTICA_FABRICACAO_DANO := "dano", //TODO falta aplicar as Subcaracteristicas Corretamente.
	CARACTERISTICA_FABRICACAO_PRECISAO := "precisao",
	CARACTERISTICA_FABRICACAO_CRITICO := "critico",
	CARACTERISTICA_FABRICACAO_PRESTIGIO := "prestigio",
	CARACTERISTICA_FABRICACAO_RESISTENCIA := "resistencia",
	CARACTERISTICA_FABRICACAO_DURABILIDADE := "durabilidade",
	CARACTERISTICA_FABRICACAO_TRANCA := "tranca",
	CARACTERISTICA_FABRICACAO_RESISTENCIACLIMAFRIO := "resistenciaClimaFrio",
	CARACTERISTICA_FABRICACAO_RESISTENCIACLIMACALOR := "resistenciaClimaCalor",
	CARACTERISTICA_FABRICACAO_INTIMIDACAO := "intimidacao",
endenum

enum COMPONENTES_SUBCARACTERISTICA
	// dano e defesa * INICIO
	SUBCARACTERISTICA_FABRICACAO_CORTANTE := "cortante", 
	SUBCARACTERISTICA_FABRICACAO_PERFURANTE := "perfurante",
	SUBCARACTERISTICA_FABRICACAO_CONTUSAO := "contusao",
	// dano e defesa * FIM
endenum

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