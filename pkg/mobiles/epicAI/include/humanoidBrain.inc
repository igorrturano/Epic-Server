use npc;
include "include/sysEvent";
include "include/sounds";
include ":combat:damage";
include "include/facings";
include ":npcs:thor/handling/thorai";
include "epicSpecial";
include "NPCMageryUtils";
include "include/epicLoggerUtils";

function HumanoidBrain()
    var ev;
    var wanders := 0;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    // Get humanoid configuration
    var npc_elem := NPC_GetNPCConfig(me);
    var speed_type := SplitWords(GetConfigString(npc_elem, "EpicAttackSpeed"), "_");
    var is_ranged := (speed_type[1] == "RANGED") || me.isSage();

    // Enable archer-specific abilities if needed
    if (is_ranged)
        GrantPrivilege(me, "firewhilemoving");
        me.enable("firewhilemoving");
    endif

    // Load Combat History
    var npc_template := NPC_ParseTemplateName(me.npctemplate).template;
    var combat_history := LoadCombatHistory(npc_template);

    var habilidades := array{};
    var calculate_level_hp := Cint(AP_GetSkill(me, WRESTLING) + AP_GetSkill(me, TACTICS)) - 80;
	var npc_level := (calculate_level_hp / 6);

	// Get valid habs for level
	var habs_by_level := GetHabsByLevel(me, npc_level);
	foreach hab in habs_by_level
		habilidades.append(hab);
	endforeach

	SetObjProperty(me, "myhabs", habilidades);

    while (me)
        ev := os::wait_for_event(0);
    
        // If we have an opponent, maintain combat position regardless of events
        if (me.opponent)
            if (!CanFight(me, me.opponent))
                HandleHumanoidLeftArea(me.opponent);
            else
                if (GetObjProperty(me, "metamorfose")) is_ranged := 0; endif
                is_ranged ? MaintainRange(me.opponent) : PursueTarget(me.opponent);
            endif
        endif
        
        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED: 
                    AlertAllies(ev.source, me);
                    HandleHumanoidCombatEvent(ev.source, is_ranged);
                SYSEVENT_DAMAGED: 
                    AskForHealing(me);
                    HandleHumanoidCombatEvent(ev.source, is_ranged);
                SYSEVENT_ENTEREDAREA:
                    HandleHumanoidSpeech(ev.text, ev.source, is_ranged);
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        AlertAllies(ev.source, me);
                        CanFight(me, ev.source) ? HandleHumanoidEnteredArea(ev.source, is_ranged) : HandleHumanoidLeftArea(is_ranged);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source)) // Only process if it's a player
                        HandleHumanoidLeftArea(is_ranged);
                    endif
                EVID_ALERT_ALLIES:
                    var opponent_serial := GetObjProperty(me, "#taunt_opponent");
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent && CanFight(me, opponent))
                            EraseObjProperty(me, "#taunt_opponent");
                            HandleHumanoidCombatEvent(opponent, is_ranged);
                        endif
                    endif
                EVID_TAUNT:
                    if (CanFight(me, ev.source))
                        HandleHumanoidCombatEvent(ev.source, is_ranged);
                    endif
                EVID_BUDDYATTACKED:
                    ShamanBehavior(me);
                default:
                    PrintTextAbove(me, "UNKNOWN EVENT: " + ev.type);
            endcase
        endif

        if (!ev)
            if (!me.opponent)
                HumanoidIdle(wanders, is_ranged);
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

function HandleHumanoidSpeech(text, source, is_ranged)
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

function HandleHumanoidLeftArea(is_ranged)
    // Clear current combat state
    ClearCombatState();
    
    // First look for new opponents
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, AREA_SIZE, LISTEX_FLAG_NORMAL)
        if (CanFight(me, mobile))
            HandleHumanoidCombatEvent(mobile, is_ranged);
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

function HandleHumanoidCombatEvent(opponent, is_ranged)
    if (!opponent || opponent.dead || !CanFight(me, opponent))
        return 0;
    endif
    if (GetObjProperty(me, "metamorfose")) is_ranged := 0; endif

    var z_diff := Abs(me.z - opponent.z);
    if (z_diff > MAX_Z_DIFF)
        return 0;
    endif

    SetOpponent(opponent);
    opponent := CheckForIdealTarget();
    SetOpponent(opponent);

    // Track initial state before combat
    var initial_state := GetCombatState(me, opponent);
    var used_ability := UseAbilities(me, opponent);
    
    if (used_ability)
        Sleep(2);
        var end_state := GetCombatState(me, opponent);
        var success := DetermineCombatSuccess(initial_state, end_state);
        LogCombatAction(me, initial_state, used_ability, success); 
    endif

    // Let the combat hook handle the actual attacks
    if (is_ranged)
        MaintainRange(opponent);
    else
        TurnToward(opponent);
        PursueTarget(opponent);
    endif

    if (me.isMage())
        MageBehavior(me, opponent);
    endif

    if (me.isShaman())
        ShamanBehavior(me);
    endif

    if (me.CanBite())
        Bite(me, opponent);
    endif

    return 1;
endfunction

function HumanoidIdle(wanders, is_ranged)
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
            HandleHumanoidCombatEvent(mobile, is_ranged);
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
endfunction

function HandleHumanoidEnteredArea(source, is_ranged)
    if (!source || !CanFight(me, source))
        return 0;
    endif
    
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

    HandleHumanoidCombatEvent(source, is_ranged);
    
    return 1;
