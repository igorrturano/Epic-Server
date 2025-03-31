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
    var is_ranged := me.weapon.attribute == "Ranged" || (me.isSage() && me.weapon.objtype == FIST);

    // Enable archer-specific abilities if needed
    if (TemHabilidade(me, "Tiro em Movimento"))
        GrantPrivilege(me, "firewhilemoving");
        me.enable("firewhilemoving");
    endif

    // Load Combat History
    var npc_template := NPC_ParseTemplateName(me.npctemplate).template;
    var combat_history := LoadCombatHistory(npc_template);

    while (me)
        ev := os::wait_for_event(0);
    
        // If we have an opponent, maintain combat position regardless of events
        if (me.opponent)
            if (!CanFight(me, me.opponent))
                HandleHumanoidLeftArea(me.opponent);
            else
                var dist := Distance(me, me.opponent);
                if (GetObjProperty(me, "metamorfose")) is_ranged := 0; endif
                
                var range_info := GetWeaponRangeInfo();
                // Only pursue if we're significantly out of range
                if (dist > (range_info.max_range + 1))
                    PursueTarget(me.opponent);
                else
                    MaintainRange(me.opponent);
                endif
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
                    HandleEnteredAreaAbilities(ev.source, is_ranged);
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
                    if (me.isShaman()   )
                        HumanoidShamanBehavior(me, ev.source);
                    endif
                    if (me.isPriest())
                        HumanoidPriestBehavior(me, ev.source);
                    endif
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
    
    // Force melee combat when metamorphosed
    if (GetObjProperty(me, "metamorfose"))
        is_ranged := 0;
    endif

    var z_diff := Abs(me.z - opponent.z);
    if (z_diff > MAX_Z_DIFF)
        return 0;
    endif

    // Check for health-based teleport
    CheckHealthTeleport(me);

    SetOpponent(opponent);
    opponent := CheckForIdealTarget();
    SetOpponent(opponent);

    if (ShouldFlee(me))
        FleeFrom(opponent);
        return 1;
    endif

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
        if (Distance(me, opponent) < 2 && !TemHabilidade(me, "Tiro em Movimento"))
            reachForMeleeWeapon(opponent);
        endif

        var range_info := GetWeaponRangeInfo();
        // Always try to maintain maximum range for ranged attackers
        if (Distance(me, opponent) < range_info.max_range)
            RunAwayFrom(opponent);
        else
            // Only move closer if we're too far to shoot
            if (Distance(me, opponent) > range_info.max_range)
                PursueTarget(opponent);
            endif
        endif
        
        // Continuously try to shoot while in combat
        if (me.isShooter())
            ShooterBehavior(me);
        endif

    else
        TurnToward(opponent);
        PursueTarget(opponent);
    endif

    if (me.isMage())
        PrintTextAbove(me, "Mage Behavior");
        MageBehavior(me, opponent);
    endif

    if (me.isShaman())
        PrintTextAbove(me, "Shaman Behavior");
        HumanoidShamanBehavior(me, opponent);
    endif

    if (me.isPriest())
        PrintTextAbove(me, "Priest Behavior");
        HumanoidPriestBehavior(me, opponent);
    endif

    if (me.CanBite())
        Bite(me, opponent);
    endif

    // if (TemHabilidade(me, "Alquimista Experiente"))
        var explosive_potions := array{0xbf13, 0xbf14, 0xbf15, 0xef60, 0xef61, 0xef62};

        foreach item in EnumerateItemsInContainer(me.backpack)
            if (item.objtype in explosive_potions)
                UseItem(item, me);
                break;
            endif
        endforeach
    // endif

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
    var success := CInt(ProccessHab(npc, best_hab));
    SetCooldown(npc, "ability_cooldown", 5);

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
    var npc_elem := NPC_GetNPCConfig(npc);
    var npc_class := npc_elem.Classe;
    
    // First determine class behavior
    case (npc_class)
        GUERREIRO: return ChooseWarriorAbility(npc, state, habs);
        LADINO: return ChooseRogueAbility(npc, state, habs);
        SABIO: return ChooseSageAbility(npc, state, habs);
        default: return habs[RandomInt(habs.size())+1];
    endcase
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

    // Build ability tree ALWAYS, regardless of subclass
    foreach hab in (GetConfigStringKeys(habcfg))
        var elem := FindConfigElem(habcfg, hab);
        var classe := GetConfigString(elem, "classe");
        if (!classe)
            continue;
        endif
        
        var params := SplitWords(classe);
        var hab_class := params[1];
        var hab_level := CInt(params[2]) * 2;
        var my_class := npc_elem.Classe;
        var req := GetConfigString(elem, "requisito");

        // Only process abilities for this NPC's class
        if (hab_class == my_class)
            // Store level 4 abilities without prerequisites as potential base choices
            if (!req && hab_level == 4)
                class_choice.append(hab);
            endif
            
            // Build the prerequisite tree
            if (req)
                if (!class_tree[req])
                    class_tree[req] := array{};
                endif
                class_tree[req].append(hab);
            endif
        endif
    endforeach

    // AFTER building the tree, handle base_hab selection
    if (subclass)
        base_hab := subclass;
    else
        // Randomly select one of the base abilities if no subclass was specified
        if (class_choice.size() > 0)
            base_hab := class_choice[RandomInt(class_choice.size())+1];
        endif
    endif

    // Only proceed if we have a valid base ability
    if (base_hab)
        // Start with the base ability
        progression.append(base_hab);
        
        // Follow the prerequisite chain to build full progression
        var current := base_hab;
        while (current && class_tree[current])
            foreach next_hab in (class_tree[current])
                if (!(next_hab in progression))
                    progression.append(next_hab);
                endif
                sleepms(2);
            endforeach
            current := progression[progression.size()];
        endwhile
    endif

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

