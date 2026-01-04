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
include "getspellicon_func";
include "include/icon_manager";
include "include/spell_index";      // O(1) spell lookups
include "include/shared_constants"; // Action types, skill maps
include "hotbar_utils";

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
    
    // Send all data in a single unified packet
    SendAllHotbarData(who);
    
    return 1;
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

// Send list of habilidades to client
function SendAbilitiesList(who)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        return 0;
    endif
    
    var char_class := chardata.classe;
    var char_level := CInt(GetObjProperty(who, "Level"));
    
    var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
    if (!habcfg)
        return 0;
    endif
    
    var all_keys := GetConfigStringKeys(habcfg);
    var hab_list := array{};
    
    // Filter habilidades by class, level, and active status
    foreach hab_name in all_keys
        var habelem := FindConfigElem(habcfg, hab_name);
        var uso := GetConfigString(habelem, "uso");
        var classe := GetConfigString(habelem, "classe");
        var nivel := CInt(GetConfigString(habelem, "nivel"));
        
        // Only send active abilities that match class and level
        if (uso == "Ativa" && (classe == char_class || classe == "Todas") && nivel <= char_level)
            var hab_data := dictionary;
            hab_data["name"] := hab_name;
            hab_data["description"] := GetConfigString(habelem, "desc");
            hab_data["class"] := classe;
            hab_data["required_level"] := nivel;
            hab_data["is_active"] := 1;
            hab_data["is_purchased"] := CheckHabPurchased(who, hab_name);
            hab_data["icon_id"] := CInt(GetConfigString(habelem, "icon"));
            hab_data["hue"] := 0;
            hab_data["stamina_cost"] := CInt(GetConfigString(habelem, "stam_cost"));
            hab_data["mana_cost"] := CInt(GetConfigString(habelem, "mana_cost"));
            hab_data["delay"] := CInt(GetConfigString(habelem, "delay"));
            hab_data["requisite"] := GetConfigString(habelem, "requisito");
            hab_data["duration"] := CInt(GetConfigString(habelem, "duracao"));
            hab_data["script"] := GetConfigString(habelem, "script");
            
            hab_list.append(hab_data);
        endif
    endforeach
    
    // Build and send packet
    SendHotbarDataPacket(who, REQUEST_ABILITIES, hab_list);
    
    return 1;
endfunction

// Send list of specials to client
function SendSpecialsList(who)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        return 0;
    endif
    
    var char_class := chardata.classe;
    var specials := array{};
    
    // Example specials - adapt to your system
    var special1 := dictionary;
    special1["name"] := "Ataque Extra";
    special1["class"] := char_class;
    special1["description"] := "Realiza um ataque adicional";
    special1["icon_id"] := 5011;
    special1["hue"] := 0;
    special1["index"] := 1;
    
    specials.append(special1);
    
    SendHotbarDataPacket(who, REQUEST_SPECIALS, specials);
    
    return 1;
endfunction

// Send list of commands to client
function SendCommandsList(who)
    var commands := array{};
    
    // Example commands
    var cmd1 := dictionary;
    cmd1["command"] := ".ficha";
    cmd1["description"] := "Visualizar ficha do personagem";
    cmd1["icon_id"] := 5360;
    cmd1["index"] := 1;
    cmd1["category"] := "Personagem";
    
    commands.append(cmd1);
    
    var cmd2 := dictionary;
    cmd2["command"] := ".point";
    cmd2["description"] := "Distribuir pontos de atributo";
    cmd2["icon_id"] := 5361;
    cmd2["index"] := 2;
    cmd2["category"] := "Personagem";
    
    commands.append(cmd2);
    
    SendHotbarDataPacket(who, REQUEST_COMMANDS, commands);
    
    return 1;
endfunction

// Build and send data response packet
function SendHotbarDataPacket(who, data_type, items)
    var packet := CreatePacket(PACKET_EXTENDED, 1024);
    
    // Header
    packet.SetInt8(0, PACKET_EXTENDED); // Packet ID
    packet.SetInt16(1, 0); // Length placeholder
    packet.SetInt16(3, SUBCOMMAND_RESPONSE); // Subcommand
    packet.SetInt8(5, data_type); // Data type
    packet.SetInt16(6, items.size()); // Item count
    
    var offset := 8;
    
    // Write items based on type
    case (data_type)
        REQUEST_ABILITIES:
            offset := WriteAbilitiesData(packet, offset, items);
        REQUEST_SPECIALS:
            offset := WriteSpecialsData(packet, offset, items);
        REQUEST_COMMANDS:
            offset := WriteCommandsData(packet, offset, items);
    endcase
    
    // Update length
    packet.SetInt16(1, offset);
    
    // Send packet
    packet.SendPacket(who);
    
    return 1;
endfunction

// Write habilidades data to packet
function WriteAbilitiesData(byref packet, offset, items)
    foreach hab in items
        offset := WriteString(packet, offset, hab["name"]);
        offset := WriteString(packet, offset, hab["description"]);
        offset := WriteString(packet, offset, hab["class"]);
        packet.SetInt16(offset, hab["required_level"]); offset += 2;
        packet.SetInt8(offset, hab["is_active"]); offset += 1;
        packet.SetInt8(offset, hab["is_purchased"]); offset += 1;
        packet.SetInt16(offset, hab["icon_id"]); offset += 2;
        packet.SetInt16(offset, hab["hue"]); offset += 2;
        packet.SetInt16(offset, hab["stamina_cost"]); offset += 2;
        packet.SetInt16(offset, hab["mana_cost"]); offset += 2;
        packet.SetInt16(offset, hab["delay"]); offset += 2;
        offset := WriteString(packet, offset, hab["requisite"]);
        packet.SetInt16(offset, hab["duration"]); offset += 2;
        offset := WriteString(packet, offset, hab["script"]);
    endforeach
    
    return offset;
endfunction

// Write specials data to packet
function WriteSpecialsData(byref packet, offset, items)
    foreach special in items
        offset := WriteString(packet, offset, special["name"]);
        offset := WriteString(packet, offset, special["class"]);
        offset := WriteString(packet, offset, special["description"]);
        packet.SetInt16(offset, special["icon_id"]); offset += 2;
        packet.SetInt16(offset, special["hue"]); offset += 2;
        packet.SetInt8(offset, special["index"]); offset += 1;
    endforeach
    
    return offset;
