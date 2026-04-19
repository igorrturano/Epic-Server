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
use vitals;
use cfgfile;

include ":wrappers:Objects/Model/MobileWrapper";

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
  if (GetObjProperty(mobile, "nocure"))
    return 0;
  endif
  AP_SetVital(mobile, "Hits", cint(AP_GetVital(mobile, "Hits")+amount));
  if (AP_GetVital(mobile, "Hits") >  AP_GetVitalMaximumValue(mobile, "Hits"))
    AP_SetVital(mobile, "Hits", AP_GetVitalMaximumValue(mobile, "Hits"));
  endif
endfunction

function HealMana(mobile, amount)
  AP_SetVital(mobile, "Mana", cint(AP_GetVital(mobile, "Mana")+amount));
  if (AP_GetVital(mobile, "Mana") >  AP_GetVitalMaximumValue(mobile, "Mana"))
    AP_SetVital(mobile, "Mana", AP_GetVitalMaximumValue(mobile, "Mana"));
  endif
endfunction

function HealStamina(mobile, amount)
  AP_SetVital(mobile, "Stamina", cint(AP_GetVital(mobile, "Stamina")+amount));
  if (AP_GetVital(mobile, "Stamina") >  AP_GetVitalMaximumValue(mobile, "Stamina"))
    AP_SetVital(mobile, "Stamina", AP_GetVitalMaximumValue(mobile, "Stamina"));
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
	var cur_amount := AP_GetVital(mobile, "Mana");

	// Se não houver mana suficiente, retornar falha (0) sem consumir mana
	if ( points > cur_amount )
		return 0;
	else
		cur_amount := cur_amount-points;
		AP_SetVital(mobile, "Mana", cur_amount);
		return 1;  // Sucesso
	endif

endfunction




function DamageStamina(mobile, points)

	points := CDbl(points);
	var cur_amount := AP_GetVital(mobile, "Stamina");

	if ( points > cur_amount )
		cur_amount := 0;
	else
		cur_amount := cur_amount-points;
	endif

	AP_SetVital(mobile, "Stamina", cur_amount);
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

/**
 * applyEpicHealing(mobile, amt)
 * 
 * Sistema central de cura que considera entropia do alvo.
 * Substitui applyEpicHealing e ApplyEpicHealing em todo o servidor.
 * 
 * ENTROPIA POSITIVA (0 a +20000):
 * - 0: Cura normal (0% bônus)
 * - +20000: +50% de cura
 * - Linear: bônus = (entropia / 40000) * amt
 * 
 * ENTROPIA NEGATIVA (0 a -20000):
 * - 0: Cura normal
 * - -10000: Sem cura (100% bloqueado)
 * - -20000: Sem cura + toma 100% do valor como dano
 * - Linear de 0 a -10000: reduz cura
 * - Linear de -10000 a -20000: adiciona dano
 * 
 * @param mobile Mobile alvo
 * @param amt Quantidade de cura
 * @return 1 se aplicou, 0 se bloqueado por nocure
 */
function applyEpicHealing(mobile, amt)
	var nocure := GetObjProperty(mobile, "nocure");
	if (nocure)
		// Value > 1 is a timestamp (from undead ability) — auto-expire
		if (nocure > 1 && polcore().systime >= nocure)
			EraseObjProperty(mobile, "nocure");
		else
			PrintTextAbove(mobile, "*Não pode ser curado*");
			return 0;
		endif
	endif

	// Demon/Undead CurseHeal — healing reduction
	var healreduction := GetObjProperty(mobile, "#healreduction");
	if (healreduction != error)
		if (polcore().systime < healreduction.expires)
			amt := CInt(amt * (1.0 - healreduction.percent / 100.0));
			SendSysMessageEx(mobile, $"A maldição reduz sua cura em {healreduction.percent}%!", SSM_FAIL);
		else
			EraseObjProperty(mobile, "#healreduction");
		endif
	endif

	var wrapper := MobileWrapper(mobile);
	var entropy := wrapper.GetEntropy();
	var healing := amt;
	var damage := 0;

	if (wrapper.IsNPC())
        if (Lower(wrapper.GetEnvironment()) == "undead" || Lower(wrapper.GetEnvironment()) == "demons")
            damage := CInt(amt / 2);
            ApplyRawDamage(mobile, damage);
			return true;
		endif
    endif

	// ENTROPIA POSITIVA: Bônus de cura (+10% por 4.000, máximo +50%)
	if (entropy > 0)
		var bonus := CDbl(entropy) / 40000.0; // 0.0 a 0.5
		if (bonus > 0.5)
			bonus := 0.5;
		endif
		healing := CInt(amt * (1.0 + bonus));

	// ENTROPIA NEGATIVA: Penalidade de cura (-10% por -4.000, máximo -50%)
	elseif (entropy < 0)
		var abs_entropy := Abs(entropy);
		var penalty := abs_entropy / 40000.0; // 0.0 a 0.5
		if (penalty > 0.5)
			penalty := 0.5;
		endif
		healing := CInt(amt * (1.0 - penalty));
	endif

	if (healing < 1 && amt > 0)
		healing := 1;
	endif

	if (damage > 0)
		Message("*A energia sagrada queima a carne corrompida! (-" + damage + " HP)*", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile);
		ApplyRawDamage(mobile, damage);
	endif

	if (healing > 0)
		if (entropy < 0)
			var pct := CInt((CDbl(Abs(entropy)) / 40000.0) * 100);
			if (pct > 50) pct := 50; endif
			Message("*Curado em " + healing + " HP (-" + pct + "% por corrupção)*", Color(SYSTEM_COLOR::INFO)).SendSys(mobile);
		endif
		wrapper.RestoreHP(healing);
	endif

	return true;
endfunction