function ChooseWarriorAbility(npc, state, habs)
    // Get count of nearby enemies
    var enemies := ListHostiles(npc, 3);
    var opponent := npc.opponent;

    if (Distance(me, opponent) > 1)
        return 0;
    endif
    
    // Emergency abilities when low health
    if (state.health == "dying")
        foreach hab in habs
            case (hab)
                "Manobra da Resiliencia":
                "Shintai to Kokoro": 
                "Shar Krith":
                "Nazg Gûl":
                "Resiliência Bárbara": return hab;
            endcase
        endforeach
    endif
    
    // Multiple enemies
    if (enemies.size() >= 2)
        foreach hab in habs
            case (hab)
                "Golpes Amplos":
                "Trespassar":
                "Provocar":
                "Manobra Desafiadora":
                "Burz":
                "Ghâshburz": return hab;
            endcase
        endforeach
    endif

    // Against ranged opponents
    if (opponent.weapon.isRanged())
        foreach hab in habs
            case (hab)
                "Ryū no Kamae": return hab;
            endcase
        endforeach
    endif

    if (opponent.master)
        foreach hab in habs
            case (hab)
                "Caçador Implacável":
                "Nazg khalug": return hab;
            endcase
        endforeach
    endif

    var chosen_hab := habs[RandomInt(habs.size())+1]; // Fallback to random
    if (chosen_hab == "Caçador Experiente")
        var net := CreateItemInBackpack(npc, 0x1EA3, 1);
        UseItem(net, npc);
    endif

    return chosen_hab;
endfunction

function ChooseRogueAbility(npc, state, habs)
    var opponent := npc.opponent;

    if (Distance(me, opponent) > 1)
        return 0;
    endif

    // Emergency abilities when low health
    if (state.health == "dying")
        foreach hab in habs
            case (hab)
                "Distrair Oponentes": return hab;
            endcase
        endforeach
    endif

    if (AP_GetVital(npc, STAMINA) < 40)
        foreach hab in habs
            case (hab)
                "Reflexo Estrategico": return hab;
            endcase
        endforeach
    endif
    
    // If hidden, prioritize surprise attacks
    if (npc.hidden)
        foreach hab in habs
            case (hab)
                "Ataque Furtivo":
                "Salto das Sombras": return hab;
            endcase
        endforeach
    endif

    if (opponent.master)
        foreach hab in habs
            case (hab)
                "Fantasma":
                "Nazg khalug": return hab;
            endcase
        endforeach
    endif

    // When behind opponent
    if (state.has_advantage)
        foreach hab in habs
            case (hab)
                "Faro por Sangue":
                "Golpe Sangrento": return hab;
            endcase
        endforeach
    endif
    
    // Against mages
    if (opponent.isSage())
        foreach hab in habs
            case (hab)
                "Ladrao de Magias":
                "Lamina da Escuridao": return hab;
            endcase
        endforeach
    endif
      
    return habs[RandomInt(habs.size())+1];
