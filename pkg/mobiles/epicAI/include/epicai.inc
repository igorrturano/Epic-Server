use cfgfile;

include ":loot:include/lootParser";
include ":tn:recipes";
include ":mounts:mounts";
include "npcUtils";
include ":fls_core:fls_characters";
include "makeRace";
include ":epicAI:epicSpecial";
include ":npcs:NPCUtilities";

const MAX_Z_DIFF := 60; // Increased to handle watchtowers, wooden walls, and multi-floor buildings

function InitializeEpicNPC(mobile)
    var npc_already_initiliazed := GetObjProperty(mobile, "started");
    if (npc_already_initiliazed)
        return 0;
    endif

	var npc_elem := NPC_GetNPCConfig(mobile);
    var npccfgfile := ReadConfigFile( ":epicai:npcdesc" );
    var mytemplate := ParseTemplateName(mobile.npctemplate).template;
    var elem := FindConfigElem(npccfgfile, mytemplate);
    var sounds := GetConfigStringArray(elem, "IdleSound");

    var isDarknessCreature := GetObjProperty(mobile, "Mobnoturno") || Lower(npc_elem.Category) == "undead";
    if ((isDarknessCreature) && !InDarkness(mobile))
        ApplyRawDamage(mobile, CInt(AP_GetVitalMaximumValue(mobile, "Hits")));
    endif

    setupAnchorNPC(mobile);
    setupChampspawnNPC(mobile);
    setupNPCBackPack(mobile);
    GenerateRecipeLoot(mobile);
    SetupAmmo(mobile, npccfgfile);
    var loot := npccfgfile[mytemplate].Loot;
    if (loot)
        Loot_Generate(mobile.backpack, loot);
    endif

    var boss_struct := GetObjProperty(mobile, "EpicBossStruct");
    if (boss_struct)
        HandleInitializationLoot(mobile, boss_struct);
    endif

    var throwItem := CInt(npc_elem.throwItem);
    if (throwItem != error)
        SetObjProperty(mobile, "throwItem", throwItem);
    endif

    var brain_type := GetConfigString(npc_elem, "BrainType");
    // Type-specific initialization
    case (brain_type)
        "monster":  InitializeMonsterNPC(mobile, npc_elem, elem, sounds);
        "animal":   InitializeAnimalNPC(mobile, npc_elem, elem, sounds);
        "humanoid": InitializeHumanoidNPC(mobile, npc_elem, elem, sounds);
        default:    InitializeGenericNPC(mobile, npc_elem, elem, sounds);
    endcase

    SetObjProperty(mobile, "started", 1);
endfunction

function InitializeMonsterNPC(mobile, npc_elem, elem, sounds)
    var has_colors := npc_elem.colors;
    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif
    SetupNPCEquip(mobile, npc_elem);

    // Monstros falam uma língua específica
    var known_languages := SplitWords(npc_elem.Languages);
    var chosen_language := known_languages[RandomInt(Len(known_languages)) + 1];
    SetObjProperty(mobile, "linguas_conhecidas", known_languages);
    SetObjProperty(mobile, "lingua_escolhida", chosen_language);

    // Add any other monster-specific setup here
endfunction

function InitializeAnimalNPC(mobile, npc_elem, elem, sounds)
    var has_colors := npc_elem.colors;
    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif

    SetupAnimalGender(mobile, npc_elem);
    SetupAnimalCriticalHit(mobile, npc_elem);

    // Animal-specific initialization
endfunction

function InitializeHumanoidNPC(mobile, npc_elem, elem, sounds)
    var has_colors := npc_elem.colors;
    var isBoss := GetObjProperty(mobile, "EpicBossStruct");
    var race := npc_elem.epicRace;

    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif

    SetupHumanoidHabs(mobile);
    SetupItemsForSpecificClasses(mobile, npc_elem);

    SetupNPCEquip(mobile, npc_elem);
    setupNPCGender(mobile, Cint(isBoss.gender));

    if (isBoss)
        race := isBoss.race;
        makeRace(mobile, race);
    elseif (race)
        makeRace(mobile, race);
    else 
        makeRace(mobile);
    endif

    SetupBaseLanguages(mobile);

    if (mobile.isSage())
        SetupWizardClass(mobile, npc_elem);
    endif

    SetupNPCAppearence(mobile);
endfunction

function SetupItemsForSpecificClasses(mobile, npc_elem)
    var habs := GetObjProperty(mobile, "myhabs");
    if (!habs)
        return 0;
    endif

    // Setup backpack if needed
    if (!mobile.backpack)
        var backpack := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, "BackPack", 1);
        EquipItem(mobile, backpack);
    endif

    case (npc_elem.Classe)
        GUERREIRO:
            if ("Budōka" in habs)
                var monk_weapons := array{0x0df0, 0x13f9, 0x0e8a, 0x0e81, 0x1F020};
                EquipRandomFromArray(mobile, monk_weapons);
            endif

            if ("Camuflagem" in habs)
                var tier := CInt(npc_elem.Tier);
                var template := FindAnimalTemplate(tier);
                if (template)
                    SetObjProperty(mobile, "#camuflagem_template", template);
                    var pet := CreateNpcFromTemplate(":epicai:"+template, mobile.x, mobile.y, mobile.z, 0, mobile.realm);
                    sleepms(250);  // This was the key - giving the NPC time to initialize
                    pet.alignment := 1;
                    SetObjProperty(pet, "owner", mobile.serial);
                    GrantPrivilege(pet, "hiddenattack");
                    pet.enable("hiddenattack");
                    GrantPrivilege(mobile, "hiddenattack");
                    mobile.enable("hiddenattack");
                    mobile.stealthsteps := 30;
                    pet.stealthsteps := 30;
                endif
                var net := CreateItemInBackpack(mobile, "net", RandomInt(2)+1);
            endif

            if ("Nazg khalug" in habs)
                makeRace(mobile, ORC);
            endif
        LADINO:
            if ("Mestre dos Venenos" in habs)
                var bleed_weapons := array{0x1401, 0x13f6, 0x0ec2, 0x26C5, 11565, 0x27af, 0x27ab};
                EquipRandomFromArray(mobile, bleed_weapons);
            endif

            if ("Arsenal Sombrio" in habs)
                var assassin_weapons := array{0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2};
                EquipRandomFromArray(mobile, assassin_weapons);
            endif

            if ("Sensibilidade Arcana" in habs)
                SetupArcaneTricksterLoadout(mobile, npc_elem);
            endif

        SABIO:
            if ("Alquimista Experiente" in habs)
                SetupAlchemistLoadout(mobile);
            endif

            if ("Artilheiro Preparado" in habs)
                makeRace(mobile, ANAO);
                SetupFireweaponLoadout(mobile);
            endif

            if ("Construtor de Golems" in habs)
                makeRace(mobile, ANAO);
                // Store mobile's current position before hiding it
                SetObjProperty(mobile, "#original_pos", struct{
                    "x" := mobile.x,
                    "y" := mobile.y,
                    "z" := mobile.z,
                    "realm" := mobile.realm
                });
                
                // Create the golem slightly offset
                var golem := CreateNpcFromTemplate(":epicai:GolemMartelo_NPC", mobile.x+1, mobile.y+1, mobile.z, 0, mobile.realm);

                SetObjProperty(golem, "golem_maker", mobile.serial);
                SetObjProperty(mobile, "pilotando", 1);

                // Make the mobile anchor in the golem
                if (golem)
                    SetObjProperty(mobile, "anchor", array{ 4230 , 25 , 1 });
                    // Hide the original NPC
                    MoveObjectToLocation(mobile, 4230, 25, 1, mobile.realm, MOVEOBJECT_FORCELOCATION);
                endif
            endif

            if ("Vínculo Elemental" in habs)
                makeRace(mobile, ELFO);
                var elementals := array{0x13fe, 0x26C9, 0x26C5, 0x26CB, 0x26CF, 0xF9E6};
                EquipRandomFromArray(mobile, elementals);
            endif

            if ("Visao verdadeira" in habs)
                mobile.gender := 1;
                makeRace(mobile, DROW);
                var whip := CreateItemInBackpack(mobile, 0x166E, 1);
                EquipItem(mobile, whip);
            endif

            if ("Campeao das Montanhas" in habs)
                makeRace(mobile, ANAO);
                var hammer := CreateItemInBackpack(mobile, 0xFA11, 1);
                EquipItem(mobile, hammer);
            endif

            if ("Sede de Sangue" in habs)
                makeRace(mobile, ORC);
                var two_handed := array{0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2};
                EquipRandomFromArray(mobile, two_handed);
            endif

            if ("Gwaenedh" in habs)
                makeRace(mobile, ELFO);
            endif

            if ("Elo Natural" in habs)
                makeRace(mobile, ELFO);
            endif

            if ("Forma Adaptavel" in habs)
                var two_handed := array{0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2};
                EquipRandomFromArray(mobile, two_handed);
            endif

    endcase

    return 1;
endfunction

function FindAnimalTemplate(tier)
    var npccfgfile := ReadConfigFile(":epicai:npcdesc");
    if (!npccfgfile)
        return;
    endif

    var valid_templates := array{};
    
    // Iterate through all entries
    foreach entry in (GetConfigStringKeys(npccfgfile))
        var elem := FindConfigElem(npccfgfile, entry);
        
        // Check if it's an animal of the same tier
        if (elem.BrainType == "animal" && CInt(elem.Tier) == tier)
            // Store the template name
            valid_templates.append(entry);
        endif
        sleepms(2);
    endforeach
    // If we found valid templates, return a random one
    if (valid_templates.size() > 0)
        return valid_templates[RandomIntMinMax(1, valid_templates.size())];
    endif
    return 0;
endfunction

function SetupFireweaponLoadout(mobile)
    // Create and equip random firearm
    var firearms := array{0xC5FD, 0xC5FE, 0xC5FF, 0xC600, 0x89A7};
    var weapon := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, firearms[RandomInt(firearms.size())+1], 1);
    if (weapon)
        SetObjProperty(weapon, "balas", 1);
        SetObjProperty(weapon, "serialid", mobile.serial);
        EquipItem(mobile, weapon);
    endif

    // Add ammo and supplies
    var ammo_amount := RandomInt(10) + 10; // 10-20
    var powder_amount := RandomInt(10) + 10;

    CreateItemInBackpack(mobile, 0x9090, ammo_amount);  // Ammo
    CreateItemInBackpack(mobile, 0x9092, powder_amount); // Powder
    CreateItemInBackpack(mobile, 0xC611, 1);            // Powder fixer
endfunction

function SetupAlchemistLoadout(mobile)
    // Create explosive potions
    var explosive_potions := array{0xbf13, 0xbf14, 0xbf15, 0xef60, 0xef61, 0xef62};
    foreach potion in explosive_potions
        for i := 1 to 2
            CreateItemInBackpack(mobile, potion, 1);
            sleepms(2);
        endfor
    endforeach
    
    // Create healing potions
    var heal_potions := array{0xDC01, 0xDC02, 0xDC03};
    foreach potion in heal_potions
        for i := 1 to 2
            CreateItemInBackpack(mobile, potion, 1);
            sleepms(2);
        endfor
    endforeach
    
    // Speed potion
    CreateItemInBackpack(mobile, 0xbf0d, 1);
endfunction

function SetupArcaneTricksterLoadout(mobile, npc_elem)
    // Create bow and quiver
    var bow := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, 0x13B2, 1);
    var quiver := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, 0x2FB7, 1);
    
    if (bow && quiver)
        EquipItem(mobile, bow);
        EquipItem(mobile, quiver);
    endif
    
    // Add arrows
    CreateItemInContainer(quiver, 0xF3F, RandomIntMinMax(20, 80));
    var tier := npc_elem.Tier;
    // Add scrolls for tiers 1-5
    for i := 1 to tier
        CreateItemInBackpack(mobile, "pergaminhospellcaster"+i, RandomInt(2)+1);
    endfor
endfunction

