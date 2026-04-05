use npc;
include "include/sysEvent";
include "include/sounds";
include ":combat:Controller/DamageController_legacy";
include "include/facings";
include ":npcs:thor/handling/thorai";
include "epicSpecial";
include "NPCMageryUtils";
include "include/epicLoggerUtils";
include ":abilities:include/models/Ability";
include ":wrappers:Objects/Model/MobileWrapper";

function HumanoidBrain()
    var ev;
    var wanders := 0;
    var last_activity_time := polcore().systime;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    // Get humanoid configuration - O(1) cache lookup
    var cache := NPC_GetCachedData(me);
    var speed_type := SplitWords(cache.attack_speed, "_");
    var is_ranged := me.weapon.attribute == "Ranged" || (me.isSage() && me.weapon.objtype == FIST);

    // Enable archer-specific abilities if needed
    if (MobileWrapper(me).HasAbility(ABILITY_ID::TIRO_EM_MOVIMENTO))
        GrantPrivilege(me, "firewhilemoving");
        me.enable("firewhilemoving");
    endif

    // Load Combat History - use cached template
    var npc_template := cache.template;
    var combat_history := LoadCombatHistory(npc_template);

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
    
        // If we have an opponent, maintain combat position regardless of events
        if (me.opponent)
            last_activity_time := polcore().systime;
            if (!CanFight(me, me.opponent))
                HandleHumanoidLeftArea(me.opponent);
            else
                var dist := Distance(me, me.opponent);
                if (GetObjProperty(me, "metamorfose")) is_ranged := 0; endif

                // PursueTarget now handles ideal distance calculation internally
                // Let it decide when to stop based on weapon type
                PursueTarget(me.opponent);
            endif
        endif
        
        if (ev)            
            case (ev.type)
                SYSEVENT_ENGAGED:
                    AlertAllies(ev.source, me);
                    HandleHumanoidCombatEvent(ev.source, is_ranged);
                SYSEVENT_DAMAGED:
                    last_activity_time := polcore().systime;
                    AskForHealing(me);
                    HandleHumanoidCombatEvent(ev.source, is_ranged);
                SYSEVENT_ENTEREDAREA:
                    HandleHumanoidSpeech(ev.text, ev.source, is_ranged);
                    HandleEnteredAreaAbilities(ev.source, is_ranged);
                    if (!IsAlly(ev.source))
                        // Phase 3: Update threat list cache
                        UpdateThreatList(me, ev);
                        AlertAllies(ev.source, me);
                        HandleHumanoidEnteredArea(ev.source, is_ranged);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source))
                        // Phase 3: Update threat list cache
                        UpdateThreatList(me, ev);
                        HandleHumanoidLeftArea(is_ranged);
                    endif
                EVID_ALERT_ALLIES:
                    var opponent_serial := Cint(GetObjProperty(me, "#taunt_opponent"));
                    if (opponent_serial)
                        var opponent := SystemFindObjectBySerial(opponent_serial);
                        if (opponent)
                            HandleHumanoidCombatEvent(opponent, is_ranged);
                            EraseObjProperty(me, "#taunt_opponent");
                        endif
                    endif
                EVID_TAUNT:
                    last_activity_time := polcore().systime;
                    // Taunt forces a target switch - clear current opponent first
                    SetOpponent(0);
                    HandleHumanoidCombatEvent(ev.source, is_ranged);
                EVID_BUDDYATTACKED:
                    if (me.isShaman())
                        HumanoidShamanBehavior(me, ev.source);
                    endif
                    if (me.isPriest())
                        HumanoidPriestBehavior(me, ev.source);
                    endif
                EVID_FLEE:
                    last_activity_time := polcore().systime;
                    FleeFrom(ev.source);
            endcase
        endif

        // Idle behavior when no events and no combat
        if (!me.opponent)
            HumanoidIdle(wanders, is_ranged);
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
    // var response := handle_combat_speech(me.name, text, source.getRace(), npcState);
    // if (response == error)
    //     return Say(handle_error_combat_AI_speech());
    // endif

    // Say(response);
    SetCooldown(me, "combat_speech", 15);
    return 1;
endfunction