endfunction

// Write commands data to packet
function WriteCommandsData(byref packet, offset, items)
    foreach cmd in items
        offset := WriteString(packet, offset, cmd["command"]);
        offset := WriteString(packet, offset, cmd["description"]);
        packet.SetInt16(offset, cmd["icon_id"]); offset += 2;
        packet.SetInt8(offset, cmd["index"]); offset += 1;
        offset := WriteString(packet, offset, cmd["category"]);
    endforeach
    
    return offset;
endfunction

// Helper: Write string with length prefix
function WriteString(byref packet, offset, str)
    var str_len := len(str);
    packet.SetInt16(offset, str_len);
    offset += 2;
    
    for i := 1 to str_len
        packet.SetInt8(offset, str[i]);
        offset += 1;
    endfor
    
    return offset;
endfunction

// Check if habilidade was purchased
function CheckHabPurchased(who, hab_name)
    var habs := GetObjProperty(who, "habilidades");
    if (!habs)
        return 0;
    endif
    
    return (hab_name in habs);
endfunction

// Execute habilidade
function ExecuteAbility(who, hab_name)
    // Call existing ProccessHab function
    return ProccessHab(who, hab_name);
endfunction

// Execute special
function ExecuteSpecial(who, special_index)
    // Check cooldown
    var cooldown_prop := "SpecialCooldown_" + special_index;
    var last_use := CInt(GetObjProperty(who, cooldown_prop));
    var current_time := ReadGameClock();
    
    if (last_use && (current_time < last_use))
        var remaining := last_use - current_time;
        SendSysMessage(who, "Aguarde " + remaining + " segundos para usar novamente!");
        return 0;
    endif
    
    // Call existing special system
    var result := start_script(":specialAbilities:specialAbilities", who);
    if (result)
        // Calculate cooldown based on character level
        var level := CInt(GetObjProperty(who, "Level"));
        if (!level)
            level := 1;
        endif
        var base_cooldown := 180; // 3 minutes base
        var cooldown_reduction := Floor(level / 4) * 30; // Reduce 30s every 4 levels
        var final_cooldown := base_cooldown - cooldown_reduction;
        if (final_cooldown < 30)
            final_cooldown := 30; // Minimum 30 seconds
        endif
        
        // Set cooldown
        SetObjProperty(who, cooldown_prop, current_time + final_cooldown);
        // SendSysMessage(who, "Executando special ability! (Cooldown: " + final_cooldown + "s)");
        return 1;
    else
        // SendSysMessage(who, "Erro ao executar special ability!");
        return 0;
    endif
endfunction

// Execute command
function ExecuteGMCommand(who, command)
    // Execute the command by calling the script directly
    // SendSysMessage(who, "Executando: " + command);
    
    // Remove the dot if present
    var cmd_name := command;
    if (cmd_name[1] == ".")
        cmd_name := cmd_name[2, len(cmd_name)];
    endif
    
    var result;
    
    // Use case statement for special package commands
    case (cmd_name)
        // Ficha package
        "ficha":
            result := start_script(":ficha:commands/player/ficha", who);
        
        // Commands package
        "pularcerca":
            result := start_script(":commands:player/pularcerca", who);
        "senha":
            SendSysMessage(who, "Use: .senha <senha atual> <nova senha>");
            result := 1;
        
        // Special Abilities package
        "special":
            result := start_script(":specialAbilities:commands/player/special", who);
        
        // Spells package (magias)
        "assumirforma":
            result := start_script(":spells:commands/player/assumirforma", who);
        "pilotar":
            result := start_script(":spells:commands/player/pilotar", who);
        "arcoelemental":
            result := start_script(":spells:commands/player/arcoelemental", who);
        "rituais":
            result := start_script(":spells:commands/player/rituais", who);
        "arremessar":
            result := start_script(":spells:commands/player/arremessar", who);
        "voltar":
            result := start_script(":spells:commands/player/voltar", who);
        
        // Equipsys package (combate)
        "golpe":
            result := start_script(":equipsys:commands/player/golpe", who);
        
        // Animal Handle package (skills)
        "adestrar":
            result := start_script(":taming:commands/player/adestrar", who);
        "acalmar":
            // Acalmar has inline logic - call the wrapper function from gmhab
            result := start_script(":specialAbilities:commands/gm/gmhab", array{who, "acalmar"});
        
        // TN package (roleplay)
        "actions":
            result := start_script(":tn:commands/player/actions", who);
        "sounds":
            result := start_script(":tn:commands/player/sounds", who);
        "arrastar":
            result := start_script(":tn:commands/player/arrastar", who);
        "erguer":
            result := start_script(":tn:commands/player/erguer", who);
        "lingua":
            result := start_script(":commands:player/lingua", who);
        
        // FLS package
        "ficharp":
            result := start_script(":fls_core:commands/player/ficharp", who);
        "pontosrp":
            result := start_script(":fls_rppoints:commands/player/pontosrp", who);
        "rps":
            result := start_script(":fls_rppoints:commands/player/rps", who);
        
        // Achievements package
        "conquistas":
            result := start_script(":achievements:commands/player/conquistas", who);
        
        // Food package
        "fome":
            result := start_script(":food:textcmd/player/fome", who);
        
        // Character Creation package
        "meditar":
            result := start_script(":charactercreation:commands/player/meditar", who);
        
        // Survival package
        "camuflar":
            result := start_script(":survival:commands/player/camuflar", who);
        
        // Commands package (skills/utility)
        "assoviar":
            result := start_script(":commands:player/assoviar", who);
        "estudar":
            result := start_script(":commands:player/estudar", who);
        
        // Brain AI package (pets)
        "pe":
            result := start_script(":brainAI:commands/player/pe", who);
        
        default:
            // Try standard location for all other commands
            result := start_script(":commands:player/" + cmd_name, who);
    endcase
    
    if (!result)
        // SendSysMessage(who, "Erro ao executar comando! Tente digitar: ." + cmd_name);
        return 0;
    else
        return 1;
    endif
endfunction