function EquipRandomFromArray(mobile, items)
    var weapon := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, items[RandomInt(items.size())+1], 1);
    if (weapon)
        if (mobile.weapon)
            DestroyItem(mobile.weapon);
        endif
        EquipItem(mobile, weapon);
    endif
endfunction

function SetupHumanoidHabs(mobile)
    var wrestling := AP_GetSkill(mobile, WRESTLING);
    var tactics := AP_GetSkill(mobile, TACTICS);
    var calculate_level_hp := CInt(wrestling + tactics) - 80;
    
    // Add safety cap to prevent overflow
    if (calculate_level_hp > 100)
        calculate_level_hp := 100;
    endif
    
    var npc_level := (calculate_level_hp / 6);
    // Ensure npc_level is within reasonable bounds
    if (npc_level < 0)
        npc_level := 0;
    elseif (npc_level > 20)
        npc_level := 20;
    endif
    
    var habs_by_level := GetHabsByLevel(mobile, npc_level);
    var habs := array{};
    foreach hab in habs_by_level
        habs.append(hab);
        sleepms(2);
    endforeach

    SetObjProperty(mobile, "myhabs", habs);
endfunction

function SetupWizardClass(mobile, npc_elem)
    // First try to get school from config
    var spellschool := GetConfigString(npc_elem, "SpellSchool") ? GetConfigString(npc_elem, "SpellSchool") : GetObjProperty(mobile, "SpellSchool");
    var base_ability := 0;
    
    // If no school defined in config, check habs
    if (!spellschool)
        var habs := GetObjProperty(mobile, "myhabs");
        if (habs)
            if ("Vínculo Elemental" in habs)
                spellschool := "Ahtaleloreth";
            elseif ("Elo Natural" in habs)
                spellschool := "Aradalore";
            elseif ("Sacrificio da Carne" in habs)
                spellschool := "Kaijin";
            elseif ("Danca das Sombras" in habs)
                spellschool := "Velkyn";
            elseif ("Determinacao Arcana" in habs)
                spellschool := "Evocatium";
            endif
        endif
    endif

    // Map school to base ability
    case (spellschool)
        "Kaijin": base_ability := "Sacrificio da Carne";
        "Ahtaleloreth": base_ability := "Vínculo Elemental";
        "Aradalore": base_ability := "Elo Natural";
        "Evocatium": base_ability := "Determinacao Arcana";
        "Velkyn": base_ability := "Danca das Sombras";
    endcase

    if (!base_ability)
        return 0;
    endif

    // Store the school
    SetObjProperty(mobile, "SpellSchool", spellschool);

    // Build progression based on base ability
    var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
    var progression := GetClassProgression(habcfg, base_ability);
    SetObjProperty(mobile, "myhabs", progression);

    return 1;
endfunction

function SetupWarriorClass(mobile, npc_elem)
    if (!npc.isWarrior())
        return 0;
    endif
    var subclass;
    var speed_type := SplitWords(GetConfigString(npc_elem, "EpicAttackSpeed"), "_");
    var isArcher := (speed_type[1] == "RANGED");
    if (isArcher)
      subclass := "Tiro Poderoso";
    endif

    var progression := GetClassProgression(habcfg, subclass);
    SetObjProperty(mobile, "myhabs", progression);

    return 1;
endfunction

function SetupBaseLanguages(mobile)
    SetObjProperty(mobile, "linguas_conhecidas", {"Comum"});
    SetObjProperty(mobile, "lingua_escolhida", "Comum");
endfunction

function InitializeGenericNPC(mobile, npc_elem, elem, sounds)
   // Chamar o initializeNPC do ghaia para fazer uma migração não destrutiva, mesmo que
   // eu deva só adicionar mobs que eu tenha criado
endfunction

function SetupAmmo(mobile, npccfgfile)

	if ( npccfgfile[ParseTemplateName(mobile.npctemplate).template].MissileWeapon )
		var weapon := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, npccfgfile[ParseTemplateName(mobile.npctemplate).template].MissileWeapon, 1, mobile.realm);
		if ( !EquipItem(mobile, weapon) )
			DestroyItem(weapon);
		endif
	endif

	if ( npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoType )
		if ( CInt(npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoAmount) > 0 )
			var ammo := CreateItemInBackpack(mobile, npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoType, CInt(npccfgfile[ParseTemplateName(mobile.npctemplate).template].AmmoAmount));
			if ( !ammo )
				return ammo;
			endif
			return 1;
		endif
	endif

	return 1;

endfunction

function SetupNPCEquip(who, npc_elem)
    var categ := GetConfigString(npc_elem, "Equip");
    categ := categ;
    if(isMaleGraphic(who))
        categ := categ + "M";
    elseif (who.graphic == 0x191)
        categ := categ + "F";
    endif
    var template := ReadConfigFile(":ghaia:config/equip2");
    var group := FindConfigElem(template, categ);
    if(group)
        var str;
        var lastc := 0;
        foreach str in GetConfigStringArray(group, "Equip")
            lastc := EquipNPCItemString(who, str, lastc);
        endforeach
        foreach str in GetConfigStringArray(group, "Armor")
            lastc := EquipNPCItemString(who, str, lastc );
        endforeach
        foreach str in GetConfigStringArray(group, "Weapon")
            lastc := EquipNPCItemString(who, str, lastc);
        endforeach
    endif
    var boss := GetObjProperty(who, "EpicBossStruct");
    if (boss.specific_weapon)
        var weapon := boss.specific_weapon;
        if (weapon)
            var item := CreateItemAtLocation(who.x, who.y, who.z, weapon, 1);
            // First Unequip hands
            if (who.weapon)
                DestroyItem(who.weapon);
            endif
            if (who.shield)
                DestroyItem(who.shield);
            endif
            EquipItem(who, item);
        endif
    endif

    if (boss.equipments && boss.equipments.size() > 0)
        foreach equipment in (boss.equipments)
            var objtype := CInt(equipment.objtype);
            if (objtype)
                // Use amount if specified, otherwise default to 1
                var amount := equipment.amount ? CInt(equipment.amount) : 1;
                var item := CreateItemAtLocation(who.x, who.y, who.z, objtype, amount);
                if (item)
                    // Apply color if specified
                    if (equipment.color && equipment.color != 0)
                        item.color := CInt(equipment.color);
                    endif
                    
                    // Apply all other properties that were captured
                    if (equipment.properties && equipment.properties.size() > 0)
                        foreach prop_name in (equipment.properties.keys())
                            var prop_value := equipment.properties[prop_name];
                            // Skip objtype, color, and amount as they're handled separately
                            if (prop_name != "objtype" && prop_name != "color" && prop_name != "amount")
                                SetObjProperty(item, prop_name, prop_value);
                            endif
                            sleepms(2);
                        endforeach
                    endif
                    
                    // Try to equip the new item first
                    if (!EquipItem(who, item))
                        // Find what layer this item wants to use
                        var equipped_items := ListEquippedItems(who);
                        var equipment_removed := 0;
                        
                        foreach equipped_item in equipped_items
                            // Try removing each equipped item and see if our item can equip
                            var temp_item := equipped_item;
                            MoveItemToContainer(temp_item, who.backpack);
                            
                            // Try to equip our new item
                            if (EquipItem(who, item))
                                // Success! The temp_item was conflicting, so destroy it
                                DestroyItem(temp_item);
                                equipment_removed := 1;
                                break;
                            else
                                // Not the conflicting item, put it back
                                EquipItem(who, temp_item);
                            endif
                            sleepms(2);
                        endforeach
                        
                        // If we still couldn't equip after removing items, put it in backpack or destroy
                        if (!equipment_removed)
                            if (!MoveItemToContainer(item, who.backpack))
                                DestroyItem(item);
                            endif
                        endif
                    endif
                endif
            endif
            sleepms(2);
        endforeach
    endif

    // Apply boss colors to clothing AFTER all equipment is set up
    if (boss && boss.color_ranges && boss.color_ranges.size() > 0)
        ApplyBossColors(who, boss.color_ranges);
    endif
    
  var montaria := Getobjproperty(who, "Montaria");
  if (boss.mount)
      // If the boss has a specific mount, use it
      montaria := boss.mount;
  endif
  if (montaria)
	var graphic := MP_GetMountGraphic(montaria);
	var item := CreateItemAtLocation(who.x, who.y, who.z, MOUNT_OBJTYPE, 1);
	item.graphic := graphic;
	Equipitem (who, item);
    who.run_speed := 250;
  endif
endfunction

function SetupAnimalCriticalHit(mobile, npc_elem)
    var gender_set := npc_elem.Gender;
    var taxonomy := GetConfigString(npc_elem, "Taxonomy");
    var evil := (mobile.alignment == 1);
    if (!evil)
        return;
    endif

    if (taxonomy == "bovidae" || taxonomy == "cervidae")
        case (RandomInt(1))
            0: SetObjProperty(mobile, "Critic", "Coice" );
            1: SetObjProperty(mobile, "Critic", "Chifrada");
        endcase
    elseif (taxonomy == "suidae" || taxonomy == "camelidae" || taxonomy == "equus")
        SetObjProperty(mobile, "Critic", "Coice");
    elseif (taxonomy == "anura" || taxonomy == "serpentes")
        case (RandomInt(1))
            0: SetObjProperty(mobile, "Critic", "Bote" );
            1: SetObjProperty(mobile, "Critic", "Glandulas_concentradas" );
        endcase
    elseif (taxonomy == "primatomorpha" || taxonomy == "ursidae")
        case (RandomInt(2))
            0: SetObjProperty(mobile, "Critic", "Frenesi" );
            1: SetObjProperty(mobile, "Critic", "Rugir" );
        endcase
    elseif (taxonomy == "canidae" || taxonomy == "felidae" || taxonomy == "chiroptera" || taxonomy == "reptilia" || taxonomy == "crustacean" || taxonomy == "scarabaeidae")
        case (RandomInt(2))
            0: SetObjProperty(mobile, "Critic", "Presas_profundas" );
            1: SetObjProperty(mobile, "Critic", "Garras_afiadas" );
        endcase
    elseif (taxonomy == "immortuos" || taxonomy == "draco")
        SetObjProperty(mobile, "Critic", "Baforada" );
    elseif (taxonomy == "aves" || taxonomy == "phasianidae")
        SetObjProperty(mobile, "Critic", "Garras_afiadas");
    endif

    SetObjProperty(mobile, "Selvagem", 1);

endfunction

function SetupAnimalGender(mobile, npc_elem)
  var gender_set := npc_elem.Gender;

  if ( gender_set == 0)
    //var gender_new := Cint(randomint(2) + 1);
    case(randomint(2))
          0: Setobjproperty(mobile, "Sexo", 1);
          1: Setobjproperty(mobile, "Sexo", 2);
        endcase
  else
    Setobjproperty(mobile, "Sexo", Cint(mobile.Gender) );
  endif

endfunction

function setupNPCBackPack(mobile)
	if ( !mobile.backpack )
		var backpack := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, "BackPack", 1);
		var result := EquipItem(mobile, backpack);
		if ( !result )
			DestroyItem(backpack);
			return result;
		else
			return backpack;
		endif
	endif

	return mobile.backpack;
endfunction

function setupNPCColor(who, colors)
    colors := SplitWords(colors);

    var color := Cint(colors[RandomInt(Len(colors)) + 1]);
    who.color := color;
    who.truecolor := color;
endfunction

function setupNPCGender(mobile, bossFemale := 0)
  var female := GetObjProperty(mobile, "female");

  if (female || bossFemale)
    case (mobile.graphic)
        400: mobile.graphic := 401;
        1830: mobile.graphic := 1831;
        1832: mobile.graphic := 1833;
        1828: mobile.graphic := 1829;
    endcase
    mobile.gender := 1;
  endif
endfunction

function setupAnchorNPC(mobile)
    var spawnpointserial := GetObjProperty(mobile, "spawnpoint");
        if (spawnpointserial)
            var spawnpoint := SystemFindObjectBySerial(spawnpointserial);
            if (spawnpoint)
                var range := cint(GetObjProperty(spawnpoint, "Range"));
            if (!range)
                range := 6;
            endif
            SetObjProperty(mobile, "anchor", array{ spawnpoint.x , spawnpoint.y , spawnpoint.z + 2 });
            SetObjProperty(mobile, "range", range);
            var e := SetAnchor(spawnpoint.x, spawnpoint.y, range, 90);
        endif
    endif