function HandleHumanoidLeftArea(is_ranged)
    // Clear current combat state
    ClearCombatState();
    
    // Check for nearby threats first
    var threat := CheckNearbyThreats();
    if (threat)
        HandleHumanoidCombatEvent(threat, is_ranged);
        return 1;
    endif

    // If no opponents found, look for corpses to loot
    var corpses := array{};
    foreach item in ListItemsNearLocation(me.x, me.y, me.z, AREA_SIZE, me.realm)
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

    // Re-check for threats after looting attempt (in case one appeared while looting)
    threat := CheckNearbyThreats();
    if (threat)
        HandleHumanoidCombatEvent(threat, is_ranged);
        return 1;
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

    // Check for health-based teleport
    CheckHealthTeleport(me);

    // Check if NPC is taunted - if so, only fight the taunter
    if (GetCooldown(me, "taunted") > 0)
        var taunter_serial := GetObjProperty(me, "#taunter");
        if (taunter_serial)
            var taunter := SystemFindObjectBySerial(taunter_serial);
            if (taunter && CanFight(me, taunter))
                // Forced to fight the taunter, ignore all other targets
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
        if (Distance(me, opponent) < 2 && !MobileWrapper(me).HasAbility(ABILITY_ID::TIRO_EM_MOVIMENTO))
            reachForMeleeWeapon(opponent);
        endif

        // PursueTarget now handles positioning for ranged attackers
        // It will maintain ideal distance internally with tolerance
        PursueTarget(opponent);

        // Continuously try to shoot while in combat
        if (me.isShooter())
            ShooterBehavior(me);
        endif

    else
        TurnToward(opponent);
        PursueTarget(opponent);
    endif

    if (me.isMage())
        MageBehavior(me, opponent);
    endif

    if (me.isShaman())
        HumanoidShamanBehavior(me, opponent);
    endif

    if (me.isPriest())
        HumanoidPriestBehavior(me, opponent);
    endif

    if (me.CanBite())
        Bite(me, opponent);
    endif

    if (MobileWrapper(me).HasAbility(ABILITY_ID::ALQUIMISTA_EXPERIENTE))
        var explosive_potions := array{0xbf13, 0xbf14, 0xbf15, 0xef60, 0xef61, 0xef62};

        foreach item in EnumerateItemsInContainer(me.backpack)
            if (item.objtype in explosive_potions)
                UseItem(item, me);
                break;
            endif
        endforeach
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

    // Removed idle target scanning - rely on ENTEREDAREA events instead (100% perf boost)

    // If didn't see anyone, try to hear footsteps (no LoS required)
    var heard_mobile := NPCHearFootsteps(me);
    if (heard_mobile)
        // Check if we can see them now (might have moved into view)
        if (CanFight(me, heard_mobile))
            HandleHumanoidCombatEvent(heard_mobile, is_ranged);
            return;
        endif

        // Only show alert and investigate if not on cooldown (prevents spam)
        if (GetCooldown(me, "heard_sound") == 0)
            Message("*!*").SetColor(500).SendAbove(me);
            SetCooldown(me, "heard_sound", 5);  // 5 second cooldown between alerts
        endif

        // Move toward the sound - NPC will investigate the last known position
        PursueTarget(heard_mobile);
    endif

    var anchor := GetObjProperty(me, "anchor");
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

function HandleHumanoidEnteredArea(source, is_ranged)
    if (!source)
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
    var cache := NPC_GetCachedData(npc);  // O(1) lookup
    var npc_class := cache.classe;

    // First determine class behavior
    case (npc_class)
        GUERREIRO: return ChooseWarriorAbility(npc, state, habs);
        LADINO: return ChooseRogueAbility(npc, state, habs);
        SABIO: return ChooseSageAbility(npc, state, habs);
        default: return habs[RandomInt(habs.size())+1];
    endcase
endfunction

// Helper function to parse ability metadata from config
function ParseAbilityMeta(elem)
  var classe := GetConfigString(elem, "classe");
  if (!classe)
    return error;
  endif

  var params := SplitWords(classe);
  return struct{
    "hab_class" := params[1],
    "level" := CInt(params[2]) * 2,
    "requisito" := GetConfigString(elem, "requisito")
  };
endfunction

