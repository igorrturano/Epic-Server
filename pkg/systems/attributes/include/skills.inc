//$Id: skills.inc 848 2005-11-02 20:54:39Z austin $

/*===============================================================
* Current Version
* SKILLS.INC - v1.0
* Updated 9/24/2005 6:52PM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

include ":fls_core:fls_characters";
include ":charactercreation:habilidades";
include ":attributes:attributes_ex";
include "include/eventID";
use uo;

/*
 * AP_GetSkill(mobile, skill_name)
 *
 * Purpose
 * Retrieves the base value of the skill plus its mod value.
 *
 * Parameters
 * mobile:	Mobile reference to retrieve the points from.
 * skill_name:	Name of the skill to retrieve the points for.
 *
 * Return value
 * Returns an integer.
 *
 */
function AP_GetSkill(who, skill_name)
	var bonus_corrucao := 0;
	var arcane_thief_bonus := 0;
	if (skill_name == MAGICLORE)
		if (TemHabilidade(who, "Sensibilidade Arcana"))
			if ( 70 > cint((AP_GetTrueSkill(who, skill_name)+AP_GetSkillMod(who, skill_name))))
				return 70;
			endif
            arcane_thief_bonus := Cint((AP_GetTrueSkill(who, AWARENESS) + AP_GetSkillMod(who, AWARENESS)) / 2);
		endif
		if (TemHabilidade(who, "Corrupcao Constante"))
			bonus_corrucao := cInt(GetObjProperty(who, "corruption")/1000);
		endif
	endif
	return AP_GetTrueSkill(who, skill_name) + AP_GetSkillMod(who, skill_name) + bonus_corrucao + arcane_thief_bonus;
endfunction

/*
 * AP_GetTrueSkill(mobile, skill_name)
 *
 * Purpose
 * Retrieves the base value of the skill and turns it into a usable number.
 *
 * Parameters
 * mobile:	Mobile reference to retrieve the points from.
 * skill_name:	Name of the skill to retrieve the points for.
 *
 * Return value
 * Returns a double.
 *
 */
function AP_GetTrueSkill(who, skill_name)
	return CDbl(GetAttributeBaseValue(who, skill_name)) / 10.0;
endfunction

/*
 * AP_GetSkillMod(mobile, skill_name)
 *
 * Purpose
 * Retrieves the base value of the skill mod and turns it into a usable number.
 *
 * Parameters
 * mobile:	Mobile reference to retrieve the points from.
 * skill_name:	Name of the skill to retrieve the modifier points for.
 *
 * Return value
 * Returns a double.
 *
 */
function AP_GetSkillMod(who, skill_name)
	return CDbl(GetAttributeTemporaryMod(who, skill_name)) / 10.0;
endfunction

/*
 * AP_SetTrueSkill(mobile, skill_name, points)
 *
 * Purpose
 * Sets the value of the skill.
 * Example: AP_SetTrueSkill(mobile, "strength", 44.5)
 *
 * Parameters
 * mobile:	Mobile reference to set the points on.
 * skill_name:	Name of the skill to set the points on.
 * points:	Number of points to set the skill at.
 *
 * Return value
 * Returns a double.
 *
 */
function AP_SetTrueSkill(mobile, skill_name, points)
	SetAttributeBaseValue(mobile, skill_name, CInt(CDbl(points) * 10.0));
	RecalcVitals(mobile);
endfunction

/*
 * AP_SetSkillMod(mobile, skill_name, points)
 *
 * Purpose
 * Sets the modifier value for the skill.
 * Example: AP_SetSkillMod(mobile, "strength", -15.3)
 *
 * Parameters
 * mobile:	Mobile reference to set the modifier points on.
 * skill_name:	Name of the skill to set the modifier points on.
 * points:	Number of points to set the modifier skill at.
 *
 * Return value
 * Returns a double.
 *
 */
function AP_SetSkillMod(mobile, skill_name, points)
	var e := SetAttributeTemporaryMod(mobile, skill_name, CInt(CDbl(points) * 10.0));
	//if (e == error)
	//	sendsysmessage(mobile, "Erro: " + e);
	//endif
	RecalcVitals(mobile);
endfunction

