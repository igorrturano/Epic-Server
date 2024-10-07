include "equipstats";

function setPropArmor(item)
	var stats := getArmorStats(item);
	setPropBase(item, stats);
endfunction

function setPropWeapon(item)
	var stats := getWeaponStats(item);
	var oldProp := getPropBase(item);
	if (oldProp.AttackType)
		stats.AttackType := oldProp.AttackType;
	endif
	setPropBase(item, stats);
endfunction

function getPropBase(item) //PEGA A PROP DE MODIFICADORES BASE
    var prop := item.GetProp("atributos");
    if (!prop) prop := struct; endif
    return prop;
endfunction

function getPropExcep(item) //PEGA A PROP DE MODIFICADORES EXCEPCIONAL
    var prop := item.GetProp("atributos_excp");
    if (!prop) prop := struct; endif
    return prop;
endfunction

function setPropBase(item, prop) //SETA A PROP DE MODIFICADORES BASE
    if (prop.Size() > 0) item.SetProp("atributos", prop); endif
endfunction

function setPropExcep(item, prop) //SETA A PROP DE MODIFICADORES EXCEPCIONAL
    if (prop.Size() > 0) item.SetProp("atributos_excp", prop); endif
endfunction