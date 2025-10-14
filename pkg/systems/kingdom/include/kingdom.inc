// ============================================================================
// Kingdom System - Kingdom Management
// Epic! Shard - Kingdom Management System v0.1.0
// ============================================================================

use uo;
use os;
use guilds;
use cfgfile;

include ":kingdom:constants";
include ":kingdom:datafile";
include ":kingdom:territory";
include ":faccao:faccao";

// ============================================================================
// KINGDOM CREATION AND DELETION
// ============================================================================

/*
 * KGD_CreateKingdom(faction_name, capital_territory, culture)
 * Creates a new kingdom for a faction
 * Parameters:
 *   faction_name - Name of the faction
 *   capital_territory - Territory ID to use as capital
 *   culture - Culture name (one of 8 cultures)
 * Returns: 1 on success, 0 on failure
 */
function KGD_CreateKingdom(faction_name, capital_territory, culture)
	if (!faction_name || faction_name == "")
		syslog("KINGDOM: CreateKingdom failed - invalid faction_name");
		return 0;
	endif

	if (!capital_territory || capital_territory == "")
		syslog("KINGDOM: CreateKingdom failed - invalid capital_territory");
		return 0;
	endif

	if (!KD_IsValidCulture(culture))
		syslog("KINGDOM: CreateKingdom failed - invalid culture: " + culture);
		return 0;
	endif

	// Check if kingdom already exists
	var existing := KD_GetKingdom(faction_name);
	if (existing)
		syslog("KINGDOM: CreateKingdom failed - kingdom already exists for: " + faction_name);
		return 0;
	endif

	// Verify territory exists
	var territory := KD_GetTerritory(capital_territory);
	if (!territory)
		syslog("KINGDOM: CreateKingdom failed - territory not found: " + capital_territory);
		return 0;
	endif

	// Build kingdom data struct
	var kingdom := struct{};
	kingdom.+faction_name := faction_name;
	kingdom.+capital_territory := capital_territory;
	kingdom.+territories := array{capital_territory};
	kingdom.+culture := culture;
	kingdom.+level := 1; // Start at level 1
	kingdom.+xp := 0;
	kingdom.+founded_date := POLCore().systime;
	kingdom.+upkeep_warnings := 0;
	kingdom.+next_upkeep_time := POLCore().systime + UPKEEP_INTERVAL;

	// Save to datafile
	var result := KD_SaveKingdom(faction_name, kingdom);
	if (result)
		syslog("KINGDOM: Kingdom created - " + faction_name + " (" + culture + ") with capital " + capital_territory);

		// Initialize resources for new kingdom
		var resources := KD_GetDefaultResources();
		KD_SaveResources(faction_name, resources);
	else
		syslog("KINGDOM: CreateKingdom failed - datafile save error");
	endif

	return result;
endfunction

/*
 * KGD_DeleteKingdom(faction_name)
 * Deletes a kingdom and releases all territories
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: 1 on success, 0 on failure
 */
function KGD_DeleteKingdom(faction_name)
	if (!faction_name || faction_name == "")
		return 0;
	endif

	var kingdom := KD_GetKingdom(faction_name);
	if (!kingdom)
		return 0; // Already deleted
	endif

	// Unclaim all territories
	if (kingdom.territories)
		foreach territory_name in (kingdom.territories)
			TRTY_UnclaimTerritory(territory_name);
			sleepms(2);
		endforeach
	endif

	// Delete kingdom data
	var result := KD_DeleteKingdom(faction_name);
	if (result)
		syslog("KINGDOM: Kingdom deleted - " + faction_name);
	else
		syslog("KINGDOM: DeleteKingdom failed - datafile error");
	endif

	return result;
endfunction

// ============================================================================
// KINGDOM QUERIES
// ============================================================================

/*
 * KGD_GetKingdomInfo(faction_name)
 * Retrieves full kingdom information
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: Kingdom struct or 0 if not found
 */
function KGD_GetKingdomInfo(faction_name)
	return KD_GetKingdom(faction_name);
endfunction

/*
 * KGD_KingdomExists(faction_name)
 * Checks if a kingdom exists for a faction
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: 1 if exists, 0 if not
 */
function KGD_KingdomExists(faction_name)
	var kingdom := KD_GetKingdom(faction_name);
	return (kingdom != 0);
endfunction

/*
 * KGD_GetCapitalTerritory(faction_name)
 * Returns the capital territory ID for a kingdom
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: Territory ID or "" if not found
 */
function KGD_GetCapitalTerritory(faction_name)
	var kingdom := KD_GetKingdom(faction_name);
	if (!kingdom)
		return "";
	endif

	return kingdom.capital_territory;
