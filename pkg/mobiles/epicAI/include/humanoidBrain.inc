include "include/sysEvent";
include "include/sounds";
include ":combat:Controller/DamageController_legacy";
include "include/facings";
include ":npcs:thor/handling/thorai";
include "epicSpecial";
include "NPCMageryUtils";
include ":abilities:include/models/Ability";
include ":wrappers:Objects/Model/MobileWrapper";
include ":charactercreation:include/resists";
include ":epicAI:include/npcDataCache";    // NPC_GetCachedData (used by GetClassProgression)

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

    // Grant stealth privileges once for Ladino NPCs (avoids per-steal/sneak GrantPrivilege calls)
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

        // If we have an opponent, maintain combat position regardless of events
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
                HandleHumanoidLeftArea(is_ranged);
            else
                if (GetObjProperty(me, "metamorfose")) is_ranged := 0; endif
                if (SM_GetState() != "FLEEING")
                    if (!TickHazardEscape())
                        PursueTarget(me.opponent);
                    endif
                endif
                // Re-stealth before the chooser runs so ASSASSINAR's hidden-gate
                // priority lands its #GuaranteedHit bonus. No-op for archetypes
                // without ASSASSINAR; SkillCheck + 3s cooldown self-throttle.
                if (me.isThief())
                    _AttemptCombatStealth(me, MobileWrapper(me));
                endif
                // Per-tick casting + abilities — keep offensive pressure
                UseAbilities(me, me.opponent);
                TickCombatAbilities(me, me.opponent, combat_abilities);
                if (me.isShaman())
                    HumanoidShamanBehavior(me, me.opponent);
                endif
                if (me.isPriest())
                    HumanoidPriestBehavior(me, me.opponent);
                endif
            endif
        endif

        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED:
                    AddThreat(ev.source, 0);
                    AlertAllies(ev.source, me);
                    HandleHumanoidCombatEvent(ev.source, is_ranged);
                SYSEVENT_DAMAGED:
                    last_activity_time := polcore().systime;
                    // Safety net for damage paths that bypass AttackLogger (DoTs,
                    // environmental). Registers source at zero; EVID_THREAT_UPDATE
                    // delivers the actual amount when AttackLogger fires.
                    AddThreat(ev.source, 0);
                    UpdateThreatList(me, ev);
                    // Break stealth states on damage
                    var dmg_state := SM_GetState();
                    if (dmg_state == "AMBUSH" || dmg_state == "STEALING" || dmg_state == "SNEAKING")
                        me.hidden := 0;
                        SM_TransitionTo("COMBAT");
                    endif
                    AskForHealing(me);
                    HandleHumanoidCombatEvent(ev.source, is_ranged);
                EVID_THREAT_UPDATE:
                    AddThreat(ev.source, ev.amount);
                SYSEVENT_ENTEREDAREA:
                    if (CheckDarknessCreatureSurvival(me))
                        return;
                    endif
                    HandleHumanoidSpeech(ev.text, ev.source, is_ranged);
                    if (!IsAlly(ev.source))
                        HandleEnteredAreaAbilities(ev.source, is_ranged);
                        UpdateThreatList(me, ev);
                        AlertAllies(ev.source, me);
                        HandleHumanoidEnteredArea(ev.source, is_ranged);
                    endif
                SYSEVENT_LEFTAREA:
                    if (!IsAlly(ev.source))
                        UpdateThreatList(me, ev);
                        // Only drop combat state if the leaver was our actual opponent.
                        // Otherwise nearby movement (other players, pets, ally deaths)
                        // would wipe opponent + threat table mid-fight and strand the NPC.
                        if (me.opponent && ev.source.serial == me.opponent.serial)
                            HandleHumanoidLeftArea(is_ranged);
                        endif
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
                    InjectTauntThreat(ev.source);
                    HandleHumanoidCombatEvent(ev.source, is_ranged);
                EVID_NOISE:
                    last_activity_time := polcore().systime;
                    if (!me.opponent && SM_GetState() == "IDLE" && me.alignment != 0)
                        print("[noise rx] " + me.name + " investigating noise from " + ev.source.name);
                        SM_TransitionTo("INVESTIGATING", struct{
                            "noise_x" := ev.noise_x,
                            "noise_y" := ev.noise_y
                        });
                    endif
                EVID_BUDDYATTACKED:
                    if (!GetCooldown(me, "buddy_heal"))
                        var hurt_ally := ev.source;
                        if (hurt_ally && !hurt_ally.dead && isHurt(hurt_ally))
                            if (me.isShaman() || me.isPriest())
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
                    endif
                EVID_FLEE:
                    last_activity_time := polcore().systime;
                    SM_TransitionTo("FLEEING", struct{ "target" := ev.source, "steps" := 0 });
            endcase
        endif

        // Idle behavior when no events and no combat
        if (!me.opponent && SM_GetState() == "IDLE")
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
        if (me.opponent || SM_GetState() != "IDLE" || (polcore().systime - last_activity_time) < 5)
            sleepms(100); // Combat mode, active state, or recently active
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
            SM_TransitionTo("FLEEING", struct{ "target" := source, "steps" := 0 });
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
    ClearCombatState();

    var threat := CheckNearbyThreats();
    if (threat)
        HandleHumanoidCombatEvent(threat, is_ranged);
        return 1;
    endif

    // Corpse looting moved to HumanoidIdle to avoid item scans on every LEFTAREA event
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
        SM_TransitionTo("FLEEING", struct{ "target" := opponent, "steps" := 0 });
        return 1;
    endif

    UseAbilities(me, opponent);

    // Let the combat hook handle the actual attacks
    if (is_ranged)
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

    // Dispatch shared abilities from pre-built table (Bite, MageBehavior, etc.)
    TickCombatAbilities(me, opponent, combat_abilities);

    // Humanoid-specific caster behaviors (heal allies, not just damage)
    if (me.isShaman())
        HumanoidShamanBehavior(me, opponent);
    endif

    if (me.isPriest())
        HumanoidPriestBehavior(me, opponent);
    endif

    if (MobileWrapper(me).HasAbility(ABILITY_ID::ALQUIMISTA_EXPERIENTE))
        NPC_UseAbilityById(me, ABILITY_ID::ALQUIMISTA_EXPERIENTE);
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

