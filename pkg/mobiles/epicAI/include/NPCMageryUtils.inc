function GetAradoloreSpells()
   // All spells that belong to the Aradalore school
   // Important: Cross reference with spellcaster.cfg to match names exactly
   return array{
     "Invoc. Espir. Urso",
     "Invoc. Espir. Lobo",
     "Invoc. Espir. Serpente", 
     "Invoc. Espir. Pantera",
     "Invoc. Espir. Cura",
     "Invoc. Driade",
     "Invoc. Ent",
     "Forma de Urso", 
     "Forma Grande Urso",
     "Forma do Lobo",
     "Forma da Serpente",
     "Forma da Pantera",
     "Forma Licantropa",
     "Invoc. Espir. Fogo",
     "Invoc. Colosso"
   };
endfunction

function RemoveNonIdealSpells(spells, school)
    var non_ideal := GetNonIdealSpells();
    var valid_spells := array{};
    
    foreach spell in spells
        if (!(spell.name in non_ideal))
            valid_spells.append(spell);
        endif
    endforeach
    
    return valid_spells;
endfunction

function ChooseIdealSpell(me, opponent, spells)
    var chosen_spells := array{};
    var opponent_class := opponent.getClass();
    var is_ranged := opponent.weapon.attribute == "RangedCombat";
    var dist := Distance(me, opponent);
    var low_health := AP_GetVital(me, HITS) < (AP_GetVitalMaximumValue(me, HITS) * 0.3);
    var is_drow := (GetObjProperty(opponent, "chardata").raca == "Drow");
    var enemy_count := ListHostiles(me, 10).size();
    var school := getSpellSchool(me);
    
    // First priority - emergency spells
    if (low_health && "Invisibilidade" in spells)
        return "Invisibilidade";
    endif
    
    // Check for summons first
    if (NPC_HasSummonedCreaturesNearby(opponent) && "Banimento Arcano" in spells)
        return "Banimento Arcano";
    endif

    // Check for spell combos first
    if (GetObjProperty(opponent, "sleep") && "Pesadelo" in spells)
        return "Pesadelo";
    endif
    
    var valid_spells := array{};
    
    // Always good spells get high priority
    foreach spell in spells
        if (spell in GetAlwaysGoodSpells(school, is_drow))
            valid_spells.append(struct{ "name" := spell, "weight" := 80 });
        endif
    endforeach
    
    // Class-specific spells
    case (opponent_class)
        GUERREIRO: 
            if ("Enfraquecer" in spells)
                valid_spells.append(struct{ "name" := "Enfraquecer", "weight" := 70 });
            endif
        LADINO:
            if ("Atrapalhar" in spells)
                valid_spells.append(struct{ "name" := "Atrapalhar", "weight" := 70 });
            endif
        SABIO:
            foreach spell in GetAntiMageSpells()
                if (spell in spells)
                    // Extra weight for Silenciar
                    var weight := (spell == "Silenciar") ? 90 : 70;
                    valid_spells.append(struct{ "name" := spell, "weight" := weight });
                endif
            endforeach
    endcase
    
    // Anti-melee spells
    if (!is_ranged)
        foreach spell in GetAntiMeleeSpells()
            if (spell in spells)
                valid_spells.append(struct{ "name" := spell, "weight" := 60 });
            endif
        endforeach
    endif
    
    // AoE spells if multiple enemies
    if (enemy_count > 2)
        foreach spell in GetAoESpells()
            if (spell in spells)
                valid_spells.append(struct{ "name" := spell, "weight" := 75 });
            endif
        endforeach
    endif
    
    // Close range spells
    if (dist <= 3 && "Maos Flamejantes" in spells)
        valid_spells.append(struct{ "name" := "Maos Flamejantes", "weight" := 65 });
    endif
    
    // Remove non-ideal spells
    valid_spells := RemoveNonIdealSpells(valid_spells, school);
    
    // Sort by circle and weight
    valid_spells := SortSpellsByPriority(valid_spells);
    
    if (valid_spells.size() > 0)
        return ChooseWeightedSpell(valid_spells);
    endif
    
    return "Misseis Magicos"; // Default fallback
endfunction

