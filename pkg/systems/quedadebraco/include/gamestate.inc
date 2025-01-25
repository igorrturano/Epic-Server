use uo;
use os;
use util;

include ":quedadebraco:quedadebraco"; // Agora vamos incluir para pegar os estados de lá

// Dicionário global para jogos ativos
var active_games := dictionary;

function RegisterGame(game)
    var game_id := _CreateGameID(game.challenger, game.opponent);
    active_games[game_id] := game;
    return game_id;
endfunction

function GetActiveGame(challenger, opponent)
    var game_id := _CreateGameID(challenger, opponent);
    return active_games[game_id];
endfunction

function RemoveActiveGame(challenger, opponent)
    var game_id := _CreateGameID(challenger, opponent);
    active_games.Erase(game_id);
endfunction

function IsPlayerInGame(who)
    if (!who)
        return 0;
    endif
    
    foreach game_id in (active_games.keys())
        var game := active_games[game_id];
        if (game.challenger == who || game.opponent == who)
            return 1;
        endif
        sleepms(2);
    endforeach
    return 0;
endfunction

function _CreateGameID(challenger, opponent)
    if (!challenger || !opponent)
        return "";
    endif
    return CStr(challenger.serial) + "_" + CStr(opponent.serial);
endfunction

function SetGameState(game, new_state)
    if (!game)
        return 0;
    endif
    
    game.stage := new_state;
    var game_id := _CreateGameID(game.challenger, game.opponent);
    active_games[game_id] := game;
    return 1;
endfunction 