endfunction

function setupChampspawnNPC(mobile)
    if (GetObjPRoperty(mobile, "championspawn") == 2)
        SetObjProperty(mobile, "#championspawn", 1);
        ApplyDamage( mobile, 20000);
    elseif (GetObjPRoperty(mobile, "championspawn") == 1)
        SetObjPRoperty(mobile, "championspawn", 2);
    endif
endfunction

function ParseTemplateName(template_name)

  if ( template_name.IsA(POLCLASS_NPC) )
    template_name := template_name.npctemplate;
  endif

  //Find positions of ":" characters
  var colon_a := Find(template_name, ":", 1);
  var colon_b := Find(template_name, ":", 2);

  var parsed := struct{"package", "template"};

  if ( !colon_a || !colon_b )
    //Not a complete package name - use default npcdesc.cfg
    parsed.package := "";
    parsed.template := template_name;
  else
    parsed.package := template_name[colon_a+1, colon_b-2];
    parsed.template := template_name[colon_b+1, Len(template_name)];
  endif

  return parsed;
endfunction

function EquipNPCItemString(who, str, lastcolor)
    var params := SplitWords(str);
    var objname;
    var color;
    var chance;
    var lastc := 0;
    var colors := ReadConfigFile(":ghaia:config/colors");
    var vestiments := ReadConfigFile(":ghaia:config/cloths");
    var epicBoss := GetObjProperty(who, "EpicBossStruct");
    var bossColor := epicBoss.color_ranges;
    objname := params[1];
    if(CInt(objname))
        objname := CInt(objname);
    endif
    if(CInt(params[2]))
        color := CInt(params[2]);
    else
        color := 0;
    if(params[2] == "lastcolor" || params[2] == "lastcolour")
        color := lastcolor;
    else
        var colorg := FindConfigElem(color, params[2]);
        if(colorg)
            var str := GetConfigStringArray(colorg, "Color");
            var ar := array{};
            var rndint := RandomInt(len(str)) + 1;
            ar := SplitWords(str[rndint]);
        if(len(ar) > 1)
            if(CInt(ar[2]) > 0)
                color := RandomInt(CInt(ar[2]) - CInt(ar[1]))+1;
             else
                color := CInt(str[RandomInt(len(str))+1]);
            endif
            else
                color := CInt(str[RandomInt(len(str))+1]);
            endif
        if(color == 1)
            color := 0;
        endif
      endif
    endif
  endif
  lastc := color;
  if(CInt(params[3]))
    chance := CInt( params[3] );
  else
    chance := 100;
  endif
  if(CInt(RandomInt(99)+1) <= CInt(chance))
    var item := CreateItemAtLocation(5852, 1163, 0, objname, 1);
    if(!item)
      var itemg := FindConfigElem(vestiments, objname);
      if(itemg)
        var str := GetConfigStringArray(itemg, "Item");
        var str3;
        var newarray := array{};
        var i := 1;
        var sex := "";
        foreach str3 in str
          var str2 := SplitWords(str3);
          sex := CStr(str2[2]);
          if(sex != error)
            if(sex == "m" || sex == "f")
              if(sex == "m" && who.graphic == 400)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "f" && who.graphic == 401)
                newarray[i] := str2[1];
                i := i + 1;
              endif
            else
              newarray[i] := str2[1];
              i := i + 1;
            endif
          else
            newarray[i] := str2[1];
            i := i + 1;
          endif
        endforeach
        objname := CInt(newarray[RandomInt(len(newarray))+1]);
        item := CreateItemAtLocation(5852, 1163, 0, objname, 1);
      endif
    endif
    item.color := color;
    if(CInt(params[4]) == 1)
      MoveItemToContainer(item, who.backpack);
    else
      if(!EquipItem(who, item))
        MoveItemToContainer(item, who.backpack);
      endif
    endif
  endif
  return lastc;
endfunction

function Flee(byref opponent, sleepdelay := 0, change_opponent := 0)
  //  if ("FleeStealth" in custombehaviors)
  //     if (!GetCooldown(me, "fleestealth"))
  //        PrintTextAbove(me, "*esconde*");
  //        sleep(1);
  //        me.hidden := 1;
  //        me.stealthsteps := 30;
  //        SetCooldown(me, "fleestealth", 30);
  //     endif
  //  endif

   var numflees := cint(GetObjProperty(me,"#flees"));
   if(numflees > 10)
      EraseObjProperty(me, "#flees");
      RestartScript(me);
   else
      numflees := numflees + 1;
      SetObjProperty(me,"#flees", numflees);
   endif
   
   var runs := 0;
   if (!sleepdelay)
      sleepdelay := 275 - me.run_speed;
   endif
   var last_loc := array{opponent.x, opponent.y, opponent.z};
   while(Distance(me, opponent) < 30 && runs < 50 && CoordinateDistance(opponent.x, opponent.y, last_loc[1], last_loc[2]) <= 3)
      sleepms(sleepdelay);
      var ev := wait_for_event(0);
      TryToRunAway(opponent);
      runs := runs +1;

      if (change_opponent && randomint(6) <= 2)
         me.setAggro(opponent.serial, -100);
         if (ChooseHighestOpponent(me, opponent))
            break;
         endif
      endif

      if (ev.type == SYSEVENT_DAMAGED)
         if (((ev.source.npctemplate) and (CheckLineOfSight(me, ev.source))) 
               || (CoordinateDistance(me.x, me.y, ev.source.x, ev.source.y) < CoordinateDistance(me.x, me.y ,opponent.x, opponent.y)) 
               || (!CheckLineOfSight(me, opponent)) || (!opponent) || (RandomInt(3)==1))
            opponent := ev.source;
            SetOpponent(opponent);
            me.setAggro(opponent, 300);
         endif
      endif

      findAnotherPath();
   endwhile

   SetObjProperty(me, "#noback", polcore().systime+15);
endfunction

function ChooseHighestOpponent(me, byref opponent, new_opponent := 0, force_change := 0)
   if (!opponent && new_opponent)
      opponent := new_opponent;
      SetOpponent(new_opponent);
      TurnToward(new_opponent);
      return 1;
   elseif (opponent && new_opponent)
      SetOpponent(opponent);
      TurnToward(opponent);
      return 1;
   elseif (opponent && !CheckLineOfSight(me, opponent))
      me.setAggro(opponent.serial, -50);
   endif

   if (GetCooldown(me, "#blockaggro"))
      return 0;
   endif
endfunction

function TacticalReposition(opponent, chasing)
    var runs := 0;
    var max_dist := 12;
    var start_pos := struct{ "x" := me.x, "y" := me.y };
    var stuck_count := 0;

    while(Distance(me, opponent) < max_dist && runs < 10)
        var old_pos := struct{ "x" := me.x, "y" := me.y };
        TryToRunAway(opponent, 1);
        runs += 1;
        sleepms(200);

        // Check if we're stuck
        if (me.x == old_pos.x && me.y == old_pos.y)
            stuck_count += 1;
            if (stuck_count >= 2)
                // Try to find an alternative path
                var possible_locations := GetAlternativeLocations(opponent);
                foreach loc in possible_locations
                    if (CheckLosAt(me, loc.x, loc.y, me.z))
                        RunTowardLocation(loc.x, loc.y);
                        return 1;
                    endif
                    sleepms(2);
                endforeach
            endif
        endif
    endwhile

    return 1;
endfunction

function GetAlternativeLocations(opponent)
    var locations := array{};
    var range := 8;
    
    // Try cardinal directions first
    var directions := array{
        struct{ "x" := range, "y" := 0 },
        struct{ "x" := -range, "y" := 0 },
        struct{ "x" := 0, "y" := range },
        struct{ "x" := 0, "y" := -range }
    };

    // Add diagonal directions
    directions.append(struct{ "x" := range, "y" := range });
    directions.append(struct{ "x" := -range, "y" := range });
    directions.append(struct{ "x" := range, "y" := -range });
    directions.append(struct{ "x" := -range, "y" := -range });

    foreach dir in directions
        var new_x := me.x + dir.x;
        var new_y := me.y + dir.y;
        
        // Check if location is away from opponent
        if (CoordinateDistance(new_x, new_y, opponent.x, opponent.y) > Distance(me, opponent))
            locations.append(struct{
                "x" := new_x,
                "y" := new_y,
                "dist" := CoordinateDistance(new_x, new_y, opponent.x, opponent.y)
            });
        endif
        sleepms(2);
    endforeach

    // Sort by distance from opponent (furthest first)
    locations.sort();
    
    return locations;
endfunction

function findAnotherPath()
    var start_pos := struct{ "x" := me.x, "y" := me.y };
    var reposition_tries := 0;

    if (me.x == start_pos.x && me.y == start_pos.y)
        reposition_tries += 1;
        if (reposition_tries >= 3)
            // Try to move laterally
            var directions := array{
                struct{ "x" := 2, "y" := 0 },
                struct{ "x" := -2, "y" := 0 },
                struct{ "x" := 0, "y" := 2 },
                struct{ "x" := 0, "y" := -2 }
            };
            foreach dir in directions
                var new_x := me.x + dir.x;
                var new_y := me.y + dir.y;
                if (CheckLosAt(me, new_x, new_y, me.z))
                    RunTowardLocation(new_x, new_y);
                    sleepms(200);
                    break;
                endif
                sleepms(2);
            endforeach
            reposition_tries := 0;
        endif
    endif
endfunction

function CheckNearbyThreats()
    // Scans area for hostile targets and returns the first one found
    // Returns: mobile object if threat found, 0 if none found
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL, me.realm)
        if (CanFight(me, mobile))
            return mobile;
        endif
        sleepms(2);
    endforeach

    return 0;
endfunction

function CheckForIdealTarget()
   // Get the target priority
   var target_priority := GetObjProperty(me, "target_priority");
   var switch_chance := 70; // Default 10% chance to switch
   case (target_priority)
      "Aggressive": switch_chance := 50;  // 15% chance to switch
      "Defensive":  switch_chance := 85;   // 5% chance to switch
      "Focus":      switch_chance := 95;   // 2% chance to switch
   endcase
   
   // If we already have an opponent, check if we should stick with them
   if (me.opponent && CanFight(me, me.opponent))
      if (RandomInt(100) < switch_chance)
         return me.opponent;
      endif
   endif

   var potential_opponents := ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL, me.realm);
   if (potential_opponents.size() < 1)
      return 0;
   endif
   var best_opponent := struct{
      "mobile" := 0,
      "priority" := -1
   };

   foreach opponent in potential_opponents
      // Skip invalid opponents
      if (!CanFight(me, opponent))
         continue;
      endif

      var opponent_class := opponent.getClass();
      var opponent_is_ranged := opponent.weapon.isRanged() || opponent_class == SABIO;
      var priority := 0;

       // Current opponent gets huge priority bonus to maintain aggro
      if (opponent == me.opponent)
         priority += 15;
      endif

      // Fast NPC prioritizes ranged players
      if (opponent_is_ranged)
         priority += 2;
      endif

      // Class-based opponenting
      if (me.isSage()) // NPC mage prefers warriors
         if (opponent_class == GUERREIRO)
            priority += 2;
         endif
      elseif (me.isWarrior()) // NPC warrior prefers rogues
         if (opponent_class == LADINO)
            priority += 2;
         endif
      elseif (me.isRogue()) // NPC rogue prefers mages
         if (opponent_class == SABIO)
            priority += 2;
         endif
      endif

      // Prioritize close opponents slightly
      var dist := Distance(me, opponent);
      if (dist <= 4)
         priority += 1;
      endif

      // Check for preferred target types
      var preferred_target := GetObjProperty(me, "preferred_target");
      if (preferred_target)
          case (preferred_target)
          "Ranged": if (opponent_is_ranged) priority += 4; endif
          "Melee": if (!opponent_is_ranged) priority += 4; endif
          "Guerreiro": if (opponent_class == GUERREIRO) priority += 4; endif
          "Ladino": if (opponent_class == LADINO) priority += 4; endif
          "Mago": if (opponent_class == SABIO) priority += 4; endif
          "Pouca vida": 
              var hp_percent := CDbl(AP_GetVital(opponent, HITS)) / CDbl(AP_GetVitalMaximumValue(opponent, HITS)) * 100;
              if (hp_percent <= 50) 
                  priority += 3;
              endif
          endcase
      endif

      // Check if this is a better opponent
      if (priority > best_opponent.priority)
         best_opponent.mobile := opponent;
         best_opponent.priority := priority;
      endif

      sleepms(2);
   endforeach

   return best_opponent.mobile;