// Execute skill
function ExecuteSkill(who, skill_id)
    // Map SkillID to skill name (must match GetSkillsData)
    var skill_map := dictionary;
    skill_map[2] := "Alchemy";        // Alchemy
    skill_map[16] := "Arcana";        // Magery (usado como Arcana)
    skill_map[21] := "Sneak";         // Stealth
    skill_map[33] := "Pickpocketing"; // Stealing
    skill_map[38] := "Awareness";     // Detect Hidden
    skill_map[48] := "Mechanics";     // Remove Trap
    skill_map[17] := "Medicine";      // Healing
    skill_map[20] := "Survival";      // Camping
    skill_map[32] := "Gathering";     // Lumberjacking
    skill_map[35] := "Handle_Animal"; // Animal Taming
    skill_map[23] := "Theology";      // Spirit Speak
    
    // Get skill name from ID
    var skill_name := skill_map[skill_id];
    
    if (!skill_name)
        // SendSysMessage(who, "Indice de skill invalido! ID: " + skill_id);
        return 0;
    endif
    
    // Get the player's skill value
    var skill_value := CInt(AP_GetSkill(who, skill_name));
    
    // DEBUG: Log skill info
    
    if (skill_value <= 0)
        SendSysMessage(who, "Voce nao possui pontos em " + skill_name + "!");
        return 0;
    endif
    
    // Get the attribute config element
    var attrib_elem := AP_GetAttributeCfgElem(skill_name);
    if (!attrib_elem)
        // SendSysMessage(who, "Skill " + skill_name + " nao encontrada!");
        return 0;
    endif
    
    // Get the script path from the attribute
    var script_path := GetConfigString(attrib_elem, "Script");
    
    // DEBUG: Log script path
    
    if (!script_path || script_path == "")
        // SendSysMessage(who, "Skill " + skill_name + " nao possui script!");
        return 0;
    endif
    
    // Check cooldown
    var delay := GetObjProperty(who, "delayskill");
    if (delay && delay > ReadGameClock())
        SendSysMessage(who, "Voce ainda esta fazendo outra coisa.");
        return 0;
    endif
    
    // Set cooldown (5 seconds)
    SetObjProperty(who, "delayskill", ReadGameClock() + 5);
    
    // SendSysMessage(who, "Usando " + skill_name + " (valor: " + skill_value + ")");
    
    // Execute the skill script with the player's skill value
    var result := start_script(script_path, who);
    if (!result)
        // SendSysMessage(who, "Erro ao executar skill!");
        return 0;
    else
        return 1;
    endif
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

// Helper functions to calculate sizes
function CalculateAbilitiesSize(items)
    var size := 0;
    foreach item in items
        size := size + 2 + len(item.name);         // name length + name
        size := size + 2 + len(item.desc);         // desc length + desc
        size := size + 2 + len(item.classe);       // classe length + classe
        size := size + 2;                           // required_level
        size := size + 1;                           // is_active (bool)
        size := size + 1;                           // is_purchased (bool)
        size := size + 2;                           // iconid
        size := size + 2;                           // hue
        size := size + 2;                           // stamina_cost
        size := size + 2;                           // mana_cost
        size := size + 2;                           // delay
        size := size + 2 + len(item.requisite);    // requisite length + requisite
        size := size + 2;                           // duration
        size := size + 2 + len(item.script);       // script length + script
    endforeach
    return size;
endfunction

function CalculateSpecialsSize(items)
    var size := 0;
    foreach item in items
        size := size + 2 + len(item.name);  // name length + name
        size := size + 2 + len(item.classe); // classe length + classe
        size := size + 2 + len(item.desc);  // desc length + desc
        size := size + 2;  // iconid
        size := size + 2;  // hue
        size := size + 1;  // index
        size := size + 2;  // delay
    endforeach
    return size;
endfunction

function CalculateSpellsSize(items)
    var size := 0;
    foreach item in items
        // name: 2 bytes (length) + string length
        size := size + 2 + len(item.name);
        // circle: 2 bytes (length) + string length
        size := size + 2 + len(CStr(item.circle));
        // desc: 2 bytes (length) + string length
        size := size + 2 + len(item.desc);
        // school: 2 bytes (length) + string length
        size := size + 2 + len(item.school);
        // iconid: 2 bytes
        size := size + 2;
        // hue: 2 bytes
        size := size + 2;
        // mana_cost: 2 bytes
        size := size + 2;
        // min_skill: 2 bytes
        size := size + 2;
        // reagents: 2 bytes (length) + 0 (empty string for now)
        size := size + 2;
        // spell_id: 2 bytes
        size := size + 2;
        // command_prefix: 2 bytes (length) + string length (default "m" = 1 byte)
        var prefix_len := 1; // Default "m"
        if (item.command_prefix)
            prefix_len := len(item.command_prefix);
        endif
        size := size + 2 + prefix_len;
    endforeach
    return size;
endfunction

function CalculateCommandsSize(items)
    var size := 0;
    foreach item in items
        size := size + 2 + len(item.command);    // command length + command
        size := size + 2 + len(item.desc);       // desc length + desc
        size := size + 2;  // iconid
        size := size + 1;  // index
        size := size + 2 + len(item.category);   // category length + category
    endforeach
    return size;
endfunction

function CalculateSkillsSize(items)
    var size := 0;
    foreach item in items
        // name: 2 bytes (length) + string length
        size := size + 2 + len(item.name);
        // desc: 2 bytes (length) + string length
        size := size + 2 + len(item.desc);
        // hue: 2 bytes
        size := size + 2;
        // iconid: 2 bytes
        size := size + 2;
        // index: 1 byte
        size := size + 1;
        // current_value: 2 bytes
        size := size + 2;
        // base_value: 2 bytes
        size := size + 2;
        // cap: 2 bytes
        size := size + 2;
        // lock: 1 byte
        size := size + 1;
    endforeach
    return size;
endfunction

// Helper function to write Int16 in Big Endian format
function SetInt16BE(packet, offset, value)
    var high_byte := (value >> 8) & 0xFF;
    var low_byte := value & 0xFF;
    packet.SetInt8(offset, high_byte);
    packet.SetInt8(offset + 1, low_byte);
endfunction

