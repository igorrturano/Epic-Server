// Script-level cached spell category arrays (built once, reused every spell decision)
var _aradalore_spells := 0;
var _anti_mage_spells := 0;
var _anti_melee_spells := 0;
var _aoe_spells := 0;
var _non_ideal_spells := 0;
var _always_good_base := 0;

function GetAradoloreSpells()
   if (_aradalore_spells) return _aradalore_spells; endif
   _aradalore_spells := array{
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
   return _aradalore_spells;
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

function ChooseIdealSpell(me, opponent, spells, school := 0)
    var chosen_spells := array{};
    var opponent_class := opponent.getClass();
    var is_ranged := opponent.weapon.attribute == "RangedCombat";
    var dist := Distance(me, opponent);
    var low_health := AP_GetVital(me, HITS) < (AP_GetVitalMaximumValue(me, HITS) * 0.3);
    var is_drow := (GetObjProperty(opponent, "chardata").raca == "Drow");
    var enemy_count := NPC_GetPlayersPetsAndSummonsNearby(me).size();
    if (!school) school := getSpellSchool(me); endif
    
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
    if (!_always_good_base)
        _always_good_base := array{
            "Misseis Magicos", "Ampliar Dor", "Drenar Folego", "Relampago",
            "Incinerar", "Congelar", "Pesadelo", "Impedir Cura",
            "Forma Amaldicoada", "Drenar Vida", "Maldicao de Zhao",
            "Paralisar", "Envenenar", "Velocidade", "Clones das sombras",
            "Hipnotizar", "Cegar"
        };
    endif

    // Copy base and append conditional entries
    var spells := _always_good_base + array{};

    if (!is_drow)
        spells.append("Escurecer");
    endif

    if (school == "Aradalore")
        spells := spells + GetAradoloreSpells();
    endif

    return spells;
endfunction

function GetAntiMageSpells()
    if (_anti_mage_spells) return _anti_mage_spells; endif
    _anti_mage_spells := array{
        "Enfraquecer Intelecto", "Atrasar Magias", "Fuga de Mana",
        "Drenar Mana", "Envenenar Espirito", "Silenciar"
    };
    return _anti_mage_spells;
endfunction

function GetAntiMeleeSpells()
    if (_anti_melee_spells) return _anti_melee_spells; endif
    _anti_melee_spells := array{
        "Retribuicao Profana", "Muralha de Fogo", "Muralha de Pedra",
        "Correntes de dor", "Medo", "Vinganca Recorrente",
        "Horror Paralisante", "Forma Espectral"
    };
    return _anti_melee_spells;
endfunction

function GetAoESpells()
    if (_aoe_spells) return _aoe_spells; endif
    _aoe_spells := array{
        "Bola de Fogo", "Nuvem Acida", "Corrente de Raios",
        "Chuva de Meteoros", "Terremoto", "Chuva de Gelo",
        "Parar o Tempo"
    };
    return _aoe_spells;
endfunction

function GetNonIdealSpells()
    if (_non_ideal_spells) return _non_ideal_spells; endif
    _non_ideal_spells := array{
        "Sangue em mana", "Satisfacao Sadica", "Presente Espiritual",
        "Invoc. Espir. Luz", "Aprisionar Alma", "Aparencia Cadaverica",
        "Pacto Sepultura", "Remover Encantamentos", "Teleportar",
        "Folego prolongado", "Encantar objeto", "Manipular Sombras",
        "Iludir", "Pressagio", "Prolongar Desmaio", "Levantar os Mortos"
    };
    return _non_ideal_spells;
endfunction

function SortSpellsByPriority(spells)
    // Bubble sort - now uses O(1) cached circle lookups instead of 4-config iteration
    var n := spells.size();
    var sorted := 0;
    while (!sorted)
        sorted := 1;
        for i := 1 to (n-1)
            var a := spells[i];
            var b := spells[i+1];

            var circle_a := GetSpellCircleCached(a.name);
            var circle_b := GetSpellCircleCached(b.name);

            var should_swap := 0;

            if (circle_a == circle_b)
                if (a.weight < b.weight)
                    should_swap := 1;
                endif
            else
                if (circle_a < circle_b)
                    should_swap := 1;
                endif
            endif

            if (should_swap)
                var temp := spells[i];
                spells[i] := spells[i+1];
                spells[i+1] := temp;
                sorted := 0;
            endif
        endfor
    endwhile

    return spells;
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

// O(1) cached lookup - replaces iterating 4 config files
function FindMageSpellInConfigs(spellname)
    return FindSpellInfoCached(spellname);
endfunction

// Kept for backward compatibility (shaman/priest code calls this directly with a config path)
// Now uses cached data instead of iterating config entries
function FindSpellInConfig(spellname, cfgFile)
    return FindSpellInfoCached(spellname);
endfunction

// O(1) cached lookup - replaces iterating 4 config files with sleepms(2) per entry
function getAvailableSpells(monster, school := 0)
   var cache := NPC_GetCachedData(monster);
   var tier := cache.tier;
   if (!tier) tier := 1; endif
   if (!school) school := getSpellSchool(monster); endif
   return GetAvailableSpellsCached(school, tier);
endfunction

function getSpellSchool(monster)
   var cache := NPC_GetCachedData(monster);  // O(1) lookup
   var school := cache.spell_school;

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

function ChooseSpell(me, opponent, spells, school := 0)
    if (!school) school := getSpellSchool(me); endif

    if (school == "Ahtaleloreth")
        var name := HandleSwordMageSpellChoice(opponent);
        return struct{ "name" := name, "info" := FindMageSpellInConfigs(name) };
    endif

    var chosen_spell := ChooseIdealSpell(me, opponent, spells, school);
    var spellInfo := FindMageSpellInConfigs(chosen_spell);
    if (!spellInfo)
        spell_target_var := opponent.serial;
        chosen_spell := "Misseis Magicos";
        spellInfo := FindMageSpellInConfigs(chosen_spell);
        return struct{ "name" := chosen_spell, "info" := spellInfo };
    endif

    // Set appropriate target based on spell type
    if (spellInfo.Target in array{"Unico", "Area"})
        spell_target_var := opponent.serial;
    elseif (spellInfo.Target == "Self")
        spell_target_var := me.serial;
    endif

    return struct{ "name" := chosen_spell, "info" := spellInfo };
endfunction

function DoHarmfulSpell()
    var diff := getNpcResist();
    if (GetCooldown(me, "DoHarmfulSpell"))
        return 0;
    endif
    
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
   endforeach

   SetCooldown(me, "DoHarmfulSpell", 15);
   return 0;
endfunction

function MageBehavior(me, opponent)
   if (GetCooldown(me, "spell_cast"))
      return 0;
   endif

   if (GetObjProperty(me, "metamorfose"))
      return 0;
   endif

   // Compute school ONCE, pass through the entire chain
   var school := getSpellSchool(me);

   var spells := getAvailableSpells(me, school);
   if (!spells.size())
      return 0;
   endif

   var result := ChooseSpell(me, opponent, spells, school);
   if (!result)
      return 0;
   endif

   PrintTextAbove(me, "Casting " + result.name);
   return NPC_CastSpell(me, result.name, result.info);
endfunction

function NPC_CastSpell(me, spellname, spellInfo := 0)
   if (!spellInfo)
      spellInfo := FindMageSpellInConfigs(spellname);
   endif

   if (!spellInfo)
      return 0;
   endif

   var targ := SystemFindObjectBySerial(spell_target_var);
   spell_target_var := 0;

   var params := array{me, spellInfo, 0, targ};
   var script := Start_Script(":spells:cast_spell", params);
   if (script.errortext)
      PrintTextAbove(me, "Error starting spell: " + script.errortext);
      return 0;
   endif

   SetCooldown(me, "spell_cast", 5);
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
            spell_target_var := me.weapon.serial;
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
            spell_target_var := opponent.serial;
            return spell;
        endif
    endif
    
    // Default return if we can't cast yet due to cooldown
    return "Misseis Magicos";
endfunction

// New functions for humanoid shaman mage behavior

// O(1) cached lookup - replaces iterating shaman config with sleepms(2) per entry
function GetAvailableShamanSpells(monster)
    var cache := NPC_GetCachedData(monster);
    var tier := cache.tier;
    if (!tier) tier := 1; endif
    return GetAvailableShamanSpellsCached(tier);
endfunction

function NPC_CastShamanSpell(me, spellname, spellInfo := 0)
    if (!spellInfo)
        spellInfo := FindSpellInConfig(spellname, ":spells:config/shaman");
    endif
    if (!spellInfo)
        return 0;
    endif

    var targ := SystemFindObjectBySerial(spell_target_var);
    spell_target_var := 0;

    var params := array{me, spellInfo, 0, targ};
    var script := Start_Script(":spells:cast_spell", params);
    if (script.errortext)
        PrintTextAbove(me, "Error starting shaman spell: " + script.errortext);
        return 0;
    endif

    SetCooldown(me, "spell_cast", 10);
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

    var result := ChooseShamanSpell(me, opponent, spells);
    if (!result)
        return 0;
    endif

    PrintTextAbove(me, "Casting " + result.name + " (Shaman)");
    return NPC_CastShamanSpell(me, result.name, result.info);
endfunction

function ChooseShamanSpell(me, opponent, spells)
    var chosen_spell := ChooseIdealShamanSpells(me, opponent, spells);
    var spellInfo := FindSpellInConfig(chosen_spell, ":spells:config/shaman");
    if (!spellInfo)
        spell_target_var := opponent.serial;
        chosen_spell := "Enfraquecer";
        spellInfo := FindSpellInConfig(chosen_spell, ":spells:config/shaman");
        return struct{ "name" := chosen_spell, "info" := spellInfo };
    endif

    // Set target based on spell type
    if (spellInfo.Target in array{"Unico", "Area"})
        spell_target_var := opponent.serial;
    elseif (spellInfo.Target == "Self")
        spell_target_var := me.serial;
    endif

    return struct{ "name" := chosen_spell, "info" := spellInfo };
endfunction

function ChooseIdealShamanSpells(me, opponent, spells)
    // Gather battlefield information
    var opponent_class := opponent.getClass();
    var is_ranged := (opponent.weapon.attribute == "RangedCombat");
    var dist := Distance(me, opponent);
    var low_health := AP_GetVital(me, HITS) < (AP_GetVitalMaximumValue(me, HITS) * 0.3);
    var hurt := AP_GetVital(me, HITS) < (AP_GetVitalMaximumValue(me, HITS) * 0.7);
    var enemy_count := NPC_GetPlayersPetsAndSummonsNearby(me).size();
    var allies := NPC_GetNearbyAllies(me);
    var ally_count := allies.size();
    var self_cache := NPC_GetCachedData(me);
    var self_is_undead := (self_cache.category_lower == "undead" || self_cache.category_lower == "demons");

    // Emergency priority: self-heal (skip if undead — healing damages them)
    if (!self_is_undead)
        if (low_health && ("Cura Grave" in spells))
            return "Cura Grave";
        endif
        if (hurt && ("Cura Leve" in spells))
            return "Cura Leve";
        endif
    endif

    // If the opponent has summoned creatures, try "Enraizar" if available.
    if (NPC_HasSummonedCreaturesNearby(opponent) && ("Enraizar" in spells))
        return "Enraizar";
    endif

    // Build a weighted list from the available shaman spells (only those in shaman.cfg).
    var valid_spells := array{};
    foreach spell in spells
        var weight := 50; // base weight for every available spell

        // Example condition: if opponent is a warrior and the spell is "Enfraquecer",
        // add bonus weight.
        if (opponent_class == GUERREIRO && spell == "Enfraquecer")
            weight += 20;
        endif

        // AoE heal boost only if caster isn't undead (healing damages undead)
        if (!self_is_undead)
            foreach ally in allies
                if (AP_GetVital(ally, HITS) < (AP_GetVitalMaximumValue(ally, HITS) * 0.7) && spell == "Cura em Area")
                    weight += 15;
                endif
            endforeach
        endif

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

// O(1) cached lookup - replaces iterating priest config with sleepms(2) per entry
function GetAvailablePriestSpells(monster)
    var cache := NPC_GetCachedData(monster);
    var tier := cache.tier;
    if (!tier) tier := 1; endif
    return GetAvailablePriestSpellsCached(tier);
endfunction

function HumanoidPriestBehavior(me, opponent)
    if (GetCooldown(me, "spell_cast"))
        return 0;
    endif

    // Get available priest spells based on the caster's tier
    var spells := GetAvailablePriestSpells(me);
    if (!spells.size())
        return 0;
    endif

    var chosen_spell := ChoosePriestSpell(me, opponent, spells);
    if (!chosen_spell)
        return 0;
    endif

    var spellInfo := FindSpellInConfig(chosen_spell, ":spells:config/priest");

    // Set target if not already set by ChoosePriestSpell (ally healing sets it there)
    if (!spell_target_var)
        if (spellInfo && spellInfo.Target == "Self")
            spell_target_var := me.serial;
        else
            spell_target_var := opponent.serial;
        endif
    endif

    PrintTextAbove(me, "Casting " + chosen_spell + " (Priest)");
    return NPC_CastPriestSpell(me, chosen_spell, spellInfo);
endfunction

function ChoosePriestSpell(me, opponent, spells)
    // Gather battlefield information
    var my_health_pct := CDbl(AP_GetVital(me, HITS)) / CDbl(AP_GetVitalMaximumValue(me, HITS));
    var opponent_health_pct := CDbl(AP_GetVital(opponent, HITS)) / CDbl(AP_GetVitalMaximumValue(opponent, HITS));
    var self_cache := NPC_GetCachedData(me);
    var self_is_undead := (self_cache.category_lower == "undead" || self_cache.category_lower == "demons");

    // Phase 4: Use cached scan instead of direct ListMobilesNearLocationEx
    var allies := GetNearbyMobilesOnce(me, 8, LISTEX_FLAG_NORMAL);
    var hurt_allies := 0;

    // Count hurt allies (skip undead/demons — healing damages them)
    foreach ally in allies
        if (!ally.npctemplate)
            continue;
        endif

        if (ally.serial != me.serial && !ally.dead)
            var ally_cache := NPC_GetCachedData(ally);
            if (ally_cache.category_lower != "undead" && ally_cache.category_lower != "demons")
                var ally_health_pct := CDbl(AP_GetVital(ally, HITS)) / CDbl(AP_GetVitalMaximumValue(ally, HITS));
                if (ally_health_pct < 0.7)
                    hurt_allies += 1;
                endif
            endif
        endif
    endforeach

    // Emergency priority spells (skip if undead — healing damages them)
    if (!self_is_undead && my_health_pct < 0.3)
        if ("Santuario" in spells)
            return "Santuario";
        elseif ("Cura Grave" in spells)
            return "Cura Grave";
        elseif ("Cura Leve" in spells)
            return "Cura Leve";
        endif
    endif

    // Healing priority for allies
    if (hurt_allies > 1 && "Descanso Tranquilo" in spells)
        return "Descanso Tranquilo";
    endif

    // Single target healing for very hurt allies (undead already filtered above)
    foreach ally in allies
        if (!ally.npctemplate && !ally.dead)
            var ally_cache := NPC_GetCachedData(ally);
            if (ally_cache.category_lower != "undead" && ally_cache.category_lower != "demons")
                var ally_health_pct := CDbl(AP_GetVital(ally, HITS)) / CDbl(AP_GetVitalMaximumValue(ally, HITS));
                if (ally_health_pct < 0.4)
                    if ("Cura Grave" in spells)
                        spell_target_var := ally.serial;
                        return "Cura Grave";
                    elseif ("Cura Leve" in spells)
                        spell_target_var := ally.serial;
                        return "Cura Leve";
                    endif
                endif
            endif
        endif
    endforeach

    // Defensive spells when under attack
    if (me.opponent)
        if ("Protecao" in spells && !GetCooldown(me, "protection_spell"))
            SetCooldown(me, "protection_spell", 30);
            return "Protecao";
        elseif ("Aura Mortis" in spells && !GetCooldown(me, "aura_mortis"))
            SetCooldown(me, "aura_mortis", 45);
            return "Aura Mortis";
        endif
    endif

    // Offensive support against strong opponents
    if (opponent_health_pct > 0.7)
        if ("Destruir o Mal" in spells && !GetCooldown(me, "destroy_evil"))
            SetCooldown(me, "destroy_evil", 60);
            return "Destruir o Mal";
        endif
    endif

    // Default to basic healing if nothing else is appropriate
    return "Cura Leve";
endfunction

function NPC_CastPriestSpell(me, spellname, spellInfo := 0)
    if (!spellInfo)
        spellInfo := FindSpellInConfig(spellname, ":spells:config/priest");
    endif

    if (!spellInfo)
        return 0;
    endif
    
    // Start the cast_spell script with target info
    var targ := SystemFindObjectBySerial(spell_target_var);
    spell_target_var := 0;
    
    var params := array{me, spellInfo, 0, targ};
    var script := Start_Script(":spells:cast_spell", params);
    if (script.errortext)
        PrintTextAbove(me, "Error starting priest spell: " + script.errortext);
        return 0;
    endif
    
    SetCooldown(me, "spell_cast", 10);
    return 1;
endfunction