endfunction

// New function to handle ally behavior
function AlertAllies(invasor, me)
  if (isAlly(invasor))
        return 0;
    endif

    if (invasor.hidden || invasor.concealed || GetObjProperty(invasor, "noattack") )
        return 0;
    endif

    if (GetCooldown(me, "alert_allies"))
        return 0;
    endif
    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 8, me.realm)
        if (mobile.npctemplate && mobile != me)
            if (isAlly(mobile))
                SetObjProperty(mobile, "#taunt_opponent", invasor.serial);
                SendEvent(mobile, struct{ "type" := EVID_ALERT_ALLIES, "source" := invasor });
            endif
        endif
        sleepms(2);
    endforeach

    SetCooldown(me, "alert_allies", 15);
endfunction

function isAlly(mobile)
    if (!mobile || mobile.dead) return 0; endif

    if (mobile.master || GetObjProperty(mobile, "summoned")) return 0; endif

    if (mobile.hidden || mobile.concealed) return 0; endif

    if (mobile.acctname || mobile.alignment != 1) return 0; endif
    
    return 1;
endfunction

// Simplified NPC version of ExaminarArea for "hearing footsteps"
// NPCs use this to detect nearby players without LoS and investigate
// Only detects players who are actually MOVING (not standing still)
// Returns: mobile reference if detected someone, 0 otherwise
function NPCHearFootsteps(npc)
    var hearing_range := 8;

    foreach mobile in ListMobilesNearLocationEx(npc.x, npc.y, npc.z, hearing_range, LISTEX_FLAG_NORMAL, npc.realm)
        if (mobile == npc)
            continue;
        endif

        if (mobile.npctemplate)
            continue;
        endif

        if (CheckLineOfSight(npc, mobile))
            continue;
        endif

        if (abs(mobile.z - npc.z) >= 19)
            continue;
        endif

        // Check if player is actually moving (not standing still)
        var last_pos := GetObjProperty(npc, "#heard_" + mobile.serial);
        var current_pos := struct{ "x" := mobile.x, "y" := mobile.y, "time" := polcore().systime };

        if (last_pos)
            var has_moved := (mobile.x != last_pos.x || mobile.y != last_pos.y);
            var time_diff := current_pos.time - last_pos.time;

            if (!has_moved || time_diff > 2)
                SetObjProperty(npc, "#heard_" + mobile.serial, current_pos);
                continue;
            endif
        endif

        SetObjProperty(npc, "#heard_" + mobile.serial, current_pos);

        var difficulty := CalculateDHDiff(npc, mobile);

        if (SkillCheck(npc, AWARENESS, difficulty) > 0)
            return mobile;
        endif

        sleepms(2);
    endforeach

    return 0;
endfunction

function CanFight(attacker, defender)
    if (!defender || defender.dead || defender.hidden || defender.concealed)
        return 0;
    endif

    if ( GetCooldown(attacker, "peacemade") > 0 )
        return 0;
    endif

    if (GetObjProperty(defender, "noattack"))
        return 0;
    endif

    if (!CheckLineOfSight(attacker, defender))
        return 0;
    endif

    // Cooldown to avoid ress kill
    if (Cint(GetCooldown(defender, "deathtime" )) > 0) 
        // Allow combat if defender initiated it
        if (attacker.opponent == defender) return 1; endif
        // Otherwise prevent attacking during cooldown
        return 0;
    endif

    var npc_elem := NPC_GetNPCConfig(attacker);
	var category := GetConfigString(npc_elem, "Category");

   if (category == "Aklats")
        // Usa função dedicada para detectar ameaças
        var aklat_threat_detected := DetectAklatsThreats(attacker);
        
        // Se não detectou ameaça, não pode atacar
        if (!aklat_threat_detected)
            return 0;
        endif
        
        // Se detectou ameaça, continua com as verificações normais
        // (não retorna 1 imediatamente para permitir outras checagens)
    endif
	
    // Grey NPCs (neutral alignment) should only fight back when attacked
    if (attacker.alignment == 0)
        // Only allow combat if the defender is already attacking this NPC
        if (attacker.opponent == defender || defender.opponent == attacker)
            return 1;
        endif
        return 0;
    endif

    // If defender is a player, normal combat rules apply
    if (!defender.npctemplate)
        return 1;
    endif

    // NPC vs NPC combat only happens if:
    if (defender.npctemplate)
        // Check if forced to attack by GM command
        var forced_target := GetObjProperty(attacker, "#ForcedTarget");
        if (forced_target == defender.serial)
            return 1;
        endif

        if (defender.master)
            return 1;
        endif

        // Check enemy categories
        var attacker_elem := NPC_GetNPCConfig(attacker);
        var defender_elem := NPC_GetNPCConfig(defender);
        var enemy_cats := GetConfigStringArray(attacker_elem, "EnemyCategories");
        
        if (enemy_cats && defender_elem.Category in enemy_cats)
            return 1;
        endif

        return 0; // No valid reason for NPCs to fight
    endif

    return 1;
endfunction

// Pursuit behavior configuration constants
const PURSUIT_PROGRESS_CHECK_INTERVAL := 3;  // seconds between progress checks
const PURSUIT_MAX_NO_PROGRESS_COUNT := 3;    // failed checks before repositioning
const PURSUIT_DISTANCE_TOLERANCE := 2;       // extra tiles before pursuing
const PURSUIT_PATH_CACHE_DURATION := 8;      // seconds to cache pathfinding results
const PURSUIT_TARGET_MOVE_THRESHOLD := 3;    // tiles moved before recalculating path

// Helper: Calculate the ideal engagement distance based on weapon/brain type
function CalculateIdealPursuitDistance()
    var range_info := GetWeaponRangeInfo();

    if (me.isHumanoidBrain())
        if (me.weapon.isRanged() || range_info.max_range > 1)
            return range_info.max_range;
        elseif (me.isSage() && me.weapon.objtype == FIST)
            return 6;
        endif
    else
        // For monsters and animals, check if they're ranged
        if (isMonsterRanged())
            // Ranged monsters should maintain distance
            return 8; // Safe distance for acid spit, web, etc.
        endif
    endif

    return 1; // Default melee distance
endfunction

// Helper: Check if we're already at the desired distance
function IsWithinEngagementRange(opponent, ideal_distance)
    var dist := Distance(me, opponent);

    // No tolerance - NPCs should close to exactly the ideal distance
    // Tolerance was causing NPCs to stop too far away (melee at 3 tiles, etc.)
    return (dist <= ideal_distance);
endfunction

// Helper: Get cached path or calculate new one
function GetOrCreatePursuitPath(opponent)
    var path_cache := GetObjProperty(me, "#pursuit_path_cache");

    // Check if we have a valid cached path
    if (path_cache)
        var cache_age := ReadGameClock() - path_cache.created_time;
        var target_moved := CoordinateDistance(opponent.x, opponent.y, path_cache.target_x, path_cache.target_y);

        // Use cached path if:
        // 1. Cache is fresh (not expired)
        // 2. Target hasn't moved significantly
        // 3. We still have path steps remaining
        if (cache_age < PURSUIT_PATH_CACHE_DURATION &&
            target_moved < PURSUIT_TARGET_MOVE_THRESHOLD &&
            path_cache.path && path_cache.path.size() > 0)
            return path_cache.path;
        endif
    endif

    // Cache invalid or expired - calculate new path
    // Use searchskirt=15 to handle complex paths (L-shapes, long detours, etc.)
    var new_path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES, 20);

    if (new_path)
        // Store new path in cache
        path_cache := struct{
            "path" := new_path,
            "target_x" := opponent.x,
            "target_y" := opponent.y,
            "created_time" := ReadGameClock()
        };
        SetObjProperty(me, "#pursuit_path_cache", path_cache);
    endif

    return new_path;
endfunction

