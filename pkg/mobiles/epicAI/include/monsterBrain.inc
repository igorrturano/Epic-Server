use npc;
include "include/sysEvent";
include "include/sounds";
include ":combat:damage";
include "include/facings";

var me := Self();
const AREA_SIZE := 16;
var combat_cooldown := 0;

function MonsterBrain()
    var ev;
    var wanders := 0;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    while (me)
        ev := os::wait_for_event(200);
        
        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED: HandleCombatEvent(ev.source);
                SYSEVENT_DAMAGED: HandleCombatEvent(ev.source);
                SYSEVENT_ENTEREDAREA: 
                    if (CanFight(me, ev.source))
                        HandleCombatEvent(ev.source);
                    endif
                SYSEVENT_OPPONENT_MOVED: HandleOpponentMovement(ev.source);
                SYSEVENT_SPEECH: HandleSpeech(ev.text, ev.source);
                SYSEVENT_DISENGAGED: HandleDisengage(ev.source);
                EVID_ALERT_ALLIES: 
                    if (ev.source)
                        HandleCombatEvent(ev.source);
                    endif
                EVID_FLEE:
                    if (ev.source)
                        FleeFrom(ev.source);
                    endif
            endcase
        endif

        sleepms(100);
        MonsterIdle(wanders);
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

function HandleSpeech(text, source)
    // Basic speech handling - could be expanded for specific responses
    if (!source || !CanFight(me, source))
        return 0;
    endif

    // Chance to aggro on speech
    if (RandomInt(100) < 30)
        HandleCombatEvent(source);
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

function HandleCombatEvent(opponent)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
        return 0;
    endif

    SetWarMode(1);
    SetOpponent(opponent);
    TurnToward(opponent);

    // Basic combat loop
    while (opponent && !opponent.dead && !me.dead)
        if (Distance(me, opponent) > 1)
            PursueTarget(opponent);
        else
            // We're in range - handle actual combat
            if (ReadGameClock() > combat_cooldown)
                PerformCombatAction(opponent);
                combat_cooldown := ReadGameClock() + 2; // Basic attack cooldown
            endif
        endif
        
        sleepms(100);
    endwhile

    SetWarMode(0);
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

function PerformCombatAction(opponent)
    if (!opponent)
        return 0;
    endif

    // Basic attack
    TurnToward(opponent);
    PerformAction(me, 0x0A);
    PlaySoundEffect(me, 0x23B);
    
    return 1;
endfunction

function MonsterIdle(wanders)
    if (wanders > 10)
        // Heal slowly while idle
        var heal_amount := CInt(CDbl(AP_GetVitalMaximumValue(me, HITS)) * 0.01);
        HealDamage(me, heal_amount);
    endif

    // Look for potential targets
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL|LISTEX_FLAG_HIDDEN)
        if (CanFight(me, mobile))
            HandleCombatEvent(mobile);
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
endfunction

function CanFight(attacker, defender)
    if (!defender || defender.dead || defender.hidden || defender.concealed)
        return 0;
    elseif (defender.npctemplate == attacker.npctemplate)
        return 0;
    endif
    
    return 1;
endfunction