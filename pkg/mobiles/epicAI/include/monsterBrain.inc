use npc;
include "include/sysEvent";
include "include/sounds";
include ":combat:damage";
include "include/facings";
include ":npcs:thor/handling/thorai";
include "epicSpecial";

var me := Self();
const AREA_SIZE := 18;
var combat_cooldown := 0;

function MonsterBrain()
    var ev;
    var wanders := 0;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    // Get monster configuration
    var npc_elem := NPC_GetNPCConfig(me);
    var speed_type := SplitWords(GetConfigString(npc_elem, "EpicAttackSpeed"), "_");
    var is_ranged := (speed_type[1] == "RANGED") || me.isSage();

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
                HandleDisengage(me.opponent);
            else
                is_ranged ? MaintainRange(me.opponent) : PursueTarget(me.opponent);
            endif
        endif
        
        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED: 
                    AlertAllies(ev.source, me);
                    HandleCombatEvent(ev.source, is_ranged);
                SYSEVENT_DAMAGED: 
                    AskForHealing(me);
                    HandleCombatEvent(ev.source, is_ranged);
                SYSEVENT_ENTEREDAREA:
                    HandleSpeech(ev.text, ev.source, is_ranged);
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        AlertAllies(ev.source, me);
                        CanFight(me, ev.source) ? HandleEnteredArea(ev.source, is_ranged) : HandleLeftArea(is_ranged);
                    endif
                SYSEVENT_DISENGAGED: 
                    HandleDisengage(ev.source);
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        HandleLeftArea(is_ranged);
                    endif
                EVID_ALERT_ALLIES:
                    var opponent_serial := GetObjProperty(me, "#taunt_opponent");
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent && CanFight(me, opponent))
                            EraseObjProperty(me, "#taunt_opponent");
                            HandleCombatEvent(opponent, is_ranged);
                        endif
                    endif
                EVID_TAUNT:
                    if (CanFight(me, ev.source))
                        HandleCombatEvent(ev.source, is_ranged);
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

function HandleSpeech(text, source, is_ranged)
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

function HandleDisengage(opponent)
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

            var max_loot := 1;
            var looted := 0;
            
            // Ensure NPC has a backpack
            if (!me.backpack)
                var backpack := CreateItemAtLocation(me.x, me.y, me.z, "Backpack", 1);
                EquipItem(me, backpack);
            endif

            // Loot items
            while (looted < max_loot)
                PrintTextAbove(me, "*revirando o corpo*");
                LootSomething(me, corpse);
                looted += 1;
                sleepms(1500);
            endwhile
        endif
    endif
    
    return 1;
endfunction

function ClearCombatState()
    SetWarMode(0);
    SetOpponent(0);
endfunction

function HandleLeftArea(is_ranged)
    // Clear current combat state
    ClearCombatState();
    
    // First look for new opponents
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL)
        if (CanFight(me, mobile))
            HandleCombatEvent(mobile, is_ranged);
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
                return 1;
            endif
            sleepms(100);
        endforeach
    endif

    // If nothing else to do, go idle
    HandleIdleState();
    return 1;
endfunction

function LootCorpse(corpse)
    if (!CorpseHasLoot(corpse))
        return 0;
    endif

    // Approach the corpse
    while (Distance(me, corpse) > 1)
        RunToward(corpse);
        sleepms(100);
        
        // Break if corpse disappeared or too far
        if (!corpse || Distance(me, corpse) > 20)
            return 0;
        endif
    endwhile

    // Calculate max items to loot based on intelligence
    var max_loot := CInt(AP_GetStat(me, INTELLIGENCE)/20);
    if (max_loot < 1)
        max_loot := 1;
    endif
    var looted := 0;
    
    // Ensure NPC has a backpack
    if (!me.backpack)
        var backpack := CreateItemAtLocation(me.x, me.y, me.z, "Backpack", 1);
        if (!EquipItem(me, backpack))
            return 0;
        endif
    endif
    
    // Loot items
    while (looted < max_loot)
        PrintTextAbove(me, "*revira o corpo*");
        if (!LootSomething(me, corpse))
            break;
        endif
        looted += 1;
        sleepms(1500);
    endwhile
    
    return (looted > 0);
