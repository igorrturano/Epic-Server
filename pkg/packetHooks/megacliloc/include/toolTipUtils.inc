use uo;
use cfgfile;
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
