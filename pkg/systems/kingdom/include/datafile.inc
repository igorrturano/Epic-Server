// ============================================================================
// Kingdom System Datafile Wrapper
// Uses DFTools for clean datafile access
// Epic! Shard - Kingdom Management System v0.1.0
// ============================================================================

use uo;
use os;
use datafile;

include ":datafile:datafile";
include ":kingdom:constants";

// ============================================================================
// DATAFILE OPENING FUNCTIONS
// ============================================================================

/*
 * KD_OpenKingdomsDF()
 * Opens the kingdoms datafile
 * Returns: Datafile object or error
 */
function KD_OpenKingdomsDF()
	var df := DFOpenDataFile(":kingdom:" + DF_KINGDOMS, DF_CREATE);
	if (!df)
		return error{"errortext" := "Failed to open kingdoms datafile"};
	endif
	return df;
endfunction

/*
 * KD_OpenTerritoriesDF()
 * Opens the territories datafile
 * Returns: Datafile object or error
 */
function KD_OpenTerritoriesDF()
	var df := DFOpenDataFile(":kingdom:" + DF_TERRITORIES, DF_CREATE);
	if (!df)
		return error{"errortext" := "Failed to open territories datafile"};
	endif
	return df;
endfunction

/*
 * KD_OpenResourcesDF()
 * Opens the resources datafile
 * Returns: Datafile object or error
 */
function KD_OpenResourcesDF()
	var df := DFOpenDataFile(":kingdom:" + DF_RESOURCES, DF_CREATE);
	if (!df)
		return error{"errortext" := "Failed to open resources datafile"};
	endif
	return df;
endfunction

/*
 * KD_OpenWavesDF()
 * Opens the waves configuration datafile (read-only)
 * Returns: Datafile object or error
 */
function KD_OpenWavesDF()
	var df := DFOpenDataFile(":kingdom:" + DF_WAVES, DF_CREATE);
	if (!df)
		return error{"errortext" := "Failed to open waves datafile"};
	endif
	return df;
endfunction

// ============================================================================
// KINGDOM DATA ACCESS
// ============================================================================

/*
 * KD_GetKingdom(faction_name)
 * Retrieves kingdom data for a faction
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: Struct with kingdom data or 0 if not found
 */
function KD_GetKingdom(faction_name)
	if (!faction_name || faction_name == "")
		return 0;
	endif

	var df := KD_OpenKingdomsDF();
	if (!df)
		return 0;
	endif

	var elem := DFFindElement(df, faction_name, DF_NO_CREATE);
	if (!elem)
		return 0;
	endif

	// Build kingdom struct from element properties
	var kingdom := struct{};
	kingdom.+faction_name := faction_name;
	kingdom.+capital_territory := elem.GetProp("capital_territory");
	kingdom.+territories := elem.GetProp("territories");
	kingdom.+culture := elem.GetProp("culture");
	kingdom.+level := CInt(elem.GetProp("level"));
	kingdom.+xp := CInt(elem.GetProp("xp"));
	kingdom.+founded_date := CInt(elem.GetProp("founded_date"));
	kingdom.+upkeep_warnings := CInt(elem.GetProp("upkeep_warnings"));
	kingdom.+next_upkeep_time := CInt(elem.GetProp("next_upkeep_time"));

	// Handle arrays properly
	if (!kingdom.territories)
		kingdom.territories := array{};
	endif

	return kingdom;
endfunction

/*
 * KD_SaveKingdom(faction_name, kingdom_data)
 * Saves kingdom data to datafile
 * Parameters:
 *   faction_name - Name of the faction
 *   kingdom_data - Struct with kingdom data
 * Returns: 1 on success, 0 on failure
 */
function KD_SaveKingdom(faction_name, kingdom_data)
	if (!faction_name || faction_name == "" || !kingdom_data)
		return 0;
	endif

	var df := KD_OpenKingdomsDF();
	if (!df)
		return 0;
	endif

	var elem := DFFindElement(df, faction_name, DF_CREATE);
	if (!elem)
		return 0;
	endif

	// Save all properties
	elem.SetProp("capital_territory", kingdom_data.capital_territory);
	elem.SetProp("territories", kingdom_data.territories);
	elem.SetProp("culture", kingdom_data.culture);
	elem.SetProp("level", CInt(kingdom_data.level));
	elem.SetProp("xp", CInt(kingdom_data.xp));
	elem.SetProp("founded_date", CInt(kingdom_data.founded_date));
	elem.SetProp("upkeep_warnings", CInt(kingdom_data.upkeep_warnings));
	elem.SetProp("next_upkeep_time", CInt(kingdom_data.next_upkeep_time));

	return 1;
