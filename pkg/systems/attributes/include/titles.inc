/*
 * $Id: titles.inc 891 2005-11-04 06:29:43Z muaddiblsd $
 *
 */

use uo;
use os;
use cfgfile;

include ":attributes:attributes";

/*
 * AP_GetLevelTitle(skill_level)
 *
 * Purpose
 * Gets the title for a particlar skill level.
 *
 * Parameters
 * skill_level:	Skill level of a skill. (e.g. 80% = 80)
 *
 * Return value
 * Returns a string on success.
 * Returns an error on failure.
 *
 */
function AP_GetLevelTitle(skill_level)
    //return skill_level;
	if (skill_level >= 110)
		return "Lendário";
	elseif (skill_level >= 100)
		return "Grão Mestre";
	elseif (skill_level >= 90)
		return "Mestre";
	elseif (skill_level >= 80)
		return "Experiente";
	elseif (skill_level >= 70)
		return "Competente";
	elseif (skill_level >= 60)
		return "Treinado";
	elseif (skill_level >= 50)
		return "Habilidoso";
	elseif (skill_level >= 40)
		return "Aprendiz";
	elseif (skill_level >= 30)
		return "Iniciante";
	elseif (skill_level >= 20)
		return "Inexperiente";
	elseif (skill_level >= 10)
		return "Incompetente";
	elseif (skill_level >= 5)
		return "Destreinado";
	else
		return "Inábil";
	endif
/*
	skill_level := Cdbl(skill_level);

	if ( skill_level < 0.0 )
		return "U SUK";
	elseif ( skill_level >= 150.0 )
		return "God Style";
	endif

	var title_cfg := ReadConfigFile(":attributes:titles");
	if ( !title_cfg )
		return title_cfg;
	endif

	var title_elem := title_cfg["LevelTitles"];
	if ( !title_elem )
		return title_elem;
	endif

	var levels_list := GetConfigStringDictionary(title_elem, "Level");

	foreach entry in ( levels_list )
		if ( skill_level < CDbl(entry) )
			return _entry_iter;
		endif
		sleepms(2);
	endforeach

	return error{"errortext":="Could not find skill title for that level."};
*/
endfunction

/*
 * AP_GetSkillTitle(skill_name)
 *
 * Purpose
 * Finds the title name of a skill.
 *
 * Parameters
 * skill_name:	Name of a skill from attributes.cfg
 *
 * Return value
 * Returns a string on success.
 * Returns an error on failure.
 *
 */
function AP_GetSkillTitle(skill_name)
	var elem_info := AP_GetAttributeCfgElem(skill_name);
	if ( !elem_info )
		return elem_info;
	endif

	if ( elem_info.Title)
		return elem_info.Title;
	else
		return error{"errortext":="Attribute does not have a 'Title' entry."};
	endif
endfunction
