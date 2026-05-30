use uo;

include ":tn:equipment";

// Consolidated incognito (face-covering mask) subsystem.
//
// One synchronous, idempotent source of truth for applying/removing the
// "Desconhecido" state. Replaces the old async scripts/misc/applyIncognito.src
// (sleepms race) and the three divergent inline remove copies
// (unequiptest.src / combat/onDestroy.src / disguise.inc).
//
// Reveal RECOMPUTES the race display name from chardata via the
// who.GetRaceDisplay() Character method instead of trusting a fragile stash,
// so a missing/late stash can never leave a player stuck as "Desconhecido".
//
// realname/realrace are still WRITTEN (= race display string) so the ~10
// value-consumers and the 3 display hooks that key on `realname` keep working
// unchanged; reveal simply no longer DEPENDS on them.
//
// Guard & isolate vs the disguise kit: these functions no-op while a disguise
// (`disfarce`) is active — the disguise kit owns `realname` in that flow.

const INCOGNITO_URL := "https://th.bing.com/th/id/OIG.0XmfkJZCmPd.wGE6C9x_?pid=ImgGn&w=1024&h=1024&rs=1";

// Incognito only counts when a disguise is NOT active (disguise reuses the
// shared `realname` key with a different meaning).
function IsIncognito(who)
    if (GetObjProperty(who, "disfarce"))
        return 0;
    endif
    return (GetObjProperty(who, "Incognito") == "true");
endfunction

function ApplyIncognito(who)
    if (who.npctemplate)
        return 0;
    endif
    if (GetObjProperty(who, "disfarce"))
        return 0;
    endif
    // Masks are NOT blocked at equip while polymorphed (combat/equip.src only
    // blocks weapon/armor), so guard here: going incognito while metamorfosed
    // would leave the player stuck when the form reverts. Error == not set.
    if (GetObjProperty(who, "metamorfose") != error)
        return 0;
    endif
    if (IsIncognito(who))
        return 1;
    endif

    var raceDisplay := who.GetRaceDisplay();
    SetObjProperty(who, "realname", raceDisplay);
    SetObjProperty(who, "realrace", raceDisplay);

    if (GetObjProperty(who, "description"))
        SetObjProperty(who, "olddescription", GetObjProperty(who, "description"));
        EraseObjProperty(who, "description");
    endif

    who.name := "Desconhecido";
    who.title_race := "Desconhecido";
    SetObjProperty(who, "Incognito", "true");

    // Only stash old_url once, so a double-apply can't overwrite the real
    // portrait with the incognito one.
    if (!GetObjProperty(who, "old_url"))
        SetObjProperty(who, "old_url", GetObjProperty(who, "url"));
    endif
    SetObjProperty(who, "url", INCOGNITO_URL);

    return 1;
endfunction

// exclude_item: the mask leaving the body (unequip/destroy). UnequipTest fires
// while the item is still on its layer, so it must be excluded from the
// stacked-mask scan or we would never reveal.
function RemoveIncognito(who, exclude_item := 0)
    if (GetObjProperty(who, "disfarce"))
        return 0;
    endif
    if (!IsIncognito(who) && !GetObjProperty(who, "realname"))
        return 1;
    endif

    // Stay incognito while any OTHER face-covering item is still worn.
    var exclude_serial := exclude_item ? exclude_item.serial : 0;
    foreach eq in (EnumeratePhysicalItems(who))
        if (eq.serial != exclude_serial && coverFace(eq))
            return 0;
        endif
    endforeach

    var raceDisplay := who.GetRaceDisplay();
    who.name := raceDisplay;
    who.title_race := raceDisplay;

    if (GetObjProperty(who, "olddescription"))
        SetObjProperty(who, "description", GetObjProperty(who, "olddescription"));
        EraseObjProperty(who, "olddescription");
    endif

    if (GetObjProperty(who, "old_url"))
        SetObjProperty(who, "url", GetObjProperty(who, "old_url"));
        EraseObjProperty(who, "old_url");
    endif

    EraseObjProperty(who, "realname");
    EraseObjProperty(who, "realrace");
    EraseObjProperty(who, "Incognito");

    return 1;
endfunction
