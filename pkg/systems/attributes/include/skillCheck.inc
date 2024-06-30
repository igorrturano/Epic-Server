// $Id: skillCheck.inc 311 2005-09-29 22:09:23Z austin $

/*===============================================================
* Current Version
* SKILLCHECK.INC - v1.0
* Updated 9/24/2005 6:52PM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

use uo;
use os;
use util;
use cfgfile;

const ADV_SKILL		:= 0x1;
const ADV_STATS		:= 0x2;
const ADV_ALL		:= ADV_SKILL+ADV_STATS; // 3
const ADV_DISABLE	:= 0x4; // 4 - because the default is ADV_ALL.

var g_skill_debug	:= 0;

/*
 * SkillCheck(mobile, attrib_name, difficulty, award_points, advance_flags)
 *
 * Purpose
 * Performs a skill check on an attribute and performs skill and stat advancements.
 * <= 0 means the skill check failed > 0 means it was successful.
 *
 * Parameters
 * mobile:		Mobile reference to do the skill check for.
 * attribute_name:	Name of the attribute to perform the check on.
 * award_diff:		Advanced as if the difficulty were this value - defaults to difficulty.
 * advance_flags:	Controls the turning on/off of stat and skill advancements for the check.
 *
 * Return value
 * Returns > 0 if the skill check was successful
 *
 */
function SkillCheck(mobile, attrib_name, difficulty, award_diff:=0, advance_flags:=ADV_SKILL)
	if ( mobile.dead )
		SendSysMessage(mobile, "Voce nao pode usar skills nesse estado.");
		return -100;
	elseif ( GetObjProperty(mobile, "DisableSkills") )
		SendSysMessage(mobile, "Suas skills estao desabilitadas.");
		return -100;
	elseif ( GetObjProperty(mobile, "NoGains") )
		advance_flags := ADV_DISABLE;
	endif
    
	// Global skill-debug ( for SkillDbgMsg() )
	if (!g_skill_debug)
		g_skill_debug := (GetObjProperty(mobile, "SkillDebug") != error);
	endif

	var effective_skill := AP_GetTrueSkill(mobile, attrib_name);
    if (TemHabilidade(mobile, "Visao Verdadeira") && attrib_name == AWARENESS)
        effective_skill += AP_GetSkill(mobile, MAGICLORE) / 3;
    endif

    var chardata := GetObjProperty(mobile, "chardata");
    if (chardata.trait == "Oito olhos" && attrib_name == AWARENESS)
        effective_skill += 25;
    endif

	SkillDbgMsg(mobile, attrib_name+": "+effective_skill+"% (+"+AP_GetSkillMod(mobile, attrib_name)+")");

	// Difficulty checks
	if ( difficulty == -1 )
		// -1 = Check skill against its self.
		SkillDbgMsg(mobile, "Skill based check. Difficulty -1");
	else
		difficulty := CDbl(difficulty);
		SkillDbgMsg(mobile, "Difficulty: "+difficulty);
	endif

	var pass_chance := CalcSuccessPercent(mobile, attrib_name, difficulty, effective_skill);
	var check_roll := RandomFloat(100);
	var pass_success := (pass_chance - check_roll);
	SkillDbgMsg(mobile, "Chance: "+pass_chance+"%");
	SkillDbgMsg(mobile, "Roll: "+check_roll+"%");
	if ( difficulty == -1 )
		difficulty := effective_skill;
	endif
	if (!award_diff)
		award_diff := difficulty;
	endif

	var chanceUp := CInt((100 - pass_chance) / 3);
	if ( !(advance_flags & ADV_SKILL) )
		SkillDbgMsg(mobile, "Skill advance disabled.");
	else
		if (chanceUp > 50)
			chanceUp := 50;
		elseif (chanceUp < 5)
			chanceUp := 5;
		endif
		CheckSkillAdvance(mobile, attrib_name, award_diff, chanceUp);
	endif

	return pass_success;
endfunction

/*
 * CalcSuccessPerfect(mobile, attrib_name, difficulty, effective_skill)
 *
 * Purpose
 * Retrieves the percentage chance of success based on mobile's skill and the difficulty.
 *
 * Parameters
 *
 * Return value
 * Returns 1
 *
 */
function CalcSuccessPercent(mobile, attrib_name, difficulty, effective_skill)
	var settings := AP_GetSettingsCfgElem("Skills");
	effective_skill += AP_GetSkillMod(mobile, attrib_name);
	SkillDbgMsg(mobile, "Final Skill: "+effective_skill+"%");

	var percent := 0;
	if ( difficulty == -1 )
		var max_skill := AP_GetSkillCap(mobile, attrib_name);
		var neg1chance := CDbl(settings.MaxNeg1Chance);
		percent := CInt(neg1chance * effective_skill / max_skill);
	elseif ( DifficultyTooHard(effective_skill, difficulty) )
		percent := 0;
	elseif ( DifficultyTooEasy(effective_skill, difficulty) )
		percent := 100;
	else
		percent := CInt(50.0 + ((effective_skill - difficulty) * 2.5));
	endif

	if ( percent < settings.MinChance )
		percent := CInt(settings.MinChance);
	elseif ( percent > settings.MaxChance )
		percent := CInt(settings.MaxChance);
	endif

	return percent;

endfunction

function DifficultyTooEasy(effective_skill, difficulty)
	var settings := AP_GetSettingsCfgElem("Skills");
	return ( (effective_skill + Cdbl(settings.TooEasy)) > difficulty );
endfunction

function DifficultyTooHard(effective_skill, difficulty)
	var settings := AP_GetSettingsCfgElem("Skills");
	return ( (effective_skill + CDbl(settings.TooHard)) < difficulty );
endfunction

/*
 * SkillDbgMsg(mobile, message)
 *
 * Purpose
 * Checks to see if it should send a skill debug message to a mobile.
 *
 * Parameters
 * mobile:	Mobile to send the messages to.
 * message:	Debug message.
 *
 * Return value
 * Returns 1
 *
 */
function SkillDbgMsg(mobile, message)
	if ( g_skill_debug )
		SendSysMessage(mobile, "Debug: "+message);
	endif

	return 1;
endfunction