endfunction

var state_values := dictionary; // Global state-value mapping

function UseAbilities(npc, opponent)
   if (GetCooldown(npc, "ability_cooldown"))
      return 0;
   endif

   var state := GetCombatState(npc, opponent);
   var habs := GetObjProperty(npc, "myhabs");
   if (!habs) 
      return 0;
   endif

    var best_hab := ChooseAbility(npc, state, habs);
    print("best_hab: " + best_hab);
    var success := CInt(ProccessHab(npc, best_hab));

    return success;
endfunction

function GetCombatState(npc, opponent)
   return struct{
      "health" := getNPCState(npc),
      "distance" := Distance(npc, opponent),
      "stamina" := AP_GetVital(npc, "Stamina"),
      "opponent_casting" := opponent.isSage(),
      "has_advantage" := IsBehind(npc, opponent.x, opponent.y)
   };
endfunction

function ChooseAbility(npc, state, habs)
   // Filter only active abilities
   var active_habs := array{};
   var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
   
   foreach hab in habs
       var elem := FindConfigElem(habcfg, hab);
       if (GetConfigString(elem, "uso") == "Ativa")
           active_habs.append(hab);
       endif
       sleepms(2);
   endforeach

   if (active_habs.size() < 1)
       return 0;
   endif

   if (RandomInt(100) < 15)
       return active_habs[RandomInt(active_habs.size())+1];
   endif

   var success_rates := LoadCombatHistory(NPC_ParseTemplateName(npc.npctemplate).template);
   var best_rate := 0;
   var best_hab := active_habs[RandomInt(active_habs.size())+1];

   foreach hab in active_habs
       var key := FormatStateKey(state, hab);
       if (success_rates[key])
           var rate := CDbl(success_rates[key].successes) / success_rates[key].total;
           if (rate > best_rate)
               best_rate := rate;
               best_hab := hab;
           endif
       endif
       sleepms(2);
   endforeach

   return best_hab;
endfunction

function GetHabsByLevel(npc, nivel)
  if (!npc) return array{}; endif
  
  var valid_habs := array{};
  var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
  var npc_elem := NPC_GetNPCConfig(npc);

  // Before level 4, only get core class abilities
  if (nivel < 4)
    foreach hab in (GetConfigStringKeys(habcfg))
      var elem := FindConfigElem(habcfg, hab);
      var classe := GetConfigString(elem, "classe");
      if (!classe) continue; endif
      
      var params := SplitWords(classe);
      var hab_class := params[1];
      var hab_level := CInt(params[2]) * 2;
      var my_class := (npc_elem.Classe);

      if (hab_level <= nivel && hab_class == my_class && !GetConfigString(elem, "requisito"))
        valid_habs.append(hab);
      endif
    endforeach
  endif

  // At level 4+, get subclass-specific abilities respecting prerequisites
  if (nivel >= 4)
    var progression := GetClassProgression(habcfg, 0);
    foreach hab in progression
      var elem := FindConfigElem(habcfg, hab);
      var classe := GetConfigString(elem, "classe"); 
      if (!classe) continue; endif
      
      var hab_level := CInt(SplitWords(classe)[2]) * 2;

      if (hab_level <= nivel)
        valid_habs.append(hab);
      endif
    endforeach
  endif

  return valid_habs;
endfunction

function GetClassProgression(habcfg, subclass)
  var progression := array{};
  var class_choice := array{};
  var npc_elem := NPC_GetNPCConfig(me);
  var class_tree := struct{};
  var base_hab := "";
  if (subclass)
    base_hab := subclass;
  else
  // Build ability tree
    foreach hab in (GetConfigStringKeys(habcfg))
        var elem := FindConfigElem(habcfg, hab);
        var classe := GetConfigString(elem, "classe");
        var params := SplitWords(classe);
        var hab_class := params[1];
        var hab_level := CInt(params[2]) * 2;
        var my_class := npc_elem.Classe;
        var req := GetConfigString(elem, "requisito");

        if (hab_class == my_class)
        if (!req && hab_level == 4)
            class_choice.append(hab);
        endif
        
        // Add to progression tree
        if (req)
            if (!class_tree[req])
            class_tree[req] := array{};
            endif
            class_tree[req].append(hab);
        endif
        endif
    endforeach

    base_hab := class_choice[RandomInt(class_choice.size())+1];
  endif

  // Pick base ability and build progression
  progression.append(base_hab);

  var current_level := base_hab;
  var processed := array{};
  
  while (current_level && current_level != processed[processed.size()])
    processed.append(current_level);
    
    if (class_tree[current_level])
      foreach hab in (class_tree[current_level])
        if (!(hab in progression))
          progression.append(hab);
          current_level := hab;
        endif
        sleepms(2);
      endforeach
    endif
  endwhile

  return progression;
endfunction

function FormatStateKey(state, hab)
   return state.health + "_" + 
          CStr(state.distance) + "_" + 
          CStr(state.stamina) + "_" + 
          CStr(state.opponent_casting) + "_" + 
          CStr(state.has_advantage) + "_" + 
          hab;
endfunction

function DetermineCombatSuccess(old_state, new_state)
    if (old_state.health == "dying" && new_state.health != "dying")
        return 1;
    elseif (old_state.has_advantage == 0 && new_state.has_advantage == 1)
        return 1; 
    endif
    
    return new_state.distance >= 1;
endfunction