// Helper functions to write data sections
function WriteAbilitiesSection(packet, offset, items)
    SetInt16BE(packet, offset, items.size());
    offset := offset + 2;
    // No items to write for now
    return offset;
endfunction

function WriteSpecialsSection(packet, offset, items)
    SetInt16BE(packet, offset, items.size());
    offset := offset + 2;
    
    foreach item in items
        packet.SetInt16(offset, len(item.name));
        offset := offset + 2;
        packet.SetString(offset, item.name, 0);
        offset := offset + len(item.name);
        
        packet.SetInt16(offset, len(item.classe));
        offset := offset + 2;
        packet.SetString(offset, item.classe, 0);
        offset := offset + len(item.classe);
        
        packet.SetInt16(offset, len(item.desc));
        offset := offset + 2;
        packet.SetString(offset, item.desc, 0);
        offset := offset + len(item.desc);
        
        packet.SetInt16(offset, item.iconid);
        offset := offset + 2;
        
        packet.SetInt16(offset, item.hue);
        offset := offset + 2;
        
        packet.SetInt8(offset, item.index);
        offset := offset + 1;
    endforeach
    
    return offset;
endfunction

function WriteSpellsSection(packet, offset, items)
    SetInt16BE(packet, offset, items.size());
    offset := offset + 2;
    
    // Write magias data
    foreach item in items
        
        // name
        var name_len := len(item.name);
        SetInt16BE(packet, offset, name_len); offset += 2;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        // circle (as string)
        var circle_str := CStr(item.circle);
        var circle_len := len(circle_str);
        SetInt16BE(packet, offset, circle_len); offset += 2;
        packet.SetString(offset, circle_str, 0); offset += circle_len;
        
        // desc (use empty string if not available)
        var desc := "";
        if (item.desc)
            desc := item.desc;
        endif
        var desc_len := len(desc);
        SetInt16BE(packet, offset, desc_len); offset += 2;
        if (desc_len > 0)
            packet.SetString(offset, desc, 0);
        endif
        offset += desc_len;
        
        // school (magic_path)
        var school := "";
        if (item.school)
            school := item.school;
        endif
        var school_len := len(school);
        SetInt16BE(packet, offset, school_len); offset += 2;
        if (school_len > 0)
            packet.SetString(offset, school, 0);
        endif
        offset += school_len;
        
        // iconid
        SetInt16BE(packet, offset, item.iconid); offset += 2;
        
        // hue (default 0)
        SetInt16BE(packet, offset, 0); offset += 2;
        
        // mana_cost
        SetInt16BE(packet, offset, item.mana_cost); offset += 2;
        
        // min_skill (default 0)
        SetInt16BE(packet, offset, 0); offset += 2;
        
        // reagents (empty string for now)
        SetInt16BE(packet, offset, 0); offset += 2;
        
        // spell_id (THIS IS THE CRITICAL FIELD!)
        SetInt16BE(packet, offset, item.id); offset += 2;
        
        // command_prefix (NEW FIELD!)
        var cmd_prefix := "m";  // Default
        if (item.magic_path == "Shaman" || item.magic_path == "shamanspells")
            cmd_prefix := "d";
        elseif (item.magic_path == "paladinspells")
            cmd_prefix := "p";
        elseif (item.magic_path == "cantripspells")
            cmd_prefix := "t";
        elseif (item.magic_path == "demonhunterspells")
            cmd_prefix := "c";
        endif
        var prefix_len := len(cmd_prefix);
        SetInt16BE(packet, offset, prefix_len); offset += 2;
        packet.SetString(offset, cmd_prefix, 0); offset += prefix_len;
    endforeach
    
    return offset;
endfunction

function WriteCommandsSection(packet, offset, items)
    SetInt16BE(packet, offset, items.size());
    offset := offset + 2;
    
    foreach item in items
        packet.SetInt16(offset, len(item.command));
        offset := offset + 2;
        packet.SetString(offset, item.command, 0);
        offset := offset + len(item.command);
        
        packet.SetInt16(offset, len(item.desc));
        offset := offset + 2;
        packet.SetString(offset, item.desc, 0);
        offset := offset + len(item.desc);
        
        packet.SetInt16(offset, item.iconid);
        offset := offset + 2;
        
        packet.SetInt8(offset, item.index);
        offset := offset + 1;
        
        packet.SetInt16(offset, len(item.category));
        offset := offset + 2;
        packet.SetString(offset, item.category, 0);
        offset := offset + len(item.category);
    endforeach
    
    return offset;
endfunction

function WriteSkillsSection(packet, offset, items)
    SetInt16BE(packet, offset, items.size());
    offset := offset + 2;
    // No items to write for now
    return offset;
endfunction

