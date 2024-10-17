// ================================================================
// Explanation and Purpose
// ================================================================
//
// This file manages the **skill and stat advancement system** for 
// the POL server. It controls how players increase their **skills** 
// (such as Swordsmanship or Magic) and **stats** (such as Strength 
// or Intelligence) through gameplay.
//
// The main steps handled in this file are:
// 
// 1. **Checking Eligibility**: 
//    - Before stats or skills can increase, the system checks if 
//      the player is eligible for a gain. This includes verifying:
//      * Has enough time passed since the last gain (cooldown)?
//      * Has the player reached the cap for that stat or skill?
//
// 2. **Rolling for Gain**: 
//    - If the player is eligible, the system performs a random roll 
//      to determine whether they successfully gain a point in a stat 
//      or skill.
//
// 3. **Applying Gains**: 
//    - If the roll is successful, the stat or skill increases by a 
//      small amount (usually 0.1). 
//    - The new value is saved, and the player is notified with a 
//      feedback message.
//
// 4. **Displaying Progress**: 
//    - Whether the player succeeds or fails, they are given clear 
//      feedback in the following forms:
//      * **Progress**: The player is shown how close they are to the 
//        next stat/skill increase.
//      * **Cap Reached**: Players are notified when they reach the 
//        maximum possible level for a stat or skill.
//      * **Cooldown**: When players need to wait before gaining more 
//        stats or skills, they are informed.
//
// ================================================================
// Quick Explanation of Key Functions:
// ================================================================
//
// 1. **CheckStatsForSkill(mobile, skill_name)**:
//    - This function checks if the player's **stats** (such as 
//      strength or intelligence) should increase when they use a 
//      particular skill.
//
// 2. **CheckStatAdvancement(mobile, stat_name, stat_mult)**:
//    - This function handles increasing the stat value.
//
// 3. **CheckSkillAdvance(mobile, skill_name, award_diff, pass_chance)**:
//    - This function checks if the player's **skills** (such as 
//      Swordsmanship or Magic) should increase after using the skill.
//
// ================================================================
use uo;
use os;
use util;

include ":tn:characters";
include ":commands:debugsystem";

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
 * Does an advancement check on a stat.
 *
 * Parameters
 * mobile:	Mobile to do the stat advancement check on.
 * stat_name:	Name of the stat to do the advancement check on.
 * stat_mult:	Multiplier to effect the chance of a gain.
 *		(rolled dice from an attributes.cfg StatCheck entry)
 * Return value
 * Returns 1
 */
function CheckStatAdvancement(mobile, stat_name, stat_mult)
    if ( AP_CheckStatCap(mobile, stat_name) )
        DebugMsg(mobile, DEBUG_UP, "Voce atingiu o cap maximo para " + stat_name + ".");
		SkillDbgMsg(mobile, "Stat cap for stat '"+stat_name+"' reached. No advance." );
		return 0;
	elseif ( AP_CheckStatCap(mobile) )
        DebugMsg(mobile, DEBUG_UP, "Voce atingiu o cap maximo.");
		return 0;
	endif

    // Retrieve stat values and calculate chance of advancement
    var stat_cap := AP_GetStatCap(stat_name);
    var stat_value := AP_GetTrueStat(mobile, stat_name);
    var stat_left := stat_cap - stat_value; // Points until it is capped
    var settings := AP_GetSettingsCfgElem("Stats");
    stat_mult := stat_mult * CDbl(settings.GainSpeed);

    // Debugging output to trace the process
    SkillDbgMsg(mobile, "Stat Cap: " + stat_cap);
    SkillDbgMsg(mobile, "Stat Current: " + stat_value);
    SkillDbgMsg(mobile, "Left to cap: " + stat_left + " ->" + stat_cap + "-" + stat_value);

    // Calculate chance to advance the stat
    var chance := ((stat_left * stat_mult) / (stat_cap / 2));
    if (chance < 1.0)
        chance := 1.0; // Minimum chance to advance
    endif

    var random := RandomFloat(100.0); // Roll for advancement

    // Debugging output for the chance and random roll
    SkillDbgMsg(mobile, "Chance to advance: " + chance + "%");
    SkillDbgMsg(mobile, "Random Roll: " + random);

    if (random < chance)
        // Determine the stat increase amount (minimum 0.1)
        var advance := CDbl(0.1 + (0.1 * CDbl(CInt(chance / 12.5))));
        if (advance > stat_left) 
            advance := stat_left; // Do not exceed the stat cap
        elseif (advance < 0.1) 
            advance := 0.1; // Ensure at least 0.1 increase
        endif

        // Handle precision bug: Ensure the stat actually increases by 0.1
        var temp := stat_value + advance;
		while ( CInt(temp*10.0) == CInt(GetAttributeBaseValue(mobile, skill_name)) )
			// Kludge for an old core precision bug.
			// Makes sure it always goes up atleast 0.1.
			temp := temp+0.1;
			sleepms(10);
		endwhile


        // Save the new stat value and update last stat gain time
        SkillDbgMsg(mobile, "Passed. Advance " + stat_name + ": " + CStr(advance));
        AP_SetTrueStat(mobile, stat_name, temp);
        SetObjProperty(mobile, "#LastStatGain", ReadGameClock());
        DebugMsg(mobile, DEBUG_UP, "Your " + stat_name + " has increased by " + CStr(advance) + "!");

        // Handle pets: Save the skill in 'myskills' if the mobile is a pet
        if (GetObjProperty(mobile, "owner"))
            var myskills := GetObjProperty(mobile, "myskills");
            if (!myskills)
                myskills := dictionary;
            endif
            myskills[stat_name] := temp;
            SetObjProperty(mobile, "myskills", myskills);
            // Notify the owner if it's a pet
            var owner := GetObjProperty(mobile, "owner");
            if (owner)
                DebugMsg(owner, DEBUG_UP, "Your pet's " + stat_name + " has increased by " + CStr(advance) + "!");
            endif
        endif

        return 1; // Stat advancement successful
    endif
    // Debugging output FAIL, return 0
    SkillDbgMsg(mobile, "Failed. No advance.");
    return 0;
