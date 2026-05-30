use uo;
use os;
use cfgfile;

include ":attributes:attributes";

/*
 * AP_GetLevelTitle(skill_level)
 * Purpose: Gets the title for a particlar skill level.
 * Parameters
 * 	skill_level:	Skill level of a skill. (e.g. 80% = 80)
 * Return: a string on success.
 */
function AP_GetLevelTitle(skill_level)
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