/*
 * AP_ModifySkillMod(mobile, skill_name, points)
 *
 * Purpose
 * Modifies the modifier value for the skill.
 * Example: 	AP_SetStatMod(mobile, "alchemy", -15.3)
 *
 * Parameters
 * mobile:	Mobile reference to adjust the modifier points on.
 * stat_name:	Name of the skill to adjust the modifier points on.
 * points:	Number of points to adjust the modifier by.
 *
 * Return value
 * Returns the previous value
 *
 */
function AP_ModifySkillMod(mobile, stat_name, points)
	var orig_mod := AP_GetStatMod(mobile, stat_name);
	var new_mod := CDbl(points)+CDbl(orig_mod);

//	sendsysmessage(mobile, "nmod " + new_mod + " " + stat_name);
	AP_SetSkillMod(mobile, stat_name, new_mod);

	return orig_mod;
endfunction


/*
 * AP_SetSkillScript(attribute_name)
 *
 * Purpose
 * Gets the script that a skill uses.
 *
 * Parameters
 * attribute_name:	Name of the attribute to get the script for.
 *
 * Return value
 * Returns a string.
 *
 */
function AP_GetSkillScript(attribute_name)
	var cfg := AP_GetAttributeCfgElem(attribute_name);
	return cfg.Script;
endfunction

/*
 * AP_GetSkillStats(attribute_name)
 *
 * Purpose
 * Retrieves a dictionary from the attribute's config elem
 * Key is the stat name, inside is an array containing the settings for it.
 * dict["StatName"] => array{gain chance, award points dice}
 *
 * Parameters
 * attribute_name:	Name of the skill to retrieve stat settings for.
 *
 * Return value
 * Returns a dictionary.
 *
 */
function AP_GetSkillStats(attribute_name)
	var cfg_elem := AP_GetAttributeCfgElem(attribute_name);
	var stat_entries := GetConfigStringDictionary(cfg_elem, "StatCheck");
	foreach textline in (stat_entries)
		textline := SplitWords(textline);
		sleepms(2);
	endforeach

	return stat_entries;
endfunction

/*
 * AP_GetSkillCap(skill_name)
 *
 * Purpose
 * Gets the number a skill caps at.
 *
 * Parameters
 * skill_name: Name of the skill to check
 *
 * Return value
 * Returns a number or error
 *
 */
function AP_GetSkillCap(mobile, skill_name)
	var cfg_elem := AP_GetSettingsCfgElem("Skills");

	// Check if there is a cap for that particular skill.
	var cap_val := GetConfigReal(cfg_elem, skill_name);

	if ( !cap_val )
		// No skill specific cap so get the general one.
		cap_val := GetConfigReal(cfg_elem, "SkillCap");
	endif

	return cap_val;

	//useless code above, this is the FLS one:
	//return GetSkillCap(mobile, skill_name);

endfunction

/*
 * AP_CheckSkillCap(mobile, skill_name)
 *
 * Purpose
 * Checks if a skill is greater than or equal to its maximum value in settings.cfg
 * If no skill_name is passed, it will check the totals.
 *
 * Parameters
 * mobile:	Mobile to check the skills on
 * stat_name:	Optional parameter - name of the skill to check.
 *
 * Return value
 * Returns 1 if skill(s) >= to the cap
 *
 */
function AP_CheckSkillCap(mobile, skill_name:="")
	var cfg_elem := AP_GetSettingsCfgElem("Skills");

	if ( skill_name )
		var cap_val := AP_GetSkillCap(mobile, skill_name);

		if ( cap_val != error )
			return ( AP_GetTrueSkill(mobile, skill_name) >= cap_val );
		endif
	endif

	return 0;
endfunction


/*
 * AP_CheckSkillTotal(mobile)
 *
 * Purpose
 * Checks the total sum of all skills a mobile has.
 *
 * Parameters
 * mobile:	Mobile to check the skills on
 *
 * Return value
 * Returns Integer of the sum total of skill points
 *
 */
function AP_CheckSkillsTotal(mobile)

	var total := 0.0;
	foreach attribute_name in ( AP_ListAttributesByType("Skill") )
		total := total + AP_GetTrueSkill(mobile, attribute_name);
		sleepms(2);
	endforeach

	return total;

endfunction


