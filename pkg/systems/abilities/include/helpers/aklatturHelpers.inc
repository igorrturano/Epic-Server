/**
 * Helper functions for Aklattur (Monster Hunter) abilities.
 *
 * GetHP, AP_GetVital, AP_GetVitalMaximumValue are transitively
 * available through CombatAbility -> MobileWrapper -> attributes.
 */

/**
 * Checks if a mobile is "raging" (low HP).
 * Uses GetHP() threshold (STR/4 + hitsmod), NOT max vital HP.
 *
 * @param mobile Raw mobile reference (GetObjRef())
 * @return Boolean
 */
function _IsRaging(mobile)
	var bonus_limiar_furia_hp := 0;

	if (mobile.npctemplate)
		if (AP_GetVital(mobile, HITS) < (CInt(AP_GetVitalMaximumValue(mobile, HITS) / 2) + bonus_limiar_furia_hp))
			return 1;
		endif
	elseif (AP_GetVital(mobile, HITS) <= GetHP(mobile) + bonus_limiar_furia_hp)
		return 1;
	endif

	return 0;
endfunction
