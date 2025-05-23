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