endfunction

function CorpseHasLoot(corpse)
    if (!corpse || corpse.IsA(POLCLASS_CORPSE) == error)
        return 0;
    endif

    foreach item in EnumerateItemsInContainer(corpse)
        if (item.movable)
            return 1;
        endif
        sleepms(2);
    endforeach

    return 0;
endfunction

function LootSomething(npc, corpse)
    // Define priority items by objtype (coins)
    var coins := array{0xBA63, 0xBA64, 0xBA65}; // copper, silver, gold
    var resources := array{0x1bf2, 0x1bd7};     // ingots, wood

    if (GetCooldown(npc, "already_looted") > 0)
        return 0;
    endif

    // First try to loot coins
    var priority_loot := FindPriorityLoot(corpse, coins);
    if (priority_loot)
        return LootItem(npc, priority_loot);
    endif

    // Then try resources
    var resource_loot := FindPriorityLoot(corpse, resources);
    if (resource_loot)
        return LootItem(npc, resource_loot);
    endif

    // Then look for equipment by class
    foreach item in EnumerateItemsInContainer(corpse)
        if (!item.movable || item.newbie)
            continue;
        endif

        if (item.IsA(POLCLASS_WEAPON) || item.IsShield() || item.IsA(POLCLASS_ARMOR))
            return LootItem(npc, item);
        endif

        // Check containers
        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindEquipmentInContainer(item);
            if (container_item)
                return LootItem(npc, container_item);
            endif
        endif
        sleepms(2);
    endforeach

    return 1;
endfunction

function FindEquipmentInContainer(container)
    foreach item in EnumerateItemsInContainer(container)
        if (!item.movable)
            continue;
        endif

        if (item.IsA(POLCLASS_WEAPON) || item.IsShield() || item.IsA(POLCLASS_ARMOR))
            return item;
        endif

        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindEquipmentInContainer(item);
            if (container_item)
                return container_item;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
endfunction

function LootItem(npc, item)
    if (!item)
        return 0;
    endif

    if (!npc.backpack)
        var backpack := CreateItemAtLocation(npc.x, npc.y, npc.z, "Backpack", 1);
        EquipItem(npc, backpack);
    endif

    PrintTextAbove(npc, "*pega " + item.desc + "*");
    SetCooldown(npc, "already_looted", 300);

    return MoveItemToContainer(item, npc.backpack);
endfunction

function FindPriorityLoot(corpse, graphics_array)
    foreach item in EnumerateItemsInContainer(corpse)
        if (!item.movable)
            continue;
        endif

        if (item.objtype in graphics_array || item.graphic in graphics_array)
            return item;
        endif

        // Check containers for these items
        if (item.IsA(POLCLASS_CONTAINER))
            var container_item := FindPriorityLoot(item, graphics_array);
            if (container_item)
                return container_item;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
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

    return 1;
endfunction

