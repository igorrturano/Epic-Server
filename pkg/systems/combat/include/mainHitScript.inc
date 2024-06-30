use uo;
use os;
use basic;
use cfgfile;

include ":attributes:attributes";
include ":brainai:npcUtil";
include ":blood:blood";
include ":combat:settings";
include ":timedscripts:timedScripts";
include "include/client";
include "include/facings";
include "include/damage";
include ":tn:combat";
include ":tn:cooldown";
include ":fls_core:packets";
include "include/say";
include ":charactercreation:habilidades";
include "include/client";
include "include/sounds";
include ":spells:swordmage";
include ":taming:taming";
include ":charactercreation:resists";

function SkillChecks(attacker, defender, weapon, rawdamage)
	var skillUsedForCritical := TACTICS;
	if(GetObjProperty(attacker, "kit") == "Mago de Combate")
		skillUsedForCritical := MAGICLORE;
	elseif(GetObjProperty(attacker, "kit") == "Medico de Combate")
		skillUsedForCritical := MEDICINE;
	elseif(temHabilidade(attacker, "Lobo Solitario"))
		skillUsedForCritical := SURVIVAL;
	endif
	//Testa se é possivel ganhar skill com o NPC alvo
	var skillAttacker := AP_GetSkill(attacker, attacker.weapon.attribute);
    var skillDefender := AP_GetSkill(defender, defender.weapon.attribute);

	if (defender.npctemplate && attacker.connected)
		skillAttacker := AP_GetSkill(attacker, attacker.weapon.attribute);
		skillDefender := AP_GetSkill(defender, WRESTLING);
        if (attacker.weapon.objtype == 0x13b9 && !attacker.shield) // bastarda
            skillAttacker := AP_GetSkill(attacker, MELEECOMBAT2H);
        endif
        if (defender.weapon.objtype == 0x13b9 && !defender.shield)
		    skillDefender := AP_GetSkill(defender, MELEECOMBAT2H);
        endif
		if(TemHabilidade(attacker, "Maestria de Devastacao"))
			if(GetObjProperty(attacker.weapon, "swordmage") == attacker.serial)
			skillAttacker := AP_GetSkill(attacker, MELEECOMBAT1H);
			endif
		endif
		if(TemHabilidade(defender, "Maestria de Devastacao"))
			if(GetObjProperty(defender.weapon, "swordmage") == defender.serial)
			skillDefender := AP_GetSkill(defender,MELEECOMBAT1H);
			endif
		endif
	endif

	//se a skill do atacante for maior 10 pontos a mais que a do defesor, não upa
	if (skillAttacker > (skillDefender+10))
        //PrintText(attacker, "NAO UPA!");
		return 0;
	endif

	//Sendo o alvo um personagem jogador ou NPC com combate melhor que o atacante, permite o up de skill de combate
	var can_gain := 0;
	var next_gain := CInt(GetObjProperty(attacker, "#CS_GainTime"));
	if ( ReadGameClock() >= next_gain )
		if ( CS_GetSettingsCfgElem("General").PvPGains )
			can_gain := 1;
		elseif ( defender.IsA(POLCLASS_NPC) )
			can_gain := 1;
		endif
	endif

	if ( can_gain )
		var gain_wait := CInt(CS_GetSettingsCfgElem("General").GainWait);
		SetObjProperty(attacker, "#CS_GainTime", ReadGameClock()+gain_wait);
		//if ( rawdamage > 0 )
			if (attacker.weapon.objtype == 0x13b9)
				if (!attacker.shield)
					SkillCheck(attacker, MELEECOMBAT2H, -1);
				else
					SkillCheck(attacker, MELEECOMBAT1H, -1);
				endif
			else
				SkillCheck(attacker, weapon.attribute, -1);
			endif
			sleepms(50);
			SkillCheck(attacker, skillUsedForCritical, -1);
		//endif
	endif
endfunction