endfunction

// ============================================================================
// TERRITORY MANAGEMENT
// ============================================================================

/*
 * KGD_AddTerritory(faction_name, territory_name)
 * Adds an expansion territory to a kingdom
 * Parameters:
 *   faction_name - Name of the faction
 *   territory_name - Territory name to add
 * Returns: 1 on success, 0 on failure
 */
function KGD_AddTerritory(faction_name, territory_name)
	if (!faction_name || faction_name == "" || !territory_name || territory_name == "")
		return 0;
	endif

	var kingdom := KD_GetKingdom(faction_name);
	if (!kingdom)
		syslog("KINGDOM: AddTerritory failed - kingdom not found: " + faction_name);
		return 0;
	endif

	// Check if already in list
	if (territory_name in kingdom.territories)
		return 1; // Already added
	endif

	// Check territory limit
	var limit := KGD_GetTerritoryLimit(faction_name);
	if (kingdom.territories.size() >= limit)
		syslog("KINGDOM: AddTerritory failed - territory limit reached for: " + faction_name);
		return 0;
	endif

	// Add territory
	kingdom.territories.append(territory_name);

	// Save kingdom data
	var result := KD_SaveKingdom(faction_name, kingdom);
	if (result)
		syslog("KINGDOM: Territory added to kingdom - " + territory_name + " to " + faction_name);
	else
		syslog("KINGDOM: AddTerritory failed - datafile save error");
	endif

	return result;
endfunction

/*
 * KGD_RemoveTerritory(faction_name, territory_name)
 * Removes a territory from a kingdom
 * Parameters:
 *   faction_name - Name of the faction
 *   territory_name - Territory name to remove
 * Returns: 1 on success, 0 on failure
 */
function KGD_RemoveTerritory(faction_name, territory_name)
	if (!faction_name || faction_name == "" || !territory_name || territory_name == "")
		return 0;
	endif

	var kingdom := KD_GetKingdom(faction_name);
	if (!kingdom)
		return 0;
	endif

	// Cannot remove capital
	if (territory_name == kingdom.capital_territory)
		syslog("KINGDOM: RemoveTerritory failed - cannot remove capital territory");
		return 0;
	endif

	// Remove from list
	var new_territories := array{};
	foreach t_id in (kingdom.territories)
		if (t_id != territory_name)
			new_territories.append(t_id);
		endif
		sleepms(2);
	endforeach

	kingdom.territories := new_territories;

	// Save kingdom data
	var result := KD_SaveKingdom(faction_name, kingdom);
	if (result)
		syslog("KINGDOM: Territory removed from kingdom - " + territory_name + " from " + faction_name);
	else
		syslog("KINGDOM: RemoveTerritory failed - datafile save error");
	endif

	return result;
endfunction

// ============================================================================
// PLAYER VALIDATION
// ============================================================================

/*
 * KGD_ValidateFactionLeader(who, faction_name)
 * Checks if player is the leader of the specified faction
 * Parameters:
 *   who - Player mobile
 *   faction_name - Faction name to check (if "", uses player's guild)
 * Returns: 1 if leader, 0 if not
 */
function KGD_ValidateFactionLeader(who, faction_name := "")
	if (!who)
		return 0;
	endif

	var guild := who.guild;
	if (!guild)
		SendSysMessage(who, ERR_NO_FACTION);
		return 0;
	endif

	// Get faction info using GetFaccao
	var faccao := GetFaccao(guild);

	if (!faccao || !faccao.name)
		SendSysMessage(who, ERR_NO_FACTION);
		return 0;
	endif

	var player_faction := faccao.name;
	// If faction_name specified, verify it matches
	if (faction_name != "" && player_faction != faction_name)
		SendSysMessage(who, "You are not a member of that faction.");
		return 0;
	endif

	var leader := guild.GetProp("#guildmaster");
	if (!leader)
		leader := guild.GetProp("leader");
	endif

	if (leader != who.serial)
		SendSysMessage(who, ERR_NOT_FACTION_LEADER);
		return 0;
	endif

	return 1;
endfunction

/*
 * KGD_GetPlayerFaction(who)
 * Returns the faction name for a player
 * Parameters:
 *   who - Player mobile
 * Returns: Faction name or "" if not in faction
 */
function KGD_GetPlayerFaction(who)
	if (!who)
		return "";
	endif

	var guild := who.guild;
	if (!guild)
		return "";
	endif

	var faccao := GetFaccao(guild);
	if (!faccao || !faccao.name)
		return "";
	endif

	return faccao.name;
endfunction

// ============================================================================
// KINGDOM PROGRESSION
// ============================================================================

