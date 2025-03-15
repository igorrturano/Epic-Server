use uo;
use os;
use cfgfile;
include ":attributes:attributes";
include ":combat:hitscripts";

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
                print("DEBUG: Adding armor hit script: " + script + " for ability: " + ability.name);
                AddArmorHitScript(npc, script);
                
                // Verify the script was added
                var armor_scripts := GetObjProperty(npc, "ArmorHitScripts");
                print("DEBUG: ArmorHitScripts after adding: " + armor_scripts);
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
    
    // Set death events
    if (boss.deathevent && boss.deathevent.size() > 0)
        var death_events := array{};
        foreach event in (boss.deathevent)
            death_events.append(event);
        endforeach
        SetObjProperty(npc, "DeathEvents", death_events);
    endif
    
    // Set profile properties
    if (boss.url)
        SetObjProperty(npc, "url", boss.url);
    endif
    
    if (boss.detailed_description)
        SetObjProperty(npc, "descricao", boss.detailed_description);
    endif
    
    // Set the boss structure on the NPC for reference
    SetObjProperty(npc, "EpicBossStruct", boss);
    
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