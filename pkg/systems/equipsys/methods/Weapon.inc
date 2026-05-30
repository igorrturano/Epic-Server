include ":equipsys:utils/consts";
include ":combat:Model/Damage";
include ":attributes:attributeConstants";

print("INSTALLING: Methods for Weapon -> Equipment System... OK!");

/**
 * getMainDamageDice(weapon)
 *
 * Purpose
 * Buscar o dado do maindamage favorito da arma.
 *
 * Parameters
 * weapon: itemRef a ser verificado.
 *
 * Return value
 *
 */
exported function getMainDamageDice(weapon)
	var prop := weapon.getAttrBase();

	if (Upper(prop.AttackType) == Upper(DAMAGE_TYPE::SLASH))
		return prop.SlashMainDamage;
	elseif (Upper(prop.AttackType) == Upper(DAMAGE_TYPE::PIERCING))
		return prop.PiercingMainDamage;
	elseif (Upper(prop.AttackType) == Upper(DAMAGE_TYPE::BLUNT))
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
 * getOffDamageDice(weapon)
 *
 * Purpose
 * Buscar o dado do offdamage favorito da arma.
 *
 * Parameters
 * weapon: itemRef a ser verificado.
 *
 * Return value
 *
 */
exported function getOffDamageDice(weapon)
	var prop := weapon.getAttrBase();
	
	if (prop.AttackType.Upper() == DAMAGE_TYPE::SLASH.Upper())
		return prop.SlashOffDamage;
	elseif (prop.AttackType.Upper() == DAMAGE_TYPE::PIERCING.Upper())
		return prop.PiercingOffDamage;
	elseif (prop.AttackType.Upper() == DAMAGE_TYPE::BLUNT.Upper())
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
 * getType(weapon)
 *
 * Purpose
 * Buscar o Tipo de Ataque (Postura) favorito da arma.
 *
 * Parameters
 * weapon: itemRef a ser verificado.
 *
 * Return value
 *
 */
exported function getType(weapon)
	var elem := weapon.getCfgElem();
	return elem.TipoAtaque ?: error{"errortext" := "The item have no Attack Type!"};
endfunction

/**
 * isDualWield(weapon)
 *
 * Purpose
 * Verifica se a arma é DualWield.
 *
 * Parameters
 * weapon: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isDualWield(weapon)
	var elem := weapon.getCfgElem();
	return Upper(elem.Attribute) == Upper(DUALWIELD);
endfunction

/**
 * isRanged(weapon)
 *
 * Purpose
 * Verifica se a arma é Ranged.
 *
 * Parameters
 * weapon: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isRanged(weapon)
	var elem := weapon.getCfgElem();
	return Upper(elem.Attribute) == Upper(RANGED);
endfunction

/**
 * isBow(weapon)
 *
 * Purpose
 * Verifica se a arma é Arco.
 *
 * Parameters
 * weapon: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isBow(weapon)
	var elem := weapon.getCfgElem();
	return elem.ProjectileType == 0xf3f;
endfunction

/**
 * isCrossBow(weapon)
 *
 * Purpose
 * Verifica se a arma é Crossbow.
 *
 * Parameters
 * weapon: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isCrossBow(weapon)
	var elem := weapon.getCfgElem();
	return elem.ProjectileType == 0x1bfb;
endfunction

/**
 * isThrowable(weapon)
 *
 * Purpose
 * Verifica se a arma é Arremessável.
 *
 * Parameters
 * weapon: itemRef a ser verificado.
 *
 * Return 1/0
 *
 */
exported function isThrowable(weapon)
	var elem := weapon.getCfgElem();
	return elem.Throwable != error;
endfunction

/**
 * IsTwoHandedWeapon(weapon)
 *
 * Verifica se uma arma é de duas mãos.
 *
 * @param weapon O item da arma ou referência para verificação
 * @return 1 se a arma for de duas mãos, 0 caso contrário
 */
exported function isTwoHanded(weapon)
	weapon := weapon.getCfgElem();
	return Upper(weapon.Attribute) == Upper(TWOHANDED);
endfunction

exported function isFireWeapon(weapon)
	var elem := weapon.getCfgElem();
	return elem.ProjectileType == 0x9090;
endfunction