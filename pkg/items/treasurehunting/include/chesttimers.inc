use uo;
use os;

include ":timedscripts:timedScripts";

const CHEST_DEFAULT_DURATION := 120; // 10 minutos

function StartChestDestructionTimer(chest, duration := CHEST_DEFAULT_DURATION)
    if (!chest)

        return 0;
    endif
    

    
    var current_time := ReadGameClock();
    SetObjProperty(chest, "CreatedOn", current_time);
    SetObjProperty(chest, "DestroyAt", current_time + duration);

    
    var script := start_script(":treasurehunting:chest/destroy", chest);
    if (!script)

        return 0;
    endif

    return 1;
endfunction

/*
 * IsChestExpired(chest)
 * Verifica se o baú já deveria ter sido destruído
 */
function IsChestExpired(chest)
    if (!chest)
        return 1;
    endif
    
    var destroy_at := GetObjProperty(chest, "DestroyAt");
    if (!destroy_at)
        return 1;
    endif
    
    return (ReadGameClock() >= destroy_at);
endfunction

/*
 * GetChestTimeRemaining(chest)
 * Retorna quanto tempo resta até a destruição do baú
 */
function GetChestTimeRemaining(chest)
    if (!chest)
        return 0;
    endif
    
    var destroy_at := GetObjProperty(chest, "DestroyAt");
    var created_on := GetObjProperty(chest, "CreatedOn");
    var current_time := ReadGameClock();
    
    if (!destroy_at)
        return 0;
    endif
    
    var time_remaining := destroy_at - current_time;

    
    return (time_remaining > 0) ? time_remaining : 0;
endfunction