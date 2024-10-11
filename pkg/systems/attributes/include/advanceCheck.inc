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
    // Check if the stat cap is reached
    if (AP_CheckStatCap(mobile, stat_name))
        SendSysMessage(mobile, "You have reached the cap for " + stat_name + ".");
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
        SendSysMessage(mobile, "Your " + stat_name + " has increased by " + CStr(advance) + "!");

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
                SendSysMessage(owner, "Your pet's " + stat_name + " has increased by " + CStr(advance) + "!");
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

    // Special handling for Budōka (Monk) skill transformations (e.g. Wrestling)
    skill_name := HandleBudokaSkillTransform(mobile, skill_name);

    // Check for valid training skills (if training skills are enforced)
    if (!ValidTrainingSkill(mobile, skill_name)) 
        SkillDbgMsg(mobile, "Not part of training skills.");
        return 0;
    endif

    // Ensure the player has filled the Role-Play profile
    if (!GetObjProperty(mobile, "fichaRP")) 
        var chardata := GetObjProperty(mobile, "chardata");
        if (chardata != error) 
            SkillDbgMsg(mobile, "Nenhuma ficha Role-Play detectada.");
            SendSysMessage(mobile, "A evolução de skills do seu personagem está bloqueada até você preencher sua ficha de Role-Play!");
            return 0;
        endif
    endif

    // Verifica se o ganho de habilidade está desabilitado
    if (GetObjProperty(mobile, "NoGains")) 
        SkillDbgMsg(mobile, "O ganho de skill está desabilitado.");
        SendSysMessage(mobile, "O ganho de skill está desabilitado!");
        return 0;
    endif

    var true_skill := AP_GetTrueSkill(mobile, skill_name);
    var maximo := AP_CheckClassSkillCap(mobile, skill_name);
    if (true_skill >= maximo)  
        SkillDbgMsg(mobile, "Atingiu o limite máximo da skill");
        SendSysMessage(mobile, "Você atingiu o nível máximo da skill!");
        return 0;
    endif

    var settings := AP_GetSettingsCfgElem("Skills");

    // Verifica o tempo de espera entre ganhos de habilidade
    if (!SkillGainCooldownPassed(mobile, skill_name, settings)) 
        SendSysMessage(mobile, "Você precisa esperar um pouco mais antes de aumentar sua habilidade.");
        return 0;
    endif

    // Calcula a chance final ajustada com base na inteligência do personagem
    pass_chance := AdjustPassChance(pass_chance, settings, mobile);

    // Verifica se o avanço da habilidade ocorre (rolagem de dados)
    if (!PerformAdvancementCheck(pass_chance)) 
        SkillDbgMsg(mobile, "A verificação de avanço de habilidade falhou.");
        SendSysMessage(mobile, "Você falhou em aprimorar a skill " + skill_name + ".");
        return 0;
    endif

    // Free gain for skills below a certain threshold
    if (true_skill < CDbl(settings.FreeGainUntil)) 
        SkillDbgMsg(mobile, "Free Gain.");
    elseif (DifficultyTooEasy(true_skill, award_diff) && !(TemHabilidade(mobile, "Budōka"))) 
        // Skill too easy to increase, no advancement
        SendSysMessage(mobile, "Esta ação está muito fácil para você aprimorar a habilidade " + skill_name + ".");
        SkillDbgMsg(mobile, "Skill too easy to advance.");
        return 0;
    endif

    // Skill advancement success
    var temp := true_skill + 0.1;

    // Handle precision issue
    while (CInt(temp * 10.0) == CInt(GetAttributeBaseValue(mobile, skill_name)))
        temp := temp + 0.1;
        sleepms(10);
    endwhile

    // Save new skill value and notify player
    AP_SetTrueSkill(mobile, skill_name, temp);
    SetObjProperty(mobile, "#LastSkillGain", ReadGameClock());

    // If it's a pet, notify the owner
    if (mobile.npctemplate) 
        var owner := GetObjProperty(mobile, "owner");
        if (owner != error) 
            var petskills := GetObjProperty(mobile, "petskills");
            if (TypeOf(petskills) != "Dictionary") 
                petskills := dictionary;
            endif
            petskills[skill_name] := temp;
            setObjProperty(mobile, "petskills", petskills);
            SendSysMessage(owner, "A habilidade " + skill_name + " do seu pet aumentou para " + CStr(temp) + "!");
        endif
    else
        SendSysMessage(mobile, "Sua habilidade " + skill_name + " aumentou para " + CStr(temp) + "!");
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
        
        foreach skill in skillsArray
            if (skill == "One_Handed")
                return "One_Handed";
            elseif (skill == "Two_Handed")
                return "Two_Handed";
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
    
    if (skill_name == "Tactics" || skill_name == "Anatomy") 
        SkillDbgMsg(mobile, "Ignoring skill gain timer for Tactics or Anatomy.");
        return true;
    elseif (next_check > ReadGameClock()) 
        SkillDbgMsg(mobile, "Skill gain cooldown active.");
        return false;
    endif
    
    return true;
endfunction

/*
 * AdjustPassChance(pass_chance, settings, mobile)
 * 
 * Adjusts the pass chance for skill advancement based on intelligence
 * and ensures it falls within valid minimum and maximum ranges.
 */
function AdjustPassChance(pass_chance, settings, mobile)
    if (pass_chance <= settings.MinChance) 
        return CInt(settings.MinChance);
    else
        pass_chance := CInt(pass_chance * (1.0 + (AP_GetTrueStat(mobile, INTELLIGENCE) / 100.0)));
        if (pass_chance > settings.MaxChance) 
            pass_chance := CInt(settings.MaxChance);
        endif
    endif
    return pass_chance;
endfunction

/*
 * PerformAdvancementCheck(pass_chance)
 * 
 * Performs the dice roll to determine if the skill advancement occurs.
 */
function PerformAdvancementCheck(pass_chance)
    var adv_check := RandomInt(100) + 1; 
    return (adv_check <= pass_chance);
endfunction

//retorna quanto o cara comecou com aquela skill
function valorInicial(mobile, skill_name)
	var skill_inicial := 0.0;

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
    var tempoDevida := tempoAgora - criadoEm;
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

    var dias := CInt(idadeChar(who));
    var lim := 12;

    if (dias <= 10)
        // Dias 1-10: Travado em 12
        lim := 12;
    elseif (dias <= 30)
        // Dias 11-30: Aumento rápido (2 por dia)
        lim := 12 + (dias - 10) * 2;
    elseif (dias <= 60)
        // Dias 31-60: Aumento médio (1 por dia)
        lim := 52 + (dias - 30);
    else
        // Dias 61+: Aumento lento (0.5 por dia)
        lim := 82 + (dias - 60) * 0.5;
    endif

    return Min(lim, 100);
endfunction