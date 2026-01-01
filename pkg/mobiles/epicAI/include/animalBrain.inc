use npc;

function AnimalBrain()
    var ev;
    var wanders := 0;
    var last_activity_time := polcore().systime;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    while (me)
        ev := os::wait_for_event(0);

        // Check FORCE dormant FIRST - before anything else
        if (ShouldGoDormant(last_activity_time))
            return;
        endif

        // Kill darkness creatures (undead/Mobnoturno) if not in darkness
        if (CheckDarknessCreatureSurvival(me))
            return;
        endif

        HandleAnimalChanges(me);

        if (me.opponent)
            last_activity_time := polcore().systime;
            if (!CanFight(me, me.opponent))
                HandleAnimalLeftArea(me.opponent);
            else
                PursueTarget(me.opponent);
            endif
        endif

        if (ev)            
            case (ev.type)
                SYSEVENT_ENGAGED:
                    last_activity_time := polcore().systime;
                    AlertAllies(ev.source, me);
                    HandleAnimalCombatEvent(ev.source);
                SYSEVENT_DAMAGED:
                    last_activity_time := polcore().systime;
                    HandleAnimalCombatEvent(ev.source);
                SYSEVENT_ENTEREDAREA:
                    if (!IsAlly(ev.source))
                        AlertAllies(ev.source, me);
                        HandleAnimalEnteredArea(ev.source);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source))
                        HandleAnimalLeftArea(ev.source);
                    endif
                EVID_ALERT_ALLIES:
                    var opponent_serial := Cint(GetObjProperty(me, "#taunt_opponent"));
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent)
                            HandleAnimalCombatEvent(opponent);
                            EraseObjProperty(me, "#taunt_opponent");
                        endif
                    endif
                EVID_TAUNT:
                    last_activity_time := polcore().systime;
                    SetOpponent(0);
                    HandleAnimalCombatEvent(ev.source);
                EVID_PEACEMADE:
                    last_activity_time := polcore().systime;
                    SetWarMode(0);
                    SetOpponent(0);
                    sleep(1);
                EVID_FLEE:
                    last_activity_time := polcore().systime;
                    FleeFrom(ev.source);
            endcase
        endif

        // Idle behavior when no events and no combat
        if (!me.opponent)
            AnimalIdle(wanders);
            if (RandomInt(100) < 30)
                wanders += 1;
                if (wanders >= 30)
                    wanders := 0;
                    HandleIdleState();
                endif
            endif
        endif

        // Dynamic sleep: fast in combat, slow when idle
        if (me.opponent || (polcore().systime - last_activity_time) < 5)
            sleepms(100); // Combat mode or recently active
        else
            sleepms(1000); // Idle mode - save CPU
        endif
    endwhile
endfunction

function HandleAnimalEnteredArea(source)
    if (!source)
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

    // Check for health-based teleport
    CheckHealthTeleport(me);

    if (me.isSpider() && opponent.getRace() == DROW)
        var tier := CInt(me.getTier());
        if (tier <= 2)
            return 0;
        elseif (tier <= 3 && opponent.getCulture == HAROLOTH)
            return 0;
        elseif (tier <= 4 && opponent.getReligion == ELUITHORTH)
            return 0;
        endif
        // Tier 5 sempre desafia drows para testar se são oponentes valiosos
    endif

    // Check if animal is taunted - if so, only fight the taunter
    if (GetCooldown(me, "taunted") > 0)
        var taunter_serial := GetObjProperty(me, "#taunter");
        if (taunter_serial)
            var taunter := SystemFindObjectBySerial(taunter_serial);
            if (taunter && CanFight(me, taunter))
                // Forced to fight the taunter, ignore all other targets
                opponent := taunter;
            else
                // Taunter is no longer valid, clear taunt
                EraseObjProperty(me, "#taunter");
                SetCooldown(me, "taunted", 0);
            endif
        endif
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
    // TODO, add dragons here
    if (me.CanFrighten())
        frightenOpponent(me, opponent);
    endif

    if (me.canPetrify())
        petrify(me, opponent);
    endif

    if (ShouldFlee(me))
        FleeFrom(opponent);
        return 1;
    endif

    return 1;
endfunction

function HandleAnimalLeftArea(opponent)
    // Clear current combat state
    ClearCombatState();

    // Check for nearby threats first
    var threat := CheckNearbyThreats();
    if (threat)
        HandleAnimalCombatEvent(threat);
        return 1;
    endif

    // Only interact with corpses if this animal actually does something with them
    if (opponent.dead && me.CanDoWebWrap())
        var corpse := opponent.GetCorpse();
        if (corpse)
            // Approach the corpse, but check for threats while approaching
            while (Distance(me, corpse) > 1)
                // Check for nearby hostiles before continuing
                foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL, me.realm)
                    if (CanFight(me, mobile))
                        HandleAnimalCombatEvent(mobile);
                        return 1;
                    endif
                    sleepms(2);
                endforeach

                RunToward(corpse);
                sleepms(100);

                // Break if corpse disappeared or too far
                if (!corpse || Distance(me, corpse) > 20)
                    return 0;
                endif
            endwhile

            WebWrapCorpse(me);
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

    // Removed idle target scanning - rely on ENTEREDAREA events instead (100% perf boost)


    var anchor := GetObjProperty(me, "anchor");
    if (CoordinateDistance(me.x, me.y, anchor[1], anchor[2]) > 35)
      PrintTextAbove(me, "*Retorna correndo* [Sistema Anti-Exploits]");
      sleep(3);
      MoveObjectToLocation(me, anchor[1], anchor[2], anchor[3], me.realm, MOVEOBJECT_FORCELOCATION);
    endif
    
    WanderAroundAnchor(me);

    if (RandomInt(100) < 10) // 10% de chance de criar traps
        if (me.isSpider())
            CreateInvisibleWeb(me); 
        endif

        WanderAroundAnchor(me);
        sleep(1);
        if (me.CanLayEggSac())
            LayEggSac(me);
        endif
    endif
endfunction

function HandleAnimalChanges(me)
    	if (me.graphic == 223)
			if (GetCooldown(me, "wool") == 0)
				me.graphic := 207;
			endif
		endif
endfunction