function HandleHumanoidEnteredArea(source, is_ranged)
    if (!source)
        return 0;
    endif

    // Dispatch enter-area abilities from pre-built table (Charge, TunnelTactics, Ambush)
    TickEnterAbilities(me, source, enter_abilities);

    // Thief behavior is humanoid-specific, not table-dispatched
    if (me.isThief())
        DoThiefCombatBehavior(source);
    endif

    HandleHumanoidCombatEvent(source, is_ranged);
    
    return 1;
endfunction

function NPC_UseAbilityById(npc, abilityId)
    if (!abilityId || abilityId == error)
        return 0;
    endif
    if (npc.dead)
        return 0;
    endif

    var habcfg := ReadConfigFile(":abilities:configs/habilidades");
    var habelem := FindConfigElem(habcfg, abilityId);
    if (!habelem)
        return 0;
    endif

    var uso := GetConfigString(habelem, "uso");
    if (uso != "Ativa")
        return 0;
    endif

    // Delegate ALL validation, resource consumption, and cooldown to AbilityMonitor
    var ability := Ability(abilityId);
    var result := AbilityMonitor(ability).ActivateOn(MobileWrapper(npc));
    if (result.errortext)
        return 0;
    endif

    // Visual feedback: emote the ability name above the NPC so players can
    // see when something fired (and react/learn the tells).
    var ability_name := GetConfigString(habelem, "nome");
    if (ability_name)
        PrintTextAbove(npc, "*"+ability_name+"*");
    endif

    return 1;
endfunction

