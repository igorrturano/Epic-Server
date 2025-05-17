use uo;
use os;
use cfgfile;
include ":attributes:attributes";
include ":combat:hitscripts";
include "include/epicNPCUtils";
include ":merchants:include/item_template";

// Creates an epic boss from a boss structure
function CreateEpicBoss(boss, x, y, z, realm)
    // Validate required fields
    if (!boss.npctemplate)
        return error{"errortext" := "Boss structure missing npctemplate"};
    endif
    
    // Create the NPC with proper template path
    var npc := CreateNpcWithPrefix(boss.npctemplate, x, y, z, realm);
    if (!npc || npc == error)
        return error{"errortext" := "Failed to create boss at x " + x + " y " + y + " z " + z + ": " + npc.errortext};
    endif
    var tier := boss.tier != 0 ? boss.tier : npc.getTier();
    customizeNpc(npc, tier);
    // Set basic properties
    if (boss.name)
        SetName(npc, boss.name);
    endif
    
    if (boss.color)
        npc.color := CInt(boss.color);
        npc.truecolor := CInt(boss.color);
    endif
    
    if (boss.trueboss)
        SetObjProperty(npc, "true_boss", 1);
    endif
    
    // Set damage modifier
    if (boss.dmg)
        SetObjProperty(npc, "dmgbonus", CDbl(boss.dmg));
    endif
