/* 
 * Hotbar Cooldown Utility
 * 
 * Sends 0xBF Extended Packet with subcommand 0x009A
 * Notifies client of action cooldowns
 * 
 * Packet Structure:
 *   Byte 0:    0xBF (Packet ID)
 *   Bytes 1-2: Packet Length (11 bytes)
 *   Bytes 3-4: Subcommand 0x009A
 *   Bytes 5-8: Action ID (uint, Big Endian)
 *   Bytes 9-10: Cooldown Duration in seconds (ushort, Big Endian)
 */

use uo;

/*
 * SendHotbarCooldown
 * 
 * Sends a cooldown notification to the client
 * 
 * Parameters:
 *   who - Character to send cooldown to
 *   actionId - Action ID (uint)
 *   duration - Cooldown duration in seconds (ushort, max 65535)
 * 
 * Returns:
 *   1 on success, 0 on failure
 */
function SendHotbarCooldown(who, actionId, duration)
    if (!who || !who.connected)
        return 0;
    endif
    
    // Validate duration
    if (duration < 0 || duration > 65535)
        Print($"[HOTBAR] ERROR: Invalid cooldown duration {duration}");
        return 0;
    endif
    
    // Create packet (11 bytes total)
    var packet := CreatePacket(0xBF, 11);
    
    // Set packet length
    packet.SetInt16(1, 11);
    
    // Set subcommand (0x009A)
    packet.SetInt16(3, 0x009A);
    
    // Set action ID (Big Endian)
    packet.SetInt32(5, actionId);
    
    // Set cooldown duration (Big Endian)
    packet.SetInt16(9, duration);
    
    // Send packet
    packet.SendPacket(who);
    
    Print($"[HOTBAR] Cooldown sent to {who.name}: ActionID={actionId}, Duration={duration}s");
    
    return 1;
endfunction

/*
 * SendPosturaCooldown
 * 
 * Convenience function for postura cooldowns
 * 
 * Parameters:
 *   who - Character to send cooldown to
 *   posturaType - Postura type (0=Slash, 1=Piercing, 2=Blunt)
 *   duration - Cooldown duration in seconds
 */
function SendPosturaCooldown(who, posturaType, duration)
    return SendHotbarCooldown(who, posturaType, duration);
endfunction

/*
 * SendHabilidadeCooldown
 * 
 * Convenience function for habilidade cooldowns
 * 
 * Parameters:
 *   who - Character to send cooldown to
 *   habId - Habilidade ID
 *   duration - Cooldown duration in seconds
 */
function SendHabilidadeCooldown(who, habId, duration)
    return SendHotbarCooldown(who, habId, duration);
endfunction

/*
 * SendSpecialCooldown
 * 
 * Convenience function for special ability cooldowns
 * 
 * Parameters:
 *   who - Character to send cooldown to
 *   specialId - Special ability ID
 *   duration - Cooldown duration in seconds
 */
function SendSpecialCooldown(who, specialId, duration)
    return SendHotbarCooldown(who, specialId, duration);
endfunction

/*
 * SendSpellCooldown
 * 
 * Convenience function for spell cooldowns
 * 
 * Parameters:
 *   who - Character to send cooldown to
 *   spellId - Spell ID
 *   duration - Cooldown duration in seconds
 */
function SendSpellCooldown(who, spellId, duration)
    return SendHotbarCooldown(who, spellId, duration);
endfunction

/*
 * SendSkillCooldown
 * 
 * Convenience function for skill cooldowns
 * 
 * Parameters:
 *   who - Character to send cooldown to
 *   skillId - Skill ID
 *   duration - Cooldown duration in seconds
 */
function SendSkillCooldown(who, skillId, duration)
    return SendHotbarCooldown(who, skillId, duration);
endfunction
