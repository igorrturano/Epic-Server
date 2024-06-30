// $Id: advanceCheck.inc 386 2005-10-03 00:22:39Z muaddiblsd $

/*===============================================================
* Current Version
* ADVANCECHECK.INC - v1.0
* Updated 9/28/2005 8:29PM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

use uo;
use os;
use util;

include ":tn:characters";

/*
 * CheckStatsForSkill(mobile, skill_name)
 *
 * Purpose
 * Performs a stat check on all stats that are listed within a skill's attributes.cfg elem.
 *
 * Parameters
 * mobile:	Mobile to do the stat checks on.
 * skill_name:	Skill to get the stat advance list from.
 *
 * Return value
 * Returns 1
 *
 */
function CheckStatsForSkill(mobile, skill_name)
	var settings := AP_GetSettingsCfgElem("Stats");

	var next_check := CInt(GetObjProperty(mobile, "#LastStatGain"))+CInt(settings.GainWait);
	if ( next_check > ReadGameClock() )
		SkillDbgMsg(mobile, "Stat advance-wait timer has not yet expired.");
		return 0;
	endif

	foreach stat_entry in (AP_GetSkillStats(skill_name))
		var stat_name := _stat_entry_iter;
		var check_chance := stat_entry[1];
		var stat_mult := CDbl(RandomDiceRoll(stat_entry[2]));

		var roll := RandomInt(100)+1;

		if ( roll <= check_chance )
			SkillDbgMsg(mobile, "("+roll+" < "+check_chance+"%) Checking stat "+stat_name);
			CheckStatAdvancement(mobile, stat_name, stat_mult);
		else
			SkillDbgMsg(mobile, "("+roll+" > "+check_chance+"%) skipping stat check for "+stat_name);
		endif

		sleepms(10);
	endforeach

	return 1;
endfunction

/*
 * CheckStatAdvancement(mobile, stat_name, stat_mult)
 *
 * Purpose
 * Does an advancement check on a stat.
 *
 * Parameters
 * mobile:	Mobile to do the stat advancement check on.
 * stat_name:	Name of the stat to do the advancement check on.
 * stat_mult:	Multiplier to effect the chance of a gain.
 *		(rolled dice from an attributes.cfg StatCheck entry)
 *
 * Return value
 * Returns 1
 *
 */
function CheckStatAdvancement(mobile, stat_name, stat_mult)
	if ( AP_CheckStatCap(mobile, stat_name) )
		SkillDbgMsg(mobile, "Stat cap for stat '"+stat_name+"' reached. No advance." );
		return 0;
	elseif ( AP_CheckStatCap(mobile) )
		SkillDbgMsg(mobile, "Total stat cap reached. No advance." );
		return 0;
	endif
	var settings := AP_GetSettingsCfgElem("Stats");
	var stat_cap := AP_GetStatCap(stat_name);
	var stat_value := AP_GetTrueStat(mobile, stat_name);
	var stat_left := stat_cap - stat_value; // Points until it is capped
	stat_mult := stat_mult * CDbl(settings.GainSpeed);

	SkillDbgMsg(mobile, "Stat Cap: "+stat_cap);
	SkillDbgMsg(mobile, "Stat Current: "+stat_value);
	SkillDbgMsg(mobile, "Left to cap: "+stat_left+" ->"+stat_cap+"-"+stat_value);

	var chance := ((stat_left * stat_mult) / (stat_cap / 2));
	if ( chance < 1.0 )
		chance := 1.0;
	endif
	SkillDbgMsg(mobile, "Chance Stat: "+chance);
	SkillDbgMsg(mobile, "---");
	var random := RandomFloat(100.0);
	SkillDbgMsg(mobile, "Check %age: "+random);

	if ( random < chance )
		var advance := CDbl(0.1 + (0.1 * CDbl(CInt(chance / 12.5))));
		if ( advance > stat_left )
			advance := stat_left;
		elseif ( advance < 0.1 )
			advance := 0.1;
		endif

		var temp := stat_value+advance;
		while ( CInt(temp*10.0) == CInt(GetAttributeBaseValue(mobile, stat_name)) )
			// Kludge for an old core precision bug.
			// Makes sure it always goes up atleast 0.1.
			temp := temp+0.1;
			sleepms(10);
		endwhile

		SkillDbgMsg(mobile, "Passed. Advance "+stat_name+": " + CStr(advance));
		AP_SetTrueStat(mobile, stat_name, temp);
		SetObjProperty(mobile, "#LastStatGain", ReadGameClock());

		if (GetObjProperty(mobile, "owner")) //se for um pet salva a skill
			var myskills := GetObjProperty(npc, "myskills");
			if (!myskills)
				myskills := dictionary;
			endif
			myskills[stat_name] := temp;
			SetObjProperty(mobile, "myskills", myskills);
		endif

		return 1;
	endif

	SkillDbgMsg(mobile, "Failed. No advance.");
	return 0;