// TODO: Validar se as resistencias estão sendo aplicadas corretamente
    // Set resistances
    if (boss.phresist)
        npc.resist_physical_mod := CInt(boss.phresist);
    endif
    
    if (boss.firesist)
        npc.resist_fire_mod := CInt(boss.firesist);
    endif
    
    if (boss.coresist)
        npc.resist_cold_mod := CInt(boss.coresist);
    endif
    
    if (boss.enresist)
        npc.resist_energy_mod := CInt(boss.enresist);
    endif
    
    if (boss.poresist)
        npc.resist_poison_mod := CInt(boss.poresist);
    endif
    
    // Set attribute modifiers
    if (boss.str)
        AP_SetStatMod(npc, STRENGTH, CInt(boss.str));
    endif
    
    if (boss.dex)
        AP_SetStatMod(npc, DEXTERITY, CInt(boss.dex));
    endif
    
    if (boss.int)
        AP_SetStatMod(npc, INTELLIGENCE, CInt(boss.int));
    endif

    if (boss.hits)
        SetObjProperty(npc, "hitsmod", CInt(boss.hits));
        RecalcVitals(npc);
        AP_RefreshVitals(npc);
    endif
    
    // Set skill modifiers
    if (boss.wrestling)
        AP_SetSkillMod(npc, WRESTLING, CInt(boss.wrestling));
    endif
    
    if (boss.tactics)
        AP_SetSkillMod(npc, TACTICS, CInt(boss.tactics));
    endif
    
    if (boss.magery)
        AP_SetSkillMod(npc, MAGERY, CInt(boss.magery));
    endif
    
    if (boss.awareness)
        AP_SetSkillMod(npc, AWARENESS, CInt(boss.awareness));
    endif
    
    // Add all other skills from attributeConstants.inc
    if (boss.alchemy)
        AP_SetSkillMod(npc, ALCHEMY, CInt(boss.alchemy));
    endif
    
    if (boss.parry)
        AP_SetSkillMod(npc, PARRY, CInt(boss.parry));
    endif
    
    if (boss.onehanded)
        AP_SetSkillMod(npc, ONEHANDED, CInt(boss.onehanded));
    endif
    
    if (boss.twohanded)
        AP_SetSkillMod(npc, TWOHANDED, CInt(boss.twohanded));
    endif
    
    if (boss.dualwield)
        AP_SetSkillMod(npc, DUALWIELD, CInt(boss.dualwield));
    endif
    
    if (boss.ranged)
        AP_SetSkillMod(npc, RANGED, CInt(boss.ranged));
    endif
    
    if (boss.sneak)
        AP_SetSkillMod(npc, SNEAK, CInt(boss.sneak));
    endif
    
    if (boss.pickpocketing)
        AP_SetSkillMod(npc, PICKPOCKETING, CInt(boss.pickpocketing));
    endif
    
    if (boss.mechanics)
        AP_SetSkillMod(npc, MECHANICS, CInt(boss.mechanics));
    endif
    
    if (boss.medicine)
        AP_SetSkillMod(npc, MEDICINE, CInt(boss.medicine));
    endif
    
    if (boss.survival)
        AP_SetSkillMod(npc, SURVIVAL, CInt(boss.survival));
    endif
    
    if (boss.metalworking)
        AP_SetSkillMod(npc, METALWORKING, CInt(boss.metalworking));
    endif
    
    if (boss.woodworking)
        AP_SetSkillMod(npc, WOODWORKING, CInt(boss.woodworking));
    endif
    
    if (boss.domestic_work)
        AP_SetSkillMod(npc, Domestic_Work, CInt(boss.domestic_work));
    endif
    
    if (boss.gathering)
        AP_SetSkillMod(npc, GATHERING, CInt(boss.gathering));
    endif
    
    if (boss.handleanimal)
        AP_SetSkillMod(npc, HANDLEANIMAL, CInt(boss.handleanimal));
    endif
    
    // Set special abilities
    if (boss.special && boss.special.size() > 0)
        foreach ability in (boss.special)
            // Get the script path from the ability
            var script := ability.script_path;
            
            if (ability.name && script)
                // For Special AI, we need to set specific properties
                if (ability.name == "Special AI")
                    SetObjProperty(npc, "AIScript", script);
                    SetObjProperty(npc, "AIType", script);
                else
                    // For all other abilities, just add the hit script directly
                    AddHitScript(npc, script);
                endif
            endif
        endforeach
    endif
    
    // Set armor hit scripts
    if (boss.armorspecial && boss.armorspecial.size() > 0)
        foreach ability in (boss.armorspecial)
            var script := ability.script_path;
            
            if (ability.name && script)
                // For all armor abilities, just add the armor hit script directly
                AddArmorHitScript(npc, script);
                
                // Verify the script was added
                var armor_scripts := GetObjProperty(npc, "ArmorHitScripts");
            endif
        endforeach
    endif
    
    // Set loot items
    if (boss.loot && boss.loot.size() > 0)
        var loot_array := array{};
        foreach item in (boss.loot)
            loot_array.append(item);
        endforeach
        SetObjProperty(npc, "LootItems", loot_array);
    endif
    
    // Set profile properties
    if (boss.url)
        SetObjProperty(npc, "url", boss.url);
    endif
    
    if (boss.detailed_description)
        SetObjProperty(npc, "descricao", boss.detailed_description);
    endif

    SetObjProperty(npc, "IsBoss", 1); // Mark this NPC as a boss
    // Set the boss structure on the NPC for reference
    SetObjProperty(npc, "EpicBossStruct", boss);

    // Set combat events
    if (boss.combatevents && boss.combatevents.size() > 0)
        // Create a clean array with consistent structure
        var cleaned_events := array{};
        
        foreach event in (boss.combatevents)
            var clean_event := struct{};
            clean_event.+name := event.name;
            clean_event.+cd := event.cd;
            clean_event.+onlybloodied := event.onlybloodied;
            clean_event.+dist := event.dist;
            clean_event.+steps := array{};
            
            // Use actions if available, otherwise fall back to steps
            if (event.actions && event.actions.size() > 0)
                foreach action in (event.actions)
                    clean_event.steps.append(action);
                endforeach
            elseif (event.steps && event.steps.size() > 0)
                foreach action in (event.steps)
                    clean_event.steps.append(action);
                endforeach
            endif
            
            cleaned_events.append(clean_event);
        endforeach
        
        SetObjProperty(npc, "CombatEvents", cleaned_events);
        var res := start_script(":epicspawn:scripts/combatEventsControl", array{npc});
        if(res == error)
            PrintTextAbove(npc, "Combat Events script error: " + res.errortext);
        endif
    endif

    if (boss.flee_health)
        SetObjProperty(npc, "flee_health", CInt(boss.flee_health));
    endif

    if (boss.class)
        SetObjProperty(npc, "subclass", boss.class);
    endif
    
    if (boss.preferred_target)
        SetObjProperty(npc, "preferred_target", boss.preferred_target);
    endif

    if (boss.target_priority)
        SetObjProperty(npc, "target_priority", boss.target_priority);
    endif

    // Make sure this line exists when setting properties on the NPC
    SetObjProperty(npc, "DeathEvents", boss.deathevents);
   
    // Set patrol points if they exist
    if (boss.patrol_points && boss.patrol_points.size() > 0)
        SetObjProperty(npc, "patrol_points", boss.patrol_points);
        SetObjProperty(npc, "#current_patrol_point", 1);
    endif
    
    // Add teleport locations to boss structure
    if (boss.teleport_locations && boss.teleport_locations.size() > 0)
        SetObjProperty(npc, "teleport_locations", boss.teleport_locations);
    endif

    if (boss.natural_traits && boss.natural_traits.size() > 0)
        SetObjProperty(npc, "natural_traits", boss.natural_traits);
    endif

    // Add Equipment if selected
    if (boss.specific_weapon)
        var weapon := boss.specific_weapon;
        if (weapon)
            SetObjProperty(npc, "specific_weapon", weapon);
        endif
    endif

    // And when creating the NPC:
    if (boss.spell_school)
        SetObjProperty(npc, "SpellSchool", boss.spell_school);
    endif

    if (boss.custom_abilities && boss.custom_abilities.size() > 0)
        SetObjProperty(npc, "CustomAbilities", boss.custom_abilities);
    endif

    if (boss.preferred_target)
        SetObjProperty(npc, "preferred_target", boss.preferred_target);
    endif

    if (boss.target_priority)
        SetObjProperty(npc, "target_priority", boss.target_priority);
    endif

    // Make sure this line exists when setting properties on the NPC
    SetObjProperty(npc, "DeathEvents", boss.deathevents);
   
    // Set patrol points if they exist
    if (boss.patrol_points && boss.patrol_points.size() > 0)
        SetObjProperty(npc, "patrol_points", boss.patrol_points);
        SetObjProperty(npc, "#current_patrol_point", 1);
    endif
    
    // Add teleport locations to boss structure
    if (boss.teleport_locations && boss.teleport_locations.size() > 0)
        SetObjProperty(npc, "teleport_locations", boss.teleport_locations);
    endif

    // Add Equipment if selected
    if (boss.specific_weapon)
        var weapon := boss.specific_weapon;
        if (weapon)
            SetObjProperty(npc, "specific_weapon", weapon);
        endif
    endif

    // And when creating the NPC:
    if (boss.spell_school)
        SetObjProperty(npc, "SpellSchool", boss.spell_school);
    endif

    if (boss.custom_abilities && boss.custom_abilities.size() > 0)
        SetObjProperty(npc, "CustomAbilities", boss.custom_abilities);
    endif

    return npc;
