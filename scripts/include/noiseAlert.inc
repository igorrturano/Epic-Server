use uo;
use os;

include "include/sysEvent";

const NOISE_DEFAULT_RADIUS := 40;
const NOISE_PER_SOURCE_COOLDOWN := 1;

function EmitNoise(source, radius := NOISE_DEFAULT_RADIUS)
    if (!source)
        return 0;
    endif

    var now := polcore().systime;
    var last := CInt(GetObjProperty(source, "#last_noise_t"));
    if (last && (now - last) < NOISE_PER_SOURCE_COOLDOWN)
        return 0;
    endif
    SetObjProperty(source, "#last_noise_t", now);

    var nearby := ListMobilesNearLocationEx(
        source.x, source.y, LIST_IGNORE_Z, radius,
        LISTEX_FLAG_NORMAL + LISTEX_FLAG_NPC_ONLY,
        source.realm
    );

    var ev := struct{
        "type"    := EVID_NOISE,
        "source"  := source,
        "noise_x" := source.x,
        "noise_y" := source.y
    };

    var sent := 0;
    foreach npc in nearby
        if (!npc || npc.dead || npc.serial == source.serial)
            continue;
        endif
        SendEvent(npc, ev);
        sent += 1;
    endforeach

    return sent;
endfunction
