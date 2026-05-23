use uo;
use cfgfile;

include ":equipsys:utils/consts";
include ":combat:Model/DamageTier";

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
// Retorna o valor da prop, ou error (intrínseco do POL) se não existir.
// Callers DEVEM checar `== error` antes de usar — error é truthy em eScript.
function PegaPropObjItemOrTemplate(item, propriedade)
	if(item.isA(POLCLASS_ITEM))
		return GetObjProperty(item, propriedade);
	elseif(item.?Cprops)
		return item.Cprops[propriedade];
	endif

	return error;
endfunction

function MontaObjCfgArmaArmadura(objeto,xDesc := "")
	var retorno := struct;

	retorno.+cfg := ReadConfigFile(":*:itemdesc");
	retorno.+elem := TypeOfInt(objeto) != OT_CONFIGELEMREF ? FindConfigElem(retorno.cfg, objeto.objtype) : objeto;
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
	elseif(objeto.ObjClass == "Weapon")
		return TEMPLATE_ARMA;
	elseif(objeto.TipoProtecao)
		return TEMPLATE_ARMADURA;
	elseif(objeto.ObjClass == "Armor")
		return TEMPLATE_ARMADURA;
	else
		return DESCONHECIDO;
	endif
endfunction

function ClassificaDanoCor(tipo)
	case(tipo)
		DAMAGE_TIER::ONEHANDED_LOW:
			return "<BASEFONT COLOR=#c2185b>"; //Tons de vermelho inicio
		DAMAGE_TIER::TWOHANDED_LOW:
			return "<BASEFONT COLOR=#e91e63>"; 
		DAMAGE_TIER::BOW_LOW:
			return "<BASEFONT COLOR=#c2185b>"; 
		DAMAGE_TIER::CROSSBOW_LOW:
			return "<BASEFONT COLOR=#f06292>"; //Tons de vermelho fim
		DAMAGE_TIER::ONEHANDED_MID:
			return "<BASEFONT COLOR=#afb42b>"; //Tons de beje inicio.
		DAMAGE_TIER::TWOHANDED_MID:
			return "<BASEFONT COLOR=#cddc39>";
		DAMAGE_TIER::BOW_MID:
			return "<BASEFONT COLOR=#afb42b>";
		DAMAGE_TIER::CROSSBOW_MID:
			return "<BASEFONT COLOR=#dce775>"; //Tons de beje inicio.
		DAMAGE_TIER::ONEHANDED_HIGH:
			return "<BASEFONT COLOR=#1976d2>"; //Tons de Azul inicio.
		DAMAGE_TIER::TWOHANDED_HIGH:
			return "<BASEFONT COLOR=#03a9f4>";
		DAMAGE_TIER::BOW_HIGH:
			return "<BASEFONT COLOR=#1976d2>";
		DAMAGE_TIER::CROSSBOW_HIGH:
			return "<BASEFONT COLOR=#4fc3f7>";
	endcase
endfunction

function ClassificaResistCor(tipo)
	case(tipo)
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
