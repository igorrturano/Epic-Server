//$Id: attributes.inc 826 2005-11-02 09:23:08Z austin $

/*===============================================================
* Current Version
* ATTRIBUTES.INC - v1.0
* Updated 9/24/2005 6:52PM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

use uo;
use os;
use cfgfile;
use vitals;
use attributes;

include ":attributes:regen";
include ":attributes:settings";
include ":attributes:skills";
include ":attributes:stats";
include ":attributes:vitals";
include ":attributes:attributeConstants";
include ":attributes:skillCheck";
include ":attributes:advanceCheck";
include ":datafile:datafile";
// V3: TemHabilidade removed - use MobileWrapper.HasAbility(ABILITY_ID::X) instead

function GetSkillIdFromName(name)
	var cfg := AP_GetAttributesCfgFile();
	var elem := FindConfigElem(cfg, name);
	if (elem == error)
		return "elem not found: " + GetConfigStringKeys( cfg );
	endif
	var id := GetConfigInt(elem, "SkillId");
	if (id == error)
		return "id error";
	endif
	return id;

endfunction

function GetSkillNameFromId(id)
	var cfg := AP_GetAttributesCfgFile();
	var keys := GetConfigStringKeys( cfg );

	foreach  key in keys
		var elem := FindConfigElem(cfg, key);
		var skillid := GetConfigInt(elem, "SkillId");
		if (id == skillid )
			return key;
		endif
	endforeach

	return "";

endfunction

/* AP_GetAttributesCfgFile()
 * 
 * Purpose
 * Loads attributes.cfg
 *
 * Parameters
 *
 * Return value
 * Config file
 *
 */
function AP_GetAttributesCfgFile()
	var cfg_file := ReadConfigFile(":attributes:attributes");
	if ( cfg_file.errortext )
		SysLog("Error::AP_GetAttributesCfgFile() - Unable to open attributes.cfg ->"+cfg_file.errortext);
	endif

	return cfg_file;
endfunction

/*
 * AP_GetAttributeCfgElem(attribute_name, cfg_file)
 *
 * Purpose
 * Retrieves a config elem for an attribute in attributes.cfg
 *
 * Parameters
 * attribute_name:	Name of the attribute to get the elem of.
 * cfg_file:		Optional parameter - reference to a config already read in.
 *
 * Return value
 * Returns a config file elem.
 *
 */
function AP_GetAttributeCfgElem(attribute_name, byref cfg_file:=0)
	if ( !cfg_file )
		cfg_file := AP_GetAttributesCfgFile();
	endif
	
	var cfg_elem := cfg_file[attribute_name];
	if ( cfg_elem.errortext )
		SysLog("Error::AP_GetAttributeCfgElem() - unable to find cfg elem ["+attribute_name+"] ->"+cfg_elem.errortext);
	endif
	
	return cfg_elem;
endfunction

/* AP_GetAttributesCfgFile()
 * 
 * Purpose
 * Loads the attributes datafile.
 *
 * Parameters
 *
 * Return value
 * Config file
 *
 */
function AP_GetAttributesDataFile()
	var data_file := DFOpenDataFile(":attributes:attributes", DF_CREATE);
	if ( data_file.errortext )
		SysLog("Error::AP_GetAttributesDataFile() - Unable to open attributes.txt ->"+data_file.errortext);
	endif

	return data_file;
endfunction

/*
 * AP_GetAttributeDataElem(attribute_name, cfg_file)
 *
 * Purpose
 * Retrieves a datafile elem from the attributes package.
 *
 * Parameters
 * attribute_name:	Name of the elem to retrieve.
 *
 * Return value
 * Returns a datafile elem.
 *
 */
function AP_GetAttributeDataElem(elem_name)
	var data_file := AP_GetAttributesDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);
	
	if ( data_elem.errortext )
		SysLog("Error::AP_GetAttributeDataElem() - unable to find elem ["+elem_name+"] ->"+data_elem.errortext);
	endif
	
	return data_elem;
endfunction

/*
 * AP_ListAttributesByType(type)
 *
 * Purpose
 * Gets the name of all attributes of a certain type.
 *
 * Parameters
 * Type:	String containing the type of attributes to list.
 *
 * Return value
 * Returns an array
 *
 */
function AP_ListAttributesByType(type:="")
	if ( !type )
		var cfg_file := ReadConfigFile(":attributes:attributes");
		return GetConfigStringKeys(cfg_file);
	endif
	
	type := Lower(type);
	var data_elem := AP_GetAttributeDataElem("Categories");
	return data_elem.GetProp(type);
endfunction

