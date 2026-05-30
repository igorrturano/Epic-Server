// Script-level cached spell lists (lazy init, built once per brain lifetime)
var _cached_anti_mage := 0;
var _cached_anti_melee := 0;
var _cached_aoe := 0;
var _cached_non_ideal := 0;
var _cached_always_good_base := 0;



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
    var opponent_class := opponent.getClass();
    var is_ranged := opponent.weapon.attribute == "RangedCombat";
    var dist := Distance(me, opponent);
    var low_health := AP_GetVital(me, HITS) < (AP_GetVitalMaximumValue(me, HITS) * 0.3);
    var enemy_count := NPC_GetPlayersPetsAndSummonsNearby(me).size();
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
        if (spell in GetAlwaysGoodSpells())
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

function GetAlwaysGoodSpells()
    if (!_cached_always_good_base)
        _cached_always_good_base := array{
            "Misseis Magicos", "Ampliar Dor", "Drenar Folego", "Relampago",
            "Incinerar", "Congelar", "Pesadelo", "Impedir Cura",
            "Forma Amaldicoada", "Drenar Vida", "Maldicao de Zhao",
            "Paralisar", "Envenenar", "Velocidade", "Clones das sombras",
            "Hipnotizar", "Cegar", "Escurecer"
        };
    endif
    return _cached_always_good_base;
endfunction

function GetAntiMageSpells()
    if (!_cached_anti_mage)
        _cached_anti_mage := array{
            "Enfraquecer Intelecto", "Atrasar Magias", "Fuga de Mana",
            "Drenar Mana", "Envenenar Espirito", "Silenciar"
        };
    endif
    return _cached_anti_mage;
endfunction

function GetAntiMeleeSpells()
    if (!_cached_anti_melee)
        _cached_anti_melee := array{
            "Retribuicao Profana", "Muralha de Fogo", "Muralha de Pedra",
            "Correntes de dor", "Medo", "Vinganca Recorrente",
            "Horror Paralisante", "Forma Espectral"
        };
    endif
    return _cached_anti_melee;
endfunction

function GetAoESpells()
    if (!_cached_aoe)
        _cached_aoe := array{
            "Bola de Fogo", "Nuvem Acida", "Corrente de Raios",
            "Chuva de Meteoros", "Terremoto", "Chuva de Gelo",
            "Parar o Tempo"
        };
    endif
    return _cached_aoe;
endfunction

function GetNonIdealSpells()
    if (!_cached_non_ideal)
        _cached_non_ideal := array{
            "Sangue em mana", "Satisfacao Sadica", "Presente Espiritual",
            "Invoc. Espir. Luz", "Aprisionar Alma", "Aparencia Cadaverica",
            "Pacto Sepultura", "Remover Encantamentos", "Teleportar",
            "Folego prolongado", "Encantar objeto", "Manipular Sombras",
            "Iludir", "Pressagio"
        };
    endif
    return _cached_non_ideal;
endfunction

// Add circle bonus to weight and do direct weighted random — O(n) replaces O(n²) bubble sort
function SortSpellsByPriority(spells)
    foreach spell in spells
        var circle := GetSpellCircleCached(spell.name);
        spell.weight += circle * 10;
    endforeach
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
        var schools := array{"Evocatium", "Ahtaleloreth", "Kaijin", "Velkyn"};
        school := schools[RandomInt(4)+1];
    endif

   return school;
endfunction