function GetHabsByLevel(npc, nivel)
  if (!npc) return array{}; endif

  var valid_habs := array{};
  var habcfg := ReadConfigFile(":abilities:configs/habilidades");

  // Get subclass progression regardless of level
  var subclass := GetObjProperty(npc, "subclass");
  var progression := GetClassProgression(habcfg, subclass);

  // Add abilities from progression that match NPC's level
  foreach hab in progression
    var elem := FindConfigElem(habcfg, hab);
    var meta := ParseAbilityMeta(elem);
    if (meta == error)
      continue;
    endif

    if (meta.level <= nivel)
      valid_habs.append(hab);
    endif
  endforeach

  return valid_habs;
endfunction

function GetClassProgression(habcfg, subclass)
    var progression := array{};
    var class_choice := array{};
    var cache := NPC_GetCachedData(me);  // O(1) lookup
    var class_tree := struct{};
    var name_to_id := struct{};  // Map ability names to IDs
    var base_hab := 0;

    // Single pass: build name-to-ID mapping AND prerequisite tree
    var my_class := cache.classe;

    foreach hab_id in (GetConfigStringKeys(habcfg))
        var elem := FindConfigElem(habcfg, hab_id);
        var ability_name := GetConfigString(elem, "nome");
        if (ability_name)
            name_to_id[ability_name] := CInt(hab_id);
        endif

        var meta := ParseAbilityMeta(elem);
        if (meta == error)
            continue;
        endif

        if (meta.hab_class == my_class)
            if (!meta.requisito && meta.level == 2)
                class_choice.append(CInt(hab_id));
            endif

            if (meta.requisito)
                var requisito_id := name_to_id[meta.requisito];
                if (requisito_id)
                    var key := CStr(requisito_id);
                    if (!class_tree[key])
                        class_tree[key] := array{};
                    endif
                    class_tree[key].append(CInt(hab_id));
                endif
            endif
        endif
    endforeach

    // AFTER building the tree, handle base_hab selection
    if (subclass)
        base_hab := CInt(subclass);
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
        var current_key := CStr(current);
        while (current && class_tree[current_key])
            foreach next_hab in (class_tree[current_key])
                if (!(next_hab in progression))
                    progression.append(next_hab);
                endif
                sleepms(2);
            endforeach
            current := progression[progression.size()];
            current_key := CStr(current);
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
    var opponent := npc.opponent;

    if (Distance(me, opponent) > 1)
        return 0;
    endif

    var hab_set := dictionary;
    foreach hab in habs
        hab_set[CInt(hab)] := 1;
    endforeach

    if (state.health == "dying")
        if (hab_set.exists(ABILITY_ID::MANOBRA_DA_RESILIENCIA))  return ABILITY_ID::MANOBRA_DA_RESILIENCIA;  endif
        if (hab_set.exists(ABILITY_ID::RESILIENCIA_BARBARA))     return ABILITY_ID::RESILIENCIA_BARBARA;     endif
    endif

    var enemies := ListHostiles(npc, 3);
    if (enemies.size() >= 2)
        if (hab_set.exists(ABILITY_ID::PROVOCAR))              return ABILITY_ID::PROVOCAR;              endif
        if (hab_set.exists(ABILITY_ID::MANOBRA_DESAFIADORA))   return ABILITY_ID::MANOBRA_DESAFIADORA;   endif
    endif

    if (opponent.master)
        if (hab_set.exists(ABILITY_ID::CACADOR_IMPLACAVEL))    return ABILITY_ID::CACADOR_IMPLACAVEL;    endif
    endif

    var chosen_hab := habs[RandomInt(habs.size())+1];
    if (CInt(chosen_hab) == ABILITY_ID::CACADOR_EXPERIENTE)
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

    var hab_set := dictionary;
    foreach hab in habs
        hab_set[CInt(hab)] := 1;
    endforeach

    if (state.health == "dying")
        if (hab_set.exists(ABILITY_ID::DISTRAIR_OPONENTES))   return ABILITY_ID::DISTRAIR_OPONENTES;   endif
    endif

    if (AP_GetVital(npc, STAMINA) < 40)
        if (hab_set.exists(ABILITY_ID::REFLEXO_ESTRATEGICO))  return ABILITY_ID::REFLEXO_ESTRATEGICO;  endif
    endif

    if (npc.hidden)
        if (hab_set.exists(ABILITY_ID::ASSASSINAR))            return ABILITY_ID::ASSASSINAR;            endif
    endif

    if (opponent.master)
        if (hab_set.exists(ABILITY_ID::FANTASMA))              return ABILITY_ID::FANTASMA;              endif
    endif

    if (state.has_advantage)
        if (hab_set.exists(ABILITY_ID::FARO_POR_SANGUE))      return ABILITY_ID::FARO_POR_SANGUE;      endif
        if (hab_set.exists(ABILITY_ID::GOLPE_SANGRENTO))       return ABILITY_ID::GOLPE_SANGRENTO;       endif
    endif

    if (opponent.isSage())
        if (hab_set.exists(ABILITY_ID::LADRAO_DE_MAGIAS))     return ABILITY_ID::LADRAO_DE_MAGIAS;     endif
        if (hab_set.exists(ABILITY_ID::LAMINA_DA_ESCURIDAO))  return ABILITY_ID::LAMINA_DA_ESCURIDAO;  endif
    endif

    return habs[RandomInt(habs.size())+1];