endfunction

/*
 * KD_DeleteKingdom(faction_name)
 * Deletes kingdom data
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: 1 on success, 0 on failure
 */
function KD_DeleteKingdom(faction_name)
	if (!faction_name || faction_name == "")
		return 0;
	endif

	var df := KD_OpenKingdomsDF();
	if (!df)
		return 0;
	endif

	df.DeleteElement(faction_name);
	return 1;
endfunction

// ============================================================================
// TERRITORY DATA ACCESS
// ============================================================================

/*
 * KD_GetTerritory(territory_name)
 * Retrieves territory data
 * Parameters:
 *   territory_name - Territory name (used as unique identifier)
 * Returns: Struct with territory data or 0 if not found
 */
function KD_GetTerritory(territory_name)
	if (!territory_name || territory_name == "")
		return 0;
	endif

	var df := KD_OpenTerritoriesDF();
	if (!df)
		return 0;
	endif

	var elem := DFFindElement(df, territory_name, DF_NO_CREATE);
	if (!elem)
		return 0;
	endif

	// Build territory struct from element properties
	var territory := struct{};
	territory.+name := elem.GetProp("name");
	territory.+rect := elem.GetProp("rect");
	territory.+size_level := CInt(elem.GetProp("size_level"));
	territory.+owner_faction := elem.GetProp("owner_faction");
	territory.+is_capital := CInt(elem.GetProp("is_capital"));
	territory.+rare_resources := elem.GetProp("rare_resources");
	territory.+buildings := elem.GetProp("buildings");
	territory.+war_totem_serial := CInt(elem.GetProp("war_totem_serial"));
	territory.+management_board_serial := CInt(elem.GetProp("management_board_serial"));
	territory.+chest_serial := CInt(elem.GetProp("chest_serial"));
	territory.+claimed_date := CInt(elem.GetProp("claimed_date"));
	territory.+next_resource_time := CInt(elem.GetProp("next_resource_time"));
	territory.+next_common_time := CInt(elem.GetProp("next_common_time"));

	// Handle arrays properly
	if (!territory.rare_resources)
		territory.rare_resources := array{};
	endif
	if (!territory.buildings)
		territory.buildings := array{};
	endif
	if (!territory.owner_faction)
		territory.owner_faction := "";
	endif

	return territory;
endfunction

/*
 * KD_SaveTerritory(territory_name, territory_data)
 * Saves territory data to datafile
 * Parameters:
 *   territory_name - Territory name (used as unique identifier)
 *   territory_data - Struct with territory data
 * Returns: 1 on success, 0 on failure
 */
function KD_SaveTerritory(territory_name, territory_data)
	if (!territory_name || territory_name == "" || !territory_data)
		return 0;
	endif

	var df := KD_OpenTerritoriesDF();
	if (!df)
		return 0;
	endif

	var elem := DFFindElement(df, territory_name, DF_CREATE);
	if (!elem)
		return 0;
	endif

	// Save all properties
	elem.SetProp("name", territory_data.name);
	elem.SetProp("rect", territory_data.rect);
	elem.SetProp("size_level", CInt(territory_data.size_level));
	elem.SetProp("owner_faction", territory_data.owner_faction);
	elem.SetProp("is_capital", CInt(territory_data.is_capital));
	elem.SetProp("rare_resources", territory_data.rare_resources);
	elem.SetProp("buildings", territory_data.buildings);
	elem.SetProp("war_totem_serial", CInt(territory_data.war_totem_serial));
	elem.SetProp("management_board_serial", CInt(territory_data.management_board_serial));
	elem.SetProp("chest_serial", CInt(territory_data.chest_serial));
	elem.SetProp("claimed_date", CInt(territory_data.claimed_date));
	elem.SetProp("next_resource_time", CInt(territory_data.next_resource_time));
	elem.SetProp("next_common_time", CInt(territory_data.next_common_time));

	return 1;
endfunction

/*
 * KD_DeleteTerritory(territory_name)
 * Deletes territory data
 * Parameters:
 *   territory_name - Territory name (used as unique identifier)
 * Returns: 1 on success, 0 on failure
 */
