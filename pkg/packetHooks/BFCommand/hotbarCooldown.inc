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

    return 1;
endfunction