endfunction

/*
 * CheckSkillAdvance(mobile, skill_name, award_diff, pass_chance)
 *
 * Purpose
 * Does an advancement check on a skill.
 *
 * Parameters
 * mobile:        Mobile to do the skill advancement check on.
 * skill_name:    Name of the skill to check.
 * award_diff:    Difficulty to use for advancement award.
 * pass_chance:   Chance of getting a gain check.
 *
 * Return value
 * Returns 1 if skill advancement occurs, otherwise 0.
 */
function CheckSkillAdvance(mobile, skill_name, award_diff, pass_chance)
    // Enable skill debugging if not already set
    if (!g_skill_debug)
        g_skill_debug := (GetObjProperty(mobile, "SkillDebug") != error);
    endif

    if (!GetObjProperty(mobile, "fichaRP")) 
        var chardata := GetObjProperty(mobile, "chardata");
        if (chardata != error) 
            SkillDbgMsg(mobile, "Nenhuma ficha Role-Play detectada.");
            DebugMsg(mobile, DEBUG_UP, "A evolução de skills do seu personagem está bloqueada até você preencher sua ficha de Role-Play!");
            return 0;
        endif
    endif

    if (GetObjProperty(mobile, "NoGains")) 
        SkillDbgMsg(mobile, "O ganho de skill está desabilitado.");
        DebugMsg(mobile, DEBUG_UP, "O ganho de skill está desabilitado!");
        return 0;
    endif

    if (AP_CheckSkillCap(mobile, skill_name))
        DebugMsg(mobile, DEBUG_UP, "Você atingiu o cap máximo para " + skill_name + ".");
        SkillDbgMsg(mobile, "Skill cap for '" + skill_name + "' reached. No advance.");
        return 0;
    endif

    // Special handling for Budōka (Monk) skill transformations (e.g. Wrestling)
    skill_name := HandleBudokaSkillTransform(mobile, skill_name);

    // Check for valid training skills (if training skills are enforced)
    if (!ValidTrainingSkill(mobile, skill_name)) 
        SkillDbgMsg(mobile, "Not part of training skills.");
        return 0;
    endif

    var true_skill := AP_GetTrueSkill(mobile, skill_name);
    var max_skillcap_class := AP_CheckClassSkillCap(mobile, skill_name);
    if (true_skill >= max_skillcap_class)  
        SkillDbgMsg(mobile, "Atingiu o limite máximo da skill");
        DebugMsg(mobile, DEBUG_UP, "Você atingiu o nível máximo da skill!");
        return 0;
    endif

    // Verifica o tempo de espera entre ganhos de habilidade
    var settings := AP_GetSettingsCfgElem("Skills");
    if (!SkillGainCooldownPassed(mobile, skill_name, settings)) 
        DebugMsg(mobile, DEBUG_UP, "Você precisa esperar um pouco mais antes de aumentar sua habilidade.");
        return 0;
    endif

    var fail_count_skill := "#FailCount_" + skill_name;
    var fail_count := GetObjProperty(mobile, fail_count_skill);
    if (fail_count == error)
        fail_count := 0;
        SetObjProperty(mobile, fail_count_skill, fail_count); 
    endif

    if (!PerformAdvancementCheck(pass_chance, true_skill, max_skillcap_class)) 
        fail_count := fail_count + 1;
        SetObjProperty(mobile, fail_count_skill, fail_count);
        SkillDbgMsg(mobile, "A verificação de avanço de habilidade falhou.");
        DebugMsg(mobile, DEBUG_UP, "Você aplicou mais esforço em " + skill_name + ", mas ainda não aprimorou. Tentativas: (" + CStr(fail_count) + ")");
        return 0;
    endif

    // Free gain for skills below a certain threshold
    if (true_skill < CDbl(settings.FreeGainUntil)) 
        SkillDbgMsg(mobile, "Free Gain.");
    elseif (DifficultyTooEasy(true_skill, award_diff)) 
        DebugMsg(mobile, DEBUG_UP, "Esta ação está muito fácil para você aprimorar a habilidade " + skill_name + ".");
        SkillDbgMsg(mobile, "Skill too easy to advance.");
        return 0;
    endif

    // Save new skill value and notify player
    var skill_gain := ApplySkillGainModifiers(mobile, skill_name, true_skill);    
    // Handle precision issue
    while (CInt(skill_gain * 10.0) == CInt(GetAttributeBaseValue(mobile, skill_name)))
        skill_gain := skill_gain + 0.1;  
        sleepms(10);  
    endwhile
    AP_SetTrueSkill(mobile, skill_name, skill_gain);
    SetObjProperty(mobile, "#LastSkillGain", ReadGameClock());

    // If it's a pet, notify the owner
    if (mobile.npctemplate) 
        var owner := GetObjProperty(mobile, "owner");
        if (owner != error) 
            var petskills := GetObjProperty(mobile, "petskills");
            if (TypeOf(petskills) != "Dictionary") 
                petskills := dictionary;
            endif
            petskills[skill_name] := skill_gain;
            setObjProperty(mobile, "petskills", petskills);
            SetObjProperty(mobile, fail_count_skill, 0);
            DebugMsg(owner, DEBUG_UP, "A habilidade " + skill_name + " do seu pet aumentou para " + FormatRealToString(skill_gain, 4) + "!");
        endif
    else
        SetObjProperty(mobile, fail_count_skill, 0);
        DebugMsg(mobile, DEBUG_UP, "Sua habilidade " + skill_name + " aumentou para " + FormatRealToString(skill_gain, 4) + "!");
    endif

    return 1;
