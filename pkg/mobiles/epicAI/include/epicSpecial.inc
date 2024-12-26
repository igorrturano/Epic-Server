const CHARGE_MINIMUM_DISTANCE := 5;
include ":timedScripts:timedScripts";

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