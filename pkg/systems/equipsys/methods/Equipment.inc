include ":equipsys:Controller/ArmorController";

print("INSTALLING: Methods for Equipment -> Equipment System... OK!");

/**
 * Function(params)
 *
 * Purpose
 *
 * Parameters
 * params:
 *
 * Return value
 *
 */
exported function GetAttrBase(item)
    return item.GetProp("atributos");
endfunction

/**
 * Function(params)
 *
 * Purpose
 *
 * Parameters
 * params:
 *
 * Return value
 *
 */
exported function GetAttrExcp(item) 
    return item.GetProp("atributos_excp");
endfunction

/**
 * Function(params)
 *
 * Purpose
 * Definição de atributos base de um equipamento.
 *
 * Parameters
 * params:
 *
 * Return value
 *
 */
exported function SetAttrBase(item, prop)
	if (prop) item.SetProp("atributos", prop); endif

	return prop;
endfunction

/**
 * Function(params)
 *
 * Purpose
 * Definição de atributos excepcionais de um equipamento.
 *
 * Parameters
 * params:
 *
 * Return value
 *
 */
exported function SetAttrExcep(item, prop)
    if (prop) item.SetProp("atributos_excp", prop); endif

	return prop;
endfunction
