use uo;
use cfgfile;

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
			var prop := item.Cprops.propriedade;
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