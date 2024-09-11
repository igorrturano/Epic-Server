use npc;
include "include/sysEvent";
include "include/sounds";
include "include/damage";
include "include/facings";

var me := Self();

function MonsterBrain()
    var ev;
    var wanders := 0;
    var last_seen_opponent := struct{"x" := 0, "y" := 0, "z" := 0, "serial" := 0};
    // EnableMainEvents();
    Set_Event_Queue_Size(50);

    while (me)
        ev := os::wait_for_event(200);
        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED:
                SYSEVENT_DAMAGED:
                    wanders := 0;
                    // Fight(ev.source);
                    last_seen_opponent := struct{"x" := ev.source.x, "y" := ev.source.y, "z" := ev.source.z, "serial" := ev.source.serial};
                SYSEVENT_ENTEREDAREA:
                    if (CanFight(me, ev.source))
                        wanders := 0;
                        // Fight(ev.source);
                        last_seen_opponent := struct{"x" := ev.source.x, "y" := ev.source.y, "z" := ev.source.z, "serial" := ev.source.serial};
                    endif
                EVID_ALERT_ALLIES:
                    wanders := 0;
                    // Fight(ev.opponent, 1);
                    last_seen_opponent := struct{"x" := ev.opponent.x, "y" := ev.opponent.y, "z" := ev.opponent.z, "serial" := ev.opponent.serial};
                EVID_HERDING:
                    // MonsterHerd(ev);
                EVID_TAUNT:
                    if (ev.source)
                        wanders := 0;
                        // Fight(ev.source, 1);
                        last_seen_opponent := struct{"x" := ev.source.x, "y" := ev.source.y, "z" := ev.source.z, "serial" := ev.source.serial};
                    endif
                EVID_FLEE:
                    if (ev.source)
                        // MonsterFlee(ev.source);
                    endif
            endcase
        endif

        MonsterLookAround(last_seen_opponent);
        MonsterWander(wanders, last_seen_opponent);
        wanders += 1;

        if (wanders >= 30)
            wanders := 0;
            // ev := MonsterSleepMode();
        endif
    endwhile
endfunction

function MonsterLookAround(byref last_seen_opponent)
    foreach mobile in ListMobilesNearLocationEx(me.x, me.y, me.z, 8, LISTEX_FLAG_NORMAL|LISTEX_FLAG_HIDDEN)
        if (CanFight(me, mobile))
            // Fight(mobile);
            last_seen_opponent := struct{"x" := mobile.x, "y" := mobile.y, "z" := mobile.z, "serial" := mobile.serial};
            return;
        endif
        sleepms(2);
    endforeach
    
    // PlayIdleSound();
endfunction

function MonsterWander(byref wanders, byref last_seen_opponent)
    if (wanders > 10)
        var heal_amount := CInt(CDbl(AP_GetVitalMaximumValue(me, HITS)) * 0.01);
        HealDamage(me, heal_amount);
    endif

    var anchor := GetObjProperty(me, "anchor");
    var range := GetObjProperty(me, "range");

    if (!range)
        range := 6;
    endif

    if (last_seen_opponent.serial != 0)
        var opponent := SystemFindObjectBySerial(last_seen_opponent.serial);
        if (!opponent)
            if (CoordinateDistance(me.x, me.y, last_seen_opponent.x, last_seen_opponent.y) > 1)
                walkTowardLocation(last_seen_opponent.x, last_seen_opponent.y);
            else
                last_seen_opponent.serial := 0;
            endif
        else
            last_seen_opponent := struct{"x" := opponent.x, "y" := opponent.y, "z" := opponent.z, "serial" := opponent.serial};
        endif
    endif
endfunction

function CanFight(attacker, defender)
    if (defender.dead || defender.hidden || defender.concealed)
        return 0;
    elseif (defender.npctemplate == attacker.npctemplate)
        return 0;
    endif
    
    return 1;
endfunction

function CloseDistance(opponent)
    var dist := CoordinateDistance(me.x, me.y, opponent.x, opponent.y);
    if (dist > 1)
        RunToward(opponent);
        return 0;
    else
        return 1;
    endif
endfunction