endfunction

function ChooseSageAbility(npc, state, habs)
    var opponent := npc.opponent;

    var hab_set := dictionary;
    foreach hab in habs
        hab_set[CInt(hab)] := 1;
    endforeach

    if (state.health == "dying")
        if (hab_set.exists(ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA)) return ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA; endif
        if (hab_set.exists(ABILITY_ID::PRISAO_DIMENSIONAL))        return ABILITY_ID::PRISAO_DIMENSIONAL;        endif
        if (hab_set.exists(ABILITY_ID::BLOODTHIRST))               return ABILITY_ID::BLOODTHIRST;               endif
        if (hab_set.exists(ABILITY_ID::SALTO_ETEREO))              return ABILITY_ID::SALTO_ETEREO;              endif
        if (hab_set.exists(ABILITY_ID::ABRIR_PORTAIS))             return ABILITY_ID::ABRIR_PORTAIS;             endif
    endif

    if (hab_set.exists(ABILITY_ID::CONTROLAR_MORTOS) || hab_set.exists(ABILITY_ID::SOUL_HARVEST))
        foreach item in ListItemsNearLocation(npc.x, npc.y, npc.z, 5, npc.realm)
            if (item.IsA(POLCLASS_CORPSE))
                if (hab_set.exists(ABILITY_ID::CONTROLAR_MORTOS)) return ABILITY_ID::CONTROLAR_MORTOS; endif
                if (hab_set.exists(ABILITY_ID::SOUL_HARVEST))     return ABILITY_ID::SOUL_HARVEST;     endif
            endif
            sleepms(2);
        endforeach
    endif

    if (hab_set.exists(ABILITY_ID::PROVOCAR) || hab_set.exists(ABILITY_ID::ELEMENTAL_ANCESTRY))
        var enemies := ListHostiles(npc, 8);
        if (enemies.size() >= 2)
            if (hab_set.exists(ABILITY_ID::PROVOCAR))              return ABILITY_ID::PROVOCAR;              endif
            if (hab_set.exists(ABILITY_ID::ELEMENTAL_ANCESTRY))    return ABILITY_ID::ELEMENTAL_ANCESTRY;    endif
        endif
    endif

    if (opponent.master)
        if (hab_set.exists(ABILITY_ID::ELO_NATURAL))
            Start_Script(":spells:commands/player/assumirforma", array{npc, NPC_GetCachedData(npc).template});
        endif
        if (hab_set.exists(ABILITY_ID::CORACAO_DA_MATILHA))    return ABILITY_ID::CORACAO_DA_MATILHA;    endif
    endif

    return habs[RandomInt(habs.size())+1];
endfunction

// TODO: Criar uma IA pro Ladrao/Punguista
function HandleEnteredAreaAbilities(opponent, is_ranged := 0)
    var habs := GetObjProperty(me, "myhabs");
    foreach hab in habs
        case (hab)
            ABILITY_ID::CAMUFLAGEM: 
                if (!me.hidden) 
                    me.hidden := 1;
                endif
                ProccessHab(me, hab);
            ABILITY_ID::INVOCACAO_DO_CAOS: 
            ABILITY_ID::ARCANE_BOW:
            ABILITY_ID::FORMA_ADAPTAVEL:
            ABILITY_ID::SAND_BROTHERS:
            ABILITY_ID::TACTICAL_SHOULDER:
            ABILITY_ID::TARGETING_VISOR:
            ABILITY_ID::GOLEM_ASSEMBLY:
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
            if (npcState == "dying" && MobileWrapper(me).HasAbility(ABILITY_ID::FUGA_DE_EMERGENCIA))
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