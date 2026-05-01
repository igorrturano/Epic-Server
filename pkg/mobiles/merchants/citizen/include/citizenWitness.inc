// =============================================================================
// citizenWitness.inc — Crime-site notification helper.
//
// Exported function NotifyCitizenWitnesses(actor, crime_type, payload).
// Called from the four crime hook sites (sneak.src, mechanicalapitude.src,
// cast_spell.src, requestAttack.src). Filters to citizen-template NPCs in the
// actor's line of sight and starts witness.src for each.
//
// All Phase-1 crime detection is gated on InValtheris(actor) — the citizen
// pipeline only fires inside Valtheris. Per-jurisdiction generalisation is
// Phase 2.
// =============================================================================

use uo;
use os;
use polsys;

include ":faccao:include/imperium";

const CITIZEN_TEMPLATE       := ":merchants:citizen";
const CITIZEN_WITNESS_RADIUS := 8;

exported function NotifyCitizenWitnesses(actor, crime_type, payload := struct)
    if (!actor)
        return;
    endif
    if (!InValtheris(actor))
        return;
    endif

    foreach mob in ListMobilesInLineOfSight(actor, CITIZEN_WITNESS_RADIUS)
        if (mob.serial == actor.serial)
            sleepms(1);
            continue;
        endif
        if (mob.npctemplate != CITIZEN_TEMPLATE)
            sleepms(1);
            continue;
        endif
        Start_Script(":merchants:citizen/witness", array{ mob, actor, crime_type, payload });
        sleepms(1);
    endforeach
endfunction
