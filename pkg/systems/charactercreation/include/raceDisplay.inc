// Single source of truth for the race display name with the "Meio-" half-race
// prefix, e.g. "Humano", "Meio-Elfo". This is the canonical anti-metagaming
// name a player shows before being recognized (.r). Kept side-effect free
// (no `use`, no top-level statements) so it can be safely included anywhere:
// the Character method script, the creation gump (pre-persist), creationUtils,
// and the incognito subsystem.
//
// The culture/race string literals mirror enums CULTURA/RACA in
// scripts/include/epicConstants.inc (AIGLANA/LESTE, ELFO/DROW/ORC).
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
