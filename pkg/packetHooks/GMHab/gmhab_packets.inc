// GMHab Packet Handler - Server Side
// Handles custom packets 0xD7/0x50 (Request) and 0xD7/0x51 (Execute)
// Sends response packets 0xD7/0x52 (Data Response)

use uo;
use os;
use polsys;

include ":gumps:gumps";
include ":fls_core:fls_characters";
include ":datafile:datafile";
include ":timedscripts:timedScripts";
include "gmhab_execute";
include "include/icon_manager";

// Packet IDs
const PACKET_EXTENDED := 0xD7;
const SUBCOMMAND_REQUEST := 0x50;
const SUBCOMMAND_EXECUTE := 0x51;
const SUBCOMMAND_RESPONSE := 0x52;

// Request Types
const REQUEST_HABILIDADES := 0x01;
const REQUEST_SPECIALS := 0x02;
const REQUEST_COMMANDS := 0x03;
const REQUEST_MAGIAS := 0x04;

// Execute Types
const EXECUTE_HABILIDADE := 0x01;
const EXECUTE_SPECIAL := 0x02;
const EXECUTE_COMMAND := 0x03;
const EXECUTE_SKILL := 0x04;

// Main packet handler - called by POL when packet 0xD7 is received
exported function HandleGMHabPacket(who, packet)
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
    SendAllGMHabData(who);
    
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
        EXECUTE_HABILIDADE:
            return ExecuteHabilidade(who, data);
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
function SendHabilidadesList(who)
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
    SendGMHabDataPacket(who, REQUEST_HABILIDADES, hab_list);
    
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
    
    SendGMHabDataPacket(who, REQUEST_SPECIALS, specials);
    
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
    
    SendGMHabDataPacket(who, REQUEST_COMMANDS, commands);
    
    return 1;
endfunction

// Build and send data response packet
function SendGMHabDataPacket(who, data_type, items)
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
        REQUEST_HABILIDADES:
            offset := WriteHabilidadesData(packet, offset, items);
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
function WriteHabilidadesData(byref packet, offset, items)
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
function ExecuteHabilidade(who, hab_name)
    // Call existing ProccessHab function
    return ProccessHab(who, hab_name);
endfunction

// Execute special
function ExecuteSpecial(who, special_index)
    // Call existing special system
    var result := start_script(":specialAbilities:specialAbilities", who);
    if (result)
        SendSysMessage(who, "Executando special ability!");
        return 1;
    else
        SendSysMessage(who, "Erro ao executar special ability!");
        return 0;
    endif
endfunction

// Execute command
function ExecuteGMCommand(who, command)
    // Execute the command by calling the script directly
    SendSysMessage(who, "Executando: " + command);
    
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
        SendSysMessage(who, "Erro ao executar comando! Tente digitar: ." + cmd_name);
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
        SendSysMessage(who, "Indice de skill invalido! ID: " + skill_id);
        return 0;
    endif
    
    // Get the player's skill value
    var skill_value := CInt(AP_GetSkill(who, skill_name));
    
    if (skill_value <= 0)
        SendSysMessage(who, "Voce nao possui pontos em " + skill_name + "!");
        return 0;
    endif
    
    // Get the attribute config element
    var attrib_elem := AP_GetAttributeCfgElem(skill_name);
    if (!attrib_elem)
        SendSysMessage(who, "Skill " + skill_name + " nao encontrada!");
        return 0;
    endif
    
    // Get the script path from the attribute
    var script_path := GetConfigString(attrib_elem, "Script");
    if (!script_path || script_path == "")
        SendSysMessage(who, "Skill " + skill_name + " nao possui script!");
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
    
    SendSysMessage(who, "Usando " + skill_name + " (valor: " + skill_value + ")");
    
    // Execute the skill script with the player's skill value
    var result := start_script(script_path, who);
    if (!result)
        SendSysMessage(who, "Erro ao executar skill!");
        return 0;
    else
        return 1;
    endif
endfunction

