/**
 * Cleric ability helpers
 * Shared functions for Cleric class abilities (IDs 2800-2804)
 */

/**
 * Check if a mobile is an ally of 'who' (party, guild, or pet/summon)
 * @param who - raw UO mobile object
 * @param mobile - raw UO mobile object to check
 * @returns 1 if ally, 0 otherwise
 */
function IsPlayerAlly(who, mobile)
	if (who.party && mobile.party && who.party == mobile.party)
		return 1;
	endif

	if (who.guild && mobile.guild && who.guild == mobile.guild)
		return 1;
	endif

	var owner := GetObjProperty(mobile, "owner");
	if (owner == who.serial)
		return 1;
	endif

	return 0;
endfunction

/**
 * Get all enemy mobiles near 'who' within range
 * @param who - raw UO mobile object
 * @param range - tile radius
 * @returns array of raw UO mobile objects
 */
function GetNearbyEnemies(who, range)
	var enemies := array{};
	var nearby := ListMobilesNearLocation(who.x, who.y, who.z, range, who.realm);

	foreach mobile in (nearby)
		if (mobile == who)
			continue;
		endif

		if (!IsPlayerAlly(who, mobile))
			enemies.append(mobile);
		endif
	endforeach

	return enemies;
endfunction

/**
 * Get all allied mobiles near 'who' within range (includes self)
 * @param who - raw UO mobile object
 * @param range - tile radius
 * @returns array of raw UO mobile objects
 */
function GetNearbyAllies(who, range)
	var allies := array{};
	var nearby := ListMobilesNearLocation(who.x, who.y, who.z, range, who.realm);

	foreach mobile in (nearby)
		if (mobile == who)
			allies.append(mobile);
			continue;
		endif

		if (IsPlayerAlly(who, mobile))
			allies.append(mobile);
		endif
	endforeach

	return allies;
endfunction