function PursueTarget(opponent)
    var dist := Distance(me, opponent);
    if (!shouldKeepPursuing(opponent, dist)) return 0; endif

    if (me.canSpitAcid())
        if (dist >= 2 && dist <= 6)
            AcidSpray(me, opponent);
        endif
    endif

    if (me.canThrow())
        if (dist >= 2 && dist <= 12)
            ThrowSpecial(me, opponent);
        endif
    endif

    // Track progress towards target
    var progress := GetObjProperty(me, "#pursue_progress");
    if (!progress)
        progress := struct{
            "last_dist" := dist,
            "last_pos" := struct{ "x" := me.x, "y" := me.y },
            "last_check_time" := ReadGameClock(),
            "no_progress_count" := 0,
            "start_time" := ReadGameClock()
        };
    endif

    // Only check progress every 2 seconds
    if (ReadGameClock() >= progress.last_check_time + 2)
        var moved := (me.x != progress.last_pos.x || me.y != progress.last_pos.y);
        var got_closer := (dist < progress.last_dist);
        
        // No progress if we haven't moved OR haven't gotten closer in 2 seconds
        if (!moved || !got_closer)
            progress.no_progress_count += 1;
        else
            progress.no_progress_count := 0;
        endif

        // Update tracking info
        progress.last_dist := dist;
        progress.last_pos.x := me.x;
        progress.last_pos.y := me.y;
        progress.last_check_time := ReadGameClock();
    endif

    // If we're not making progress after several checks, force a reposition
    if (progress.no_progress_count >= 1) // Zero tolerance for no progress
        EraseObjProperty(me, "path");
        EraseObjProperty(me, "#enemy_last_loc");
        TacticalReposition(opponent);
        progress.no_progress_count := 0;
        progress.last_check_time := ReadGameClock();
        SetObjProperty(me, "#pursue_progress", progress);
        return 1;
    endif

    SetObjProperty(me, "#pursue_progress", progress);

    // Normal pursuit logic
    if (CheckLineOfSight(me, opponent))
        if (Distance(me, opponent) > 1)
            RunToward(opponent);
        endif
        sleepms(100);
    else
        var path := FindPath(me.x, me.y, me.z, opponent.x, opponent.y, opponent.z, me.realm, FP_IGNORE_MOBILES);
        if (path)
            foreach coord in path
                if (Distance(me, opponent) <= 1)
                    break;
                endif
                if (!shouldKeepPursuing(opponent, dist)) break; endif
                RunTowardLocation(coord.x, coord.y);
                sleepms(100);
            endforeach
        else
            progress.no_progress_count += 1;
            if (me.hasTunnelTactics())
                TunnelTactics(me, opponent);
            endif
        endif
    endif

    return 1;
endfunction

function shouldKeepPursuing(opponent, dist)
    if (!opponent)
        return 0;
    endif

    // Don't chase too far
    if (dist > AREA_SIZE + 6)
        return 0;
    endif

    // If we're in melee range and can see opponent, stop pursuing
    if (dist <= 1 && CheckLineOfSight(me, opponent))
        return 0;
    endif

    // Prevent flee spam by adding a cooldown
    if (GetCooldown(me, "flee_check"))
        return 1; // Continue pursuing if on cooldown
    endif
    var npcState := getNPCState(me);
    var cd := 15;
    
    if (npcState == "dying")
        cd := 5;
    endif

    SetCooldown(me, "flee_check", cd);
    var allies := GetNearbyAllies(me);
    if (allies.size() > 0)
        if (!GetCooldown(me, "allies"))
            SetObjProperty(me, "initial_allies", allies.size());
            SetCooldown(me, "allies", 15);
        endif

        var initial_allies := GetObjProperty(me, "initial_allies");
        var already_fighting := GetObjProperty(me, "already_fighting");
        
        // Only consider fleeing if we've lost allies
        if (initial_allies && allies.size() <= CInt(initial_allies/2))
            // Don't flee if already engaged unless heavily outnumbered
            if (!already_fighting || ListHostiles(me, 3).size() >= allies.size() * 2)
                // Higher chance to flee when heavily damaged
                var flee_chance := (npcState == "dying") ? 40 : 15;
                
                if (RandomInt(100) < flee_chance)
                    PrintTextAbove(me, "*"+me.name+" recua*");
                    FleeFrom(opponent);
                    return 0;  // Added return 0 after fleeing
                endif
            endif
        endif
    else
        // If alone, don't pursue new targets but stay if already fighting
        if (!GetObjProperty(me, "already_fighting"))
            return 0;
        endif
    endif

    return 1;  // Continue pursuing if none of the above conditions are met
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
            HandleCombatEvent(mobile, is_ranged);
            return;
        endif
        sleepms(2);
    endforeach

    if (me.isLizardMan() && !GetCooldown(me, "throw_cooldown") && me.canThrowSpear()) me.graphic := 0x23; endif // Retorna o gráfico de lizard com lança
    
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
    var spawn_point := SystemFindObjectBySerial(GetObjProperty(me, "spawnpoint"));
    var anchor := GetObjProperty(me, "anchor");
    var home_x, home_y;
    
    if (spawn_point)
        home_x := spawn_point.x;
        home_y := spawn_point.y;
    elseif (anchor)
        home_x := anchor[1];
        home_y := anchor[2];
    else
        return;
    endif

    while (!hasPlayerNear(me))    
        if (CoordinateDistance(me.x, me.y, home_x, home_y) > 5)
            WalkTowardLocation(home_x, home_y);
        endif
    endwhile
    // No need for event checking here since MonsterBrain already does it
    return 1;
