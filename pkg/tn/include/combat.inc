use uo;
use os;
use cfgfile;

include ":brainAI:npcUtil";
include ":attributes:attributes";

function CalculateRawDamage(attacker, weapon)

	if (attacker.npctemplate)
		var npctemplate := NPC_ParseTemplateName(attacker.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var dmg := cfg[npctemplate.template].AttackDamage;
		//printtextabove(attacker, "dmg dice " + dmg);
		return ( RandomDiceRoll(dmg) );
	else
		var weaponcfg := ReadConfigFile(":*:itemdesc");
		var weaponelem := FindConfigElem(weaponcfg, weapon.objtype);
		var dmg;
		if ( GetObjProperty(attacker, "metamorfose") != error)
      		dmg := RandomDiceRoll( GetObjProperty(attacker, "dicedmg"));
		else
    		dmg :=  RandomDiceRoll(cstr(weaponelem.Damage));
    	endif
		return (  dmg + weapon.dmg_mod );
	endif

endfunction
