//$Id: vitals.inc 274 2005-09-28 19:07:32Z austin $

/*===============================================================
* Current Version
* VITALS.INC - v1.0
* Updated 9/24/2005 6:52PM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

use uo;
use os;
use cfgfile;

/* AP_GetVitalsCfgFile()
 *
 * Purpose
 * Loads vitals.cfg
 *
 * Parameters
 *
 * Return value
 * Config file
 *
 */
function AP_GetVitalsCfgFile()
	var cfg_file := ReadConfigFile(":attributes:vitals");
	if ( cfg_file.errortext )
		SysLog("Error::AP_GetVitalsCfgFile() - Unable to open vitals.cfg ->"+cfg_file.errortext);
	endif

	return cfg_file;
endfunction

/*
 * AP_GetVitalCfgElem(vital_name, cfg_file)
 *
 * Purpose
 * Retrieves a config elem for an vital in vitals.cfg
 *
 * Parameters
 * vital_name:	Name of the vital to get the elem of.
 * cfg_file:	Optional parameter - reference to a config already read in.
 *
 * Return value
 * Returns a config file elem.
 *
 */
function AP_GetVitalCfgElem(vital_name, byref cfg_file:=0)
	if ( !cfg_file )
		cfg_file := AP_GetVitalsCfgFile();
	endif

	var cfg_elem := cfg_file[vital_name];
	if ( cfg_elem.errortext )
		SysLog("Error::AP_GetVitalsCfgElem() - unable to find cfg elem ["+vital_name+"] ->"+cfg_elem.errortext);
	endif

	return cfg_elem;
endfunction

/*
 * AP_GetVitalNames()
 *
 * Purpose
 * Retrieves a list of all vitals in vitals.cfg
 *
 * Parameters
 *
 * Return value
 * Returns an array of strings
 *
 */
function AP_GetVitalNames()
	return GetConfigStringKeys(AP_GetVitalsCfgFile());
endfunction

/*
 * AP_GetVital(mobile, vital_name)
 *
 * Purpose
 * Retrieves the number of vital points a mobile has.
 *
 * Parameters
 * mobile:		Mobile reference to retrieve the points from.
 * vital_name:	Name of the vital to retrieve the points for.
 *
 * Return value
 * Returns a double.
 *
 */
function AP_GetVital(mobile, vital_name)
	return CDbl(GetVital(mobile, vital_name)) / 100.0;
endfunction

/*
 * AP_GetVitalMaximumValue(mobile, vital_name)
 *
 * Purpose
 * Retrieves the maximum number of vital points a mobile can have.
 *
 * Parameters
 * mobile:		Mobile reference to check.
 * vital_name:	Name of the vital to retrieve the max. points for.
 *
 * Return value
 * Returns an integer.
 *
 */
function AP_GetVitalMaximumValue(mobile, vital_name)
	return CDbl(GetVitalMaximumValue(mobile, vital_name)) / 100.0;
endfunction

/*
 * AP_GetVitalRegenRate(mobile, vital_name)
 *
 * Purpose
 * Retrieves the (base + mod) regen rate of a vital on a mobile.
 *
 * Parameters
 * mobile:	Mobile reference to check.
 * vital_name:	Name of the vital to retrieve the regen rate of.
 *
 * Return value
 * Returns an integer.
 *
 */
function AP_GetVitalRegenRate(mobile, vital_name)
	return GetVitalRegenRate(mobile, vital_name);
endfunction

function HealHits(mobile, amount)
  AP_SetVital(mobile, "Hits", cint(AP_GetVital(mobile, "Hits")+amount));
  if (AP_GetVital(mobile, "Hits") >  AP_GetVitalMaximumValue(mobile, "Hits"))
    AP_SetVital(mobile, "Hits", AP_GetVitalMaximumValue(mobile, "Hits"));
  endif
endfunction

function HealMana(mobile, amount)
  AP_SetVital(mobile, MANA, cint(AP_GetVital(mobile, MANA)+amount));
  if (AP_GetVital(mobile, MANA) >  AP_GetVitalMaximumValue(mobile, MANA))
    AP_SetVital(mobile, MANA, AP_GetVitalMaximumValue(mobile, MANA));
  endif
endfunction

function HealStamina(mobile, amount)
  AP_SetVital(mobile, STAMINA, cint(AP_GetVital(mobile, STAMINA)+amount));
  if (AP_GetVital(mobile, STAMINA) >  AP_GetVitalMaximumValue(mobile, STAMINA))
    AP_SetVital(mobile, STAMINA, AP_GetVitalMaximumValue(mobile, STAMINA));
  endif
endfunction

/*
 * AP_SetVital(mobile, vital_name, points)
 *
 * Purpose
 * Sets vital points on a mobile.
 *
 * Parameters
 * mobile:	Mobile reference to set the vital on.
 * vital_name:	Name of the vital to set.
 * points:	Number of points to set the vital to.
 *
 * Return value
 * Returns result of the SetVital() core function.
 *
 */
function AP_SetVital(mobile, vital_name, amount)
	return SetVital(mobile, vital_name, CInt(CDbl(amount)*100.0));
endfunction

/*
 * AP_ConsumeVital(mobile, vital_name, points)
 *
 * Purpose
 * Attempts to remove points from a vital.
 *
 * Parameters
 * mobile:	Mobile reference to consume the vital on.
 * vital_name:	Name of the vital to consume.
 * points:	Number of points to attempt to consume.
 *
 * Return value
 * Returns 1 on success
 *
 */
function AP_ConsumeVital(mobile, vital_name, points)
	points := CDbl(points);
	var cur_amount := AP_GetVital(mobile, vital_name);

	if ( points > cur_amount )
		return error{"errortext":="Insufficient points - unable to consume."};
	else
		cur_amount := cur_amount-points;
		AP_SetVital(mobile, vital_name, cur_amount);
		return 1;
	endif
endfunction

function DamageMANA(mobile, points)

	points := CDbl(points);
	var cur_amount := AP_GetVital(mobile, MANA);

	if ( points > cur_amount )
		cur_amount := 0;
	else
		cur_amount := cur_amount-points;
	endif

	AP_SetVital(mobile, MANA, cur_amount);
	return 1;


endfunction


function DamageStamina(mobile, points)

	points := CDbl(points);
	var cur_amount := AP_GetVital(mobile, STAMINA);

	if ( points > cur_amount )
		cur_amount := 0;
	else
		cur_amount := cur_amount-points;
	endif

	AP_SetVital(mobile, STAMINA, cur_amount);
	return 1;


endfunction

/*
 * AP_RefreshVitals(mobile)
 *
 * Purpose
 * Sets all vitals on a mobile to their maximum values.
 *
 * Parameters
 * mobile:	Mobile to refresh.
 *
 * Return value
 * Returns 1
 *
 */
function AP_RefreshVitals(mobile)
	var vital_cfg := ReadConfigFile(":attributes:vitals");
	foreach elem_name in GetConfigStringKeys(vital_cfg)
		if (elem_name != "SkillCap")
			AP_SetVital(mobile, elem_name, AP_GetVitalMaximumValue(mobile, elem_name));
		endif
        if (elem_name == "Hunger")
            AP_SetVital(mobile, elem_name, 0);
        endif
		sleepms(2);
	endforeach

	return 1;
endfunction
