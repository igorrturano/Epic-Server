const CHARGE_MINIMUM_DISTANCE := 5;
include ":timedScripts:timedScripts";
include ":awareness:awareness";

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
   PrintText(attacker, "*inicia carga*");
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
   var charge_damage := Cint(dist * 2);

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
                    PrintText(attacker, "*libera esporos hipnóticos*");
                    PlayStationaryEffectEx(mobile.x, mobile.y, mobile.z+10, mobile.realm, 0x6e0d, 5, 45, 2, 2740);
                    
                    SetObjProperty(mobile, "charmed_by", attacker.serial);
                    sleep(15);
                    EraseObjProperty(mobile, "charmed_by");
                    
                    SendSysMessageEx(mobile, "Você foi hipnotizado pelo mirconídeo.", SSM_INFO);
                endif
            
            2: // Poison
                if (!CheckResist(mobile, FORTITUDE, RESIST_HARD))
                    PrintText(attacker, "*libera esporos venenosos*");
                    var sounds := { 560, 561, 562 };
                    PlaysoundEffect(mobile, sounds[RandomInt(sounds.size())+1]);
                    PlayStationaryEffectEx(mobile.x, mobile.y, mobile.z, mobile.realm, 0x54f7, 10, 30, 2283);
                    TS_StartTimer(mobile, "defaultPoison", 120, RandomInt(4)+1);
                endif
            
            3: // Blind
                if (!CheckResist(mobile, REFLEXOS, RESIST_MEDIUM))
                    PrintText(attacker, "*libera esporos cegantes*");
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
        if (Distance(me, opponent) > 16)
            break;
        endif
    endwhile

    // Try to steal if close enough
    if (Distance(me, opponent) <= 1)
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

