// ==========================================================================
// KINGDOM CACHE MANAGER
// High-performance caching system for kingdom/faction operations
// Uses DFTools for efficient database access patterns
// ==========================================================================

use uo;
use os;
use datafile;

include ":datafile:DFTools";
include ":faccao:include/faccao_constants";
include ":faccao:include/logging";

// Cache TTL constants (in seconds)
const CACHE_TTL_TERRITORIES := 300;        // 5 minutes
const CACHE_TTL_GUILDS := 180;             // 3 minutes
const CACHE_TTL_RESOURCES := 120;          // 2 minutes
const CACHE_TTL_BUILDING_DETAILS := 600;   // 10 minutes

// Global cache storage
var TERRITORY_CACHE := dictionary{};
var GUILD_CACHE := dictionary{};
var RESOURCE_CACHE := dictionary{};
var BUILDING_CACHE := dictionary{};
var CACHE_TIMESTAMPS := dictionary{};

// Singleton cache manager instance
var CACHE_MANAGER_INSTANCE := 0;

// ==========================================================================
// KINGDOM CACHE MANAGER CLASS
// ==========================================================================

class KingdomCacheManager()
    var territoryTools;
    var resourceTools;
    var initialized;

    function KingdomCacheManager(this)
        this.territoryTools := DFTools(":architect:areas");
        this.resourceTools := DFTools(KINGDOM_RESOURCE_DATAFILE);
        this.initialized := 1;
        LogInfo("cache_manager", "KingdomCacheManager initialized");
    endfunction

    // =======================================================================
    // TERRITORY CACHING METHODS
    // =======================================================================

    function GetActiveGuildsWithTerritories()
        var cache_key := "active_guilds";
        var cached_data := this.GetFromCache(cache_key, CACHE_TTL_TERRITORIES);

        if (cached_data)
            LogDebug("cache", "Cache hit for active guilds");
            return cached_data;
        endif

        LogDebug("cache", "Cache miss for active guilds, fetching from database");
        var active_guilds := this.FetchActiveGuildsFromDB();
        this.SetCache(cache_key, active_guilds);

        return active_guilds;
    endfunction

    function GetAllTerritoryData()
        var cache_key := "all_territories";
        var cached_data := this.GetFromCache(cache_key, CACHE_TTL_TERRITORIES);

        if (cached_data)
            return cached_data;
        endif

        var territory_data := this.FetchAllTerritoryDataFromDB();
        this.SetCache(cache_key, territory_data);

        return territory_data;
    endfunction

    function GetGuildTerritories(guild_name)
        var cache_key := "guild_territories_" + guild_name;
        var cached_data := this.GetFromCache(cache_key, CACHE_TTL_TERRITORIES);

        if (cached_data)
            return cached_data;
        endif

        var territories := this.FetchGuildTerritoriesFromDB(guild_name);
        this.SetCache(cache_key, territories);

        return territories;
    endfunction

    // =======================================================================
    // RESOURCE CACHING METHODS
    // =======================================================================

    function GetFactionResources(faction_name)
        var cache_key := "faction_resources_" + faction_name;
        var cached_data := this.GetFromCache(cache_key, CACHE_TTL_RESOURCES);

        if (cached_data)
            return cached_data;
        endif

        var resources := this.FetchFactionResourcesFromDB(faction_name);
        this.SetCache(cache_key, resources);

        return resources;
    endfunction

    // =======================================================================
    // BUILDING DETAILS CACHING
    // =======================================================================

    function GetBuildingDetails(building_type, level)
        var cache_key := "building_" + building_type + "_" + level;
        var cached_data := this.GetFromCache(cache_key, CACHE_TTL_BUILDING_DETAILS);

        if (cached_data)
            return cached_data;
        endif

        var details := FEUDO_GetBuildingLevelDetails(building_type, level);
        if (details && !details.errortext)
            this.SetCache(cache_key, details);
        endif

        return details;
    endfunction

    // =======================================================================
    // CORE CACHE METHODS
    // =======================================================================

    function GetFromCache(cache_key, ttl_seconds)
        var timestamp_key := cache_key + "_timestamp";
        var cached_time := CACHE_TIMESTAMPS[timestamp_key];

        if (!cached_time)
            return 0; // No cache entry
        endif

        var current_time := POLCore().systime;
        if ((current_time - cached_time) > ttl_seconds)
            // Cache expired
            this.InvalidateCache(cache_key);
            return 0;
        endif

        // Return cached data based on cache type
        if (cache_key["territories"] || cache_key["guild_territories"] || cache_key["active_guilds"])
            return TERRITORY_CACHE[cache_key];
        elseif (cache_key["resources"])
            return RESOURCE_CACHE[cache_key];
        elseif (cache_key["building"])
            return BUILDING_CACHE[cache_key];
        else
            return TERRITORY_CACHE[cache_key]; // Default
        endif
    endfunction

    function SetCache(cache_key, data)
        var timestamp_key := cache_key + "_timestamp";
        var current_time := POLCore().systime;

        // Store data in appropriate cache
        if (cache_key["territories"] || cache_key["guild_territories"] || cache_key["active_guilds"])
            TERRITORY_CACHE[cache_key] := data;
        elseif (cache_key["resources"])
            RESOURCE_CACHE[cache_key] := data;
        elseif (cache_key["building"])
            BUILDING_CACHE[cache_key] := data;
        else
            TERRITORY_CACHE[cache_key] := data; // Default
        endif

        CACHE_TIMESTAMPS[timestamp_key] := current_time;
        LogDebug("cache", "Cached data for key: " + cache_key);
    endfunction

    function InvalidateCache(cache_key)
        var timestamp_key := cache_key + "_timestamp";

        // Remove from all possible caches
        TERRITORY_CACHE.erase(cache_key);
        RESOURCE_CACHE.erase(cache_key);
        BUILDING_CACHE.erase(cache_key);
        CACHE_TIMESTAMPS.erase(timestamp_key);

        LogDebug("cache", "Invalidated cache for key: " + cache_key);
    endfunction

    function InvalidateGuildCache(guild_name)
        // Invalidate all cache entries related to this guild
        var keys_to_invalidate := array{
            "guild_territories_" + guild_name,
            "faction_resources_" + guild_name,
            "active_guilds"
        };

        foreach key in keys_to_invalidate
            this.InvalidateCache(key);
        endforeach

        LogInfo("cache", "Invalidated all cache entries for guild: " + guild_name);
    endfunction

    function InvalidateTerritoryCache(territory_id)
        // Invalidate territory-related caches
        var keys_to_invalidate := array{
            "all_territories",
            "active_guilds"
        };

        foreach key in keys_to_invalidate
            this.InvalidateCache(key);
        endforeach

        LogInfo("cache", "Invalidated territory cache for: " + territory_id);
    endfunction

    // =======================================================================
    // DATABASE FETCH METHODS (Using DFTools for efficiency)
    // =======================================================================

    function FetchActiveGuildsFromDB()
        var active_guilds := struct{};

        var all_elements := this.territoryTools.GetAllElements();
        if (all_elements.errortext)
            LogError("cache", "Failed to get territory elements: " + all_elements.errortext);
            return struct{};
        endif

        foreach territory_id in all_elements
            var faction := this.territoryTools.LoadValue(territory_id, "faction");
            if (faction && faction != error && faction != DEFAULT_FACTION)
                active_guilds[faction] := 1;
            endif
        endforeach

        LogInfo("cache", "Fetched " + active_guilds.keys().size() + " active guilds from database");
        return active_guilds.keys();
    endfunction

    function FetchAllTerritoryDataFromDB()
        var territory_map := dictionary{};

        var all_elements := this.territoryTools.GetAllElements();
        if (all_elements.errortext)
            LogError("cache", "Failed to get territory elements: " + all_elements.errortext);
            return territory_map;
        endif

        foreach territory_id in all_elements
            var faction := this.territoryTools.LoadValue(territory_id, "faction");
            var name := this.territoryTools.LoadValue(territory_id, "name");

            if (faction && faction != error)
                territory_map[territory_id] := struct{
                    "faction" := faction,
                    "name" := name,
                    "id" := territory_id
                };
            endif
        endforeach

        LogInfo("cache", "Fetched " + territory_map.keys().size() + " territories from database");
        return territory_map;
    endfunction

    function FetchGuildTerritoriesFromDB(guild_name)
        var territories := array{};
        var all_territory_data := this.GetAllTerritoryData();

        foreach territory_id in (all_territory_data.keys())
            var territory_info := all_territory_data[territory_id];
            if (territory_info.faction == guild_name)
                territories.append(territory_info);
            endif
        endforeach

        return territories;
    endfunction

    function FetchFactionResourcesFromDB(faction_name)
        var element_name := NormalizarNomeElementoFaccao(faction_name);
        if (element_name == error)
            LogError("cache", "Failed to normalize faction name: " + faction_name);
            return CreateEmptyResourceStruct();
        endif

        var resources := this.resourceTools.LoadValue(element_name, "recursos");
        if (resources.errortext)
            LogDebug("cache", "No resources found for faction: " + faction_name);
            return CreateEmptyResourceStruct();
        endif

        return resources;
    endfunction

    // =======================================================================
    // CACHE STATISTICS AND MAINTENANCE
    // =======================================================================

    function GetCacheStats()
        return struct{
            "territory_cache_size" := TERRITORY_CACHE.keys().size(),
            "resource_cache_size" := RESOURCE_CACHE.keys().size(),
            "building_cache_size" := BUILDING_CACHE.keys().size(),
            "total_timestamps" := CACHE_TIMESTAMPS.keys().size()
        };
    endfunction

    function ClearAllCaches()
        TERRITORY_CACHE := dictionary{};
        RESOURCE_CACHE := dictionary{};
        BUILDING_CACHE := dictionary{};
        CACHE_TIMESTAMPS := dictionary{};

        LogInfo("cache", "All caches cleared");
    endfunction
