use uo;
use os;
use util;
use datafile;

include ":attributes:stats";
// Constantes monetárias
const MOEDA_COBRE := 0xBA63;
const MOEDA_PRATA := 0xBA64;
const MOEDA_OURO := 0xBA65;

// Constantes de estado
const BET_STATUS_WAITING   := 0;  // Aceitando apostas
const BET_STATUS_LOCKED    := 1;  // Apostas encerradas
const BET_STATUS_RUNNING   := 2;  // Evento em andamento
const BET_STATUS_FINISHED  := 3;  // Evento finalizado
const BET_STATUS_CANCELLED := 4;  // Evento cancelado

// Funções de gerenciamento de dados
function GetBettingDataFile()
    var df := DFOpenDataFile(":betting:events", DF_CREATE);
    return df;
endfunction

function GenerateEventID()
    var df := GetBettingDataFile();
    var counter := CInt(df.GetProp("event_counter")) + 1;
    df.SetProp("event_counter", counter);
    return "EVT_" + counter;
endfunction

function RegisterEvent(event)
    var df := GetBettingDataFile();
    var elem := DFFindElement(df, event.eventID, DF_CREATE);
    elem.SetProp("event_data", event);
    return event.eventID;
endfunction

function GetEvent(event_id)
    var df := GetBettingDataFile();
    var elem := DFFindElement(df, event_id);
    if (elem)
        return elem.GetProp("event_data");
    endif
    return 0;
endfunction

function RegisterBet(bet)
    var df := GetBettingDataFile();
    var elem := DFFindElement(df, bet.eventID);
    if (!elem)
        return 0;
    endif
    
    var event := elem.GetProp("event_data");
    event.bets.append(bet);
    event.total_pool += bet.amount;
    
    elem.SetProp("event_data", event);
    return 1;
endfunction

function CloseEvent(event_id)
    var df := GetBettingDataFile();
    var elem := DFFindElement(df, event_id);
    if (elem)
        var event := elem.GetProp("event_data");
        event.status := BET_STATUS_FINISHED;
        elem.SetProp("event_data", event);
        return 1;
    endif
    return 0;
endfunction

function CancelEvent(event_id)
    var df := GetBettingDataFile();
    var elem := DFFindElement(df, event_id);
    if (!elem)
        return 0;
    endif
    
    var event := elem.GetProp("event_data");
    event.status := BET_STATUS_CANCELLED;
    elem.SetProp("event_data", event);
    
    // Retorna apostas
    foreach bet in (event.bets)
        CreateItemInBackpack(bet.better, MOEDA_OURO, bet.amount);
        SendSysMessage(bet.better, "O evento foi cancelado. Suas moedas foram devolvidas.");
        sleepms(2);
    endforeach
    
    return 1;
endfunction

// Interface principal expandida
function CreateBettableEvent(params)
    if (!params.player1 || !params.player2)
        return error{"errortext":="Participantes inválidos"};
    endif
    
    var event := struct{
        eventID := GenerateEventID(),
        player1 := params.player1,
        player2 := params.player2,
        event_type := params.event_type,
        min_bet := CInt(params.min_bet),
        max_bet := CInt(params.max_bet),
        duration := CInt(params.duration),
        status := BET_STATUS_WAITING,
        bets := array{},
        total_pool := 0,
        created_on := PolCore().systime,
        betting_closes_at := params.duration ? (PolCore().systime + params.duration) : 0
    };
    
    return RegisterEvent(event);
endfunction

// Função para verificar validade do evento
function ValidateEvent(event_id)
    var event := GetEvent(event_id);
    if (!event)
        return 0;
    endif
    
    // Verifica status
    if (event.status != BET_STATUS_WAITING)
        return 0;
    endif
    
    // Verifica tempo limite se existir
    if (event.duration && (PolCore().systime > event.betting_closes_at))
        event.status := BET_STATUS_LOCKED;
        RegisterEvent(event);
        return 0;
    endif
    
    return 1;
endfunction

// Função para listar eventos ativos
function ListActiveEvents()
    var df := GetBettingDataFile();
    var active_events := array{};
    
    foreach event_id in (df.Keys())
        var event := GetEvent(event_id);
        if (event && event.status == BET_STATUS_WAITING)
            active_events.append(event);
        endif
        sleepms(2);
    endforeach
    
    return active_events;
endfunction

// Função para obter apostas de um evento
function GetEventBets(event_id)
    var event := GetEvent(event_id);
    if (event)
        return event.bets;
    endif
    return array{};
endfunction
// Estrutura para armazenar apostas
var BetStruct := struct;
BetStruct.+better;     // Quem apostou
BetStruct.+amount;     // Quanto apostou
BetStruct.+player;     // Em quem apostou

