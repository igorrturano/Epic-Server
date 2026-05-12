use polsys;

include ":timedScripts:timedScripts";
include "include/epicNPCUtils";
include "include/facings";
include ":abilities:models/Ability";
include ":destroyable:include/destroyItem";
include ":combat:include/Controller/FXController";
include ":charactercreation:include/resists";

const CHARGE_MINIMUM_DISTANCE := 5;

function StartCharge(attacker, defender)
   var dist := CoordinateDistance(attacker.x, attacker.y, defender.x, defender.y);
   if (dist < CHARGE_MINIMUM_DISTANCE)
      return 0; 
   endif

   // Store initial position to calculate charge damage later
   SetObjProperty(attacker, "#chargestart", struct{
      "x" := attacker.x,
      "y" := attacker.y,
      "target" := defender.serial
   });

   // Grant speed boost during charge
   SetCooldown(attacker, "charge", 120);
   PrintTextAbove(attacker, "*inicia carga*");
   TS_StartTimer(attacker, "velocidade", 10);

   // Impact fires when NPC reaches target (separate script watches distance)
   start_script(":epicAI:abilities/ChargeImpact", array{ attacker, defender });

   return 1;
endfunction

function SporeSpecial(attacker, defender)
    if (GetCooldown(attacker, "spore_ability") > 0)
        return 0;
    endif

    SetCooldown(attacker, "spore_ability", 15);

    // Get all potential targets in range
    var targets := ListMobilesNearLocation(attacker.x, attacker.y, attacker.z, 8);

    foreach mobile in targets
        if (mobile.npctemplate || !CanFight(attacker, mobile))
            continue; 
        endif

        // Randomly choose between charm, poison and blind
        var spore_type := RandomInt(3)+1;
        
        case (spore_type)
            1: // Charm
                charmOpponent(attacker, mobile);
            
            2: // Poison
                if (!CheckResist(mobile, FORTITUDE, RESIST_HARD))
                    PrintTextAbove(attacker, "*libera esporos venenosos*");
                    var sounds := { 560, 561, 562 };
                    PlaysoundEffect(mobile, sounds[RandomInt(sounds.size())+1]);
                    PlayStationaryEffectEx(mobile.x, mobile.y, mobile.z, mobile.realm, 0x54f7, 10, 30, 2283);
                    TS_StartTimer(mobile, "defaultPoison", 120, RandomInt(4)+1);
                endif
            
            3: // Blind
                if (!CheckResist(mobile, REFLEXES, RESIST_MEDIUM))
                    PrintTextAbove(attacker, "*libera esporos cegantes*");
                    TS_StartTimer(mobile, "blinded", 15, 0, attacker);
                endif
        endcase

        sleepms(2);
    endforeach

    return 1;
endfunction

// Non-blocking: transitions to STEALING state, one step per brain tick
function DoThiefSteal(opponent)
    if (!opponent.backpack)
        return 0;
    endif

    me.hidden := 1;
    me.stealthsteps := 30;

    SM_TransitionTo("STEALING", struct{ "target" := opponent, "phase" := "approach" });
    return 1;
endfunction

// Non-blocking: transitions to SNEAKING state, one step per brain tick
function DoThiefSneakAttack(opponent)
    me.hidden := 1;
    me.stealthsteps := 100;

    SM_TransitionTo("SNEAKING", struct{ "target" := opponent, "attempts" := 0 });
    return 1;
endfunction

function PerformHeal(shaman, targ, tier)
    if (!targ)
        return 0;
    endif

    PrintTextAbove(shaman, "*curando " + targ.name + "*");
    PerformAction(shaman, HUMAN_ANIM::CAST_AREA);
    PlaySoundEffect(shaman, SFX_SPELL_HEAL);
    PlayObjectCenteredEffect(targ, FX_HEAL_EFFECT, 7, 0x10);
    
    var heal_amount := tier * 12;
    ApplyEpicHealing(targ, heal_amount);
    
    return 1;
endfunction