endfunction

/*
 * CheckSkillAdvance(mobile, stat_name, stat_mult)
 *
 * Purpose
 * Does an advancement check on a stat.
 *
 * Parameters
 * mobile:	Mobile to do the skill advancement check on.
 * skill_name:	Name of the skill to check.
 * award_diff:	Difficulty to use for advancement award.
 * pass_chance:	Chance of getting a gain check.
 *
 *
 * Return value
 * Returns 1
 *
 */
function CheckSkillAdvance(mobile, skill_name, award_diff, pass_chance)
	if (!g_skill_debug)
		g_skill_debug := (GetObjProperty(mobile, "SkillDebug") != error);
	endif

    if(TemHabilidade(mobile, "Artista Marcial") && skill_name == "Wrestling")
    	var chardata := GetObjProperty(mobile, "chardata");
        var skillsmap := chardata.Skills;
        var skillsArray := skillsmap.keys();
        foreach skill in skillsArray
            if (skill == "MeleeCombat1H")
                skill_name := "MeleeCombat1H";
                break;
            elseif (skill == "MeleeCombat2H")
                skill_name := "MeleeCombat2H";
                break;
            endif
        endforeach
    endif

	var trainingSkills := getObjProperty(mobile, "trainingSkills"); //se nao tiver skill pra subir
	if (trainingSkills != error)
		if (trainingSkills.size() > 0 )
			if (!(skill_name in trainingSkills))
				SkillDbgMsg(mobile,"nao ta no training skills");

				return;
			endif
		endif
	endif

    if (!GetObjProperty(mobile, "fichaRP"))
		var chardata := GetObjProperty(mobile, "chardata");
			if (chardata != error)
				SkillDbgMsg(mobile,"nao tem ficharp!");
				SendSysMessage(mobile, "FACA sua ficharp para poder upar!");
				return;
			endif
	endif

	if ( GetObjProperty(mobile, "NoGains") ) //se tiver sem upar skill
		SkillDbgMsg(mobile, "Flag NoGains setada.");
		return 0;
	endif


	var true_skill := AP_GetTrueSkill(mobile, skill_name);
	var maximo := AP_CheckClassSkillCap(mobile, skill_name);
	if ( true_skill >= maximo ) //verifica quanto pode subir com base na classe
		//printtextabove(mobile, "to no maximo da skill");
		SkillDbgMsg(mobile, "Chegou no maximo da classe: " + maximo);
		return 0;
	endif
