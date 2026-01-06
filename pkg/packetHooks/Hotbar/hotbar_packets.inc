// Hotbar Packet Handler - Server Side
// Handles custom packets 0xD7/0x50 (Request) and 0xD7/0x51 (Execute)
// Sends response packets 0xD7/0x52 (Data Response)

use uo;
use os;
use polsys;

include ":gumps:gumps";
include ":fls_core:fls_characters";
include ":attributes:attributes";
include ":charactercreation:habilidades";
include "include/icon_manager";
include "include/spell_index";      // O(1) spell lookups
include "include/shared_constants"; // Action types, skill maps
include "include/hotbar_data";      // Get*Data functions (includes command_discovery)
include "include/hotbar_write";     // Calculate*Size, Write*Section functions
include "hotbar_utils";
include "include/hotbar_actions";   // Shared execution functions (DRY)
include ":spells:include/spell_executor";

// Constants defined in shared_constants.inc

// Main packet handler - called by POL when packet 0xD7 is received
exported function HandleHotbarPacket(who, packet)
    // Packet structure:
    // 0: PacketID (0xD7)
    // 1-2: Length
    // 3-6: Player Serial (4 bytes)
    // 7-8: Subcommand (0x0050 or 0x0051)
    
    var subcommand := packet.GetInt16(7); // Offset 7-8 = subcommand
    case (subcommand)
        SUBCOMMAND_REQUEST:
            return HandleRequest(who, packet);
        SUBCOMMAND_EXECUTE:
            return HandleExecute(who, packet);
        default:
            return 0;
    endcase
endfunction

// Handle data requests from client
function HandleRequest(who, packet)
    // Packet structure:
    // 0: PacketID (0xD7)
    // 1-2: Length
    // 3-6: Player Serial (4 bytes)
    // 7-8: Subcommand (0x0050)
    // 9: Request type
    
    var serial := packet.GetInt32(3); // Offset 3-6 = player serial
    var request_type := packet.GetInt8(9); // Offset 9 = request type
    // Send all data for the visualizer (client expects unified response)
    // Individual focused packets can be enabled when client is updated
    return SendAllHotbarData(who);
endfunction

// Handle execute commands from client
function HandleExecute(who, packet)
    // Packet structure:
    // 0: PacketID (0xD7)
    // 1-2: Length
    // 3-6: Player Serial (4 bytes)
    // 7-8: Subcommand (0x0051)
    // 9: Execute type
    // 10-11: Data length
    // 12+: Data
    var action_id   := packet.GetInt8(9); // Pega os 4 bytes do Action ID
    var action_type := packet.GetInt8(10); // Pega o byte do Action Type
    case (action_type)
        ACTION_TYPE_POSTURE:
            return ExecutePosture(who, action_id);
        ACTION_TYPE_ABILITY:
            return ExecuteAbility(who, action_id);
        ACTION_TYPE_SPECIAL:
            return ExecuteSpecial(who, action_id);
        ACTION_TYPE_SPELL:
            return ExecuteSpell(who, action_id);
        ACTION_TYPE_COMMAND:
            return ExecuteCommand(who, action_id);
        ACTION_TYPE_SKILL:
            return ExecuteSkill(who, action_id);
        default:
            SendSysMessage(who, "Erro: Tipo de ação desconhecido.");
            return 1;
    endcase
    
    return 1;
endfunction

// ============================================================================
// Execution Functions (delegate to shared/hotbar_actions.inc for DRY)
// ============================================================================

// Execute habilidade
function ExecuteAbility(who, hab_name)
    // Call existing ProccessHab function
    return ProccessHab(who, hab_name);
endfunction

// Execute special - delegates to shared implementation
function ExecuteSpecial(who, special_index)
    var result := ExecuteSpecialAction(who);
    if (result.success)
        // Store cooldown using property (0xD7 protocol doesn't have binary cooldown)
        var cooldown_prop := "SpecialCooldown_" + special_index;
        SetObjProperty(who, cooldown_prop, ReadGameClock() + result.cooldown_duration);
    endif
    return result.success;
endfunction

// Execute command
function ExecuteGMCommand(who, command)
    // Remove the dot if present
    var cmd_name := command;
    if (cmd_name[1] == ".")
        cmd_name := cmd_name[2, len(cmd_name)];
    endif
    
    // Try standard location for commands
    var result := start_script(":commands:player/" + cmd_name, who);
    
    if (!result)
        return 0;
    else
        return 1;
    endif
endfunction

// // Execute skill - delegates to shared implementation
// function ExecuteSkill(who, skill_id)
//     return ExecuteSkillAction(who, skill_id, 0); // verbose=0
// endfunction

