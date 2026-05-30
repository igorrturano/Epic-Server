use uo;

include ":charactercreation:raceDisplay";

print("INSTALLING: Methods for Character -> Character Creation System... OK!");

exported function getRace(who)
    return GetObjProperty(who, "chardata").raca;
endfunction

exported function getClass(who)
    return GetObjProperty(who, "chardata").classe;
endfunction

exported function getCulture(who)
    return GetObjProperty(who, "chardata").culture;
endfunction

exported function getReligion(who)
    return GetObjProperty(who, "chardata").religion;
endfunction

// RaceDisplayFromChardata lives in :charactercreation:raceDisplay (included
// above) so it stays side-effect free and shareable by the creation gump and
// the incognito subsystem. GetRaceDisplay is the convenience wrapper that
// reads the persisted chardata. Falls back to title_race only if chardata is
// missing (should not happen for a real player).
exported function GetRaceDisplay(who)
    var cd := GetObjProperty(who, "chardata");
    if (!cd || cd == error)
        return who.title_race;
    endif
    return RaceDisplayFromChardata(cd);
endfunction