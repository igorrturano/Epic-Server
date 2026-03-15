use os;
use uo;

include "include/facings";
include "include/utils";

/**
 * Shinobi Helper Functions
 * Shared utilities for Shinobi abilities.
 */

/**
 * TeleportBehindTarget - Moves who behind targ, positioning at their back.
 * Extracted from ShadowLeap logic.
 *
 * @param who - Mobile to teleport
 * @param targ - Target mobile to position behind
 * @return 1 on success
 */
function TeleportBehindTarget(who, targ)
	// Move in line to target
	var coords := GetCoordsInLine(who.x, who.y, targ.x, targ.y);
	coords.Erase(coords.size());
	foreach coord in coords
		MoveObjectToLocation(who, coord.x, coord.y, targ.z, who.realm, MOVEOBJECT_FORCELOCATION);
		sleepms(50);
	endforeach

	// Position behind target
	var behind := InverseFacing(targ.facing);
	behind := GetModByFacing(behind);
	if (CanWalk("L", who.x, who.y, who.z, targ.x + behind[1], targ.y + behind[2]))
		MoveObjectToLocation(who, targ.x + behind[1], targ.y + behind[2], targ.z, who.realm, MOVEOBJECT_FORCELOCATION);
	else
		var radius := GetPointsInRadius(targ.x, targ.y, 1);
		var random := radius.randomentry();
		while (!MoveObjectToLocation(who, random.x, random.y, targ.z, who.realm))
			random := radius.randomentry();
			sleep(2);
		endwhile
	endif

	who.SetFacing(GetFacing(who.x, who.y, targ.x, targ.y));
	return 1;
endfunction

/**
 * IsInSmokeZone - Checks if a mobile is inside an active smoke zone.
 *
 * @param who - Mobile to check (raw obj ref)
 * @return 1 if in active smoke zone, 0 otherwise
 */
function IsInSmokeZone(who)
	var smoke_expires := GetObjProperty(who, "#smoke_protected");
	if (!smoke_expires)
		return 0;
	endif

	if (ReadGameClock() > smoke_expires)
		EraseObjProperty(who, "#smoke_protected");
		return 0;
	endif

	return 1;
endfunction
