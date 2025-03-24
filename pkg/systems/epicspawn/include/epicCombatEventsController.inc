/* 
 * epicCombatEvents.inc
 *
 * Core functions for executing boss combat events and actions
 *
 */
 
use uo;
use os;
use util;
use math;

include ":damage:damage";
include ":epicspawn:epicBoss";
/*
 * ExecuteCombatEvent(npc, event, actions)
 * Purpose: Executes a combat event with its actions
 * Parameters: npc - The NPC executing the event
 *             event - The event configuration
 *             actions - The actions to perform
 * Return: 1 if executed successfully, 0 if not
 */
function ExecuteCombatEvent(npc, event, actions)
    print("DEBUG: Starting combat event: " + event.name);
    
    // Process each action in sequence
    foreach action in actions
        if (!ExecuteCombatAction(npc, action))
            print("DEBUG: Failed to execute action type: " + action.type);
            continue; // Try the next action
        endif
        
        // Sleep briefly between actions to allow for animations etc.
        sleepms(200);
    endforeach
    
    return 1;
endfunction

/*
 * ExecuteCombatAction(npc, action)
 * Purpose: Executes a single combat action
 * Parameters: npc - The NPC executing the action
 *             action - The action configuration
 * Return: 1 if executed successfully, 0 if not
 */
function ExecuteCombatAction(npc, action)
    if (!action.type)
        return 0;
    endif
    
    print("DEBUG: Executing action type: " + action.type);
    
    // Get targets based on action's targ property
    var targets := GetActionTargets(npc, action);
    
    // Execute the action based on its type
    case (action.type)
        "Move":
            return ExecuteMoveAction(npc, action);
            
        "Teleport":
            return ExecuteTeleportAction(npc, targets, action);
            
        "Perform Action":
            return ExecutePerformAction(targets, action);
            
        "Text":
            return ExecuteTextAction(npc, action);
            
        "Play Sound":
            return ExecutePlaySoundAction(npc, action);
            
        "Delay":
            return ExecuteDelayAction(action);
            
        "Heal":
            return ExecuteHealAction(targets, action);
            
        "Dmg":
            return ExecuteDamageAction(targets, action);
            
        "Graphic Effect":
            return ExecuteGraphicEffectAction(npc, targets, action);
            
        "Summon Npcs":
            return ExecuteSummonNpcsAction(npc, action);
            
        "Create Items":
            return ExecuteCreateItemsAction(npc, action);
            
        default:
            print("DEBUG: Unknown action type: " + action.type);
            return 0;
    endcase
    
    return 0;
endfunction

/*
 * GetActionTargets(npc, action)
 * Purpose: Gets targets for an action based on its targ property
 * Parameters: npc - The NPC executing the action
 *             action - The action configuration
 * Return: Array of targets
 */
function GetActionTargets(npc, action)
    var targets := array{};
    
    // Determine targets based on the action's targ property
    if (!action.targ || action.targ == "Self")
        targets.append(npc);
    elseif (action.targ == "Opponent")
        var opponent := npc.opponent;
        if (opponent)
            targets.append(opponent);
        endif
    elseif (action.targ == "All Opponents")
        targets := GetAllOpponents(npc);
    endif
    
    return targets;
endfunction

/* 
 * Action Implementation Functions
 */

function ExecuteMoveAction(npc, action)
    if (!action.x || !action.y || !action.z)
        return 0;
    endif
    
    print("DEBUG: Moving to " + action.x + ", " + action.y + ", " + action.z);
    return npc.WalkToLocation(action.x, action.y, action.z);
endfunction

function ExecuteTeleportAction(npc, targets, action)
    if (!action.x || !action.y || !action.z)
        return 0;
    endif
    
    var success := 0;
    foreach targ in targets
        print("DEBUG: Teleporting " + targ.name + " to " + action.x + ", " + action.y + ", " + action.z);
        if (MoveObjectToLocation(targ, action.x, action.y, action.z, targ.realm, MOVEOBJECT_FORCELOCATION))
            success := 1;
        endif
    endforeach
    
    return success;
endfunction

function ExecutePerformAction(targets, action)
    if (!action.id)
        return 0;
    endif
    
    var success := 0;
    foreach targ in targets
        print("DEBUG: Performing action " + action.id + " on " + targ.name);
        PerformAction(targ, CInt(action.id));
        success := 1;
    endforeach
    
    return success;
endfunction

function ExecuteTextAction(npc, action)
    print("DEBUG: action: " + action);
    if (!action.text)
        return 0;
    endif
    
    // Default color if not specified
    var color := action.color ? CInt(action.color) : 1153;
    if (action.target)
        var targets := GetActionTargets(npc, action);
        foreach targ in targets
            PrintTextAbove(targ, action.text, color);
        endforeach
    else
        PrintTextAbove(npc, action.text, color);
    endif

    return 1;
endfunction

function ExecutePlaySoundAction(npc, action)
    if (!action.id)
        return 0;
    endif
    
    print("DEBUG: Playing sound " + action.id);
    PlaySoundEffect(npc, CInt(action.id));
    return 1;