function ChooseSpell(me, opponent, spells, school := 0)
    if (!school) school := getSpellSchool(me); endif

    if (school == "Ahtaleloreth")
        var name := HandleSwordMageSpellChoice(opponent);
        return name;
    endif

    // Get ideal spell based on tactical considerations
    var chosen_spell := ChooseIdealSpell(me, opponent, spells);
    // Get spell info for the chosen spell - search across all mage school configs
    var spellInfo := FindSpellInfoCached(chosen_spell);
    if (!spellInfo)
        spell_target_var := opponent.serial;
        return "Misseis Magicos";
    endif

    // Set appropriate target based on spell type
    if (spellInfo.Target in array{"Unico", "Area"})
        spell_target_var := opponent.serial;
    elseif (spellInfo.Target == "Self")
        spell_target_var := me.serial;
    endif

    // Special targeting for corpse-related spells
    if (chosen_spell in array{"Prolongar Desmaio", "Levantar os Mortos"})
        var nearby_corpses := ListItemsNearLocation(me.x, me.y, me.z, 12);
        foreach item in nearby_corpses
            if (item.IsA(POLCLASS_CORPSE))
                spell_target_var := item.serial;
                break;
            endif
            sleepms(2);
        endforeach
    endif

    return chosen_spell;
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

   return NPC_CastSpell(me, result);
endfunction

function NPC_CastSpell(me, spellname)
   var spellInfo := FindSpellInfoCached(spellname);

   if (!spellInfo)
      return 0;
   endif

   var targ := SystemFindObjectBySerial(spell_target_var);
   spell_target_var := 0;

   var params := array{me, spellInfo, 0, targ};
   var script := Start_Script(":spells:cast_spell", params);
   if (script.errortext)
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
            DAMAGE_TYPE::FIRE:
                spell := ChooseFireSpell();
            DAMAGE_TYPE::ENERGY:
                spell := ChooseEnergySpell();  
            DAMAGE_TYPE::COLD:
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

function HumanoidShamanBehavior(me, opponent)
    if (GetCooldown(me, "spell_cast"))
        return 0;
    endif

    var spells := GetAvailableShamanSpells(me);
    if (!spells.size())
        return 0;
    endif

    // ChooseShamanSpell already sets spell_target_var
    var chosen_spell := ChooseShamanSpell(me, opponent, spells);
    if (!chosen_spell)
        return 0;
    endif

    return NPC_CastSpell(me, chosen_spell);
endfunction

function ChooseShamanSpell(me, opponent, spells)
    // Get ideal spell based on tactical considerations
    var chosen_spell := ChooseIdealShamanSpells(me, opponent, spells);
    var spellInfo := FindSpellInfoCached(chosen_spell);
    if (!spellInfo)
        spell_target_var := opponent.serial;
        return "Dardo Igneo";
    endif

    // Set target based on spell type
    if (spellInfo.Target in array{"Unico", "Area"})
        spell_target_var := opponent.serial;
    elseif (spellInfo.Target == "Self")
        spell_target_var := me.serial;
    endif

    return chosen_spell;
endfunction