function CheckBetGold(who, amount)
    var total := 0;
    foreach item in EnumerateItemsInContainer(who.backpack)
        if (item.objtype == MOEDA_OURO)
            total += item.amount;
        endif
        SleepMS(2);
    endforeach
    return (total >= amount);
endfunction

function RemoveGoldAmount(container, amount)
    var total_removed := 0;
    foreach item in EnumerateItemsInContainer(container)
        if (item.objtype == MOEDA_OURO)
            if ((item.amount + total_removed) > amount)
                var remove_amount := (amount - total_removed);
                if (SubtractAmount(item, remove_amount))
                    return 1;
                endif
            else
                total_removed += item.amount;
                DestroyItem(item);
                if (total_removed >= amount)
                    return 1;
                endif
            endif
        endif
        SleepMS(2);
    endforeach
    return 0;
endfunction

// Função para processar aposta principal entre jogadores
function ProcessMainBet(winner, loser, amount)
    if (!amount)
        return 1;
    endif
    
    // Verifica se ambos ainda têm o gold
    if (!CheckBetGold(winner, amount) || !CheckBetGold(loser, amount))
        SendSysMessage(winner, "Aposta cancelada - um dos jogadores não tem moedas de ouro suficientes.");
        SendSysMessage(loser, "Aposta cancelada - um dos jogadores não tem moedas de ouro suficientes.");
        return 0;
    endif
    
    // Remove gold do perdedor
    if (!RemoveGoldAmount(loser.backpack, amount))
        SendSysMessage(winner, "Erro ao processar aposta.");
        return 0;
    endif
    
    // Adiciona gold ao vencedor
    CreateItemInBackpack(winner, MOEDA_OURO, amount);
    
    // Anuncia
    SendSysMessage(winner, "Você ganhou " + amount + " moedas de ouro na aposta!");
    SendSysMessage(loser, "Você perdeu " + amount + " moedas de ouro na aposta!");
    
    return 1;
endfunction

// Função para gerir apostas de espectadores
function AddSpectatorBet(game, better, target, amount)
    if (better == game.challenger || better == game.target)
        SendSysMessage(better, "Jogadores não podem fazer apostas laterais!");
        return 0;
    endif
    
    if (!CheckGold(better, amount))
        SendSysMessage(better, "Você não tem gold suficiente para esta aposta.");
        return 0;
    endif
    
    // Cria estrutura da aposta
    var bet := struct;
    bet.+better := better;
    bet.+amount := amount;
    bet.+player := target;
    
    // Adiciona à lista de apostas do jogo
    if (!game.spectator_bets)
        game.spectator_bets := array{};
    endif
    
    game.spectator_bets.append(bet);
    
    // Remove o gold do apostador
    if (!SubtractAmount(better.backpack, 0xEED, amount))
        SendSysMessage(better, "Erro ao processar sua aposta.");
        return 0;
    endif
    
    SendSysMessage(better, "Aposta registrada: " + amount + " moedas em " + target.name);
    return 1;
endfunction

// Processa todas as apostas de espectadores
function ProcessSpectatorBets(game, winner)
    if (!game.spectator_bets)
        return 1;
    endif
    
    var bet_list := game.spectator_bets;
    foreach currentBet in bet_list
        var better := currentBet.better;
        var amount := currentBet.amount;
        var player := currentBet.player;
        
        if (player == winner)
            // Apostador ganhou - recebe 2x a aposta
            CreateItemInBackpack(better, 0xEED, amount * 2);
            SendSysMessage(better, "Você ganhou " + (amount * 2) + " moedas na sua aposta!");
        else
            SendSysMessage(better, "Você perdeu " + amount + " moedas na sua aposta!");
        endif
        SleepMS(2);
    endforeach
    return 1;
endfunction

function EndGame(byref game)
    if (!game)
        return 0;
    endif
    
    UpdateGameState(game, STATE_GAME_OVER);
    
    var winner := (game.rounds_won_challenger >= ROUNDS_TO_WIN) ? game.challenger : game.opponent;
    var loser := (winner == game.challenger) ? game.opponent : game.challenger;
    
    if (winner && loser)
        SendSysMessage(winner, "Você venceu a queda de braço!");
        SendSysMessage(loser, "Você perdeu a queda de braço!");
        
        // Atualiza ranking, títulos e estatísticas
        UpdateLeaderboard(winner, loser);
        
        // Efeitos visuais/sonoros
        PrintTextAbove(winner, "*VITORIOSO!*");
        PlaySoundEffect(winner, 0x4C);
    endif
    
    UnregisterGame(game.challenger, game.opponent);
    return 1;
endfunction