function PursueTarget(opponent)
    var dist := Distance(me, opponent);

    // Early validation check
    if (!shouldKeepPursuing(opponent, dist))
        return 0;
    endif

    // Store last known position whenever we can see the target
    if (CheckLineOfSight(me, opponent))
        var last_loc := struct{
            "x" := opponent.x,
            "y" := opponent.y,
            "z" := opponent.z,
            "time" := ReadGameClock()
        };
        SetObjProperty(me, "#enemy_last_loc", last_loc);
    endif

    // Calculate ideal engagement distance once
    var ideal_distance := CalculateIdealPursuitDistance();

    // Check if we need to retreat (too close for ranged)
    if (dist < ideal_distance)
        // Too close! Back away for ranged creatures
        if (ideal_distance > 1)
            // Ranged creature - run away in a loop until at safe distance
            var should_run := !(me.hidden || me.concealed);
            var retreat_attempts := 0;
            var stuck_retreating := 0;

            while (Distance(me, opponent) < ideal_distance && retreat_attempts < 10)
                if (!shouldKeepPursuing(opponent, Distance(me, opponent)))
                    break;
                endif

                if (!me.opponent && CheckNearbyThreats())
                    opponent := CheckNearbyThreats();
                endif

                var old_x := me.x;
                var old_y := me.y;

                if (should_run)
                    RunAwayFrom(opponent);
                else
                    WalkAwayFrom(opponent);
                endif
                sleepms(100);
                retreat_attempts += 1;

                // Check if we actually moved
                if (me.x == old_x && me.y == old_y)
                    stuck_retreating += 1;

                    // If stuck for 3 attempts, try alternative locations
                    if (stuck_retreating >= 3)
                        var alternative_locations := GetAlternativeLocations(opponent);
                        if (alternative_locations.size() > 0)
                            // Try to move to the furthest location
                            RunTowardLocation(alternative_locations[1].x, alternative_locations[1].y);
                            sleepms(100);
                        endif
                        // Break out if we've tried alternatives and still stuck
                        if (stuck_retreating >= 5)
                            break;
                        endif
                    endif
                else
                    stuck_retreating := 0; // Reset if we moved
                endif
            endwhile
            return 1;
        endif
    endif

    // Check if we're already at the desired distance - if so, we're done pursuing
    if (IsWithinEngagementRange(opponent, ideal_distance))
        return 1;
    endif

    // Execute special abilities at appropriate ranges
    if (dist >= 2 && dist <= 6)
        if (me.canSpitAcid())
            AcidSpray(me, opponent);
        endif

        if (me.canShootWeb())
            WebShot(me, opponent);
        endif
    endif

    if (me.canThrow())
        if (dist >= 2 && dist <= 8)
            ThrowSpecial(me, opponent);
        endif
    endif

    // Determine movement speed based on stealth status
    var should_run := !(me.hidden || me.concealed);

    // Main movement logic - unified section (no more duplicates)
    var use_pathfinding := 0;
    var has_los := CheckLineOfSight(me, opponent);

    // Track if NPC is stuck in same general area over time (handles oscillating movement)
    var stuck_tracker := GetObjProperty(me, "#pursuit_stuck_tracker");
    if (!stuck_tracker)
        stuck_tracker := struct{
            "start_time" := ReadGameClock(),
            "start_pos" := struct{ "x" := me.x, "y" := me.y },
            "start_distance" := dist,
            "target_serial" := 0,
            "failed_moves" := 0,
            "total_moves" := 0,
            "last_check_pos" := struct{ "x" := me.x, "y" := me.y }
        };
    endif

    // Reset tracker if target changed
    if (stuck_tracker.target_serial != opponent.serial)
        stuck_tracker.start_time := ReadGameClock();
        stuck_tracker.target_serial := opponent.serial;
        stuck_tracker.start_pos.x := me.x;
        stuck_tracker.start_pos.y := me.y;
        stuck_tracker.start_distance := dist;
        stuck_tracker.failed_moves := 0;
        stuck_tracker.total_moves := 0;
        stuck_tracker.last_check_pos.x := me.x;
        stuck_tracker.last_check_pos.y := me.y;
    endif

    if (has_los)
        // Direct line of sight - but verify we can actually move toward target
        var movement_attempts := 0;
        var stuck_count := 0;
        var last_dist := dist;

        while (Distance(me, opponent) > ideal_distance && movement_attempts < 10)
            // Re-check if we should keep pursuing
            if (!shouldKeepPursuing(opponent, Distance(me, opponent)))
                break;
            endif

            var old_x := me.x;
            var old_y := me.y;
            var old_dist := Distance(me, opponent);

            if (should_run)
                RunToward(opponent);
            else
                WalkToward(opponent);
            endif
            sleepms(100);
            movement_attempts += 1;

            var new_dist := Distance(me, opponent);
            var moved := (me.x != old_x || me.y != old_y);
            var got_closer := (new_dist < old_dist);

            // Check if we actually made progress (moved AND got closer)
            if (!moved || !got_closer)
                stuck_count += 1;

                // If stuck after 2 attempts, try to find alternative path
                if (stuck_count >= 2)
                    // We have LOS but can't move directly - probably a wall
                    EraseObjProperty(me, "#pursuit_path_cache");

                    // Only use TacticalReposition for ranged creatures
                    // Melee creatures should just switch to pathfinding
                    if (ideal_distance > 1)
                        TacticalReposition(opponent, 1);
                    endif

                    use_pathfinding := 1; // Try pathfinding from current position
                    break;
                endif
            else
                stuck_count := 0; // Reset counter if we made progress
            endif
        endwhile
    endif

    // Check if NPC has been stuck in same area for too long (handles oscillating movement)
    var time_elapsed := ReadGameClock() - stuck_tracker.start_time;
    var distance_from_start := CoordinateDistance(me.x, me.y, stuck_tracker.start_pos.x, stuck_tracker.start_pos.y);
    var distance_improvement := stuck_tracker.start_distance - dist;

    // Track if NPC is actually moving (not bumping into walls)
    var moved_this_cycle := (me.x != stuck_tracker.last_check_pos.x || me.y != stuck_tracker.last_check_pos.y);
    stuck_tracker.total_moves += 1;
    if (!moved_this_cycle)
        stuck_tracker.failed_moves += 1;
    endif
    stuck_tracker.last_check_pos.x := me.x;
    stuck_tracker.last_check_pos.y := me.y;

    // Calculate movement success rate
    var movement_success_rate := 0;
    if (stuck_tracker.total_moves > 0)
        movement_success_rate := CInt((CDbl(stuck_tracker.total_moves - stuck_tracker.failed_moves) / CDbl(stuck_tracker.total_moves)) * 100);
    endif

    // Flee if: been trying 5+ seconds, in same area (<=6 tiles), AND failing to move (< 30% success rate)
    // This distinguishes between stuck-on-wall vs successfully-kiting-in-circles
    if (time_elapsed >= 5 && distance_from_start <= 6 && movement_success_rate < 30)
        EraseObjProperty(me, "#pursuit_stuck_tracker");
        EraseObjProperty(me, "#pursuit_path_cache");

        // Use existing Flee() function - runs away and sets cooldown to prevent re-engagement
        Flee(opponent, 100, 1);  // 100ms sleep, change_opponent = 1 (can switch targets)
        return 0; // Abandon pursuit - target is unreachable
    endif

    // Reset timer if made significant progress (moved far from start or got much closer)
    if (distance_from_start > 8 || distance_improvement > 3)
        stuck_tracker.start_time := ReadGameClock();
        stuck_tracker.start_pos.x := me.x;
        stuck_tracker.start_pos.y := me.y;
        stuck_tracker.start_distance := dist;
        stuck_tracker.failed_moves := 0;
        stuck_tracker.total_moves := 0;
    endif

    // Save stuck tracker
    SetObjProperty(me, "#pursuit_stuck_tracker", stuck_tracker);

    // Use pathfinding if: no LOS OR stuck with LOS
    if (!has_los || use_pathfinding)
        // Use cached pathfinding
        var path := GetOrCreatePursuitPath(opponent);
        if (path)
            var path_step := 0;
            foreach coord in path
                path_step += 1;
                // Re-check distance in case target moved
                if (Distance(me, opponent) <= ideal_distance)
                    break;
                endif

                // Re-check if we should keep pursuing
                if (!shouldKeepPursuing(opponent, Distance(me, opponent)))
                    break;
                endif

                // Move to next waypoint
                if (should_run)
                    RunTowardLocation(coord.x, coord.y);
                else
                    WalkTowardLocation(coord.x, coord.y);
                endif
                sleepms(100);
            endforeach
        else
            // Path failed - try to move to last known position
            var last_loc := GetObjProperty(me, "#enemy_last_loc");
            if (last_loc)
                var time_since := ReadGameClock() - last_loc.time;

                // Only use last known position if it's recent (within 30 seconds)
                if (time_since < 30)

                    // Try to path to last known position
                    var last_loc_path := FindPath(me.x, me.y, me.z, last_loc.x, last_loc.y, last_loc.z, me.realm, FP_IGNORE_MOBILES, 20);
                    if (last_loc_path)
                        foreach coord in last_loc_path
                            if (CheckLineOfSight(me, opponent))
                                break; // Found the target again!
                            endif

                            if (should_run)
                                RunTowardLocation(coord.x, coord.y);
                            else
                                WalkTowardLocation(coord.x, coord.y);
                            endif
                            sleepms(100);
                        endforeach
                    endif
                endif
            endif

            // Check if it's due to height difference
            var z_diff := Abs(me.z - opponent.z);

            if (z_diff >= 19)
                // Significant height difference - try to find stairs/ramps by moving toward target
                // Move in the general direction even without a perfect path
                var attempts := 0;
                while (attempts < 5 && !CheckLineOfSight(me, opponent))
                    if (should_run)
                        RunToward(opponent);
                    else
                        WalkToward(opponent);
                    endif
                    sleepms(200);
                    attempts += 1;

                    // Check if we found a path now
                    if (CheckLineOfSight(me, opponent))
                        break;
                    endif
                endwhile
            endif

            // Try tunnel tactics if available
            if (me.hasTunnelTactics())
                TunnelTactics(me, opponent);
            endif
        endif
    endif

    // Track progress towards target (moved to end, after movement attempt)
    var progress := GetObjProperty(me, "#pursue_progress");
    if (!progress)
        progress := struct{
            "last_dist" := dist,
            "last_pos" := struct{ "x" := me.x, "y" := me.y },
            "last_check_time" := ReadGameClock(),
            "no_progress_count" := 0,
            "start_time" := ReadGameClock()
        };
    endif

    // Only check progress every PURSUIT_PROGRESS_CHECK_INTERVAL seconds
    if (ReadGameClock() >= progress.last_check_time + PURSUIT_PROGRESS_CHECK_INTERVAL)
        var current_dist := Distance(me, opponent);
        var moved := (me.x != progress.last_pos.x || me.y != progress.last_pos.y);
        var got_closer := (current_dist < progress.last_dist);

        // No progress if we haven't moved OR haven't gotten closer
        if (!moved || !got_closer)
            progress.no_progress_count += 1;
        else
            progress.no_progress_count := 0;
        endif

        // Update tracking info
        progress.last_dist := current_dist;
        progress.last_pos.x := me.x;
        progress.last_pos.y := me.y;
        progress.last_check_time := ReadGameClock();

        // If we're not making progress after PURSUIT_MAX_NO_PROGRESS_COUNT checks, force a reposition
        if (progress.no_progress_count >= PURSUIT_MAX_NO_PROGRESS_COUNT)
            EraseObjProperty(me, "path");
            EraseObjProperty(me, "#enemy_last_loc");
            EraseObjProperty(me, "#pursuit_path_cache"); // Clear cached path when stuck

            // Only use TacticalReposition for ranged creatures
            // Melee creatures should just retry with pathfinding instead
            if (ideal_distance > 1)
                TacticalReposition(opponent, 1);
            endif

            progress.no_progress_count := 0;
        endif
    endif

    SetObjProperty(me, "#pursue_progress", progress);

    return 1;
endfunction

function EnableMainEvents()
    DisableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED);
    EnableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA, AREA_SIZE);
    EnableEvents(SYSEVENT_DISENGAGED + SYSEVENT_LEFTAREA, AREA_SIZE);
endfunction

function DisableMainEvents()
    DisableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA);
    DisableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED);
endfunction

function HandleIdleState()
    DisableMainEvents();
    EnableEvents(SYSEVENT_ENTEREDAREA + SYSEVENT_ENGAGED + SYSEVENT_DAMAGED, AREA_SIZE);
    
    // Return to spawn point if we have one
    var spawn_serial := GetObjProperty(me, "spawnpoint");
    var spawn_point := 0;
    if (spawn_serial)
        spawn_point := SystemFindObjectBySerial(spawn_serial);
    endif

    var anchor := GetObjProperty(me, "anchor");
    var home_x, home_y;

    if (spawn_point)
        home_x := spawn_point.x;
        home_y := spawn_point.y;
    elseif (anchor)
        home_x := anchor[1];
        home_y := anchor[2];
    else
        return;
    endif

    if (!hasPlayerNear(me))
        var attempts := 0;
        while (CoordinateDistance(me.x, me.y, home_x, home_y) > 5 && attempts < 20)
            if (CheckNearbyThreats())
                return; // Abort returning home if threats are nearby
            endif
            WalkTowardLocation(home_x, home_y);
            sleepms(100);
            attempts += 1;
        endwhile
        
        // If still not home after 20 attempts, just teleport
        if (CoordinateDistance(me.x, me.y, home_x, home_y) > 5)
            PrintTextAbove(me, "*Retorna correndo* [Sistema Anti-Exploits]");
            sleep(3);
            MoveObjectToLocation(me, home_x, home_y, anchor[3], me.realm, MOVEOBJECT_FORCELOCATION);
        endif
    endif
    
    return 1;
endfunction

function FleeFrom(opponent)
    if (!opponent)
        return 0;
    endif

    var runs := 0;
    while (opponent && Distance(me, opponent) < 16 && runs < 20)
        TryToRunAway(opponent);
        if(!AP_ConsumeVital(me, STAMINA, 5))
            PrintTextAbove(me, "*Exausto*");
            break;
        endif
        runs += 1;
        sleepms(100);
    endwhile
endfunction

function MaintainRange(opponent)
    var sleepdelay := 275 - me.run_speed;
    if (sleepdelay < 0)
        sleepdelay := 50;
    endif

    var dist := Distance(me, opponent);
    var z_diff := Abs(me.z - opponent.z);

    if (z_diff > MAX_Z_DIFF)
        return 0;
    endif

    var range_info := GetWeaponRangeInfo();
    
    if (me.isHumanoidBrain())
        return MaintainHumanoidDistance(opponent, dist, sleepdelay, range_info);
    endif

    return MaintainSimpleDistance(opponent, dist, sleepdelay);
endfunction

