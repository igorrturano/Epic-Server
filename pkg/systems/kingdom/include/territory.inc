// ============================================================================
// Kingdom System - Territory Management
// Epic! Shard - Kingdom Management System v0.1.0
// ============================================================================

use uo;
use os;
use cfgfile;

include ":kingdom:constants";
include ":kingdom:datafile";

// ============================================================================
// TERRITORY CREATION AND DELETION
// ============================================================================

/*
 * TRTY_CreateTerritory(name, rect, size_level, rare_resources, who)
 * Creates a new unclaimed territory
 * Parameters:
 *   name - Territory name (used as unique identifier)
 *   rect - String "x1 y1 x2 y2 realm" or calculated from center and size
 *   size_level - 1-5 (territory size)
 *   rare_resources - Array of resource names (e.g., array{"Iron", "Gold"})
 *   who - Character creating the territory (for positioning)
 * Returns: 1 on success, 0 on failure
 */
function TRTY_CreateTerritory(name, rect, size_level, rare_resources, who)
	if (!name || name == "")
		syslog("KINGDOM: CreateTerritory failed - invalid name");
		return 0;
	endif

	if (!KD_IsValidTerritorySize(size_level))
		syslog("KINGDOM: CreateTerritory failed - invalid size_level: " + size_level);
		return 0;
	endif

	// Check if territory already exists
	var existing := KD_GetTerritory(name);
	if (existing)
		syslog("KINGDOM: CreateTerritory failed - territory already exists: " + name);
		return 0;
	endif

	// Build territory data struct
	var territory := struct{};
	territory.+name := name;
	territory.+rect := rect;
	territory.+size_level := size_level;
	territory.+owner_faction := ""; // Unclaimed
	territory.+is_capital := 0;
	territory.+rare_resources := rare_resources;
	territory.+buildings := array{};
	territory.+war_totem_serial := 0;
	territory.+management_board_serial := 0;
	territory.+chest_serial := 0;
	territory.+claimed_date := 0;
	territory.+next_resource_time := 0;
	territory.+next_common_time := 0;

	// Create war totem at territory center
	var totem := CreateItemAtLocation(who.x, who.y, who.z, 0x5005, 1, who.realm);
	if (!totem)
		syslog("KINGDOM: CreateTerritory failed - could not create war totem");
		return 0;
	endif

	// Link totem to territory
	totem.SetProp("territory_name", name);
	totem.name := "War Totem [" + name + "]";
	territory.war_totem_serial := totem.serial;

	// Save to datafile
	var result := KD_SaveTerritory(name, territory);
	if (result)
		syslog("KINGDOM: Territory created - " + name + " with totem at " + who.x + "," + who.y);
	else
		syslog("KINGDOM: CreateTerritory failed - datafile save error");
		// Clean up totem if save failed
		DestroyItem(totem);
	endif

	return result;
endfunction

/*
 * TRTY_DeleteTerritory(territory_name)
 * Deletes a territory and cleans up associated items
 * Parameters:
 *   territory_name - Territory name (used as unique identifier)
 * Returns: 1 on success, 0 on failure
 */
function TRTY_DeleteTerritory(territory_name)
	if (!territory_name || territory_name == "")
		return 0;
	endif

	// Get territory data for cleanup
	var territory := KD_GetTerritory(territory_name);
	if (!territory)
		return 0; // Already deleted
	endif

	// Cleanup items
	TRTY_CleanupTerritoryItems(territory);

	// Delete from datafile
	var result := KD_DeleteTerritory(territory_name);
	if (result)
		syslog("KINGDOM: Territory deleted - " + territory_name);
	else
		syslog("KINGDOM: DeleteTerritory failed - datafile error");
	endif

	return result;
endfunction

/*
 * TRTY_CleanupTerritoryItems(territory)
 * Destroys all items associated with a territory
 * Parameters:
 *   territory - Territory struct
 * Returns: None
 */
function TRTY_CleanupTerritoryItems(territory)
	// Destroy war totem
	if (territory.war_totem_serial)
		var totem := SystemFindObjectBySerial(territory.war_totem_serial);
		if (totem)
			DestroyItem(totem);
		endif
	endif

	// Destroy management board
	if (territory.management_board_serial)
		var board := SystemFindObjectBySerial(territory.management_board_serial);
		if (board)
			DestroyItem(board);
		endif
	endif

	// Destroy donation chest
	if (territory.chest_serial)
		var chest := SystemFindObjectBySerial(territory.chest_serial);
		if (chest)
			DestroyItem(chest);
		endif
	endif

	// Destroy buildings
	if (territory.buildings)
		foreach building_serial in (territory.buildings)
			var building := SystemFindObjectBySerial(building_serial);
			if (building)
				DestroyItem(building);
			endif
			sleepms(2);
		endforeach
	endif