endfunction

function FleeFrom(opponent)
    if (!opponent)
        return 0;
    endif

    var runs := 0;
    while (opponent && Distance(me, opponent) < 16 && runs < 20)
        RunAwayFrom(opponent);
        if(!AP_ConsumeVital(me, STAMINA, 5))
            PrintTextAbove(me, "*Exausto*");
            break;
        endif
        runs += 1;
        sleepms(100);
    endwhile
endfunction

function EnableMainEvents()
    DisableEvents(SYSEVENT_OPPONENT_MOVED); // Explicitly disable opponent movement events
    EnableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA, AREA_SIZE);
    EnableEvents(SYSEVENT_DISENGAGED + SYSEVENT_LEFTAREA, AREA_SIZE);
endfunction

function CanFight(attacker, defender)
    if (!defender || defender.dead || defender.hidden || defender.concealed)
        return 0;
    endif

    // Cooldown to avoid ress kill
    if (Cint(GetCooldown(defender, "deathtime" )) > 0) 
        // Allow combat if defender initiated it
        if (me.opponent == defender) return 1; endif
        // Otherwise prevent attacking during cooldown
        return 0;
    endif

    // If defender is a player, normal combat rules apply
    if (!defender.npctemplate)
        return 1;
    endif

    // NPC vs NPC combat only happens if:
    if (defender.npctemplate)
        // Check if forced to attack by GM command
        var forced_target := GetObjProperty(attacker, "#ForcedTarget");
        if (forced_target == defender.serial)
            return 1;
        endif

        // Check enemy categories
        var attacker_elem := NPC_GetNPCConfig(attacker);
        var defender_elem := NPC_GetNPCConfig(defender);
        var enemy_cats := GetConfigStringArray(attacker_elem, "EnemyCategories");
        
        if (enemy_cats && defender_elem.Category in enemy_cats)
            return 1;
        endif

        return 0; // No valid reason for NPCs to fight
    endif

    return 1;
endfunction

function HandleEnteredArea(source, is_ranged)
    if (!source || !CanFight(me, source))
        return 0;
    endif
    
    handleInitialReaction(source);
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

    HandleCombatEvent(source, is_ranged);
    
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
        // Try to stay closer to allies if possible
        var allies := GetNearbyAllies(me);
        if (allies.size() > 0)
            var closest_ally := allies[1];
            var ally_dist := Distance(me, closest_ally);
            
            if (ally_dist > 4)
                RunToward(closest_ally);
                sleepms(sleepdelay);
                return 0;
            endif
        endif
        
        TurnToward(opponent);
        return 1;
    endif

    return 0;
endfunction

// New function to handle ally behavior
function AlertAllies(invasor, me)
    if (isAlly(invasor))
        return 0;
    endif

    if (invasor.hidden || invasor.concealed)
        return 0;
    endif

    if (GetCooldown(me, "alert_allies"))
        return 0;
    endif
    
    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 8)
        if (mobile.npctemplate && mobile != me)
            if (isAlly(mobile))
                SetObjProperty(mobile, "#taunt_opponent", invasor.serial);
                SendEvent(mobile, struct{ "type" := EVID_ALERT_ALLIES, "source" := invasor });
            endif
        endif
        sleepms(2);
    endforeach

    SetCooldown(me, "alert_allies", 15);
endfunction

function handleInitialReaction(source)
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

function DoThiefCombatBehavior(opponent)
    if (Distance(me, opponent) < 6)
        return 0;
    endif

    // 50% chance to steal, 50% chance to sneak attack
    if (RandomInt(100) < 50)
        DoThiefSteal(opponent);
    else
        DoThiefSneakAttack(opponent);
    endif
endfunction

