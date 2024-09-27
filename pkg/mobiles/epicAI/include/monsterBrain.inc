use npc;
include "include/sysEvent";
include "include/sounds";
include ":combat:damage";
include "include/facings";

var me := Self();
const AREA_SIZE := 16;
var last_seen_opponent := struct{"x" := 0, "y" := 0, "z" := 0, "serial" := 0};
var chasing := 0;

function MonsterBrain()
    print("MonsterBrain");
    var ev;
    var wanders := 0;
    EnableMainEvents();
    Set_Event_Queue_Size(50);

    while (me)
        ev := os::wait_for_event(200);
        print("ev: "+ev);
        print("ev.type: "+ev.type);
        if (ev)
            case (ev.type)
                SYSEVENT_ENGAGED:
                PrintTextAbove(me, "SYSEVENT_ENGAGED");
                UpdateLastSeenOpponent(ev.source);
                SYSEVENT_DAMAGED:
                    printtextabove(me, "SYSEVENT_DAMAGED");
                    wanders := 0;
                    // Fight(ev.source);
                    UpdateLastSeenOpponent(ev.source);
                SYSEVENT_ENTEREDAREA:
                    if (CanFight(me, ev.source))
                        wanders := 0;
                        PrintTextAbove(me, "SYSEVENT_ENTEREDAREA");
                        UpdateLastSeenOpponent(ev.source);
                    endif
                SYSEVENT_LEFTAREA:
                    // Saiu da Area Size de 16 tiles, inclusive aqui o mob pode andar até o telepad que o jogador se escondeu
                    PrintTextAbove(me, "SYSEVENT_LEFTAREA");
                    ChaseLastSeenOpponent();
                SYSEVENT_OPPONENT_MOVED:
                // Quando o oponente anda. Não sei se vamos precisar ainda
                    printtextabove(me, "SYSEVENT_OPPONENT_MOVED");
                    print("SYSEVENT_OPPONENT_MOVED");
                SYSEVENT_SPEECH:
                    // Provocar de voltar com ChatGPT
                    printtextabove(me, "SYSEVENT_SPEECH");
                    print("SYSEVENT_SPEECH");
                SYSEVENT_DISENGAGED:
                    // Aqui quando o player fica peace ou morre. O mob vai roubar ele.
                    printtextabove(me, "SYSEVENT_DISENGAGED");
                    print("SYSEVENT_DISENGAGED");
                EVID_ALERT_ALLIES:
                    wanders := 0;
                    printtextabove(me, "EVID_ALERT_ALLIES");
                    // Fight(ev.opponent, 1);
                    last_seen_opponent := struct{"x" := ev.opponent.x, "y" := ev.opponent.y, "z" := ev.opponent.z, "serial" := ev.opponent.serial};
                EVID_HERDING:
                    printtextabove(me, "EVID_HERDING");
                    // MonsterHerd(ev);
                EVID_TAUNT:
                    printtextabove(me, "EVID_TAUNT");
                    if (ev.source)
                        wanders := 0;
                        // Fight(ev.source, 1);
                        last_seen_opponent := struct{"x" := ev.source.x, "y" := ev.source.y, "z" := ev.source.z, "serial" := ev.source.serial};
                    endif
                EVID_FLEE:
                    printtextabove(me, "EVID_FLEE");
                    if (ev.source)
                        // MonsterFlee(ev.source);
                    endif
            endcase
        endif

        sleepms(100);

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

function EnableMainEvents()
  EnableEvents(SYSEVENT_SPEECH + SYSEVENT_LEFTAREA + SYSEVENT_DISENGAGED + SYSEVENT_OPPONENT_MOVED, AREA_SIZE);
  EnableEvents(SYSEVENT_ENGAGED + SYSEVENT_DAMAGED + SYSEVENT_ENTEREDAREA, AREA_SIZE);
endfunction

function UpdateLastSeenOpponent(opponent)
    last_seen_opponent.x := opponent.x;
    last_seen_opponent.y := opponent.y;
    last_seen_opponent.z := opponent.z;
    last_seen_opponent.serial := opponent.serial;
    chasing := 0;
endfunction

function ChaseLastSeenOpponent()
    var time := ReadGameClock() + 60;
    var dist := CoordinateDistance(me.x, me.y, last_seen_opponent.x, last_seen_opponent.y);
    var telepad := FindNearbyTelepad();
    var initial_x := me.x;
    var initial_y := me.y;
    var initial_z := me.z;
    
    if (telepad)
        last_seen_opponent.x := telepad.x;
        last_seen_opponent.y := telepad.y;
    endif
    
    while (dist > 1 && ReadGameClock() < time)
        RunTowardLocation(last_seen_opponent.x, last_seen_opponent.y);
        dist := CoordinateDistance(me.x, me.y, last_seen_opponent.x, last_seen_opponent.y);
        
        // Check if the NPC has crossed the telepad
        if (HasCrossedTelepad(initial_x, initial_y, initial_z))
            PrintTextAbove(me, "*reached point*");
            break;
        endif
        
        sleepms(10);
    endwhile
    
    // If we've reached this point and haven't crossed a telepad, we're at the destination
    if (dist <= 1)
        PrintTextAbove(me, "*reached point*");
    endif
    
    var result := SetOpponent(last_seen_opponent.serial);
    print("SetOpponent: " + result);
endfunction

function FindNearbyTelepad()
    var items := ListItemsNearLocation(me.x, me.y, me.z, AREA_SIZE, me.realm);
    foreach item in items
        if (item.graphic == 0x17E5)
            return item;
        endif
    endforeach
    return 0;
endfunction

function HasCrossedTelepad(initial_x, initial_y, initial_z)
    // Check if the NPC's position has changed significantly, indicating teleport
    if (CoordinateDistance(initial_x, initial_y, me.x, me.y) > 5 || Abs(me.z - initial_z) > 5)
        return 1;
    endif
    return 0;
endfunction