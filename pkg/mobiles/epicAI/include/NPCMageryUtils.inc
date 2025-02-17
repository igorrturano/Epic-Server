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
                "script" -> GetConfigString( elem, "SpellScript" )
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