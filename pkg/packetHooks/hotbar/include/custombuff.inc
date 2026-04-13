// ============================================================================
// Custom Buff Packet Functions
// ============================================================================
// Sends buff/debuff icons to the client using ExternalImages/gumps icons.
// Uses packet 0xD7 subcommand 0x62.
//
// Usage:
//   SendCustomBuff(who, buffId, gumpIcon, durationSecs, name, desc)
//   RemoveCustomBuff(who, buffId)
//   SendSpellBuff(who, targ, spellId, durationSecs, desc)      // auto-reads spell cfg
//   SendAbilityBuff(targ, abilityId, durationSecs, desc)        // auto-reads hab cfg
//
// BuffId offsets (to avoid collisions):
//   Spells:    spellId directly (0-9999)
//   Abilities: abilityId + 10000 (10000-19999)
// ============================================================================

use uo;
use cfgfile;

include ":hotbar:include/shared_constants";
include ":hotbar:include/hotbar_write";

const ABILITY_BUFF_OFFSET := 10000;

// Send a custom buff icon to the client
// @param who       - the player to receive the buff icon
// @param buffId    - unique buff identifier (e.g. spell ID, ability ID)
// @param gumpIcon  - gump graphic ID from ExternalImages/gumps/
// @param durationSecs - duration in seconds
// @param name      - buff display name
// @param description - optional description text
function SendCustomBuff(who, buffId, gumpIcon, durationSecs, name, description := "", totalDurationSecs := 0)
    RemoveCustomBuff(who, buffId);
    if (!totalDurationSecs)
        totalDurationSecs := durationSecs;
    endif
    var name_len := len(name);
    var desc_len := len(description);
    // Header: packetId(1) + size(2) + subcommand(2) = 5
    // Data: action(1) + buffId(2) + gumpId(2) + duration(2) + nameLen(2) + name + descLen(2) + desc + totalDuration(2)
    var packet_size := 5 + 1 + 2 + 2 + 2 + 2 + name_len + 2 + desc_len + 2;

    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_CUSTOM_BUFF);
    packet.SetInt8(5, 1); // action: add

    var offset := 6;
    offset := SetInt16BE(packet, offset, buffId);
    offset := SetInt16BE(packet, offset, gumpIcon);
    offset := SetInt16BE(packet, offset, durationSecs);
    offset := WriteStringBE(packet, offset, name);
    offset := WriteStringBE(packet, offset, description);
    offset := SetInt16BE(packet, offset, totalDurationSecs);

    packet.SendPacket(who);
endfunction

// Remove a custom buff icon from the client
// @param who    - the player
// @param buffId - the same identifier used in SendCustomBuff
function RemoveCustomBuff(who, buffId)
    // Header: packetId(1) + size(2) + subcommand(2) = 5
    // Data: action(1) + buffId(2) = 3
    var packet_size := 5 + 1 + 2;

    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_CUSTOM_BUFF);
    packet.SetInt8(5, 0); // action: remove

    SetInt16BE(packet, 6, buffId);

    packet.SendPacket(who);
endfunction

// Convenience: send a spell buff reading icon/hue/name from spell config
// @param who          - the caster
// @param targ         - the target receiving the buff icon
// @param spellId      - spell ID (matches Spell cfg entry, e.g. 715)
// @param durationSecs - duration in seconds
// @param description  - optional description (e.g. "+10 AR")
function SendSpellBuff(who, targ, spellId, durationSecs, description := "", totalDurationSecs := 0)
    var spell_cfgs := array{
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
        ":spells:config/trickster",
        ":spells:config/shinobi"
    };

    foreach cfg_path in spell_cfgs
        var cfg := ReadConfigFile(cfg_path);
        if (!cfg)
            continue;
        endif

        var elem := FindConfigElem(cfg, spellId);
        if (elem)
            var icon := CInt(GetConfigString(elem, "Icon"));
            var name := GetConfigString(elem, "Name");

            if (!icon)
                return; // No icon configured
            endif

            SendCustomBuff(targ, spellId, icon, durationSecs, name, description, totalDurationSecs);
            return;
        endif
    endforeach
endfunction

// Convenience: remove a spell buff by spell ID
// @param targ    - the target to remove the buff icon from
// @param spellId - spell ID used in SendSpellBuff
function RemoveSpellBuff(targ, spellId)
    RemoveCustomBuff(targ, spellId);
endfunction

// Convenience: send an ability buff reading icon/hue/name from habilidades.cfg
// @param targ         - the player receiving the buff icon
// @param abilityId    - ability ID (matches hab cfg entry, e.g. 404)
// @param durationSecs - duration in seconds
// @param description  - optional description (e.g. "+50 Stats")
function SendAbilityBuff(targ, abilityId, durationSecs, description := "")
    var cfg := ReadConfigFile(":abilities:configs/habilidades");
    if (!cfg)
        return;
    endif

    var elem := FindConfigElem(cfg, abilityId);
    if (!elem)
        return;
    endif

    var icon := CInt(GetConfigString(elem, "icon"));
    var name := GetConfigString(elem, "nome");

    if (!icon)
        return;
    endif

    SendCustomBuff(targ, abilityId + ABILITY_BUFF_OFFSET, icon, durationSecs, name, description);
endfunction

// Convenience: remove an ability buff by ability ID
// @param targ      - the player to remove the buff icon from
// @param abilityId - ability ID used in SendAbilityBuff
function RemoveAbilityBuff(targ, abilityId)
    RemoveCustomBuff(targ, abilityId + ABILITY_BUFF_OFFSET);
endfunction
