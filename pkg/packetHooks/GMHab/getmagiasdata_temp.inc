// Get magias (spells) data from player's grimório and class abilities
function GetMagiasData(who)
    var magias := array{};
    var chardata := GetObjProperty(who, "chardata");
    
    // 1. MAGE - Natureza Arcana trait + grimório
    if (chardata && chardata.trait == "Natureza Arcana")
        LoadGrimorioSpells(magias, who, ":spells:config/spellcaster", "m");
    endif
    
    // 2. PALADIN - Consagracao Divina ability
    if (TemHabilidade(who, "Consagracao Divina"))
        LoadClassSpells(magias, who, ":spells:config/paladinspells", "Divine", "p", "Theology");
    endif
    
    // 3. DRUID - magia = "Druid" (memorized spells)
    if (chardata && chardata.magia == "Druid")
        LoadMemorizedSpells(magias, who, "druid_list", "Druid", "d");
    endif
    
    // 4. TRICKSTER - Sensibilidade Arcana ability
    if (TemHabilidade(who, "Sensibilidade Arcana"))
        LoadClassSpells(magias, who, ":spells:config/tricksterspells", "Trickster", "m", "Magery");
    endif
    
    // 5. DEMON HUNTER - Sigilo de Protecao ability
    if (TemHabilidade(who, "Sigilo de Protecao"))
        LoadClassSpells(magias, who, ":spells:config/demonhunterspells", "Divine", "c", "Theology");
    endif
    
    // 6. PRIEST - magia = "Sacerdote" (memorized spells)
    if (chardata && chardata.magia == "Sacerdote")
        LoadMemorizedSpells(magias, who, "priest_list", "Priest", "d");
    endif
    
    // 7. SHAMAN - magia = "Shaman" (memorized spells)
    if (chardata && chardata.magia == "Shaman")
        LoadMemorizedSpells(magias, who, "shaman_list", "Shaman", "d");
    endif
    
    // 8. CANTRIPS - spellcaster OR Natureza Arcana OR Arqueiro Arcano
    if ((chardata && chardata.magia == "spellcaster") || 
        (chardata && chardata.trait == "Natureza Arcana") || 
        (chardata && chardata.magia == "Arqueiro Arcano"))
        LoadClassSpells(magias, who, ":spells:config/cantripspells", "Cantrips", "t", "");
    endif
    
    return magias;
endfunction

// Load spells from grimório (for Mage)
function LoadGrimorioSpells(byref magias, who, cfg_path, cmd_prefix)
    var book := FindSpellbook(who);
    if (!book)
        return;
    endif
    
    var spells := GetObjProperty(book, "spells");
    if (!spells || TypeOf(spells) != "Array")
        return;
    endif
    
    var spell_cfg := ReadConfigFile(cfg_path);
    if (!spell_cfg)
        return;
    endif
    
    foreach spell_struct in spells
        var spell_id := 0;
        if (spell_struct.id)
            spell_id := CInt(spell_struct.id);
        else
            continue;
        endif
        
        var spell_elem := FindConfigElem(spell_cfg, spell_id);
        if (!spell_elem)
            continue;
        endif
        
        var magia := struct;
        magia.+spell_id := spell_id;
        magia.+name := GetConfigString(spell_elem, "Name");
        magia.+desc := GetConfigString(spell_elem, "Desc");
        magia.+school := GetConfigString(spell_elem, "School");
        magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
        magia.+circle := CInt(GetConfigString(spell_elem, "Circle"));
        magia.+command_prefix := cmd_prefix;
        
        // ALWAYS get icon from icons.cfg
        var icon_struct := GetSpellIcon(magia.name);
        magia.+iconid := icon_struct.iconid;
        magia.+hue := icon_struct.hue;
        
        magias.append(magia);
    endforeach
endfunction

// Load class-specific spells (Paladin, Druid, Trickster, etc)
function LoadClassSpells(byref magias, who, cfg_path, school_name, cmd_prefix, skill_name)
    var spell_cfg := ReadConfigFile(cfg_path);
    if (!spell_cfg)
        return;
    endif
    
    var skill_value := CInt(AP_GetSkill(who, skill_name));
    var max_circle := skill_value / 20;
    if (max_circle < 1)
        max_circle := 1;
    endif
    
    foreach spell_id in (GetConfigIntKeys(spell_cfg))
        var spell_elem := FindConfigElem(spell_cfg, spell_id);
        if (!spell_elem)
            continue;
        endif
        
        var spell_circle := CInt(GetConfigString(spell_elem, "Circle"));
        if (spell_circle > max_circle)
            continue;
        endif
        
        var magia := struct;
        magia.+spell_id := spell_id;
        magia.+name := GetConfigString(spell_elem, "Name");
        magia.+desc := GetConfigString(spell_elem, "Desc");
        magia.+school := school_name;
        magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
        magia.+circle := spell_circle;
        magia.+command_prefix := cmd_prefix;
        
        // Get icon from icons.cfg
        var icon_struct := GetSpellIcon(magia.name);
        magia.+iconid := icon_struct.iconid;
        magia.+hue := icon_struct.hue;
        
        magias.append(magia);
    endforeach
endfunction

// Load memorized spells (for Priest/Shaman/Druid)
function LoadMemorizedSpells(byref magias, who, list_prop, school_name, cmd_prefix)
    var spell_list := GetObjProperty(who, list_prop);
    if (!spell_list || TypeOf(spell_list) != "Array")
        return;
    endif
    
    if (spell_list.size() < 1)
        return;
    endif
    
    // Determine config file based on school
    var cfg_path;
    if (school_name == "Priest")
        cfg_path := ":spells:config/priestspells";
    elseif (school_name == "Shaman")
        cfg_path := ":spells:config/shamanspells";
    elseif (school_name == "Druid")
        cfg_path := ":spells:config/druidspells";
    else
        return; // Unknown school
    endif
    
    var spell_cfg := ReadConfigFile(cfg_path);
    if (!spell_cfg)
        return;
    endif
    
    foreach spell in spell_list
        if (!spell.name || !spell.id)
            continue;
        endif
        
        // Load full spell data from config to get mana cost
        var spell_elem := FindConfigElem(spell_cfg, spell.id);
        if (!spell_elem)
            continue;
        endif
        
        var magia := struct;
        magia.+spell_id := spell.id;
        magia.+name := spell.name;
        magia.+desc := spell.description; // Use description from memorized list
        magia.+school := school_name;
        magia.+mana_cost := CInt(GetConfigString(spell_elem, "Mana")); // Get from config
        magia.+circle := CInt(spell.circle);
        magia.+command_prefix := cmd_prefix;
        
        // Get icon from icons.cfg
        var icon_struct := GetSpellIcon(magia.name);
        magia.+iconid := icon_struct.iconid;
        magia.+hue := icon_struct.hue;
        
        magias.append(magia);
        sleepms(2);
    endforeach
endfunction
