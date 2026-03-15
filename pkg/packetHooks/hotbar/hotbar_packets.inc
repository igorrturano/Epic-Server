include "include/shared_constants"; // Action types, skill maps
include "include/hotbar_actions";   // Shared execution functions (DRY)
include ":spells:include/spell_executor";
include ":hotbar:include/command_discovery"; // For tooltip desc lookup
include ":hotbar:include/hotbar_write"; // For SetInt16BE in tooltip response

exported function HandleHotbarPacket(who, packet)
    var subcommand := packet.GetInt16(7);
    case (subcommand)
        SUBCOMMAND_REQUEST:
            return HandleRequest(who, packet);
        SUBCOMMAND_EXECUTE:
            return HandleExecute(who, packet);
        SUBCOMMAND_TOOLTIP_REQUEST:
            return HandleTooltipRequest(who, packet);
        default:
            return 0;
    endcase
endfunction

// Handle data requests from client - spawn per-type sender scripts
function HandleRequest(who, packet)
    Start_Script(":hotbar:scripts/send_abilities", who);
    Start_Script(":hotbar:scripts/send_combat_hotkeys", who);
    Start_Script(":hotbar:scripts/send_spells", who);
    Start_Script(":hotbar:scripts/send_commands", who);
    Start_Script(":hotbar:scripts/send_skills", who);
    return 1;
endfunction

// Handle execute commands from client
function HandleExecute(who, packet)
    if (who.dead)
        Message("You cannot do that while dead!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
        return 1;
    endif

    var actionId   := packet.GetInt16(9);
    var actionType := packet.GetInt8(11);

    case (actionType)
        ACTION_TYPE_POSTURE: return ExecutePosture(who, actionId);
        ACTION_TYPE_ABILITY: return ExecuteAbility(who, actionId);
        ACTION_TYPE_SPECIAL: return ExecuteCombatHotkey(who, actionId);
        ACTION_TYPE_SPELL:   return ExecuteSpell(who, actionId);
        ACTION_TYPE_COMMAND: return ExecuteCommand(who, actionId);
        ACTION_TYPE_SKILL:   return ExecuteSkill(who, actionId);
        default:
            SendSysMessage(who, "Erro: Tipo de ação desconhecido.");
            return false;
    endcase

    return true;
endfunction

// Handle tooltip requests from client (0x60)
// Client sends: [0xD7][size][0x0060][actionType:1][actionId:2]
// Server responds: [0xD7][size][0x0061][actionType:1][actionId:2][desc_string]
function HandleTooltipRequest(who, packet)
    var action_type := packet.GetInt8(9);
    var action_id := packet.GetInt16(10);

    var desc := "";

    case (action_type)
        ACTION_TYPE_COMMAND:
            var cmd_data := GetCommandByIndex(action_id, who);
            if (cmd_data && cmd_data != error)
                var cmd_name := cmd_data.command;
                if (cmd_name[1] == ".")
                    cmd_name := cmd_name[2, len(cmd_name)];
                endif
                desc := GetCommandDescription(cmd_name);
            endif
        ACTION_TYPE_SPELL:
            desc := GetSpellDescription(action_id);
        default:
            desc := "";
    endcase

    // Send tooltip response (0x61)
    var desc_len := len(desc);
    var packet_size := 5 + 1 + 2 + 2 + desc_len;
    var response := CreatePacket(PACKET_EXTENDED, packet_size);

    response.SetInt8(0, PACKET_EXTENDED);
    response.SetInt16(1, packet_size);
    response.SetInt16(3, SUBCOMMAND_TOOLTIP_RESPONSE);
    response.SetInt8(5, action_type);
    SetInt16BE(response, 6, action_id);
    SetInt16BE(response, 8, desc_len);
    response.SetString(10, desc, 0);

    response.SendPacket(who);
    return 1;
endfunction

// Look up spell description by ID across all spell configs
function GetSpellDescription(spell_id)
    var spell_configs := array{
        ":spells:config/cantrips",
        ":spells:config/evocation",
        ":spells:config/necromancy",
        ":spells:config/illusion",
        ":spells:config/swordmage",
        ":spells:config/priest",
        ":spells:config/shaman",
        ":spells:config/druid",
        ":spells:config/paladin",
        ":spells:config/demonhunter",
        ":spells:config/trickster"
    };

    foreach cfg_path in spell_configs
        var spell_cfg := ReadConfigFile(cfg_path);
        if (!spell_cfg)
            continue;
        endif
        var spell_elem := FindConfigElem(spell_cfg, spell_id);
        if (spell_elem)
            var desc := GetConfigString(spell_elem, "Desc");
            if (desc)
                return desc;
            endif
            return GetConfigString(spell_elem, "Name");
        endif
    endforeach

    return "";
endfunction
