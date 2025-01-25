use uo;
use os;
use util;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include "include/sounds";
include ":attributes:attributes";
include ":quedadebraco:constants";

// Constantes do sistema
var GAME_RADIUS := 2;


// Constantes para críticos
var CRIT_FAIL_MIN := 1;
var CRIT_FAIL_MAX := 10;
var CRIT_SUCCESS_MIN := 990;
var CRIT_SUCCESS_MAX := 1000;

// Constantes para cadeiras
var CHAIR_NS_1 := 0x0B2C; // Cadeira norte/sul 1
var CHAIR_NS_2 := 0x0B2D; // Cadeira norte/sul 2
var CHAIR_EW_1 := 0x0B2E; // Cadeira leste/oeste 1
var CHAIR_EW_2 := 0x0B2F; // Cadeira leste/oeste 2



// Estrutura do jogo (redefinida para ser criada dinamicamente)
function CreateGameStruct(challenger, opponent)
    var game := struct;
    game.+challenger := challenger;
    game.+opponent := opponent; // antes era target
    game.+rounds_won_challenger := 0;
    game.+rounds_won_opponent := 0; // antes era target
    game.+current_round := 1;
    game.+stage := STATE_WAITING;
    
    return game;
endfunction

function IsPlayerSeated(mobile)
    if (!mobile)
        return 0;
    endif
    
    if (GetObjProperty(mobile, "#Sitting") != 1)
        return 0;
    endif
    
    var chair_types := array{CHAIR_NS_1, CHAIR_NS_2, CHAIR_EW_1, CHAIR_EW_2};
    foreach item in ListItemsAtLocation(mobile.x, mobile.y, mobile.z)
        if (item.graphic in chair_types)
            return 1;
        endif
    endforeach
    
    return 0;
endfunction

function CheckDistance(who, targ, range)
    if (!who || !targ)
        return 0;
    endif
    
    return (who.x >= (targ.x - range) && who.x <= (targ.x + range) &&
            who.y >= (targ.y - range) && who.y <= (targ.y + range));
endfunction



function UnregisterGame(challenger, opponent)
    // Remove quaisquer props temporárias relacionadas ao jogo
    EraseObjProperty(challenger, "#InArmWrestling");
    EraseObjProperty(opponent, "#InArmWrestling");
    return 1;
endfunction