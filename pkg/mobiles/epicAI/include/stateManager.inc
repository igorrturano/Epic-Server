/*
 * State Manager (HFSM Framework)
 *
 * Purpose: Provides non-blocking state management for NPC brains.
 * Instead of blocking while-loops (flee, ambush, herding), actions
 * become STATES that execute ONE step per brain tick.
 *
 * NPCs continue processing events (DAMAGED, ENGAGED, etc.) while
 * in any state - no more 2-30 second event blackouts.
 *
 * Usage:
 *   SM_TransitionTo("FLEEING", struct{ "target" := opponent, "steps" := 0 })
 *   // In brain loop:
 *   case (SM_GetState())
 *       "FLEEING": SM_TickFlee();
 */

var _sm_state := "IDLE";
var _sm_data := struct{};
var _sm_enter_time := 0;

function SM_GetState()
    return _sm_state;
endfunction

function SM_TransitionTo(new_state, data := struct{})
    _sm_state := new_state;
    _sm_data := data;
    _sm_enter_time := ReadGameClock();
endfunction

function SM_GetData()
    return _sm_data;
endfunction

function SM_ElapsedInState()
    return ReadGameClock() - _sm_enter_time;
endfunction

// Returns 1 when flee is complete (should transition out)
function SM_TickFlee()
    if (!_sm_data.target || _sm_data.target.dead)
        SM_TransitionTo("IDLE");
        return 1;
    endif

    if (Distance(me, _sm_data.target) >= 16)
        SM_TransitionTo("IDLE");
        return 1;
    endif

    if (_sm_data.steps >= 20)
        SM_TransitionTo("IDLE");
        return 1;
    endif

    TryToRunAway(_sm_data.target);
    _sm_data.steps += 1;

    // Anti-exploit: detect stuck flee using native lastcoord
    if (me.x == me.lastcoord.lastx && me.y == me.lastcoord.lasty)
        if (!_sm_data.stuck_count)
            _sm_data.stuck_count := 0;
        endif
        _sm_data.stuck_count += 1;

        if (_sm_data.stuck_count >= 3)
            // Try random direction first
            var dirs := array{0, 1, 2, 3, 4, 5, 6, 7};
            var dir := dirs[RandomInt(8) + 1];
            WalkTowardLocation(me.x + (dir % 3) - 1, me.y + (dir / 3) - 1);

            if (_sm_data.stuck_count >= 6)
                // Last resort: teleport to anchor
                var anchor := GetObjProperty(me, "anchor");
                if (anchor)
                    MoveObjectToLocation(me, anchor[1], anchor[2], anchor[3], me.realm, MOVEOBJECT_FORCELOCATION);
                endif
                SM_TransitionTo("IDLE");
                return 1;
            endif
        endif
    else
        _sm_data.stuck_count := 0;
    endif

    return 0;
endfunction

// Returns 1 when ambush is complete (should transition out)
function SM_TickAmbush()
    if (!_sm_data.target || _sm_data.target.dead)
        me.hidden := 0;
        SM_TransitionTo("IDLE");
        return 1;
    endif

    // Ambush broken (revealed by damage, spell, etc.)
    if (!me.hidden)
        SM_TransitionTo("COMBAT");
        return 1;
    endif

    // Timeout - give up waiting
    if (SM_ElapsedInState() >= 30)
        me.hidden := 0;
        SM_TransitionTo("IDLE");
        return 1;
    endif

    // Target in range - spring the ambush!
    if (Distance(me, _sm_data.target) <= 3)
        MoveObjectToLocation(me, _sm_data.target.x, _sm_data.target.y, _sm_data.target.z, _sm_data.target.realm, MOVEOBJECT_FORCELOCATION);
        me.hidden := 0;
        PrintTextAbove(me, "*emboscada*");
        SetObjProperty(me, "backstab", 1);
        SetObjProperty(me.weapon, "backstab", 1);
        me.attack_once(_sm_data.target);
        EraseObjProperty(me, "backstab");
        EraseObjProperty(me.weapon, "backstab");
        SM_TransitionTo("COMBAT");
        return 1;
    endif

    return 0;
endfunction

// Returns 1 when herding is complete
function SM_TickHerding()
    if (!_sm_data.target)
        SM_TransitionTo("IDLE");
        return 1;
    endif

    if (CoordinateDistance(me.x, me.y, _sm_data.target.x, _sm_data.target.y) <= 1)
        SM_TransitionTo("IDLE");
        return 1;
    endif

    RunTowardLocation(_sm_data.target.x, _sm_data.target.y);
    return 0;
endfunction

// Returns 1 when following is complete
function SM_TickFollowing()
    if (!_sm_data.target || _sm_data.target.dead)
        SM_TransitionTo("IDLE");
        return 1;
    endif

    // Check if enchantment expired
    var timers := TS_GetTimerNames(me);
    if (!("encantar" in timers))
        SM_TransitionTo("IDLE");
        return 1;
    endif

    if (Distance(me, _sm_data.target) > 2)
        RunToward(_sm_data.target);
    endif

    return 0;
endfunction
