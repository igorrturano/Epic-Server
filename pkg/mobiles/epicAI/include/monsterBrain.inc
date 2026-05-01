include "include/sysEvent";
include "include/sounds";
include ":combat:Controller/DamageController_legacy";
include "include/facings";
include ":npcs:thor/handling/thorai";
include "epicSpecial";
include "NPCMageryUtils";

function MonsterBrain()
    var ev;
    var wanders := 0;
    var last_activity_time := polcore().systime;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    var is_ranged := isMonsterRanged();

    if (is_ranged)
        GrantPrivilege(me, "firewhilemoving");
        me.enable("firewhilemoving");
    endif

    // Grant stealth privileges once for Ladino monsters
    if (me.isThief())
        GrantPrivilege(me, "hiddenattack");
        me.enable("hiddenattack");
        GrantPrivilege(me, "runwhilestealth");
        me.enable("runwhilestealth");
    endif

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

        // Process active states (non-blocking, ONE step per tick)
        var state := SM_GetState();
        if (state == "FLEEING")
            SM_TickFlee();
        elseif (state == "AMBUSH")
            SM_TickAmbush();
        elseif (state == "STEALING")
            SM_TickStealing();
        elseif (state == "SNEAKING")
            SM_TickSneaking();
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
                // Loot on kill: if opponent died (not just left area), loot their corpse
                if (me.opponent.dead && !GetCooldown(me, "loot_cooldown"))
                    foreach item in ListItemsNearLocation(me.x, me.y, me.z, 5, me.realm)
                        if (item.IsA(POLCLASS_CORPSE))
                            if (LootCorpse(item))
                                SetCooldown(me, "loot_cooldown", 300);
                                break;
                            endif
                        endif
                        sleepms(2);
                    endforeach
                endif
                HandleMonsterLeftArea(is_ranged);
            else
                if (GetObjProperty(me, "metamorfose"))
                    is_ranged := 0;
                endif
                if (SM_GetState() != "FLEEING")
                    if (!TickHazardEscape())
                        PursueTarget(me.opponent);
                    endif
                endif
                // Per-tick casting — keep offensive pressure on opponent
                TickCombatAbilities(me, me.opponent, combat_abilities);
                var tick_cache := NPC_GetCachedData(me);
                if (tick_cache.spell_school == "Shaman")
                    ShamanBehavior(me);
                elseif (tick_cache.spell_school == "Druid")
                    DruidBehavior(me, me.opponent);
                endif
            endif
        endif

        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED:
                    AddThreat(ev.source, 0);
                    AlertAllies(ev.source, me);
                    HandleMonsterCombatEvent(ev.source, is_ranged);
                SYSEVENT_DAMAGED:
                    last_activity_time := polcore().systime;
                    // Safety net for damage paths that bypass AttackLogger.
                    AddThreat(ev.source, 0);
                    UpdateThreatList(me, ev);
                    AskForHealing(me);
                    // Break stealth states on damage
                    var dmg_state := SM_GetState();
                    if (dmg_state == "AMBUSH" || dmg_state == "STEALING" || dmg_state == "SNEAKING")
                        me.hidden := 0;
                        SM_TransitionTo("COMBAT");
                    endif
                    HandleMonsterCombatEvent(ev.source, is_ranged);
                EVID_THREAT_UPDATE:
                    AddThreat(ev.source, ev.amount);
                SYSEVENT_ENTEREDAREA:
                    if (CheckDarknessCreatureSurvival(me))
                        return;
                    endif
                    if (!IsAlly(ev.source))
                        UpdateThreatList(me, ev);
                        AlertAllies(ev.source, me);
                        HandleMonsterEnteredArea(ev.source, is_ranged);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source))
                        UpdateThreatList(me, ev);
                        // Only drop combat state if the leaver was our actual opponent.
                        // Otherwise nearby movement (other players, pets, ally deaths)
                        // would wipe opponent + threat table mid-fight and strand the NPC.
                        if (me.opponent && ev.source.serial == me.opponent.serial)
                            HandleMonsterLeftArea(is_ranged);
                        endif
                    endif
                EVID_ALERT_ALLIES:
                    var opponent_serial := Cint(GetObjProperty(me, "#taunt_opponent"));
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent)
                            HandleMonsterCombatEvent(opponent, is_ranged);
                            EraseObjProperty(me, "#taunt_opponent");
                        endif
                    endif
                EVID_TAUNT:
                    last_activity_time := polcore().systime;
                    InjectTauntThreat(ev.source);
                    HandleMonsterCombatEvent(ev.source, is_ranged);
                EVID_BUDDYATTACKED:
                    if (!GetCooldown(me, "buddy_heal"))
                        var hurt_ally := ev.source;
                        if (hurt_ally && !hurt_ally.dead && isHurt(hurt_ally))
                            var heal_cache := NPC_GetCachedData(me);
                            var heal_spell := "Cura Leve";
                            if (heal_cache.tier >= 3)
                                heal_spell := "Cura Grave";
                            endif
                            spell_target_var := hurt_ally.serial;
                            NPC_CastSpell(me, heal_spell);
                            SetCooldown(me, "buddy_heal", 15);
                        endif
                    endif
                EVID_FLEE:
                    last_activity_time := polcore().systime;
                    SM_TransitionTo("FLEEING", struct{ "target" := ev.source, "steps" := 0 });
            endcase
        endif

        // Idle behavior when no events and no combat
        if (!me.opponent && SM_GetState() == "IDLE")
            MonsterIdle(wanders, is_ranged);
            if (RandomInt(100) < 30)
                wanders += 1;
                if (wanders >= 30)
                    wanders := 0;
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