function Wait(who, segundos)
	if ( GetObjProperty(who, "#IsWaiting") > ReadGameClock() )
		return 0; // Já está esperando
	else
		EraseObjProperty(who, "#IsWaiting");
	endif

	var x := who.x;
	var y := who.y;
	var hp := AP_GetVital(who, "Hits");
	SetObjProperty(who, "#IsWaiting", ReadGameClock()+segundos+1);
	while (segundos)
		sleep(1);
		if (x != who.x || y != who.y ||
		    AP_GetVital(who, "Hits") < hp ||
		    !CInt(GetObjProperty(who, "#IsWaiting")) ||
		    who.warmode ||
		    GetEquipmentByLayer(who, 0x01) ||
		    GetEquipmentByLayer(who, 0x02)
		   )
			EraseObjProperty(who, "#IsWaiting");
			return 0;
		endif
		segundos := segundos - 1;
	endwhile

	EraseObjProperty(who, "#IsWaiting");

	return 1;
endfunction

function maolivre(who, qtd, checkmount := 1)
    var rightHand := GetEquipmentByLayer(who, 0x01);
    var leftHand := GetEquipmentByLayer(who, 0x02);

    // Verifica montaria se necessário
    if (checkmount)
        var mounted := GetEquipmentByLayer(who, 0x19);
        if (mounted)
            SendSysMessage(who, "Voce nao pode fazer isto montado.");
            return 0;
        endif
    endif

    // Verifica se há itens nas mãos
    if (rightHand && leftHand)
        return 0;
    endif

    return 1;
endfunction

// Phase 4 rework helpers.
//
// ParseDieValue converts a chardata.dice string ("d4".."d12") into its
// numeric die size. Returns 0 for unrecognized input.
function ParseDieValue(dice_str)
	case (dice_str)
		"d4":  return 4;
		"d6":  return 6;
		"d8":  return 8;
		"d10": return 10;
		"d12": return 12;
	endcase
	return 0;
endfunction

// CountOddLevelsReached returns how many HP-gain levels (3, 5, 7, ..., 19)
// the character has reached. Level 1 = 0, Level 3 = 1, ..., Level 19 = 9.
// Level >= 19 saturates at 9 (no more HP grants past L19).
function CountOddLevelsReached(level)
	if (level < 3)
		return 0;
	endif
	if (level >= 19)
		return 9;
	endif
	return (level - 1) / 2;
endfunction

// HasNewSystemDice tells whether `who` is a path-rework character.
// Returns 1 if chardata.dice is set to a valid die string, 0 otherwise.
// Legacy characters (no chardata.dice) keep the old HP formula.
function HasNewSystemDice(who)
	var chardata := GetObjProperty(who, "chardata");
	if (chardata == error || !chardata)
		return 0;
	endif
	var d := chardata.dice;
	if (d == error || d == "" || !d)
		return 0;
	endif
	return ParseDieValue(d) > 0;
endfunction

// EffectiveSkill returns the maximum of a new-system split skill and its
// legacy parent, so crafting/skill code calling EffectiveSkill(who, "X")
// stays productive for both pre-migration legacy characters (who still
// have the legacy parent skill) and post-migration new characters (who
// have the specific split skill).
//
// Renames (Stealth↔Sneak, Theft↔Pickpocketing, Smithing↔Metal_Working,
// Carpentry↔Wood_Working, Perception↔Awareness) are handled at the cfg
// layer via `Alias` directives — they share a SkillId, so this helper is
// NOT needed for them. AP_GetSkill(who, "Stealth") and AP_GetSkill(who,
// "Sneak") already return the same value.
//
// For splits, callers should migrate `AP_GetSkill(who, <legacy>)` calls
// to `EffectiveSkill(who, <new>)` opportunistically. Once the player base
// has fully migrated (Phase 6), this helper collapses into AP_GetSkill.
function EffectiveSkill(who, skill_name)
	case (skill_name)
		"Provisioning":
			var cur := AP_GetSkill(who, "Provisioning");
			var legacy := AP_GetSkill(who, "Domestic_Work");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Tailoring":
			var cur := AP_GetSkill(who, "Tailoring");
			var legacy := AP_GetSkill(who, "Domestic_Work");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Mining":
			var cur := AP_GetSkill(who, "Mining");
			var legacy := AP_GetSkill(who, "Gathering");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Lumberjacking":
			var cur := AP_GetSkill(who, "Lumberjacking");
			var legacy := AP_GetSkill(who, "Gathering");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Taming":
			var cur := AP_GetSkill(who, "Taming");
			var legacy := AP_GetSkill(who, "Handle_Animal");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Beastmastery":
			var cur := AP_GetSkill(who, "Beastmastery");
			var legacy := AP_GetSkill(who, "Handle_Animal");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Stealth":
			var cur := AP_GetSkill(who, "Stealth");
			var legacy := AP_GetSkill(who, "Sneak");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Sneak":
			var cur := AP_GetSkill(who, "Sneak");
			var legacy := AP_GetSkill(who, "Stealth");
			if (cur >= legacy) return cur; endif
			return legacy;
	endcase
	return AP_GetSkill(who, skill_name);