endfunction

/*
 * HandleBudokaSkillTransform(mobile, skill_name)
 * 
 * Transforms Wrestling skill into One-Handed or Two-Handed 
 * if the character has the "Budōka" (Monk) ability.
 */
function HandleBudokaSkillTransform(mobile, skill_name)
    if (TemHabilidade(mobile, "Budōka") && skill_name == "Wrestling")
        var chardata := GetObjProperty(mobile, "chardata");
        var skillsmap := chardata.Skills;
        var skillsArray := skillsmap.keys();
        //add dual wield
        foreach skill in skillsArray
            if (skill == "One_Handed")
                return "One_Handed";
            elseif (skill == "Two_Handed")
                return "Two_Handed";
            elseif (skill == "Dual_Wield")
                return "Dual_Wield";
            endif
        endforeach
    endif
    return skill_name;
endfunction

/*
 * ValidTrainingSkill(mobile, skill_name)
 * 
 * Ensures the skill is part of the character's valid training skills.
 */
function ValidTrainingSkill(mobile, skill_name)
    var trainingSkills := getObjProperty(mobile, "trainingSkills");
    if (trainingSkills != error && trainingSkills.size() > 0)
        return (skill_name in trainingSkills);
    endif
    return true;
endfunction

/*
 * SkillGainCooldownPassed(mobile, skill_name, settings)
 * 
 * Ensures enough time has passed since the last skill gain.
 */
function SkillGainCooldownPassed(mobile, skill_name, settings)
    var next_check := CInt(GetObjProperty(mobile, "#LastSkillGain")) + GetConfigInt(settings, "GainWait");
    
    if (next_check > ReadGameClock()) 
        SkillDbgMsg(mobile, "Skill gain cooldown active.");
        return false;
    endif
    
    return true;
endfunction

/*
 * PerformAdvancementCheck(pass_chance, true_skill, max_skillcap_class)
 * 
 * Performs the dice roll to determine if the skill advancement occurs.
 */
function PerformAdvancementCheck(pass_chance, true_skill, max_skillcap_class)
    if (CInt(pass_chance < 5))
        pass_chance := 5;
    endif
    var adjusted_pass_chance := ApplyProgressCurve(pass_chance, true_skill, max_skillcap_class);
    var adv_check := CInt(RandomInt(100) + 1); 
    return (CInt(adv_check <= adjusted_pass_chance));
endfunction

