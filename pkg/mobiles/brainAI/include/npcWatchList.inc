/*===============================================================
* Current Version
* NPCWATCHLIST.INC - v1.0
* Updated 9/13/2005 3:21AM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

use uo;
use os;
use cfgfile;

include ":brainAI:npcError";
include ":brainAI:npcUtil";

// Performance optimization: Global cache for watchlist
var g_watchlist_dict := dictionary;
var g_watchlist_dirty := 1;

/////////////////////////////////////////////////////////////////
// * Function List *
//
// NPC_AddToWatchList(npc, mobile)
// NPC_IsInWatchList(npc, mobile)
// NPC_LoadWatchList(npc)
// NPC_RemoveFromWatchList(npc, mobile)
// NPC_SaveWatchList(npc, dict_list)
//
/////////////////////////////////////////////////////////////////
//
// Note: Once a function leaves scope, it stops being critical.
//       As such, no set_critical(0) statements are made.
//
/*
 * NPC_LoadWatchList(npc)
 *
 * Purpose
 * Loads the watch list on an NPC.
 *
 * Parameters
 * npc:	NPC to load the watch list from.
 *
 * Return value
 * Returns a dictionary
 *
 */
function NPC_LoadWatchList(npc)
	// Performance: Use cached watchlist with dirty-flag pattern
	if (g_watchlist_dirty)
		set_critical(1);
		var watch_dict := GetObjProperty(npc, "#WatchList");
		if ( Lower(TypeOf(watch_dict)) != "dictionary" )
			watch_dict := dictionary;
		endif
		g_watchlist_dict := watch_dict;
		g_watchlist_dirty := 0;
	endif
	
	return g_watchlist_dict;
endfunction

/*
 * NPC_SaveWatchList(npc)
 *
 * Purpose
 * Saves the NPC's watch list.
 *
 * Parameters
 * npc:		NPC to save the watch list on.
 * dict_list:	Dictionary containing serials the npc is watching.
 *
 * Return value
 * Returns the result of SetObjProperty()
 *
 */
function NPC_SaveWatchList(npc, dict_list)
	// Performance: Invalidate cache and update
	g_watchlist_dict := dict_list;
	g_watchlist_dirty := 0; // Cache is fresh after save
	return SetObjProperty(npc, "#WatchList", dict_list);
endfunction

/*
 * NPC_AddToWatchList(npc, mobile)
 *
 * Purpose
 * Adds a mobile to the NPC's watch list.
 *
 * Parameters
 * npc:		NPC that will have its list added to.
 * mobile:	Mobile to add to the watch list.
 *
 * Return value
 * Returns 1 if the mobile was added.
 *
 */
function NPC_AddToWatchList(npc, mobile)
	set_critical(1);
	if ( mobile.IsA(POLCLASS_MOBILE) )
		mobile := mobile.serial;
	endif
	
	var watch_dict := NPC_LoadWatchList(npc);
	if ( !watch_dict.Exists(mobile) )
		watch_dict.Insert(mobile, polcore().systime);
		NPC_SaveWatchList(npc, watch_dict);
		
		return 1;
	else
		return 0;
	endif
endfunction

/*
 * NPC_RemoveFromWatchList(npc)
 *
 * Purpose
 * Removes a mobile from the NPC's watch list.
 *
 * Parameters
 * npc:		NPC that will have its list modified.
 * mobile:	Mobile to remove from the watch list.
 *
 * Return value
 * Returns 1 if the mobile was removed.
 *
 */
function NPC_RemoveFromWatchList(npc, mobile)
	set_critical(1);
	if ( mobile.IsA(POLCLASS_MOBILE) )
		mobile := mobile.serial;
	endif
	
	var watch_dict := NPC_LoadWatchList(npc);
	if ( watch_dict.Exists(mobile) )
		watch_dict.Erase(mobile);
		NPC_SaveWatchList(npc, watch_dict);
		
		return 1;
	else
		return 0;
	endif
endfunction

