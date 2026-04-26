// ============================================================================
// threatModel.inc - damage-weighted aggro for NPC brains
// ============================================================================
// Accumulates threat per attacker. Damage events (routed via AttackLogger /
// LogLastHit / EVID_THREAT_UPDATE) are the primary input. Taunt is an explicit
// override that injects a bias above the current max so the taunter wins
// selection until they are out-damaged or their score decays.
//
// EV_ENGAGED grants zero threat by design — exploit-proof against double-click
// spam because target selection is data-driven, not event-driven.
//
// Hysteresis (THREAT_SWITCH_MARGIN) replaces the legacy 20s target_lock_var
// and 90% keep_target_chance: a candidate must beat the current opponent's
// score by 25% to take over, which prevents tick-flipping between attackers
// dealing similar damage.
// ============================================================================

use uo;
use os;

const THREAT_DECAY_FACTOR     := 0.93;   // multiplied per decay tick (~10s half-life at 1Hz)
const THREAT_SWITCH_MARGIN    := 1.25;   // new candidate must exceed current * 1.25
const THREAT_TAUNT_BIAS       := 5000;   // added on top of current max for taunters
const THREAT_MIN_KEEP         := 5;      // entries below this are evicted after decay
const THREAT_DECAY_INTERVAL   := 1;      // seconds between decay passes

// Script-level state. Reset on brain dormancy (see brain.src).
var threat_table_var := dictionary;
var last_threat_decay_var := 0;


function ClearThreatTable()
    threat_table_var := dictionary;
    last_threat_decay_var := 0;
endfunction


// Normalize damage to per-mille of self max HP, so the same constants tune
// equally for chickens and dragons.
function _ThreatNormalize(amount)
    var max_hp := AP_GetVitalMaximumValue(me, HITS);
    if (!max_hp || max_hp < 1)
        max_hp := 1;
    endif
    return CDbl(amount) * 1000.0 / CDbl(max_hp);
endfunction


// Add (or register) a threat source. amount=0 just registers the candidate
// without granting threat — used by SYSEVENT_ENGAGED so engagement spam
// cannot manipulate target selection.
function AddThreat(source, amount)
    if (!source || source == me)
        return;
    endif
    var key := source.serial;
    var entry;
    if (threat_table_var.exists(key))
        entry := threat_table_var[key];
    else
        entry := struct{ "score" := 0.0, "last_hit" := ReadGameClock() };
    endif
    if (amount > 0)
        entry.score := CDbl(entry.score) + _ThreatNormalize(amount);
        entry.last_hit := ReadGameClock();
    endif
    threat_table_var[key] := entry;
endfunction


function _MaxThreatScore()
    var max_score := 0.0;
    foreach key in (threat_table_var.keys())
        var entry := threat_table_var[key];
        var s := CDbl(entry.score);
        if (s > max_score)
            max_score := s;
        endif
    endforeach
    return max_score;
endfunction


// Force taunter above current max threat. Multiple taunts stack: each call
// re-reads the current max, so the latest taunter always wins.
function InjectTauntThreat(taunter)
    if (!taunter)
        return;
    endif
    threat_table_var[taunter.serial] := struct{
        "score" := _MaxThreatScore() + THREAT_TAUNT_BIAS,
        "last_hit" := ReadGameClock()
    };
endfunction


// Time-throttled decay. Safe to call every brain tick.
function DecayThreat()
    var now := ReadGameClock();
    if (last_threat_decay_var && (now - last_threat_decay_var) < THREAT_DECAY_INTERVAL)
        return;
    endif
    last_threat_decay_var := now;

    var dead_keys := array{};
    foreach key in (threat_table_var.keys())
        var entry := threat_table_var[key];
        entry.score := CDbl(entry.score) * THREAT_DECAY_FACTOR;
        if (entry.score < THREAT_MIN_KEEP)
            dead_keys.append(key);
        else
            threat_table_var[key] := entry;
        endif
    endforeach
    foreach k in dead_keys
        threat_table_var.erase(k);
    endforeach
endfunction


// Highest-scoring entry that still passes CanFight. Returns 0 if table empty
// or every candidate is invalid (dead, hidden, out-of-LOS for new targets).
function GetTopThreat()
    var best := 0;
    var best_score := -1.0;
    foreach key in (threat_table_var.keys())
        var entry := threat_table_var[key];
        var s := CDbl(entry.score);
        if (s <= best_score)
            continue;
        endif
        var mob := SystemFindObjectBySerial(key);
        if (!mob || !CanFight(me, mob))
            continue;
        endif
        best := mob;
        best_score := s;
    endforeach
    return best;
endfunction


function _GetThreatScore(serial)
    if (!threat_table_var.exists(serial))
        return 0.0;
    endif
    return CDbl(threat_table_var[serial].score);
endfunction


// Hysteresis-gated reevaluation. Called per tick from each brain's combat
// block; only triggers a SetOpponent swap when there's a clear winner.
function ShouldReevaluateTarget()
    if (!me.opponent)
        return 1;
    endif
    if (!CanFight(me, me.opponent))
        return 1;
    endif
    var top := GetTopThreat();
    if (!top || top == me.opponent)
        return 0;
    endif
    var current_score := _GetThreatScore(me.opponent.serial);
    var top_score := _GetThreatScore(top.serial);
    if (top_score > current_score * THREAT_SWITCH_MARGIN)
        return 1;
    endif
    return 0;
endfunction
