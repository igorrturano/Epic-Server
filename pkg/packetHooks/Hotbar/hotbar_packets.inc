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
    
    var serial := packet.GetInt32(3); // Offset 3-6 = player serial
    var execute_type := packet.GetInt8(9); // Offset 9 = execute type
    var data_length := packet.GetInt16(10); // Offset 10-11 = data length
    var data := packet.GetString(12, data_length); // Offset 12+ = data (ASCII)
    
    case (execute_type)
        EXECUTE_ABILITY:
            return ExecuteAbility(who, data);
        EXECUTE_SPECIAL:
            return ExecuteSpecial(who, CInt(data));
        EXECUTE_COMMAND:
            return ExecuteGMCommand(who, data);
        EXECUTE_SKILL:
            return ExecuteSkill(who, CInt(data));
        default:
            return 0;
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

// Execute skill - delegates to shared implementation
function ExecuteSkill(who, skill_id)
    return ExecuteSkillAction(who, skill_id, 0); // verbose=0
endfunction

// ============================================================================
// PHASE 2: Focused Packet Sending Functions
// ============================================================================
// Send only the requested data type to reduce bandwidth usage

function SendAbilitiesPacket(who)
    var hab_list := GetAbilitiesData(who);
    var hab_count := hab_list.size();
    var hab_size := CalculateAbilitiesSize(hab_list);
    var packet_size := 8 + hab_size;
    
    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_ABILITIES_RESPONSE);
    packet.SetInt8(5, REQUEST_ABILITIES);
    
    var offset := 6;
    offset := SetInt16BE(packet, offset, hab_count);
    
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
    
    packet.SendPacket(who);
    return 1;
endfunction

function SendSpecialsPacket(who)
    var specials := GetSpecialsData(who);
    var spec_count := specials.size();
    var spec_size := CalculateSpecialsSize(specials);
    var packet_size := 8 + spec_size;
    
    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_SPECIALS_RESPONSE);
    packet.SetInt8(5, REQUEST_SPECIALS);
    
    var offset := 6;
    offset := SetInt16BE(packet, offset, spec_count);
    
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
    
    packet.SendPacket(who);
    return 1;
endfunction

function SendSpellsPacket(who)
    var magias := GetSpellsData(who);
    var mag_count := magias.size();
    var mag_size := CalculateSpellsSize(magias);
    var packet_size := 8 + mag_size;
    
    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_SPELLS_RESPONSE);
    packet.SetInt8(5, REQUEST_SPELLS);
    
    var offset := 6;
    offset := SetInt16BE(packet, offset, mag_count);
    
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
    
    packet.SendPacket(who);
    return 1;
endfunction

function SendCommandsPacket(who)
    var commands := GetCommandsData(who);
    var cmd_count := commands.size();
    var cmd_size := CalculateCommandsSize(commands);
    var packet_size := 8 + cmd_size;
    
    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_RESPONSE);
    packet.SetInt8(5, REQUEST_COMMANDS);
    
    var offset := 6;
    offset := SetInt16BE(packet, offset, cmd_count);
    
    foreach item in commands
        offset := WriteStringBE(packet, offset, item.command);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := SetInt16BE(packet, offset, item.iconid);
        packet.SetInt8(offset, item.index); offset += 1;
        offset := WriteStringBE(packet, offset, item.category);
    endforeach
    
    packet.SendPacket(who);
    return 1;
endfunction

function SendSkillsPacket(who)
    var skills := GetSkillsData(who);
    var skill_count := skills.size();
    var skill_size := CalculateSkillsSize(skills);
    var packet_size := 8 + skill_size;
    
    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_SKILLS_RESPONSE);
    packet.SetInt8(5, REQUEST_SKILLS);
    
    var offset := 6;
    offset := SetInt16BE(packet, offset, skill_count);
    
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