// Send all GMHab data in a single unified packet
function SendAllGMHabData(who)
    // Collect all data
    var hab_list := GetHabilidadesData(who);
    var specials := GetSpecialsData(who);
    var magias := GetMagiasData(who);  // ✅ Get magias from grimório
    var commands := GetCommandsData(who);
    var skills := GetSkillsData(who);
    
    // Calculate total packet size
    var packet_size := 6; // Header: PacketID(1) + Length(2) + Subcommand(2) + DataType(1)
    
    // Add size for each data type (count + data)
    var hab_size := CalculateHabilidadesSize(hab_list);
    var spec_size := CalculateSpecialsSize(specials);
    var mag_size := CalculateMagiasSize(magias);
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
        
        // spell_id
        packet.SetInt8(offset, CInt(item.spell_id / 256)); offset += 1;
        packet.SetInt8(offset, item.spell_id % 256); offset += 1;
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
function CalculateHabilidadesSize(items)
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

function CalculateMagiasSize(items)
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
    Print("SetInt16BE: offset=" + offset + ", value=" + value + ", high=" + high_byte + ", low=" + low_byte);
    packet.SetInt8(offset, high_byte);
    packet.SetInt8(offset + 1, low_byte);
endfunction

// Helper functions to write data sections
function WriteHabilidadesSection(packet, offset, items)
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

function WriteMagiasSection(packet, offset, items)
    SetInt16BE(packet, offset, items.size());
    offset := offset + 2;
    // No items to write for now
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

// Get habilidades data - full implementation from gmhab.src lines 199-267
function GetHabilidadesData(who)
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
    
    // Load all habilidades from config
    var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
    if (!habcfg)
        return array{};
    endif
    
    var all_keys := GetConfigStringKeys(habcfg);
    var filtered_habs := array{};
    var hab_index := 1;
    
    // Filter habilidades by class, level, and active status
    foreach hab_name in all_keys
        var habelem := FindConfigElem(habcfg, hab_name);
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
        var required_level := 0;
        
        if (classes[char_class] != error)
            required_level := CInt(classes[char_class]);
            if (char_level >= required_level)
                is_eligible := 1;
            endif
        endif
        
        // Check if purchased (always show purchased abilities)
        var has_purchased := 0;
        foreach key in (myhabs.keys())
            if (Lower(myhabs[key]) == Lower(hab_name))
                has_purchased := 1;
                is_eligible := 1; // Always eligible if purchased
                break;
            endif
        endforeach
        
        // Add to filtered list if eligible
        if (is_eligible)
            // Buscar ícone do icons.cfg
            var icon_data := GetHabilidadeIcon(hab_name, char_class);
            
            // Read additional fields from config
            var stamina_cost := CInt(GetConfigString(habelem, "stam"));
            var mana_cost := CInt(GetConfigString(habelem, "mana"));
            var delay_val := CInt(GetConfigString(habelem, "delay"));
            var requisite_str := GetConfigString(habelem, "requisito");
            var duration_val := CInt(GetConfigString(habelem, "duracao"));
            var script_str := GetConfigString(habelem, "script");
            
            // Set defaults if not found
            if (!stamina_cost) stamina_cost := 0; endif
            if (!mana_cost) mana_cost := 0; endif
            if (!delay_val) delay_val := 0; endif
            if (!requisite_str) requisite_str := ""; endif
            if (!duration_val) duration_val := 0; endif
            if (!script_str) script_str := ""; endif
            
            var hab_data := struct{
                "name" := hab_name,
                "classe" := char_class,
                "desc" := GetConfigString(habelem, "desc"),
                "required_level" := required_level,
                "is_active" := (uso == "Ativa"),
                "is_purchased" := has_purchased,
                "iconid" := icon_data.iconid,
                "hue" := icon_data.hue,
                "stamina_cost" := stamina_cost,
                "mana_cost" := mana_cost,
                "delay" := delay_val,
                "requisite" := requisite_str,
                "duration" := duration_val,
                "script" := script_str
            };
            
            filtered_habs.append(hab_data);
            hab_index := hab_index + 1;
        endif
        
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