function PerformHealingRain(shaman, allies)
    var mlore := CInt(AP_GetSkill(shaman, ARCANA));
    
    PrintTextAbove(shaman, "*invoca tempestade*");
    PerformAction(shaman, HUMAN_ANIM::CAST_AREA);
    
    // Heal allies
    foreach ally in allies
        if (!ally.isUnderRoof())
            HealDamage(ally, CInt(mlore/3));
            PlayObjectCenteredEffect(ally, FX_HEAL_EFFECT, 7, 0x10);
        endif
        sleepms(2);
    endforeach
    
    // Damage enemies
    foreach mobile in GetNearbyMobilesOnce(shaman, 12, LISTEX_FLAG_NORMAL)
        if (!mobile.isUnderRoof() && CanFight(shaman, mobile))
            PlayLightningBoltEffect(mobile);
            DamageFLS(mobile, RandomDiceRoll("1d10") + 20, DMG_ENERGY, shaman);
            PrintTextAbove(mobile, "*atingido por um raio*");
            SetWeatherForPlayer(mobile, 1, 50);
            _wetPowderLocal(mobile);
        endif
        sleepms(2);
    endforeach

    PlayWeatherEffect(shaman);
    return 1;
endfunction

function PlayWeatherEffect(shaman)
    var wind_sounds := array{0x15, 0x16, 0x17};
    if (RandomInt(100) < 25) // 25% chance to play sound
        PlaySoundEffect(shaman, wind_sounds[RandomInt(3) + 1]);
    endif
endfunction

function CastEnraizar(shaman, targ, diff)
   if (!targ)
      return 0;
   endif
   
   PrintTextAbove(shaman, "*conjura raízes*");
   PerformAction(shaman, HUMAN_ANIM::CAST_AREA);
   
   if (!CheckResist(targ, WILL, diff))
      var duration := CInt(AP_GetSkill(shaman, ARCANA) / 10);
      PlayStationaryEffect(targ.x, targ.y, targ.z-5, 0x6D01, 7, 40, 0, targ.realm);
      TS_StartTimer(targ, "paralysis", duration);
      SetCooldown(targ, "enraizar_cooldown", 120); // 2 minute cooldown per target
      return 1;
   endif
   
   return 0;
endfunction

function CastWeaken(shaman, targ, diff)
   if (!targ)
      return 0;
   endif

   PrintTextAbove(shaman, "*enfraquece inimigo*");
   PerformAction(shaman, HUMAN_ANIM::CAST_AREA);
   
   var bonus := CInt(AP_GetSkill(shaman, ARCANA) / 20); // Small bonus based on skill
   
   if (!CheckResist(targ, WILL, diff, -bonus))
      var duration := CInt(AP_GetSkill(shaman, ARCANA));
      var amount := 10 + CInt(bonus / 2); // Base amount plus skill bonus
      TS_StartTimer(targ, "weaken", duration, amount, shaman, 1);
      SetCooldown(targ, "weaken_cooldown", 60); // 1 minute cooldown per target
      return 1;
   endif
   
   return 0;
endfunction

function AcidSpray(me, targ)
   if (!CheckLineOfSight(me, targ))
      return 0;
   endif
   
   if (GetCooldown(me, "acid_spit") > 0)
      return 0;
   endif

   SetCooldown(me, "acid_spit", 15); // 15 second cooldown
   
   PrintTextAbove(me, "*cospe ácido*");
   PlayMovingEffectXYZHued(me, targ, 0x36d4, 5, 1, 1, 0x47, 0);
   var acid_pool := CreateItemAtLocation(targ.x, targ.y, GetMapInfo(targ.x, targ.y).z, "acid", 1, targ.realm);
   
   // Deal initial acid damage
   var acid_damage := "1d6";
   _dealAcidDamageLocal(me, targ, acid_pool, acid_damage);
   
   return 1;
endfunction

function HiveMind(me)
    // Get all nearby me allies
    // Phase 4: Use cached scan instead of direct ListMobilesNearLocationEx
    var allies := GetNearbyMobilesOnce(me, 8, LISTEX_FLAG_NORMAL);
    var ally := 0;
    
    foreach mobile in allies
        if (mobile.npctemplate && mobile != me)
            if (mobile.hasHiveMind())
                ally += 1;
            endif
        endif
        sleepms(2);
    endforeach
    
    // Apply hive mind bonus
    if (ally > 0)
        var bonus := 2;
        me.hitchance_mod := me.hitchance_mod + bonus;
        if (!GetCooldown(me, "hivemind_effect"))
            PrintTextAbove(me, "*sincroniza com a colmeia*");
            PlayObjectCenteredEffect(me, 0x376A, 7, 16);
            SetCooldown(me, "hivemind_effect", 15);
        endif
    endif
    
    return ally;
endfunction

