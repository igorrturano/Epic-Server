// Helper function to get spell icon from icons.cfg
function GetSpellIcon(spell_name)
    var icon_cfg := ReadConfigFile(":gmhab:config/icons");
    if (!icon_cfg)
        return struct{ "iconid" := 0x5DCD, "hue" := 0 };
    endif
    
    // Create safe config name (remove spaces and special chars)
    var safe_name := "";
    for i := 1 to len(spell_name)
        var ch := spell_name[i];
        if ((ch >= "a" && ch <= "z") || (ch >= "A" && ch <= "Z") || (ch >= "0" && ch <= "9"))
            safe_name := safe_name + ch;
        endif
    endfor
    
    var elem := FindConfigElem(icon_cfg, "spell " + safe_name);
    if (!elem)
        // Default spell icon if not found
        return struct{ "iconid" := 0x5DCD, "hue" := 0 };
    endif
    
    var iconid := CInt(GetConfigString(elem, "iconid"));
    var hue := CInt(GetConfigString(elem, "hue"));
    
    return struct{ "iconid" := iconid, "hue" := hue };
endfunction
