use uo;
use os;
use util;
use math;

include ":datafile:datafile";
include ":quedadebraco:emotes";

// Estrutura de dados do jogador no ranking
var rank_data := struct;
rank_data.+serial;           // Serial do jogador
rank_data.+display_name;     // Nome de exibição personalizado
rank_data.+account_name;     // Nome da conta (opcional)
rank_data.+wins;            // Total de vitórias
rank_data.+losses;          // Total de derrotas
rank_data.+points;          // Pontos totais
rank_data.+title;           // Título atual

// Obtém ou cria o datafile do ranking
function GetLeaderboardDataFile()
    var df := DFOpenDataFile(":quedadebraco:leaderboard", DF_CREATE);
    return df;
endfunction

// Atualiza o ranking após uma partida
function UpdateLeaderboard(winner, loser)
    var df := GetLeaderboardDataFile();
    
    // Obtém ou cria dados dos jogadores
    var winner_data := GetPlayerData(df, winner);
    var loser_data := GetPlayerData(df, loser);
    
    // Atualiza estatísticas
    winner_data.wins += 1;
    winner_data.points += 10;  // Pontos por vitória
    loser_data.losses += 1;
    loser_data.points := Max(0, loser_data.points - 5);  // Perde pontos, mínimo 0
    
    // Atualiza títulos
    winner_data.title := CalculateTitle(winner_data.points);
    loser_data.title := CalculateTitle(loser_data.points);
    
    // Salva dados
    SavePlayerData(df, winner.serial, winner_data);
    SavePlayerData(df, loser.serial, loser_data);
    
    // Anuncia novo título se mudou
    CheckAndAnnounceTitleChange(winner, winner_data.title);
    CheckAndAnnounceTitleChange(loser, loser_data.title);
endfunction

// Configura ou atualiza o nome de exibição de um jogador
function SetPlayerDisplayName(who, display_name)
    var df := GetLeaderboardDataFile();
    
    // Verifica se o nome já está em uso por outro jogador
    var current_elem := DFFindElement(df, who.serial);
    var current_name := current_elem.GetProp("display_name");
    
    if (current_name == display_name)
        // Mesmo jogador querendo usar o mesmo nome, permite
        return 1;
    endif
    
    // Verifica se o nome está em uso por outro jogador
    if (IsDisplayNameTaken(df, display_name, who.serial))
        return error{"errortext":="Este nome já está em uso por outro competidor."};
    endif
    
    var data := GetPlayerData(df, who);
    data.display_name := display_name;
    SavePlayerData(df, who.serial, data);
    
    return 1;
endfunction

// Verifica se um nome de exibição já está em uso
function IsDisplayNameTaken(df, display_name, exclude_serial := 0)
    var names := df.keys();
    foreach serial in names
        var elem := DFFindElement(df, serial);
        if (elem && elem.GetProp("display_name") == display_name && serial != exclude_serial)
            return 1;
        endif
        sleepms(2);
    endforeach
    return 0;
endfunction

// Obtém dados de um jogador
function GetPlayerData(df, player)
    var elem := DFFindElement(df, player.serial, DF_CREATE);
    var data := struct;
    data.+serial := player.serial;
    data.+display_name := elem.GetProp("display_name") ? elem.GetProp("display_name") : "";
    data.+account_name := player.acctname;
    data.+wins := elem.GetProp("wins") ? elem.GetProp("wins") : 0;
    data.+losses := elem.GetProp("losses") ? elem.GetProp("losses") : 0;
    data.+points := elem.GetProp("points") ? elem.GetProp("points") : 0;
    data.+title := elem.GetProp("title") ? elem.GetProp("title") : "Novato";
    
    return data;
endfunction

// Salva dados de um jogador
function SavePlayerData(df, serial, data)
    var elem := DFFindElement(df, serial, DF_CREATE);
    elem.SetProp("display_name", data.display_name);
    elem.SetProp("account_name", data.account_name);
    elem.SetProp("wins", data.wins);
    elem.SetProp("losses", data.losses);
    elem.SetProp("points", data.points);
    elem.SetProp("title", data.title);
endfunction

// Função de classificação para ordenar jogadores por pontos
function SortByPoints(a, b)
    if (CInt(a.points) > CInt(b.points))
        return -1;  // a vem antes de b
    elseif (CInt(a.points) < CInt(b.points))
        return 1;   // b vem antes de a
    else
        // Se os pontos são iguais, desempata por vitórias
        if (CInt(a.wins) > CInt(b.wins))
            return -1;
        elseif (CInt(a.wins) < CInt(b.wins))
            return 1;
        endif
    endif
    return 0; // mantém a ordem se tudo for igual
endfunction