function KD_DeleteTerritory(territory_name)
	if (!territory_name || territory_name == "")
		return 0;
	endif

	var df := KD_OpenTerritoriesDF();
	if (!df)
		return 0;
	endif

	df.DeleteElement(territory_name);
	return 1;
endfunction

/*
 * KD_ListAllTerritories()
 * Returns array of all territory IDs
 * Returns: Array of territory IDs or empty array
 */
function KD_ListAllTerritories()
	var df := KD_OpenTerritoriesDF();
	if (!df)
		return array{};
	endif

	var territories := array{};
	foreach key in (df.Keys())
		territories.append(key);
		sleepms(2);
	endforeach

	return territories;
endfunction

// ============================================================================
// RESOURCE DATA ACCESS
// ============================================================================

/*
 * KD_GetResources(faction_name)
 * Retrieves resource data for a faction
 * Parameters:
 *   faction_name - Name of the faction
 * Returns: Dictionary with resource counts or empty dict
 */
function KD_GetResources(faction_name)
	if (!faction_name || faction_name == "")
		return dictionary{};
	endif

	var df := KD_OpenResourcesDF();
	if (!df)
		return dictionary{};
	endif

	var elem := DFFindElement(df, faction_name, DF_NO_CREATE);
	if (!elem)
		// Return default empty resources
		return KD_GetDefaultResources();
	endif

	// Build resource dict from element properties
	var resources := dictionary{};
	resources[RESOURCE_COMIDA] := CInt(elem.GetProp(RESOURCE_COMIDA));
	resources[RESOURCE_MOEDAS] := CInt(elem.GetProp(RESOURCE_MOEDAS));
	resources[RESOURCE_COURO] := CInt(elem.GetProp(RESOURCE_COURO));
	resources[RESOURCE_METAL] := CInt(elem.GetProp(RESOURCE_METAL));
	resources[RESOURCE_MADEIRA] := CInt(elem.GetProp(RESOURCE_MADEIRA));
	resources[RESOURCE_PANO] := CInt(elem.GetProp(RESOURCE_PANO));
	resources[RESOURCE_PEDRA] := CInt(elem.GetProp(RESOURCE_PEDRA));
	resources[RESOURCE_JOIAS] := CInt(elem.GetProp(RESOURCE_JOIAS));

	return resources;
endfunction

/*
 * KD_SaveResources(faction_name, resources)
 * Saves resource data for a faction
 * Parameters:
 *   faction_name - Name of the faction
 *   resources - Dictionary with resource counts
 * Returns: 1 on success, 0 on failure
 */
function KD_SaveResources(faction_name, resources)
	if (!faction_name || faction_name == "" || !resources)
		return 0;
	endif

	var df := KD_OpenResourcesDF();
	if (!df)
		return 0;
	endif

	var elem := DFFindElement(df, faction_name, DF_CREATE);
	if (!elem)
		return 0;
	endif

	// Save all resource properties
	elem.SetProp(RESOURCE_COMIDA, CInt(resources[RESOURCE_COMIDA]));
	elem.SetProp(RESOURCE_MOEDAS, CInt(resources[RESOURCE_MOEDAS]));
	elem.SetProp(RESOURCE_COURO, CInt(resources[RESOURCE_COURO]));
	elem.SetProp(RESOURCE_METAL, CInt(resources[RESOURCE_METAL]));
	elem.SetProp(RESOURCE_MADEIRA, CInt(resources[RESOURCE_MADEIRA]));
	elem.SetProp(RESOURCE_PANO, CInt(resources[RESOURCE_PANO]));
	elem.SetProp(RESOURCE_PEDRA, CInt(resources[RESOURCE_PEDRA]));
	elem.SetProp(RESOURCE_JOIAS, CInt(resources[RESOURCE_JOIAS]));
	elem.SetProp("last_sync", POLCore().systime);

	return 1;
endfunction

/*
 * KD_GetDefaultResources()
 * Returns dictionary with all resources set to 0
 * Returns: Dictionary
 */
function KD_GetDefaultResources()
	var resources := dictionary{};
	resources[RESOURCE_COMIDA] := 0;
	resources[RESOURCE_MOEDAS] := 0;
	resources[RESOURCE_COURO] := 0;
	resources[RESOURCE_METAL] := 0;
	resources[RESOURCE_MADEIRA] := 0;
	resources[RESOURCE_PANO] := 0;
	resources[RESOURCE_PEDRA] := 0;
	resources[RESOURCE_JOIAS] := 0;
	return resources;
endfunction