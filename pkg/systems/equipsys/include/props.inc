use uo;
use cfgfile;

include "equipstats";
include "include/say";
include "include/client";

function setPropArmor(item, tier)
	var resists := getArmorStats(item, tier);
	setPropBase(item, resists);
endfunction

function setPropWeapon(item)
	var stats := getWeaponStats(item);
	var oldProp := getPropBase(item);
	if (oldProp.AttackType)
		stats.AttackType := oldProp.AttackType;
	endif
	setPropBase(item, stats);
endfunction