// Calcula título baseado nos pontos
function CalculateTitle(points)
    points := CInt(points);
    
    if (points < 100)
        return "Novato";
    elseif (points < 300)
        return "Iniciante";
    elseif (points < 600)
        return "Competidor";
    elseif (points < 1000)
        return "Veterano";
    elseif (points < 1500)
        return "Mestre";
    endif
    
    return "Campeão";
endfunction

// Verifica e anuncia mudança de título
function CheckAndAnnounceTitleChange(who, new_title)
    var df := GetLeaderboardDataFile();
    var elem := DFFindElement(df, who.serial);
    var old_title := elem.GetProp("old_title");
    
    if (!old_title)
        elem.SetProp("old_title", new_title);
        return;
    endif
    
    if (old_title != new_title)
        var display_name := elem.GetProp("display_name");
        if (!display_name)
            display_name := who.name;
        endif
        
        Broadcast(display_name + " alcançou o título de " + new_title + "!", 3, 40);
        elem.SetProp("old_title", new_title);
    endif
endfunction

// Verifica se o jogador tem um nome de exibição configurado

function HasDisplayName(who)
    var df := GetLeaderboardDataFile();
    var elem := DFFindElement(df, who.serial);
    var name := elem.GetProp("display_name");
    return (elem && name && name != "");
endfunction

// Obtém o nome de exibição do jogador
function GetDisplayName(who)
    var df := GetLeaderboardDataFile();
    var elem := DFFindElement(df, who.serial);
    if (elem)
        return elem.GetProp("display_name");
    endif
    return "";
endfunction

// Obtém os top jogadores ordenados por pontos

function GetTopPlayers(limit := 20)
    var df := GetLeaderboardDataFile();
    var players := array{};
    
    // Coleta dados de todos os jogadores
    foreach serial in (df.Keys())
        var elem := DFFindElement(df, serial);
        if (elem)
            var data := struct;
            data.+serial := CInt(serial);
            data.+display_name := elem.GetProp("display_name");
            data.+wins := CInt(elem.GetProp("wins"));
            data.+losses := CInt(elem.GetProp("losses"));
            data.+points := CInt(elem.GetProp("points"));
            data.+title := elem.GetProp("title");
            
            // Só adiciona jogadores que têm nome de exibição
            if (data.display_name)
                players.append(data);
            endif
        endif
        sleepms(2);
    endforeach
    
    // Ordena por pontos (do maior para o menor)
    var sorted := array{};
    foreach player in players
        var inserted := 0;
        for i := 1 to sorted.size()
            if (SortByPoints(player, sorted[i]) < 0)  // player tem mais pontos que sorted[i]
                sorted.insert(i, player);
                inserted := 1;
                break;
            endif
        endfor
        if (!inserted)
            sorted.append(player);
        endif
        sleepms(2);
    endforeach
    
    // Retorna apenas o número solicitado de jogadores
    if (sorted.size() > limit)
        var result := array{};
        for i := 1 to limit
            result.append(sorted[i]);
            sleepms(2);
        endfor
        return result;
    endif
    
    return sorted;
endfunction

// Função auxiliar para formatar strings com padding
function PadString(text, length, char := " ")
    text := CStr(text);
    var padding := length - Len(text);
    
    if (padding <= 0)
        return text[1, length];
    endif
    
    return text + char[1] * padding;
endfunction

// Função auxiliar para formatar nomes longos
function FormatDisplayName(name, max_length := 20)
    if (Len(name) > max_length)
        return name[1, max_length-3] + "...";
    endif
    return PadString(name, max_length);
endfunction

// Função para calcular estatísticas gerais
function GetLeaderboardStats()
    var df := GetLeaderboardDataFile();
    var stats := struct;
    stats.+total_players := 0;
    stats.+total_matches := 0;
    stats.+highest_points := 0;
    stats.+highest_wins := 0;
    
    foreach serial in (df.Keys())
        var elem := DFFindElement(df, serial);
        var wins := CInt(elem.GetProp("wins"));
        var points := CInt(elem.GetProp("points"));
        
        if (elem.GetProp("display_name")) // Só conta jogadores ativos
            stats.total_players += 1;
            stats.total_matches += wins + CInt(elem.GetProp("losses"));
            stats.highest_points := Max(stats.highest_points, points);
            stats.highest_wins := Max(stats.highest_wins, wins);
        endif
        
        sleepms(2);
    endforeach
    
    return stats;
endfunction

// Função alternativa de classificação que inclui nome para desempate final
function SortByPointsAndName(a, b)
    var result := SortByPoints(a, b);
    if (result == 0)
        // Se pontos e vitórias são iguais, ordena por nome
        if (a.display_name < b.display_name)
            return -1;
        elseif (a.display_name > b.display_name)
            return 1;
        endif
    endif
    return result;
endfunction