endfunction

// Creates an NPC with the appropriate package prefix
function CreateNpcWithPrefix(template, x, y, z, realm)
    var npc;
    
    // Check if template already has a package prefix

    if (template[1] == ":")
        npc := CreateNpcFromTemplate(template, x, y, z, 0, realm);

        if (npc && npc != error)
            return npc;
        endif
    else
        // Try different package prefixes
        var prefixes := array{
            ":epicAI:",
            ":brainAI:",
            ":merchants:"
        };
        
        foreach prefix in prefixes
            npc := CreateNpcFromTemplate(prefix + template, x, y, z, 0, realm);
            if (npc && npc != error)
                return npc;
            endif
            sleepms(2);
        endforeach
    endif
    
    // If all attempts fail, return the error
    if (npc == error)
        return npc;
    endif
    
    return error{"errortext" := "Could not create NPC with template " + template};
endfunction

// Adds a hit script to an NPC
function AddHitScript(npc, script)
    // Check if NPC is valid
    if (!npc)
        return 0;
    endif
    
    // Get current hit scripts
    var hit_scripts := GetObjProperty(npc, "HitScripts");
    
    if (!hit_scripts)
        hit_scripts := array{};
    endif
    
    // Add the new script
    hit_scripts.append(script);
    
    // Set the property
    SetObjProperty(npc, "HitScripts", hit_scripts);
    
    return 1;
endfunction

// Adds an armor hit script to an NPC
function AddArmorHitScript(npc, script) 
    // Check if NPC is valid
    if (!npc)
        return 0;
    endif
    
    // Get current armor hit scripts
    var hit_scripts := GetObjProperty(npc, "ArmorHitScripts");
    
    if (!hit_scripts)
        hit_scripts := array{};
    endif
    
    // Add the new script
    hit_scripts.append(script);
    
    // Set the property
    SetObjProperty(npc, "ArmorHitScripts", hit_scripts);
    
    return 1;
