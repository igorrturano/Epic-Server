use cfgfile;

include ":loot:include/lootParser";
include ":tn:recipes";
include ":mounts:mounts";
include "npcUtils";
include ":fls_core:fls_characters";
include "makeRace";
include ":epicAI:epicSpecial";
const MAX_Z_DIFF := 18;

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
    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif

    setupNPCGender(mobile);
    SetupHumanoidHabs(mobile);
    SetupItemsForSpecificClasses(mobile, npc_elem);
    var race := GetObjProperty(mobile, "myrace");
    if (!race)
        makeRace(mobile);
    endif
    SetupNPCEquip(mobile, npc_elem);
    SetupBaseLanguages(mobile);

    if (mobile.isSage())
        SetupWizardClass(mobile, npc_elem);
    endif


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
    if (GetObjProperty(who, "specific_weapon"))
        var weapon := GetObjProperty(who, "specific_weapon");
        if (weapon)
            var item := CreateItemAtLocation(who.x, who.y, who.z, weapon, 1);
            // First Unequip hands
            if (who.weapon)
                DestroyItem(who.weapon);
            endif
            EquipItem(who, item);
        endif
    endif
    
  var montaria := Getobjproperty(who, "Montaria");
  if (!montaria)
  else
	var graphic := MP_GetMountGraphic(montaria);
	var item := CreateItemAtLocation(who.x, who.y, who.z, MOUNT_OBJTYPE, 1);
	item.graphic := graphic;
	Equipitem (who, item);
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

function setupNPCGender(mobile)
  var female := GetObjProperty(mobile, "female");

  if (female)
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
  //        PrintText(me, "*esconde*");
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

   var highest_aggro := getHighestAggro(me);
   if (highest_aggro && (!opponent || highest_aggro != opponent.serial))
      highest_aggro := SystemFindObjectBySerial(highest_aggro);
      opponent := highest_aggro;
      SetOpponent(highest_aggro);
      TurnToward(highest_aggro);
      return 1;
   endif
endfunction

function getHighestAggro(creature)
   var aggros := creature.aggroProp();
   var highest := array{0, -9999999};
   foreach serial in (aggros.keys())
      if (aggros[serial] >= highest[2])
         highest := array{serial, aggros[serial]};
      endif
      sleepms(2);
   endforeach

   return highest[1];
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

    if (invasor.hidden || invasor.concealed)
        return 0;
    endif

    if (GetCooldown(me, "alert_allies"))
        return 0;
    endif
    
    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 8)
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

function CanFight(attacker, defender)
    if (!defender || defender.dead || defender.hidden || defender.concealed)
        return 0;
    endif

    if ( GetCooldown(me, "peacemade") > 0 )
        return 0;
    endif

    // Cooldown to avoid ress kill
    if (Cint(GetCooldown(defender, "deathtime" )) > 0) 
        // Allow combat if defender initiated it
        if (me.opponent == defender) return 1; endif
        // Otherwise prevent attacking during cooldown
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

function PursueTarget(opponent)
    var dist := Distance(me, opponent);
    if (!shouldKeepPursuing(opponent, dist)) return 0; endif

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

    var range_info := GetWeaponRangeInfo();
    if (me.isHumanoidBrain())
        if (me.weapon.isRanged() || range_info.max_range > 1)
            // Don't get closer than max_range
            if (dist <= range_info.max_range)
                return 1;
            else
                if (me.hidden || me.concealed)
                    WalkToward(opponent);
                else
                    RunToward(opponent);
                endif
            endif
        elseif (me.isSage() && me.weapon.objtype == FIST)
            // Don't get closer than 6 tiles
            if (dist <= 6)
                return 1;
            else
                if (me.hidden || me.concealed)
                    WalkToward(opponent);
                else
                    RunToward(opponent);
                endif
            endif
        else
            if (me.hidden || me.concealed)
                WalkToward(opponent);
            else
                RunToward(opponent);
            endif
        endif
    else
        if (dist > 1)
            if (me.hidden || me.concealed)
                WalkToward(opponent);
            else
                RunToward(opponent);
            endif
        endif
    endif

    // Track progress towards target
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

    // Only check progress every 2 seconds
    if (ReadGameClock() >= progress.last_check_time + 2)
        var moved := (me.x != progress.last_pos.x || me.y != progress.last_pos.y);
        var got_closer := (dist < progress.last_dist);
        
        // No progress if we haven't moved OR haven't gotten closer in 2 seconds
        if (!moved || !got_closer)
            progress.no_progress_count += 1;

        else
            progress.no_progress_count := 0;
        endif

        // Update tracking info
        progress.last_dist := dist;
        progress.last_pos.x := me.x;
        progress.last_pos.y := me.y;
        progress.last_check_time := ReadGameClock();
    endif
    // If we're not making progress after several checks, force a reposition
    if (progress.no_progress_count >= 1)
        EraseObjProperty(me, "path");
        EraseObjProperty(me, "#enemy_last_loc");
        TacticalReposition(opponent, 1);
        progress.no_progress_count := 0;
        progress.last_check_time := ReadGameClock();
        SetObjProperty(me, "#pursue_progress", progress);
        return 1;
    endif

    SetObjProperty(me, "#pursue_progress", progress);

    // Normal pursuit logic
    if (CheckLineOfSight(me, opponent))
        var min_distance := 1;
        if (me.weapon.isRanged() || range_info.max_range > 1)
            min_distance := range_info.max_range;
        elseif (me.isSage() && me.weapon.objtype == FIST)
            min_distance := 6;
        endif

        if (Distance(me, opponent) > min_distance)
            if (me.hidden || me.concealed)
                WalkToward(opponent);
            else
                RunToward(opponent);
            endif
        endif
        sleepms(100);
    else
        var path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES);
        if (path)
            foreach coord in path
                var min_distance := 1;
                if (me.weapon.isRanged() || range_info.max_range > 1)
                    min_distance := range_info.max_range;
                elseif (me.isSage() && me.weapon.objtype == FIST)
                    min_distance := 6;
                endif

                if (Distance(me, opponent) <= min_distance)
                    break;
                endif
                if (!shouldKeepPursuing(opponent, dist)) break; endif
                if (me.hidden || me.concealed)
                    WalkTowardLocation(coord.x, coord.y);
                else
                    RunTowardLocation(coord.x, coord.y);
                endif
                sleepms(100);
            endforeach
        else
            progress.no_progress_count += 1;
            if (me.hasTunnelTactics())
                TunnelTactics(me, opponent);
            endif
        endif
    endif

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
    var spawn_point := SystemFindObjectBySerial(GetObjProperty(me, "spawnpoint"));
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

    while (!hasPlayerNear(me))    
        if (CoordinateDistance(me.x, me.y, home_x, home_y) > 5)
            WalkTowardLocation(home_x, home_y);
        endif
    endwhile
    
    // No need for event checking here since MonsterBrain already does it
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

    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 12)
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

    // If we're in melee range and can see opponent, stop pursuing
    if (dist <= 1 && CheckLineOfSight(me, opponent))
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

    // Approach the corpse
    while (Distance(me, corpse) > 1)
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
    
    // Loot items
    while (looted < max_loot)
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