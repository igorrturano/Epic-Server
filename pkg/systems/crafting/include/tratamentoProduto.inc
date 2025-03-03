use uo;
include "/include/epicPropNames";

enum EV_TRATAMENTO
	EV_FIM_CICLO,
	EV_GUMP_CANCELADO,
	EV_ITEM_QUEBROU,
	EV_ITEM_PRODUZIDO,
endenum

enum PROCESSOS_GERENCIADOS
	GUMP_INTERATIVO := "tratamentoGump",
	GUMP_STATICO:= "tratamentoGumpStatico",
	CICLO := "cicloTratamento",
	ANIMACAO := "animacao"
endenum

function PegaMaximoFalhas(item)
		var propFabricado := GetObjProperty(item,PROPNAME_FABRICADO);
		if(propFabricado)
			return cint(propFabricado.receita.sucessosNecessarios * 2);
		endif
		return 0;
endfunction