endfunction

// Get all available weapon hit scripts
function GetWeaponHitScripts()
    return array{
        "Ice Hit",
        "Fire Hit",
        "Energy Hit",
        "Life Drain",
        "Bleeding Strike",
        "Concussion Hit",
        "Unmount Hit",
        "Anti Mounted Hit",
        "Charge Hit",
        "Height Hit",
        "Free Armor Hit",
        "Queima Roupa",
        "Area Damage",
        "Freeze Hit",
        "Slayer",
        "Pushback",
        "Spit Acid",
        "Custom Hit Script"
    };
endfunction

// Get all available armor hit scripts
function GetArmorHitScripts()
    return array{
        "Damage Return",
        "Raw Damage Return",
        "Bog Thing",
        "Campo Eletrico",
        "Dupe Me",
        "Custom Armor Hit Script"
    };
endfunction

/*
 * GetAllOpponents(npc)
 * Purpose: Gets all opponents (players) in combat with the NPC
 * Parameters: npc - The NPC to get opponents for
 * Return: Array of opponents
 */
function GetAllOpponents(npc)
    var opponents := array{};
    var range := 8; // Search range
    
    foreach mobile in ListMobilesNearLocation(npc.x, npc.y, npc.z, range, npc.realm)
        if (mobile.IsA(POLCLASS_MOBILE) && !mobile.dead && mobile.opponent == npc)
            opponents.append(mobile);
        endif
    endforeach
    
    return opponents;
endfunction 

function OneHandedWeapons(who, title, gump_width, gump_height)
    var weapons := GetWeaponsByAttribute("One_Handed");
    return ShowWeaponsWithPagination(who, title, gump_width, gump_height, weapons, "ONE-HANDED");
endfunction

function TwoHandedWeapons(who, title, gump_width, gump_height)
    var weapons := GetWeaponsByAttribute("Two_Handed");
    return ShowWeaponsWithPagination(who, title, gump_width, gump_height, weapons, "TWO-HANDED");
endfunction

function ShowDualWieldWeapons(who, title, gump_width, gump_height)
    var weapons := GetWeaponsByAttribute("Dual_Wield");
    return ShowWeaponsWithPagination(who, title, gump_width, gump_height, weapons, "DUAL WIELD");
endfunction

function RangedWeapons(who, title, gump_width, gump_height)
    var weapons := GetWeaponsByAttribute("Ranged");
    return ShowWeaponsWithPagination(who, title, gump_width, gump_height, weapons, "RANGED");
endfunction

function GetWeaponsByAttribute(attribute)
    var weapons := array{};
    var config := ReadConfigFile(":combat:itemdesc");
    
    if (!config)
        return weapons;
    endif
    
    // Get all entries from the config file
    var entries := GetConfigStringKeys(config);
    
    foreach entry in entries
        var elem := FindConfigElem(config, entry);
        if (elem)
            // Check if it's a weapon with the desired attribute
            if (elem.Attribute == attribute)
                var weapon := struct{
                    "objtype" := CInt(entry),
                    "name" := elem.Name
                };
                weapons.append(weapon);
            endif
        endif
        sleepms(2);
    endforeach
    
    return weapons;
endfunction