/*
	var bonus_tempo := 0;
	if(GetObjProperty(mobile, "bonus_tempo"))
		bonus_tempo := bonus_tempo + 20;
	endif

	SendSysMessage(mobile, "TL: "+ limiteportempo(mobile)+ " VI " + valorinicial(mobile, skill_name) + "TS" + true_skill);
*/
 	if ( (limiteportempo(mobile) + valorinicial(mobile, skill_name))  <= true_skill )
  		SkillDbgMsg(mobile, "Chegou no maximo do limite por tempo: " + limiteportempo(mobile) + " idade do char  " + idadeChar( mobile) + " valor inicial "  + valorinicial(mobile, skill_name) + " da skill " + skill_name);
		return 0;
  	endif

	var settings := AP_GetSettingsCfgElem("Skills");
	//verifica se ja nao subiu skills em XX segundos
	var next_check := CInt(GetObjProperty(mobile, "#LastSkillGain"))+GetConfigInt(settings, "GainWait");
	if (skill_name == "Tactics" || skill_name == "Anatomy")
		SkillDbgMsg(mobile, "Ignore Timer.");
	elseif ( next_check > ReadGameClock() )
		SkillDbgMsg(mobile, "Nao pode subir skill ainda. Next: " + next_check + " Current: " + ReadGameClock()  );
		return 0;
	endif

	//calcula a chance de passar

	if ( pass_chance <= settings.MinChance )
		pass_chance := CInt(settings.MinChance);
	else
		//aumenta chance com base na inteligencia
		pass_chance := CInt(pass_chance * (1.0 + (AP_GetTrueStat(mobile, INTELLIGENCE)/100.0)));
	endif
	if ( pass_chance > settings.MaxChance )
		pass_chance := CInt(settings.MaxChance);
	endif

	var adv_check := RandomInt(100)+1;

	SkillDbgMsg(mobile, "Chance: "+pass_chance+"%");

	if ( true_skill < CDbl(settings.FreeGainUntil) )
		SkillDbgMsg(mobile, "Free Gain.");
	elseif ( DifficultyTooEasy(true_skill, award_diff)  && !(TemHabilidade(mobile, "Artista Marcial")))
		SkillDbgMsg(mobile, "Too Easy.");
		return 0;
	elseif ( adv_check > pass_chance )
		SkillDbgMsg(mobile, "Nao passou.");
		return 0;
	endif


	//ESSA PARADA É A BATERIA DE CAP
/*
	if ( !CheckVitalSkillCap(mobile, true_skill) ) //verifica se tem cap pro temmpo ainda
	//	printtextabove(mobile, "to sem cap");
		SkillDbgMsg(mobile, "SkillCap insuficiente.");
		return 0;
	endif
*/

	//printtextabove(mobile, "oi, minha skill subiu");

	//if (!PointsToIncrease(AP_GetTrueSkill(mobile, skill_name)))
	//	SkillDbgMsg(mobile, "Nao passou.");
	//	return 0;
	//endif

	//sobe a skill
	SkillDbgMsg(mobile, "Passou.");
	var temp := true_skill+0.1;
	//SendSysMessage(mobile, true_skill + "-> "+ temp);
	while ( CInt(temp*10.0) == CInt(GetAttributeBaseValue(mobile, skill_name)) )
		// Kludge for an old core precision bug.
		// Makes sure it always goes up atleast 0.1.
		temp := temp+0.1;
		sleepms(10);
	endwhile

	//guarda as skills dos npcs tamados
	if (mobile.npctemplate)
		if ( GetObjProperty(mobile, "owner") != error)
			var petskills := GetObjProperty(mobile, "petskills");
			if ( TypeOf(petskills) != "Dictionary")
				petskills := dictionary;
			endif
			petskills[skill_name] := temp;
			setObjProperty(mobile, "petskills", petskills);
		endif
	endif

	SetObjProperty(mobile, "#LastSkillGain", ReadGameClock());
	AP_SetTrueSkill(mobile, skill_name, temp);

	return 1;

endfunction


//retorna quanto o cara comecou com aquela skill
function valorInicial(mobile, skill_name)
	var skill_inicial := 0;

	if (mobile.npctemplate)
		return 100;
	endif

	var chardata := GetObjProperty(mobile, "chardata");
	if (chardata == error)
		return 0;
	endif

	var myskills := chardata.Skills;

	if ( myskills[skill_name] == error)
		return 0.0;
	elseif (myskills[skill_name] == 1)
		skill_inicial := skill_inicial + 0.0;
	elseif (myskills[skill_name] == 2)
		skill_inicial := skill_inicial + 20.0;
	elseif ( myskills[skill_name] == 3)
		skill_inicial := skill_inicial + 40.0;
	endif

	return skill_inicial;
endfunction

function AP_CheckClassSkillCap(mobile, skill_name)

	if (mobile.npctemplate)
		return 100;
	endif

	var chardata := GetObjProperty(mobile, "chardata");
	if (chardata == error)
		return 0;
	endif

	var myskills := chardata.Skills;

	if ( myskills[skill_name] == error)
		return 0;
	elseif (myskills[skill_name] == 1)
		if (chardata.vantagemracial == "Sabedoria dos Antigos")
			return 50.0;
		else
			return 40.0;
		endif
	elseif (myskills[skill_name] == 2)
		return 70.0;
	elseif ( myskills[skill_name] == 3)
		return 100.0;
	endif