endfunction

// ============================================================================
// TERRITORY CLAIMING
// ============================================================================

/*
 * TRTY_ClaimTerritory(territory_name, faction_name, is_capital)
 * Claims a territory for a faction
 * Parameters:
 *   territory_name - Territory name (used as unique identifier)
 *   faction_name - Name of the faction claiming
 *   is_capital - 1 if capital, 0 if expansion
 * Returns: 1 on success, 0 on failure
 */
function TRTY_ClaimTerritory(territory_name, faction_name, is_capital)
	if (!territory_name || territory_name == "" || !faction_name || faction_name == "")
		return 0;
	endif

	var territory := KD_GetTerritory(territory_name);
	if (!territory)
		syslog("KINGDOM: ClaimTerritory failed - territory not found: " + territory_name);
		return 0;
	endif

	// Check if already claimed
	if (territory.owner_faction != "")
		syslog("KINGDOM: ClaimTerritory failed - already claimed by: " + territory.owner_faction);
		return 0;
	endif

	// Update territory data
	territory.owner_faction := faction_name;
	territory.is_capital := is_capital;
	territory.claimed_date := POLCore().systime;

	// Initialize resource timers
	var current_time := POLCore().systime;
	territory.next_resource_time := current_time + RESOURCE_INTERVAL_RARE;
	territory.next_common_time := current_time + RESOURCE_INTERVAL_COMMON;

	// Save to datafile
	var result := KD_SaveTerritory(territory_name, territory);
	if (result)
		syslog("KINGDOM: Territory claimed - " + territory_name + " by " + faction_name + " (capital: " + is_capital + ")");
	else
		syslog("KINGDOM: ClaimTerritory failed - datafile save error");
	endif

	return result;
endfunction

/*
 * TRTY_UnclaimTerritory(territory_name)
 * Releases a territory back to unclaimed status
 * Parameters:
 *   territory_name - Territory name (used as unique identifier)
 * Returns: 1 on success, 0 on failure
 */
function TRTY_UnclaimTerritory(territory_name)
	if (!territory_name || territory_name == "")
		return 0;
	endif

	var territory := KD_GetTerritory(territory_name);
	if (!territory)
		return 0;
	endif

	var old_owner := territory.owner_faction;

	// Reset territory ownership
	territory.owner_faction := "";
	territory.is_capital := 0;
	territory.claimed_date := 0;
	territory.buildings := array{};
	territory.management_board_serial := 0;
	territory.chest_serial := 0;
	territory.next_resource_time := 0;
	territory.next_common_time := 0;

	// Save to datafile
	var result := KD_SaveTerritory(territory_name, territory);
	if (result)
		syslog("KINGDOM: Territory unclaimed - " + territory_name + " (was owned by " + old_owner + ")");
	else
		syslog("KINGDOM: UnclaimTerritory failed - datafile save error");
	endif

	return result;
endfunction

// ============================================================================
// TERRITORY QUERIES
// ============================================================================

/*
 * TRTY_GetTerritoryInfo(territory_name)
 * Retrieves full territory information
 * Parameters:
 *   territory_name - Territory name (used as unique identifier)
 * Returns: Territory struct or 0 if not found
 */
function TRTY_GetTerritoryInfo(territory_name)
	return KD_GetTerritory(territory_name);
endfunction

/*
 * TRTY_ListAllTerritories()
 * Returns array of all territory IDs
 * Returns: Array of territory IDs
 */
function TRTY_ListAllTerritories()
	return KD_ListAllTerritories();
endfunction

/*
 * TRTY_ListFactionTerritories(faction_name)
 * Returns array of territory IDs owned by a faction
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: Array of territory IDs
 */
function TRTY_ListFactionTerritories(faction_name)
	if (!faction_name || faction_name == "")
		return array{};
	endif

	var all_territories := TRTY_ListAllTerritories();
	var faction_territories := array{};

	foreach territory_name in (all_territories)
		var territory := KD_GetTerritory(territory_name);
		if (territory && territory.owner_faction == faction_name)
			faction_territories.append(territory_name);
		endif
		sleepms(2);
	endforeach

	return faction_territories;
