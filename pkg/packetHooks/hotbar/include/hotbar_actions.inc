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
include ":hotbar:include/hotbar_exec_helpers";
include ":hotbar:hotbar_utils";
include ":hotbar:include/command_discovery";
include ":wrappers:Objects/Model/MobileWrapper";
include ":spells:SpellRegistry";
include ":abilities:AbilityRegistry";
include ":abilities:controllers/AbilityMonitor";
include ":abilities:models/Ability";
include ":charactercreation:habilidades";
include ":hotbar:include/cooldown_packet";

// ============================================================================
// ExecuteSkillAction - Unified skill execution
// ============================================================================
function ExecuteSkillAction(who, skill_id, verbose := 0)
    var skill_data := GetSkillDataById(skill_id);

    if (!skill_data || skill_data == error)
        if (verbose)
            SendSysMessage(who, $"Skill ID {skill_id} não encontrada em attributes.cfg");
        endif
        return 0;
    endif

    var skill_name := skill_data.name;
    var script_path := skill_data.script;

    var skill_value := CInt(AP_GetSkill(who, skill_name));

    if (skill_value <= 0 && GetObjProperty(who, "chardata").Skills[skill_name] == 0)
        SendSysMessage(who, $"Você não possui pontos em {skill_name}!");
        return 0;
    endif

    if (!script_path || script_path == "")
        if (verbose)
            SendSysMessage(who, $"Skill {skill_name} não possui script!");
        endif
        return 0;
    endif

    var delay := GetObjProperty(who, "delayskill");
    if (delay && delay > ReadGameClock())
        SendSysMessage(who, "Você ainda está fazendo outra coisa.");
        return 0;
    endif

    SetObjProperty(who, "delayskill", ReadGameClock() + 5);

    if (verbose)
        SendSysMessage(who, $"Usando {skill_name} (valor: {skill_value})");
    endif

    var result := start_script(script_path, who);
    if (!result)
        if (verbose)
            SendSysMessage(who, "Erro ao executar skill!");
        endif
        return 0;
    endif

    return 1;
endfunction

function ExecuteSpecialAction(who)
    var script := Start_Script(":specialAbilities:specialAbilities", who);
    if (!script)
        SendSysMessage(who, "Erro ao executar habilidade especial.");
        return struct{ success := 0, cooldown_duration := 0 };
    endif

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

// ============================================================================
// Execution Functions
// ============================================================================

function ExecuteAbility(who, actionId)
    var hab_id := GetAbilityBySequentialId(who, actionId);
    if (hab_id == error)
        SendSysMessage(who, "Habilidade não encontrada.");
        return 0;
    endif

    var mobileWrapper := MobileWrapper(who);
    var abilityId := CInt(hab_id);
    var ability := Ability(abilityId);
    if (!ability.GetName())
        SendSysMessage(who, "Habilidade inválida.");
        return 0;
    endif

    var monitor := AbilityMonitor(ability);
    var result := monitor.ActivateOn(mobileWrapper);

    if (result.errortext)
        return 0;
    endif

    if (ability.GetAbilityCooldown() > 0)
        SendCooldownPacket(who, ability.GetName(), ability.GetAbilityCooldown(), ability.GetIcon(), COOLDOWN_TYPE_ABILITY);
    endif

    return 1;
endfunction

function ExecuteCombatHotkey(who, actionId)
    var hotkey_data := GetCombatHotkeyByIndex(who, actionId);

    if (hotkey_data.errortext)
        SendSysMessage(who, "Erro: Combat hotkey inválido.");
        return 0;
    endif

    var result := Start_Script(hotkey_data.script_path, who);
    if (!result)
        SendSysMessage(who, "Erro ao executar ação de combate.");
        return 0;
    endif

    if (hotkey_data.command_type == 0 && hotkey_data.delay > 0)
        var cooldown_prop := "SpecialCooldown_" + actionId;
        SetObjProperty(who, cooldown_prop, ReadGameClock() + hotkey_data.delay);
    endif

    return 1;
endfunction

function ExecutePosture(who, actionId)
    var posturaName;
    var posturaNamePT;

    case (actionId)
        0:
            posturaName := "slash";
            posturaNamePT := "Cortante";
        1:
            posturaName := "piercing";
            posturaNamePT := "Perfurante";
        2:
            posturaName := "blunt";
            posturaNamePT := "Contusivo";
        default:
            SendSysMessage(who, "Erro: Postura inválida.");
            return 1;
    endcase

    var script := Start_Script(":equipsys:golpe", {who, posturaName});
    if (!script)
        SendSysMessage(who, "Erro ao executar postura.");
        return 0;
    endif

    SendSysMessage(who, $"Você adota postura {posturaNamePT}");

    return 1;
endfunction

function ExecuteSpell(who, actionId)
    // With unique spell IDs, use SpellRegistry for O(1) lookup
    var spell_type := GetSpellTypeFromId(actionId);
    if (!spell_type)
        SendSysMessage(who, $"[BINARY] Magia ID {actionId} não encontrada");
        return 1;
    endif

    return ExecuteSpellById(who, actionId, spell_type);
endfunction

function ExecuteCommand(who, actionId)
    var cmd_data := GetCommandByIndex(actionId, who);

    if (cmd_data && cmd_data != error)
        var result := Start_Script(cmd_data.script_path, who);
        if (!result)
            SendSysMessage(who, "Erro ao executar comando.");
            return 0;
        endif
        return 1;
    endif

    SendSysMessage(who, $"Comando não encontrado (Index: {actionId})");
    return 0;
endfunction

function ExecuteSkill(who, actionId)
    return ExecuteSkillAction(who, actionId, 1);
endfunction
