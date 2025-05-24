include ":equipsys:utils";

print("INSTALLING: Methods for Weapon -> Equipment System... OK!");

/**
 * getMainDamageDice(item)
 *
 * Purpose
 * Buscar o dado do maindamage favorito da arma.
 *
 * Parameters
 * item: itemRef a ser verificado.
 *
 * Return value
 *
 */
exported function getMainDamageDice(item)
	var prop := item.getAttrBase();

	if (Upper(prop.AttackType) == Upper(SLASHDAMAGE))
		return prop.SlashMainDamage;
	elseif (Upper(prop.AttackType) == Upper(PIERCINGDAMAGE))
		return prop.PiercingMainDamage;
	elseif (Upper(prop.AttackType) == Upper(BLUNTDAMAGE))
		return prop.BluntMainDamage;
	else
		return error{"errortext" := "No dice damage found!"};
	endif
endfunction

exported function getMainDamageMax(weapon)
	var weaponAttr := weapon.getAttrBase();
	var dices := struct;
	dices.+SlashDamage := weaponAttr.SlashMainDamage;
	dices.+PiercingDamage := weaponAttr.PiercingMainDamage;
	dices.+BluntDamage := weaponAttr.BluntMainDamage;

	dices.SlashDamage := SplitWords(dices.SlashDamage, "d");
	dices.PiercingDamage := SplitWords(dices.PiercingDamage, "d");
	dices.BluntDamage := SplitWords(dices.BluntDamage, "d");

	dices.SlashDamage := CInt(dices.SlashDamage[1])*CInt(dices.SlashDamage[2]);
	dices.PiercingDamage := CInt(dices.PiercingDamage[1])*CInt(dices.PiercingDamage[2]);
	dices.BluntDamage := CInt(dices.BluntDamage[1])*CInt(dices.BluntDamage[2]);

	return dices;
endfunction

/**
 * getOffDamageDice(item)
 *
 * Purpose
 * Buscar o dado do offdamage favorito da arma.
 *
 * Parameters
 * item: itemRef a ser verificado.
 *
 * Return value
 *
 */
exported function getOffDamageDice(item)
	var prop := item.getAttrBase();
	
	if (prop.AttackType.Upper() == SLASHDAMAGE.Upper())
		return prop.SlashOffDamage;
	elseif (prop.AttackType.Upper() == PIERCINGDAMAGE.Upper())
		return prop.PiercingOffDamage;
	elseif (prop.AttackType.Upper() == BLUNTDAMAGE.Upper())
		return prop.BluntOffDamage;
	else
		return error{"errortext" := "No dice damage found!"};
	endif
endfunction

exported function getOffDamageMax(weapon)
	var weaponAttr := weapon.getAttrBase();
	var dices := struct;
	dices.+SlashDamage := weaponAttr.SlashOffDamage;
	dices.+PiercingDamage := weaponAttr.PiercingOffDamage;
	dices.+BluntDamage := weaponAttr.BluntOffDamage;

	dices.SlashDamage := SplitWords(dices.SlashDamage, "d");
	dices.PiercingDamage := SplitWords(dices.PiercingDamage, "d");
	dices.BluntDamage := SplitWords(dices.BluntDamage, "d");

	dices.SlashDamage := CInt(dices.SlashDamage[1])*CInt(dices.SlashDamage[2]);
	dices.PiercingDamage := CInt(dices.PiercingDamage[1])*CInt(dices.PiercingDamage[2]);
	dices.BluntDamage := CInt(dices.BluntDamage[1])*CInt(dices.BluntDamage[2]);

	return dices;
endfunction

/**
 * getType(item)
 *
 * Purpose
 * Buscar o Tipo de Ataque (Postura) favorito da arma.
 *
 * Parameters
 * item: itemRef a ser verificado.
 *
 * Return value
 *
 */
exported function getType(item)
	var elem := getItemCfgElem(item);
	return elem.TipoAtaque ? elem.TipoAtaque : error{"errortext" := "The item have no Attack Type!"};
endfunction

/**
 * isDualWield(item)
 *
 * Purpose
 * Verifica se a arma é DualWield.
 *
 * Parameters
 * item: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isDualWield(item)
	var elem := getItemCfgElem(item);

	return Upper(elem.Attribute) == Upper(DUALWIELD);
endfunction

/**
 * isRanged(item)
 *
 * Purpose
 * Verifica se a arma é Ranged.
 *
 * Parameters
 * item: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isRanged(item)
	var elem := getItemCfgElem(item);

	return Upper(elem.Attribute) == Upper(RANGED);
endfunction

/**
 * isBow(item)
 *
 * Purpose
 * Verifica se a arma é Arco.
 *
 * Parameters
 * item: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isBow(item)
	var elem := getItemCfgElem(item);
	return elem.ProjectileType == 0xf3f;
endfunction

/**
 * isCrossBow(item)
 *
 * Purpose
 * Verifica se a arma é Crossbow.
 *
 * Parameters
 * item: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isCrossBow(item)
	var elem := getItemCfgElem(item);
	return elem.ProjectileType == 0x1bfb;
endfunction

/**
 * isThrowable(item)
 *
 * Purpose
 * Verifica se a arma é Arremessável.
 *
 * Parameters
 * item: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isThrowable(item)
	var elem := getItemCfgElem(item);
	return elem.Throwable != error;
endfunction