function HandleMonsterSpeech(text, source, is_ranged)
    if (!source || !CanFight(me, source))
        return 0;
    endif
	EraseObjProperty(me, "#taunt_opponent");
    // Evita spam de falas
    if (GetCooldown(me, "combat_speech") > 0)
        return;
    endif

    var npcState := getNPCState(me);

    // Adds diff to check resist based on npcState and Tier
    var diff := getNpcResist();

    // Se perto da morte, pode querer fugir
    if (npcState == "dying")
        if (!CheckResist(me, FORTITUDE, 0, diff))
            PrintTextAbove(me, "*"+me.name+" se acovarda*");
            SM_TransitionTo("FLEEING", struct{ "target" := source, "steps" := 0 });
            return 1;
        endif
    endif

    // // Gera e processa resposta
    // // var response := handle_combat_speech(me.name, text, source.getRace(), npcState);
    // if (response == error)
    //     return Say(handle_error_combat_AI_speech());
    // endif

    // Say(response);
    SetCooldown(me, "combat_speech", 15);
    return 1;
endfunction

function HandleMonsterLeftArea(is_ranged)
    ClearCombatState();

    var threat := CheckNearbyThreats();
    if (threat)
        HandleMonsterCombatEvent(threat, is_ranged);
        return 1;
    endif

    // Corpse looting moved to MonsterIdle to avoid item scans on every LEFTAREA event
    HandleIdleState();
    return 1;
endfunction

function HandleMonsterCombatEvent(opponent, is_ranged)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
        return 0;
    endif
    if (GetObjProperty(me, "metamorfose")) is_ranged := 0; endif

    // Check for health-based teleport
    CheckHealthTeleport(me);

    // Check if NPC is taunted - if so, only fight the taunter
    if (GetCooldown(me, "taunted") > 0)
        var taunter_serial := GetObjProperty(me, "#taunter");
        if (taunter_serial)
            var taunter := SystemFindObjectBySerial(taunter_serial);
            if (taunter && CanFight(me, taunter))
                // Forced to fight the taunter; threat injection from EVID_TAUNT
                // already biases selection in favor of the taunter via hysteresis.
                opponent := taunter;
                SetOpponent(opponent);
            else
                // Taunter is no longer valid, clear taunt
                EraseObjProperty(me, "#taunter");
                SetCooldown(me, "taunted", 0);
            endif
        endif
    else
        // Not taunted - use normal target selection logic
        // Only check for ideal target if we don't have a valid opponent already
        // This prevents constant target switching when receiving multiple SYSEVENT_ENGAGED
        if (!me.opponent)
            SetOpponent(opponent);
            opponent := CheckForIdealTarget();
            SetOpponent(opponent);
        else
            // We already have a valid opponent, stick with them
            opponent := me.opponent;
        endif
    endif

    TurnToward(opponent);
    PursueTarget(opponent);

    // Dispatch abilities from pre-built table (replaces 12+ sequential if-checks)
    TickCombatAbilities(me, opponent, combat_abilities);

    var mon_cache := NPC_GetCachedData(me);
    if (mon_cache.spell_school == "Shaman")
        ShamanBehavior(me);
    elseif (mon_cache.spell_school == "Druid")
        DruidBehavior(me, opponent);
    endif

    if (ShouldFlee(me))
        SM_TransitionTo("FLEEING", struct{ "target" := opponent, "steps" := 0 });
        return 1;
    endif

    return 1;