function ShowWeaponsWithPagination(who, title, gump_width, gump_height, weapons, weapon_type, page := 1)
    var gump := GFECreateGump(title, gump_width, gump_height);
    
    // Add navigation buttons
    AddNavigationButtons(gump, gump_width);
    
    // Title
    GFTextMid(gump, 0, 130, gump_width, 1153, "SELECT " + weapon_type + " WEAPON");
    
    // Add a divider line
    GFResizePic(gump, 40, 150, 3000, gump_width - 80, 2);
    
    // Calculate pagination
    var items_per_page := 12; // 3 columns x 4 rows
    var total_pages := Ceil(weapons.size() / items_per_page);
    var start_index := (page) * items_per_page;
    var end_index := Min(start_index + items_per_page, weapons.size());
    
    // Display weapons in a grid (3 columns x 4 rows)
    var x := 50;
    var y := 170;
    var column := 0;
    
    for i := start_index to end_index
        var weapon := weapons[i];
        
        // Display weapon image
        GFTilePic(gump, x, y, weapon.objtype);
        
        // Display weapon name
        GFTextLine(gump, x + 50, y + 10, 1153, weapon.name);
        
        // Add button to select weapon
        GFAddButton(gump, x + 150, y + 10, 2117, 2118, 1, weapon.objtype);
        
        // Move to next position
        column += 1;
        if (column >= 3) // Move to next row after 3 items
            column := 0;
            x := 50;
            y += 80;
        else
            x += 250; // Move to next column
        endif
    endfor
    
    // Add pagination controls at the bottom
    var pagination_y := gump_height - 100;
    
    // Previous page button
    if (page > 1)
        GFAddButton(gump, gump_width/2 - 100, pagination_y, 2117, 2118, 1, 900);
        GFTextLine(gump, gump_width/2 - 70, pagination_y, 1153, "Previous");
    endif
    
    // Page indicator
    GFTextMid(gump, 0, pagination_y, gump_width, 1153, "Page " + page + " of " + total_pages);
    
    // Next page button
    if (page < total_pages)
        GFAddButton(gump, gump_width/2 + 50, pagination_y, 2117, 2118, 1, 901);
        GFTextLine(gump, gump_width/2 + 80, pagination_y, 1153, "Next");
    endif
    
    // Add Cancel button
    GFAddButton(gump, gump_width - 100, gump_height - 50, 2119, 2120, 1, 0);
        
    var input := GFSendGump(who, gump);
    // Handle pagination buttons
    if (input[0] == 900) // Previous page
        return ShowWeaponsWithPagination(who, title, gump_width, gump_height, weapons, weapon_type, page - 1);
    elseif (input[0] == 901) // Next page
        return ShowWeaponsWithPagination(who, title, gump_width, gump_height, weapons, weapon_type, page + 1);
    endif
    
    // Handle weapon selection
    if (input[0] > 0 && input[0] != 900 && input[0] != 901)
        form_data.specific_weapon := input[0];
        SendSysMessage(who, "Selected weapon ID: " + input[0]);
        return ShowPage5(who, title, gump_width, gump_height);
    endif

    return input;
endfunction

function ShowClassAbilities(who, title, gump_width, gump_height, class_name)
    var gump := GFECreateGump(title, gump_width, gump_height);
    
    // Add navigation buttons
    AddNavigationButtons(gump, gump_width);
    
    // Title
    GFTextMid(gump, 0, 130, gump_width, 1153, "SELECT " + class_name + " LEVEL 4 ABILITY");
    
    // Add a divider line
    GFResizePic(gump, 40, 150, 3000, gump_width - 80, 2);
    
    // Get abilities for the selected class
    var abilities := GetClassAbilities(class_name);
    var ability_map := dictionary{}; // Map to store button ID := ability mapping
    
    // Display abilities in a list format
    var y := 170;
    var button_id := 100;

    foreach ability in abilities
        // Convert button_id to string for dictionary key
        var key := CStr(button_id);
        ability_map[key] := ability;
        
        // Add the ability name with a button
        GFAddButton(gump, 50, y, 2117, 2118, 1, button_id);
        GFTextLine(gump, 80, y, 1153, ability.name);
        
        // Add tooltip with description
        if (ability.desc)
            GFTooltipText(gump, ability.desc);
        endif
        
        y += 30;
        button_id += 1;
    endforeach

    
    // Add Cancel button
    GFAddButton(gump, gump_width - 100, gump_height - 50, 2119, 2120, 1, 0);
    
    var input := GFSendGump(who, gump);
    
    // Handle ability selection
    if (input[0] >= 100)
        var key := CStr(input[0]);
        
        var selected_ability := ability_map[key];
        if (selected_ability)
            return selected_ability.name;
        endif
    else
        return ShowPage5(who, title, gump_width, gump_height);
    endif

endfunction