// Get specials data
function GetSpecialsData(who)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        return array{};
    endif
    
    var specials := array{};
    var special_name := "";
    var special_class := "";
    var special_hue := 0;
    var special_index := 0;
    
    // Determine which special ability based on class/path (from gmhab.src lines 151-183)
    case (chardata.classe)
        "Guerreiro":
            special_name := "Ataque Extra";
            special_class := "Guerreiro";
            special_hue := 33; // Red
            special_index := 1;
            
        "Ladino":
            special_name := "Evasao";
            special_class := "Ladino";
            special_hue := 63; // Green
            special_index := 2;
            
        "Sabio":
            // Check wisdom_path for Sabio
            if (chardata.wisdom_path == "Engenheiro")
                special_name := "Recarga Rapida";
                special_class := "Engenheiro";
                special_hue := 2101; // Gray
                special_index := 3;
            elseif (chardata.magia == "Arqueiro Arcano")
                special_name := "Velocidade Arcana";
                special_class := "Arqueiro Arcano";
                special_hue := 16; // Purple
                special_index := 5;
            else
                // Mago, Xamã, Clérigo, etc.
                special_name := "Conjuracao Rapida";
                special_class := "Conjurador";
                special_hue := 88; // Blue
                special_index := 4;
            endif
    endcase
    
    // Calculate dynamic cooldown based on level
    var level := CInt(GetObjProperty(who, "Level"));
    if (!level)
        level := 1;
    endif
    var base_cooldown := 180; // 3 minutes base
    var cooldown_reduction := Floor(level / 4) * 30; // Reduce 30s every 4 levels
    var final_cooldown := base_cooldown - cooldown_reduction;
    if (final_cooldown < 30)
        final_cooldown := 30; // Minimum 30 seconds
    endif
    
    // Check if special is currently on cooldown
    var remaining_cooldown := CInt(TS_GetCooldown(who, "special_ability"));
    if (!remaining_cooldown || remaining_cooldown < 0)
        remaining_cooldown := 0;
    endif
    
    // Create special if found
    if (special_name != "")
        // Set specific description based on special type
        var special_desc := "";
        case (special_index)
            1: // Ataque Extra (Guerreiro)
                special_desc := "Executa um ataque instantâneo adicional";
            2: // Evasão (Ladino)
                special_desc := "Fica invisível por 0.5s e recua 4 tiles";
            3: // Recarga Rápida (Engenheiro)
                special_desc := "Adiciona 1 bala instantaneamente na arma de fogo";
            4: // Conjuração Rápida (Conjurador)
                special_desc := "Permite conjurar a próxima magia instantaneamente";
            5: // Velocidade Arcana (Arqueiro Arcano)
                special_desc := "Aumenta a velocidade de movimento por 4 segundos";
            default:
                special_desc := "Habilidade especial de " + special_class;
        endcase
        
        // Buscar ícone do icons.cfg
        var icon_data := GetSpecialIcon(special_name);
        
        var special1 := struct{
            "name" := special_name,
            "classe" := special_class,
            "desc" := special_desc,
            "iconid" := icon_data.iconid,
            "hue" := icon_data.hue,
            "index" := special_index,
            "delay" := final_cooldown,
            "remaining" := remaining_cooldown  // Time left on current cooldown
        };
        
        specials.append(special1);
    endif
    
    return specials;
endfunction

