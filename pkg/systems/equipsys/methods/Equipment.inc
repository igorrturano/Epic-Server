include ":equipsys:equipstats";

print("INSTALLING: Methods for Item -> Equipment System... OK!");

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
exported function getAttrBase(item)
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
exported function getAttrExcp(item) 
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
exported function setAttrBase(item, prop) // SETA A PROP DE MODIFICADORES BASE
	if (item.isA(POLCLASS_WEAPON)) 
		if (!prop) prop := getWeaponStats(item); endif
		// var oldProp := item.getAttrBase();
		// if (oldProp.AttackType) // Não modificar a postura, caso seja arma e já tenha
		// 	prop.AttackType := oldProp.AttackType;
		// endif
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
exported function setAttrExcep(item, prop) // SETA A PROP DE MODIFICADORES EXCEPCIONAL
    if (prop) item.SetProp("atributos_excp", prop); endif

	return prop;
endfunction