function MaintainHumanoidDistance(opponent, dist, sleepdelay, range_info)
    if (isFireWeapon(me.weapon))
        var balas := CInt(GetObjProperty(me.weapon, "balas"));
        if (balas <= 0)
            UseItem(me.weapon, me);
        endif
    endif

    if (GetObjProperty(me, "metamorfose"))
        // Force melee behavior
        if (dist > 1)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return StayNearAllies(4, sleepdelay, opponent);
        endif
        return 0;
    endif

    if (me.weapon.isRanged() || range_info.max_range > 1)
        // Check if we can actually move to ideal range
        var ideal_range := range_info.max_range;
        var can_move_back := CanMoveInDirection(me, opponent, 1); // 1 means moving away
        var can_move_forward := CanMoveInDirection(me, opponent, 0); // 0 means moving toward

        if (dist < ideal_range)
            if (can_move_back)
                TryToRunAway(opponent);
                sleepms(sleepdelay);
            else
                // If we can't move back, just maintain current position
                TurnToward(opponent);
                return StayNearAllies(4, sleepdelay, opponent);
            endif
        elseif (dist > ideal_range)
            if (can_move_forward)
                if (GetObjProperty(me, "metamorfose"))
                    RunToward(opponent);
                    sleepms(sleepdelay);
                else
                    RunToward(opponent);
                    sleepms(sleepdelay);
                endif
            else
                // If we can't move forward, just maintain current position
                TurnToward(opponent);
                return StayNearAllies(4, sleepdelay, opponent);
            endif
        else
            TurnToward(opponent);
            return StayNearAllies(4, sleepdelay, opponent);
        endif
    elseif (me.isSage() && me.weapon.objtype == FIST)
        if (dist < 6)
            TryToRunAway(opponent);
            sleepms(sleepdelay);
        elseif (dist > 6)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return StayNearAllies(4, sleepdelay, opponent);
        endif
    else
        if (dist > range_info.max_range)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return StayNearAllies(4, sleepdelay, opponent);
        endif
    endif

    return 0;
endfunction

function CanMoveInDirection(me, opponent, moving_away := 0)
    var test_distance := 3; // Check 3 tiles ahead
    var angle := GetFacing(me.x, me.y, opponent.x, opponent.y);
    var test_coords;
    
    if (moving_away)
        // Reverse the angle when moving away
        angle := (angle + 4) % 8;
    endif
    
    case (angle)
        0: test_coords := struct{ "x" := me.x, "y" := me.y - test_distance }; // North
        1: test_coords := struct{ "x" := me.x + test_distance, "y" := me.y - test_distance }; // Northeast
        2: test_coords := struct{ "x" := me.x + test_distance, "y" := me.y }; // East
        3: test_coords := struct{ "x" := me.x + test_distance, "y" := me.y + test_distance }; // Southeast
        4: test_coords := struct{ "x" := me.x, "y" := me.y + test_distance }; // South
        5: test_coords := struct{ "x" := me.x - test_distance, "y" := me.y + test_distance }; // Southwest
        6: test_coords := struct{ "x" := me.x - test_distance, "y" := me.y }; // West
        7: test_coords := struct{ "x" := me.x - test_distance, "y" := me.y - test_distance }; // Northwest
    endcase
    
    // Check if we can actually move there
    return CheckLosAt(me, test_coords.x, test_coords.y, me.z);
endfunction

function MaintainSimpleDistance(opponent, dist, sleepdelay)  
    if (isMonsterRanged())
        // Simple ranged monster logic
        if (dist < 4)
            TryToRunAway(opponent);
            sleepms(sleepdelay);
        elseif (dist > 10)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return 1;
        endif
    else
        // Simple melee monster logic
        if (dist > 1)
            RunToward(opponent);
            sleepms(sleepdelay);
        else
            TurnToward(opponent);
            return 1;
        endif
    endif

    return 0;
endfunction

function DoThiefCombatBehavior(opponent)
    if (Distance(me, opponent) < 6)
        return 0;
    endif

    // 50% chance to steal, 50% chance to sneak attack
    if (RandomInt(100) < 50)
        DoThiefSteal(opponent);
    else
        DoThiefSneakAttack(opponent);
    endif
endfunction

function ShamanBehavior(shaman)
    // First priority: Check allies health
    if (GetCooldown(shaman, "heal_check"))
        return 0;
    endif

    // Get all allies nearby
    var wounded_allies := array{};

    // Check allies' health
    wounded_allies := CheckForWoundedAllies(shaman);

    if (wounded_allies.size() == 0)
        DoHarmfulSpell();
    endif

    // If multiple allies need help, cast healing rain
    if (wounded_allies.size() >= 3 && !GetCooldown(shaman, "healing_rain"))
        PerformHealingRain(shaman, wounded_allies);
        SetCooldown(shaman, "healing_rain", 45);
        return 1;
    endif

    var most_wounded := FindMostWoundedAlly(wounded_allies);
    if (most_wounded)
        PerformHeal(shaman, most_wounded, 1);
        SetCooldown(shaman, "heal_check", 15);
        return 1;
    endif

    return 0;
endfunction

function CheckForWoundedAllies(who)
	var allies := NPC_GetNearbyAllies(who);
	var wounded_allies := array{};

	// Check allies' health
	foreach ally in allies
		if (isHurt(ally)) // Is Hurt
			wounded_allies.append(ally);
		endif
	endforeach

	return wounded_allies;
endfunction

function FindMostWoundedAlly(wounded_allies)
    // Initialize our best candidate with a full 100% health
    var most_wounded := struct{
        "mobile" := 0,
        "hp_percent" := 100.0
    };

    foreach ally in wounded_allies
        // Calculate current HP percentage for each ally
        var hp_percent := (CDbl(AP_GetVital(ally, HITS)) / CDbl(AP_GetVitalMaximumValue(ally, HITS))) * 100;
        // Update if this ally has a lower (worse) percentage
        if (hp_percent < most_wounded.hp_percent)
            most_wounded.mobile := ally;
            most_wounded.hp_percent := hp_percent;
        endif
        sleepms(2);
    endforeach

    return most_wounded.mobile;
endfunction

function AskForHealing(me)
    // First look specifically for shamans
    if (!isHurt(me))
        return 0;
    endif

    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 12, me.realm)
        if (mobile.npctemplate && mobile != me)
            if (mobile.isSage())
                SetObjProperty(mobile, "#healing_needed", me.serial);
                SendEvent(mobile, struct{ "type" := EVID_BUDDYATTACKED, "source" := me });
            endif
        endif
        sleepms(2);
    endforeach
endfunction 

function shouldKeepPursuing(opponent, dist)
    if (!opponent)
        return 0;
    endif

    // Don't chase too far
    if (dist > AREA_SIZE + 6)
        return 0;
    endif

    // If we're in melee range and can see opponent, stop pursuing (only for melee)
    // Ranged creatures need to keep pursuing to retreat away
    var ideal_distance := CalculateIdealPursuitDistance();
    if (dist <= 1 && ideal_distance == 1 && CheckLineOfSight(me, opponent))
        return 0;
    endif

    // Prevent flee spam by adding a cooldown
    if (GetCooldown(me, "flee_check"))
        return 1; // Continue pursuing if on cooldown
    endif
    var npcState := getNPCState(me);
    var cd := 15;
    
    if (npcState == "dying")
        cd := 5;
    endif

    SetCooldown(me, "flee_check", cd);
    var allies := NPC_GetNearbyAllies(me);
    if (allies.size() > 0)
        if (!GetCooldown(me, "allies"))
            SetObjProperty(me, "initial_allies", allies.size());
            SetCooldown(me, "allies", 15);
        endif

        var initial_allies := GetObjProperty(me, "initial_allies");
        var already_fighting := GetObjProperty(me, "already_fighting");
        
        // Only consider fleeing if we've lost allies
        if (initial_allies && allies.size() <= CInt(initial_allies/2))
            // Don't flee if already engaged unless heavily outnumbered
            if (!already_fighting || ListHostiles(me, 3).size() >= allies.size() * 2)
                // Higher chance to flee when heavily damaged
                var flee_chance := (npcState == "dying") ? 40 : 15;
                
                if (RandomInt(100) < flee_chance)
                    PrintTextAbove(me, "*"+me.name+" recua*");
                    FleeFrom(opponent);
                    return 0;  // Added return 0 after fleeing
                endif
            endif
        endif
    else
        // If alone, don't pursue new targets but stay if already fighting
        if (!GetObjProperty(me, "already_fighting"))
            return 0;
        endif
    endif

    return 1;  // Continue pursuing if none of the above conditions are met
endfunction

function LootCorpse(corpse)
    if (!CorpseHasLoot(corpse))
        return 0;
    endif

    if (GetCooldown(me, "loot_cooldown") > 0)
        return 0;
    endif

    // Approach the corpse, but check for threats while approaching
    while (Distance(me, corpse) > 1)
        // Check for nearby hostiles - combat takes priority over looting
        foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL, me.realm)
            if (CanFight(me, mobile))
                // Hostile nearby - abort looting and engage
                return 0;
            endif
            sleepms(20);
        endforeach

        RunToward(corpse);
        sleepms(100);

        // Break if corpse disappeared or too far
        if (!corpse || Distance(me, corpse) > 20)
            return 0;
        endif
    endwhile

    // Calculate max items to loot based on intelligence
    var max_loot := 1/* CInt(AP_GetStat(me, INTELLIGENCE)/20) */;
    if (max_loot < 1)
        max_loot := 1;
    endif
    var looted := 0;

    // Ensure NPC has a backpack
    if (!me.backpack)
        var backpack := CreateItemAtLocation(me.x, me.y, me.z, "Backpack", 1);
        if (!EquipItem(me, backpack))
            return 0;
        endif
    endif

    // Loot items, but check for threats between each item
    while (looted < max_loot)
        // Check for nearby hostiles before looting each item
        foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL, me.realm)
            if (CanFight(me, mobile))
                // Hostile nearby - abort looting and let the brain handle combat
                return (looted > 0);
            endif
            sleepms(2);
        endforeach

        PrintTextAbove(me, "*revira o corpo*");
        if (!LootSomething(me, corpse))
            break;
        endif
        looted += 1;
        sleepms(1500);
    endwhile

    return (looted > 0);
endfunction

function CorpseHasLoot(corpse)
    if (!corpse || corpse.IsA(POLCLASS_CORPSE) == error)
        return 0;
    endif

    foreach item in EnumerateItemsInContainer(corpse)
        if (item.movable)
            return 1;
        endif
        sleepms(2);
    endforeach

    return 0;
endfunction

function LootSomething(npc, corpse)
    // Define priority items by objtype (coins)
    var coins := array{0xBA63, 0xBA64, 0xBA65}; // copper, silver, gold
    var resources := array{0x1bf2, 0x1bd7};     // ingots, wood

    if (GetCooldown(npc, "already_looted") > 0)
        return 0;
    endif

    // First try to loot coins
    var priority_loot := FindPriorityLoot(corpse, coins);
    if (priority_loot)
        return LootItem(npc, priority_loot);
    endif

    // Then try resources
    var resource_loot := FindPriorityLoot(corpse, resources);
    if (resource_loot)
        return LootItem(npc, resource_loot);
    endif

    // Then look for equipment by class
    foreach item in EnumerateItemsInContainer(corpse)
        if (!item.movable || item.newbie)
            continue;
        endif

        if (CInt(GetObjProperty(item, "#EquippedOn")) == corpse.serial && item.IsA(POLCLASS_ARMOR))
            continue;
        endif

        if (item.IsA(POLCLASS_WEAPON) || item.IsShield() || item.IsA(POLCLASS_ARMOR))
            return LootItem(npc, item);
        endif

        // Check containers
        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindEquipmentInContainer(item);
            if (container_item)
                return LootItem(npc, container_item);
            endif
        endif
        
        // Adicionei pra meter o bicho em alguma coisa pelo menos (bandages, tesoura)
        return LootItem(npc, item);
        sleepms(2);
    endforeach

    return 1;
endfunction

function FindEquipmentInContainer(container)
    foreach item in EnumerateItemsInContainer(container)
        if (!item.movable)
            continue;
        endif

        if (item.IsA(POLCLASS_WEAPON) || item.IsShield() || item.IsA(POLCLASS_ARMOR))
            return item;
        endif

        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindEquipmentInContainer(item);
            if (container_item)
                return container_item;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
endfunction

function LootItem(npc, item)
    if (!item)
        return 0;
    endif

    if (!npc.backpack)
        var backpack := CreateItemAtLocation(npc.x, npc.y, npc.z, "Backpack", 1);
        EquipItem(npc, backpack);
    endif
    SetObjProperty(item, "#LootedBy", npc.serial);

    PrintTextAbove(npc, "*pega " + item.desc + "*");
    SetCooldown(npc, "already_looted", 300);

    return MoveItemToContainer(item, npc.backpack);
