use npc;
include "include/sysEvent";
include "include/sounds";
include ":combat:damage";
include "include/facings";
include ":npcs:thor/handling/thorai";
include "epicSpecial";
include "NPCMageryUtils";

function MonsterBrain()
    var ev;
    var wanders := 0;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    // Get monster configuration
    var is_ranged := isMonsterRanged();

    // Enable archer-specific abilities if needed
    if (is_ranged)
        GrantPrivilege(me, "firewhilemoving");
        me.enable("firewhilemoving");
    endif

    while (me)
        ev := os::wait_for_event(0);
    
        // If we have an opponent, maintain combat position regardless of events
        if (me.opponent)
            if (!CanFight(me, me.opponent))
                HandleMonsterLeftArea(me.opponent);
            else
                if (GetObjProperty(me, "metamorfose")) is_ranged := 0; endif
                is_ranged ? MaintainRange(me.opponent) : PursueTarget(me.opponent);
            endif
        endif
        
        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED: 
                    AlertAllies(ev.source, me);
                    HandleMonsterCombatEvent(ev.source, is_ranged);
                SYSEVENT_DAMAGED: 
                    AskForHealing(me);
                    HandleMonsterCombatEvent(ev.source, is_ranged);
                SYSEVENT_ENTEREDAREA:
                    // HandleMonsterSpeech(ev.text, ev.source, is_ranged);
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        AlertAllies(ev.source, me);
                        CanFight(me, ev.source) ? HandleMonsterEnteredArea(ev.source, is_ranged) : HandleMonsterLeftArea(is_ranged);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        HandleMonsterLeftArea(is_ranged);
                    endif
                EVID_ALERT_ALLIES:
                    var opponent_serial := GetObjProperty(me, "#taunt_opponent");
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent && CanFight(me, opponent))
                            EraseObjProperty(me, "#taunt_opponent");
                            HandleMonsterCombatEvent(opponent, is_ranged);
                        endif
                    endif
                EVID_TAUNT:
                    if (CanFight(me, ev.source))
                        HandleMonsterCombatEvent(ev.source, is_ranged);
                    endif
                EVID_BUDDYATTACKED:
                    ShamanBehavior(me);
                default:
                    PrintTextAbove(me, "UNKNOWN EVENT: " + ev.type);
            endcase
        endif

        if (!ev)
            if (!me.opponent)
                MonsterIdle(wanders, is_ranged);
                wanders += 1;
                if (wanders >= 30)
                    ev := os::wait_for_event(10);
                    wanders := 0;
                    HandleIdleState();
                endif
            endif
        endif

        sleepms(100);
    endwhile
endfunction

function HandleMonsterSpeech(text, source, is_ranged)
    if (!source || !CanFight(me, source))
        return 0;
    endif

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
            FleeFrom(source);
            return 1;
        endif
    endif

    // Gera e processa resposta
    var response := handle_combat_speech(me.name, text, source.getRace(), npcState);
    if (response == error)
        return Say(handle_error_combat_AI_speech());
    endif

    Say(response);
    SetCooldown(me, "combat_speech", 15);
    return 1;
endfunction

function HandleMonsterLeftArea(is_ranged)
    // Clear current combat state
    ClearCombatState();
    
    // First look for new opponents
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL)
        if (CanFight(me, mobile))
            HandleMonsterCombatEvent(mobile, is_ranged);
            return 1;
        endif
        sleepms(2);
    endforeach

    // If no opponents found, look for corpses to loot
    var corpses := array{};
    foreach item in ListItemsNearLocation(me.x, me.y, me.z, AREA_SIZE)
        if (item.IsA(POLCLASS_CORPSE))
            corpses.append(item);
        endif
        sleepms(2);
    endforeach

    // Try to loot any found corpses
    if (corpses.size() > 0)
        foreach corpse in corpses
            if (LootCorpse(corpse))
                SetCooldown(me, "loot_cooldown", 300);
                return 1;
            endif
            sleepms(100);
        endforeach
    endif

    // If nothing else to do, go idle
    HandleIdleState();
    return 1;
