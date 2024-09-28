use uo;
use cfgfile;

include "equipstats";
include "include/say";
include "include/client";

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