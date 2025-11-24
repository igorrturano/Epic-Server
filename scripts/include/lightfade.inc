use uo;
use os;

include "packets";

// Light Fade Style Constants
const LIGHTFADE_STYLE_STATIC   := 0;
const LIGHTFADE_STYLE_FADEIN   := 1;
const LIGHTFADE_STYLE_FADEOUT  := 2;
const LIGHTFADE_STYLE_PULSE    := 3;

/*
 * SendLightFade(who, level, r, g, b, duration, style := 0, transition_time := 0)
 *
 * level: 0-30 (0 = remove effect)
 * r, g, b: Color of the fade (0-255)
 * duration: Duration in milliseconds (0 = infinite)
 * style: Use LIGHTFADE_STYLE_* constants (Static, FadeIn, FadeOut, Pulse)
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