//////////////////////////////////////////////////////////////////////////////////////////
//
//	AP_GetPlayerTotalSkillPoints( player )
//
//	Purpose:	Totals all of the attribute (skill) points for a player.
//
//	Parameter:	mobileref::player
//
//	Returns:	The aggregate of a player's skill points.
//
//	Created:	2022-10-08 18:44:06 by Yukiko
//
//////////////////////////////////////////////////////////////////////////////////////////
function AP_GetPlayerTotalSkillPoints( player )

	var attrib_names := AP_ListAttributesByType( "Skill" );
	var attrib_points := 0;
	foreach attrib_name in attrib_names
		attrib_points := attrib_points + GetAttributeBaseValue( player, attrib_name );
	endforeach
	return attrib_points;

endfunction

// The following function will replace SetBaseSkill
function SetBaseAttribute(who, skillid, skill)
  return SetBaseAttributeBaseValue(who, skillid, Cint(skill*10));
endfunction



//////////////////////////////////////////////////////
//
//  These are wrapper functions to suport function
// calls for scripts written prior to Austin's
// attributes package for Distro 0.98.
// Hopefully this will provide a transition phase
// and eventually lead to those scripts being
// rewritten to take advantage of direct calls
// to the proper functions.
//
//////////////////////////////////////////////////////
const CORE_SETTINGS_MIN_ATTRIBUTE  := 0;
const CORE_SETTINGS_MAX_ATTRIBUTE  := 60000;
const CORE_SETTINGS_MIN_VITAL      := 100;
const CORE_SETTINGS_MAX_VITAL      := 10000000;



/////////////////////////////////////////////////////////
//
// Function returns an array of the skill numbers
// from uoskills.cfg
//
/////////////////////////////////////////////////////////

function GetSkillIDs()

var uoskillsfile := ReadConfigFile(":attributes:uoskills");
return GetConfigIntKeys(uoskillsfile);

endfunction

function GetEffectiveSkill(who, skillid)
  return AP_GetSkill(who, AP_SkillIdToAttributeName(skillid));
endfunction

function GetBaseSkill(who, skillid)
  return Cint(AP_GetTrueSkill(who, AP_SkillIdToAttributeName(skillid)));
endfunction

function GetBaseSkillBaseValue(who, skillid)
  return AP_GetTrueSkill(who, AP_SkillIdToAttributeName(skillid) * 10);
endfunction

function GetTrueEffectiveSkill(who, skillid)
  var id   := AP_SkillIdToAttributeName(skillid);
  var base := GetAttributeBaseValue(who, id);
  var mod1 := GetAttributeTemporaryMod(who, id);
  var mod2 := GetAttributeIntrinsicMod(who, id);
  var true := Cdbl((base + mod1 + mod2)/ 10.0);
  if(true < 0.0)
    return 0.0;
  else
    return true;
  endif
endfunction

function SetBaseAttributeBaseValue(who, attribid, skill)
  var skill_tenth := Cint(skill*10);
  if(skill_tenth < CORE_SETTINGS_MIN_ATTRIBUTE)
    skill_tenth := CORE_SETTINGS_MIN_ATTRIBUTE;
  elseif(skill_tenth > CORE_SETTINGS_MAX_ATTRIBUTE)
    skill_tenth := CORE_SETTINGS_MAX_ATTRIBUTE;
  endif
  return SetAttributeBaseValue(who, attribid, Cint(skill_tenth));
endfunction

function SetBaseSkill(who, skillid, skill)
  return SetBaseSkillBaseValue(who, skillid, Cint(skill*10));
endfunction

function SetBaseSkillBaseValue(who, skillid, skill_tenth)
  if(skill_tenth < CORE_SETTINGS_MIN_ATTRIBUTE)
    skill_tenth := CORE_SETTINGS_MIN_ATTRIBUTE;
  elseif(skill_tenth > CORE_SETTINGS_MAX_ATTRIBUTE)
    skill_tenth := CORE_SETTINGS_MAX_ATTRIBUTE;
  endif
  return SetAttributeBaseValue(who, AP_SkillIdToAttributeName(skillid), Cint(skill_tenth));
endfunction

function ModifyAttributeTemporaryMod(who, attributeid, moda)
  return SetAttributeTemporaryMod(who, attributeid, Cint(GetAttributeTemporaryMod(who, attributeid)+ Cint(moda*10)));
endfunction

function ModifyAttributeTemporaryModBaseValue(who, attributeid, moda)
  return SetAttributeTemporaryMod(who, attributeid, Cint(GetAttributeTemporaryMod(who, attributeid)+ moda));
endfunction
