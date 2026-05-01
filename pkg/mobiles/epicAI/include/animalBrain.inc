const FOLLOW_CMD := 0xA001;
function AnimalBrain()
    var ev;
    var wanders := 0;
    var last_activity_time := polcore().systime;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    // Build ability tables ONCE at brain activation (script-level vars in brain.src)
    combat_abilities := BuildCombatAbilities(me);
    enter_abilities := BuildEnterAbilities(me);

    SM_TransitionTo("IDLE");

    while (me)
        ev := os::wait_for_event(0);

        // Check FORCE dormant FIRST - before anything else
        if (ShouldGoDormant(last_activity_time))
            return;
        endif

        HandleAnimalChanges(me);

        // Process active states (non-blocking, ONE step per tick)
        var state := SM_GetState();
        if (state == "FLEEING")
            SM_TickFlee();
        elseif (state == "HERDING")
            SM_TickHerding();
        elseif (state == "FOLLOWING")
            SM_TickFollowing();
        elseif (state == "AMBUSH")
            SM_TickAmbush();
        elseif (state == "INVESTIGATING")
            SM_TickInvestigate();
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

        if (me.opponent)
            last_activity_time := polcore().systime;
            if (!CanFight(me, me.opponent))
                HandleAnimalLeftArea(me.opponent);
            else
                if (SM_GetState() != "FLEEING")
                    if (!TickHazardEscape())
                        PursueTarget(me.opponent);
                    endif
                endif
            endif
        endif

        if (ev)
            // Cache flee check once per event (avoids 3x GetObjProperty + vital reads)
            var should_flee := ShouldFlee(me);

            case (ev.type)
                SYSEVENT_ENGAGED:
                    last_activity_time := polcore().systime;
                    AddThreat(ev.source, 0);
                    if (should_flee)
                        SM_TransitionTo("FLEEING", struct{ "target" := ev.source, "steps" := 0 });
                    else
                        AlertAllies(ev.source, me);
                        HandleAnimalCombatEvent(ev.source);
                    endif
                SYSEVENT_DAMAGED:
                    last_activity_time := polcore().systime;
                    AddThreat(ev.source, 0);
                    UpdateThreatList(me, ev);
                    if (SM_GetState() == "AMBUSH")
                        me.hidden := 0;
                        SM_TransitionTo("COMBAT");
                    endif
                    if (should_flee)
                        SM_TransitionTo("FLEEING", struct{ "target" := ev.source, "steps" := 0 });
                    else
                        HandleAnimalCombatEvent(ev.source);
                    endif
                EVID_THREAT_UPDATE:
                    AddThreat(ev.source, ev.amount);
                SYSEVENT_ENTEREDAREA:
                    if (CheckDarknessCreatureSurvival(me))
                        return;
                    endif
                    if (!IsAlly(ev.source))
                        UpdateThreatList(me, ev);
                        AlertAllies(ev.source, me);
                        HandleAnimalEnteredArea(ev.source);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source))
                        UpdateThreatList(me, ev);
                        // Only drop combat state if the leaver was our actual opponent.
                        // Otherwise nearby movement (other players, pets, ally deaths)
                        // would wipe opponent + threat table mid-fight and strand the NPC.
                        if (me.opponent && ev.source.serial == me.opponent.serial)
                            HandleAnimalLeftArea(ev.source);
                        endif
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
                    InjectTauntThreat(ev.source);
                    HandleAnimalCombatEvent(ev.source);
                EVID_NOISE:
                    last_activity_time := polcore().systime;
                    if (!me.opponent && SM_GetState() == "IDLE" && me.alignment != 0)
                        print("[noise rx] " + me.name + " investigating noise from " + ev.source.name);
                        SM_TransitionTo("INVESTIGATING", struct{
                            "noise_x" := ev.noise_x,
                            "noise_y" := ev.noise_y
                        });
                    endif
                EVID_PEACEMADE:
                    last_activity_time := polcore().systime;
                    SetWarMode(0);
                    SetOpponent(0);
                    sleep(1);
                EVID_FLEE:
                    last_activity_time := polcore().systime;
                    SM_TransitionTo("FLEEING", struct{ "target" := ev.source, "steps" := 0 });
                EVID_HERDING:
                    PrintTextAbove(me, "*pastoreado*");
                    SM_TransitionTo("HERDING", struct{ "target" := ev.data });
                FOLLOW_CMD:
                    PrintTextAbove(me, "*segue*");
                    SM_TransitionTo("FOLLOWING", struct{ "target" := ev.source });
            endcase
        endif

        // Idle behavior when no events and no combat
        if (!me.opponent && SM_GetState() == "IDLE")
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
        if (me.opponent || SM_GetState() != "IDLE" || (polcore().systime - last_activity_time) < 5)
            sleepms(100); // Combat mode, active state, or recently active
        else
            sleepms(1000); // Idle mode - save CPU
        endif
    endwhile

endfunction

function HandleAnimalEnteredArea(source)
    if (!source)
        return 0;
    endif

    // Dispatch enter-area abilities from pre-built table (Charge, TunnelTactics, Ambush)
    TickEnterAbilities(me, source, enter_abilities);

    HandleAnimalCombatEvent(source);

    return 1;
endfunction

function HandleAnimalCombatEvent(opponent)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
        return 0;
    endif

    // Rural Life is player-only, so on pets we check the master, on players the player itself.
    var rural_target := opponent.master ?: opponent;
    if (MobileWrapper(rural_target).HasAbility(ABILITY_ID::RURAL_LIFE))
        return 0;
    endif

    // Check for health-based teleport
    CheckHealthTeleport(me);

    if (NPC_GetCachedData(me).npc_race == "aranha" && opponent.getRace() == DROW)
        var tier := CInt(me.getTier());
        if (tier <= 2)
            return 0;
        elseif (tier <= 3 && opponent.getCulture() == HAROLOTH)
            return 0;
        elseif (tier <= 4 && opponent.getReligion() == ELUITHORTH)
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
    
    // Dispatch abilities from pre-built table (replaces 9+ sequential if-checks)
    TickCombatAbilities(me, opponent, combat_abilities);

    if (ShouldFlee(me))
        SM_TransitionTo("FLEEING", struct{ "target" := opponent, "steps" := 0 });
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
                // Phase 4: Use cached scan instead of direct ListMobilesNearLocationEx
                foreach mobile in GetNearbyMobilesOnce(me, AREA_SIZE, LISTEX_FLAG_NORMAL)
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


    var anchor := anchor_var;
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