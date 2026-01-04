// ============================================================================
// Hotbar Packet Writing Module
// ============================================================================
// Purpose: Functions that calculate packet sizes and write data to packets
// Functions: Calculate*Size, Write*Section, SetInt16BE
// ============================================================================

use uo;

// ============================================================================
// Size Calculation Functions
// ============================================================================

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
        size := size + 2 + len(item.name);   // name length + name
        size := size + 2 + len(item.classe); // classe length + classe
        size := size + 2 + len(item.desc);   // desc length + desc
        size := size + 2;                     // iconid
        size := size + 2;                     // hue
        size := size + 1;                     // index
        size := size + 2;                     // delay
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
        size := size + 2;                         // iconid
        size := size + 1;                         // index
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

// ============================================================================
// Helper Functions
// ============================================================================

// Write Int16 in Big Endian format
function SetInt16BE(packet, offset, value)
    var high_byte := (value >> 8) & 0xFF;
    var low_byte := value & 0xFF;
    packet.SetInt8(offset, high_byte);
    packet.SetInt8(offset + 1, low_byte);
endfunction

// ============================================================================
// Section Writing Functions
// ============================================================================

function WriteAbilitiesSection(packet, offset, items)
    SetInt16BE(packet, offset, items.size());
    offset := offset + 2;
    // No items to write for now (client uses separate format)
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

        // spell_id (CRITICAL FIELD)
        SetInt16BE(packet, offset, item.id); offset += 2;

        // command_prefix
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
    // No items to write for now (client uses separate format)
    return offset;
endfunction
