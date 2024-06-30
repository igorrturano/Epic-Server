
use uo;
use os;
use cfgfile;

include ":brainAI:npcUtil";
include ":attributes:attributes";

function FL_CalculateRawDamage(attacker, weapon)

	if (attacker.npctemplate)
		var npcelem := NPC_GetNPCConfig(attacker.npctemplate);
		return ( RandomDiceRoll(cstr(npcelem.AttackDamage)) );
	else
		var weaponcfg := ReadConfigFile(":*:itemdesc");
		var weaponelem := FindConfigElem(weaponcfg, weapon.objtype);
		return ( RandomDiceRoll(cstr(weaponelem.Damage)) + weapon.dmg_mod );
	endif

endfunction

function FL_CalculateFinalDamage(attacker, weapon, rawdamage)

	var tacticsDmg := Cint( (AP_GetSkill(attacker, TACTICS)/1.6) );
	var strDmg := Cint(AP_GetStat(attacker, STRENGTH)*0.3);
	var anatomyDmg := Cint( AP_GetSkill(attacker, ANATOMY)/2 ) + 5;
	var backstabDmg := 0;
	if (GetObjProperty(attacker, "backstab") == 1)
		backstabDmg := CInt(AP_GetSkill(attacker, BACKSTAB))*2;
	elseif (GetObjProperty(attacker, "backstab") == 2)
		backstabDmg := CInt(AP_GetSkill(attacker, BACKSTAB))*0.4;
	endif
	var lumberjackingDmg := 0;
	if (weapon.attribute == "Axe")
		lumberjackingDmg := Cint( AP_GetSkill(attacker, LUMBERJACKING)/5 );
	endif
	var totalDmg := tacticsDmg + strDmg + anatomyDmg + lumberjackingDmg + backstabDmg;
	
	return CINT (rawdamage + ((rawdamage*totalDmg)/100));


endfunction
