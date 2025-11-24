use uo;
use os;
use uo;
use os;

include "packets";

// Packet 0xE0 - Light Fade (Dynamic Length)
// Format:
// Byte 0: 0xE0
// Byte 1-2: Lengthuse uo;
use os;
include "packets";

/*
 * SendLightFade(who, level, r, g, b, duration, style := 0, transition_time := 0)
 *
 * level: 0-30 (0 = remove effect)
 * r, g, b: Color of the fade (0-255)
 * duration: Duration in milliseconds (0 = infinite)
 * style: 0=Static, 1=FadeIn, 2=FadeOut, 3=Pulse
 * transition_time: Time for fade transition in ms
 */
function SendLightFade(who, level, r, g, b, duration, style := 0, transition_time := 0)
    if (!who.ip)
        return;
    endif

    var packet := CreatePacket(0xE0, 18); // Correct size: 1 ID + 17 Payload = 18 bytes
    // packet.SetInt16(1, 15); // Length - Removed, length is now implicit in CreatePacket(..., 20) and serial starts at byte 1
    packet.SetInt32(1, who.serial);
    packet.SetInt8(5, level);
    packet.SetInt8(6, r);
    packet.SetInt8(7, g);
    packet.SetInt8(8, b);
    packet.SetInt32(9, duration);
    packet.SetInt8(13, style);
    packet.SetInt32(14, transition_time);

    packet.SendPacket(who);
endfunction
