/*
 * $Id: key.inc 1259 2006-05-01 21:16:11Z austin $
 *
 */

use uo;

include ":itemutils:itemdesc";
include "include/say";

function AllocLockId()
  var lockid := GetGlobalProperty( "nextlockid" );
  if (!lockid)
    lockid := 1;
  endif
  SetGlobalProperty( "nextlockid", lockid+1 );
  return lockid;
endfunction

/*
 * KP_ToggleLock(object)
 *
 * Purpose
 * Toggles the locked member of an object.
 *
 * Parameters
 * object:	Container or door to toggle the locked status on.
 * mobile:	Optional - Mobile to show the locked status to.
 *
 * Return value
 * Returns 1
 *
 */
function KP_ToggleLock(object, mobile:=0)
	if ( object.locked )
		// PrintTextAbovePrivate(object, "*destrancado*", mobile);
		PrintText(mobile, "*Destrancou "+object.desc+"*");
		object.locked := 0;
	else
		PrintText(mobile, "*Trancou "+object.desc+"*");
		// PrintTextAbovePrivate(object, "*trancado*", mobile);
		object.locked := 1;
	endif

	return 1;
endfunction

/*
 * KP_IsLockable(object)
 *
 * Purpose
 * Determines if an item is lockable or not.
 *
 * Parameters
 * Object:	Object to check.
 *
 * Return value
 * Returns 1 if the object is lockable
 * Returns 0 if the object is not lockable.
 *
 */
function KP_IsLockable(object)
	if ( object.IsA(POLCLASS_ITEM) )
		object := object.objtype;
	endif

	return GetItemDescInfo(object).Lockable;
endfunction

/*
 * KP_HasKeyForLock(container, lock_id)
 *
 * Purpose
 * Determines if a container has a key that matches a lock_id
 *
 * Parameters
 * container:	Container to search for keys in.
 * lock_id:	Lock ID to match.
 *
 * Return value
 * Returns 1 if a match was found.
 * Returns 0 if no keys match the lock.
 *
 */
function KP_HasKeyForLock(container, lock_id)
	if ( container.IsA(POLCLASS_MOBILE) )
		container := container.backpack;
	endif
	if ( lock_id.IsA(POLCLASS_ITEM) )
		lock_id := GetObjProperty(lock_id, "LockId");
	endif

	foreach item in ( EnumerateItemsInContainer(container) )
		if ( item.IsKey() )
			if ( item.KeyMatchesLock(lock_id) )
				return 1;
			endif
		endif
		sleepms(2);
	endforeach

	return 0;
endfunction