function GetAlwaysGoodSpells(school, is_drow)
    var spells := array{
        "Misseis Magicos", "Ampliar Dor", "Drenar Folego", "Relampago",
        "Incinerar", "Congelar", "Pesadelo", "Impedir Cura",
        "Forma Amaldicoada", "Drenar Vida", "Maldicao de Zhao",
        "Paralisar", "Envenenar", "Velocidade", "Clones das sombras",
        "Hipnotizar", "Cegar"
    };
    
    // Add Escurecer only against non-drow
    if (!is_drow)
        spells.append("Escurecer");
    endif
    
    // Add all Aradalore spells if that's the school

    if (school == "Aradalore")
        spells := spells + GetAradoloreSpells();
    endif
    
    return spells;
endfunction

function GetAntiMageSpells()
    return array{
        "Enfraquecer Intelecto", "Atrasar Magias", "Fuga de Mana",
        "Drenar Mana", "Envenenar Espirito", "Silenciar"
    };
endfunction

function GetAntiMeleeSpells()
    return array{
        "Retribuicao Profana", "Muralha de Fogo", "Muralha de Pedra",
        "Correntes de dor", "Medo", "Vinganca Recorrente",
        "Horror Paralisante", "Forma Espectral"
    };
endfunction

function GetAoESpells()
    return array{
        "Bola de Fogo", "Nuvem Acida", "Corrente de Raios",
        "Chuva de Meteoros", "Terremoto", "Chuva de Gelo",
        "Parar o Tempo"
    };
endfunction

function GetNonIdealSpells()
    return array{
        "Sangue em mana", "Satisfacao Sadica", "Presente Espiritual",
        "Invoc. Espir. Luz", "Aprisionar Alma", "Aparencia Cadaverica",
        "Pacto Sepultura", "Remover Encantamentos", "Teleportar",
        "Folego prolongado", "Encantar objeto", "Manipular Sombras",
        "Iludir", "Pressagio"
    };
endfunction

function SortSpellsByPriority(spells)
    // Bubble sort implementation for POL
    var n := spells.size();
    var sorted := 0;
    while (!sorted)
        sorted := 1;
        for i := 1 to (n-1)
            var a := spells[i];
            var b := spells[i+1];
            
            var circle_a := GetSpellCircle(a.name);
            var circle_b := GetSpellCircle(b.name);
            
            var should_swap := 0;
            
            if (circle_a == circle_b)
                // If same circle, sort by weight
                if (a.weight < b.weight)
                    should_swap := 1;
                endif
            else
                // Sort by circle (higher circle first)
                if (circle_a < circle_b)
                    should_swap := 1;
                endif
            endif
            
            if (should_swap)
                // Swap elements
                var temp := spells[i];
                spells[i] := spells[i+1];
                spells[i+1] := temp;
                sorted := 0;
            endif
        endfor
    endwhile
  
    return spells;
endfunction

function GetSpellCircle(spellname)
    // Need access to spell config to get circle
    var cfg := ReadConfigFile(":spells:config/spellcaster");
    foreach spellId in (GetConfigIntKeys(cfg))
        var elem := FindConfigElem(cfg, spellId);
        if (Lower(elem.Name) == Lower(spellname))
            return CInt(elem.Circle);
        endif
        sleepms(2);
    endforeach
    
    return 1; // Default to circle 1 if not found
endfunction

function ChooseWeightedSpell(valid_spells)
    var total_weight := 0;
    foreach spell in valid_spells
        total_weight += spell.weight;
    endforeach
    
    var roll := RandomInt(total_weight);
    var current_weight := 0;
    
    foreach spell in valid_spells
        current_weight += spell.weight;
        if (roll <= current_weight)
            return spell.name;
        endif
    endforeach
    
    return valid_spells[1].name; // Fallback to highest priority spell
endfunction

function FindSpellInConfig( spellname, cfgFile )
    var cfg := ReadConfigFile( cfgFile );

        foreach spellId in ( GetConfigIntKeys( cfg ) )
            var elem := FindConfigElem( cfg, spellId );

            if ( Lower( GetConfigString( elem, "Name" ) ) == Lower( spellname ) )
                var dict := dictionary{ 
                "id" -> spellId,
                "circle" -> GetConfigInt( elem,"Circle" ),
                "name" -> GetConfigString( elem, "Name" ),
                "Difficulty" -> GetConfigInt( elem, "Difficulty" ),
                "School" -> GetConfigString( elem, "School" ),
                "Target" -> GetConfigString( elem, "Target" ),
                "script" -> GetConfigString( elem, "SpellScript" ),
                "Cycles" -> GetConfigInt( elem, "Cycles" ),
                "PowerWords" -> GetConfigString( elem, "PowerWords" ),
                "mana" -> GetConfigInt( elem, "Mana" )
                };
                return dict;
            endif
        endforeach

    return 0;