function TunnelTactics(me, targ)
    if (GetCooldown(me, "tunnel_tactics") > 0)
        return 0;
    endif

    if (!CanFight(me, targ))
        return 0;
    endif
    
    SetCooldown(me, "tunnel_tactics", 30); // 30 second cooldown

    // Alert nearby allies to join the attack
    // Phase 4: Use cached scan instead of direct ListMobilesNearLocationEx
    var allies := GetNearbyMobilesOnce(me, 4, LISTEX_FLAG_NORMAL);
    var attacking_formation := array{me};

    foreach mobile in allies
        if (mobile.npctemplate && mobile != me)
            if (mobile.hasTunnelTactics())
                attacking_formation.append(mobile);
            endif
        endif
        sleepms(2);
    endforeach
  
    // Perform tunnel tactics for all participating allies
    foreach ally in attacking_formation
        // Make them disappear
        ally.concealed := 1;
        PlayStationaryEffect(ally.x, ally.y, ally.z, 0x3735, 10, 30);
        PrintTextAbove(ally, "*escava no subsolo*");

        // Wait a moment before emerging
        sleep(3);
        // ReCheck so they don't emerge if target is dead or missing
        if (!CanFight(ally, targ) || Distance(ally, targ) > AREA_SIZE)
            ally.concealed := 0;
            return 0;
        endif
        // Emerge behind targ in coordinated attack
        MoveObjectToLocation(ally, targ.x, targ.y, targ.z, targ.realm, MOVEOBJECT_FORCELOCATION);
        ally.concealed := 0;
        PlayStationaryEffect(ally.x, ally.y, ally.z, 0x3735, 10, 30);
        PrintTextAbove(ally, "*emerge do subsolo*");
        TS_StartTimer(ally, "hole", 5);
        if (ally.isUmberHulk())
            earthquake(ally);
        endif
        // Set all allies to attack the targ
        ally.opponent := targ;
        
        sleepms(100);
    endforeach

    return 1;
endfunction

function earthquake(attacker)
    var dmg := RandomDiceRoll("1d12");
    var nearby := ListMobilesNearLocation(attacker.x, attacker.y, attacker.z, 2 , attacker.realm );
    DanificaObjetosProximos(attacker,attacker.x, attacker.y, attacker.z,attacker.realm,struct{dano := dmg, desc := "epicSpecial: Earth Quake" },2,0x6d67);
    foreach defender in nearby
        if (!(isAlly(defender) || defender == attacker))
            PlayObjectCenteredEffect( defender, 0x6d67, 0, 10);
            TS_StartTimer(defender, "tombar", 5);
            TS_StartTimer(defender, "lentidao", 5*2);
            TS_StartTimer(defender, "paralysis", 5);
            DamageFLS(defender, dmg, DMG_FORCED, attacker);
        endif
    endforeach
endfunction

function doConfusion(opponent)
    if (GetCooldown(me, "confusion_ability") > 0)
        return 0;
    endif

    // Phase 4: Use cached scan instead of direct ListMobilesNearLocationEx
    var nearby := GetNearbyMobilesOnce(me, 8, LISTEX_FLAG_NORMAL);
    
    var enemies := array{};
    foreach mobile in nearby
        if (!isAlly(mobile) && CanFight(me, mobile))
            enemies.append(mobile);
        endif
        sleepms(2);
    endforeach

    if (enemies.size() < 1)
        return 0;
    endif

    SetCooldown(me, "confusion_ability", 30);
 	PlayMovingEffectXYZHued(me, opponent, 0xF79, 10, 10, 0, 0);

    var confusion_type := RandomInt(3)+1;
    if (enemies.size() < 2) confusion_type := RandomInt(2)+1; endif
    if (CheckResist(opponent, WILL, RESIST_HARD)) return 0; endif;

    case (confusion_type)
        1: // Paralyze
            PlayStationaryEffect(opponent.x, opponent.y, opponent.z, 0x6e0d, 5, 15);
            TS_StartTimer(opponent, "paralysis", 5);
            PrintTextAbove(opponent, "*paralisado*");
        2: // Walk randomly
            PrintTextAbove(opponent, "*movendo aleatoriamente*");
            if (!opponent.npctemplate)
                var packet := CreatePacket(0x97, 2);
                var steps := RandomInt(4) + 4;
                for i := 1 to steps
                    if (opponent.dead)
                        break;
                    endif
                    var facing := RandomInt(8);
                    var mods := GetModByFacing(facing);
                    if (!mods)
                        continue;
                    endif
                    if (CheckLOSAt(opponent, opponent.x + mods[1], opponent.y + mods[2], opponent.z))
                        packet.SetInt8(1, facing | 0x80);
                        packet.SendPacket(opponent);
                    endif
                    sleepms(150);
                endfor
            else
                SetOpponent(0);
                for i := 1 to 5
                    opponent.Wander();
                    sleepms(200);
                endfor
                PrintTextAbove(opponent, "*vagueia*");
            endif
        3: // Attack allies
            if (opponent.npctemplate)
                // For NPCs, make them attack other enemies
                var potential_targets := enemies;
                potential_targets.Erase(potential_targets.Find(opponent)); // Remove self from targets

                if (potential_targets.size() > 0)
                    var random_target := potential_targets[RandomInt(potential_targets.size())+1];
                    if (random_target.hidden || random_target.concealed) return 0; endif
                    opponent.opponent := random_target;
                    PrintTextAbove(opponent, "*ataca " + random_target.name + "*");
                endif
            else
                // For players, make them walk to and attack an ally
                if (enemies.size() > 0)
                    var random_ally := enemies[RandomInt(enemies.size())+1];
                    var coords := GetCoordsInLine(opponent.x, opponent.y, random_ally.x, random_ally.y);
                    foreach coord in coords
                        MoveObjectToLocation(opponent, coord.x, coord.y, opponent.z, opponent.realm, MOVEOBJECT_FORCELOCATION);
                        PerformAction(opponent, HUMAN_ANIM::WALK_WARMODE);
                        sleepms(200);
                        if (Distance(opponent, random_ally) <= 1)
                            if(!opponent.warmode || opponent.hidden || opponent.concealed) return 0; endif
                            opponent.attack_once(random_ally);
                            break;
                        endif
                    endforeach
                    PrintTextAbove(opponent, "*ataca aliado " + random_ally.name + "*");
                endif
            endif
    endcase
    
    return 1;
