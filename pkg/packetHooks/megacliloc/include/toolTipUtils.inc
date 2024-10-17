use uo;
use cfgfile;

include ":equipsys:include/utils";

enum TT_CLASSE_PARAM
	DESCONHECIDO := 1,
	ITEM_ARMA := 2,
	TEMPLATE_ARMA := 3,
	ITEM_ARMADURA := 4,
	TEMPLATE_ARMADURA := 5
endenum
function ParseScriptString(crit)

	var str := "";
	var i := 1;

	while(crit[i] != "/")
		i := i + 1;
	endwhile
	i := i +1;
	while (crit[i])
		str := str + crit[i];
		i := i + 1;
	endwhile

	return str;

endfunction

//Função criada para pergar Prop de Objeto tanto na forma de item quanto de item template.
function PegaPropObjItemOrTemplate(item, propriedade)
	if(item.isA(POLCLASS_ITEM))
		return GetObjProperty(item, propriedade);
	else
		if(item.?Cprops)
			var prop := item.Cprops[propriedade];
			if(prop != error)
				return prop;
			endif
		endif
	endif

	return error{"errortext":="Property not found"};
endfunction

function MontaObjCfgArmaArmadura(objeto,xDesc := "")
	var retorno := struct;
	retorno.+cfg := ReadConfigFile(":*:itemdesc");
	retorno.+elem := FindConfigElem(retorno.cfg, objeto.objtype);
	retorno.+desc := xDesc;
	return retorno;
endfunction

function ClassificaParametro(objeto)
	if(objeto.isA(POLCLASS_WEAPON))
		return ITEM_ARMA;
	elseif(objeto.isA(POLCLASS_ARMOR))
		return ITEM_ARMADURA;
	elseif(objeto.TipoAtaque)
		return TEMPLATE_ARMA;
	elseif(objeto.TipoProtecao)
		return TEMPLATE_ARMADURA;
	else
		return DESCONHECIDO;
	endif
endfunction

function ClassificaDanoCor(tipo)
	case(tipo)
		ONEHANDED_LOW:
			return "<BASEFONT COLOR=#c2185b>"; //Tons de vermelho inicio
		TWOHANDED_LOW:
			return "<BASEFONT COLOR=#e91e63>"; 
		BOW_LOW:
			return "<BASEFONT COLOR=#c2185b>"; 
		CROSSBOW_LOW:
			return "<BASEFONT COLOR=#f06292>"; //Tons de vermelho fim
		ONEHANDED_MID:
			return "<BASEFONT COLOR=#afb42b>"; //Tons de beje inicio.
		TWOHANDED_MID:
			return "<BASEFONT COLOR=#cddc39>";
		BOW_MID:
			return "<BASEFONT COLOR=#afb42b>";
		CROSSBOW_MID:
			return "<BASEFONT COLOR=#dce775>"; //Tons de beje inicio.
		ONEHANDED_HIGH:
			return "<BASEFONT COLOR=#1976d2>"; //Tons de Azul inicio.
		TWOHANDED_HIGH:
			return "<BASEFONT COLOR=#03a9f4>";
		BOW_HIGH:
			return "<BASEFONT COLOR=#1976d2>";
		CROSSBOW_HIGH:
			return "<BASEFONT COLOR=#4fc3f7>";
	endcase
endfunction

function ClassificaResistCor(tipo)
	case(Upper(tipo))
		ARMOR_LOWLOW:
			return "<BASEFONT COLOR=#c2185b>"; //Tons de vermelho inicio
		ARMOR_LOWMID:
			return "<BASEFONT COLOR=#e91e63>"; 
		ARMOR_LOWHIGH:
			return "<BASEFONT COLOR=#f06292>"; 
		ARMOR_MIDLOW:
			return "<BASEFONT COLOR=#afb42b>"; //Tons de beje inicio.
		ARMOR_MIDMID:
			return "<BASEFONT COLOR=#cddc39>";
		ARMOR_MIDHIGH:
			return "<BASEFONT COLOR=#dce775>";
		ARMOR_HIGHLOW:
			return "<BASEFONT COLOR=#0288d1>";
		ARMOR_HIGHMID:
			return "<BASEFONT COLOR=#03a9f4>";
		ARMOR_HIGHHIGH:
			return "<BASEFONT COLOR=#4fc3f7>";
	endcase
endfunction