endfunction

function getAvailableSpells(monster)
   var npc_elem := NPC_GetNPCConfig(monster);
   var tier := CInt(npc_elem.Tier);
   if (!tier) tier := 1; endif
   
   var spells := array{"Misseis Magicos"}; // All casters have magic missile
   
   // Get spellcaster config
   var cfg := ReadConfigFile(":spells:config/spellcaster");
   var school := getSpellSchool(monster);
   
   // Add spells from school up to tier
   foreach spellId in (GetConfigIntKeys(cfg))
      var elem := FindConfigElem(cfg, spellId);
      if (elem.School == school && elem.Circle <= tier)
         spells.append(elem.Name);
      endif
      sleepms(2);
   endforeach
   return spells;
endfunction

function getSpellSchool(monster)
   var npc_elem := NPC_GetNPCConfig(monster);
   var school := npc_elem.SpellSchool;

   var spellSchool := GetObjProperty(monster, "SpellSchool");
    if (spellSchool)
        school := spellSchool;
    endif
   if (!school)
        var schools := array{"Evocatium", "Aradalore", "Ahtaleloreth", "Kaijin", "Velkyn"};
        school := schools[RandomInt(5)+1];
    endif

   return school;
endfunction

function ChooseSpell(me, opponent, spells)
    var cfgFile := ":spells:config/spellcaster";
    var school := getSpellSchool(me);

    if (school == "Ahtaleloreth")
        return HandleSwordMageSpellChoice(opponent);
    endif

    // Get ideal spell based on tactical considerations
    var chosen_spell := ChooseIdealSpell(me, opponent, spells);
    // Get spell info for the chosen spell
    var spellInfo := FindSpellInConfig(chosen_spell, cfgFile);
    if (!spellInfo)
        SetObjProperty(me, "#SpellTarget", opponent.serial);
        return "Misseis Magicos";
    endif

    // Set appropriate target based on spell type
    if (spellInfo.Target in array{"Unico", "Area"})
        SetObjProperty(me, "#SpellTarget", opponent.serial);
    elseif (spellInfo.Target == "Self")
        SetObjProperty(me, "#SpellTarget", me.serial);
    endif

    // Special targeting for corpse-related spells
    if (chosen_spell in array{"Prolongar Desmaio", "Levantar os Mortos"})
        var nearby_corpses := ListItemsNearLocation(me.x, me.y, me.z, 12);
        foreach item in nearby_corpses
            if (item.IsA(POLCLASS_CORPSE))
                SetObjProperty(me, "#SpellTarget", item.serial);
                break;
            endif
            sleepms(2);
        endforeach
    endif

    return chosen_spell;
endfunction

function DoHarmfulSpell()
    var diff := getNpcResist();
   // If me is being attacked, prioritize Enraizar
   if (me.opponent)
      if (!GetCooldown(me.opponent, "enraizar_cooldown"))
         CastEnraizar(me, me.opponent, diff);
         return 1;
      endif
   endif
   
   // Otherwise, look for enemies attacking allies to weaken
   var allies := NPC_GetNearbyAllies(me);
   foreach ally in allies
      if (ally.opponent)
         if (!GetCooldown(ally.opponent, "weaken_cooldown"))
            CastWeaken(me, ally.opponent, diff);
            return 1;
         endif
      endif
      sleepms(2);
   endforeach

   return 0;
endfunction

function MageBehavior(me, opponent)
   if (GetCooldown(me, "spell_cast"))
      return 0;
   endif

   // Get available spells
   var spells := getAvailableSpells(me);
   if (!spells.size())
      return 0;
   endif

   // Choose spell based on situation
   var chosen_spell := ChooseSpell(me, opponent, spells);

   if (!chosen_spell)
      return 0;
   endif

   PrintTextAbove(me, "Casting " + chosen_spell);
   return NPC_CastSpell(me, chosen_spell);
endfunction