function ShamanBehavior(shaman)
    // First priority: Check allies health
    if (GetCooldown(shaman, "heal_check"))
        return 0;
    endif

    var npc_elem := NPC_GetNPCConfig(shaman);
    var caster_tier := CInt(npc_elem.Tier);
    if (!caster_tier)
        caster_tier := 1;
    endif

    // Get all allies nearby
    var allies := GetNearbyAllies(shaman);
    var wounded_allies := array{};

    // Check allies' health
    foreach ally in allies
        if (isHurt(ally)) // Is Hurt
            wounded_allies.append(ally);
        endif
    endforeach

    if (wounded_allies.size() == 0)
        DoHarmfulSpell();
    endif

    // If multiple allies need help, cast healing rain
    if (wounded_allies.size() >= 3 && !GetCooldown(shaman, "healing_rain"))
        PerformHealingRain(shaman, wounded_allies);
        SetCooldown(shaman, "healing_rain", 45);
        return 1;
    endif

    foreach ally in wounded_allies
        var most_wounded := struct{
            "mobile" := 0,
            "hp_percent" := 100
        };

        foreach ally in wounded_allies
            var hp_percent := CDbl(AP_GetVital(ally, HITS)) / CDbl(AP_GetVitalMaximumValue(ally, HITS)) * 100;
            if (hp_percent < most_wounded.hp_percent)
                most_wounded.mobile := ally;
                most_wounded.hp_percent := hp_percent;
            endif
            sleepms(2);
        endforeach

        if (most_wounded.mobile)
            PerformHeal(shaman, most_wounded.mobile, caster_tier);
            SetCooldown(shaman, "heal_check", 15);
            return 1;
        endif
        sleepms(2);
    endforeach

    return 0;
endfunction

function GetNearbyAllies(mobile)
    var allies := array{};
    foreach mobile in ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 8)
        if (isAlly(mobile))
            allies.append(mobile);
        endif
        sleepms(2);
    endforeach
    
    return allies;
endfunction

function GetPlayersPetsAndSummonsNearby(mobile)
    var enemies := array{};
    foreach mobile in ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 8)
        if (!isAlly(mobile))
            enemies.append(mobile);
        endif
        sleepms(2);
    endforeach
    
    return enemies;
endfunction

function isAlly(mobile)
    if (!mobile || mobile.dead) return 0; endif

    if (mobile.master || GetObjProperty(mobile, "summoned")) return 0; endif

    if (mobile.hidden || mobile.concealed) return 0; endif

    if (mobile.acctname || mobile.alignment != 1) return 0; endif
    
    return 1;
endfunction

function AskForHealing(me)
    // First look specifically for shamans
    if (!isHurt(me))
        return 0;
    endif

    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 12)
        if (mobile.npctemplate && mobile != me)
            if (mobile.isSage())
                SetObjProperty(mobile, "#healing_needed", me.serial);
                SendEvent(mobile, struct{ "type" := EVID_BUDDYATTACKED, "source" := me });
            endif
        endif
        sleepms(2);
    endforeach
endfunction 

function DoHarmfulSpell()
    var diff := getNpcResist();
   // If me is being attacked, prioritize Enraizar
   if (me.opponent)
      if (!GetCooldown(me.opponent, "enraizar_cooldown"))
         CastEnraizar(me, me.opponent, diff);
         return 1;
      endif
   endif
   
   // Otherwise, look for enemies attacking allies to weaken
   var allies := GetNearbyAllies(me);
   foreach ally in allies
      if (ally.opponent)
         if (!GetCooldown(ally.opponent, "weaken_cooldown"))
            CastWeaken(me, ally.opponent, diff);
            return 1;
         endif
      endif
      sleepms(2);
   endforeach

   return 0;
endfunction

function MageBehavior(me, opponent)
   if (GetCooldown(me, "spell_cast"))
      return 0;
   endif

   // Get available spells
   var spells := getAvailableSpells(me);
   if (!spells.size())
      return 0;
   endif

   // Choose spell based on situation
   var chosen_spell := ChooseSpell(me, opponent, spells);

   if (!chosen_spell)
      return 0;
   endif

   return CastMonsterSpell(me, chosen_spell);