endfunction

function ExecuteDelayAction(action)
    var delay_time := action.delay ? CInt(action.delay) : 3;
    print("DEBUG: Delaying for " + delay_time + " seconds");
    sleep(delay_time);
    return 1;
endfunction

function ExecuteHealAction(targets, action)
    if (!action.healamt)
        return 0;
    endif
    
    var heal_amount := CInt(action.healamt);
    var success := 0;
    
    foreach targ in targets
        print("DEBUG: Healing " + targ.name + " for " + heal_amount);
        HealDamage(targ, heal_amount);
        PlayObjectCenteredEffect(targ, 0x376A, 10, 10);
        success := 1;
    endforeach
    
    return success;
endfunction

function ExecuteDamageAction(targets, action)
    if (!action.dmg)
        return 0;
    endif
    
    var damage := CInt(action.dmg);
    var damage_type := action.dmgtype ? action.dmgtype : "Physical";
    var success := 0;
    
    foreach targ in targets
        print("DEBUG: Damaging " + targ.name + " for " + damage + " " + damage_type);
        
        // Apply damage
        ApplyRawDamageEX(targ, damage, damage_type, "Combat Event");
        success := 1;
    endforeach
    
    return success;
endfunction

function ExecuteGraphicEffectAction(npc, targets, action)
    if (!action.fxid)
        return 0;
    endif
    
    var fx_id := CInt(action.fxid);
    var speed := action.speed ? CInt(action.speed) : 7;
    var loops := action.loops ? CInt(action.loops) : 5;
    var success := 0;
    
    print("DEBUG: Creating graphic effect " + fx_id);
    
    if (action.graphictype == "Moving")
        // Moving effect from NPC to targets
        foreach targ in targets
            PlayMovingEffect(npc, targ, fx_id, speed, loops);
            success := 1;
        endforeach
    else
        // Stationary effect on each targ
        foreach targ in targets
            PlayObjectCenteredEffect(targ, fx_id, speed, loops);
            success := 1;
        endforeach
    endif
    
    return success;
endfunction

function ExecuteSummonNpcsAction(npc, action)
    if (!action.npctemplate)
        return 0;
    endif
    
    var template := action.npctemplate;
    var amount := action.amount ? CInt(action.amount) : 3;
    var range := action.range ? CInt(action.range) : 5;
    var lifetime := action.tempo ? CInt(action.tempo) : 120;
    var success := 0;
    
    print("DEBUG: Summoning " + amount + " " + template + " NPCs");
    
    // Determine spawn location
    var spawn_x, spawn_y, spawn_z;
    if (action.x && action.y && action.z && 
        action.x != 0 && action.y != 0 && 
        !(action.x == 6 && action.y == 6))
        // Use specified location
        spawn_x := action.x;
        spawn_y := action.y;
        spawn_z := action.z;
    else
        // Use NPC location
        spawn_x := npc.x;
        spawn_y := npc.y;
        spawn_z := npc.z;
    endif
    
    // Spawn the NPCs
    for i := 1 to amount
        // Find a valid spawn point
        var loc := FindValidLocation(npc, spawn_x, spawn_y, spawn_z, range);
        if (loc)
            var summoned := CreateNpcFromTemplate(template, loc.x, loc.y, loc.z, 0, npc.realm);
            if (summoned && !summoned.errortext)
                // Set summoned property
                SetObjProperty(summoned, "summoned", 1);
                SetObjProperty(summoned, "summoner", npc.serial);
                
                // Set lifetime if specified
                if (lifetime > 0)
                    SetObjProperty(summoned, "lifetime", ReadGameClock() + lifetime);
                endif
                
                // If boss is in combat, set summoned to attack the same targ
                if (npc.opponent)
                    summoned.SetOpponent(npc.opponent);
                endif
                
                success := 1;
            endif
        endif
    endfor
    
    return success;
endfunction

function ExecuteCreateItemsAction(npc, action)
    if (!action.objtype)
        return 0;
    endif
    
    var objtype := action.objtype;
    var amount := action.amount ? CInt(action.amount) : 1;
    var success := 0;
    
    print("DEBUG: Creating " + amount + " items of type " + objtype);
    
    // Create the item at the NPC's feet
    var item := CreateItemAtLocation(npc.x, npc.y, npc.z, objtype, amount, npc.realm);
    if (item)
        success := 1;
    endif
    
    return success;
endfunction

/*
 * Helper Functions
 */

function FindValidLocation(npc, x, y, z, range)
    var attempts := 0;
    var max_attempts := 10;
    
    while (attempts < max_attempts)
        var try_x := x + RandomInt(range*2+1) - range;
        var try_y := y + RandomInt(range*2+1) - range;
        var try_z := GetMapInfo(try_x, try_y, npc.realm).z;

        if (CheckLosAt(npc, try_x, try_y, try_z))
            return struct{ "x" := try_x, "y" := try_y, "z" := try_z };
        endif
        
        attempts += 1;
    endwhile
    
    return 0;
endfunction