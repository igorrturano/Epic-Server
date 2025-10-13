// ============================================================================
// Kingdom System Constants
// Epic! Shard - Kingdom Management System v0.1.0
// ============================================================================

use uo;
use os;

// ============================================================================
// RESOURCE TYPES
// ============================================================================

const RESOURCE_COMIDA := "Comida";
const RESOURCE_MOEDAS := "Moedas";
const RESOURCE_COURO := "Couro";
const RESOURCE_METAL := "Metal";
const RESOURCE_MADEIRA := "Madeira";
const RESOURCE_PANO := "Pano";
const RESOURCE_PEDRA := "Pedra";
const RESOURCE_JOIAS := "Joias";

// Resource array for iteration
var RESOURCE_TYPES := array{
	RESOURCE_COMIDA,
	RESOURCE_MOEDAS,
	RESOURCE_COURO,
	RESOURCE_METAL,
	RESOURCE_MADEIRA,
	RESOURCE_PANO,
	RESOURCE_PEDRA,
	RESOURCE_JOIAS
};

// ============================================================================
// CULTURES
// ============================================================================

const CULTURE_AIGLA := "Aigla";
const CULTURE_LESTE := "Leste";
const CULTURE_HAROLOTH := "Haroloth";
const CULTURE_BJORSKE := "Bjorske";
const CULTURE_ALURIEL := "Aluriel";
const CULTURE_POLKINEA := "Polkinea";
const CULTURE_BADURAN := "Baduran";
const CULTURE_GULTHRAK := "Gulthrak";

// Culture array for iteration
var CULTURE_TYPES := array{
	CULTURE_AIGLA,
	CULTURE_LESTE,
	CULTURE_HAROLOTH,
	CULTURE_BJORSKE,
	CULTURE_ALURIEL,
	CULTURE_POLKINEA,
	CULTURE_BADURAN,
	CULTURE_GULTHRAK
};

// ============================================================================
// BUILDING TYPES
// ============================================================================

const BUILDING_HOUSE := "house";
const BUILDING_FARM := "farm";
const BUILDING_QUARRY := "quarry";
const BUILDING_LUMBERMILL := "lumbermill";
const BUILDING_TANNERY := "tannery";

// ============================================================================
// TIMING CONSTANTS (Default values, can be overridden by config)
// ============================================================================

const RESOURCE_INTERVAL_RARE := 3600;      // 1 hour in seconds
const RESOURCE_INTERVAL_COMMON := 7200;    // 2 hours in seconds
const UPKEEP_INTERVAL := 604800;           // 1 week in seconds
const CONSTRUCTION_BASE_TIME := 3600;      // 1 hour in seconds

// ============================================================================
// TERRITORY CONSTANTS
// ============================================================================

const TERRITORY_SIZE_SMALL := 1;
const TERRITORY_SIZE_MEDIUM := 2;
const TERRITORY_SIZE_LARGE := 3;
const TERRITORY_SIZE_HUGE := 4;
const TERRITORY_SIZE_MASSIVE := 5;

// ============================================================================
// KINGDOM LEVEL CONSTANTS
// ============================================================================

const KINGDOM_LEVEL_MIN := 1;
const KINGDOM_LEVEL_MAX := 5;

// ============================================================================
// CONQUEST CONSTANTS
// ============================================================================

const CONQUEST_WAVE_1 := 1;
const CONQUEST_WAVE_2 := 2;
const CONQUEST_WAVE_3 := 3;
const CONQUEST_BOSS := 4;

const CONQUEST_STATUS_INACTIVE := 0;
const CONQUEST_STATUS_ACTIVE := 1;
const CONQUEST_STATUS_COMPLETE := 2;

// ============================================================================
// ITEM TYPES (Will be defined in itemdesc.cfg later)
// ============================================================================

const ITEMTYPE_WAR_TOTEM := 0x0ED4;         // Placeholder
const ITEMTYPE_KINGDOM_TOTEM := 0x0ED5;     // Placeholder
const ITEMTYPE_DONATION_CHEST := 0x0E43;    // Placeholder
const ITEMTYPE_MANAGEMENT_BOARD := 0x1E5E;  // Placeholder (map)
const ITEMTYPE_BUILDING_FOUNDATION := 0x1;  // Placeholder
const ITEMTYPE_BUILDING_SCAFFOLDING := 0x2; // Placeholder