/*
 * ApplyProgressCurve(pass_chance, true_skill, max_skillcap_class)
 * 
 * Purpose:
 * Adjusts pass chances based on a progression model. 
*/
function ApplyProgressCurve(pass_chance, true_skill, max_skillcap_class)
    var skill_ratio := CInt((true_skill * 100) / max_skillcap_class);
    var skill_decrease_point := 50;
    if (skill_ratio <= skill_decrease_point)
        pass_chance := CInt(pass_chance + (skill_ratio * (100 - pass_chance)) / skill_decrease_point); // Progressive increase
    elseif (skill_ratio > 50 && skill_ratio <= 65)
        pass_chance := CInt(pass_chance - ((skill_ratio - skill_decrease_point) * (pass_chance - 50)) / 40); 
    elseif (skill_ratio > 65 && skill_ratio <= 70)
        pass_chance := CInt(pass_chance - ((skill_ratio - skill_decrease_point) * (pass_chance - 15)) / 40); 
        pass_chance := Min(pass_chance, 30);  
    elseif (skill_ratio > 70 && skill_ratio <= 75) 
        pass_chance := CInt(pass_chance - ((skill_ratio - skill_decrease_point) * (pass_chance - 15)) / 40); 
        pass_chance := Min(pass_chance, 15);
    elseif (skill_ratio > 75 && skill_ratio <= 80) 
        pass_chance := CInt(pass_chance - ((skill_ratio - skill_decrease_point) * (pass_chance - 15)) / 40); 
        pass_chance := Min(pass_chance, 10);
    elseif (skill_ratio > 80 && skill_ratio <= 85) 
        pass_chance := CInt(pass_chance - ((skill_ratio - skill_decrease_point) * (pass_chance - 15)) / 40); 
        pass_chance := Min(pass_chance, 5);
    elseif (skill_ratio > 85 && skill_ratio <= 90) 
        pass_chance := CInt(pass_chance - ((skill_ratio - skill_decrease_point) * (pass_chance - 15)) / 40); 
        pass_chance := Min(pass_chance, 3);
    elseif (skill_ratio > 90) 
        pass_chance := 1; //max 1
    else
        pass_chance := 5;
    endif

    pass_chance := ClampValue(pass_chance, 0, 100);
    return pass_chance;
endfunction

function ClampValue(value, min_value, max_value)
    if (value < min_value)
        return min_value;
    elseif (value > max_value)
        return max_value;
    endif
    return value;
endfunction

/*
 * ApplySkillGainModifiers(mobile, skill_name, true_skill)
 * 
 * Purpose:
 * This function calculates the amount of skill gain based on various factors, such as the player's current skill level,
 * predefined gain values, and potential future conditions (like being in a dangerous zone or specific skill restrictions).
 * 
 * Key Features:
 * - Default Skill Gain (`base_gain`): 0.1
 * - High Skill Gain (`high_skill_gain`): 0.3, which can be applied in special scenarios
 * - No Skill Gain (`no_skill_gain`): 0, this can be used in cases where skill advancement should not occur due
 *   to certain restrictions 
 * - Precision Handling: The function ensures that the skill gain is accurately applied, even when floating-point precision
 *   issues arise.
 * 
 * Parameters:
 * - `mobile`: The player or character object whose skill is being modified.
 * - `skill_name`: The name of the skill that is being advanced.
 * - `true_skill`: The current level of the skill that is being updated.
 * 
 * Returns:
 * - The final new skill value after applying the calculated skill gain and ensuring precision.
 * 
 * Future Modifications:
 * - This function is designed to be extended with additional conditions (e.g., special scenarios like sneaking in dangerous
 *   zones or preventing skill gains with certain conditions like animal handling restrictions).
 */
function ApplySkillGainModifiers(mobile, skill_name, true_skill)
    // Fixed values for skill gain
    var base_gain := 0.1;       
    var high_skill_gain := 0.3;
    var no_skill_gain := 0;   

    // Default to base skill gain for now (can be modified based on future conditions)
    var skill_gain := base_gain;

    // TODO: Add special case for sneaking in dangerous zones to apply high_skill_gain
    // TODO: Add special case for handle animal to potentially set no_skill_gain

    // If no skill gain is applied, return the current skill value immediately
    if (skill_gain == no_skill_gain)
        return true_skill;  
    endif

    var new_skill_value := true_skill + skill_gain;  
    return new_skill_value;
endfunction

function CalculateDaysInReal(days)// para 10000 deu 292 segundos 300
    var criadoEm := cint(days);
    var tempoAgora := polcore().systime;// segundos
    var tempoDevida:= tempoAgora - criadoEm;
    var dias := cint(tempoDevida/(86400));//24horas*60minutos*60segundos = 86400 segundos
    return dias;
endfunction