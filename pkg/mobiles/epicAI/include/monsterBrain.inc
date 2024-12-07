use npc;
include "include/sysEvent";
include "include/sounds";
include ":combat:damage";
include "include/facings";
include ":npcs:thor/handling/thorai";

var me := Self();
const AREA_SIZE := 16;
var combat_cooldown := 0;

function MonsterBrain()
    var ev;
    var wanders := 0;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    // Get monster configuration
    var npc_elem := NPC_GetNPCConfig(me);
    var speed_type := SplitWords(GetConfigString(npc_elem, "EpicAttackSpeed"), "_");
    var is_ranged := (speed_type[1] == "RANGED");

    // Enable archer-specific abilities if needed
    if (is_ranged)
        GrantPrivilege(me, "firewhilemoving");
        me.enable("firewhilemoving");
    endif

    while (me)
        ev := os::wait_for_event(200);
        
        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED: HandleCombatEvent(ev.source, is_ranged);
                SYSEVENT_DAMAGED: HandleCombatEvent(ev.source, is_ranged);
                SYSEVENT_ENTEREDAREA: 
                    if (CanFight(me, ev.source))
                        HandleEnteredArea(ev.source, is_ranged);
                    endif
                SYSEVENT_OPPONENT_MOVED:                     
                    if (me.opponent == ev.source)
                        if (is_ranged)
                            MaintainRange(ev.source);
                        else
                            TurnToward(ev.source);
                            PursueTarget(ev.source);
                        endif
                    endif
                SYSEVENT_SPEECH: HandleSpeech(ev.text, ev.source, is_ranged);
                SYSEVENT_DISENGAGED: HandleDisengage(ev.source);
                EVID_ALERT_ALLIES: 
                    if (ev.source)
                        HandleCombatEvent(ev.source, is_ranged);
                    endif
                EVID_FLEE:
                    if (ev.source)
                        FleeFrom(ev.source);
                    endif
            endcase
        endif

        sleepms(100);
        MonsterIdle(wanders, is_ranged);
        wanders += 1;

        if (wanders >= 30)
            wanders := 0;
            HandleIdleState();
        endif
    endwhile
endfunction

function HandleOpponentMovement(opponent)
    if (!opponent || !CanFight(me, opponent))
        return 0;
    endif

    // Update combat state if opponent moves
    if (me.opponent == opponent)
        TurnToward(opponent);
        PursueTarget(opponent);
    endif
    
    return 1;
endfunction

function HandleSpeech(text, source, is_ranged)
    // Basic speech handling - could be expanded for specific responses
    if (!source || !CanFight(me, source))
        return 0;
    endif

    // Chance to aggro on speech
    if (RandomInt(100) < 30)
        HandleCombatEvent(source, is_ranged);
    endif
    
    return 1;
endfunction

function HandleDisengage(opponent)
    if (!opponent)
        return 0;
    endif

    // Clear combat state
    SetWarMode(0);
    SetOpponent(0);
    
    // Check for loot if opponent is dead
    if (opponent.dead)
        // Add looting behavior here if desired
    endif
    
    return 1;
endfunction

function DisableMainEvents()
    DisableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA);
    DisableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED);
endfunction

function HandleCombatEvent(opponent, is_ranged)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
        return 0;
    endif

    const MAX_Z_DIFF := 18;
    var z_diff := Abs(me.z - opponent.z);
    if (z_diff > MAX_Z_DIFF)
        return 0;
    endif

    SetOpponent(opponent);
    TurnToward(opponent);

    // Let the combat hook handle the actual attacks
    if (is_ranged)
        MaintainRange(opponent);
    endif

    return 1;
endfunction

function PursueTarget(opponent)
    if (!opponent)
        return 0;
    endif

    var dist := Distance(me, opponent);
    
    // Don't chase too far
    if (dist > AREA_SIZE)
        return 0;
    endif

    // Basic pathfinding approach
    if (CheckLineOfSight(me, opponent))
        RunToward(opponent);
    else
        // Try to find a path around obstacles
        var path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES);
        if (path)
            foreach coord in path
                RunTowardLocation(coord.x, coord.y);
                if (Distance(me, opponent) <= 1)
                    break;
                endif
                sleepms(100);
            endforeach
        endif
    endif
endfunction

function PerformRangedAttack(opponent)
    if (!opponent || !CheckLineOfSight(me, opponent))
        return 0;
    endif

    TurnToward(opponent);
    PerformAction(me, 0x12); // Bow animation
    PlaySoundEffect(me, 0x235); // Bow sound
    
    return 1;
endfunction

function PerformMeleeAttack(opponent)
    if (!opponent)
        return 0;
    endif

    TurnToward(opponent);
    PerformAction(me, 0x0A);
    PlaySoundEffect(me, 0x23B);
    
    return 1;
endfunction