endfunction

// EffectiveTrueSkill is the AP_GetTrueSkill-flavored mirror of EffectiveSkill.
// Used internally by SkillCheck so that migrating a callsite from a legacy
// split-parent skill name to a new-system split name keeps legacy chars
// (whose value lives under the parent name) productive while letting
// new-system chars (whose value lives under the new name) advance normally.
//
// For non-split names this is just AP_GetTrueSkill(who, name) — backward
// compatible with every existing SkillCheck call.
function EffectiveTrueSkill(who, skill_name)
	case (skill_name)
		"Provisioning":
			var cur := AP_GetTrueSkill(who, "Provisioning");
			var legacy := AP_GetTrueSkill(who, "Domestic_Work");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Tailoring":
			var cur := AP_GetTrueSkill(who, "Tailoring");
			var legacy := AP_GetTrueSkill(who, "Domestic_Work");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Mining":
			var cur := AP_GetTrueSkill(who, "Mining");
			var legacy := AP_GetTrueSkill(who, "Gathering");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Lumberjacking":
			var cur := AP_GetTrueSkill(who, "Lumberjacking");
			var legacy := AP_GetTrueSkill(who, "Gathering");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Taming":
			var cur := AP_GetTrueSkill(who, "Taming");
			var legacy := AP_GetTrueSkill(who, "Handle_Animal");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Beastmastery":
			var cur := AP_GetTrueSkill(who, "Beastmastery");
			var legacy := AP_GetTrueSkill(who, "Handle_Animal");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Stealth":
			var cur := AP_GetTrueSkill(who, "Stealth");
			var legacy := AP_GetTrueSkill(who, "Sneak");
			if (cur >= legacy) return cur; endif
			return legacy;
		"Sneak":
			var cur := AP_GetTrueSkill(who, "Sneak");
			var legacy := AP_GetTrueSkill(who, "Stealth");
			if (cur >= legacy) return cur; endif
			return legacy;
	endcase
	return AP_GetTrueSkill(who, skill_name);
endfunction

// GetHP returns the STR-derived portion of the player's max HP. Same formula
// for legacy and path-rework characters: raw STR plus ability/prop mods.
function GetHP(who)
	var hp := CInt(AP_GetTrueStat(who, "Strength"));

	if (MobileWrapper(who).HasAbility(ABILITY_ID::RESISTENCIA_DO_DESERTO))
		hp := hp + 30;
	endif

	hp := hp + Cint(GetObjProperty(who, "hitsmod")) + Cint(GetObjProperty(who, "#hitsmod"));

	return hp;
endfunction

// GetPH returns the level/class-derived portion of the player's max HP.
//
// Path-rework characters: PH = ClassDie · (1 + OddLevelsReached(Level)).
//   - At L1: +ClassDie HP (e.g. +10 for d10, +6 for d6) replaces the old
//     archetype-wide bonus (Guerreiro 25 / Ladino 20 / Sabio 15).
//   - Each odd level reached (3, 5, ..., 19) adds another ClassDie worth of HP.
//   - At L20: +10·ClassDie total (e.g. +100 for d10, +120 for d12).
//
// Legacy characters fall through to the original archetype lookup.
function GetPH(who)
	var ph;

	if (HasNewSystemDice(who))
		var chardata := GetObjProperty(who, "chardata");
		var die := ParseDieValue(chardata.dice);
		var level := CInt(GetObjProperty(who, "Level"));
		if (level < 1)
			level := 1;
		endif
		ph := die * (1 + CountOddLevelsReached(level));
	else
		var classe := GetObjProperty(who, "classe");
		if ( (classe == "Guerreiro") || (GetObjProperty(who, "kit") == "Mago de Combate")) // 125
			ph := 25; // 25 + 10;
		elseif (classe == "Artifice")
			ph := 105;
		elseif (classe == "Ladino") // 100
			ph := 20; // 20 + 80
		else // 75
			ph := 15; // 15 + 60;
		endif
	endif

	if (MobileWrapper(who).HasAbility(ABILITY_ID::CACADOR_IMPLACAVEL))
		ph := ph + 25;
	endif

	ph := ph + Cint(GetObjProperty(who, "phmod")) + Cint(GetObjProperty(who, "#phmod"));

	return ph;
endfunction