endfunction

function Bite(me, opponent)
    if (GetCooldown(me, "bite_cooldown") > 0)
        return 0;
    endif

    if (Distance(me, opponent) > 1)
        return 0;
    endif

    var cd := 15;
    var bite_dmg := "1d6";
    if (me.hasRage() && IsRaging(me))
        bite_dmg := "2d6";
        cd := 10;
    endif
    
    PrintTextAbove(me, "*morde*");
    PlaySoundEffect(me, 0x28a);
    PlayAttackFX(AttackInfo(MobileWrapper(me), MobileWrapper(false)));
    
    var resist := CheckResist(opponent, REFLEXES, RESIST_MEDIUM);
    if (!resist)
        DamageFLS(opponent, bite_dmg, DMG_FORCED, me);
    elseif (resist == -1) // Falha crítica
        TS_StartTimer(opponent, "paralysis", 5);
    else
        PrintTextAbove(opponent, "*esquiva*");
    endif

    SetCooldown(me, "bite_cooldown", cd);
    return 1;
endfunction

function HandleAmbush(me, opponent)
    // Only some monsters will ambush - 30% chance or 70% if in swamp water
    var chance := 30;
    if (inSwampWater(me))
        chance += 40;
    endif

    if (RandomInt(100) > chance)
        return 0;
    endif

    // Don't ambush if already in combat or already ambushing
    if (me.opponent || SM_GetState() == "AMBUSH")
        return 0;
    endif

    // Set hidden state
    GrantPrivilege(me, "hiddenattack");
    me.enable("hiddenattack");
    me.hidden := 1;

    // Transition to AMBUSH state - SM_TickAmbush in stateManager.inc handles the rest
    // Brain loop processes ONE tick per iteration, events still fire (damage breaks ambush)
    SM_TransitionTo("AMBUSH", struct{ "target" := opponent });
    return 1;
endfunction

function inSwampWater(who)
    var area := ListStaticsAtLocation(who.x, who.y, LIST_IGNORE_Z, 0, who.realm);
    var is_on_swamp := 0;

    foreach swamp in area
        if (Cint(swamp.objtype) >= 5850 && Cint(swamp.objtype) <= 5871)
            is_on_swamp := 1;
            break;
        endif
    endforeach

    return is_on_swamp;
endfunction

