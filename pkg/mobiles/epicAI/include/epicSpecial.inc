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
                if (!CheckResist(mobile, VONTADE, RESIST_MEDIUM))
                    PrintText(attacker, "*libera esporos hipnóticos*");
                    PlayStationaryEffectEx(mobile.x, mobile.y, mobile.z+10, mobile.realm, 0x6e0d, 5, 15, 2, 2740);
                    
                    SetObjProperty(mobile, "charmed_by", attacker.serial);
                    SetCooldown(mobile, "charmed", 15);
                    
                    SendSysMessageEx(mobile, "Você foi hipnotizado pelo homem-cogumelo.", SSM_INFO);
                endif
            
            2: // Poison
                if (!CheckResist(mobile, FORTITUDE, RESIST_MEDIUM))
                    PrintText(attacker, "*libera esporos venenosos*");
                    var sounds := { 560, 561, 562 };
                    PlaysoundEffect(mobile, sounds[RandomInt(sounds.size())+1]);
                    PlayStationaryEffectEx(mobile.x, mobile.y, mobile.z, mobile.realm, 0x54f7, 10, 30, 2283);
                    TS_StartTimer(mobile, "defaultPoison", 120, RandomInt(4)+1);
                endif
            
            3: // Blind
                if (!CheckResist(mobile, REFLEXOS, RESIST_EASY))
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