use npc;
include "include/sysEvent";
include "include/sounds";
include ":combat:damage";
include "include/facings";
include ":npcs:thor/handling/thorai";
include "epicSpecial";

var me := Self();
const AREA_SIZE := 16;
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
        ev := os::wait_for_event(200000);
    
        // If we have an opponent, maintain combat position regardless of events
        if (me.opponent)
            if (is_ranged)
                MaintainRange(me.opponent);
            else
                PursueTarget(me.opponent);
            endif
        endif
        
        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED: 
                    // PrintTextAbove(me, "ENGAGED");
                    AlertAllies(ev.source, me);
                    HandleCombatEvent(ev.source, is_ranged);
                SYSEVENT_DAMAGED: 
                    // PrintTextAbove(me, "DAMAGED");
                    AskForHealing(me);
                    HandleCombatEvent(ev.source, is_ranged);
                SYSEVENT_ENTEREDAREA:
                    // PrintTextAbove(me, "ENTERED AREA"); 
                    AlertAllies(ev.source, me);
                    if (CanFight(me, ev.source))
                        HandleEnteredArea(ev.source, is_ranged);
                    endif
                SYSEVENT_OPPONENT_MOVED:
                    // PrintTextAbove(me, "OPPONENT MOVED");
                    if (!ev.source || ev.source.hidden || ev.source.concealed)
                        return 0;
                    endif
                    
                    HandleOpponentMovement(ev.source, is_ranged);
                SYSEVENT_SPEECH: 
                    PrintTextAbove(me, "SPEECH");
                    HandleSpeech(ev.text, ev.source, is_ranged);
                SYSEVENT_DISENGAGED: 
                    // PrintTextAbove(me, "DISENGAGED");
                    HandleDisengage(ev.source);
                SYSEVENT_LEFTAREA:
                    // PrintTextAbove(me, "LEFT AREA");
                    // if (ev.source == me.opponent || opponent.dead)
                        HandleLeftArea(is_ranged);
                    // endif
                EVID_ALERT_ALLIES:
                    // PrintTextAbove(me, "ALERT ALLIES"); 
                    var opponent_serial := GetObjProperty(me, "#taunt_opponent");
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent && CanFight(me, opponent))
                            EraseObjProperty(me, "#taunt_opponent");
                            HandleCombatEvent(opponent, is_ranged);
                        endif
                    endif
                EVID_FLEE:
                    PrintTextAbove(me, "FLEE");
                    if (ev.source)
                        FleeFrom(ev.source);
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

        // Also debug the idle state
        if (!ev)
            PrintTextAbove(me, "*IDLE*");
        endif

        sleepms(100);
        if (!me.opponent)
            MonsterIdle(wanders, is_ranged);
            wanders += 1;

            if (wanders >= 30)
                PrintTextAbove(me, "*HANDLING IDLE STATE*");
                wanders := 0;
                HandleIdleState();
            endif
        endif
    endwhile
endfunction

function HandleOpponentMovement(opponent, is_ranged)
    if (!opponent || !CanFight(me, opponent))
        PrintTextAbove(me, "Invalid opponent");
        return 0;
    endif

    if (me.opponent == opponent)
        if (is_ranged)
            MaintainRange(opponent);
        else
            TurnToward(opponent);
            PursueTarget(opponent);
        endif
    endif
    
    return 1;
endfunction

function HandleSpeech(text, source, is_ranged)
    // Basic speech handling - could be expanded for specific responses
    if (!source || !CanFight(me, source))
        return 0;
    endif

    handle_combat_speech(me.name, text, source.getRace());

    return 1;
endfunction