function ChooseIdealShamanSpells(me, opponent, spells)
    var dist := Distance(me, opponent);
    var enemy_count := NPC_GetPlayersPetsAndSummonsNearby(me).size();

    // Non-ideal shaman spells (useless for NPCs) — healing handled by BUDDYATTACKED event
    var non_ideal := array{
        "Criar comida", "Levitar", "Manter Fogueira", "Visao Noturna", "Voz Fantasma",
        "Observar", "Despertar", "Sacrificar Animal", "Montaria das Areias",
        "Encantar Monstros", "Cura Leve", "Cura Grave", "Nutrir o corpo",
        "Protecao Contra a Morte", "Forca do Touro", "Lama", "Circulo de Pedras"
    };

    // Debuffs — cast once per target, don't spam (they don't stack)
    var debuffs := array{ "Enfraquecer", "Intimidar", "Enraizar", "Rajada de Vento" };

    // Root if enemy in melee (once per cooldown)
    if (dist <= 2 && ("Enraizar" in spells) && !GetCooldown(opponent, "shaman_enraizar"))
        SetCooldown(opponent, "shaman_enraizar", 30);
        return "Enraizar";
    endif

    // Build weighted list of offensive spells
    var valid_spells := array{};
    foreach spell in spells
        if (spell in non_ideal)
            continue;
        endif

        // Skip debuffs already applied to this target
        if (spell in debuffs && GetCooldown(opponent, "shaman_" + spell))
            continue;
        endif

        var weight := 50;

        if (spell == "Dardo Igneo" || spell == "Raio de Gelo")
            weight := 70;
        elseif (spell == "Enraizar")
            weight := 40;
        elseif (spell == "Intimidar")
            weight := 40;
        elseif (spell == "Enfraquecer")
            weight := 40;
        elseif ((spell == "Terremoto" || spell == "Trovao") && enemy_count > 2)
            weight := 90;
        elseif (spell == "Sugar Alma")
            weight := 75;
        elseif (spell == "Rajada de Vento" && dist <= 3)
            weight := 50;
        elseif (spell == "Frenesi")
            weight := 55;
        elseif (spell == "Rito de Sangue")
            weight := 60;
        endif

        valid_spells.append(struct{ "name" := spell, "weight" := weight });
    endforeach

    valid_spells := SortSpellsByPriority(valid_spells);

    if (valid_spells.size() > 0)
        var chosen := ChooseWeightedSpell(valid_spells);
        // Set debuff cooldown on target so we don't re-cast it
        if (chosen in debuffs)
            SetCooldown(opponent, "shaman_" + chosen, 30);
        endif
        return chosen;
    endif

    return "Dardo Igneo";
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

    var spells := GetAvailablePriestSpells(me);
    if (!spells.size())
        return 0;
    endif

    // ChoosePriestSpell already sets spell_target_var
    var chosen_spell := ChoosePriestSpell(me, opponent, spells);
    if (!chosen_spell)
        return 0;
    endif

    return NPC_CastSpell(me, chosen_spell);
endfunction

function ChoosePriestSpell(me, opponent, spells)
    // Healing handled by BUDDYATTACKED event — this function is offense/protection only
    var non_ideal := array{
        "Luz Divina", "Orientar", "Benzer Atadura", "Olhar da Justica",
        "Detectar Magia", "Detectar Impurezas", "Imbuir Protecao",
        "Ressureicao", "Detectar Corrupcao", "Cura Leve", "Cura Grave",
        "Cura em Massa", "Santuario", "Prece Curativa", "Despertar",
        "Descanso", "Sacrificio", "Campo Anti-Magia"
    };

    // Defensive buffs when under attack
    if (me.opponent)
        if ("Escudo de Fe" in spells && !GetCooldown(me, "shield_spell"))
            SetCooldown(me, "shield_spell", 30);
            spell_target_var := me.serial;
            return "Escudo de Fe";
        endif
        if ("Abencoar" in spells && !GetCooldown(me, "bless_spell"))
            SetCooldown(me, "bless_spell", 30);
            spell_target_var := me.serial;
            return "Abencoar";
        endif
        if ("Circulo de Protecao" in spells && !GetCooldown(me, "circle_protection"))
            SetCooldown(me, "circle_protection", 45);
            spell_target_var := me.serial;
            return "Circulo de Protecao";
        endif
        if ("Aura Mortis" in spells && !GetCooldown(me, "aura_mortis"))
            SetCooldown(me, "aura_mortis", 45);
            spell_target_var := me.serial;
            return "Aura Mortis";
        endif
    endif

    // Build weighted list of offensive/protective spells
    var valid_spells := array{};
    foreach spell in spells
        if (spell in non_ideal)
            continue;
        endif

        var weight := 50;

        if (spell == "Chama Sagrada")
            weight := 70;
        elseif (spell == "Juizo Flamejante")
            weight := 80;
        elseif (spell == "Destruir o Mal")
            weight := 85;
        elseif (spell == "Remover Maldicao")
            weight := 40;
        elseif (spell == "Dissipar Magia")
            weight := 45;
        elseif (spell == "Repelir Magias")
            weight := 50;
        elseif (spell == "Protecao Contra a Morte")
            weight := 45;
        elseif (spell == "Imunidade a Veneno")
            weight := 40;
        endif

        valid_spells.append(struct{ "name" := spell, "weight" := weight });
    endforeach

    valid_spells := SortSpellsByPriority(valid_spells);

    if (valid_spells.size() > 0)
        var chosen := ChooseWeightedSpell(valid_spells);
        spell_target_var := opponent.serial;
        return chosen;
    endif

    // Fallback: Chama Sagrada if available, else first available non-ideal spell
    spell_target_var := opponent.serial;
    if ("Chama Sagrada" in spells)
        return "Chama Sagrada";
    endif
    return spells[1];