endfunction

function CastMonsterSpell(me, spellname)
   var cfgFile := ":spells:config/spellcaster";
   var spellInfo := FindSpellInConfig(spellname, cfgFile);
   
   if (!spellInfo)
      return 0;
   endif
   
   // Start the cast_spell script with target info
   var targ := SystemFindObjectBySerial(GetObjProperty(me, "#SpellTarget"));
   EraseObjProperty(me, "#SpellTarget");
   
   var params := array{me, spellInfo, 0, targ};
   var script := Start_Script(":spells:cast_spell", params);
   if (script.errortext)
      PrintTextAbove(me, "Error starting spell: " + script.errortext);
      return 0;
   endif
   
   SetCooldown(me, "spell_cast", 15);
   return 1;
endfunction

function ChooseSpell(me, opponent, spells)
    var cfgFile := ":spells:config/spellcaster";
    var school := getSpellSchool(me);

    // Get ideal spell based on tactical considerations
    var chosen_spell := ChooseIdealSpell(me, opponent, spells);

    // Get spell info for the chosen spell
    var spellInfo := FindSpellInConfig(chosen_spell, cfgFile);
    if (!spellInfo)
        SetObjProperty(me, "#SpellTarget", opponent.serial);
        return "Misseis Magicos";
    endif

    // Set appropriate target based on spell type
    if (spellInfo.Target in array{"Unico", "Area"})
        SetObjProperty(me, "#SpellTarget", opponent.serial);
    elseif (spellInfo.Target == "Self")
        SetObjProperty(me, "#SpellTarget", me.serial);
    endif

    // Special targeting for corpse-related spells
    if (chosen_spell in array{"Prolongar Desmaio", "Levantar os Mortos"})
        var nearby_corpses := ListItemsNearLocation(me.x, me.y, me.z, 12);
        foreach item in nearby_corpses
            if (item.IsA(POLCLASS_CORPSE))
                SetObjProperty(me, "#SpellTarget", item.serial);
                break;
            endif
            sleepms(2);
        endforeach
    endif

    return chosen_spell;
endfunction

function getSpellSchool(monster)
   var npc_elem := NPC_GetNPCConfig(monster);
   var school:= npc_elem.SpellSchool;

   if (!school)
        var schools := array{"Evocatium", "Aradalore", "Ahtaleloreth", "Kaijin", "Velkyn"};
        school := schools[RandomInt(5)+1];
    endif

   return npc_elem.SpellSchool;
endfunction

function getAvailableSpells(monster)
   var npc_elem := NPC_GetNPCConfig(monster);
   var tier := CInt(npc_elem.Tier);
   if (!tier) tier := 1; endif
   
   var spells := array{"Misseis Magicos"}; // All casters have magic missile
   
   // Get spellcaster config
   var cfg := ReadConfigFile(":spells:config/spellcaster");
   var school := getSpellSchool(monster);
   
   // Add spells from school up to tier
   foreach spellId in (GetConfigIntKeys(cfg))
      var elem := FindConfigElem(cfg, spellId);
      if (elem.School == school && elem.Circle <= tier)
         spells.append(elem.Name);
      endif
      sleepms(2);
   endforeach
   return spells;
endfunction

function FindSpellInConfig( spellname, cfgFile )
    var cfg := ReadConfigFile( cfgFile );

        foreach spellId in ( GetConfigIntKeys( cfg ) )
            var elem := FindConfigElem( cfg, spellId );

            if ( Lower( GetConfigString( elem, "Name" ) ) == Lower( spellname ) )
                var dict := dictionary{ 
                "id" -> spellId,
                "circle" -> GetConfigInt( elem,"Circle" ),
                "name" -> GetConfigString( elem, "Name" ),
                "Difficulty" -> GetConfigInt( elem, "Difficulty" ),
                "School" -> GetConfigString( elem, "School" ),
                "Target" -> GetConfigString( elem, "Target" ),
                "script" -> GetConfigString( elem, "SpellScript" )
                };
                return dict;
            endif
        endforeach

    return 0;
