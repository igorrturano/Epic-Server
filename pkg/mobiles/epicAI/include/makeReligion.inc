// ============================================================================
// makeReligion.inc - cold-init religion setter for NPCs
// ============================================================================
// Mirror of makeRace: picks (or accepts) a religion ONCE at NPC spawn and
// persists it via ObjProperty. MobileWrapper.GetReligion reads the property
// lazily at runtime.
//
// Called from epicNPCInitialization.inc after makeRace, conditional on the
// NPC's ability pool:
//   - CONEXAO_DIVINA in habs  -> random cleric religion (KAI/VENERATIO/HALLEUAD)
//   - ARCANE_BOW / ARCANE_MASTERY / ELEMENTAL_AFFINITY in habs -> LORE
//   - Otherwise: not called; GetReligion returns IsNPCException as before.
//
// Religions excluded from the cleric pool because their NPC branches are
// impractical (party-dependent or requires complex area reveal logic):
//   - BOKK_BHARAZ (shield wall requires party members)
//   - ELUITHORTH (truesight area reveal, complex)
// ============================================================================

include "include/epicConstants";   // RELIGIAO enum constants

function makeReligion(mobile, religion := 0)
    if (!religion)
        var pool := _ReligionPoolFor(mobile);
        if (!pool || pool.size() == 0)
            return 0;
        endif
        religion := pool[RandomInt(pool.size()) + 1];
    endif

    SetObjProperty(mobile, "npc_religion", religion);
    return religion;
endfunction

// Returns array of religion strings (RELIGIAO enum values) valid for the NPC,
// based on which ability tree they're on. Empty array = no religion.
function _ReligionPoolFor(mobile)
    var habs := GetObjProperty(mobile, "myhabs");
    if (!habs)
        return array{};
    endif

    // Arqueiro Arcano: needs LORE for ARCANE_BOW.CanActivate to pass
    if (ABILITY_ID::ARCANE_BOW in habs ||
        ABILITY_ID::ARCANE_MASTERY in habs ||
        ABILITY_ID::ELEMENTAL_AFFINITY in habs)
        return array{LORE};
    endif

    // Cleric: 3 religions with simple NPC-friendly branches
    if (ABILITY_ID::CONEXAO_DIVINA in habs)
        return array{KAI, VENERATIO, HALLEUAD};
    endif

    return array{};
endfunction