endfunction

function FindPriorityLoot(corpse, graphics_array)
    foreach item in EnumerateItemsInContainer(corpse)
        if (!item.movable)
            continue;
        endif

        if (item.objtype in graphics_array || item.graphic in graphics_array)
            return item;
        endif

        // Check containers for these items
        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindPriorityLoot(item, graphics_array);
            if (container_item)
                return container_item;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
endfunction

function ClearCombatState()
    SetWarMode(0);
    SetOpponent(0);
endfunction

function reachForMeleeWeapon(opponent)
    // Switch to melee weapon when enemy gets close
    if (Distance(me, opponent) <= 2)
        var backup_weapon := 0;
        foreach item in (EnumerateItemsInContainer(me.backpack))
            if (item.isa(POLCLASS_WEAPON) && !item.isRanged())
                backup_weapon := item;
                break;
            endif
            sleepms(2);
        endforeach
    endif
endfunction

function GetWeaponRangeInfo()
    // Get the actual equipped item in hand
    var weapon := GetEquipmentByLayer(me, LAYER_HAND1);
    if (!weapon)
        weapon := GetEquipmentByLayer(me, LAYER_HAND2);
    endif

    if (!weapon)
        return struct{ "min_range" := 1, "max_range" := 1 };
    endif

    var wpn_elem := GetWpnCfgInfo(me, weapon);
    var min_range := 1;
    var max_range := 1;
    
    // Handle fireweapons specifically
    var fire_weapons := array{0xC5FD, 0xC5FE, 0xC5FF, 0xC600, 0x89A7};
    if (weapon.objtype in fire_weapons)
        case (weapon.objtype)
            0xC5FD: // Pistol
            0xC5FE:
                min_range := 1;
                max_range := 6;
            0xC5FF: // Hand Cannon
                min_range := 2;
                max_range := 6;
            0xC600: // Carabine
                min_range := 3;
                max_range := 12;
            0x89A7: // Musket
                min_range := 4;
                max_range := 20;
        endcase
    else
        // For all other weapons, get their specific ranges from config
        min_range := CInt(wpn_elem.MinRange) ? CInt(wpn_elem.MinRange) : 1;
        max_range := CInt(wpn_elem.MaxRange) ? CInt(wpn_elem.MaxRange) : 1;
    endif

    return struct{
        "min_range" := min_range,
        "max_range" := max_range
    };
endfunction

function StayNearAllies(max_distance, sleepdelay, opponent)
    var allies := NPC_GetNearbyAllies(me);
    if (allies.size() > 0)
        var closest_ally := allies[1];
        var ally_dist := Distance(me, closest_ally);
        
        if (ally_dist > max_distance)
            RunToward(closest_ally);
            sleepms(sleepdelay);
            return 0;
        endif
    endif
    
    TurnToward(opponent);
    return 1;
endfunction

function isMonsterRanged()
    // Check for doppelganger override first
    if (GetObjProperty(me, "#force_ranged"))
        return 1;
    endif

    // Get monster configuration
    var npc_elem := NPC_GetNPCConfig(me);
    var speed_type := SplitWords(GetConfigString(npc_elem, "EpicAttackSpeed"), "_");
    var is_ranged := (speed_type[1] == "RANGED") || me.isSage();

    return is_ranged;
endfunction

function TryToRunAway(opponent, chasing := 0)
    var progress := GetObjProperty(me, "#runaway_progress");
    if (!progress)
        progress := struct{
            "last_pos" := struct{ "x" := me.x, "y" := me.y },
            "last_check_time" := ReadGameClock(),
            "no_progress_count" := 0
        };
    endif

    // Run away from opponent
    if (!chasing)
        RunAwayFrom(opponent);
    endif

    // Check progress every 1 seconds
    if (ReadGameClock() >= progress.last_check_time + 1)
        // Check if we're stuck in roughly the same area (2-3 tiles)
        var current_dist := CoordinateDistance(me.x, me.y, progress.last_pos.x, progress.last_pos.y);
        if (current_dist <= 3)
            progress.no_progress_count += 1;
        else
            progress.no_progress_count := 0;
        endif

        // If stuck for multiple checks, find alternative path
        if (progress.no_progress_count >= 1)
            var alternative_locations := GetAlternativeLocations(opponent);
            if (alternative_locations.size() > 0)
                RunTowardLocation(alternative_locations[1].x, alternative_locations[1].y);
            endif
            progress.no_progress_count := 0;
        endif

        // Update tracking info
        progress.last_pos.x := me.x;
        progress.last_pos.y := me.y;
        progress.last_check_time := ReadGameClock();
    endif

    SetObjProperty(me, "#runaway_progress", progress);
endfunction

/*
 * CheckHealthTeleport(mobile)
 * Checks if mobile should teleport based on current health percentage
 * Returns: 1 if teleported, 0 if not
 */
function CheckHealthTeleport(mobile)
    var teleport_locations := GetObjProperty(mobile, "teleport_locations");
    
    if (!teleport_locations || teleport_locations.size() < 1)
        return 0;
    endif
    
    var max_hp := AP_GetVitalMaximumValue(mobile, HITS);
    var current_hp_percent := CInt((CDbl(AP_GetVital(mobile, HITS)) / CDbl(max_hp)) * 100);
    
    // Find best matching location (highest hp_percent threshold that's <= current_hp_percent)
    var best_location := 0;
    var best_index := -1;
    var i := 1;  // Start from 1 since POL arrays are 1-indexed
    
    foreach location in teleport_locations
        if (current_hp_percent <= location.hp_percent)
            if (!best_location || location.hp_percent > best_location.hp_percent)
                best_location := location;
                best_index := i;
            endif
        endif
        i := i + 1;
    endforeach
    
    if (best_index >= 0)
        // Teleport the mobile to the best location
        if (MoveObjectToLocation(mobile, best_location.x, best_location.y, best_location.z, best_location.realm, MOVEOBJECT_FORCELOCATION))
            PlaySoundEffect(mobile, 0x1FE);
            PlayObjectCenteredEffect(mobile, 0x3728, 7, 0x10);
            
            // Remove this location from the array so it won't be used again
            teleport_locations.erase(best_index);
            SetObjProperty(mobile, "teleport_locations", teleport_locations);
            return 1;
        else
            return 0;
        endif
    endif
    
    return 0;
endfunction

function ShouldFlee(me)
    // should flee when life is below X%
    var flee_health := CInt(GetObjProperty(me, "flee_health"));
    if (flee_health)
        var max_hp := AP_GetVitalMaximumValue(me, HITS);
        var current_hp_percent := CInt((CDbl(AP_GetVital(me, HITS)) / CDbl(max_hp)) * 100);
        if (current_hp_percent <= flee_health)
            return 1;
        endif
    endif
     // if mob is neutral alignment, alaways flee
    if (GetAlignment(me) == ALIGN_NEUTRAL)
        return 1;
    endif

    return 0;
endfunction

function HandleInitializationLoot(mobile, boss_struct)   
    if (!boss_struct)
        return 0;
    endif
    
    if (!boss_struct.loot || boss_struct.loot.size() < 1)
        return 0;
    endif
    
    foreach loot_item in (boss_struct.loot)       
        if (!loot_item.on_death) // Only process initialization items
            var roll := RandomInt(100);
            
            if (roll < loot_item.drop_chance)
                // Create item directly with the template
                var item := CreateIteminBackpackFromTemplate(mobile, loot_item.template);
                if (item)
                    item.movable := 1;
                endif
            endif
        endif

        sleepms(2);
    endforeach

    return 1;
endfunction

function startPatrolling(byref me, patrol_points, current_patrol_point)
    var patrol_direction := GetObjProperty(me, "#patrol_direction");
    if (patrol_direction == error)
        patrol_direction := 1; // 1 = forward, -1 = backward
        SetObjProperty(me, "#patrol_direction", patrol_direction);
    endif
    
    var dest := patrol_points[current_patrol_point];
    
    // Check if we've reached the destination (within 1 tile)
    if (CoordinateDistance(me.x, me.y, dest.x, dest.y) <= 1)
        // Determine next patrol point based on direction
        if (patrol_direction == 1)
            current_patrol_point += 1;
            // If reached the end, reverse direction
            if (current_patrol_point > patrol_points.size())
                current_patrol_point := patrol_points.size() - 1;
                patrol_direction := -1;
                SetObjProperty(me, "#patrol_direction", patrol_direction);
            endif
        else // patrol_direction == -1
            current_patrol_point -= 1;
            // If reached the beginning, reverse direction
            if (current_patrol_point < 1)
                current_patrol_point := 2;
                patrol_direction := 1;
                SetObjProperty(me, "#patrol_direction", patrol_direction);
            endif
        endif
        SetObjProperty(me, "#current_patrol_point", current_patrol_point);
    endif

    // Walk toward destination
    WalkTowardLocation(dest.x, dest.y);
endfunction

function SetupNPCAppearence(mobile)
    // Get boss data to check for appearance settings
    var boss := GetObjProperty(mobile, "EpicBossStruct");
    if (mobile.gender == 1)
        var existing_beard := GetEquipmentByLayer(mobile, LAYER_BEARD);
        if (existing_beard)
            DestroyItem(existing_beard);
        endif
    endif
    
    if (!boss)
        return 0;
    endif
    
    if (!boss.appearance)
        return 0;
    endif
    
    var appearance := boss.appearance;
    // Setup hair if specified
    if (appearance.hair_style && appearance.hair_style != 0)
        // Check if hair color is specified
        // Remove existing hair first
        var existing_hair := GetEquipmentByLayer(mobile, LAYER_HAIR);
        if (existing_hair)
            DestroyItem(existing_hair);
        endif
        
        // Create and equip new hair using proper objtype
        var hair_item := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, CInt(appearance.hair_style), 1, mobile.realm);
        if (hair_item)
            if (appearance.hair_color)
                hair_item.color := CInt(appearance.hair_color);
            endif
            if (!EquipItem(mobile, hair_item))
                DestroyItem(hair_item);
            endif
        endif
    endif
    
    // Setup beard if specified
    if (appearance.beard_style && appearance.beard_style != 0)
        // Remove existing beard first
        var existing_beard := GetEquipmentByLayer(mobile, LAYER_BEARD);
        if (existing_beard)
            DestroyItem(existing_beard);
        endif
        
        // Create and equip new beard using proper objtype
        var beard_item := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, CInt(appearance.beard_style), 1, mobile.realm);
        if (beard_item)
            if (appearance.beard_color)
                beard_item.color := CInt(appearance.beard_color);
            endif
            if (!EquipItem(mobile, beard_item))
                DestroyItem(beard_item);
            endif
        endif
    endif
    
    // Setup skin color if specified
    if (appearance.skin_color)
        mobile.color := CInt(appearance.skin_color);
        mobile.truecolor := CInt(appearance.skin_color);
    endif
    
    return 1;
endfunction

// New function to apply boss colors to clothing only
function ApplyBossColors(mobile, color_ranges)    
    if (!color_ranges || color_ranges.size() == 0)
        return 0;
    endif
    
    // Define clothing layers (not armor or weapons)
    var clothing_layers := array{
        LAYER_SHIRT,      // Shirt
        LAYER_PANTS,      // Pants
        LAYER_SHOES,      // Shoes
        LAYER_ROBE,       // Robe
        LAYER_CAPE,       // Cloak/Cape
        LAYER_SKIRT,      // Skirt
        LAYER_TUNIC       // Tunic/Sash
    };
    
    // Define armor layers that we DON'T want to color
    var armor_layers := array{
        LAYER_HELM,       // Helmet
        LAYER_GLOVES,     // Gloves
        LAYER_CHEST,      // Chest armor
        LAYER_ARMS,       // Arm armor
        LAYER_LEGS        // Leg armor
    };
    
    var items_colored := 0;
    
    // Apply color to clothing items that have color 0 (uncolored)
    foreach layer in clothing_layers
        var item := GetEquipmentByLayer(mobile, layer);
        if (item)            
            if (item.color == 0)
                // Skip actual armor layers (not clothing that happens to be POLCLASS_ARMOR)
                if (layer in armor_layers)
                    continue;
                endif
                
                // Get a random color from the selected ranges for each item
                var selected_color := GetRandomColorFromRanges(color_ranges);
                
                item.color := selected_color;
                item.truecolor := selected_color;
                items_colored += 1;
            endif
        endif
        sleepms(2);
    endforeach
        
    return 1;
