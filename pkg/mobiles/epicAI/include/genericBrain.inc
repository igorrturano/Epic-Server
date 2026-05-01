include "include/sysEvent";
include ":combat:Controller/DamageController_legacy";
include "include/facings";

function GenericBrain()
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

        // Damage-weighted threat: decay table + reevaluate target if hysteresis
        // margin is exceeded. DecayThreat self-throttles to 1Hz.
        DecayThreat();
        if (ShouldReevaluateTarget())
            var threat_top := GetTopThreat();
            if (threat_top && threat_top != me.opponent)
                SetOpponent(threat_top);
            endif
        endif

        // If we have an opponent, maintain combat position regardless of events
        if (me.opponent)
            last_activity_time := polcore().systime;
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
            case (ev.type)
                SYSEVENT_ENGAGED:
                    AddThreat(ev.source, 0);
                    SetOpponent(ev.source);
                    HandleGenericCombatEvent(ev.source);
                SYSEVENT_DAMAGED:
                    last_activity_time := polcore().systime;
                    AddThreat(ev.source, 0);
                    UpdateThreatList(me, ev);
                    SetOpponent(ev.source);
                    HandleGenericCombatEvent(ev.source);
                EVID_THREAT_UPDATE:
                    AddThreat(ev.source, ev.amount);
                SYSEVENT_ENTEREDAREA:
                    if (!IsAlly(ev.source))
                        // Phase 3: Update threat list cache
                        UpdateThreatList(me, ev);
                        SetOpponent(ev.source);
                        HandleGenericCombatEvent(ev.source);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source))
                        // Phase 3: Update threat list cache
                        UpdateThreatList(me, ev);
                        // Only drop combat state if the leaver was our actual opponent.
                        // Otherwise nearby movement (other players, pets, ally deaths)
                        // would wipe opponent + threat table mid-fight and strand the NPC.
                        if (me.opponent && ev.source.serial == me.opponent.serial)
                            ClearCombatState();
                        endif
                    endif
                default:
                    // Ignore other events
            endcase
        endif

        // Idle behavior when no events and no combat
        if (!me.opponent)
            GenericIdle(wanders);
            if (RandomInt(100) < 30)
                wanders += 1;
                if (wanders >= 30)
                    wanders := 0;
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

function HandleGenericCombatEvent(opponent)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
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

    // Removed idle target scanning - rely on ENTEREDAREA events instead (100% perf boost)

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