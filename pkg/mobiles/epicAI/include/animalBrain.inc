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
                HandleAnimalLeftArea(me.opponent);
            else
                PursueTarget(me.opponent);
            endif
        endif

        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED: 
                    PrintTextAbove(me, "ENGAGED");
                    AlertAllies(ev.source, me);
                    HandleAnimalCombatEvent(ev.source);
                SYSEVENT_DAMAGED: 
                    PrintTextAbove(me, "DAMAGED");
                    HandleAnimalCombatEvent(ev.source);
                SYSEVENT_ENTEREDAREA:
                    PrintTextAbove(me, "ENTERED AREA");
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        AlertAllies(ev.source, me);
                        CanFight(me, ev.source) ? HandleAnimalEnteredArea(ev.source) : HandleAnimalLeftArea(ev.source);
                    endif
                SYSEVENT_LEFTAREA:
                    PrintTextAbove(me, "LEFT AREA");
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        HandleAnimalLeftArea(ev.source);
                    endif
                EVID_ALERT_ALLIES:
                    PrintTextAbove(me, "ALERT ALLIES");
                    var opponent_serial := GetObjProperty(me, "#taunt_opponent");
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent && CanFight(me, opponent))
                            EraseObjProperty(me, "#taunt_opponent");
                            HandleAnimalCombatEvent(opponent);
                        endif
                    endif
                EVID_TAUNT:
                    PrintTextAbove(me, "TAUNT");
                    if (CanFight(me, ev.source))
                        HandleAnimalCombatEvent(ev.source);
                    endif
                EVID_BUDDYATTACKED:
                    // Talvez nao precise usar
                EVID_PEACEMADE:
                    PrintTextAbove(me, "PEACEMADE");
                    SetWarMode( 0 );
                    SetOpponent( 0 );
                    sleep(1);
                default:
                    PrintTextAbove(me, "UNKNOWN EVENT: " + ev.type);
            endcase
        endif

        // If no events and no combat, do idle behavior
        if (!ev && !me.opponent)
            AnimalIdle(wanders);
            if (RandomInt(100) < 30) // 30% chance to wander
                wanders += 1;
                if (wanders >= 30)
                    wanders := 0;
                    HandleIdleState();
                endif
            endif
        endif

        sleepms(100);
    endwhile
endfunction

function HandleAnimalEnteredArea(source)
    if (!source || !CanFight(me, source))
        return 0;
    endif

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

    // Check for health-based teleport
    CheckHealthTeleport(me);

    if (me.isSpider() && opponent.getRace() == DROW)
        var npc_elem := NPC_GetNPCConfig(me);
        var tier := CInt(npc_elem.Tier);
        if (tier <= 2)
            return 0;
        elseif (tier <= 3 && opponent.getCulture == HAROLOTH)
            return 0;
        elseif (tier <= 4 && opponent.getReligion == ELUITHORTH)
            return 0;
        endif
        // Tier 5 sempre desafia drows para testar se são oponentes valiosos
    endif
 
    SetOpponent(opponent);

    // Let the combat hook handle the actual attacks
    TurnToward(opponent);
    PursueTarget(opponent);
    
    if (me.hasHiveMind())
        HiveMind(me);
    endif

    if (me.canDoConfusion())
        doConfusion(opponent);
    endif

    if (me.CanBite())
        Bite(me, opponent);
    endif

    if (me.CanTongueGrab())
        TongueGrab(me, opponent);
    endif

    if (me.CanPoisonBite())
        PoisonBite(me, opponent);
    endif

    if (me.CanClimbWall())
        WallClimb(me);
    endif

    if (me.CanFrighten())
        frightenOpponent(me, opponent);
    endif

    return 1;
endfunction

function HandleAnimalLeftArea(opponent)
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

            if (me.CanDoWebWrap())
                WebWrapCorpse(me);
            endif

        endif
    endif

    // If nothing else to do, go idle
    HandleIdleState();
    return 1;
endfunction

function AnimalIdle(wanders)
   if (me.opponent)
        return;
    endif

    if (wanders > 10)
        // Heal slowly while idle
        var heal_amount := CInt(CDbl(AP_GetVitalMaximumValue(me, HITS)) * 0.01);
        HealDamage(me, heal_amount);
    endif

    // Look for potential targets
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL)
        if (CanFight(me, mobile))
            HandleAnimalCombatEvent(mobile);
            return;
        endif
        sleepms(2);
    endforeach

    var anchor := GetObjProperty(me, "anchor");
    if (CoordinateDistance(me.x, me.y, anchor[1], anchor[2]) > 35)
      PrintText(me, "*Retorna correndo* [Sistema Anti-Exploits]");
      sleep(3);
      MoveObjectToLocation(me, anchor[1], anchor[2], anchor[3], me.realm, MOVEOBJECT_FORCELOCATION);
    endif
    
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

    if (RandomInt(100) < 10) // 10% de chance de criar traps
        if (me.isSpider())
            CreateInvisibleWeb(me); 
        endif

        wander();
        sleep(1);
        if (me.CanLayEggSac())
            LayEggSac(me);
        endif
    endif
endfunction