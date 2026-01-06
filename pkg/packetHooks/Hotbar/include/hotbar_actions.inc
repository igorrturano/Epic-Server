// ============================================================================
// Shared Hotbar Action Execution Functions
// ============================================================================
// This file contains common execution logic used by both:
// - pkg/packetHooks/Hotbar/hotbar_packets.inc (0xD7 protocol)
// - pkg/packetHooks/BFCommand/hotbarExecute.src (0xBF protocol)
//
// DRY: Single source of truth for action execution
// ============================================================================

use uo;
use os;

include ":attributes:attributes";
include ":hotbar:include/shared_constants";

// ============================================================================
// ExecuteSkillAction - Unified skill execution
// ============================================================================
// Uses attributes.cfg as single source of truth for skill data
// 
// Parameters:
//   who      - Player character
//   skill_id - Skill ID from attributes.cfg
//   verbose  - If true, sends debug messages (default: false)
//
// Returns: 1 on success, 0 on failure

function ExecuteSkillAction(who, skill_id, verbose := 0)
    // Use shared_constants.inc to get skill data from attributes.cfg (single source of truth)
    var skill_data := GetSkillDataById(skill_id);
    
    if (!skill_data || skill_data == error)
        if (verbose)
            SendSysMessage(who, $"Skill ID {skill_id} não encontrada em attributes.cfg");
        endif
        return 0;
    endif
    
    var skill_name := skill_data.name;
    var script_path := skill_data.script;
    
    // Get the player's skill value
    var skill_value := CInt(AP_GetSkill(who, skill_name));
    
    if (skill_value <= 0)
        SendSysMessage(who, $"Você não possui pontos em {skill_name}!");
        return 0;
    endif
    
    // Script path already comes from attributes.cfg via GetSkillDataById
    if (!script_path || script_path == "")
        if (verbose)
            SendSysMessage(who, $"Skill {skill_name} não possui script!");
        endif
        return 0;
    endif
    
    // Check cooldown
    var delay := GetObjProperty(who, "delayskill");
    if (delay && delay > ReadGameClock())
        SendSysMessage(who, "Você ainda está fazendo outra coisa.");
        return 0;
    endif
    
    // Set cooldown (5 seconds)
    SetObjProperty(who, "delayskill", ReadGameClock() + 5);
    
    if (verbose)
        SendSysMessage(who, $"Usando {skill_name} (valor: {skill_value})");
    endif
    
    // Execute the skill script
    var result := start_script(script_path, who);
    if (!result)
        if (verbose)
            SendSysMessage(who, "Erro ao executar skill!");
        endif
        return 0;
    endif
    
    return 1;
endfunction

// ============================================================================
// ExecuteSpecialAction - Unified special ability execution
// ============================================================================
// Executes special ability and calculates level-based cooldown
//
// Parameters:
//   who      - Player character
//
// Returns: struct{ success, cooldown_duration } 
//   success          - 1 on success, 0 on failure
//   cooldown_duration - Calculated cooldown in seconds (for binary protocol)

function ExecuteSpecialAction(who)
    // Execute special ability via existing system
    var script := Start_Script(":specialAbilities:specialAbilities", who);
    if (!script)
        SendSysMessage(who, "Erro ao executar habilidade especial.");
        return struct{ success := 0, cooldown_duration := 0 };
    endif
    
    // Calculate cooldown based on character level
    // Base cooldown: 180s, reduced by level (-30s every 4 levels, min 30s)
    var level := CInt(GetObjProperty(who, "level"));
    if (!level)
        level := 1;
    endif
    
    var cooldown_duration := 180 - (CInt(level / 4) * 30);
    if (cooldown_duration < 30)
        cooldown_duration := 30;
    endif
    
    return struct{ success := 1, cooldown_duration := cooldown_duration };
endfunction