// ============================================================================
// ERROR MESSAGES
// ============================================================================

const ERR_NOT_FACTION_LEADER := "You must be a faction leader to perform this action.";
const ERR_NO_FACTION := "You are not in a faction.";
const ERR_TERRITORY_NOT_FOUND := "Territory not found.";
const ERR_KINGDOM_NOT_FOUND := "Kingdom not found.";
const ERR_ALREADY_HAS_KINGDOM := "This faction already has a kingdom.";
const ERR_TERRITORY_ALREADY_CLAIMED := "This territory is already claimed.";
const ERR_INVALID_CULTURE := "Invalid culture selected.";
const ERR_INSUFFICIENT_RESOURCES := "Insufficient resources.";
const ERR_MAX_TERRITORIES_REACHED := "Maximum territories reached for current kingdom level.";

// ============================================================================
// SUCCESS MESSAGES
// ============================================================================

const MSG_TERRITORY_CREATED := "Territory created successfully.";
const MSG_TERRITORY_CLAIMED := "Territory claimed successfully.";
const MSG_KINGDOM_CREATED := "Kingdom founded successfully!";
const MSG_LEVEL_UP := "Your kingdom has leveled up!";
const MSG_UPKEEP_PAID := "Upkeep paid successfully.";

// ============================================================================
// DATAFILE NAMES
// ============================================================================

const DF_KINGDOMS := "kingdoms";
const DF_TERRITORIES := "territories";
const DF_RESOURCES := "resources";
const DF_WAVES := "waves";

// ============================================================================
// PROPERTY NAMES (CProps on items)
// ============================================================================

const PROP_KINGDOM_territory_name := "kingdom_territory_name";
const PROP_KINGDOM_FACTION := "kingdom_faction";
const PROP_KINGDOM_WAVE_TEMPLATE := "kingdom_wave_template";
const PROP_KINGDOM_CURRENT_WAVE := "kingdom_current_wave";
const PROP_KINGDOM_TOTAL_KILLS := "kingdom_total_kills";
const PROP_KINGDOM_REQUIRED_KILLS := "kingdom_required_kills";
const PROP_KINGDOM_ACTIVE := "kingdom_active";
const PROP_KINGDOM_CLAIMABLE := "kingdom_claimable";
const PROP_KINGDOM_BUILDING_TYPE := "kingdom_building_type";
const PROP_KINGDOM_BUILDING_LEVEL := "kingdom_building_level";
const PROP_KINGDOM_BUILDING_INFO := "kingdom_building_info";
const PROP_KINGDOM_MATERIALS := "kingdom_materials";
const PROP_KINGDOM_COMPLETION_TIME := "kingdom_completion_time";
const PROP_KINGDOM_RESOURCE_CACHE := "kingdom_resource_cache";
const PROP_KINGDOM_LAST_CACHE := "kingdom_last_cache";
const PROP_KINGDOM_PIECES := "kingdom_pieces";

// ============================================================================
// GUILD PROPERTY NAMES (CProps on Guild objects)
// ============================================================================

const GPROP_KINGDOM_TERRITORIES := "kingdom_territories";
const GPROP_KINGDOM_XP := "kingdom_xp";
const GPROP_KINGDOM_LEVEL := "kingdom_level";
const GPROP_KINGDOM_CULTURE := "kingdom_culture";

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Returns array of all resource types
function KD_GetAllResourceTypes()
	return RESOURCE_TYPES;
endfunction

// Returns array of all cultures
function KD_GetAllCultures()
	return CULTURE_TYPES;
endfunction

// Validates if culture name is valid
function KD_IsValidCulture(culture_name)
	return (culture_name in CULTURE_TYPES);
endfunction

// Validates if resource type is valid
function KD_IsValidResourceType(resource_type)
	return (resource_type in RESOURCE_TYPES);
endfunction

// Validates if territory size level is valid
function KD_IsValidTerritorySize(size_level)
	return (size_level >= TERRITORY_SIZE_SMALL && size_level <= TERRITORY_SIZE_MASSIVE);
endfunction

// Validates if kingdom level is valid
function KD_IsValidKingdomLevel(level)
	return (level >= KINGDOM_LEVEL_MIN && level <= KINGDOM_LEVEL_MAX);
endfunction