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
    packet.SetInt8(offset, CInt(hab_count / 256)); offset += 1;
    packet.SetInt8(offset, hab_count % 256); offset += 1;
    
    foreach item in hab_list
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        var classe_len := len(item.classe);
        packet.SetInt8(offset, CInt(classe_len / 256)); offset += 1;
        packet.SetInt8(offset, classe_len % 256); offset += 1;
        packet.SetString(offset, item.classe, 0); offset += classe_len;
        
        packet.SetInt8(offset, CInt(item.required_level / 256)); offset += 1;
        packet.SetInt8(offset, item.required_level % 256); offset += 1;
        packet.SetInt8(offset, item.is_active ? 1 : 0); offset += 1;
        packet.SetInt8(offset, item.is_purchased ? 1 : 0); offset += 1;
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        packet.SetInt8(offset, CInt(item.hue / 256)); offset += 1;
        packet.SetInt8(offset, item.hue % 256); offset += 1;
        packet.SetInt8(offset, CInt(item.stamina_cost / 256)); offset += 1;
        packet.SetInt8(offset, item.stamina_cost % 256); offset += 1;
        packet.SetInt8(offset, CInt(item.mana_cost / 256)); offset += 1;
        packet.SetInt8(offset, item.mana_cost % 256); offset += 1;
        packet.SetInt8(offset, CInt(item.delay / 256)); offset += 1;
        packet.SetInt8(offset, item.delay % 256); offset += 1;
        
        var req_len := len(item.requisite);
        packet.SetInt8(offset, CInt(req_len / 256)); offset += 1;
        packet.SetInt8(offset, req_len % 256); offset += 1;
        packet.SetString(offset, item.requisite, 0); offset += req_len;
        
        packet.SetInt8(offset, CInt(item.duration / 256)); offset += 1;
        packet.SetInt8(offset, item.duration % 256); offset += 1;
        
        var script_len := len(item.script);
        packet.SetInt8(offset, CInt(script_len / 256)); offset += 1;
        packet.SetInt8(offset, script_len % 256); offset += 1;
        packet.SetString(offset, item.script, 0); offset += script_len;
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
    packet.SetInt8(offset, CInt(spec_count / 256)); offset += 1;
    packet.SetInt8(offset, spec_count % 256); offset += 1;
    
    foreach item in specials
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        var classe_len := len(item.classe);
        packet.SetInt8(offset, CInt(classe_len / 256)); offset += 1;
        packet.SetInt8(offset, classe_len % 256); offset += 1;
        packet.SetString(offset, item.classe, 0); offset += classe_len;
        
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        packet.SetInt8(offset, CInt(item.hue / 256)); offset += 1;
        packet.SetInt8(offset, item.hue % 256); offset += 1;
        packet.SetInt8(offset, item.index); offset += 1;
        
        var delay_value := CInt(item.delay);
        packet.SetInt8(offset, CInt(delay_value / 256)); offset += 1;
        packet.SetInt8(offset, delay_value % 256); offset += 1;
        
        var remaining_value := CInt(item.remaining);
        packet.SetInt8(offset, CInt(remaining_value / 256)); offset += 1;
        packet.SetInt8(offset, remaining_value % 256); offset += 1;
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
    packet.SetInt8(offset, CInt(mag_count / 256)); offset += 1;
    packet.SetInt8(offset, mag_count % 256); offset += 1;
    
    foreach item in magias
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        var circle_str := CStr(item.circle);
        var circle_len := len(circle_str);
        packet.SetInt8(offset, CInt(circle_len / 256)); offset += 1;
        packet.SetInt8(offset, circle_len % 256); offset += 1;
        packet.SetString(offset, circle_str, 0); offset += circle_len;
        
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        var school_len := len(item.school);
        packet.SetInt8(offset, CInt(school_len / 256)); offset += 1;
        packet.SetInt8(offset, school_len % 256); offset += 1;
        packet.SetString(offset, item.school, 0); offset += school_len;
        
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, CInt(item.mana_cost / 256)); offset += 1;
        packet.SetInt8(offset, item.mana_cost % 256); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, CInt(item.id / 256)); offset += 1;
        packet.SetInt8(offset, item.id % 256); offset += 1;
        
        var cmd_prefix := item.command_prefix ? item.command_prefix : "m";
        var prefix_len := len(cmd_prefix);
        packet.SetInt8(offset, CInt(prefix_len / 256)); offset += 1;
        packet.SetInt8(offset, prefix_len % 256); offset += 1;
        packet.SetString(offset, cmd_prefix, 0); offset += prefix_len;
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
    packet.SetInt8(offset, CInt(cmd_count / 256)); offset += 1;
    packet.SetInt8(offset, cmd_count % 256); offset += 1;
    
    foreach item in commands
        var cmd_len := len(item.command);
        packet.SetInt8(offset, CInt(cmd_len / 256)); offset += 1;
        packet.SetInt8(offset, cmd_len % 256); offset += 1;
        packet.SetString(offset, item.command, 0); offset += cmd_len;
        
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        packet.SetInt8(offset, item.index); offset += 1;
        
        var cat_len := len(item.category);
        packet.SetInt8(offset, CInt(cat_len / 256)); offset += 1;
        packet.SetInt8(offset, cat_len % 256); offset += 1;
        packet.SetString(offset, item.category, 0); offset += cat_len;
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
    packet.SetInt8(offset, CInt(skill_count / 256)); offset += 1;
    packet.SetInt8(offset, skill_count % 256); offset += 1;
    
    foreach item in skills
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        packet.SetInt8(offset, CInt(item.hue / 256)); offset += 1;
        packet.SetInt8(offset, item.hue % 256); offset += 1;
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        packet.SetInt8(offset, item.index); offset += 1;
        packet.SetInt8(offset, CInt(item.current_value / 256)); offset += 1;
        packet.SetInt8(offset, item.current_value % 256); offset += 1;
        packet.SetInt8(offset, CInt(item.base_value / 256)); offset += 1;
        packet.SetInt8(offset, item.base_value % 256); offset += 1;
        packet.SetInt8(offset, CInt(item.cap / 256)); offset += 1;
        packet.SetInt8(offset, item.cap % 256); offset += 1;
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
    var magias := GetSpellsData(who);  // ✅ Get magias from grimório
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
    
    // Write all 5 counts inline (helper functions don't work due to POL packet reference issues)
    
    // 1. Habilidades count
    var hab_count := hab_list.size();
    packet.SetInt8(offset, CInt(hab_count / 256)); offset += 1;
    packet.SetInt8(offset, hab_count % 256); offset += 1;
    
    // Write habilidades data
    foreach item in hab_list
        // name
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        // desc (description)
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        // classe (class)
        var classe_len := len(item.classe);
        packet.SetInt8(offset, CInt(classe_len / 256)); offset += 1;
        packet.SetInt8(offset, classe_len % 256); offset += 1;
        packet.SetString(offset, item.classe, 0); offset += classe_len;
        
        // required_level
        packet.SetInt8(offset, CInt(item.required_level / 256)); offset += 1;
        packet.SetInt8(offset, item.required_level % 256); offset += 1;
        
        // is_active (bool)
        if (item.is_active)
            packet.SetInt8(offset, 1); offset += 1;
        else
            packet.SetInt8(offset, 0); offset += 1;
        endif
        
        // is_purchased (bool)
        if (item.is_purchased)
            packet.SetInt8(offset, 1); offset += 1;
        else
            packet.SetInt8(offset, 0); offset += 1;
        endif
        
        // iconid
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        
        // hue
        packet.SetInt8(offset, CInt(item.hue / 256)); offset += 1;
        packet.SetInt8(offset, item.hue % 256); offset += 1;
        
        // stamina_cost
        packet.SetInt8(offset, CInt(item.stamina_cost / 256)); offset += 1;
        packet.SetInt8(offset, item.stamina_cost % 256); offset += 1;
        
        // mana_cost
        packet.SetInt8(offset, CInt(item.mana_cost / 256)); offset += 1;
        packet.SetInt8(offset, item.mana_cost % 256); offset += 1;
        
        // delay
        packet.SetInt8(offset, CInt(item.delay / 256)); offset += 1;
        packet.SetInt8(offset, item.delay % 256); offset += 1;
        
        // requisite
        var req_len := len(item.requisite);
        packet.SetInt8(offset, CInt(req_len / 256)); offset += 1;
        packet.SetInt8(offset, req_len % 256); offset += 1;
        packet.SetString(offset, item.requisite, 0); offset += req_len;
        
        // duration
        packet.SetInt8(offset, CInt(item.duration / 256)); offset += 1;
        packet.SetInt8(offset, item.duration % 256); offset += 1;
        
        // script
        var script_len := len(item.script);
        packet.SetInt8(offset, CInt(script_len / 256)); offset += 1;
        packet.SetInt8(offset, script_len % 256); offset += 1;
        packet.SetString(offset, item.script, 0); offset += script_len;
    endforeach
    
    // 2. Specials count  
    var spec_count := specials.size();
    packet.SetInt8(offset, CInt(spec_count / 256)); offset += 1;
    packet.SetInt8(offset, spec_count % 256); offset += 1;
    
    // Write specials data
    foreach item in specials
        // name
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        // classe
        var classe_len := len(item.classe);
        packet.SetInt8(offset, CInt(classe_len / 256)); offset += 1;
        packet.SetInt8(offset, classe_len % 256); offset += 1;
        packet.SetString(offset, item.classe, 0); offset += classe_len;
        
        // desc
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        // iconid
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        
        // hue
        packet.SetInt8(offset, CInt(item.hue / 256)); offset += 1;
        packet.SetInt8(offset, item.hue % 256); offset += 1;
        
        // index
        packet.SetInt8(offset, item.index); offset += 1;
        
        // delay - Force explicit conversion to ensure proper type handling
        var delay_value := CInt(item.delay);
        var temp_delay := delay_value + 0;  // Force new integer variable
        var delay_low := temp_delay % 256;
        var delay_high := CInt(temp_delay / 256);
        
        packet.SetInt8(offset, delay_high); offset += 1;  // High byte (Big Endian)
        packet.SetInt8(offset, delay_low); offset += 1;   // Low byte
        
        // remaining - Time left on current cooldown
        var remaining_value := CInt(item.remaining);
        var temp_remaining := remaining_value + 0;  // Force new integer variable
        var remaining_low := temp_remaining % 256;
        var remaining_high := CInt(temp_remaining / 256);
        
        packet.SetInt8(offset, remaining_high); offset += 1;  // High byte (Big Endian)
        packet.SetInt8(offset, remaining_low); offset += 1;   // Low byte
    endforeach
    
    // 3. Magias count
    var mag_count := magias.size();
    packet.SetInt8(offset, CInt(mag_count / 256)); offset += 1;
    packet.SetInt8(offset, mag_count % 256); offset += 1;
    
    // Write magias data
    foreach item in magias
        // name
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        // circle (as string)
        var circle_str := CStr(item.circle);
        var circle_len := len(circle_str);
        packet.SetInt8(offset, CInt(circle_len / 256)); offset += 1;
        packet.SetInt8(offset, circle_len % 256); offset += 1;
        packet.SetString(offset, circle_str, 0); offset += circle_len;
        
        // desc
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        // school
        var school_len := len(item.school);
        packet.SetInt8(offset, CInt(school_len / 256)); offset += 1;
        packet.SetInt8(offset, school_len % 256); offset += 1;
        packet.SetString(offset, item.school, 0); offset += school_len;
        
        // iconid
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        
        // hue (default 0)
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        
        // mana_cost
        packet.SetInt8(offset, CInt(item.mana_cost / 256)); offset += 1;
        packet.SetInt8(offset, item.mana_cost % 256); offset += 1;
        
        // min_skill (default 0)
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        
        // reagents (empty string for now)
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        
        // spell_id (CRITICAL: use 'id' not 'spell_id' to match GetSpellsData!)
        packet.SetInt8(offset, CInt(item.id / 256)); offset += 1;
        packet.SetInt8(offset, item.id % 256); offset += 1;
        
        // command_prefix (for Paladin spells = "p", default = "m")
        var cmd_prefix := "m"; // Default to .m command
        if (item.command_prefix)
            cmd_prefix := item.command_prefix;
        endif
        
        var prefix_len := len(cmd_prefix);
        packet.SetInt8(offset, CInt(prefix_len / 256)); offset += 1;
        packet.SetInt8(offset, prefix_len % 256); offset += 1;
        packet.SetString(offset, cmd_prefix, 0); offset += prefix_len;
    endforeach
    
    // 4. Commands count
    var cmd_count := commands.size();
    packet.SetInt8(offset, CInt(cmd_count / 256)); offset += 1;
    packet.SetInt8(offset, cmd_count % 256); offset += 1;
    
    // Write commands data
    foreach item in commands
        // command
        var cmd_len := len(item.command);
        packet.SetInt8(offset, CInt(cmd_len / 256)); offset += 1;
        packet.SetInt8(offset, cmd_len % 256); offset += 1;
        packet.SetString(offset, item.command, 0); offset += cmd_len;
        
        // desc
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        // iconid
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        
        // index
        packet.SetInt8(offset, item.index); offset += 1;
        
        // category
        var cat_len := len(item.category);
        packet.SetInt8(offset, CInt(cat_len / 256)); offset += 1;
        packet.SetInt8(offset, cat_len % 256); offset += 1;
        packet.SetString(offset, item.category, 0); offset += cat_len;
    endforeach
    
    // 5. Skills count
    var skill_count := skills.size();
    packet.SetInt8(offset, CInt(skill_count / 256)); offset += 1;
    packet.SetInt8(offset, skill_count % 256); offset += 1;
    
    // Write skills data
    foreach item in skills
        // name
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        // desc
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        // hue (2 bytes) - ADICIONADO!
        packet.SetInt8(offset, CInt(item.hue / 256)); offset += 1;
        packet.SetInt8(offset, item.hue % 256); offset += 1;
        
        // iconid
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        
        // index (skillID)
        packet.SetInt8(offset, item.index); offset += 1;
        
        // current_value (2 bytes)
        packet.SetInt8(offset, CInt(item.current_value / 256)); offset += 1;
        packet.SetInt8(offset, item.current_value % 256); offset += 1;
        
        // base_value (2 bytes)
        packet.SetInt8(offset, CInt(item.base_value / 256)); offset += 1;
        packet.SetInt8(offset, item.base_value % 256); offset += 1;
        
        // cap (2 bytes)
        packet.SetInt8(offset, CInt(item.cap / 256)); offset += 1;
        packet.SetInt8(offset, item.cap % 256); offset += 1;
        
        // lock (1 byte)
        packet.SetInt8(offset, item.lock); offset += 1;
    endforeach
    
    packet.SendPacket(who);
    
    return 1;
endfunction

// ============================================================================
// Helper functions for packet writing are now in include files:
// - include/hotbar_data.inc: GetSpecialsData, GetCommandsData, GetSpellsData, GetAbilitiesData, GetSkillsData
// - include/hotbar_write.inc: Calculate*Size, Write*Section, SetInt16BE
// ============================================================================