endclass

// ==========================================================================
// GLOBAL CACHE MANAGER FUNCTIONS
// ==========================================================================

function GetKingdomCacheManager()
    if (!CACHE_MANAGER_INSTANCE)
        CACHE_MANAGER_INSTANCE := KingdomCacheManager();
        LogInfo("cache_manager", "Created new KingdomCacheManager instance");
    endif

    return CACHE_MANAGER_INSTANCE;
endfunction

function InvalidateKingdomCache(guild_name)
    var cache_manager := GetKingdomCacheManager();
    cache_manager.InvalidateGuildCache(guild_name);
endfunction

function InvalidateTerritoryCache(territory_id)
    var cache_manager := GetKingdomCacheManager();
    cache_manager.InvalidateTerritoryCache(territory_id);
endfunction

// ==========================================================================
// CACHE EVENT HOOKS
// ==========================================================================

function OnTerritoryChanged(territory_id, old_faction, new_faction)
    var cache_manager := GetKingdomCacheManager();

    // Invalidate affected caches
    if (old_faction && old_faction != DEFAULT_FACTION)
        cache_manager.InvalidateGuildCache(old_faction);
    endif

    if (new_faction && new_faction != DEFAULT_FACTION)
        cache_manager.InvalidateGuildCache(new_faction);
    endif

    cache_manager.InvalidateTerritoryCache(territory_id);

    LogInfo("cache_events", "Territory " + territory_id + " changed from " + old_faction + " to " + new_faction);
endfunction

function OnUpkeepPayment(guild_name, amount)
    var cache_manager := GetKingdomCacheManager();
    cache_manager.InvalidateCache("faction_resources_" + guild_name);

    LogDebug("cache_events", "Upkeep payment invalidated resource cache for: " + guild_name);
endfunction

function OnResourceGeneration(guild_name)
    var cache_manager := GetKingdomCacheManager();
    cache_manager.InvalidateCache("faction_resources_" + guild_name);

    LogDebug("cache_events", "Resource generation invalidated cache for: " + guild_name);
endfunction