function GetClassAbilities(class_name)
    var abilities := array{};
    var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
        
    foreach hab in (GetConfigStringKeys(habcfg))
        var elem := FindConfigElem(habcfg, hab);
        var classe := GetConfigString(elem, "classe");
        
        if (classe)
            var params := SplitWords(classe);
            var hab_class := params[1];
            var hab_level := CInt(params[2]);
                      
            // Only get level 4 abilities for the selected class
            if (hab_class == class_name && hab_level == 2)
                var ability := struct{
                    "name" := GetConfigString(elem, "nome"),
                    "desc" := GetConfigString(elem, "desc")
                };
                abilities.append(ability);
            endif
        endif
        sleepms(2);
    endforeach
    
    return abilities;
endfunction

function ShowMagicSchoolSelection(who, title, gump_width, gump_height)
    var gump := GFCreateGump();
    GFResizePic(gump, 0, 0, GFGetCfgConst("Backgrounds", "GOLDTRIM"), 500, 400);
    GFResizePic(gump, 15, 15, 9350, 470, 370);
    
    GFTextMid(gump, 0, 25, 500, 1153, "Select Magic School");
    
    // Add school descriptions with their associated abilities
    var schools := array{
        struct{
            "name" := "Ahtaleloreth",
            "desc" := "School of Elements - Base: Vínculo Elemental",
            "color" := 1645 // Light blue
        },
        struct{
            "name" := "Evocatium",
            "desc" := "School of Pure Magic - Base: Determinacao Arcana",
            "color" := 2100 // Purple
        },
        struct{
            "name" := "Kaijin",
            "desc" := "School of Blood - Base: Sacrificio da Carne",
            "color" := 32 // Dark red
        },
        struct{
            "name" := "Aradalore",
            "desc" := "School of Nature - Base: Elo Natural",
            "color" := 272 // Green
        },
        struct{
            "name" := "Velkyn",
            "desc" := "School of Shadows - Base: Danca das Sombras",
            "color" := 902 // Dark purple
        }
    };
    
    var y_pos := 60;
    for i := 1 to schools.size()
        // Add selection button
        GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, i);
        
        // School name in its thematic color
        GFTextLine(gump, 60, y_pos, schools[i].color, schools[i].name);
        
        // Description in normal color
        GFTextLine(gump, 60, y_pos + 20, 2100, schools[i].desc);
        
        y_pos += 60;
    endfor
    
    // Add Cancel button
    GFAddButton(gump, 200, 350, 2119, 2120, GF_CLOSE_BTN, 0);
    
    var input := GFSendGump(who, gump);
    
    if (input[0] == 0)
        return "";
    endif
    
    var selected_school := schools[input[0]].name;
    SendSysMessage(who, "Selected " + selected_school + " magic school.");
    
    return selected_school;
endfunction

// Add this new function to handle loot selection and configuration
function HandleAddLootItem(who, byref form_data)
    SendSysMessage(who, "Target the item you want to add as loot.", color := 33);
    var item := Target(who);
    
    if (!item || !item.IsA(POLCLASS_ITEM))
        SendSysMessage(who, "Cancelled or invalid target.", color := 33);
        return 0;
    endif

    var template := CreateItemTemplate(item);
    if (template == error)
        SendSysMessage(who, "Error creating item template: " + template.errortext);
        return 0;
    endif

    // Create drop chance selection gump
    var gump := GFCreateGump();
    GFResizePic(gump, 0, 0, GFGetCfgConst("Backgrounds", "GOLDTRIM"), 400, 300);
    GFResizePic(gump, 15, 15, 9350, 370, 270);
    
    GFTextMid(gump, 0, 25, 400, 1153, "Set Loot Details");
    GFTextLine(gump, 50, 60, 1153, "Item: " + item.desc);
    
    // Add drop chance entry
    GFTextLine(gump, 50, 90, 1153, "Drop chance (1-100%):");
    GFTextEntry(gump, 50, 120, 100, 20, 1153, "25", 1);
    
    // Simplified death option - just a checkbox
    GFCheckBox(gump, 50, 160, 2510, 2511, 0, 2);
    GFTextLine(gump, 80, 160, 1153, "Generate on death only");
    
    // Add Create and Cancel buttons
    GFAddButton(gump, 50, 250, 2128, 2129, 1, 100);
    GFTextLine(gump, 90, 250, 33, "Add Item");
    
    GFAddButton(gump, 200, 250, 2119, 2120, 1, 0);
    GFTextLine(gump, 240, 250, 33, "Cancel");
    
    var input := GFSendGump(who, gump);
    if (!input[0])
        return 0;
    endif
    
    var drop_chance := CInt(GFExtractData(input, 1));
    if (!drop_chance || drop_chance < 1 || drop_chance > 100)
        SendSysMessage(who, "Invalid drop chance. Must be between 1 and 100.", color := 33);
        return 0;
    endif
    
    // Create loot item structure
    var loot_item := struct{
        "template" := template,
        "drop_chance" := drop_chance,
        "on_death" := input[2] ? 1 : 0,  // If checkbox is checked, it's a death drop
        "desc" := item.desc
    };

    // Initialize loot array if it doesn't exist
    if (!form_data.loot)
        form_data.+loot := array{};
    endif
    
    form_data.loot.append(loot_item);

    SendSysMessage(who, "Added " + item.desc + " with " + drop_chance + "% drop chance to generate on " + 
                  (loot_item.on_death ? "death" : "initialization") + ".", color := 66);
    return 1;