endfunction

function MonsterIdle(wanders, is_ranged)
    if (me.opponent)
        return;
    endif

    if (wanders > 10)
        // Heal slowly while idle
        var heal_amount := CInt(CDbl(AP_GetVitalMaximumValue(me, HITS)) * 0.01);
        HealDamage(me, heal_amount);
    endif

    if (me.isLizardMan() && !GetCooldown(me, "throw_cooldown") && me.canThrowSpear()) me.graphic := 0x23; endif // Retorna o gráfico de lizard com lança
    var anchor := anchor_var;
    if (CoordinateDistance(me.x, me.y, anchor[1], anchor[2]) > 35)
      PrintTextAbove(me, "*Retorna correndo* [Sistema Anti-Exploits]");
      sleep(3);
      MoveObjectToLocation(me, anchor[1], anchor[2], anchor[3], me.realm, MOVEOBJECT_FORCELOCATION);
     endif

    // Check if NPC should patrol
    var patrol_points := GetObjProperty(me, "patrol_points");
    var current_patrol_point := GetObjProperty(me, "#current_patrol_point");
    if (!current_patrol_point)
        current_patrol_point := 1;
    endif
    
    if (patrol_points && patrol_points.size() >= 2)
        // Handle patrolling
        startPatrolling(me, patrol_points, current_patrol_point);
        return; // Skip regular wandering
    endif
    
    WanderAroundAnchor(me);
endfunction

function HandleMonsterEnteredArea(source, is_ranged)
    if (!source)
        return 0;
    endif

    // Handle doppelganger transformation first
    if (me.isDoppelganger())
        // Hide the doppelganger
        me.hidden := 1;

        // Perform transformation
        if (DoppelgangerTransformation(me, source))
            sleepms(500); // Brief delay for effect
            // Re-evaluate ranged status after transformation
            is_ranged := isMonsterRanged();
            // Enable archer-specific abilities if needed
            if (is_ranged)
                GrantPrivilege(me, "firewhilemoving");
                me.enable("firewhilemoving");
            endif
        endif

        // Reveal the transformed doppelganger
        me.hidden := 0;
    endif

    // Dispatch enter-area abilities from pre-built table (Charge, TunnelTactics, Ambush)
    TickEnterAbilities(me, source, enter_abilities);

    // Thief behavior is monster-specific, not table-dispatched
    if (me.isThief())
        DoThiefCombatBehavior(source);
    endif

    HandleMonsterCombatEvent(source, is_ranged);

    return 1;
endfunction

// function handleMonsterInitialReaction(source)
//     var npc_elem := NPC_GetNPCConfig(me);
//     var environment := GetConfigString(npc_elem, "Environment");

//     if (category == "Goblin")
//         var playerRaca := source.getRace();
//         var name_mob := GetConfigString(npc_elem, "Name");
//         if (playerRaca == ORC || playerRaca == DROW)
//             // High chance to flee from these races
//             var randomDice := RandomInt(100);
//             if (randomDice < 30)
//                 var playerinput := "*raça " + playerRaca + " assusta*";
//                 var responseAI := handle_combat_ai(category, name_mob, playerinput, playerRaca);
//                 if (responseAI)
//                     Say(responseAI);
//                 else
//                     Say("Ahhh " + playerRaca + "!!!");
//                     PrintTextAbove(me, "*Aterrorizado*");
//                 endif
//                 FleeFrom(source);
//                 return 1;
//             endif
//         endif
//     endif
// endfunction