// Send all Hotbar data in a single unified packet
function SendAllHotbarData(who)
    // Collect all data
    var hab_list := GetAbilitiesData(who);
    var specials := GetSpecialsData(who);
    var magias := GetSpellsData(who);
    var commands := GetCommandsData(who);
    var skills := GetSkillsData(who);
    
    // Calculate total packet size
    var packet_size := 6; // Header: PacketID(1) + Length(2) + Subcommand(2) + DataType(1)
    
    // Add size for each data type (count + data)
    var hab_size := CalculateAbilitiesSize(hab_list);
    var spec_size := CalculateSpecialsSize(specials);
    var mag_size := CalculateSpellsSize(magias);
    var cmd_size := CalculateCommandsSize(commands);
    var skill_size := CalculateSkillsSize(skills);
    
    packet_size := packet_size + 2 + hab_size;
    packet_size := packet_size + 2 + spec_size;
    packet_size := packet_size + 2 + mag_size;
    packet_size := packet_size + 2 + cmd_size;
    packet_size := packet_size + 2 + skill_size;
    
    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    
    // Header
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_RESPONSE);
    packet.SetInt8(5, 0xFF); // Unified packet marker
    
    var offset := 6;
    
    // 1. Habilidades
    offset := SetInt16BE(packet, offset, hab_list.size());
    foreach item in hab_list
        offset := WriteStringBE(packet, offset, item.name);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := WriteStringBE(packet, offset, item.classe);
        offset := SetInt16BE(packet, offset, item.required_level);
        packet.SetInt8(offset, item.is_active ? 1 : 0); offset += 1;
        packet.SetInt8(offset, item.is_purchased ? 1 : 0); offset += 1;
        offset := SetInt16BE(packet, offset, item.iconid);
        offset := SetInt16BE(packet, offset, item.hue);
        offset := SetInt16BE(packet, offset, item.stamina_cost);
        offset := SetInt16BE(packet, offset, item.mana_cost);
        offset := SetInt16BE(packet, offset, item.delay);
        offset := WriteStringBE(packet, offset, item.requisite);
        offset := SetInt16BE(packet, offset, item.duration);
        offset := WriteStringBE(packet, offset, item.script);
    endforeach
    
    // 2. Specials
    offset := SetInt16BE(packet, offset, specials.size());
    foreach item in specials
        offset := WriteStringBE(packet, offset, item.name);
        offset := WriteStringBE(packet, offset, item.classe);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := SetInt16BE(packet, offset, item.iconid);
        offset := SetInt16BE(packet, offset, item.hue);
        packet.SetInt8(offset, item.index); offset += 1;
        offset := SetInt16BE(packet, offset, CInt(item.delay));
        offset := SetInt16BE(packet, offset, CInt(item.remaining));
    endforeach
    
    // 3. Magias
    offset := SetInt16BE(packet, offset, magias.size());
    foreach item in magias
        offset := WriteStringBE(packet, offset, item.name);
        offset := WriteStringBE(packet, offset, CStr(item.circle));
        offset := WriteStringBE(packet, offset, item.desc);
        offset := WriteStringBE(packet, offset, item.school);
        offset := SetInt16BE(packet, offset, item.iconid);
        offset := SetInt16BE(packet, offset, 0);  // hue
        offset := SetInt16BE(packet, offset, item.mana_cost);
        offset := SetInt16BE(packet, offset, 0);  // min_skill
        offset := SetInt16BE(packet, offset, 0);  // reagents length
        offset := SetInt16BE(packet, offset, item.id);
        var cmd_prefix := item.command_prefix ? item.command_prefix : "m";
        offset := WriteStringBE(packet, offset, cmd_prefix);
    endforeach
    
    // 4. Commands
    offset := SetInt16BE(packet, offset, commands.size());
    foreach item in commands
        offset := WriteStringBE(packet, offset, item.command);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := SetInt16BE(packet, offset, item.iconid);
        packet.SetInt8(offset, item.index); offset += 1;
        offset := WriteStringBE(packet, offset, item.category);
    endforeach
    
    // 5. Skills
    offset := SetInt16BE(packet, offset, skills.size());
    foreach item in skills
        offset := WriteStringBE(packet, offset, item.name);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := SetInt16BE(packet, offset, item.hue);
        offset := SetInt16BE(packet, offset, item.iconid);
        packet.SetInt8(offset, item.index); offset += 1;
        offset := SetInt16BE(packet, offset, item.current_value);
        offset := SetInt16BE(packet, offset, item.base_value);
        offset := SetInt16BE(packet, offset, item.cap);
        packet.SetInt8(offset, item.lock); offset += 1;
    endforeach
    
    packet.SendPacket(who);
    return 1;
endfunction

