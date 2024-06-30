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
include ":charactercreation:habilidades";

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

	if (checkmount)
		var mounted := GetEquipmentByLayer(who, 0x19);

		if (mounted)
			SendSysMessage(who, "Voce nao pode fazer isto montado.");
			return 0;
		endif
	endif

	return 1;
endfunction

function GetHP(who)
	var hp := CInt(AP_GetTrueStat(who, "Strength"))/2;

	if (TemHabilidade(who, "Sangue Grosso"))
		hp := hp + 35;
	endif

	hp := hp + Cint(GetObjProperty(who, "hitsmod")) + Cint(GetObjProperty(who, "#hitsmod"));

	return hp;
endfunction

function GetPH(who)
	var ph;
	var classe := GetObjProperty(who, "classe");

	if ( (classe == "Guerreiro") || (GetObjProperty(who, "kit") == "Mago de Combate"))
		ph := 125;
    elseif (classe == "Artifice")
		ph := 105;
	elseif (classe == "Ladino")
		ph := 100;
	else
		ph := 75;
	endif

	if (TemHabilidade(who, "Grande Fortitude"))
		ph := ph + 25;
	endif

	ph := ph + Cint(GetObjProperty(who, "phmod")) + Cint(GetObjProperty(who, "#phmod")) +  CInt(AP_GetTrueStat(who, "Strength"))/2;

	return ph;
endfunction
