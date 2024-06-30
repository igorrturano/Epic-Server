include ":attributes:attributes";
function process_combat_event(opponent)
    if ((AP_GetVital(me, HITS) * 100/AP_GetVitalMaximumValue(me, HITS)) < flee_point)
      EraseObjProperty(me,"#flees");
      flee(opponent);
    elseif (AP_GetVital(me, HITS) < AP_GetVitalMaximumValue(me, HITS)/2)
      ApplyHealing();
    endif
endfunction

function process_flee_event(opponent)
  opponent := opponent;
  return 0;
endfunction

function in_combat_event_loop(opponent, loops)
  if ( loops > 50 )
    flee(opponent);
    return;
  endif
endfunction

function post_combat()
  DisableCombatEvents();
  EnableMainEvents();
  SetWarMode( 0 );
  SetOpponent( 0 );
  sleep(1);
  look_around();
endfunction

function prepare_for_fight(opponent)
  opponent := opponent;
  DisableMainEvents();
  EnableCombatEvents();
endfunction

function ApplyHealing()
  var bandages := Cint(GetObjProperty(me, "bandages"));
  if(!bandages)
    var nextrestock := Cint(GetObjProperty(me, "nextrestock"));
    if (!nextrestock)
      SetObjProperty(me, "nextrestock", ReadGameClock() + 1800);
    endif
    if(nextrestock < ReadGameClock())
      bandages := 10 + RandomInt(10);
      SetObjProperty(me, "bandages", bandages);
      SetObjProperty(me, "nextrestock", ReadGameClock() + 1800);
    endif
  endif
  if((GetObjProperty(me,"#nextheal") < ReadGameClock()) || !GetObjProperty(me,"#nextheal"))
    bandages := Cint(GetObjProperty(me, "bandages"));
    if(bandages)
      AP_SetVital(me, HITS, AP_GetVital(me, HITS) + (RandomInt(AP_GetVitalMaximumValue(me, HITS) - AP_GetVital(me, HITS))+1));
      SetObjProperty(me,"#nextheal", ReadGameClock()+20);
      bandages := bandages - 1;
      if(bandages <= 0)
        bandages := 0;
      endif
      SetObjProperty(me, "bandages", bandages);
    endif
  endif
endfunction