endfunction

// Druid NPC behavior

function GetAvailableDruidSpells(monster)
    var cache := NPC_GetCachedData(monster);
    var tier := cache.tier;
    if (!tier) tier := 1; endif
    return GetAvailableDruidSpellsCached(tier);
endfunction

function DruidBehavior(me, opponent)
    if (GetCooldown(me, "spell_cast"))
        return 0;
    endif

    var spells := GetAvailableDruidSpells(me);
    if (!spells.size())
        return 0;
    endif

    var chosen_spell := ChooseDruidSpell(me, opponent, spells);
    if (!chosen_spell)
        return 0;
    endif

    return NPC_CastSpell(me, chosen_spell);
endfunction

function ChooseDruidSpell(me, opponent, spells)
    var chosen_spell := ChooseIdealDruidSpells(me, opponent, spells);
    var spellInfo := FindSpellInfoCached(chosen_spell);
    if (!spellInfo)
        spell_target_var := opponent.serial;
        return "Fogo Fatuo";
    endif

    if (spellInfo.Target in array{"Unico", "Area"})
        spell_target_var := opponent.serial;
    elseif (spellInfo.Target == "Self")
        spell_target_var := me.serial;
    endif

    return chosen_spell;
endfunction

function ChooseIdealDruidSpells(me, opponent, spells)
    var dist := Distance(me, opponent);
    var enemy_count := NPC_GetPlayersPetsAndSummonsNearby(me).size();

    // Non-ideal druid spells for NPC combat
    var non_ideal := array{
        "Criar comida", "Manter Fogueira", "Fala Selvagem", "Acalmar bestas",
        "Percepção da Coruja", "Invocar Wisp", "Cura Leve", "Pomo da Vitalidade",
        "Canção das águas cristalinas", "Revelação da Lua", "Despetrificar",
        "Encantar Pessoa", "Saltar", "Caminhada do vento", "Reviver animais",
        "Manto da Casca de Carvalho", "Misericórdia da Corrente"
    };

    // Root if enemy in melee
    if (dist <= 2 && ("Enraizar" in spells))
        return "Enraizar";
    endif

    var valid_spells := array{};
    foreach spell in spells
        if (spell in non_ideal)
            continue;
        endif

        var weight := 50;

        if (spell == "Fogo Fatuo" || spell == "Queimadura por frio" || spell == "Vento Silvestre")
            weight := 70;
        elseif (spell == "Tempestade de Espinhos")
            weight := 75;
        elseif (spell == "Enraizar")
            weight := 60;
        elseif (spell == "Pira Purificadora")
            weight := 70;
        elseif (spell == "Renascimento da Terra" && enemy_count > 2)
            weight := 85;
        elseif (spell == "Enxame")
            weight := 75;
        elseif (spell == "Ventania selvagem" && dist <= 3)
            weight := 80;
        elseif (spell == "Corrente viva")
            weight := 70;
        elseif (spell == "Crescimento selvagem")
            weight := 65;
        elseif (spell == "Tremor dos antigos" && enemy_count > 2)
            weight := 90;
        endif

        valid_spells.append(struct{ "name" := spell, "weight" := weight });
    endforeach

    valid_spells := SortSpellsByPriority(valid_spells);

    if (valid_spells.size() > 0)
        return ChooseWeightedSpell(valid_spells);
    endif

    return "Fogo Fatuo";
endfunction
