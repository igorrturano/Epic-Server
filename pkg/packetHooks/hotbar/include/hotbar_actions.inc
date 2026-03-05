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
include ":abilities:AbilityRegistryBase";
include ":abilities:controllers/AbilityMonitor";
include ":abilities:models/Ability";
include ":charactercreation:habilidades";

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

    if (skill_value <= 0)
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
        SendSysMessageEx(who, $"[HAB_CD] {ability.GetName()}|{ability.GetAbilityCooldown()}|{ability.GetIcon()}", SSM_INFO);
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
    var chardata := GetObjProperty(who, "chardata");
    var player_magia := "";

    if (chardata)
        player_magia := chardata.magia;
    endif

    var spell_configs := array{};
    var spell_types := array{};

    var has_natureza_arcana := (chardata.trait == "Natureza Arcana");

    if (has_natureza_arcana)
        spell_configs.append(":spells:config/cantripspells");
        spell_types.append(SPELL_TYPE_CANTRIP);
    endif

    if (player_magia == "Shaman")
        spell_configs.append(":spells:config/shamanspells");
        spell_types.append(SPELL_TYPE_SHAMAN);
    elseif (player_magia == "spellcaster" || player_magia == "Arqueiro Arcano")
        spell_configs.append(":spells:config/cantripspells");
        spell_types.append(SPELL_TYPE_CANTRIP);
        spell_configs.append(":spells:config/spellcaster");
        spell_types.append(SPELL_TYPE_MAGE);
    elseif (player_magia == "Druid")
        spell_configs.append(":spells:config/druidspells");
        spell_types.append(SPELL_TYPE_DRUID);
    elseif (player_magia == "Sacerdote")
        spell_configs.append(":spells:config/priestspells");
        spell_types.append(SPELL_TYPE_PRIEST);
    endif

    if (TemHabilidade(who, "Consagracao Divina"))
        spell_configs.append(":spells:config/paladinspells");
        spell_types.append(SPELL_TYPE_PALADIN);
    endif
    if (MobileWrapper(who).HasAbility(ABILITY_ID::SIGILO_DE_PROTECAO))
        spell_configs.append(":spells:config/demonhunterspells");
        spell_types.append(SPELL_TYPE_DEMONHUNTER);
    endif
    if (TemHabilidade(who, "Sensibilidade Arcana"))
        spell_configs.append(":spells:config/tricksterspells");
        spell_types.append(SPELL_TYPE_TRICKSTER);
    endif

    var found_spell_type := "";

    var i := 1;
    foreach cfg_path in spell_configs
        var spell_cfg := ReadConfigFile(cfg_path);
        if (!spell_cfg)
            i += 1;
            continue;
        endif

        var spell_elem := FindConfigElem(spell_cfg, actionId);
        if (spell_elem)
            found_spell_type := spell_types[i];
            break;
        endif
        i += 1;
    endforeach

    if (found_spell_type)
        return ExecuteSpellById(who, actionId, found_spell_type);
    else
        SendSysMessage(who, $"[BINARY] Magia ID {actionId} não encontrada");
    endif

    return 1;
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
