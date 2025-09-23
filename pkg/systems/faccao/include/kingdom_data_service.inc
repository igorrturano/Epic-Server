// ==========================================================================
// KINGDOM DATA SERVICE
// High-level data access layer with caching for kingdom operations
// Provides clean interfaces for common operations while using caching underneath
// ==========================================================================

use uo;
use os;

include ":faccao:include/kingdom_cache_manager";
include ":faccao:include/shared_functions";
include ":faccao:include/safe_guild_utils";
include ":faccao:include/logging";

// ==========================================================================
// KINGDOM DATA SERVICE CLASS
// ==========================================================================

class KingdomDataService()
    var cacheManager;
    var initialized;

    function KingdomDataService(this)
        this.cacheManager := GetKingdomCacheManager();
        this.initialized := 1;
        LogDebug("data_service", "KingdomDataService initialized");
    endfunction

    // =======================================================================
    // TERRITORY OPERATIONS
    // =======================================================================

    function GetActiveGuilds()
        return this.cacheManager.GetActiveGuildsWithTerritories();
    endfunction

    function GetGuildTerritories(guild_name)
        if (!guild_name || guild_name == "")
            LogError("data_service", "GetGuildTerritories called with empty guild_name");
            return array{};
        endif

        return this.cacheManager.GetGuildTerritories(guild_name);
    endfunction

    function GetTerritoryCount(guild_name)
        var territories := this.GetGuildTerritories(guild_name);
        return territories.size();
    endfunction

    function GetGuildCapitalTerritory(guild_name)
        var territories := this.GetGuildTerritories(guild_name);

        foreach territory in territories
            if (TM_IsTerritoryCapital(territory.id))
                return territory;
            endif
        endforeach

        return 0; // No capital found
    endfunction

    function HasTerritories(guild_name)
        var territory_count := this.GetTerritoryCount(guild_name);
        return (territory_count > 0);
    endfunction

    // =======================================================================
    // GUILD OPERATIONS
    // =======================================================================

    function GetGuildInfo(guild_name)
        var cache_key := "guild_info_" + guild_name;
        var cached_data := this.cacheManager.GetFromCache(cache_key, CACHE_TTL_GUILDS);

        if (cached_data)
            return cached_data;
        endif

        // Fetch fresh guild data
        var guild_id := FindGuildByName(guild_name);
        if (!guild_id)
            LogWarning("data_service", "Guild not found: " + guild_name);
            return 0;
        endif

        var guild := FindGuild(guild_id);
        if (!guild)
            LogWarning("data_service", "Guild object not found for ID: " + guild_id);
            return 0;
        endif

        var guild_info := struct{
            "guild_id" := guild_id,
            "guild" := guild,
            "name" := SafeGetGuildName(guild),
            "territory_count" := this.GetTerritoryCount(guild_name),
            "has_territories" := this.HasTerritories(guild_name)
        };

        this.cacheManager.SetCache(cache_key, guild_info);
        return guild_info;
    endfunction

    function IsValidGuild(guild_name)
        var guild_info := this.GetGuildInfo(guild_name);
        return (guild_info != 0);
    endfunction

    // =======================================================================
    // RESOURCE OPERATIONS
    // =======================================================================

    function GetFactionResources(faction_name)
        return this.cacheManager.GetFactionResources(faction_name);
    endfunction

    function UpdateFactionResources(faction_name, new_resources)
        // Update in database first
        var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE, 1);
        var elem_name := NormalizarNomeElementoFaccao(faction_name);

        if (df && elem_name != error)
            var elem := SafeFindElement(df, elem_name, 1);
            SetElementProp(elem, "recursos", new_resources);

            // Invalidate cache to force refresh
            this.cacheManager.InvalidateCache("faction_resources_" + faction_name);

            LogInfo("data_service", "Updated resources for faction: " + faction_name);
            return 1;
        else
            LogError("data_service", "Failed to update resources for faction: " + faction_name);
            return 0;
        endif
    endfunction

    function AddResourcesToFaction(faction_name, resource_type, amount)
        var current_resources := this.GetFactionResources(faction_name);

        // Add to existing resources
        if (!current_resources[resource_type])
            current_resources[resource_type] := 0;
        endif

        current_resources[resource_type] := CInt(current_resources[resource_type]) + CInt(amount);

        return this.UpdateFactionResources(faction_name, current_resources);
    endfunction

    function DeductResourcesFromFaction(faction_name, resource_costs)
        var current_resources := this.GetFactionResources(faction_name);

        // Check if we have enough resources
        foreach resource_type in (resource_costs.keys())
            var required := CInt(resource_costs[resource_type]);
            var available := CInt(current_resources[resource_type]);

            if (available < required)
                LogWarning("data_service", "Insufficient " + resource_type + " for faction " + faction_name +
                          " (need: " + required + ", have: " + available + ")");
                return 0; // Insufficient resources
            endif
        endforeach

        // Deduct resources
        foreach resource_type in (resource_costs.keys())
            var cost := CInt(resource_costs[resource_type]);
            current_resources[resource_type] := CInt(current_resources[resource_type]) - cost;
        endforeach

        return this.UpdateFactionResources(faction_name, current_resources);
    endfunction

    // =======================================================================
    // BUILDING OPERATIONS
    // =======================================================================

    function GetBuildingDetails(building_type, level)
        return this.cacheManager.GetBuildingDetails(building_type, level);
    endfunction

    function CalculateBuildingUpkeepCost(guild_name)
        // This function is deprecated - use CalculateUpkeepCostOptimized in upkeep controller
        // Keeping for compatibility, but redirects to optimized version
        var territories := this.GetGuildTerritories(guild_name);
        return this.CalculateBasicUpkeepCost(territories);
    endfunction

    function CalculateBasicUpkeepCost(territories)
        if (!territories || territories.size() == 0)
            return 0;
        endif

        var base_cost_per_territory := 100;
        return territories.size() * base_cost_per_territory;
    endfunction

    function CalculateTerritoryUpkeepCost(territory)
        // This would need to be implemented based on your building system
        // For now, return a base cost per territory
        var base_cost := 100; // Base upkeep cost per territory

        // Add costs based on buildings in territory
        // This is where you'd iterate through buildings and sum their upkeep costs

        return base_cost;
    endfunction

    // =======================================================================
    // CHEST OPERATIONS (Optimized)
    // =======================================================================

    function FindFactionChestOptimized(faction_name, territory_id)
        var cache_key := "faction_chest_" + faction_name + "_" + territory_id;
        var cached_data := this.cacheManager.GetFromCache(cache_key, 60); // 1 minute cache

        if (cached_data)
            // Verify the chest still exists
            var chest := SystemFindObjectBySerial(cached_data.serial);
            if (chest && chest.isA(POLCLASS_CONTAINER))
                return chest;
            else
                // Chest no longer exists, invalidate cache
                this.cacheManager.InvalidateCache(cache_key);
            endif
        endif

        // Find chest using existing function
        var chest := FindFactionChest(faction_name, territory_id);
        if (chest)
            var chest_info := struct{
                "serial" := chest.serial,
                "location" := struct{
                    "x" := chest.x,
                    "y" := chest.y,
                    "z" := chest.z
                }
            };
            this.cacheManager.SetCache(cache_key, chest_info);
        endif

        return chest;
    endfunction

    // =======================================================================
    // BATCH OPERATIONS
    // =======================================================================

    function BatchGetGuildInfos(guild_names)
        var results := dictionary{};

        foreach guild_name in guild_names
            results[guild_name] := this.GetGuildInfo(guild_name);
        endforeach

        return results;
    endfunction

    function BatchGetTerritoryData(guild_names)
        var results := dictionary{};

        foreach guild_name in guild_names
            results[guild_name] := this.GetGuildTerritories(guild_name);
        endforeach

        return results;
    endfunction

    // =======================================================================
    // CACHE MANAGEMENT
    // =======================================================================

    function InvalidateGuildData(guild_name)
        this.cacheManager.InvalidateGuildCache(guild_name);
        LogInfo("data_service", "Invalidated all data for guild: " + guild_name);
    endfunction

    function GetCacheStatistics()
        return this.cacheManager.GetCacheStats();
    endfunction

    function ClearAllCaches()
        this.cacheManager.ClearAllCaches();
        LogInfo("data_service", "All caches cleared via data service");
    endfunction
endclass

// ==========================================================================
// GLOBAL DATA SERVICE FUNCTIONS
// ==========================================================================

var KINGDOM_DATA_SERVICE_INSTANCE := 0;

function GetKingdomDataService()
    if (!KINGDOM_DATA_SERVICE_INSTANCE)
        KINGDOM_DATA_SERVICE_INSTANCE := KingdomDataService();
        LogInfo("data_service", "Created new KingdomDataService instance");
    endif

    return KINGDOM_DATA_SERVICE_INSTANCE;
endfunction

// ==========================================================================
// CONVENIENCE FUNCTIONS FOR COMMON OPERATIONS
// ==========================================================================

function GetActiveGuildsOptimized()
    var data_service := GetKingdomDataService();
    return data_service.GetActiveGuilds();
endfunction

function GetGuildTerritoriesOptimized(guild_name)
    var data_service := GetKingdomDataService();
    return data_service.GetGuildTerritories(guild_name);
endfunction

function GetFactionResourcesOptimized(faction_name)
    var data_service := GetKingdomDataService();
    return data_service.GetFactionResources(faction_name);
endfunction

// CalculateUpkeepCostOptimized moved to upkeep controller for better performance