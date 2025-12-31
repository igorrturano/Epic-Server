// ============================================================================
// GMHab Utilities - Shared functions for GMHab system
// ============================================================================
// Purpose: Centralize common logic to follow DRY principle
// Used by: gmhab_packets.inc, hotbarExecute.src
// ============================================================================

use uo;
use os;
use cfgfile;

// ============================================================================
// GetFilteredHabilidades - Returns list of habilidades eligible for player
// ============================================================================
// Applies filtering logic:
// 1. Active abilities only (uso == "Ativa")
// 2. Class and level requirements
// 3. Purchased abilities (always included)
//
// Returns: Array of habilidade names (strings) in config file order
// ============================================================================
function GetFilteredHabilidades(who)
    // Get player data
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        return array{};
    endif
    
    var char_class := chardata.classe;
    var char_level := CInt(GetObjProperty(who, "Level"));
    if (!char_level)
        char_level := 1;
    endif
    
    // Get purchased habilidades
    var myhabs := GetObjProperty(who, "myhabs");
    if (!myhabs)
        myhabs := dictionary;
    endif
    
    // Load config
    var hab_cfg := ReadConfigFile(":charactercreation:config/habilidades");
    if (!hab_cfg)
        return array{};
    endif
    
    // Filter habilidades
    var all_keys := GetConfigStringKeys(hab_cfg);
    var filtered_habs := array{};
    
    foreach hab_name in all_keys
        var habelem := FindConfigElem(hab_cfg, hab_name);
        if (!habelem)
            continue;
        endif
        
        var classes := GetConfigStringDictionary(habelem, "classe");
        var uso := GetConfigString(habelem, "uso");
        
        // Skip passive abilities - only send active ones
        if (uso != "Ativa")
            continue;
        endif
        
        // Check if eligible for this character's class
        var is_eligible := 0;
        
        if (classes[char_class] != error)
            var required_level := CInt(classes[char_class]);
            if (char_level >= required_level)
                is_eligible := 1;
            endif
        endif
        
        // Check if purchased (always show purchased abilities)
        foreach key in (myhabs.keys())
            if (Lower(myhabs[key]) == Lower(hab_name))
                is_eligible := 1; // Always eligible if purchased
                break;
            endif
        endforeach
        
        // Add to filtered list if eligible
        if (is_eligible)
            filtered_habs.append(hab_name);
        endif
    endforeach
    
    return filtered_habs;
endfunction

// ============================================================================
// GetHabilidadeBySequentialId - Get habilidade name by sequential ID
// ============================================================================
// Uses filtered list to map sequential ID (1-based) to habilidade name
// Returns: Habilidade name (string) or error
// ============================================================================
function GetHabilidadeBySequentialId(who, actionId)
    var filtered_habs := GetFilteredHabilidades(who);
    
    if (actionId < 1 || actionId > filtered_habs.size())
        return error{"errortext" := "ActionID out of range"};
    endif
    
    return filtered_habs[actionId]; // POL arrays are 1-based
endfunction