// Abilities the chooser must never select for an NPC. Two reasons land here:
//   1. Hard player-only gates (party/UI) where CanActivate always returns false
//      for NPCs (AURA_DE_FE, PRESENCA_DIVINA).
//   2. Per-file IsNPC fail-fast guards (CONTROLAR_MORTOS, CONVOCACAO_SELVAGEM,
//      TRANSMUTAR, TRUQUE_SUJO) — selecting them just burns the 2s ability
//      cooldown on a guaranteed-false ExecuteOn.
function _IsNPCBlacklistedAbility(id)
    case (id)
        ABILITY_ID::AURA_DE_FE:           return 1;
        ABILITY_ID::PRESENCA_DIVINA:      return 1;
        ABILITY_ID::TRANSMUTAR:           return 1;
        ABILITY_ID::TRUQUE_SUJO:          return 1;
        ABILITY_ID::CONTROLAR_MORTOS:     return 1;
        ABILITY_ID::CONVOCACAO_SELVAGEM:  return 1;
    endcase
    return 0;
endfunction

// Re-stealth between attacks for Assassino-subclass NPCs so the next
// ASSASSINAR call lands the #GuaranteedHit bonus (the chooser already
// prioritizes ASSASSINAR when npc.hidden). Ladino NPCs receive
// runwhilestealth at brain init, so positioning preserves the hidden
// state.
//
// Cooldown-throttled: a failed SkillCheck still eats the cooldown so we
// don't spam SNEAK every 100ms tick. Successful re-stealth is consumed
// almost immediately by the next ASSASSINAR attack.
function _AttemptCombatStealth(npc, wrapper)
    if (npc.hidden)
        return 0;
    endif
    if (GetCooldown(npc, "combat_stealth"))
        return 0;
    endif
    if (!wrapper.HasAbility(ABILITY_ID::ASSASSINAR))
        return 0;
    endif

    SetCooldown(npc, "combat_stealth", 3);
    if (SkillCheck(npc, SNEAK, -1) > 0)
        npc.hidden := 1;
        npc.stealthsteps := 30;
        return 1;
    endif
    return 0;
endfunction

// Reads the NPC's myhabs live (same data shape as players) and returns only
// the abilities tagged uso=Ativa in config — passives fire automatically via
// the combat damage pipeline, so the chooser never needs to see them.
// Blacklisted IDs (see _IsNPCBlacklistedAbility) are also excluded.
function GetActiveHabs(npc)
    var abilities := MobileWrapper(npc).GetAbilities();
    if (!abilities)
        return array{};
    endif

    var habcfg := ReadConfigFile(":abilities:configs/habilidades");
    var actives := array{};
    foreach slot in (abilities.keys())
        var id := CInt(abilities[slot]);
        if (_IsNPCBlacklistedAbility(id))
            sleepms(2);
            continue;
        endif
        var elem := FindConfigElem(habcfg, id);
        if (elem && GetConfigString(elem, "uso") == "Ativa")
            actives.append(id);
        endif
        sleepms(2);
    endforeach
    return actives;
endfunction

function UseAbilities(npc, opponent)
    if (GetCooldown(npc, "ability_cooldown"))
        return 0;
    endif

    var actives := GetActiveHabs(npc);
    if (actives.size() == 0)
        return 0;
    endif

    var state := GetCombatState(npc, opponent);
    var best_hab := ChooseAbility(npc, state, actives);
    if (!best_hab)
        return 0;
    endif

    var success := CInt(NPC_UseAbilityById(npc, best_hab));
    // Always set cooldown — 5s on success, 2s on failure (prevents 100ms spam)
    if (success)
        SetCooldown(npc, "ability_cooldown", 5);
    else
        SetCooldown(npc, "ability_cooldown", 2);
    endif
    return success;
endfunction