function HandleDisengage(opponent)
    if (!opponent)
        return 0;
    endif

    SetWarMode(0);
    SetOpponent(0);
    
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

            // Calculate max items to loot based on intelligence
            var max_loot := CInt(AP_GetStat(me, INTELLIGENCE)/30);
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
    PrintTextAbove(me, "*procura corpos*");
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
    PrintTextAbove(me, "*procura alvos*");
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
        if (!item.movable)
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

    SetCooldown(npc, "already_looted", 120);
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
        PrintTextAbove(me, "Invalid opponent");
        return 0;
    endif

    const MAX_Z_DIFF := 18;
    var z_diff := Abs(me.z - opponent.z);
    if (z_diff > MAX_Z_DIFF)
        return 0;
    endif

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

    if (me.isSage())
        ShamanBehavior(me);
    endif

    if (me.hasHiveMind())
        HiveMind(me);
    endif

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

    if (me.canSpitAcid())
        var dist := Distance(me, opponent);
        if (dist >= 2 && dist <= 6)
            AcidSpray(me, opponent);
        endif
    endif

    if (CheckLineOfSight(me, opponent))
        while (Distance(me, opponent) > 1 && CanFight(me, opponent))
            RunToward(opponent);
            sleepms(100);
        endwhile
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
        PrintTextAbove(me, "Has opponent - skipping idle");
        return;
    endif

    if (wanders > 10)
        // Heal slowly while idle
        var heal_amount := CInt(CDbl(AP_GetVitalMaximumValue(me, HITS)) * 0.01);
        HealDamage(me, heal_amount);
    endif

    // Look for potential targets
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL|LISTEX_FLAG_HIDDEN)
        if (CanFight(me, mobile))
            HandleCombatEvent(mobile, is_ranged);
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
    EnableEvents(SYSEVENT_OPPONENT_MOVED + SYSEVENT_DISENGAGED + SYSEVENT_LEFTAREA, AREA_SIZE);
    EnableEvents(SYSEVENT_SPEECH, 4);
endfunction

function CanFight(attacker, defender)
    if (!defender || defender.dead || defender.hidden || defender.concealed)
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
    // If attacker or defender aren't NPCs, return
    if (!invasor.npctemplate)
        return 0;
    endif
  
    foreach mobile in ListMobilesNearLocation(me.x, me.y, me.z, 8)
        if (mobile.npctemplate && mobile != me)
            if (isAlly(mobile))
                SetObjProperty(mobile, "#taunt_opponent", invasor.serial);
                SendEvent(mobile, struct{ "type" := EVID_ALERT_ALLIES, "source" := invasor });
                PrintTextAbove(mobile, "*alerted* by " + me.name);
            endif
        endif
        sleepms(2);
    endforeach
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
        DoHarmfulSpell(shaman);
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

function GetNearbyAllies(shaman)
    var allies := array{};
    foreach mobile in ListMobilesNearLocation(shaman.x, shaman.y, shaman.z, 8)
        if (isAlly(mobile))
            allies.append(mobile);
        endif
        sleepms(2);
    endforeach
    
    return allies;
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

    PrintTextAbove(me, RandomHealingCall());
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

function RandomHealingCall()
    // Sons de dor/grunhidos simples
    case(RandomInt(6))
        0: return "Ughh!";
        1: return "Argh!";
        2: return "Khh!";
        3: return "*sangra*";
        4: return "*cambaleia*";
        5: return "*ferido*";
    endcase
endfunction

function DoHarmfulSpell(shaman)
   // Get NPC configuration for spell difficulty
   var npc_elem := NPC_GetNPCConfig(shaman);
   var tier := CInt(npc_elem.Tier);
   if (!tier)
      tier := 1;
   endif
   
   // Calculate spell difficulty based on tier
   var diff := RESIST_EASY;
   case (tier)
      1: diff := RESIST_EASY;
      2: diff := RESIST_MEDIUM;
      3: diff := RESIST_HARD;
      4: diff := RESIST_VERY_HARD;
      5: diff := RESIST_EXTREMELY_HARD;
   endcase

   // If shaman is being attacked, prioritize Enraizar
   if (shaman.opponent)
      if (!GetCooldown(shaman.opponent, "enraizar_cooldown"))
         CastEnraizar(shaman, shaman.opponent, diff);
         return 1;
      endif
   endif
   
   // Otherwise, look for enemies attacking allies to weaken
   var allies := GetNearbyAllies(shaman);
   foreach ally in allies
      if (ally.opponent)
         if (!GetCooldown(ally.opponent, "weaken_cooldown"))
            CastWeaken(shaman, ally.opponent, diff);
            return 1;
         endif
      endif
      sleepms(2);
   endforeach

   return 0;
endfunction