endfunction

function ChooseIdealSpell(me, opponent, spells)
    var chosen_spells := array{};
    var opponent_class := opponent.getClass();
    var is_ranged := opponent.weapon.attribute == "RangedCombat";
    var dist := Distance(me, opponent);
    var low_health := AP_GetVital(me, HITS) < (AP_GetVitalMaximumValue(me, HITS) * 0.3);
    var is_drow := (GetObjProperty(opponent, "chardata").raca == "Drow");
    var enemy_count := ListHostiles(me, 10).size();
    var school := getSpellSchool(me);
    
    // First priority - emergency spells
    if (low_health && "Invisibilidade" in spells)
        return "Invisibilidade";
    endif
    
    // Check for summons first
    if (HasSummonedCreaturesNearby(opponent) && "Banimento Arcano" in spells)
        return "Banimento Arcano";
    endif

    // Check for spell combos first
    if (GetObjProperty(opponent, "sleep") && "Pesadelo" in spells)
        return "Pesadelo";
    endif
    
    var valid_spells := array{};
    
    // Always good spells get high priority
    foreach spell in spells
        if (spell in GetAlwaysGoodSpells(school, is_drow))
            valid_spells.append(struct{ "name" := spell, "weight" := 80 });
        endif
    endforeach
    
    // Class-specific spells
    case (opponent_class)
        GUERREIRO: 
            if ("Enfraquecer" in spells)
                valid_spells.append(struct{ "name" := "Enfraquecer", "weight" := 70 });
            endif
        LADINO:
            if ("Atrapalhar" in spells)
                valid_spells.append(struct{ "name" := "Atrapalhar", "weight" := 70 });
            endif
        SABIO:
            foreach spell in GetAntiMageSpells()
                if (spell in spells)
                    // Extra weight for Silenciar
                    var weight := (spell == "Silenciar") ? 90 : 70;
                    valid_spells.append(struct{ "name" := spell, "weight" := weight });
                endif
            endforeach
    endcase
    
    // Anti-melee spells
    if (!is_ranged)
        foreach spell in GetAntiMeleeSpells()
            if (spell in spells)
                valid_spells.append(struct{ "name" := spell, "weight" := 60 });
            endif
        endforeach
    endif
    
    // AoE spells if multiple enemies
    if (enemy_count > 2)
        foreach spell in GetAoESpells()
            if (spell in spells)
                valid_spells.append(struct{ "name" := spell, "weight" := 75 });
            endif
        endforeach
    endif
    
    // Close range spells
    if (dist <= 3 && "Maos Flamejantes" in spells)
        valid_spells.append(struct{ "name" := "Maos Flamejantes", "weight" := 65 });
    endif
    
    // Remove non-ideal spells
    valid_spells := RemoveNonIdealSpells(valid_spells, school);
    
    // Sort by circle and weight
    valid_spells := SortSpellsByPriority(valid_spells);
    
    if (valid_spells.size() > 0)
        return ChooseWeightedSpell(valid_spells);
    endif
    
    return "Misseis Magicos"; // Default fallback
endfunction

function GetAlwaysGoodSpells(school, is_drow)
    var spells := array{
        "Misseis Magicos", "Ampliar Dor", "Drenar Folego", "Relampago",
        "Incinerar", "Congelar", "Pesadelo", "Impedir Cura",
        "Forma Amaldicoada", "Drenar Vida", "Maldicao de Zhao",
        "Paralisar", "Envenenar", "Velocidade", "Clones das sombras",
        "Hipnotizar", "Cegar"
    };
    
    // Add Escurecer only against non-drow
    if (!is_drow)
        spells.append("Escurecer");
    endif
    
    // Add all Aradalore spells if that's the school

    if (school == "Aradalore")
        spells := spells + GetAradoloreSpells();
    endif
    
    return spells;
endfunction

function GetAntiMageSpells()
    return array{
        "Enfraquecer Intelecto", "Atrasar Magias", "Fuga de Mana",
        "Drenar Mana", "Envenenar Espirito", "Silenciar"
    };
endfunction

