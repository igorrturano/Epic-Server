include ":equipsys:Controller/EquipmentController";

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
 *
 * Parameters
 * params:
 *
 * Return value
 *
 */
exported function SetAttrBase(item, prop) // SETA A PROP DE MODIFICADORES BASE
	if (item.isA(POLCLASS_WEAPON)) 
		if (!prop) prop := getWeaponStats(item); endif
	elseif (item.isA(POLCLASS_ARMOR))
		if (!prop) prop := getArmorStats(item); endif
	endif

	if (prop) item.SetProp("atributos", prop); endif

	return prop;
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
exported function SetAttrExcep(item, prop) // SETA A PROP DE MODIFICADORES EXCEPCIONAL
    if (prop) item.SetProp("atributos_excp", prop); endif

	return prop;
endfunction
