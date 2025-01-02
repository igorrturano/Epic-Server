const CHARGE_MINIMUM_DISTANCE := 5;
include ":timedScripts:timedScripts";
include ":awareness:awareness";
include ":nature:nature";
include ":acid:acid";
include "include/epicNPCUtils";

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
   ProcessChargeImpact(attacker, defender);

   // Grant speed boost during charge
   SetCooldown(attacker, "charge", 120);
   PrintTextAbove(attacker, "*inicia carga*");
   TS_StartTimer(attacker, "velocidade", 5);

   return 1;
endfunction

function ProcessChargeImpact(attacker, defender)
   var charge_start := GetObjProperty(attacker, "#chargestart");
   if (!charge_start)
      return 0;
   endif

   // Only apply charge damage to intended target
   if (charge_start.target != defender.serial)
      return 0;
   endif

   // Calculate distance charged
   var dist := CoordinateDistance(charge_start.x, charge_start.y, defender.x, defender.y);
   var charge_damage := Cint(dist);

   EraseObjProperty(attacker, "#chargestart");
   SetObjProperty(attacker, "charge_damage", charge_damage);
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
                if (!CheckResist(mobile, VONTADE, RESIST_VERY_HARD))
                    PrintTextAbove(attacker, "*libera esporos hipnóticos*");
                    PlayStationaryEffectEx(mobile.x, mobile.y, mobile.z+10, mobile.realm, 0x6e0d, 5, 45, 2, 2740);
                    
                    SetObjProperty(mobile, "charmed_by", attacker.serial);
                    sleep(15);
                    EraseObjProperty(mobile, "charmed_by");
                    
                    SendSysMessageEx(mobile, "Você foi hipnotizado pelo mirconídeo.", SSM_INFO);
                endif
            
            2: // Poison
                if (!CheckResist(mobile, FORTITUDE, RESIST_HARD))
                    PrintTextAbove(attacker, "*libera esporos venenosos*");
                    var sounds := { 560, 561, 562 };
                    PlaysoundEffect(mobile, sounds[RandomInt(sounds.size())+1]);
                    PlayStationaryEffectEx(mobile.x, mobile.y, mobile.z, mobile.realm, 0x54f7, 10, 30, 2283);
                    TS_StartTimer(mobile, "defaultPoison", 120, RandomInt(4)+1);
                endif
            
            3: // Blind
                if (!CheckResist(mobile, REFLEXOS, RESIST_MEDIUM))
                    PrintTextAbove(attacker, "*libera esporos cegantes*");
                    PlayStationaryEffectEx(mobile.x, mobile.y, mobile.z+10, mobile.realm, 0x6e0d, 5, 15, 2, 2740);
                    SetCooldown(mobile, "blinded", 15);
                    PrintTextAbove(mobile, "*cegueira*");
                    
                    foreach mob in ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 16)
                        DeleteObject(mobile, mob);
                        sleepms(20);
                    endforeach
                    
                    sleep(15);
                    foreach mob in ListMobilesNearLocation(mobile.x, mobile.y, mobile.z, 16)
                        DrawObject(mobile, mob, mob.graphic, mob.color);
                        sleepms(20);
                    endforeach
                endif
        endcase

        sleepms(2);
    endforeach

    return 1;
endfunction

function DoThiefSteal(opponent)
    if (!opponent.backpack)
        return 0;
    endif

    // First try to hide
    GrantPrivilege(me, "hiddenattack");
    me.enable("hiddenattack");
    me.hidden := 1;
    me.stealthsteps := 30;

    // Move close to target
    while (Distance(me, opponent) > 1)
        WalkToward(opponent);
        sleepms(100);
        if (Distance(me, opponent) > AREA_SIZE)
            break;
        endif
    endwhile

    // Try to steal if close enough
    if (Distance(me, opponent) <= 1)
        var diff := 15 + CInt(AP_GetSkill(opponent, AWARENESS));
        var check := SkillCheck(me, PICKPOCKETING, diff);

        if (check > 0)
            // Check for elemental weapon
            if (TemHabilidade(opponent, "Vínculo Elemental"))
                if (GetObjProperty(opponent.weapon, "#elementalweapon"))
                    SendSysMessageEx(who, "O alvo tem uma ligacao magica com a arma.", SSM_FAIL);
                    return 1;
                endif
            endif

            // Attempt steal
            var items := EnumerateItemsInContainer(opponent.backpack);
            if (items.size() > 0)
                var item := items[RandomInt(items.size())+1];
                if (item.movable)
                    if (!me.backpack)
                        var backpack := CreateItemAtLocation(me.x, me.y, me.z, "Backpack", 1);
                        EquipItem(me, backpack);
                    endif
                    MoveItemToContainer(item, me.backpack);
                    PrintTextAbove(me, "*furtou " + item.desc + "*");
                endif
            endif
        else
            CriticalFailure(me, opponent);
        endif
    endif
    
    if (me.hidden == 1)
        me.hidden := 0;
    endif