endfunction

function GetAvailableCustomAbilities()
    var abilities := dictionary{
        "WebWrap" := struct{
            "name" := "Web Wrap",
            "description" := "Allows the NPC to wrap targets in web",
            "method" := "CanDoWebWrap"
        },
        "SpitAcid" := struct{
            "name" := "Spit Acid",
            "description" := "Allows the NPC to spit acid at targets",
            "method" := "canSpitAcid"
        },
        "TongueGrab" := struct{
            "name" := "Tongue Grab",
            "description" := "Allows the NPC to grab targets with tongue",
            "method" := "CanTongueGrab"
        },
        "Charge" := struct{
            "name" := "Charge",
            "description" := "Allows the NPC to charge at targets",
            "method" := "canCharge"
        },
        "Ambush" := struct{
            "name" := "Ambush",
            "description" := "Allows the NPC to ambush targets",
            "method" := "CanAmbush"
        },
        "Confusion" := struct{
            "name" := "Confusion",
            "description" := "Allows the NPC to confuse targets",
            "method" := "canDoConfusion"
        },
        "WebShoot" := struct{
            "name" := "Web Shoot",
            "description" := "Allows the NPC to shoot webs at targets",
            "method" := "CanShootWeb"
        },
        "PoisonBite" := struct{
            "name" := "Poison Bite",
            "description" := "Allows the NPC to poison targets with bite",
            "method" := "CanPoisonBite"
        },
        "ClimbWall" := struct{
            "name" := "Climb Wall",
            "description" := "Allows the NPC to climb walls",
            "method" := "CanClimbWall"
        },
        "LayEggSac" := struct{
            "name" := "Lay Egg Sac",
            "description" := "Allows the NPC to lay egg sacs",
            "method" := "CanLayEggSac"
        },
        "Charm" := struct{
            "name" := "Charm",
            "description" := "Allows the NPC to charm targets",
            "method" := "CanCharm"
        },
        "Bite" := struct{
            "name" := "Bite",
            "description" := "Allows the NPC to bite targets",
            "method" := "canBite"
        },
        "Throw" := struct{
            "name" := "Throw",
            "description" := "Allows the NPC to throw objects or weapons",
            "method" := "canThrow"
        },
            "HiveMind" := struct{
            "name" := "Hive Mind",
            "description" := "Allows the NPC to share information with others of its kind",
            "method" := "hasHiveMind"
        },
        "TunnelTactics" := struct{
            "name" := "Tunnel Tactics",
            "description" := "Allows the NPC to use special tactics in tunnels",
            "method" := "hasTunnelTactics"
        },
        "Rage" := struct{
            "name" := "Rage",
            "description" := "Allows the NPC to enter a rage state",
            "method" := "hasRage"
        }
    };
    
    return abilities;
endfunction