// ============================================================================
// Helper functions for packet writing are now in include files:
// - include/hotbar_data.inc: GetSpecialsData, GetCommandsData, GetSpellsData, GetAbilitiesData, GetSkillsData
// - include/hotbar_write.inc: Calculate*Size, Write*Section, SetInt16BE, WriteStringBE
// ============================================================================

exported function HandleHotbarExecute(who, byref packet)
    if (who.dead)
        Message("You cannot do that while dead!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
        return 1;
    endif
    // Parse packet structure:
    // Byte 0: 0xBF
    // Bytes 1-2: Length
    // Byte 3: Padding
    // Bytes 4-5: Subcommand (0x0099) - handled by POL
    // Bytes 6-9: Action ID (4 bytes)
    // Byte 10: Action Type (1 byte)
    var actionId   := packet.GetInt8(9);  // Lê apenas 1 byte
    var actionType := packet.GetInt8(10); // Lê apenas 1 byte    print($"Action ID: {actionId}, Action Type: {actionType}");
    // Dispatch based on action type
    case (actionType)
        ACTION_TYPE_POSTURE: return ExecutePosture(who, actionId);
        ACTION_TYPE_ABILITY: return ExecuteAbility(who, actionId);
        ACTION_TYPE_SPECIAL: return ExecuteSpecial(who, actionId);
        ACTION_TYPE_SPELL:   return ExecuteSpell(who, actionId);
        ACTION_TYPE_COMMAND: return ExecuteCommand(who, actionId);
        ACTION_TYPE_SKILL:   return ExecuteSkill(who, actionId);
        default:
            SendSysMessage(who, "Erro: Tipo de ação desconhecido.");
            return false;
    endcase
    
    return true;
endfunction

// ============================================================================
// Action Handlers
// ============================================================================

function ExecutePosture(who, actionId)
    
    // Map actionId to postura type
    // 0 = Slash, 1 = Piercing, 2 = Blunt
    var posturaName;
    var posturaNamePT; // Nome em português para feedback
    
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
    
    
    // Execute postura change via existing system
    var script := Start_Script(":equipsys:golpe", {who, posturaName});
    if (!script)
        SendSysMessage(who, "Erro ao executar postura.");
        return 0;
    endif
    
    // Send feedback message in Portuguese for PosturaManager detection
    // This message will be detected by PosturaManager to show red mask on active postura
    SendSysMessage(who, $"Você adota postura {posturaNamePT}");
    
    return 1;
endfunction

function ExecuteSpell(who, actionId)
    
    // Get player's class and magia to prioritize correct spell configs
    var chardata := GetObjProperty(who, "chardata");
    var player_magia := "";
    
    if (chardata)
        player_magia := chardata.magia;
    endif
    
    // Build prioritized spell config list based on player type
    // Order matters! First match wins.
    var spell_configs := array{};
    var spell_types := array{};  // Parallel array for spell type
    
    // Check for Natureza Arcana trait (gives access to cantrips regardless of class)
    var has_natureza_arcana := (chardata.trait == "Natureza Arcana");
    
    // IMPORTANT: Cantrips FIRST for anyone with Natureza Arcana trait
    if (has_natureza_arcana)
        spell_configs.append(":spells:config/cantripspells");
        spell_types.append(SPELL_TYPE_CANTRIP);
    endif

    // Then add class-specific configs
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
    
    // Ability-based spell access
    if (TemHabilidade(who, "Consagracao Divina"))
        spell_configs.append(":spells:config/paladinspells");
        spell_types.append(SPELL_TYPE_PALADIN);
    endif
    if (TemHabilidade(who, "Sigilo de Protecao"))
        spell_configs.append(":spells:config/demonhunterspells");
        spell_types.append(SPELL_TYPE_DEMONHUNTER);
    endif
    if (TemHabilidade(who, "Sensibilidade Arcana"))
        spell_configs.append(":spells:config/tricksterspells");
        spell_types.append(SPELL_TYPE_TRICKSTER);
    endif
    
    var found_spell_type := "";
    
    // Search configs in priority order - first match wins
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
        // Execute via unified spell executor (by ID for efficiency)
        return ExecuteSpellById(who, actionId, found_spell_type);
    else
        SendSysMessage(who, $"[BINARY] Magia ID {actionId} não encontrada");
    endif
    
    return 1;
endfunction

function ExecuteCommand(who, actionId)
    // Use command_discovery.inc to get command by index (single source of truth)
    // actionId is the sequential index (1, 2, 3...) sent by client
    var cmd_data := GetCommandByIndex(actionId);
    
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
    // DRY: Use shared implementation from hotbar_actions.inc
    // verbose=1 because 0xBF protocol is used for active hotbar clicks
    return ExecuteSkillAction(who, actionId, 1);
endfunction