endfunction

function HandleMonsterCombatEvent(opponent, is_ranged)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
        return 0;
    endif
    if (GetObjProperty(me, "metamorfose")) is_ranged := 0; endif

    var z_diff := Abs(me.z - opponent.z);
    if (z_diff > MAX_Z_DIFF)
        return 0;
    endif

    // Check for health-based teleport
    CheckHealthTeleport(me);

    SetOpponent(opponent);
    opponent := CheckForIdealTarget();
    SetOpponent(opponent);

    // Let the combat hook handle the actual attacks
    if (is_ranged)
        MaintainRange(opponent);
    else
        TurnToward(opponent);
        PursueTarget(opponent);
    endif
    
    if (me.isMyrkonid())
        SporeSpecial(me, opponent);
    endif

    if (me.isMage())
        MageBehavior(me, opponent);
    endif

    if (me.isShaman())
        ShamanBehavior(me);
    endif

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

    if (me.CanCharm())
        CharmOpponent(me, opponent);
    endif

    if (ShouldFlee(me))
        FleeFrom(opponent);
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

    // Look for potential targets
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL)
        if (CanFight(me, mobile))
            HandleMonsterCombatEvent(mobile, is_ranged);
            return;
        endif
        sleepms(2);
    endforeach

    if (me.isLizardMan() && !GetCooldown(me, "throw_cooldown") && me.canThrowSpear()) me.graphic := 0x23; endif // Retorna o gráfico de lizard com lança
    var anchor := GetObjProperty(me, "anchor");
    if (CoordinateDistance(me.x, me.y, anchor[1], anchor[2]) > 35)
      PrintText(me, "*Retorna correndo* [Sistema Anti-Exploits]");
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
        var dest := patrol_points[current_patrol_point];
        
        // Check if we've reached the destination (within 1 tile)
        if (CoordinateDistance(me.x, me.y, dest.x, dest.y) <= 1)
            // Move to next patrol point
            current_patrol_point += 1;
            if (current_patrol_point > patrol_points.size())
                current_patrol_point := 1; // Loop back to first point
            endif
            SetObjProperty(me, "#current_patrol_point", current_patrol_point);
        endif
        
        // Walk toward destination
        WalkTowardLocation(dest.x, dest.y);
        return; // Skip regular wandering
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
endfunction

function HandleMonsterEnteredArea(source, is_ranged)
    if (!source || !CanFight(me, source))
        return 0;
    endif
    
    handleMonsterInitialReaction(source);
    if (me.canCharge())
        StartCharge(me, source);
    endif

    if (me.isThief())
        DoThiefCombatBehavior(source);
    endif

    if (me.hasTunnelTactics())
        TunnelTactics(me, source);
    endif

    if (me.CanAmbush())
        HandleAmbush(me, source);
    endif

    HandleMonsterCombatEvent(source, is_ranged);
    
    return 1;
endfunction

function handleMonsterInitialReaction(source)
    var npc_elem := NPC_GetNPCConfig(me);
    var category := GetConfigString(npc_elem, "Category");

    if (category == "Goblin")
        var playerRaca := source.getRace();
        var name_mob := GetConfigString(npc_elem, "Name");
        if (playerRaca == ORC || playerRaca == DROW)
            // High chance to flee from these races
            var randomDice := RandomInt(100);
            if (randomDice < 30)
                var playerinput := "*raça " + playerRaca + " assusta*";
                var responseAI := handle_combat_ai(category, name_mob, playerinput, playerRaca);
                if (responseAI)
                    Say(responseAI);
                else
                    Say("Ahhh " + playerRaca + "!!!");
                    PrintTextAbove(me, "*Aterrorizado*");
                endif
                FleeFrom(source);
                return 1;
            endif
        endif
    endif
endfunction

