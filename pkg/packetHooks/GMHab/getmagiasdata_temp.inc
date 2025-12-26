// Get magias (spells) data from player's grimório
function GetMagiasData(who)
    print("[GMHab DEBUG] GetMagiasData called for " + who.name);
    var magias := array{};
    
    // Find player's spellbook (grimório)
    var book := FindSpellbook(who);
    print("[GMHab DEBUG] FindSpellbook returned: " + book);
    if (!book)
        print("[GMHab DEBUG] Returning 0 magias");
        return magias;  // Return empty array if no spellbook
    endif
    
    // Get spells from grimório
    var spells := GetObjProperty(book, "spells");
    print("[GMHab DEBUG] Spells from grimorio: " + spells);
    if (!spells || TypeOf(spells) != "Array")
        print("[GMHab DEBUG] Returning 0 magias");
        return magias;  // Return empty array if no spells
    endif
    
    // Load spell config to get spell details
    var spell_cfg := ReadConfigFile(":spells:config/spellcaster");
    if (!spell_cfg)
        print("[GMHab DEBUG] ERROR: Could not load spellcaster.cfg!");
        return magias;
    endif
    
    // Convert spells to magia data format
    foreach spell_struct in spells
        var magia := struct;
        
        // Extract spell ID
        var spell_id := 0;
        if (spell_struct.id)
            spell_id := CInt(spell_struct.id);
        else
            continue;  // Skip if no ID
        endif
        
        // Load spell data from config
        var spell_elem := FindConfigElem(spell_cfg, spell_id);
        if (!spell_elem)
            print("[GMHab DEBUG] WARNING: Spell ID " + spell_id + " not found in config!");
            continue;
        endif
        
        // Read spell data from config
        magia.+spell_id := spell_id;
        magia.+name := GetConfigString(spell_elem, "Name");
        magia.+desc := GetConfigString(spell_elem, "Desc");
        magia.+school := GetConfigString(spell_elem, "School");
        magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
        magia.+circle := CInt(GetConfigString(spell_elem, "Circle"));
        
        // Icon ID - use spell ID + offset or from config if available
        var icon_str := GetConfigString(spell_elem, "IconId");
        if (icon_str)
            magia.+iconid := CInt(icon_str);
        else
            magia.+iconid := spell_id + 8000;  // Default icon offset
        endif
        
        magias.append(magia);
        print("[GMHab DEBUG] Added spell: " + magia.name + " (ID: " + magia.spell_id + ", Icon: " + magia.iconid + ")");
    endforeach
    
    print("[GMHab DEBUG] Returning " + magias.size() + " magias");
    return magias;
endfunction
