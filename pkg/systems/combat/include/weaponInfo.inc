/*
 * $Id: weaponInfo.inc 1041 2005-11-23 14:15:01Z austin $
 *
 */

use cfgfile;

include ":equipsys:props";

/*
 * CS_GetWeaponInfo(weapon, prop_name)
 *
 * Purpose
 * Retrieves a config property for a weapon.
 *
 * Parameters
 * weapon:	Weapon that is being looked up.
 * prop_name:	Itemdesc.cfg property to retrieve.
 *
 * Return value
 * Returns a string
 *
 */
function CS_GetWeaponInfo(weapon, prop_name)
	var config := ReadConfigFile(":*:itemdesc");
	config := Config[weapon.objtype];
	
	return GetConfigString(config, prop_name);
endfunction

/*
 * CS_GetAverageDamage(weapon)
 *
 * Purpose
 * Takes in the weapon's damage dice and damage modifier and
 * calculates its average damage.
 *
 * Parameters
 * weapon:	Weapon object to get the average damage for.
 *
 * Return value
 * Integer
 *
 */
function CS_GetAverageDamage(weapon)
	var dmg := CS_GetWeaponInfo(weapon, "Damage");
	
	dmg["d"] := " ";
	dmg["-"] := " ";
	dmg["+"] := " ";
	dmg := SplitWords(dmg);
	var X := CInt(dmg[1]);
	var Y := CInt(dmg[2]);
	var Z := CInt(dmg[3]);
	Z := Z+CInt((weapon).dmg_mod);
	// (x*((1+y)/2))+z average formula
	var average := (X*((1+Y)/2))+Z;
	
	return CInt(average);
endfunction

function getMaxDamages(weapon)
	var weaponAttr := weapon.getAttrBase();
	var dices := struct;
	dices.+SlashDamage := weaponAttr.SlashDamage;
	dices.+PiercingDamage := weaponAttr.PiercingDamage;
	dices.+BluntDamage := weaponAttr.BluntDamage;

	dices.SlashDamage := SplitWords(dices.SlashDamage, "d");
	dices.PiercingDamage := SplitWords(dices.PiercingDamage, "d");
	dices.BluntDamage := SplitWords(dices.BluntDamage, "d");

	dices.SlashDamage := dices.SlashDamage[1]*dices.SlashDamage[2];
	dices.PiercingDamage := dices.PiercingDamage[1]*dices.PiercingDamage[2];
	dices.BluntDamage := dices.BluntDamage[1]*dices.BluntDamage[2];

	return dices;
endfunction

function CheckBastardSword(who)
	if (who.weapon.objtype == 0x13b9 && !who.shield)
		return 1;
	endif
	return 0;
endfunction