function GetCombatState(npc, opponent)
   return struct{
      "health" := getNPCState(npc),
      "distance" := Distance(npc, opponent),
      "stamina" := AP_GetVital(npc, "Stamina"),
      "opponent_casting" := opponent.isSage(),
      "has_advantage" := IsBehind(npc, opponent.x, opponent.y),
      "is_ranged" := npc.weapon && npc.weapon.attribute == "Ranged"
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
  var progression := GetClassProgression(habcfg, subclass, npc);

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

function GetClassProgression(habcfg, subclass, npc)
    var progression := array{};
    var class_choice := array{};
    var cache := NPC_GetCachedData(npc);  // O(1) lookup
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

function ChooseWarriorAbility(npc, state, habs)
    var opponent := npc.opponent;
    var wrapper := MobileWrapper(npc);

    // Melee warriors shouldn't burn abilities while still chasing.
    // Ranged warriors (archers) skip the gate — their abilities are designed
    // for distance and would never trigger otherwise.
    if (!state.is_ranged && state.distance > 1)
        return 0;
    endif

    // Light up Furia first — it's the prerequisite for the entire barbaro
    // chain (RESILIENCIA_BARBARA / FURIA_INDOMAVEL / FURIA_IMPLACAVEL all
    // gate on the "furia" cooldown that Fury.OnActivate sets).
    if (wrapper.HasAbility(ABILITY_ID::FURIA) && !GetCooldown(npc, "furia"))
        return ABILITY_ID::FURIA;
    endif

    // Mutante self-buff — fire FORMA_ADAPTAVEL once so the resist mod is in
    // place before random combat picks. AdaptableForm.ExecuteOn writes the
    // "Mutacao" ObjProperty and erases it on toggle-off.
    if (wrapper.HasAbility(ABILITY_ID::FORMA_ADAPTAVEL) && !GetObjProperty(npc, "Mutacao"))
        return ABILITY_ID::FORMA_ADAPTAVEL;
    endif

    if (state.health == "dying")
        if (wrapper.HasAbility(ABILITY_ID::MANOBRA_DA_RESILIENCIA))  return ABILITY_ID::MANOBRA_DA_RESILIENCIA;  endif
        // RESILIENCIA_BARBARA.CanActivate requires the furia cooldown to be live.
        if (GetCooldown(npc, "furia") && wrapper.HasAbility(ABILITY_ID::RESILIENCIA_BARBARA))     return ABILITY_ID::RESILIENCIA_BARBARA;     endif
    endif

    var enemies := NPC_GetPlayersPetsAndSummonsNearby(npc);
    if (enemies.size() >= 2)
        if (wrapper.HasAbility(ABILITY_ID::PROVOCAR))              return ABILITY_ID::PROVOCAR;              endif
        if (wrapper.HasAbility(ABILITY_ID::MANOBRA_DESAFIADORA))   return ABILITY_ID::MANOBRA_DESAFIADORA;   endif
    endif

    if (opponent.master)
        if (wrapper.HasAbility(ABILITY_ID::CACADOR_IMPLACAVEL))    return ABILITY_ID::CACADOR_IMPLACAVEL;    endif
    endif

    return habs[RandomInt(habs.size())+1];
endfunction

function ChooseRogueAbility(npc, state, habs)
    var opponent := npc.opponent;
    var wrapper := MobileWrapper(npc);

    // Melee rogues gate on adjacency; ranged rogues (e.g. crossbow Caçadores) skip it.
    if (!state.is_ranged && state.distance > 1)
        return 0;
    endif

    // Shinobi foundation — NINJA_PATH writes "#ninja_mark" + "#ninja_mark_expires"
    // which VITAL_STRIKE/PHANTOM_STRIKE damage-amplify off. Without the mark,
    // dependents fire as plain hits. Re-fire when the mark window has lapsed.
    if (wrapper.HasAbility(ABILITY_ID::NINJA_PATH) && !GetObjProperty(npc, "#ninja_mark_expires"))
        return ABILITY_ID::NINJA_PATH;
    endif

    if (state.health == "dying")
        if (wrapper.HasAbility(ABILITY_ID::DISTRAIR_OPONENTES))   return ABILITY_ID::DISTRAIR_OPONENTES;   endif
    endif

    if (AP_GetVital(npc, STAMINA) < 40)
        if (wrapper.HasAbility(ABILITY_ID::POSTURA_DE_DUELO))  return ABILITY_ID::POSTURA_DE_DUELO;  endif
    endif

    if (npc.hidden)
        if (wrapper.HasAbility(ABILITY_ID::ASSASSINAR))            return ABILITY_ID::ASSASSINAR;            endif
    endif

    if (opponent.master)
        if (wrapper.HasAbility(ABILITY_ID::FANTASMA))              return ABILITY_ID::FANTASMA;              endif
    endif

    if (state.has_advantage)
        if (wrapper.HasAbility(ABILITY_ID::FARO_POR_SANGUE))      return ABILITY_ID::FARO_POR_SANGUE;      endif
        if (wrapper.HasAbility(ABILITY_ID::GOLPE_SANGRENTO))       return ABILITY_ID::GOLPE_SANGRENTO;       endif
    endif

    if (opponent.isSage())
        if (wrapper.HasAbility(ABILITY_ID::LADRAO_DE_MAGIAS))     return ABILITY_ID::LADRAO_DE_MAGIAS;     endif
        if (wrapper.HasAbility(ABILITY_ID::LAMINA_DA_ESCURIDAO))  return ABILITY_ID::LAMINA_DA_ESCURIDAO;  endif
    endif

    return habs[RandomInt(habs.size())+1];
endfunction

function ChooseSageAbility(npc, state, habs)
    var opponent := npc.opponent;
    var wrapper := MobileWrapper(npc);
    var has_souls := CInt(GetObjProperty(npc, "shaman_souls")) > 0;

    // Engenheiro foundations — fire GOLEM_ASSEMBLY before any siblings if not
    // already piloting (sets the "pilotando" prop that SALTO_A_VAPOR /
    // OMBREIRA_TATICA / EXPLOSIVE_FIST ammo path read). Then EXPLOSIVE_FIST
    // toggles the "#explosive_fist" damage bonus.
    if (wrapper.HasAbility(ABILITY_ID::GOLEM_ASSEMBLY) && !GetObjProperty(npc, "pilotando"))
        return ABILITY_ID::GOLEM_ASSEMBLY;
    endif
    if (wrapper.HasAbility(ABILITY_ID::EXPLOSIVE_FIST) && !GetObjProperty(npc, "#explosive_fist"))
        return ABILITY_ID::EXPLOSIVE_FIST;
    endif

    // Arqueiro Arcano foundation — ARCANE_BOW sets the "arcane_bow" cooldown
    // that CHANNELED_SHOT / PRISMATIC_SHOT need to fire. Without it the
    // dependents fail CanActivate. (Religion=LORE is set at NPC spawn — until
    // then this gate fails harmlessly and falls through.)
    if (wrapper.HasAbility(ABILITY_ID::ARCANE_BOW) && !GetCooldown(npc, "arcane_bow"))
        return ABILITY_ID::ARCANE_BOW;
    endif

    if (state.health == "dying")
        if (wrapper.HasAbility(ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA)) return ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA; endif
        if (wrapper.HasAbility(ABILITY_ID::PRISAO_DIMENSIONAL))        return ABILITY_ID::PRISAO_DIMENSIONAL;        endif
        // Bloodthirst heals Theology/25 per soul — without souls it heals trivially.
        if (has_souls && wrapper.HasAbility(ABILITY_ID::BLOODTHIRST))  return ABILITY_ID::BLOODTHIRST;               endif
        if (wrapper.HasAbility(ABILITY_ID::SALTO_ETEREO))              return ABILITY_ID::SALTO_ETEREO;              endif
        if (wrapper.HasAbility(ABILITY_ID::ABRIR_PORTAIS))             return ABILITY_ID::ABRIR_PORTAIS;             endif
        // RESILIENCIA_BARBARA also requires Furia (gate same as the warrior chooser).
        if (GetCooldown(npc, "furia") && wrapper.HasAbility(ABILITY_ID::RESILIENCIA_BARBARA))       return ABILITY_ID::RESILIENCIA_BARBARA;       endif
    endif

    if (opponent.hidden && npc.getRace() == DROW)
        if (wrapper.HasAbility(ABILITY_ID::CONEXAO_DIVINA))            return ABILITY_ID::CONEXAO_DIVINA;            endif
    endif

    if (wrapper.HasAbility(ABILITY_ID::CONTROLAR_MORTOS) || wrapper.HasAbility(ABILITY_ID::SOUL_HARVEST))
        foreach item in ListItemsNearLocation(npc.x, npc.y, npc.z, 5, npc.realm)
            if (item.IsA(POLCLASS_CORPSE))
                if (wrapper.HasAbility(ABILITY_ID::CONTROLAR_MORTOS)) return ABILITY_ID::CONTROLAR_MORTOS; endif
                if (wrapper.HasAbility(ABILITY_ID::SOUL_HARVEST))     return ABILITY_ID::SOUL_HARVEST;     endif
            endif
        endforeach
    endif

    if (wrapper.HasAbility(ABILITY_ID::PROVOCAR) || wrapper.HasAbility(ABILITY_ID::ELEMENTAL_ANCESTRY))
        var enemies := NPC_GetPlayersPetsAndSummonsNearby(npc);
        if (enemies.size() >= 2)
            if (wrapper.HasAbility(ABILITY_ID::PROVOCAR))              return ABILITY_ID::PROVOCAR;              endif
            // Elemental Ancestry consumes shaman_souls — same heal-per-soul math.
            if (has_souls && wrapper.HasAbility(ABILITY_ID::ELEMENTAL_ANCESTRY)) return ABILITY_ID::ELEMENTAL_ANCESTRY; endif
        endif
    endif

    if (opponent.master)
        if (wrapper.HasAbility(ABILITY_ID::CORACAO_DA_MATILHA))    return ABILITY_ID::CORACAO_DA_MATILHA;    endif
        if (wrapper.HasAbility(ABILITY_ID::ELO_NATURAL))           return ABILITY_ID::ELO_NATURAL;           endif
    endif

    // Filter shaman soul-consumers from the random fallback when no souls are
    // banked (SOUL_HARVEST is a passive on-kill hook, so souls only land via
    // combat — until then these abilities have nothing to spend).
    if (!has_souls)
        var filtered := array{};
        foreach hab in habs
            var skip := 0;
            case (hab)
                ABILITY_ID::BLOODTHIRST:
                ABILITY_ID::SAND_BROTHERS:
                ABILITY_ID::ELEMENTAL_ANCESTRY:
                ABILITY_ID::BLOOD_ARENA:
                    skip := 1;
            endcase
            if (!skip)
                filtered.append(hab);
            endif
            sleepms(2);
        endforeach
        if (filtered.size() > 0)
            return filtered[RandomInt(filtered.size())+1];
        endif
    endif

    return habs[RandomInt(habs.size())+1];
endfunction

function HandleEnteredAreaAbilities(opponent, is_ranged := 0)
    var actives := GetActiveHabs(me);
    if (actives.size() == 0)
        return;
    endif
    foreach hab in actives
        case (hab)
            ABILITY_ID::CAMUFLAGEM:
                NPC_UseAbilityById(me, hab);
            ABILITY_ID::INVOCACAO_DO_CAOS: 
            ABILITY_ID::ARCANE_BOW:
            ABILITY_ID::FORMA_ADAPTAVEL:
            ABILITY_ID::SAND_BROTHERS:
            ABILITY_ID::TACTICAL_SHOULDER:
            ABILITY_ID::TARGETING_VISOR:
            ABILITY_ID::GOLEM_ASSEMBLY:
                NPC_UseAbilityById(me, hab);
        endcase
    endforeach
endfunction

function HandleThiefBehavior(opponent)
    if (!GetCooldown(me, "thief_abilities"))
        if (!me.opponent) // Try to steal first
            DoThiefSteal(opponent);
        else // In combat 
            var npcState := getNPCState(me);
            if (npcState == "dying" && MobileWrapper(me).HasAbility(ABILITY_ID::FUGA_DE_EMERGENCIA))
                SM_TransitionTo("FLEEING", struct{ "target" := opponent, "steps" := 0 });
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