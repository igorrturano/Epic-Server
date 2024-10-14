// Purpose:
// This script handles skill checks for a player (mobile), including:
// - Checking if the player is allowed to use skills.
// - Calculating success chances based on skill levels and difficulty.
// - Handling skill and stat advancement if applicable.
//
// check results:
// - > 0:  skill action succeeded
// - <= 0: skill action failed
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
 * - mobile:       The player or NPC using the skill.
 * - attrib_name:  Name of the skill or attribute to check.
 * - difficulty:   Difficulty level for the skill check.
 * - award_diff:   Optional. If specified, awards advancement as if difficulty is this value.
 * - advance_flags: Controls whether to advance skills, stats, or both (default: skill only).
 *
 * Returns > 0 if the skill check was successful
 */
function SkillCheck(mobile, attrib_name, difficulty, award_diff:=0, advance_flags:=ADV_SKILL)
	var validation_result := ValidateSkillUsage(mobile);
	if (validation_result == -100)
        return -100;  // Player can't use skills
    elseif (validation_result == ADV_DISABLE)
        advance_flags := ADV_DISABLE;  // Disable advancements if gains are blocked
    endif
    
	// Global skill-debug ( for SkillDbgMsg() )
	if (!g_skill_debug)
		g_skill_debug := (GetObjProperty(mobile, "SkillDebug") != error);
	endif

	var effective_skill := AP_GetTrueSkill(mobile, attrib_name);
	effective_skill := ApplySkillModifiers(mobile, attrib_name, effective_skill); //Modificadores de habilidade

    if (difficulty == -1)
        difficulty := effective_skill;
        SkillDbgMsg(mobile, "Difficulty set to effective skill level: " + difficulty);
    endif
    if (!award_diff)
        award_diff := difficulty;
        SkillDbgMsg(mobile, "Award difficulty set to " + award_diff);
    endif

	var success_chance := CalcSuccessPercent(mobile, attrib_name, difficulty, effective_skill);
	var advancement_chance := CInt((100 - success_chance) / 3);

	if ( !(advance_flags & ADV_SKILL) )
		SkillDbgMsg(mobile, "Skill advance disabled.");
	else
		if (advancement_chance > 50)
			advancement_chance := 50;
		elseif (advancement_chance < 5)
			advancement_chance := 5;
		endif
		CheckSkillAdvance(mobile, attrib_name, award_diff, advancement_chance);
	endif

	var random_roll := RandomFloat(100);
	var success_margin := (success_chance - random_roll);
	return success_margin ;
endfunction

/*
 * ValidateSkillUsage(mobile)
 *
 * Purpose:
 * Checks if the player (mobile) is allowed to use skills.
 */
function ValidateSkillUsage(mobile)
    if (mobile.dead)
        SendSysMessage(mobile, "You cannot use skills while dead.");
        return -100;  // Player is dead, can't use skills
    elseif (GetObjProperty(mobile, "DisableSkills"))
        SendSysMessage(mobile, "Your skills are currently disabled.");
        return -100;  // Skills are disabled for the player
    elseif (GetObjProperty(mobile, "NoGains"))
        return ADV_DISABLE;  // Gains are blocked, disable advancements
    endif

    return 0;  // Player is allowed to use skills
endfunction

/*
 * ApplySkillModifiers(mobile, attrib_name, base_skill)
 *
 * Purpose:
 * Apply specific modifiers to the player's skill level based on their traits or abilities.
 *
 */
function ApplySkillModifiers(mobile, attrib_name, base_skill)
    if (TemHabilidade(mobile, "Visao Verdadeira") && attrib_name == AWARENESS)
        base_skill += AP_GetSkill(mobile, MAGICLORE) / 3;  
    endif

    var chardata := GetObjProperty(mobile, "chardata");
    if (chardata.trait == "Oito olhos" && attrib_name == AWARENESS)
        base_skill += 10;  
    endif

    return base_skill + AP_GetSkillMod(mobile, attrib_name);  
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

    var base_chance := 0;
	var max_skill := 0;
	var neg1chance := 0;
	if ( difficulty == -1 )
		max_skill := AP_GetSkillCap(mobile, attrib_name);
		neg1chance := CDbl(settings.MaxNeg1Chance);
		base_chance := CInt(neg1chance * effective_skill / max_skill);
	elseif ( DifficultyTooHard(effective_skill, difficulty) )
		base_chance := 0;
	elseif ( DifficultyTooEasy(effective_skill, difficulty) )
		base_chance := 100;
	else
		base_chance := CInt(50.0 + ((effective_skill - difficulty) * 2.5));
	endif

	var pass_chance := AdjustForIntelligence(base_chance, mobile);
	pass_chance := ClampPassChance(pass_chance, settings);

	return pass_chance;

endfunction

function DifficultyTooEasy(effective_skill, difficulty)
	var settings := AP_GetSettingsCfgElem("Skills");
	return ( (effective_skill + Cdbl(settings.TooEasy)) > difficulty );
endfunction

function DifficultyTooHard(effective_skill, difficulty)
	var settings := AP_GetSettingsCfgElem("Skills");
	return ( (effective_skill + CDbl(settings.TooHard)) < difficulty );
endfunction

function AdjustForIntelligence(pass_chance, mobile)
    var intelligence := AP_GetTrueStat(mobile, INTELLIGENCE);
    // Apply intelligence adjustment, increasing pass chance
    return CInt(pass_chance * (1.0 + intelligence / 100.0));
endfunction

function ClampPassChance(pass_chance, settings)
    if (pass_chance < settings.MinChance)
        return CInt(settings.MinChance); //0
    elseif (pass_chance > settings.MaxChance)
        return CInt(settings.MaxChance); //100
	endif
    return pass_chance;
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