function MonsterIdle(wanders, is_ranged)
    if (wanders > 10)
        // Heal slowly while idle
        var heal_amount := CInt(CDbl(AP_GetVitalMaximumValue(me, HITS)) * 0.01);
        HealDamage(me, heal_amount);
    endif

    // Look for potential targets
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL|LISTEX_FLAG_HIDDEN)
        if (CanFight(me, mobile))
            HandleCombatEvent(mobile, is_ranged);
            return;
        endif
        sleepms(2);
    endforeach
    
    // Random wandering when no targets
    if (RandomInt(100) < 30)
        var direction := RandomInt(8);
        case (direction)
            0: WalkTowardLocation(me.x, me.y-1);
            1: WalkTowardLocation(me.x+1, me.y-1);
            2: WalkTowardLocation(me.x+1, me.y);
            3: WalkTowardLocation(me.x+1, me.y+1);
            4: WalkTowardLocation(me.x, me.y+1);
            5: WalkTowardLocation(me.x-1, me.y+1);
            6: WalkTowardLocation(me.x-1, me.y);
            7: WalkTowardLocation(me.x-1, me.y-1);
        endcase
    endif
endfunction

function HandleIdleState()
    DisableMainEvents();
    EnableEvents(SYSEVENT_ENTEREDAREA + SYSEVENT_ENGAGED + SYSEVENT_DAMAGED, AREA_SIZE);
    
    // Return to spawn point if we have one
    var spawn_point := GetObjProperty(me, "spawn_loc");
    if (spawn_point)
        if (Distance(me, spawn_point) > 5)
            WalkTowardLocation(spawn_point.x, spawn_point.y);
        endif
    endif
endfunction

function FleeFrom(opponent)
    if (!opponent)
        return 0;
    endif

    var runs := 0;
    while (opponent && Distance(me, opponent) < 20 && runs < 50)
        RunAwayFrom(opponent);
        runs += 1;
        sleepms(100);
    endwhile
endfunction

function EnableMainEvents()
    EnableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA, AREA_SIZE);
    EnableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED, AREA_SIZE);
    EnableEvents(SYSEVENT_SPEECH, 4);
endfunction

function CanFight(attacker, defender)
    if (!defender || defender.dead || defender.hidden || defender.concealed)
        return 0;
    elseif (defender.npctemplate == attacker.npctemplate)
        return 0;
    endif
    
    return 1;
endfunction

function HandleEnteredArea(source, is_ranged)
    if (!source || !CanFight(me, source))
        return 0;
    endif

    // Get monster category and player race
    var npc_elem := NPC_GetNPCConfig(me);
    var category := GetConfigString(npc_elem, "Category");
    var playerRaca := source.getRace();
    var name_mob := GetConfigString(npc_elem, "Name");
    
    if (!category)
        HandleCombatEvent(source, is_ranged);
        return 0;
    endif

    // Special handling for Goblins
    if (category == "Goblin")
        if (playerRaca == ORC || playerRaca == DROW)
            // High chance to flee from these races
            var randomDice := RandomInt(100);
            if (randomDice < 75)
                var playerinput := "*raça " + playerRaca + " assusta*";
                var responseAI := handle_combat_ai(category, name_mob, playerinput, playerRaca);
                if (responseAI)
                    Say(responseAI);
                endif
                FleeFrom(source);
                return 1;
            endif
        endif
    endif

    // Initial reaction based on category
    var aggro_chance;
    var playerinput;
    
    case (category)
        "Goblin": 
            aggro_chance := 20;
            playerinput := "*raça " + playerRaca + " encontrada*";
        "Giants": 
            aggro_chance := 70;
            playerinput := "*raça " + playerRaca + " invade território*";
        "EvilHuman": 
            aggro_chance := 60;
            playerinput := "*raça " + playerRaca + " avistada*";
        default: 
            aggro_chance := 40;
            playerinput := "*raça " + playerRaca + " aparece*";
    endcase

    // Generate and say initial response
    var responseAI := handle_combat_ai(category, name_mob, playerinput, playerRaca);
    if (responseAI)
        Say(responseAI);
    endif

    // Roll for aggro
    if (RandomInt(100) < aggro_chance)
        HandleCombatEvent(source, is_ranged);
    endif
    
    return 1;
endfunction

function MaintainRange(opponent)
    var sleepdelay := 275 - me.run_speed;
    if (sleepdelay < 0)
        sleepdelay := 50;
    endif

    var dist := CoordinateDistance(me.x, me.y, opponent.x, opponent.y);
    var z_diff := Abs(me.z - opponent.z);

    if (z_diff > 18)
        return 0;
    endif

    if (dist < 4)
        RunAwayFrom(opponent);
        sleepms(sleepdelay);
    elseif (dist > 10)
        RunToward(opponent);
        sleepms(sleepdelay);
    else
        TurnToward(opponent);
        return 1;
    endif

    return 0;
endfunction