/*
 * NPC_IsInWatchList(npc, mobile)
 *
 * Purpose
 * Checks if a mobile is in an NPC's watch list.
 *
 * Parameters
 * npc:		NPC with the list to check
 * mobile:	Mobile that will be checked for in the list.
 *
 * Return value
 * Returns 1 if the mobile is in the list.
 *
 */
function NPC_IsInWatchList(npc, mobile)
	set_critical(1);
	if ( mobile.IsA(POLCLASS_MOBILE) )
		mobile := mobile.serial;
	endif
	
	var watch_dict := NPC_LoadWatchList(npc);
	return watch_dict.Exists(mobile);
endfunction

/*
 * NPC_ShouldWatch(npc, mobile, script_path)
 *
 * Purpose
 * Checks if the NPC should watch the mobile.
 *
 * Parameters
 * npc:		NPC that will do the checking
 * mobile:	Mobile to check to be added to the watch list.
 * settings:	Dictionary of the NPC's settings.
 * scripts:	Dictionary of the npcs script settings.
 *
 * Return value
 * Returns 1 if the mobile is a potential target.
 *
 */
function NPC_ShouldWatch(npc, mobile, byref settings, byref scripts)
	if ( !scripts["ShouldWatch"] )
		// Assume it wants to watch everything if it has no script.
		return 1;
	endif
	
	// Performance: Inline common shouldWatch checks instead of spawning scripts
	var script_name := scripts["ShouldWatch"].script;
	
	// targetPlayers: Most common - check for player characters only
	if (script_name == ":brainAI:scripts/shouldWatch/targetPlayers")
		if (mobile.cmdlevel) return 0; endif
		if (mobile.npctemplate) return 0; endif
		return mobile.connected ? 1 : 0;
	// targetAny: Attack any non-GM
	elseif (script_name == ":brainAI:scripts/shouldWatch/targetAny")
		return mobile.cmdlevel ? 0 : 1;
	// watchHated: Check hate list config
	elseif (script_name == ":brainAI:scripts/shouldWatch/watchHated")
		if (!mobile.npctemplate) return 0; endif
		// Cache hate list in settings if not present
		if (!settings["_cached_hatelist"])
			var hate_config := ReadConfigFile(":brainAI:config/hatelist");
			var config_list := FindConfigElem(hate_config, NPC_ParseTemplateName(npc.npctemplate).template);
			settings["_cached_hatelist"] := GetConfigStringArray(config_list, "Hates");
		endif
		var target_template := NPC_ParseTemplateName(mobile.npctemplate).template;
		return (target_template in settings["_cached_hatelist"]) ? 1 : 0;
	// watchHatedandPlayer: Hate list + players
	elseif (script_name == ":brainAI:scripts/shouldWatch/watchHatedandPlayer")
		if (mobile.cmdlevel) return 0; endif
		// Cache hate list config
		if (!settings["_cached_hatelist"])
			var hate_config := ReadConfigFile(":brainAI:config/hatelist");
			var config_list := FindConfigElem(hate_config, NPC_ParseTemplateName(npc.npctemplate).template);
			settings["_cached_hatelist"] := GetConfigStringArray(config_list, "Hates");
			settings["_cached_hateprop"] := GetConfigStringArray(config_list, "HateProp");
		endif
		if (mobile.npctemplate)
			var target_template := NPC_ParseTemplateName(mobile.npctemplate).template;
			return (target_template in settings["_cached_hatelist"]) ? 1 : 0;
		elseif (mobile.connected)
			// For players, would need HateProp logic - fallback to script for complex cases
			if (settings["_cached_hateprop"] && settings["_cached_hateprop"].size() > 0)
				var result := Run_Script_To_Completion(script_name, {npc, mobile, settings});
				return result;
			else
				return 0;
			endif
		endif
		return 0;
	else
		// Fallback: Unknown script, use original method
		var result := Run_Script_To_Completion(script_name, {npc, mobile, settings});
		if ( result.errortext )
			NPC_ErrorSysLog(npc, "Error::NPC_ShouldWatch() - "+result.errortext);
		endif
		return result;
	endif
endfunction
