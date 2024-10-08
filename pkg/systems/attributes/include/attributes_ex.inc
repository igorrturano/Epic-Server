//$Id: attributes_ex.inc 809 2005-11-01 22:11:38Z austin $

/*===============================================================
* Current Version
* ATTRIBUTES_EX.INC - v1.0
* Updated 9/20/2005 11:42AM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

include ":attributes:include/attributes";

/*
 * AP_SkillIdToAttributeName(skill_number)
 *
 * Purpose
 * Gets the name of the attribute that is associated with a skill number.
 * Skills.cfg -> Attributes.cfg
 *
 * Parameters
 * skill_number:	Skill ID to convert
 *
 * Return value
 * Returns a string.
 *
 */
function AP_SkillIdToAttributeName(skillid)
	if (skillid == 90)
		return "Alchemy";
	endif
	var cfg := AP_GetAttributesCfgFile();
	var keys := GetConfigStringKeys( cfg );
	foreach key in keys
		var elem := FindConfigElem( cfg, key );
		if (elem.SkillId == skillid)
			return key;
		endif
	endforeach
	return "";
endfunction

/*
 * AP_AttributeNameToSkillId(attribute_name)
 *
 * Purpose
 * Gets the skill number (ID) that an attribute name is associated with.
 * Attributes.cfg -> Skills.cfg
 *
 * Parameters
 * attribute_name:	Attribute name to convert.
 *
 * Return value
 * Returns an integer.
 *
 */
function AP_AttributeNameToSkillId(skill)
	if (skill == "Alchemy")
		return 90;
	endif
	var elem := AP_GetAttributeCfgElem(skill);
	var id := elem.SkillId;
	return id;
endfunction