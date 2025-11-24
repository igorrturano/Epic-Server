use uo;
use os;

include "packets";

// Packet 0xE0 - Light Fade (Dynamic Length)
// Format:
// Byte 0: 0xE0
// Byte 1-2: Length (15)
// Byte 3-6: Serial (Player)
// Byte 7: Level (0-30)
// Byte 8: R
// Byte 9: G
// Byte 10: B
// Byte 11-14: Duration (ms)

function SendLightFade(who, level, r, g, b, duration)
    if (!who.ip)
        return 0;
    endif

    var packet := CreatePacket(0xE0, 15);
    packet.SetInt16(1, 15); // Length
    packet.SetInt32(3, who.serial);
    packet.SetInt8(7, level);
    packet.SetInt8(8, r);
    packet.SetInt8(9, g);
    packet.SetInt8(10, b);
    packet.SetInt32(11, duration);

    packet.SendPacket(who);
    return 1;
endfunction