function TongueGrab(toad, opponent)
    if (GetCooldown(toad, "tongue_grab"))
        return 0;
    endif

    const TONGUE_RANGE := 5;
    var dist := Distance(toad, opponent);
    
    // Check if in range and LoS
    if (!(dist >= 3 && dist <= TONGUE_RANGE) || !CheckLineOfSight(toad, opponent))
        return 0;
    endif
    
    // Animation and sound effects
    PrintTextAbove(toad, "*dispara a língua*");
    PlayAttackFX(AttackInfo(MobileWrapper(toad), MobileWrapper(false)));
    PlaySoundEffect(toad, 0x2b2); //slurp sound
    
    // Create tongue effect from toad to player
    PlayMovingEffectXYZ(toad.x, toad.y, toad.z+5, 
                      opponent.x, opponent.y, opponent.z+5,
                      0x0F51, 8, 0, 0, toad.realm); //red line effect
    
    // Initial resistance check
    if (CheckResist(opponent, REFLEXES, RESIST_MEDIUM))
        PrintTextAbove(opponent, "*desvia da língua*");
        SetCooldown(toad, "tongue_grab", 10);
        return 0;
    endif
    
    // Success! Start dragging animation
    PrintTextAbove(opponent, "*preso pela língua*");
    opponent.frozen := 1;
    
    // Store original position to move back
    var original_x := opponent.x;
    var original_y := opponent.y;
    var original_z := opponent.z;
    
    // Calculate steps needed
    var steps := dist;
    var dx := CInt((toad.x - opponent.x) / steps);
    var dy := CInt((toad.y - opponent.y) / steps);
    
    var success := 1;
    
    // Move one tile at a time towards the toad
    for i := 1 to steps
        // Check if either creature died/disconnected
        if (opponent.dead || toad.dead)
            success := 0;
            break;
        endif
        
        // Try to move player
        var new_x := original_x + (dx * i);
        var new_y := original_y + (dy * i);
        var new_z := GetWorldHeight(new_x, new_y, opponent.realm);
        
        // Move and animation
        MoveObjectToLocation(opponent, new_x, new_y, new_z, opponent.realm, MOVEOBJECT_FORCELOCATION);
        PlayMovingEffectXYZ(toad.x, toad.y, toad.z+5, 
                          new_x, new_y, new_z+5,
                          0x0F51, 8, 0, 0, toad.realm);
        
        // Per-tile resistance check (20% chance)
        if (RandomInt(100) < 20)
            if (CheckResist(opponent, FORTITUDE, RESIST_MEDIUM))
                PrintTextAbove(opponent, "*se liberta*");
                success := 0;
                break;
            endif
        endif
        
        sleep(1);
    endfor

    opponent.frozen := 0;
    if (success)
        // Final grab damage
        var raw_dmg := RandomDiceRoll("2d6");
        DamageFLS(opponent, raw_dmg, DMG_PHYSICAL, toad);
        PrintTextAbove(toad, "*engole " + opponent.name + "*");
        TS_StartTimer(opponent, "paralysis", 5);
    endif
    
    SetCooldown(toad, "tongue_grab", 15);
    return 1;
endfunction

function ThrowSpecial(attacker, defender)
   if (!CanThrow(attacker, defender))
      return 0;
   endif

   var throw_info := GetThrowInfo(attacker);
   if (!throw_info)
      return 0;
   endif

   // Set cooldown immediately after validation
   SetCooldown(attacker, "throw_cooldown", throw_info.cooldown);

   PlayThrowAnimation(attacker);
   CreateThrowEffect(attacker, defender, throw_info);
   ApplyThrowEffect(attacker, defender, throw_info);

   return 1;
endfunction

function CanThrow(attacker, defender)
   if (!CheckLineOfSight(attacker, defender))
      return 0;
   endif
   
   if (GetCooldown(attacker, "throw_cooldown") > 0)
      return 0;
   endif

   var dist := Distance(attacker, defender);
   if (dist < 2 || dist > 12)
      return 0;
   endif

   return 1;
endfunction

function GetThrowInfo(attacker)
   var throw_item := GetObjProperty(attacker, "throwItem");
   if (!throw_item)
      return 0;
   endif

   var throw_info := struct{
      "graphic" := throw_item,
      "dmg" := "1d6",
      "cooldown" := 15,
      "effects" := array{}
   };

   case (throw_item)
      0x995b: // Toxic waste
         throw_info.cooldown := 30;
         throw_info.effects.append("apodrecer");
      0x1402: // Spear
         throw_info.effects.append("desarmar");
         throw_info.effects.append("tombar");
         throw_info.effects.append("lentidao");
         throw_info.cooldown := 150;
   endcase

   return throw_info;
endfunction

function PlayThrowAnimation(attacker)
   PrintTextAbove(attacker, "*arremessa*");
   PlayAttackFX(AttackInfo(MobileWrapper(attacker), MobileWrapper(false)));
   if (attacker.isLizardMan()) 
      attacker.graphic := 0x21;
   endif
endfunction

