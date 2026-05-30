// ============================================================================
// Hotbar Packet Writing Module
// ============================================================================
// Purpose: Functions that calculate packet sizes and write data to packets
// Functions: CalculateAbilitiesSize, CalculateCombatHotkeysSize, CalculateSpellsSize,
//            CalculateCommandsSize, CalculateSkillsSize, SetInt16BE, WriteStringBE
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

function CalculateCombatHotkeysSize(items)
    var size := 0;
    foreach item in items
        size := size + 2 + len(item.name);   // name length + name
        size := size + 2 + len(item.classe); // classe length + classe
        size := size + 2 + len(item.desc);   // desc length + desc
        size := size + 2;                     // iconid
        size := size + 2;                     // hue
        size := size + 1;                     // index
        size := size + 2;                     // delay
        size := size + 2;                     // remaining (added for combat hotkeys)
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
        size := size + 2;                         // desc length (empty string, loaded on hover)
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

// Write Int16 in Big Endian format and return new offset
function SetInt16BE(byref packet, offset, value)
    var high_byte := CInt(value / 256);
    var low_byte := value % 256;
    packet.SetInt8(offset, high_byte);
    packet.SetInt8(offset + 1, low_byte);
    return offset + 2;
endfunction

// Write a length-prefixed string and return new offset
function WriteStringBE(byref packet, offset, str)
    var str_len := len(str);
    offset := SetInt16BE(packet, offset, str_len);
    packet.SetString(offset, str, 0);
    return offset + str_len;
endfunction