// Get commands data
function GetCommandsData(who)
    var commands := array{};
    var index := 1;
    var icon_data;
    
    // Comandos Gerais (6)
    icon_data := GetCommandIcon(".ficha");
    commands.append(struct{"command" := ".ficha", "desc" := "Ficha do personagem", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;
    
    icon_data := GetCommandIcon(".point");
    commands.append(struct{"command" := ".point", "desc" := "Apontar", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;
    
    icon_data := GetCommandIcon(".pularcerca");
    commands.append(struct{"command" := ".pularcerca", "desc" := "Pular cerca", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;
    
    icon_data := GetCommandIcon(".revistar");
    commands.append(struct{"command" := ".revistar", "desc" := "Revistar alguem", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;
    
    icon_data := GetCommandIcon(".senha");
    commands.append(struct{"command" := ".senha", "desc" := "Trocar senha", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;
    
    icon_data := GetCommandIcon(".showdmg");
    commands.append(struct{"command" := ".showdmg", "desc" := "Mostrar dano", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;
    
    // Magias (6)
    icon_data := GetCommandIcon(".arcoelemental");
    commands.append(struct{"command" := ".arcoelemental", "desc" := "Arco elemental", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;
    
    icon_data := GetCommandIcon(".arremessar");
    commands.append(struct{"command" := ".arremessar", "desc" := "Arremessar magia", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;
    
    icon_data := GetCommandIcon(".assumirforma");
    commands.append(struct{"command" := ".assumirforma", "desc" := "Assumir forma animal", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;
    
    icon_data := GetCommandIcon(".pilotar");
    commands.append(struct{"command" := ".pilotar", "desc" := "Pilotar", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;
    
    icon_data := GetCommandIcon(".rituais");
    commands.append(struct{"command" := ".rituais", "desc" := "Rituais magicos", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;
    
    icon_data := GetCommandIcon(".voltar");
    commands.append(struct{"command" := ".voltar", "desc" := "Voltar forma original", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;
    
    // Combate (1)
    icon_data := GetCommandIcon(".golpe");
    commands.append(struct{"command" := ".golpe", "desc" := "Golpe especial de arma", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Combate"}); index += 1;
    
    // Skills (5)
    icon_data := GetCommandIcon(".acalmar");
    commands.append(struct{"command" := ".acalmar", "desc" := "Acalmar animal", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Skill"}); index += 1;
    
    icon_data := GetCommandIcon(".adestrar");
    commands.append(struct{"command" := ".adestrar", "desc" := "Adestrar criatura", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Skill"}); index += 1;
    
    icon_data := GetCommandIcon(".camuflar");
    commands.append(struct{"command" := ".camuflar", "desc" := "Camuflar-se", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Skill"}); index += 1;
    
    icon_data := GetCommandIcon(".estudar");
    commands.append(struct{"command" := ".estudar", "desc" := "Estudar item", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Skill"}); index += 1;
    
    // Roleplay (6)
    icon_data := GetCommandIcon(".actions");
    commands.append(struct{"command" := ".actions", "desc" := "Menu de acoes de RP", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;
    
    icon_data := GetCommandIcon(".arrastar");
    commands.append(struct{"command" := ".arrastar", "desc" := "Arrastar corpo", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;
    
    icon_data := GetCommandIcon(".erguer");
    commands.append(struct{"command" := ".erguer", "desc" := "Erguer objeto", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;
    
    icon_data := GetCommandIcon(".ficharp");
    commands.append(struct{"command" := ".ficharp", "desc" := "Ficha de RP", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;
    
    icon_data := GetCommandIcon(".lingua");
    commands.append(struct{"command" := ".lingua", "desc" := "Trocar idioma falado", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;
    
    icon_data := GetCommandIcon(".sounds");
    commands.append(struct{"command" := ".sounds", "desc" := "Menu de sons", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;
    
    // Pets (2)
    icon_data := GetCommandIcon(".assoviar");
    commands.append(struct{"command" := ".assoviar", "desc" := "Chamar pet do limbo", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Pet"}); index += 1;
    
    icon_data := GetCommandIcon(".pe");
    commands.append(struct{"command" := ".pe", "desc" := "Faz o pet fazer emotes", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Pet"}); index += 1;
    
    // Outros (5)
    icon_data := GetCommandIcon(".conquistas");
    commands.append(struct{"command" := ".conquistas", "desc" := "Ver conquistas", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;
    
    icon_data := GetCommandIcon(".fome");
    commands.append(struct{"command" := ".fome", "desc" := "Ver fome", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;
    
    icon_data := GetCommandIcon(".meditar");
    commands.append(struct{"command" := ".meditar", "desc" := "Meditar", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;
    
    icon_data := GetCommandIcon(".pontosrp");
    commands.append(struct{"command" := ".pontosrp", "desc" := "Pontos de RP", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;
    
    icon_data := GetCommandIcon(".rps");
    commands.append(struct{"command" := ".rps", "desc" := "Sistema de RP", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;
    return commands;
endfunction

// Get magias data - load from grimoire
function GetSpellsData(who)
    var magias := array{};
    var chardata := GetObjProperty(who, "chardata");
    
    // Find spellbook in backpack
    var backpack := who.backpack;
    if (!backpack)
        return magias;
    endif
    
    
    // Search for all spellbooks
    foreach item in EnumerateItemsInContainer(backpack)
        
        // Check if it's a spellbook
        // 0xEFA (3834) = vanilla spellbook
        // 0x2253 (8787) = necro spellbook
        // 0x9E80 (40576) = custom grimoire
        if (item.objtype == 0xEFA || item.objtype == 0x2253 || item.objtype == 0x9E80)
            
            // Get booktype to know which config to load
            var booktype := GetObjProperty(item, "Booktype");
            if (!booktype)
                booktype := "spellcaster";  // Default
            endif
            
            // Determine config path based on booktype
            var spell_cfg_path := ":spells:config/spellcaster";
            case (booktype)
                "spellcaster":
                "mage":
                    spell_cfg_path := ":spells:config/spellcaster";
                "priest":
                    spell_cfg_path := ":spells:config/priestspells";
                "druid":
                    spell_cfg_path := ":spells:config/druidspells";
                "shaman":
                "Shaman":  // Capital S variant
                    spell_cfg_path := ":spells:config/shamanspells";
                "paladin":
                    spell_cfg_path := ":spells:config/paladinspells";
                "demonhunter":
                    spell_cfg_path := ":spells:config/demonhunterspells";
                "trickster":
                    spell_cfg_path := ":spells:config/tricksterspells";
                "cantrip":
                    spell_cfg_path := ":spells:config/cantripspells";
            endcase
            
            var spell_cfg := ReadConfigFile(spell_cfg_path);
            
            // Get spells directly from spellbook property
            var spells_array := GetObjProperty(item, "spells");
            
            if (!spells_array || TypeOf(spells_array) != "Array")
                continue;
            endif
            
            
            // Iterate through spells and add to magias array
            foreach spell_struct in spells_array
                if (TypeOf(spell_struct) == "Struct")
                    // Get spell details from config using ID
                    var spell_id := spell_struct.id;
                    var spell_elem := FindConfigElem(spell_cfg, spell_id);
                    
                    var spell_name := "<unknown>";
                    var spell_circle := 1;
                    var spell_mana := 0;
                    
                    if (spell_elem)
                        spell_name := GetConfigString(spell_elem, "Name");
                        spell_circle := CInt(GetConfigString(spell_elem, "Circle"));
                        spell_mana := CInt(GetConfigString(spell_elem, "Mana"));
                    endif
                    
                    var magia := struct{};
                    magia.+id := spell_id;
                    magia.+name := spell_name;
                    magia.+magic_path := spell_struct.magic_path;
                    magia.+school := spell_struct.school;
                    magia.+circle := spell_circle;
                    magia.+mana_cost := spell_mana;
                    magia.+iconid := 0x9BAC;  // Default icon
                    magia.+hue := 0;
                    
                    magias.append(magia);
                endif
            endforeach
        endif
    endforeach
    
    // Load cantrips if player has the trait or magia
    // chardata already declared at function start
    if (chardata)
        var magia := chardata.magia;
        var trait := chardata.trait;
        
        // Check if player can use cantrips
        if (magia == "spellcaster" || trait == "Natureza Arcana" || magia == "Arqueiro Arcano")
            var cantrip_cfg := ReadConfigFile(":spells:config/cantripspells");
            
            if (cantrip_cfg)
                var cantrip_ids := GetConfigIntKeys(cantrip_cfg);
                
                foreach cantrip_id in cantrip_ids
                    var cantrip_elem := FindConfigElem(cantrip_cfg, cantrip_id);
                    if (cantrip_elem)
                        var cantrip_name := GetConfigString(cantrip_elem, "Name");
                        var cantrip_circle := CInt(GetConfigString(cantrip_elem, "Circle"));
                        var cantrip_mana := CInt(GetConfigString(cantrip_elem, "Mana"));
                        var cantrip_school := GetConfigString(cantrip_elem, "School");
                        
                        var magia := struct{};
                        magia.+id := cantrip_id;
                        magia.+name := cantrip_name;
                        magia.+magic_path := "cantripspells";
                        magia.+school := cantrip_school;
                        magia.+circle := cantrip_circle;
                        magia.+mana_cost := cantrip_mana;
                        magia.+iconid := 0x9BAC;  // Default icon
                        magia.+hue := 0;
                        
                        magias.append(magia);
                    endif
                endforeach
            endif
        endif
    endif
    
    // Load Shaman memorized spells
    if (chardata && chardata.magia == "Shaman")
        var shaman_list := GetObjProperty(who, "shaman_list");
        
        if (shaman_list && TypeOf(shaman_list) == "Array" && shaman_list.size() > 0)
            var shaman_cfg := ReadConfigFile(":spells:config/shamanspells");
            
            if (shaman_cfg)
                foreach spell in shaman_list
                    // Skip if no ID
                    if (!spell.id)
                        continue;
                    endif
                    
                    // Load full spell data from config
                    var spell_elem := FindConfigElem(shaman_cfg, spell.id);
                    if (!spell_elem)
                        continue;
                    endif
                    
                    // Get name from config (spell.name might not exist)
                    var spell_name := GetConfigString(spell_elem, "Name");
                    if (!spell_name)
                        continue;
                    endif
                    
                    var magia := struct{};
                    magia.+id := spell.id;
                    magia.+name := spell_name;  // From config, not from spell struct
                    magia.+magic_path := "Shaman";
                    magia.+school := "Shaman";
                    magia.+circle := CInt(spell.circle);
                    magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                    magia.+iconid := 0x9BAC;  // Default icon
                    magia.+hue := 0;
                    
                    magias.append(magia);
                endforeach
            endif
        else
        endif
    endif
    
    // ✅ NEW: Load Priest memorized spells
    if (chardata && (chardata.magia == "Priest" || chardata.magia == "Sacerdote"))
        var priest_list := GetObjProperty(who, "priest_list");
        
        if (priest_list && TypeOf(priest_list) == "Array" && priest_list.size() > 0)
            var priest_cfg := ReadConfigFile(":spells:config/priestspells");
            
            if (priest_cfg)
                foreach spell in priest_list
                    var spell_elem;
                    var spell_id;
                    var spell_name;
                    
                    // ✅ BACKWARD COMPATIBILITY: Support both old (name-based) and new (id-based) formats
                    if (spell.id)
                        // New format: has ID field
                        spell_id := spell.id;
                        spell_elem := FindConfigElem(priest_cfg, spell_id);
                    elseif (spell.name)
                        // Old format: only has name field - O(1) lookup via index
                        spell_name := spell.name;
                        spell_id := GetSpellIdByName("priest", spell_name);
                        if (spell_id != error)
                            spell_elem := GetSpellById("priest", spell_id);
                        endif
                    else
                        // Invalid spell entry - skip
                        continue;
                    endif
                    
                    if (!spell_elem)
                        continue;
                    endif
                    
                    // Get name from config (in case we found by ID)
                    if (!spell_name)
                        spell_name := GetConfigString(spell_elem, "Name");
                    endif
                    
                    if (!spell_name)
                        continue;
                    endif
                    
                    var magia := struct{};
                    magia.+id := spell_id;
                    magia.+name := spell_name;
                    magia.+magic_path := "Priest";
                    magia.+school := "Priest";
                    magia.+circle := CInt(spell.circle);
                    magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                    magia.+iconid := 0x9BAC;  // Default icon
                    magia.+hue := 0;
                    
                    magias.append(magia);
                endforeach
            endif
        endif
    endif
    
    // ✅ NEW: Load Druid memorized spells
    if (chardata && (chardata.magia == "Druid" || chardata.magia == "Druida"))
        var druid_list := GetObjProperty(who, "druid_list");
        
        if (druid_list && TypeOf(druid_list) == "Array" && druid_list.size() > 0)
            var druid_cfg := ReadConfigFile(":spells:config/druidspells");
            
            if (druid_cfg)
                foreach spell in druid_list
                    var spell_elem;
                    var spell_id;
                    var spell_name;
                    
                    // ✅ BACKWARD COMPATIBILITY: Support both old (name-based) and new (id-based) formats
                    if (spell.id)
                        // New format: has ID field
                        spell_id := spell.id;
                        spell_elem := FindConfigElem(druid_cfg, spell_id);
                    elseif (spell.name)
                        // Old format: only has name field - O(1) lookup via index
                        spell_name := spell.name;
                        spell_id := GetSpellIdByName("druid", spell_name);
                        if (spell_id != error)
                            spell_elem := GetSpellById("druid", spell_id);
                        endif
                    else
                        // Invalid spell entry - skip
                        continue;
                    endif

                    if (!spell_elem)
                        continue;
                    endif
                    
                    // Get name from config (in case we found by ID)
                    if (!spell_name)
                        spell_name := GetConfigString(spell_elem, "Name");
                    endif
                    
                    if (!spell_name)
                        continue;
                    endif
                    
                    var magia := struct{};
                    magia.+id := spell_id;
                    magia.+name := spell_name;
                    magia.+magic_path := "Druid";
                    magia.+school := "Druid";
                    magia.+circle := CInt(spell.circle);
                    magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                    magia.+iconid := 0x9BAC;  // Default icon
                    magia.+hue := 0;
                    
                    magias.append(magia);
                endforeach
            endif
        endif
    endif
    
    // ✅ NEW: Load Paladin spells (based on Consagracao Divina habilidade)
    var has_paladin := TemHabilidade(who, "Consagracao Divina");
    if (has_paladin)
        var paladin_cfg := ReadConfigFile(":spells:config/paladinspells");
        
        if (paladin_cfg)
            var theology_skill := CInt(AP_GetSkill(who, "Theology"));
            var max_circle := theology_skill / 20;
            if (max_circle < 1)
                max_circle := 1;
            endif
            
            foreach spell_id in (GetConfigIntKeys(paladin_cfg))
                var spell_elem := FindConfigElem(paladin_cfg, spell_id);
                if (!spell_elem)
                    continue;
                endif
                
                var spell_circle := CInt(GetConfigInt(spell_elem, "Circle"));
                if (spell_circle > max_circle)
                    continue; // Player doesn't have enough Theology
                endif
                
                var spell_name := GetConfigString(spell_elem, "Name");
                if (!spell_name)
                    continue;
                endif
                
                var magia := struct{};
                magia.+name := spell_name;
                magia.+school := "Paladino";
                magia.+circle := spell_circle;
                magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                magia.+iconid := 0x9BAC;  // Default icon
                magia.+hue := 0;
                magia.+id := spell_id; // ✅ FIX: Use 'id' property name
                magia.+command_prefix := "p"; // Use .p command
                
                magias.append(magia);
                sleepms(2);
            endforeach
        endif
    endif

    // ✅ NEW: Load Trickster spells (based on Sensibilidade Arcana habilidade)
    var has_trickster := TemHabilidade(who, "Sensibilidade Arcana");
    if (has_trickster)
        var trickster_cfg := ReadConfigFile(":spells:config/tricksterspells");
        
        if (trickster_cfg)
            foreach spell_id in (GetConfigIntKeys(trickster_cfg))
                var spell_elem := FindConfigElem(trickster_cfg, spell_id);
                if (!spell_elem)
                    continue;
                endif
                
                var spell_name := GetConfigString(spell_elem, "Name");
                if (!spell_name)
                    continue;
                endif
                
                var magia := struct{};
                magia.+name := spell_name;
                magia.+school := "Trickster";
                magia.+circle := CInt(GetConfigInt(spell_elem, "Circle"));
                magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                magia.+iconid := 0x9BAC;  // Default icon
                magia.+hue := 0;
                magia.+id := spell_id; 
                magia.+command_prefix := "m"; // Use .m command
                
                magias.append(magia);
                sleepms(2);
            endforeach
        endif
    endif

    // ✅ NEW: Load Demon Hunter spells (based on Sigilo de Protecao habilidade)
    var has_demonhunter := TemHabilidade(who, "Sigilo de Protecao");
    if (has_demonhunter)
        var dh_cfg := ReadConfigFile(":spells:config/demonhunterspells");
        
        if (dh_cfg)
            foreach spell_id in (GetConfigIntKeys(dh_cfg))
                var spell_elem := FindConfigElem(dh_cfg, spell_id);
                if (!spell_elem)
                    continue;
                endif
                
                var spell_name := GetConfigString(spell_elem, "Name");
                if (!spell_name)
                    continue;
                endif
                
                var magia := struct{};
                magia.+name := spell_name;
                magia.+school := "Demon Hunter";
                magia.+circle := CInt(GetConfigInt(spell_elem, "Circle"));
                magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                magia.+iconid := 0x9BAC;  // Default icon
                magia.+hue := 0;
                magia.+id := spell_id; 
                magia.+command_prefix := "c"; // Use .c command
                
                magias.append(magia);
                sleepms(2);
            endforeach
        endif
    endif
    
    return magias;
endfunction

// Get habilidades data - uses shared filtering logic (DRY principle)
function GetAbilitiesData(who)
    // ✅ DRY: Use shared function instead of duplicating filtering logic
    var filtered_hab_names := GetFilteredAbilities(who);
    
    // Load config to get full habilidade data
    var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
    if (!habcfg)
        return array{};
    endif
    
    // Get player data for icon lookup
    var chardata := GetObjProperty(who, "chardata");
    var char_class := chardata.classe;
    var myhabs := GetObjProperty(who, "myhabs");
    if (!myhabs) myhabs := dictionary; endif
    
    // Build full data structures for each filtered habilidade
    var filtered_habs := array{};
    var hab_index := 1;
    
    foreach hab_name in filtered_hab_names
        var habelem := FindConfigElem(habcfg, hab_name);
        if (!habelem) continue; endif
        
        // Get icon
        var icon_data := GetAbilityIcon(hab_name, char_class);
        
        // Read additional fields
        var classes := GetConfigStringDictionary(habelem, "classe");
        var required_level := CInt(classes[char_class]);
        var uso := GetConfigString(habelem, "uso");
        
        // Check if purchased
        var has_purchased := 0;
        foreach key in (myhabs.keys())
            if (Lower(myhabs[key]) == Lower(hab_name))
                has_purchased := 1;
                break;
            endif
        endforeach
        
        // Build struct
        var hab_data := struct{
            "habilidade_id" := hab_index,  // Sequential ID
            "name" := hab_name,
            "classe" := char_class,
            "desc" := GetConfigString(habelem, "desc"),
            "required_level" := required_level,
            "is_active" := (uso == "Ativa"),
            "is_purchased" := has_purchased,
            "iconid" := icon_data.iconid,
            "hue" := icon_data.hue,
            "stamina_cost" := CInt(GetConfigString(habelem, "stam")),
            "mana_cost" := CInt(GetConfigString(habelem, "mana")),
            "delay" := CInt(GetConfigString(habelem, "delay")),
            "requisite" := GetConfigString(habelem, "requisito"),
            "duration" := CInt(GetConfigString(habelem, "duracao")),
            "script" := GetConfigString(habelem, "script")
        };
        
        filtered_habs.append(hab_data);
        hab_index := hab_index + 1;
        sleepms(2);
    endforeach
    
    return filtered_habs;
endfunction

// Get skills data
function GetSkillsData(who)
    var skills := array{};
    
    // Usar os IDs reais das skills do UO (não índices sequenciais)
    // Formato: name, desc, iconid, skillID (ID real do UO)
    // Agora também incluindo valores do jogador
    
    // Nomes exibidos (podem ter espaços)
    var skill_names := array{
        "Alchemy",
        "Arcana", 
        "Sneak",
        "Pickpocketing",
        "Awareness",
        "Mechanics",
        "Medicine",
        "Survival",
        "Gathering",
        "Handle Animal",
        "Theology"
    };
    
    // Nomes internos (usados para AP_GetSkill - devem corresponder aos nomes no attributes.cfg)
    var skill_internal_names := array{
        "Alchemy",
        "Arcana", 
        "Sneak",
        "Pickpocketing",
        "Awareness",
        "Mechanics",
        "Medicine",
        "Survival",
        "Gathering",
        "Handle_Animal",  // Nome interno com underscore
        "Theology"
    };
    
    var skill_descs := array{
        "Avalie uma poção",
        "Conhecimento arcano",
        "Mover furtivamente",
        "Roubar objetos",
        "Encontrar armadilhas",
        "Desarmar armadilhas",
        "Primeiros socorros",
        "Sobrevivencia",
        "Coletar recursos",
        "Domar animais",
        "Conhecimento religioso"
    };
    
    var skill_ids := array{2, 16, 21, 33, 38, 48, 17, 20, 32, 35, 23};
    
    for i := 1 to skill_names.size()
        var skill_name := skill_names[i];
        var skill_internal_name := skill_internal_names[i];
        
        var skill_value := CInt(AP_GetSkill(who, skill_internal_name));
        var skill_base := CInt(AP_GetTrueSkill(who, skill_internal_name));
        var skill_cap := CInt(AP_GetSkillCap(who, skill_internal_name));
        
        // Buscar ícone do icons.cfg
        var icon_data := GetSkillIcon(skill_name);
        
        skills.append(struct{
            "name" := skill_name,  // Nome exibido
            "desc" := skill_descs[i],
            "iconid" := icon_data.iconid,
            "hue" := icon_data.hue,
            "index" := skill_ids[i],
            "current_value" := skill_value,
            "base_value" := skill_base,
            "cap" := skill_cap,
            "lock" := 0  // Default: Up (0=up, 1=down, 2=locked)
        });
    endfor
    
    return skills;

endfunction


// ? Helper functions now loaded from includes