endfunction

function ChooseSageAbility(npc, state, habs)
    var opponent := npc.opponent;
    
    // Emergency abilities when low health
    if (state.health == "dying")
        foreach hab in habs
            case (hab)
                "Tactus Tenebris":
                "Guardião do Bosque":
                "Instinto de Sobrevivência":
                "Prisão Dimensional":
                "Aegis Vitalis":
                "Decretum Venerationis":
                "Sede de Sangue": 
                "Salto Etéreo":
                "Pele de pedra":
                "Abrir Portais": return hab;
            endcase
        endforeach
    endif

    // Find corpses nearby
    var nearby_corpses := ListItemsNearLocation(npc.x, npc.y, npc.z, 5);
    foreach item in nearby_corpses
        if (item.IsA(POLCLASS_CORPSE))
            foreach hab in habs
                case (hab)
                    "Controlar Mortos":
                    "Colheita de Almas": return hab;
                endcase
            endforeach
            break;
        endif
        sleepms(2);
    endforeach

    // Multiple enemies
    var enemies := ListHostiles(npc, 8);
    if (enemies.size() >= 2)
        foreach hab in habs
            case (hab)
                "Natureza Selvagem":
                "Tormento de Idrith":
                "Provocar":
                "Ancestralidade Elemental": return hab;
            endcase
        endforeach
    endif

    // Against mages
    if (opponent.isSage())
        foreach hab in habs
            case (hab)
                "Vindicta Sancta": return hab;
            endcase
        endforeach
    endif

    if (opponent.hidden)
        foreach hab in habs
            case (hab)
                "Visao verdadeira": return hab;
            endcase
        endforeach
    endif

    if (opponent.master)
        foreach hab in habs
            case (hab)
                "Elo Natural": Start_Script(":spells:commands/player/assumirforma", array{npc, NPC_ParseTemplateName(npc.npctemplate).template}); 
                "Treinamento Druídico": return hab; // controlar a besta
            endcase
        endforeach
    endif
       
    return habs[RandomInt(habs.size())+1];
endfunction

// TODO: Criar uma IA pro Ladrao/Punguista
function HandleEnteredAreaAbilities(opponent, is_ranged := 0)
    var habs := GetObjProperty(me, "myhabs");
    foreach hab in habs
        case (hab)
            "Camuflagem": 
                if (!me.hidden) 
                    me.hidden := 1;
                endif
                ProccessHab(me, hab);
            "Invocacao do Caos": 
            "Gwaenedh":
            "Forma Adaptavel":
            "Irmaos de Areia":
            "Canhoneiro":
            "Quinquilharias":
            "Construtor de Golems": 
                ProccessHab(me, hab);
        endcase
    endforeach
endfunction

function HandleAssassinBehavior(opponent)
    if (!me.hidden && Distance(me, opponent) <= 6)
        GrantPrivilege(me, "hiddenattack");
        me.enable("hiddenattack");
        me.hidden := 1;
        me.stealthsteps := 30;
        
        if (GetCooldown(me, "FaroPorSangue"))
            SetObjProperty(me, "backstab", 1);
            SetObjProperty(me.weapon, "backstab", 1);
            me.attack_once(opponent);
        endif
    endif
endfunction

function HandleThiefBehavior(opponent)
    if (!GetCooldown(me, "thief_abilities"))
        if (!me.opponent) // Try to steal first
            DoThiefSteal(opponent);
        else // In combat 
            var npcState := getNPCState(me);
            if (npcState == "dying" && TemHabilidade(me, "Fuga de Emergencia"))
                FleeFrom(opponent);
                SetCooldown(me, "thief_abilities", 30);
            endif
        endif
    endif
endfunction

function ShooterBehavior(me)
    if (GetCooldown(me, "shooter_behavior"))
        return;
    endif

    var equipped_weapon := GetEquipmentByLayer(me, LAYER_HAND1);
    if (!equipped_weapon)
        equipped_weapon := GetEquipmentByLayer(me, LAYER_HAND2);
    endif

    if (equipped_weapon)
        UseItem(equipped_weapon, me);
        SetCooldown(me, "shooter_behavior", 1); // Very short cooldown to prevent script overload
    endif
endfunction