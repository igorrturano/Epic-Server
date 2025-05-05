use uo;
use os;
use util;

include ":timedscripts:timedScripts";
include ":attributes:attributes";

/*
 * ProcessTimedScripts(npc)
 * Purpose: Processes any TimedScripts configured for the NPC
 * Parameters: npc - The NPC to process TimedScripts for
 * Return: 1 if any scripts were processed, 0 if none
 */
function ProcessTimedScripts(npc)
    // Get the TimedScripts property
    var timedscripts := GetObjProperty(npc, "TimedScripts");
    if (!timedscripts || !timedscripts.size())
        return 0;
    endif
    
    // Check if we should run any TimedScripts
    var next_check := GetObjProperty(npc, "NextTimedScriptCheck");
    if (next_check && next_check > ReadGameClock())
        return 0; // Not time to check yet
    endif
    
    // Set next check time (random between 10-30 seconds)
    SetObjProperty(npc, "NextTimedScriptCheck", ReadGameClock() + RandomInt(10) + 20);
    
    var processed := 0;
    
    // Process each TimedScript
    foreach script in timedscripts
        // Random chance to use the script (about 10%)
        if (RandomInt(100) < 10)
            var targets := array{};
            
            // Determine targets based on the script's target property
            if (script.target == "Self")
                targets.append(npc);
            elseif (script.target == "Opponent")
                var opponent := npc.opponent;
                if (opponent)
                    targets.append(opponent);
                endif
            elseif (script.target == "All Opponents")
                targets := GetAllOpponents(npc);
            endif
            
            // Apply the TimedScript to each target
            foreach targ in targets
                if (targ && !targ.dead)
                    TS_StartTimer(targ, script.name, CInt(script.duration), CInt(script.level));
                    processed += 1;
                    
                    // Debug message
                    print("DEBUG: Applied TimedScript " + script.name + " to " + targ.name + 
                          " with duration " + script.duration + " and level " + script.level);
                endif
            endforeach
        endif
    endforeach
    
    return processed;
endfunction

/*
 * GetAllOpponents(npc)
 * Purpose: Gets all opponents (players) in combat with the NPC
 * Parameters: npc - The NPC to get opponents for
 * Return: Array of opponents
 */
function GetAllOpponents(npc)
    var opponents := array{};
    var range := 8; // Search range
    
    foreach mobile in ListMobilesNearLocation(npc.x, npc.y, npc.z, range, npc.realm)
        if (mobile.IsA(POLCLASS_MOBILE) && !mobile.dead && mobile.opponent == npc)
            opponents.append(mobile);
        endif
    endforeach
    
    return opponents;
endfunction 