function CreateThrowEffect(attacker, defender, throw_info)
   PlayMovingEffectXYZHued(attacker, defender, throw_info.graphic, 5, 1, 0, 0, 0);
   CreateItemAtLocation(defender.x, defender.y, GetMapInfo(defender.x, defender.y).z, throw_info.graphic, 1, defender.realm);
endfunction

function ApplyThrowEffect(attacker, defender, throw_info)
   if (CheckResist(defender, REFLEXES, RESIST_MEDIUM))
      return 0;
   endif

   foreach effect in (throw_info.effects)
      case (effect)
         "apodrecer": TS_StartTimer(defender, "apodrecer", 20); break;
         "desarmar": 
            if (defender.shield)
               MoveObjectToLocation(defender.shield, defender.x, defender.y, defender.z, defender.realm, MOVEOBJECT_FORCELOCATION);
               TS_StartTimer(defender, "desarmar", 15);
            endif
            break;
         "tombar": TS_StartTimer(defender, "tombar", 5); break;
         "lentidao": TS_StartTimer(defender, "lentidao", 20); break;
      endcase
   endforeach

   DamageFLS(defender, throw_info.dmg, DMG_FORCED, attacker);

   return 1;
endfunction

function WebWrapCorpse(spider)
   if (GetCooldown(spider, "web_wrap"))
      return 0;
   endif

   var corpses := ListItemsNearLocation(spider.x, spider.y, spider.z, 3);
   foreach item in corpses
      if (item.IsA(POLCLASS_CORPSE) && !GetObjProperty(item, "webseal") && !GetObjProperty(item, "morto"))
         // Create web animation/effect
         Set_Critical(1);

         PlayStationaryEffect(item.x, item.y, item.z, 0xCCA1, 5, 80);
         for i:= 0 to 3
            PlaySoundEffect(spider, 0x5A); // Web sound
            PlayAttackFX(AttackInfo(MobileWrapper(spider), MobileWrapper(false)));
            sleep(1);
         endfor
         sleep(1);

         // Transform corpse appearance and set properties
         item.graphic := 0x10DA;  // Web cocoon graphic
    
         start_script(":epicai:scripts/webcheck", {item, spider});     
         PrintTextAbove(spider, "*envolve o corpo em teia*");
         SetCooldown(spider, "web_wrap", 30);
         Set_Critical(0);

         return 1;
      endif
      sleepms(2);
   endforeach

   return 0;
endfunction

function WebShot(spider, opponent)
   if (GetCooldown(spider, "web_shot"))
      return 0;
   endif
   
   if (!CheckLineOfSight(spider, opponent))
      return 0;
   endif

   var dist := Distance(spider, opponent);
   if (dist > 6)
      return 0;
   endif

   PrintTextAbove(spider, "*dispara teia*");
   PlayMovingEffect(spider, opponent, 3814, 50);

   if (!CheckResist(opponent, REFLEXES, RESIST_MEDIUM))
      PrintTextAbove(opponent, "*preso na teia*");
	  TS_StartTimer(opponent, "paralysis", 5);
   else
      TS_StartTimer(opponent, "lentidao", 10);
      PrintTextAbove(opponent, "*parcialmente preso*");
   endif

   SetCooldown(spider, "web_shot", 15);
   return 1;
endfunction

function WallClimb(spider)
   if (GetCooldown(spider, "wall_climb"))
      return 0;
   endif

   // Check if any opponents below
   var opponents := ListHostiles(spider, 6);
   if (opponents.size() < 1)
      return 0;
   endif

   // 50% chance to climb when possible
   if (RandomInt(100) < 50)
      spider.concealed := 1;
      PrintTextAbove(spider, "*sobe pela parede*");
      
      // Wait a moment
      sleep(2);
      
      // Find closest opponent
      var closest := opponents[1];
      var closest_dist := Distance(spider, closest);
      
      foreach mobile in opponents
         var dist := Distance(spider, mobile);
         if (dist < closest_dist)
            closest := mobile;
            closest_dist := dist;
         endif
         sleepms(2);
      endforeach

      GrantPrivilege(me, "hiddenattack");
      me.enable("hiddenattack");
      GrantPrivilege(me, "runwhilestealth");
      me.enable("runwhilestealth");

      // Drop down for surprise attack
      MoveObjectToLocation(spider, closest.x, closest.y, closest.z, closest.realm, MOVEOBJECT_FORCELOCATION);
      spider.concealed := 0;
      me.hidden := 1;
      me.stealthsteps := 100;
      
      PrintTextAbove(spider, "*ataca do alto*");
      SetObjProperty(spider, "#GuaranteedHit", 1);
      SetObjProperty(spider, "#nextcritical", 1);

      spider.attack_once(closest);

      // hiddenattack suppresses POL's auto-reveal; force the spider visible
      // after the surprise blow so it can't keep attacking invisibly.
      spider.hidden := 0;

      SetCooldown(spider, "wall_climb", 30);
      return 1;
   endif

   return 0;
