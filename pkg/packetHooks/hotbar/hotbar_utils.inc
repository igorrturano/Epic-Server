// ============================================================================
// Hotbar Utilities - Shared functions for Hotbar system
// ============================================================================
// Purpose: Centralize common logic to follow DRY principle
// Used by: hotbar_packets.inc, hotbarExecute.src
// ============================================================================

use uo;
use os;
use cfgfile;

// ============================================================================
// GetFilteredAbilities - Returns list of abilities eligible for player
// ============================================================================
// Filtering logic:
// - GM (cmdlevel > 1): Show ALL active abilities from config
// - Player (cmdlevel <= 1): Show ONLY purchased abilities (from myhabs)
//
// Returns: Array of ability names (strings) in config file order
// ============================================================================
function GetFilteredAbilities(who)
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
    
    // Check if GM (cmdlevel > 1)
    var is_gm := (who.cmdlevel > 1);
    
    // Load config
    var hab_cfg := ReadConfigFile(":abilities:configs/habilidades");
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
        
        var is_eligible := 0;
        
        // GM: Show all active abilities for their class (regardless of purchase)
        if (is_gm)
            if (classes[char_class] != error)
                var required_level := CInt(classes[char_class]);
                if (char_level >= required_level)
                    is_eligible := 1;
                endif
            endif
        else
            // Player: Show ONLY purchased abilities
            foreach key in (myhabs.keys())
                if (Lower(myhabs[key]) == Lower(hab_name))
                    is_eligible := 1;
                    break;
                endif
            endforeach
        endif
        
        // Add to filtered list if eligible
        if (is_eligible)
            filtered_habs.append(hab_name);
        endif
    endforeach
    
    return filtered_habs;
endfunction

// ============================================================================
// GetAbilityBySequentialId - Get ability name by sequential ID
// ============================================================================
// Uses filtered list to map sequential ID (1-based) to ability name
// Returns: Ability name (string) or error
// ============================================================================
function GetAbilityBySequentialId(who, actionId)
    var filtered_habs := GetFilteredAbilities(who);
    
    if (actionId < 1 || actionId > filtered_habs.size())
        return error{"errortext" := "ActionID out of range"};
    endif
    
    return filtered_habs[actionId]; // POL arrays are 1-based
endfunction
