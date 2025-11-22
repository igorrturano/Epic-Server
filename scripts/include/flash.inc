use uo;
use os;
use polsys;

// Constants for Screen Flash Types (must match Client Constants)
const FLASH_FADEOUT       := 0;
const FLASH_FADEIN        := 1;
const FLASH_LIGHT         := 2;
const FLASH_FADEINOUT     := 3;
const FLASH_DARK          := 4;
const FLASH_WHITE         := 5;

/*
 * SendScreenFlash(who, flash_type)
 *
 * Purpose
 * Sends a 0x70 packet to trigger a screen flash effect in the client.
 *
 * Parameters
 * who:        The character to send the packet to.
 * flash_type: The type of flash (0-5).
 *
 * Packet 0x70 Structure for ScreenFade:
 * Byte 0: 0x70 (Cmd)
 * Byte 1: 0x04 (Type = ScreenFade)
 * Byte 2-9: Skipped by client
 * Byte 10-11: Flash Type (UInt16BE)
 * Byte 12-27: Padding
 */
function SendScreenFlash(who, flash_type)
    if (!who.ip)
        return 0;
    endif

    var packet := CreatePacket(0x70, 28);
    
    packet.SetInt8(1, 0x04); // Effect Type: ScreenFade
    
    // Bytes 2-5: Source Serial (0)
    packet.SetInt32(2, 0);
    
    // Bytes 6-9: Target Serial (Unused by Client for ScreenFade)
    // Client Logic:
    // 1. Reads Type (Byte 1)
    // 2. Skips 8 bytes (Bytes 2-9)
    // 3. Reads UInt16BE (Bytes 10-11) -> This is where the flash_type must be!
    
    // So we need to put the value at offset 10.
    
    packet.SetInt8(10, 0);           // High Byte
    packet.SetInt8(11, flash_type);  // Low Byte
    
    packet.SendPacket(who);
    return 1;
endfunction