endfunction

function PoisonBite(spider, opponent)
   if (GetCooldown(spider, "poison_bite"))
      return 0;
   endif

   if (Distance(spider, opponent) > 1)
      return 0;
   endif

   var cache := NPC_GetCachedData(spider);  // O(1) lookup
   var tier := cache.tier;
   if (!tier) tier := 1; endif

   PrintTextAbove(spider, "*morde venenosamente*");
   
   if (!CheckResist(opponent, FORTITUDE, getNpcResist()))
      case (RandomInt(6)+1)
         1: TS_StartTimer(opponent, "defaultPoison", 60, tier, spider);
         2: TS_StartTimer(opponent, "strPoison", 60, tier, spider);
         3: TS_StartTimer(opponent, "dexPoison", 60, tier, spider);
         4: TS_StartTimer(opponent, "intPoison", 60, tier, spider);
         5: TS_StartTimer(opponent, "staminaPoison", 60, tier, spider);
         6: TS_StartTimer(opponent, "manaPoison", 60, tier, spider);
      endcase
   endif

   SetCooldown(spider, "poison_bite", 20);
   return 1;
endfunction

function LayEggSac(spider)
   if (GetCooldown(spider, "egg_sac"))
      return 0;
   endif

//    // Only lay eggs if in combat and health above 50%
//    if (!spider.opponent || AP_GetVital(spider, "Hits") < AP_GetVitalMaximumValue(spider, "Hits")/2)
//       return 0;
//    endif

   var egg := CreateItemAtLocation(spider.x, spider.y, spider.z, "SpiderEggSac", 1, spider.realm);
   if (egg)
      PrintTextAbove(spider, "*deposita ovos*");
      SetObjProperty(egg, "ParentTier", NPC_GetCachedData(spider).tier);
      SetCooldown(spider, "egg_sac", 600);

      egg.decayAt := ReadGameClock() + (60 * 15);
      return 1;
   endif

   return 0;
endfunction

function CreateInvisibleWeb(spider)
    if (GetCooldown(spider, "create_web"))
        return 0;
    endif

    var web := CreateItemAtLocation(spider.x, spider.y, spider.z, "SpiderWebTrap", 1, spider.realm);
    if (web)
        PrintTextAbove(spider, "*tece teia*");
        SetObjProperty(web, "Creator", spider.serial);
        SetCooldown(spider, "create_web", 600);
        
        // Random static web graphic
        var web_graphics := array{4280, 4281, 4282, 4283, 4284, 4285, 4286, 4287, 4288, 
                                4289, 4290, 4291, 4292, 4293, 4294, 4295, 4296, 4297, 
                                4298, 4299, 4300, 4301, 4302, 4303, 4304};
        
        web.graphic := web_graphics[RandomInt(web_graphics.size())+1];
        web.invisible := 1;
        web.decayAt := ReadGameClock() + 120;
        SetObjProperty(web, "level", NPC_GetCachedData(spider).tier);
        return 1;
    endif

    return 0;
endfunction

function charmOpponent(attacker, defender)
    if (!CheckResist(defender, WILL, RESIST_VERY_HARD))
        PrintTextAbove(attacker, "*Hipnotiza*");
        PlayStationaryEffectEx(defender.x, defender.y, defender.z+10, defender.realm, 0x6e0d, 5, 45, 2, 2740);

        SetObjProperty(defender, "charmed_by", attacker.serial);
        sleep(15);
        EraseObjProperty(defender, "charmed_by");
        SendSysMessageEx(defender, "Você foi hipnotizado por " + attacker.name, SSM_INFO);
    endif
endfunction

// Function to frighten opponents if they fail VONTADE check
function frightenOpponent(attacker, defender)
    if (!CheckResist(defender, WILL, RESIST_HARD))
        PlayStationaryEffectEx(defender.x, defender.y, defender.z+10, defender.realm, 0x6e0d, 5, 45, 2, 2740);
        var skill := Cint(AP_GetSkill(attacker, WRESTLING));

        TS_StartTimer(defender, "dmgreduction", 15, skill, attacker, 1);

        SendSysMessageEx(defender, "Você foi aterrorizado por " + attacker.name, SSM_INFO);
    endif
