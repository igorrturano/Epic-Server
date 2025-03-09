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
    
    // Set special abilities
    if (boss.special && boss.special.size() > 0)
        foreach ability in (boss.special)
            if (ability.name && ability.ai)
                case (ability.name)
                    "Special AI":
                        SetObjProperty(npc, "AIScript", ability.ai);
                        SetObjProperty(npc, "AIType", ability.ai);
                        break;
                    "Ice Hit":
                        AddHitScript(npc, ":combat:icehit");
                        break;
                    "Fire Hit":
                        AddHitScript(npc, ":combat:firehit");
                        break;
                    "Energy Hit":
                        AddHitScript(npc, ":combat:energyhit");
                        break;
                    "Life Drain":
                        AddHitScript(npc, ":combat:lifedrain");
                        break;
                    "Damage Return":
                        AddHitScript(npc, ":combat:damagereturn");
                        break;
                    "Custom Hit Script":
                        AddHitScript(npc, ability.ai);
                        break;
                endcase
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
    var hit_scripts := GetObjProperty(npc, "HitScripts");
    if (!hit_scripts)
        hit_scripts := array{};
    endif
    hit_scripts.append(script);
    SetObjProperty(npc, "HitScripts", hit_scripts);
endfunction

// Adds an armor hit script to an NPC
function AddArmorHitScript(npc, script) 
    var hit_scripts := GetObjProperty(npc, "ArmorHitScripts");
    if (!hit_scripts)
        hit_scripts := array{};
    endif
    hit_scripts.append(script);
    SetObjProperty(npc, "ArmorHitScripts", hit_scripts);
endfunction
