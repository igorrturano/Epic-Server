// =============================================================================
// citizenAuthority.inc — Locate nearby authority NPCs by faction match.
//
// A "faction" on the citizen is stored as an integer index 1..5 into
// FACCAO_NOMES (see :faccao:include/faccao). To match a player or NPC's
// faction we look at:
//   1. mob.guild.name  (native guild membership; primary path)
//   2. GetObjProperty(mob, "nomefaccao")  (integer 1..5; secondary path)
//
// The legacy "faccao" ObjProperty is intentionally ignored — only NPC-vendor
// codes ("A.C." etc.) live there; canonical faction strings are never written
// to it (verified with grep across pkg/ and scripts/).
// =============================================================================

use uo;
use os;
use guilds;

include ":faccao:include/faccao";

const AUTHORITY_SCAN_RADIUS := 12;

// Returns the canonical faction index (1..5) for a mobile, or 0 if none.
function GetMobileFactionIndex(mob)
    if (!mob)
        return 0;
    endif

    if (mob.guild)
        var idx := 1;
        foreach name in (FACCAO_NOMES)
            if (mob.guild.name == name)
                return idx;
            endif
            idx := idx + 1;
            sleepms(1);
        endforeach
    endif

    var n := GetObjProperty(mob, "nomefaccao");
    if (n != error)
        var ni := CInt(n);
        if (ni >= 1 && ni <= FACCAO_NOMES.size())
            return ni;
        endif
    endif

    return 0;
endfunction

// Returns the canonical faction name string for a mobile, or "".
function GetMobileFactionName(mob)
    var idx := GetMobileFactionIndex(mob);
    if (!idx)
        return "";
    endif
    return FACCAO_NOMES[idx];
endfunction

// Returns the closest NPC whose faction index is in the citizen's configured
// list, or 0 if none in range. `factions` is array of integer indexes.
function FindNearbyAuthority(citizen, factions)
    if (!factions || !factions.size())
        return 0;
    endif

    var best := 0;
    var best_dist := AUTHORITY_SCAN_RADIUS + 1;

    foreach mob in ListMobilesNearLocation(citizen.x, citizen.y, citizen.z, AUTHORITY_SCAN_RADIUS, citizen.realm)
        if (mob.serial == citizen.serial || !mob.npctemplate)
            sleepms(1);
            continue;
        endif
        var idx := GetMobileFactionIndex(mob);
        if (!idx || !(idx in factions))
            sleepms(1);
            continue;
        endif
        var d := Distance(citizen, mob);
        if (d < best_dist)
            best := mob;
            best_dist := d;
        endif
        sleepms(1);
    endforeach

    return best;
endfunction