endfunction

function petrify(attacker, defender)
    if (GetCooldown(me, "petrifiy") > 0)
        return 0;
    endif
    var duration := 18;
	// Check if already petrified
	if (GetObjProperty(defender, "#petrified"))
		return 0;
	endif

	// Check resistance
	if (CheckResist(defender, WILL, RESIST_HARD))
		PrintTextAbove(defender, "*resiste à petrificação*");
		return 0;
	endif

	// Apply petrification effect for 3 minutes (180 seconds)
	PrintTextAbove(attacker, "*petrifica " + defender.name + "*");
	TS_StartTimer(defender, "petrify", duration);
    SetObjProperty(defender, "noattack", 1);
    Setcooldown(defender, "silence", duration);

    SetCooldown(me, "petrify", 30);

	return 1;
endfunction

function _wetPowderLocal(who)
    SetObjProperty(who, "molhado", 1);
    SendSysMessageEx(who, "Você está molhado!", SSM_INFO);
    var fire_weapons := Array{0xC5FD, 0xC5FE, 0xC5FF, 0xC600, 0x89A7};

    var itemsinbag := EnumerateItemsInContainer(who.backpack);
    foreach item in itemsinbag
        var container := item.container;
        if ((item.objtype == 0x9092 && container.objtype != 0xDE31))
            SendSysMessageEx(who, "A chuva danificou parte de sua pólvora!", SSM_FAIL);
            SubtractAmount(item, RandomInt(5) + 1);
        endif
        if (item.objtype in fire_weapons)
            SetObjProperty(item, "balas", 0);
            SendSysMessageEx(who, "A pólvora da sua arma molhou!", SSM_FAIL);
        endif
    endforeach

    var items := ListItemsNearLocation(who.x, who.y, who.z, 20, who.realm);
    foreach item in items
        if (item.objtype == 0x9092)
            PrintTextAbove(item, "*Molhada*");
            SubtractAmount(item, RandomInt(5) + 1);
        endif
    endforeach
endfunction

function _isAcidImuneLocal(mobile)
    if (Cint(GetObjProperty(mobile, "AcidImune")))
        return 1;
    endif
    if (mobile.npctemplate)
        var npc_info := NPC_GetNPCConfig(mobile.npctemplate);
        if (Cint(npc_info.AcidImune))
            return 1;
        endif
    endif
    return 0;
endfunction

function _getAcidDiceFromTemplateLocal(npc)
    if (npc.isA(POLCLASS_NPC))
        var npc_info := NPC_GetNPCConfig(npc.npctemplate);
        if (!npc_info.AcidDamage)
            return 0;
        else
            return npc_info.AcidDamage;
        endif
    else
        return 0;
    endif
endfunction

function _getAcidDiceFromItemLocal(acid_pool)
    if (acid_pool.isA(POLCLASS_ITEM))
        if (TypeOf(GetObjProperty(acid_pool, "AcidDice")) == "String")
            return GetObjProperty(acid_pool, "AcidDice");
        else
            return 0;
        endif
    else
        return 0;
    endif
endfunction

function _dealAcidDamageLocal(npc, mobile, acid_pool, acid_damage := 0)
    if (_isAcidImuneLocal(mobile))
        return 0;
    endif
    var acid_dice := _getAcidDiceFromTemplateLocal(npc);
    if (TypeOf(acid_dice) != "String")
        acid_dice := _getAcidDiceFromItemLocal(acid_pool);
    endif
    if (acid_damage)
        acid_damage := RandomDiceRoll(acid_damage);
    elseif (TypeOf(acid_dice) == "String")
        acid_damage := RandomDiceRoll(acid_dice);
    else
        return 0;
    endif
    DamageFLS(mobile, acid_damage, DMG_POISON, npc);
    var equipped_items := ListEquippedItems(mobile);
    if (RandomInt(10) > 5)
        var acid_burned := equipped_items[RandomInt(equipped_items.size())+1];
        if (acid_burned.isA(POLCLASS_WEAPON) || acid_burned.isA(POLCLASS_ARMOR))
            var armor_damage := RandomInt(4) + 1;
            if (acid_burned.hp > armor_damage)
                acid_burned.hp := acid_burned.hp - armor_damage;
                PrintTextPrivate(mobile, "*Acido danificou seu equipamento*", mobile, _DEFAULT_TEXT_FONT, 72);
            else
                DestroyItem(acid_burned);
            endif
        endif
    endif
    return acid_dice;
endfunction
