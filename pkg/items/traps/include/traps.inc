/* $Id$
 *
 */

use uo;
use cfgfile;
use util;
use os;

/*
 * GetTrapInfo(trap_name)
 *
 * Purpose
 * Retrieves the config elem for a trap from traps.cfg
 *
 * Parameters
 * trap_name:	Config elem name of the trap.
 *
 * Return value
 * Returns a config elem on success.
 * Returns an error on failure.
 *
 */
function GetTrapInfo(trap_name)
	var trap_cfg := ReadConfigFile(":traps:config/traps");
	if ( trap_cfg.errortext )
		var temp := error{"errortxt":="GetTrapInfo()::Could not open :traps:config/traps.cfg ->"+trap_cfg.errortext};
		SysLog(temp.errortext);
		return temp;
	endif

	var trap_elem := trap_cfg[trap_name];
	if ( trap_elem.errortext )
		var temp := error{"errortext":="GetTrapInfo()::Could not find elem ["+trap_name+"] -> "+trap_elem.errortext};
		SysLog(temp.errortext);
		return temp;
	endif

	return trap_elem;
endfunction

function ExistTrap(object)
   var trap_list := GetObjProperty(object, "TrapList");
	if ( Lower(TypeOf(trap_list)) != "array" )
		trap_list := array{};
	endif

   if (trap_list.size() < 1 )
      return 0;
   endif

   return 1;
endfunction
/*
 * AddTrap(object, trap_name, strength, count)
 *
 * Purpose
 * Adds a trap to an object (its use-script must use this include file)
 *
 * Parameters
 * object:	Object to add the trap to.
 * trap_name:	Name of the trap to add.
 * strength:	How powerful the trap will be.
 * count:	How many times the trap can be triggered when the object is used.
 *		See traps.cfg for more information on 'count'
 *
 * Return value
 * Returns 1 on success.
 * Returns an error on failure.
 *
 */
function AddTrap(object, trap_name, strength, count:=0)
	var trap_list := GetObjProperty(object, "TrapList");

	if ( Lower(TypeOf(trap_list)) != "array" )
		trap_list := array{};
	endif

	var trap_elem := GetTrapInfo(trap_name);
	if ( trap_elem.errortext )
		return trap_elem; // Contains an error
	endif

	if ( !count )
		count := trap_elem.Count;
		if ( !count )
			count := -1;
//		elseif ( count["d"] )
//			count := RandomDiceRoll(cstr(trap_elem.count));
		else
			count := CInt(cstr(trap_elem.Count));
		endif
	endif

	var trap := struct;
	trap.+Name := trap_name;
	trap.+strength := strength;
	// trap.+danger := danger;
	trap.+Count := count;

	trap_list.Append(trap);

	SetObjProperty(object, "TrapList", trap_list);

	return 1;
endfunction

/*
 * RemoveTrap(object, index)
 *
 * Purpose
 * Removes a trap from the object's trap list.
 *
 * Parameters
 * object:	Object to remove the trap from.
 * index:	Index in the list's array to remove the trap from.
 *
 * Return value
 * Returns 1 on success.
 * Returns an error on failure.
 *
 */
function RemoveTrap(object, index)
	var trap_list := GetObjProperty(object, "TrapList");
	if ( trap_list == error )
		return error{"errortext":="Object has no traps."};
	else
		trap_list.Erase(index); //remove uma das armadilhas
		if ( trap_list.Size() > 0 )
			SetObjProperty(object, "TrapList", trap_list);
		else
			EraseObjProperty(object, "TrapList");
			object.usescript := "";
			var name := object.desc;
			if(name[" [!]"])
				name[" [!]"] := " [O]";
			else
				name := name + " [O]";
				SetName(object, name);
			endif
		endif
	endif
	return 1;
endfunction

/*
 * RunTraps(object, mobile)
 *
 * Purpose
 * Runs all traps that are set on the object.
 *
 * Parameters
 * object:	Object to run the traps on.
 * mobile:	Who triggered the traps.
 *
 * Return value
 * Returns 1 on completion.
 *
 */
function RunTraps(object, mobile)
	var trap_list := GetObjProperty(object, "TrapList");
	if ( trap_list == error )
		return 0;
	endif

	foreach trap_info in ( trap_list )
		var cfg_info := GetTrapInfo(trap_info.name);

		var result := start_script(cfg_info.Script, {mobile, object, trap_info, _trap_info_iter});
		if ( result.errortext )
			PrintTextAbove(object, result.errortext);
			continue;
		endif

		//if ( trap_info.Count > 0 ) // -1 = permanent trap
		//	trap_info.Count := trap_info.Count - 1;
		//
		//	if ( trap_info.count == 0 )
		///		RemoveTrap(object, _trap_info_iter);
		//	endif
		//endif
		sleepms(2);
	endforeach

	return 1;
endfunction

function RunTrapEffect(trap_list, mobile)

	if ( trap_list == error )
		return 1;
	endif

	foreach trap_info in ( trap_list )
		var cfg_info := GetTrapInfo(trap_info.name);

		var result := start_script(cfg_info.Script, {mobile, mobile, trap_info, _trap_info_iter});

		//if ( trap_info.Count > 0 ) // -1 = permanent trap
		//	trap_info.Count := trap_info.Count - 1;
		//
		//	if ( trap_info.count == 0 )
		///		RemoveTrap(object, _trap_info_iter);
		//	endif
		//endif
		sleepms(2);
	endforeach

	return 1;
endfunction

function TrapFound(object, founder)
	SetObjProperty(object, "trapSpoted", founder.serial);
	SetName(object, object.desc+ " [!]");
endfunction

function getTrapDamage(strength, danger)
   if (!strength) strength := 2; endif

   var power := 80;
   case (danger)
      3:
         power := 250;
      2:
         power := 140;
   endcase

   return power + RandomDiceRoll("5d10");
endfunction

function getTrapAC(danger)
   case (danger)
      3: return RandomIntMinMax(16,21);
      2: return RandomIntMinMax(13,16);
      default: return RandomIntMinMax(10,12);
   endcase
   return RandomIntMinMax(10,12);
endfunction
