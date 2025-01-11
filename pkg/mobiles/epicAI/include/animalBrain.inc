use npc;

function AnimalBrain()
    var ev;
    var wanders := 0;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    while (me)
        ev := os::wait_for_event(0);

        if (me.opponent)
            if (!CanFight(me, me.opponent))
                HandleAnimalDisengage(me.opponent);
            else
                PursueTarget(me.opponent);
            endif
        endif

        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED: 
                    AlertAllies(ev.source, me);
                    HandleAnimalCombatEvent(ev.source);
                SYSEVENT_DAMAGED: 
                    HandleAnimalCombatEvent(ev.source);
                SYSEVENT_ENTEREDAREA:
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        AlertAllies(ev.source, me);
                        CanFight(me, ev.source) ? HandleAnimalEnteredArea(ev.source) : HandleAnimalLeftArea();
                    endif
                SYSEVENT_DISENGAGED:
                    HandleAnimalDisengage(ev.source);
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        HandleAnimalLeftArea();
                    endif
                EVID_ALERT_ALLIES:
                    var opponent_serial := GetObjProperty(me, "#taunt_opponent");
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent && CanFight(me, opponent))
                            EraseObjProperty(me, "#taunt_opponent");
                            HandleAnimalCombatEvent(opponent);
                        endif
                    endif
                EVID_TAUNT:
                    if (CanFight(me, ev.source))
                        HandleAnimalCombatEvent(ev.source);
                    endif
                EVID_BUDDYATTACKED:
                    // Talvez nao precise usar
                default:
                    PrintTextAbove(me, "UNKNOWN EVENT: " + ev.type);
            endcase
        endif

        // Animal-specific behavior
        // AnimalLookAround();
        // AnimalWander();
    endwhile
endfunction

function HandleAnimalEnteredArea(source)
    if (!source || !CanFight(me, source))
        return 0;
    endif

    handleInitialReaction(source);
    if (me.canCharge())
        StartCharge(me, source);
    endif

    if (me.hasTunnelTactics())
        TunnelTactics(me, source);
    endif

    if (me.CanAmbush())
        HandleAmbush(me, source);
    endif

    HandleAnimalCombatEvent(source);

    return 1;
endfunction

function HandleAnimalCombatEvent(opponent)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
        return 0;
    endif

    var z_diff := Abs(me.z - opponent.z);
    if (z_diff > MAX_Z_DIFF)
        return 0;
    endif

    SetOpponent(opponent);
    opponent := CheckForIdealTarget();
    SetOpponent(opponent);

    // Let the combat hook handle the actual attacks
    TurnToward(opponent);
    PursueTarget(opponent);
    
    if (me.hasHiveMind())
        HiveMind(me);
    endif

    if (me.isUmberHulk())
        doConfusion(opponent);
    endif

    if (me.CanBite())
        Bite(me, opponent);
    endif

    if (me.CanTongueGrab())
        TongueGrab(me, opponent);
    endif

    return 1;
endfunction

function HandleAnimalLeftArea()
    // Clear current combat state
    ClearCombatState();
    
    // First look for new opponents
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL)
        if (CanFight(me, mobile))
            HandleAnimalCombatEvent(mobile);
            return 1;
        endif
        sleepms(2);
    endforeach

    // If nothing else to do, go idle
    HandleIdleState();
    return 1;
endfunction

function HandleAnimalDisengage(opponent)
    if (!opponent)
        return 0;
    endif

    ClearCombatState();

    if (opponent.dead)
        var corpse := opponent.GetCorpse();
        if (corpse)

            // First, approach the corpse
            while (Distance(me, corpse) > 1)
                RunToward(corpse);
                sleepms(100);
                
                // Break if corpse disappeared or too far
                if (!corpse || Distance(me, corpse) > 20)
                    return 0;
                endif
            endwhile

        endif
    endif
    
    return 1;
endfunction