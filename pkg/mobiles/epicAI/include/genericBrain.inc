include "include/sysEvent";
include ":combat:damage";
include "include/facings";

function GenericBrain()
    var ev;
    var wanders := 0;
    var idle_checks := 0;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    while (me)
        ev := os::wait_for_event(1);
    
        // If we have an opponent, maintain combat position regardless of events
        if (me.opponent)
            idle_checks := 0;
            if (!CanFight(me, me.opponent))
                ClearCombatState();
            else
                var dist := Distance(me, me.opponent);
                // Only pursue if out of range
                if (dist > 1)
                    PursueTarget(me.opponent);
                else
                    TurnToward(me.opponent);
                endif
            endif
        endif
        
        if (ev)
            idle_checks := 0;
            
            case (ev.type)
                SYSEVENT_ENGAGED: 
                    SetOpponent(ev.source);
                    HandleGenericCombatEvent(ev.source);
                SYSEVENT_DAMAGED: 
                    SetOpponent(ev.source);
                    HandleGenericCombatEvent(ev.source);
                SYSEVENT_ENTEREDAREA:
                    if (!IsAlly(ev.source) && CanFight(me, ev.source))
                        SetOpponent(ev.source);
                        HandleGenericCombatEvent(ev.source);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source))
                        ClearCombatState();
                    endif
                default:
                    // Ignore other events
            endcase
        endif

        // Idle behavior when no events and no combat
        if (!me.opponent)
            idle_checks += 1;
            
            if (idle_checks >= 60)
                if (!HasPlayerNear(me))
                    return;
                else
                    idle_checks := 0;
                endif
            endif
            
            GenericIdle(wanders);
            if (RandomInt(100) < 30)
                wanders += 1;
                if (wanders >= 30)
                    wanders := 0;
                endif
            endif
        endif

        sleepms(100);
    endwhile
endfunction

function HandleGenericCombatEvent(opponent)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
        return 0;
    endif
    
    var z_diff := Abs(me.z - opponent.z);
    if (z_diff > 2) // Assuming MAX_Z_DIFF is 2 for simplicity
        return 0;
    endif

    SetOpponent(opponent);

    // Simple attack: turn toward and pursue
    TurnToward(opponent);
    PursueTarget(opponent);

    return 1;
endfunction

function GenericIdle(byref wanders)
    if (me.opponent)
        return;
    endif

    // Look for potential targets
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL, me.realm)
        if (CanFight(me, mobile))
            SetOpponent(mobile);
            HandleGenericCombatEvent(mobile);
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