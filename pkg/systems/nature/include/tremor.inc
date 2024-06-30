use uo;
use util;
use vitals;

include ":charactercreation:resists";
include ":timedScripts:timedScripts";
include "include/damage";

function EfeitosTerremoto(who,danobase,danoale,dif)
     if (who.dead)
        return;
     endif

     if(who.cmdlevel > 0)
         if(!GetObjProperty (who, "lastpossessedserial"))
             return;
         endif
     elseif (GetObjProperty(who, "chardata").povo == "Numajene")
        return;
     endif

     PlaySoundEffectPrivate(who, 0x222, who);
     PrintTextAbove(who, "*Raizes se movem ao redor*");

     sleep(5);

     if (CheckResist(who,  REFLEXOS, dif))//note: colocar uma variavel de chance de resistir ao terremoto
        PrintTextAbove(who, "*Evitou as raizes*");
        PerformAction(who, 20);
     else
        PrintTextAbove(who, "*Preso pelas raizes*");
         TS_StartTimer(who, "paralysis", 5);
         PerformAction(who, 21);
         DamageFLS(who, danobase + (RandomInt(danoale)));
         var vine := CreateItemAtLocation(who.x, who.y, who.z, 0x0CF2);
         SetObjProperty(vine,"movable", 0);
         sleep(5);
         DestroyItem(vine);
     endif
endfunction