endfunction

/*
 * TRTY_IsInTerritory(x, y, realm, territory_name)
 * Checks if coordinates are within territory bounds
 * Parameters:
 *   x, y - Coordinates
 *   realm - Realm name
 *   territory_name - Territory name to check
 * Returns: 1 if inside, 0 if outside
 */
function TRTY_IsInTerritory(x, y, realm, territory_name)
	var territory := KD_GetTerritory(territory_name);
	if (!territory)
		return 0;
	endif

	// Parse rect string: "x1 y1 x2 y2 realm"
	var rect_parts := SplitWords(territory.rect);
	if (rect_parts.size() < 5)
		return 0;
	endif

	var x1 := CInt(rect_parts[1]);
	var y1 := CInt(rect_parts[2]);
	var x2 := CInt(rect_parts[3]);
	var y2 := CInt(rect_parts[4]);
	var territory_realm := rect_parts[5];

	// Check realm
	if (realm != territory_realm)
		return 0;
	endif

	// Check if within bounds
	if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
		return 1;
	endif

	return 0;
endfunction

/*
 * TRTY_GetTerritoryAtLocation(x, y, realm)
 * Finds which territory contains the given coordinates
 * Parameters:
 *   x, y - Coordinates
 *   realm - Realm name
 * Returns: Territory struct or 0 if not found
 */
function TRTY_GetTerritoryAtLocation(x, y, realm)
	var all_territories := TRTY_ListAllTerritories();

	foreach territory_name in (all_territories)
		if (TRTY_IsInTerritory(x, y, realm, territory_name))
			return KD_GetTerritory(territory_name);
		endif
		sleepms(2);
	endforeach

	return 0;
endfunction

// ============================================================================
// TERRITORY UTILITIES
// ============================================================================

/*
 * TRTY_CalculateRectFromCenter(center_x, center_y, realm, size_level)
 * Calculates territory rect string from center point and size level
 * Parameters:
 *   center_x, center_y - Center coordinates
 *   realm - Realm name
 *   size_level - 1-5 (territory size)
 * Returns: Rect string "x1 y1 x2 y2 realm"
 */
function TRTY_CalculateRectFromCenter(center_x, center_y, realm, size_level)
	// Load territory size from config
	var cfg := ReadConfigFile(":kingdom:config/territories");
	if (!cfg)
		return "";
	endif

	var size_elem := cfg[size_level];
	if (!size_elem)
		print("Invalid size_level in territory config: " + size_level);
		return "";
	endif

	var width := GetConfigInt(size_elem, "Width");
	var height := GetConfigInt(size_elem, "Height");

	// Calculate bounds
	var half_width := CInt(width / 2);
	var half_height := CInt(height / 2);

	var x1 := center_x - half_width;
	var y1 := center_y - half_height;
	var x2 := center_x + half_width;
	var y2 := center_y + half_height;

	return x1 + " " + y1 + " " + x2 + " " + y2 + " " + realm;
endfunction

/*
 * TRTY_GetTerritoryCenter(territory_name)
 * Calculates center point of territory
 * Parameters:
 *   territory_name - Territory name (used as unique identifier)
 * Returns: Struct with x, y, realm or 0 on error
 */
function TRTY_GetTerritoryCenter(territory_name)
	var territory := KD_GetTerritory(territory_name);
	if (!territory)
		return 0;
	endif

	return TRTY_GetTerritoryCenter_FromRect(territory.rect);
endfunction

/*
 * TRTY_GenerateUniqueID()
 * Generates a unique territory ID
 * Returns: String "territory_XXX" where XXX is incremental number
 */
function TRTY_GenerateUniqueID()
	var all_territories := TRTY_ListAllTerritories();
	var max_num := 0;

	// Find highest number
	foreach territory_name in (all_territories)
		// Extract number from "territory_XXX"
		var parts := SplitWords(territory_name, "_");
		if (parts.size() >= 2)
			var num := CInt(parts[2]);
			if (num > max_num)
				max_num := num;
			endif
		endif
		sleepms(2);
	endforeach

	// Generate new ID
	var new_num := max_num + 1;

	// Pad number with leading zeros (3 digits)
	var num_str := CStr(new_num);
	while (num_str.length() < 3)
		num_str := "0" + num_str;
	endwhile

	var new_id := "territory_" + num_str;

	return new_id;
endfunction