endfunction

function GetIncreaseCapFactor(skill)

	if (skill < 20 )
		return 1;
	elseif (skill < 40 )
		return 2;
	elseif (skill < 60 )
		return 4;
	elseif (skill < 80)
		return 8;
	elseif (skill < 90)
		return 16;
	elseif (skill < 100)
		return 25;
	endif

endfunction

function CheckVitalSkillCap(mobile, skill)
	var qtd := costSkill(skill);
	if (qtd > 0)
		if (CInt(AP_GetVital(mobile, "SkillCap")) >= qtd )
			AP_ConsumeVital(mobile, "SkillCap", qtd);
		else
			return 0;
		endif
	endif

	return 1;
endfunction

function costSkill(value)
		return 1;
endfunction

function AwardPoints(who, skill, points)

	var skills := GetObjProperty(who, "skills");
	if (!skills)
		skills := dictionary;
	endif

	skills[skill] := skills[skill] + points;

	if ( skills[skill] >= GetNeededPoints(skill) )
		skills[skill] := 0;
		SetObjProperty(who, "skills", skills);
		return 1;
	endif

	SetObjProperty(who, "skills", skills);
	return 0;

endfunction


function PointsToIncrease(basevalue)

	if(basevalue < 100)
		return 10.0;
	elseif ( basevalue < 30 )
		return 100.0;
	elseif ( basevalue < 600 )
		return  500.0;
	elseif ( basevalue < 700 )
		return 1000;
	elseif ( basevalue < 800 )
		return 4000;
	elseif ( basevalue < 900 )
		return 16000;
	else
		return 32000;
	endif

endfunction

function idadeChar( character)// para 10000 deu 292 segundos 300
    var criadoEm := cint(GetObjProperty(character, "createdate"));
    var tempoAgora := polcore().systime;// segundos
    var tempoDevida:= tempoAgora - criadoEm;
    var dias := cint(tempoDevida/(86400));//24horas*60minutos*60segundos = 86400 segundos
    return dias;
endfunction

function CalculateDaysInReal(days)// para 10000 deu 292 segundos 300
    var criadoEm := cint(days);
    var tempoAgora := polcore().systime;// segundos
    var tempoDevida:= tempoAgora - criadoEm;
    var dias := cint(tempoDevida/(86400));//24horas*60minutos*60segundos = 86400 segundos
    return dias;
endfunction

function limiteportempo(who)
	if (who.npctemplate)
		return 100;
	endif

	var dias := CInt(idadeChar(who)); // Colocar aqui a prop pra pegar os dias do char
	var lim := 0;

	if(dias >= 0)
		if(dias > 11)
			if(dias > 67)
				if(dias > 92) //dias 93 a 126 => Primárias até 10 (Até level 20)
					lim := 50.0 + cdbl(dias-92) * 0.3;
				else //dias 68 a 92 => Primárias até 90 (Até level 16)
					lim := 40.0 + cdbl(dias-67) * 0.4;
				endif
			else // dias 13 a 67 => Primárias até 80 (Até level 13)
				lim := 12.0 + cdbl(dias-11) * 0.5;
			endif
		else //dias 1 a 12 => Primárias 40-52 (Até level 4)
			lim := 1.0 + cdbl(dias);
		endif
	endif
	//SendSysMessage(who,lim);
	return lim;

/*
   var lim := 6; // Valor inicial

   if (dias >= 5)
      if (dias > 75)
         if (dias > 125)
            lim := 50.0 + ((dias - 125) * 0.1);
         else
            lim := 40.0 + ((dias - 75) * 0.2);
         endif
      else
         lim := 6.0 + ((dias - 5) * 0.5);
      endif
   endif

   lim := lim + CDbl(FindAccount(who.acctname).getprop("nota"))*2.5; // Influencia de -10 -5 0 +5 +10 dependendo da nota RP


   if (lim < 2.5)
      lim := 2.5; // Valor minimo de 2.5
   endif
*/
 endfunction
