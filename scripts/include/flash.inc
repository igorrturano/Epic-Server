use uo;
use os;
use polsys;

// Constants for Screen Flash Types (must match Client Constants)
const FLASH_DROW_OFF       := 0; // Nao usar para drows porque desabilita para eles o Drow Vision
const FLASH_FADEIN        := 1;
const FLASH_LIGHT         := 2;
const FLASH_FADEINOUT     := 3;
const FLASH_DARK          := 4;
const FLASH_DROW_ON         := 5;

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
    
    // Bytes 6-9: Target Serial (MUST BE INITIALIZED!)
    packet.SetInt32(6, 0);
    
    // Bytes 10-11: Flash Type (UInt16BE)
    packet.SetInt8(10, 0);           // High Byte
    packet.SetInt8(11, flash_type);  // Low Byte
    
    // Initialize remaining bytes to 0 (12-27)
    var i := 12;
    while (i < 28)
        packet.SetInt8(i, 0);
        i := i + 1;
    endwhile
    
    packet.SendPacket(who);
    return 1;
endfunction