// Get magias (spells) data from player's grimório
function GetMagiasData(who)

    var magias := array{};
    
    // Find ALL player's spellbooks (grimórios) - pode ter múltiplos!
    var books := FindAllSpellbooks(who);

    
    if (books.size() == 0)

        return magias;  // Return empty array if no spellbooks
    endif
    
    // Load spell config to get spell details
    var spell_cfg := ReadConfigFile(":spells:config/spellcaster");
    if (!spell_cfg)

        return magias;
    endif
    
    // Process ALL spellbooks
    foreach book in books

        
        // Get spells from this grimório
        var spells := GetObjProperty(book, "spells");

        
        if (!spells || TypeOf(spells) != "Array")

            continue;  // Skip this book if no spells
        endif
        
        // Convert spells to magia data format
        foreach spell_struct in spells
            var magia := struct;
            
            // Extract spell ID
            var spell_id := 0;
            if (spell_struct.id)
                spell_id := CInt(spell_struct.id);
            else
                continue;  // Skip if no ID
            endif
            
            // Load spell data from config
            var spell_elem := FindConfigElem(spell_cfg, spell_id);
            if (!spell_elem)

                continue;
            endif
            
            // Read spell data from config
            magia.+spell_id := spell_id;
            magia.+name := GetConfigString(spell_elem, "Name");
            magia.+desc := GetConfigString(spell_elem, "Desc");
            magia.+school := GetConfigString(spell_elem, "School");
            magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
            magia.+circle := CInt(GetConfigString(spell_elem, "Circle"));
            
            // Get icon from icons.cfg

            var icon_data := GetSpellIcon(magia.name);

            magia.+iconid := icon_data.iconid;
            magia.+hue := icon_data.hue;
            
            magias.append(magia);

        endforeach
    endforeach
    

    return magias;
endfunction

// Find ALL spellbooks (grimórios) - modified to return array of all books
function FindAllSpellbooks(player)

    var books := array{};
    
    // Search in backpack
    foreach item in EnumerateItemsInContainer(player.backpack)

        var booktype := GetObjProperty(item, "Booktype");

        if (booktype == "spellcaster")  // Only spellcaster books

            books.append(item);
        endif
    endforeach
    
    // Search equipped (layer 1 = spellbook)

    var book := GetEquipmentByLayer(player, 0x01);
    if (book)

        var booktype := GetObjProperty(book, "Booktype");

        if (booktype == "spellcaster")

            books.append(book);
        endif
    endif
    

    return books;
endfunction

// Helper function to find player's spellbook
function FindSpellbook(player)

    
    // Search in backpack
    foreach item in EnumerateItemsInContainer(player.backpack)

        var booktype := GetObjProperty(item, "Booktype");

        if (booktype)  // Has Booktype property = custom grimorio

            return item;
        endif
    endforeach
    
    // Search equipped (layer 1 = spellbook)

    var book := GetEquipmentByLayer(player, 0x01);
    if (book)

        var booktype := GetObjProperty(book, "Booktype");

        if (booktype)

            return book;
        endif
    endif
    

    return 0;
endfunction







// Helper function to get spell icon from icons.cfg
function GetSpellIcon(spell_name)
    var icon_cfg := ReadConfigFile(":gmhab:config/icons");
    if (!icon_cfg)
        return struct{ "iconid" := 0x5DCD, "hue" := 0 };
    endif
    
    // Create safe config name (remove spaces and special chars)
    var safe_name := "";
    for i := 1 to len(spell_name)
        var ch := spell_name[i];
        if ((ch >= "a" && ch <= "z") || (ch >= "A" && ch <= "Z") || (ch >= "0" && ch <= "9"))
            safe_name := safe_name + ch;
        endif
    endfor
    
    var elem := FindConfigElem(icon_cfg, "spell " + safe_name);
    if (!elem)
        // Default spell icon if not found
        return struct{ "iconid" := 0x5DCD, "hue" := 0 };
    endif
    
    var iconid := CInt(GetConfigString(elem, "iconid"));
    var hue := CInt(GetConfigString(elem, "hue"));
    
    return struct{ "iconid" := iconid, "hue" := hue };
endfunction
