use uo;

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

// Race display string with the "Meio-" half-race prefix, e.g. "Humano",
// "Meio-Elfo". This is the canonical anti-metagaming name a player shows
// before being recognized (see racialSelection in epicgumpcriacao.src).
// Pure: operates on an in-memory chardata struct so it can be called during
// character creation, before chardata is persisted. The culture/race string
// literals mirror enums CULTURA/RACA in scripts/include/epicConstants.inc.
exported function RaceDisplayFromChardata(chardata)
    if (!chardata || chardata == error)
        return "";
    endif
    var prefix := "";
    if ((chardata.culture == "Aiglana" || chardata.culture == "Povos do Leste")
        && (chardata.raca == "Elfo" || chardata.raca == "Drow" || chardata.raca == "Orc"))
        prefix := "Meio-";
    endif
    return prefix + chardata.raca;
endfunction

// Convenience wrapper that reads the persisted chardata. Falls back to
// title_race only if chardata is missing (should not happen for a real player).
exported function GetRaceDisplay(who)
    var cd := GetObjProperty(who, "chardata");
    if (!cd || cd == error)
        return who.title_race;
    endif
    return RaceDisplayFromChardata(cd);
endfunction