endfunction

function DoThiefSneakAttack(opponent)   
    // Grant stealth privileges
    GrantPrivilege(me, "hiddenattack");
    me.enable("hiddenattack");
    GrantPrivilege(me, "runwhilestealth");
    me.enable("runwhilestealth");

    me.hidden := 1;
    me.stealthsteps := 100;

    // Try to get behind the target without breaking stealth
    var attempts := 0;
    while (!IsBehind(me, opponent.x, opponent.y) && attempts < 10)
        attempts += 1;
        
        // Calculate position behind target
        var behind_x := opponent.x + (opponent.x - me.x);
        var behind_y := opponent.y + (opponent.y - me.y);
        
        WalkTowardLocation(behind_x, behind_y);
        sleepms(100);

        // Break if we lost stealth or got too far
        if (!me.hidden || Distance(me, opponent) > 16)
            break;
        endif
    endwhile

    // Only attack if still hidden and in position
    if (me.hidden && IsBehind(me, opponent.x, opponent.y))
        PrintTextAbove(me, "*ataque furtivo*");
        SetObjProperty(me, "backstab", 1);
        SetObjProperty(me.weapon, "backstab", 1);
        me.attack_once(opponent);
    else
        // If stealth attack failed, engage in normal combat
        HandleCombatEvent(opponent, 0);
    endif
endfunction

function PerformHeal(shaman, targ, tier)
    if (!targ)
        return 0;
    endif

    PrintTextAbove(shaman, "*curando " + targ.name + "*");
    PerformAction(shaman, ANIM_CAST_AREA);
    PlaySoundEffect(shaman, SFX_SPELL_HEAL);
    PlayObjectCenteredEffect(targ, FX_HEAL_EFFECT, 7, 0x10);
    
    var heal_amount := tier * 12;
    HealDamage(targ, heal_amount);
    
    return 1;
endfunction

function PerformHealingRain(shaman, allies)
    var mlore := CInt(AP_GetSkill(shaman, MAGICLORE));
    
    PrintTextAbove(shaman, "*invoca tempestade*");
    PerformAction(shaman, ANIM_CAST_AREA);
    
    // Heal allies
    foreach ally in allies
        if (!ally.isUnderRoof())
            HealDamage(ally, CInt(mlore/3));
            PlayObjectCenteredEffect(ally, FX_HEAL_EFFECT, 7, 0x10);
        endif
        sleepms(2);
    endforeach
    
    // Damage enemies
    foreach mobile in ListMobilesNearLocation(shaman.x, shaman.y, shaman.z, 12)
        if (!mobile.isUnderRoof() && CanFight(shaman, mobile))
            PlayLightningBoltEffect(mobile);
            DamageFLS(mobile, RandomDiceRoll("1d10") + 20, DMG_ENERGY, shaman);
            PrintTextAbove(mobile, "*atingido por um raio*");
            SetWeatherForPlayer(mobile, 1, 50);
            wetPowder(mobile);
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
   PerformAction(shaman, ANIM_CAST_AREA);
   
   if (!CheckResist(targ, VONTADE, diff))
      var duration := CInt(AP_GetSkill(shaman, MAGICLORE) / 10);
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
   PerformAction(shaman, ANIM_CAST_AREA);
   
   var bonus := CInt(AP_GetSkill(shaman, MAGICLORE) / 20); // Small bonus based on skill
   
   if (!CheckResist(targ, VONTADE, diff, -bonus))
      var duration := CInt(AP_GetSkill(shaman, MAGICLORE));
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
   DealAcidDamage(me, targ, acid_pool, acid_damage);
   
   return 1;
endfunction

function HiveMind(me)
    // Get all nearby me allies
    var allies := ListMobilesNearLocationEx(me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL);
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
        var bonus := ally * 2;
        SetObjProperty(me, "#attackbonus", bonus);
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
    var allies := ListMobilesNearLocationEx(me.x, me.y, me.z, 4, LISTEX_FLAG_NORMAL);
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
        
        // Set all allies to attack the targ
        ally.opponent := targ;
        
        sleepms(100);
    endforeach
    
    return 1;
endfunction