endfunction

// Function to get a random color value from the selected color ranges
function GetRandomColorFromRanges(color_ranges)
    if (!color_ranges || color_ranges.size() == 0)
        return 0; // Default color
    endif
    
    // Pick a random color range
    var selected_range := color_ranges[RandomInt(color_ranges.size()) + 1];
    
    // Return a color value based on the selected range (using the same logic as cor.src)
    return GetColorValue(selected_range);
endfunction

// Copy the GetColorValue function from cor.src for consistency
function GetColorValue(color)
    case (color)
        "Rosa":      return RandomIntMinMax(1201, 1254);
        "Vermelho":  return RandomIntMinMax(1601, 1654);
        "Azul":      return RandomIntMinMax(1301, 1354);
        "Verde":     return RandomIntMinMax(1401, 1454);
        "Amarelo":   return RandomIntMinMax(1701, 1754);
        "Laranja":   return RandomIntMinMax(1501, 1554);
        "Neutro":    return RandomIntMinMax(1801, 1908);
        "Cinza":     return RandomInt({0, 946, 947});
        AIGLANA:     return RandomColorFromRanges({
                        RandomIntMinMax(1601, 1654),  // Vermelho
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        HAROLOTH:    return RandomColorFromRanges({
                        RandomIntMinMax(1201, 1254),  // Rosa
                        RandomInt({0, 946, 947}),     // Cinza
                        RandomIntMinMax(1301, 1354),  // Azul
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        LESTE:       return RandomColorFromRanges({
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1501, 1554),  // Laranja
                        RandomIntMinMax(1201, 1254),  // Rosa
                        RandomInt({0, 946, 947})      // Cinza
                     });
        BJORSKA:     return RandomColorFromRanges({
                        RandomIntMinMax(1401, 1454),  // Verde
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        ALURIEL:     return RandomColorFromRanges({
                        RandomIntMinMax(1201, 1254),  // Rosa
                        RandomIntMinMax(1301, 1354),  // Azul
                        RandomIntMinMax(1401, 1454),  // Verde
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        BADURAN:     return RandomColorFromRanges({
                        RandomIntMinMax(1501, 1554),  // Laranja
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1801, 1908),  // Neutro
                        RandomInt({0, 946, 947})      // Cinza
                     });
        GULTHRAK:    return RandomColorFromRanges({
                        RandomIntMinMax(1201, 1254),  // Rosa
                        RandomIntMinMax(1601, 1654),  // Vermelho
                        RandomIntMinMax(1801, 1908),  // Neutro
                        RandomInt({0, 946, 947})      // Cinza
                     });
        POLKINEA:    return RandomColorFromRanges({
                        RandomIntMinMax(1501, 1554),  // Laranja
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1401, 1454),  // Verde
                        RandomIntMinMax(1301, 1354),  // Azul
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        default:     return color; // In case color is already a numeric value
    endcase
endfunction

function RandomColorFromRanges(colorRanges)
    return colorRanges[RandomInt(colorRanges.size()) + 1];
endfunction

// Função específica para detecção dos Aklats
function DetectAklatsThreats(npc)
    // Verifica se já tem oponente ativo
    if (npc.opponent)
        return 1;
    endif
    
    // Verifica se foi alertado por som (taunt_opponent)
    var taunt_opponent := GetObjProperty(npc, "#taunt_opponent");
    if (taunt_opponent)
        PrintTextAbove(npc, "*Alertado pelo som!*");
        return 1;
    endif
    
    // Detecção de pegadas alinhadas
    var pegadas := {0x1E03, 0x1E04, 0x1E05, 0x1E06}; 
    var footsteps := array;
    
    // Coleta todas as pegadas na área
    foreach objtype in pegadas
        foreach pegada in ListItemsNearLocationOfType(npc.x, npc.y, npc.z, 10, objtype, npc.realm)
            footsteps.append(pegada);
        endforeach
        sleepms(2);
    endforeach
    
    // Verifica se há pegadas suficientes e alinhadas
    if (footsteps.size() >= 3 && CheckAlinhadas(footsteps))
        PrintTextAbove(npc, "*Alertado pelas vibrações!*");
        return 1;
    endif
    
    return 0;
endfunction

//FUNCÕES PARA AKLATS E PEGADAS PARA ACIONAR O ATAQUE
// Função para verificar se 3 pegadas estão alinhadas (em linha reta)
function CheckAlinhadas(footsteps)
    if (footsteps.size() < 3)
        return 0; // Não há pegadas suficientes
    endif
    
    // Pega as 3 pegadas mais recentes (assumindo que são as últimas na lista)
    var step1 := footsteps[footsteps.size() - 3];
    var step2 := footsteps[footsteps.size() - 2];
    var step3 := footsteps[footsteps.size() - 1];
    
    // Verifica se os 3 pontos estão alinhados
    return ArePointsAligned(step1, step2, step3);
endfunction

// Função para verificar se 3 pontos estão em linha reta
function ArePointsAligned(p1, p2, p3)
    // Calcula os vetores entre os pontos
    var dx1 := p2.x - p1.x;
    var dy1 := p2.y - p1.y;
    var dx2 := p3.x - p2.x;
    var dy2 := p3.y - p2.y;
    
    // Se os vetores são proporcionais, os pontos estão alinhados
    // Usando produto cruzado: se dx1*dy2 - dy1*dx2 == 0, então estão alinhados
    var crossProduct := (dx1 * dy2) - (dy1 * dx2);
    
    // Considera alinhado se o produto cruzado for 0 (ou muito próximo de 0)
    // Valor absoluto manual já que POL pode não ter abs()
    if (crossProduct < 0)
        crossProduct := -crossProduct;
    endif
    
    return (crossProduct <= 1); // Tolerância de 1 para compensar arredondamentos
endfunction

function DoppelgangerTransformation(mobile, opponent)
    if (!mobile || !opponent || !mobile.isDoppelganger())
        return 0;
    endif

    // Don't transform if already transformed
    if (GetObjProperty(mobile, "#doppelganger_transformed"))
        return 0;
    endif

    // Don't transform NPCs or already dead opponents
    if (opponent.npctemplate || opponent.dead)
        return 0;
    endif

    // Store original appearance and equipment for cleanup
    var original_data := struct{
        "graphic" := mobile.graphic,
        "color" := mobile.color,
        "name" := mobile.name,
        "title_race" := mobile.title_race,
        "title_suffix" := mobile.title_suffix,
        "title_prefix" := mobile.title_prefix,
        "gender" := mobile.gender,
        "trueobjtype" := mobile.trueobjtype,
        "truecolor" := mobile.truecolor
    };

    // Create safe storage area for original equipment
    var storage_pack := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, 0xe75, 1, mobile.realm);
    if (!storage_pack)
        return 0;
    endif
    MoveObjectToLocation(storage_pack, 4230, 25, 1, mobile.realm, MOVEOBJECT_FORCELOCATION);

    // Store original equipment
    var original_items := ListEquippedItems(mobile);
    foreach item in original_items
        MoveItemToContainer(item, storage_pack);
        sleepms(2);
    endforeach

    // Copy opponent's appearance
    mobile.gender := opponent.gender;
    mobile.graphic := opponent.graphic;
    mobile.color := opponent.color;
    mobile.title_race := opponent.title_race;
    mobile.title_suffix := opponent.title_suffix;
    mobile.title_prefix := opponent.title_prefix;
    mobile.name := opponent.name;
    mobile.TrueColor := opponent.TrueColor;
    mobile.trueobjtype := opponent.trueobjtype;

    // Copy opponent's description if available
    var desc := GetObjProperty(opponent, "description");
    if (desc != error)
        SetObjProperty(mobile, "description", desc);
    else
        EraseObjProperty(mobile, "description");
    endif

    // Copy and equip opponent's equipment
    var copied_items := array{};
    var opponent_items := ListEquippedItems(opponent);
    foreach item in opponent_items
        var copied_item := CreateItemCopyAtLocation(mobile.x, mobile.y, mobile.z, item);
        if (copied_item)
            copied_item.color := item.color;
            copied_item.name := item.name;
            copied_item.desc := item.desc;
            EquipItem(mobile, copied_item);
            copied_items.append(copied_item.serial);
        endif
        sleepms(2);
    endforeach

    // Set combat behavior based on opponent's class
    var opponent_class := GetObjProperty(opponent, "classe");
    var combat_properties := struct{
        "original_behavior" := isMonsterRanged() ? "ranged" : "melee",
        "adopted_behavior" := "melee",
        "spell_school" := "",
        "is_ranged" := 0
    };

    // Determine combat style
    if (opponent_class == "Sabio")
        combat_properties.adopted_behavior := "mage";
        combat_properties.spell_school := GetObjProperty(opponent, "SpellSchool");
        if (combat_properties.spell_school)
            SetObjProperty(mobile, "SpellSchool", combat_properties.spell_school);
        endif
        combat_properties.is_ranged := 1;
    elseif (opponent.weapon && opponent.weapon.isRanged())
        combat_properties.adopted_behavior := "ranged";
        combat_properties.is_ranged := 1;
    else
        combat_properties.adopted_behavior := "melee";
        combat_properties.is_ranged := 0;
    endif

    // Store transformation data for cleanup
    var transformation_data := struct{
        "original_data" := original_data,
        "storage_pack" := storage_pack,
        "copied_items" := copied_items,
        "combat_properties" := combat_properties,
        "transformation_time" := ReadGameClock()
    };

    SetObjProperty(mobile, "#doppelganger_transformed", 1);
    SetObjProperty(mobile, "#doppelganger_data", transformation_data);

    // Set combat behavior override
    if (combat_properties.is_ranged)
        SetObjProperty(mobile, "#force_ranged", 1);
    else
        EraseObjProperty(mobile, "#force_ranged");
    endif

    return 1;
endfunction

function RestoreDoppelgangerAppearance(mobile)
    var transformation_data := GetObjProperty(mobile, "#doppelganger_data");
    if (!transformation_data || !transformation_data.original_data)
        return 0;
    endif

    var original := transformation_data.original_data;
    mobile.graphic := original.graphic;
    mobile.color := original.color;
    mobile.name := original.name;
    mobile.title_race := original.title_race;
    mobile.title_suffix := original.title_suffix;
    mobile.title_prefix := original.title_prefix;
    mobile.gender := original.gender;
    mobile.trueobjtype := original.trueobjtype;
    mobile.truecolor := original.truecolor;

    return 1;
endfunction

function CleanupDoppelgangerEquipment(mobile)
    var transformation_data := GetObjProperty(mobile, "#doppelganger_data");
    if (!transformation_data)
        return 0;
    endif

    // Destroy all copied equipment (using serials to find items)
    if (transformation_data.copied_items)
        foreach item_serial in (transformation_data.copied_items)
            var item := SystemFindObjectBySerial(item_serial);
            if (item)
                DestroyItem(item);
            endif
            sleepms(2);
        endforeach
    endif

    // Destroy the storage pack and its contents
    if (transformation_data.storage_pack)
        var storage_pack := transformation_data.storage_pack;
        if (storage_pack)
            DestroyItem(storage_pack);
        endif
    endif

    // Clear transformation properties
    EraseObjProperty(mobile, "#doppelganger_transformed");
    EraseObjProperty(mobile, "#doppelganger_data");
    EraseObjProperty(mobile, "#force_ranged");
    EraseObjProperty(mobile, "SpellSchool");

    return 1;
endfunction

function CleanupDoppelgangerTransformation(mobile)
    // This function calls both appearance restoration and equipment cleanup
    // for backward compatibility
    RestoreDoppelgangerAppearance(mobile);
    CleanupDoppelgangerEquipment(mobile);
    return 1;
endfunction