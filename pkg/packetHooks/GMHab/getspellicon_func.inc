// Helper function to get spell icon from icons.cfg
function GetSpellIcon(spell_name)
    print("[ICON DEBUG] Looking for icon: '" + spell_name + "'");
    var icon_cfg := ReadConfigFile(":gmhab:config/icons");
    if (!icon_cfg)
        print("[ICON DEBUG] Failed to read icons.cfg");
        return struct{ "iconid" := 0x5DCD, "hue" := 0 };
    endif
    
    // Try 1: Search with original name (with spaces and accents)
    var lookup_key := "spell " + spell_name;
    print("[ICON DEBUG] Try 1 - lookup_key: '" + lookup_key + "'");
    var elem := FindConfigElem(icon_cfg, lookup_key);
    
    if (elem)
        var iconid := CInt(GetConfigString(elem, "iconid"));
        var hue := CInt(GetConfigString(elem, "hue"));
        print("[ICON DEBUG] Try 1 SUCCESS - iconid: 0x" + Hex(iconid) + ", hue: " + hue);
        return struct{ "iconid" := iconid, "hue" := hue };
    endif
    print("[ICON DEBUG] Try 1 failed");
    
    // Try 2: Remove spaces and normalize accents
    var safe_name := "";
    for i := 1 to len(spell_name)
        var ch := spell_name[i];
        
        // Keep alphanumeric
        if ((ch >= "a" && ch <= "z") || (ch >= "A" && ch <= "Z") || (ch >= "0" && ch <= "9"))
            safe_name := safe_name + ch;
        // Replace accented characters with non-accented versions
        elseif (ch == "á" || ch == "à" || ch == "â" || ch == "ã")
            safe_name := safe_name + "a";
        elseif (ch == "é" || ch == "è" || ch == "ê")
            safe_name := safe_name + "e";
        elseif (ch == "í" || ch == "ì" || ch == "î")
            safe_name := safe_name + "i";
        elseif (ch == "ó" || ch == "ò" || ch == "ô" || ch == "õ")
            safe_name := safe_name + "o";
        elseif (ch == "ú" || ch == "ù" || ch == "û")
            safe_name := safe_name + "u";
        elseif (ch == "ç")
            safe_name := safe_name + "c";
        elseif (ch == "Á" || ch == "À" || ch == "Â" || ch == "Ã")
            safe_name := safe_name + "A";
        elseif (ch == "É" || ch == "È" || ch == "Ê")
            safe_name := safe_name + "E";
        elseif (ch == "Í" || ch == "Ì" || ch == "Î")
            safe_name := safe_name + "I";
        elseif (ch == "Ó" || ch == "Ò" || ch == "Ô" || ch == "Õ")
            safe_name := safe_name + "O";
        elseif (ch == "Ú" || ch == "Ù" || ch == "Û")
            safe_name := safe_name + "U";
        elseif (ch == "Ç")
            safe_name := safe_name + "C";
        endif
        // Skip other characters (spaces, special chars)
    endfor
    
    lookup_key := "spell " + safe_name;
    print("[ICON DEBUG] Try 2 - safe_name: '" + safe_name + "', lookup_key: '" + lookup_key + "'");
    elem := FindConfigElem(icon_cfg, lookup_key);
    
    if (elem)
        var iconid := CInt(GetConfigString(elem, "iconid"));
        var hue := CInt(GetConfigString(elem, "hue"));
        print("[ICON DEBUG] Try 2 SUCCESS - iconid: 0x" + Hex(iconid) + ", hue: " + hue);
        return struct{ "iconid" := iconid, "hue" := hue };
    endif
    print("[ICON DEBUG] Try 2 failed");
    
    // Try 3: Just the safe name without "spell " prefix
    print("[ICON DEBUG] Try 3 - safe_name only: '" + safe_name + "'");
    elem := FindConfigElem(icon_cfg, safe_name);
    
    if (elem)
        var iconid := CInt(GetConfigString(elem, "iconid"));
        var hue := CInt(GetConfigString(elem, "hue"));
        print("[ICON DEBUG] Try 3 SUCCESS - iconid: 0x" + Hex(iconid) + ", hue: " + hue);
        return struct{ "iconid" := iconid, "hue" := hue };
    endif
    print("[ICON DEBUG] Try 3 failed");
    
    // Not found - return default
    print("[ICON DEBUG] NOT FOUND - returning default icon 0x5DCD");
    return struct{ "iconid" := 0x5DCD, "hue" := 0 };
endfunction