/*
 * KGD_GetTerritoryLimit(faction_name)
 * Returns the current territory limit based on kingdom level
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: Territory limit (int)
 */
function KGD_GetTerritoryLimit(faction_name)
	var kingdom := KD_GetKingdom(faction_name);
	if (!kingdom)
		return 1; // Default
	endif

	// Load culture config
	var cfg := ReadConfigFile(":kingdom:config/cultures");
	if (!cfg)
		return kingdom.level; // Fallback to level number
	endif

	var culture_elem := FindConfigElem(cfg, "Culture " + kingdom.culture);
	if (!culture_elem)
		return kingdom.level;
	endif

	var level_elem := FindConfigElem(culture_elem, "Level " + kingdom.level);
	if (!level_elem)
		return kingdom.level;
	endif

	return GetConfigInt(level_elem, "TerritoryLimit");
endfunction

/*
 * KGD_GetLevelName(faction_name)
 * Returns the culture-specific level name for a kingdom
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: Level name string or "" if not found
 */
function KGD_GetLevelName(faction_name)
	var kingdom := KD_GetKingdom(faction_name);
	if (!kingdom)
		return "";
	endif

	// Load culture config
	var cfg := ReadConfigFile(":kingdom:config/cultures");
	if (!cfg)
		return "Level " + kingdom.level;
	endif

	var culture_elem := FindConfigElem(cfg, "Culture " + kingdom.culture);
	if (!culture_elem)
		return "Level " + kingdom.level;
	endif

	var level_elem := FindConfigElem(culture_elem, "Level " + kingdom.level);
	if (!level_elem)
		return "Level " + kingdom.level;
	endif

	return GetConfigString(level_elem, "Name");
endfunction

/*
 * KGD_GetXPRequired(faction_name)
 * Returns XP required for next level
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: XP required (int) or -1 if max level
 */
function KGD_GetXPRequired(faction_name)
	var kingdom := KD_GetKingdom(faction_name);
	if (!kingdom)
		return 0;
	endif

	// If max level, return -1
	if (kingdom.level >= KINGDOM_LEVEL_MAX)
		return -1;
	endif

	var next_level := kingdom.level + 1;

	// Load culture config
	var cfg := ReadConfigFile(":kingdom:config/cultures");
	if (!cfg)
		return 10000 * next_level; // Fallback
	endif

	var culture_elem := FindConfigElem(cfg, "Culture " + kingdom.culture);
	if (!culture_elem)
		return 10000 * next_level;
	endif

	var level_elem := FindConfigElem(culture_elem, "Level " + next_level);
	if (!level_elem)
		return 10000 * next_level;
	endif

	return GetConfigInt(level_elem, "XPRequired");
endfunction

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/*
 * KGD_ListAllKingdoms()
 * Returns array of all kingdom faction names
 * Returns: Array of faction names
 */
function KGD_ListAllKingdoms()
	var df := KD_OpenKingdomsDF();
	if (!df)
		return array{};
	endif

	var kingdoms := array{};
	foreach key in (df.Keys())
		kingdoms.append(key);
		sleepms(2);
	endforeach

	return kingdoms;
endfunction

/*
 * KGD_FormatKingdomInfo(kingdom)
 * Formats kingdom info for display
 * Parameters:
 *   kingdom - Kingdom struct
 * Returns: Formatted string
 */
function KGD_FormatKingdomInfo(kingdom)
	var info := "";
	info += "Kingdom: " + kingdom.faction_name + "\n";
	info += "Culture: " + kingdom.culture + "\n";
	info += "Level: " + kingdom.level + " (" + KGD_GetLevelName(kingdom.faction_name) + ")\n";
	info += "XP: " + kingdom.xp + "\n";
	info += "Capital: " + kingdom.capital_territory + "\n";
	info += "Territories: " + kingdom.territories.size() + "/" + KGD_GetTerritoryLimit(kingdom.faction_name) + "\n";
	info += "Founded: " + FormatTimestamp(kingdom.founded_date) + "\n";
	info += "Upkeep Warnings: " + kingdom.upkeep_warnings + "\n";

	return info;
endfunction

/*
 * FormatTimestamp(timestamp)
 * Formats a POL timestamp into readable date
 * Parameters:
 *   timestamp - POL systime value
 * Returns: Formatted string
 */
function FormatTimestamp(timestamp)
	if (!timestamp || timestamp == 0)
		return "Never";
	endif

	// Simple formatting (can be improved)
	var elapsed := POLCore().systime - timestamp;
	var days := CInt(elapsed / 86400);

	if (days == 0)
		return "Today";
	elseif (days == 1)
		return "Yesterday";
	else
		return days + " days ago";
	endif
endfunction