function HandleAddCustomAbility(who, byref form_data)
    var abilities := GetAvailableCustomAbilities();
    
    var gump := GFECreateGump("Select Custom Behavior", 450, 600);
    
    var y_pos := 80;
    var btn_id := 1;
    var ability_map := dictionary{}; // To track button-to-ability mapping
    
    // Store ability keys in an array to maintain order
    var ability_keys := array{};
    foreach key in (abilities.keys())
        ability_keys.append(key);
        sleepms(2);
    endforeach
    
    foreach ability in ability_keys
        ability_map[btn_id] := ability;   // Map button to ability
        
        // Add button and display the proper ability name
        GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, btn_id);
        GFTextLine(gump, 60, y_pos, 1153, ability.name);
        GFTooltipText(gump, ability.description);
        
        y_pos += 25;
        btn_id += 1;
        sleepms(2);
    endforeach

    // Add Cancel button
    GFAddButton(gump, 30, y_pos + 20, 2119, 2120, GF_CLOSE_BTN, 999);
    
    var result := GFSendGump(who, gump);
    
    if (result[0] == 999 || !result[0])
        return 0;
    endif
    
    // Get selected ability name using the ability map
    var selected_ability := ability_map[result[0]];
   
    if (!selected_ability)
        return 0;
    endif
    
    // Initialize custom abilities array if needed
    if (!form_data.custom_abilities)
        form_data.+custom_abilities := array{};
    endif

    // Add ability if not already present
    if (!(selected_ability in form_data.custom_abilities))
        form_data.custom_abilities.append(selected_ability);
        SendSysMessage(who, "Added " + abilities[selected_ability].name + " ability.");
    else
        SendSysMessage(who, "This ability is already added.");
    endif
    
    return 1;
endfunction

function ShowTargetSelectionMenu(who, byref form_data)
    var gump := GFCreateGump();
    GFResizePic(gump, 0, 0, GFGetCfgConst("Backgrounds", "GOLDTRIM"), 400, 450);
    GFResizePic(gump, 15, 15, 9350, 370, 420);
    
    GFTextMid(gump, 0, 25, 400, 1153, "Target Selection Strategy");
    
    var y_pos := 60;
    var options := array{
        struct{ "name" := "Ranged", "desc" := "Prioritizes ranged attackers" },
        struct{ "name" := "Melee", "desc" := "Prioritizes melee attackers" },
        struct{ "name" := "Guerreiro", "desc" := "Prioritizes warrior class" },
        struct{ "name" := "Ladino", "desc" := "Prioritizes rogue class" },
        struct{ "name" := "Mago", "desc" := "Prioritizes mage class" },
        struct{ "name" := "Pouca vida", "desc" := "Prioritizes targets below 50% health" }    };
    
    foreach option in options
        GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, _option_iter);
        GFTextLine(gump, 60, y_pos, 1153, option.name);
        GFTooltipText(gump, option.desc);
        y_pos += 30;
    endforeach
    
    // Cancel button
    GFAddButton(gump, 150, 380, 2119, 2120, GF_CLOSE_BTN, 999);
    
    var input := GFSendGump(who, gump);
    
    if (!input[0] || input[0] == 999)
        SendSysMessage(who, "Cancelled target selection.");
        return 0;
    endif
    
    // Save selection to form_data
    form_data.target_selection := options[input[0]].name;
    form_data.preferred_target := options[input[0]].name;
    
    SendSysMessage(who, "Target selection set to: " + options[input[0]].name);
    return 1;
endfunction

function ShowTargetPriorityMenu(who, byref form_data)
    var gump := GFCreateGump();
    GFResizePic(gump, 0, 0, GFGetCfgConst("Backgrounds", "GOLDTRIM"), 400, 300);
    GFResizePic(gump, 15, 15, 9350, 370, 270);
    
    GFTextMid(gump, 0, 25, 400, 1153, "Select Target Priority");
    
    var priorities := array{
        struct{ "name" := "Normal", "desc" := "30% de chance de mudar de alvo" },
        struct{ "name" := "Aggressive", "desc" := "50% de chance de mudar de alvo" },
        struct{ "name" := "Defensive", "desc" := "20% de chance de mudar de alvo" },
        struct{ "name" := "Focus", "desc" := "5% de chance de mudar de alvo" }
    };
    
    var y_pos := 60;
    foreach priority in priorities
        GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, _priority_iter);
        GFTextLine(gump, 60, y_pos, 1153, priority.name);
        GFTooltipText(gump, priority.desc);
        y_pos += 30;
    endforeach
    
    // Cancel button
    GFAddButton(gump, 150, 240, 2119, 2120, GF_CLOSE_BTN, 999);
    
    var input := GFSendGump(who, gump);
    
    if (!input[0] || input[0] == 999)
        return 0;
    endif
    
    form_data.target_priority := priorities[input[0]].name;
    
    return 1;
endfunction