function NPC_CastSpell(me, spellname)
   var cfgFile := ":spells:config/spellcaster";
   var spellInfo := FindSpellInConfig(spellname, cfgFile);
   
   if (!spellInfo)
      return 0;
   endif
   
   // Start the cast_spell script with target info
   var targ := SystemFindObjectBySerial(GetObjProperty(me, "#SpellTarget"));
   EraseObjProperty(me, "#SpellTarget");
   
   var params := array{me, spellInfo, 0, targ};
   var script := Start_Script(":spells:cast_spell", params);
   if (script.errortext)
      PrintTextAbove(me, "Error starting spell: " + script.errortext);
      return 0;
   endif
   
   SetCooldown(me, "spell_cast", 15);
   return 1;
endfunction

function ChooseFireSpell()
    var spells := array{
        "Ataque elem. Fogo",
        "Circ. Devast. Fogo", 
        "Pandemonio Infernal",
        "Chamas Perpetuas",
        "Sacrificio Flamejante"
    };
    return spells[RandomInt(spells.size())+1];
endfunction

function ChooseEnergySpell()
    var spells := array{
        "Ataque elem. Energia",
        "Circ. Devast. Energia",
        "Canalizar Tempestade", 
        "Campo Eletrico",
        "Ataque Relampago"
    };
    return spells[RandomInt(spells.size())+1];
endfunction

function ChooseColdSpell() 
    var spells := array{
        "Ataque elem. Gelo",
        "Circ. Devast. Gelo",
        "Furia Glacial",
        "Ataque Congelante",
        "Prisao Gelida"
    };
    return spells[RandomInt(spells.size())+1];
endfunction

function HandleSwordMageSpellChoice(opponent)
    if (!GetCooldown(me, "elemental_weapon"))
        // First ensure weapon is enchanted
        var elementalWeapon := GetObjProperty(me.weapon, "#elementalweapon");
        if (!elementalWeapon)
            var spells := array{"Lamina Flamejante", "Lamina de Gelo", "Lamina Eletrica"};
            var chosen_spell := spells[RandomInt(spells.size())+1];
            SetObjProperty(me, "#SpellTarget", me.weapon.serial);
            return chosen_spell;
        endif

        // Choose spells based on enchantment type
        var spell := "";
        case (elementalWeapon.type)
            DMG_FIRE:
                spell := ChooseFireSpell();
            DMG_ENERGY:
                spell := ChooseEnergySpell();  
            DMG_COLD:
                spell := ChooseColdSpell();
        endcase

        if (spell)
            SetObjProperty(me, "#SpellTarget", opponent.serial);
            return spell;
        endif
    endif
    
    // Default return if we can't cast yet due to cooldown
    return "Misseis Magicos";
endfunction

// New functions for humanoid shaman mage behavior

function GetAvailableShamanSpells(monster)
    var npc_elem := NPC_GetNPCConfig(monster);
    var tier := CInt(npc_elem.Tier);
    if (!tier) tier := 1; endif;

    // All shamans have a basic fallback spell
    var spells := array{"Enfraquecer"};

    // Read shaman spells configuration file
    var cfg := ReadConfigFile(":spells:config/shamanspells");
    foreach spellId in GetConfigIntKeys(cfg)
        var elem := FindConfigElem(cfg, spellId);
        // Use the spell's Circle value to determine if it is available at the current tier
        // TODO: Ver se vale a pena deixar a 0, e colocar um CD menor, depois que fazer o Ideal Spell funcionar pra cada magia shaman
        if (CInt(elem.Circle) <= tier && Cint(elem.Circle) > 0)
            spells.append(elem.Name);
        endif
        sleepms(2);
    endforeach
    return spells;
endfunction

function NPC_CastShamanSpell(me, spellname)
    var cfgFile := ":spells:config/shamanspells";
    var spellInfo := FindSpellInConfig(spellname, cfgFile);
    if (!spellInfo)
        return 0;
    endif

    // Look up the target as set by the behavior (or default to opponent)
    var targ := SystemFindObjectBySerial(GetObjProperty(me, "#SpellTarget"));
    EraseObjProperty(me, "#SpellTarget");

    var params := array{me, spellInfo, 0, targ};
    var script := Start_Script(":spells:cast_spell", params);
    if (script.errortext)
        PrintTextAbove(me, "Error starting shaman spell: " + script.errortext);
        return 0;
    endif

    SetCooldown(me, "spell_cast", 15);
    return 1;
endfunction