function GetAntiMeleeSpells()
    return array{
        "Retribuicao Profana", "Muralha de Fogo", "Muralha de Pedra",
        "Correntes de dor", "Medo", "Vinganca Recorrente",
        "Horror Paralisante", "Forma Espectral"
    };
endfunction

function GetAoESpells()
    return array{
        "Bola de Fogo", "Nuvem Acida", "Corrente de Raios",
        "Chuva de Meteoros", "Terremoto", "Chuva de Gelo",
        "Parar o Tempo"
    };
endfunction

function GetNonIdealSpells()
    return array{
        "Sangue em mana", "Satisfacao Sadica", "Presente Espiritual",
        "Invoc. Espir. Luz", "Aprisionar Alma", "Aparencia Cadaverica",
        "Pacto Sepultura", "Remover Encantamentos", "Teleportar",
        "Folego prolongado", "Encantar objeto", "Manipular Sombras",
        "Iludir", "Pressagio"
    };
endfunction

function SortSpellsByPriority(spells)
    // Bubble sort implementation for POL
    var n := spells.size();
    var sorted := 0;
    while (!sorted)
        sorted := 1;
        for i := 1 to (n-1)
            var a := spells[i];
            var b := spells[i+1];
            
            var circle_a := GetSpellCircle(a.name);
            var circle_b := GetSpellCircle(b.name);
            
            var should_swap := 0;
            
            if (circle_a == circle_b)
                // If same circle, sort by weight
                if (a.weight < b.weight)
                    should_swap := 1;
                endif
            else
                // Sort by circle (higher circle first)
                if (circle_a < circle_b)
                    should_swap := 1;
                endif
            endif
            
            if (should_swap)
                // Swap elements
                var temp := spells[i];
                spells[i] := spells[i+1];
                spells[i+1] := temp;
                sorted := 0;
            endif
        endfor
    endwhile
  
    return spells;
endfunction

function GetSpellCircle(spellname)
    // Need access to spell config to get circle
    var cfg := ReadConfigFile(":spells:config/spellcaster");
    foreach spellId in (GetConfigIntKeys(cfg))
        var elem := FindConfigElem(cfg, spellId);
        if (Lower(elem.Name) == Lower(spellname))
            return CInt(elem.Circle);
        endif
        sleepms(2);
    endforeach
    
    return 1; // Default to circle 1 if not found
endfunction

function ChooseWeightedSpell(valid_spells)
    var total_weight := 0;
    foreach spell in valid_spells
        total_weight += spell.weight;
    endforeach
    
    var roll := RandomInt(total_weight);
    var current_weight := 0;
    
    foreach spell in valid_spells
        current_weight += spell.weight;
        if (roll <= current_weight)
            return spell.name;
        endif
    endforeach
    
    return valid_spells[1].name; // Fallback to highest priority spell
endfunction

function HasSummonedCreaturesNearby(opponent)
    var summons := GetPlayersPetsAndSummonsNearby(opponent);
    foreach summon in summons
        if (GetObjProperty(summon, "summoned"))
            return 1;
        endif
        sleepms(2);
    endforeach
    
    return 0;
endfunction

function GetAradoloreSpells()
   // All spells that belong to the Aradalore school
   // Important: Cross reference with spellcaster.cfg to match names exactly
   return array{
     "Invoc. Espir. Urso",
     "Invoc. Espir. Lobo",
     "Invoc. Espir. Serpente", 
     "Invoc. Espir. Pantera",
     "Invoc. Espir. Cura",
     "Invoc. Driade",
     "Invoc. Ent",
     "Forma de Urso", 
     "Forma Grande Urso",
     "Forma do Lobo",
     "Forma da Serpente",
     "Forma da Pantera",
     "Forma Licantropa",
     "Invoc. Espir. Fogo",
     "Invoc. Colosso"
   };
endfunction

function RemoveNonIdealSpells(spells, school)
    var non_ideal := GetNonIdealSpells();
    var valid_spells := array{};
    
    foreach spell in spells
        if (!(spell.name in non_ideal))
            valid_spells.append(spell);
        endif
    endforeach
    
    return valid_spells;
endfunction