function HumanoidShamanBehavior(me, opponent)
    if (GetCooldown(me, "spell_cast"))
        return 0;
    endif

    // Get castable shaman spells based on the caster's tier
    var spells := GetAvailableShamanSpells(me);
    if (!spells.size())
        return 0;
    endif

    // Choose a spell based on current conditions.
    // (Here we re-use the generic ChooseSpell function for simplicity.
    // Later you could add additional conditions to favor buffs versus harmful spells.)
    var chosen_spell := ChooseShamanSpell(me, opponent, spells);
    if (!chosen_spell)
        return 0;
    endif

    // Retrieve the chosen spell's configuration so that we can set the proper target.
    var cfgFile := ":spells:config/shamanspells";
    var spellInfo := FindSpellInConfig(chosen_spell, cfgFile);
    if (!spellInfo) 
        // Default to opponent if no spell info is found
        SetObjProperty(me, "#SpellTarget", opponent.serial);
    else
        // Target selection: For spells marked "Unico" or "Area" use opponent; for "Self" use self.
        if (spellInfo.Target in array{"Unico", "Area"})
            SetObjProperty(me, "#SpellTarget", opponent.serial);
        elseif (spellInfo.Target == "Self")
            SetObjProperty(me, "#SpellTarget", me.serial);
        endif
    endif

    PrintTextAbove(me, "Casting " + chosen_spell + " (Shaman)");
    return NPC_CastShamanSpell(me, chosen_spell);
endfunction

function ChooseShamanSpell(me, opponent, spells)
    var cfgFile := ":spells:config/shamanspells";

    // Get ideal spell based on tactical considerations
    var chosen_spell := ChooseIdealShamanSpells(me, opponent, spells);
    // Get spell info for the chosen spell
    var spellInfo := FindSpellInConfig(chosen_spell, cfgFile);
    if (!spellInfo)
        SetObjProperty(me, "#SpellTarget", opponent.serial);
        return "Enfraquecer";
    endif

    // Set appropriate target based on spell type
    if (spellInfo.Target in array{"Unico", "Area"})
        SetObjProperty(me, "#SpellTarget", opponent.serial);
    elseif (spellInfo.Target == "Self")
        SetObjProperty(me, "#SpellTarget", me.serial);
    endif

    return chosen_spell;
endfunction

function ChooseIdealShamanSpells(me, opponent, spells)
    // Gather battlefield information
    var opponent_class := opponent.getClass();
    var is_ranged := (opponent.weapon.attribute == "RangedCombat");
    var dist := Distance(me, opponent);
    var low_health := AP_GetVital(me, HITS) < (AP_GetVitalMaximumValue(me, HITS) * 0.3);
    var hurt := AP_GetVital(me, HITS) < (AP_GetVitalMaximumValue(me, HITS) * 0.7);
    var enemy_count := ListHostiles(me, 10).size();
    var allies := NPC_GetNearbyAllies(me);
    var ally_count := allies.size();
    // Emergency priority:
    // If low on health, try "Cura Grave" if available.
    if (low_health && ("Cura Grave" in spells))
        return "Cura Grave";
    endif

    if (hurt && ("Cura Leve" in spells))
        return "Cura Leve";
    endif

    // If the opponent has summoned creatures, try "Enraizar" if available.
    if (NPC_HasSummonedCreaturesNearby(opponent) && ("Enraizar" in spells))
        return "Enraizar";
    endif

    // Build a weighted list from the available shaman spells (only those in shamanspells.cfg).
    var valid_spells := array{};
    foreach spell in spells
        var weight := 50; // base weight for every available spell

        // Example condition: if opponent is a warrior and the spell is "Enfraquecer",
        // add bonus weight.
        if (opponent_class == GUERREIRO && spell == "Enfraquecer")
            weight += 20;
        endif

        // Example condition: if many enemies are present, and the spell is an AoE like "Cura em Area",
        // add extra weight.
        // Also check for hurt allies
        foreach ally in allies
            if (AP_GetVital(ally, HITS) < (AP_GetVitalMaximumValue(ally, HITS) * 0.7) && spell == "Cura em Area")
                weight += 15;
            endif
        endforeach

        // Add the spell structure into our weighted list.
        valid_spells.append(struct{ "name" := spell, "weight" := weight });
    endforeach

    // Sort the spells by priority (this function can take into account spell circles as well if applicable).
    valid_spells := SortSpellsByPriority(valid_spells);

    // Choose one spell by weighted selection.
    if (valid_spells.size() > 0)
        return ChooseWeightedSpell(valid_spells);
    endif

    // If no appropriate spell is found, do not cast any spell.
    return "Enfraquecer";
endfunction