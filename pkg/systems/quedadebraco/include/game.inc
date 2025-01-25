use uo;
use os;
use attributes;
use basic;
use polsys;

include ":gumps:yesNo";
include ":attributes:attributes";
include ":quedadebraco:emotes";
include ":quedadebraco:constants";
include ":charactercreation:resists";  // Para GetResist
include ":quedadebraco:leaderboard";   // Para UpdateLeaderboard
include ":attributes:stats";            // Para constantes de atributos


function ProcessRound(byref game, challenger_move, opponent_move)
    if (!game || !game.challenger || !game.opponent)
        return 0;
    endif

    var challenger := game.challenger;
    var opponent := game.opponent;
    
    // Cria o cabeçalho do log
    print("===========================================================");
    print("QUEDA DE BRACO - LOG DE COMBATE - " + StrFormatTime("%Y-%m-%d %H:%M:%S"));
    print("Desafiante: " + challenger.name + " (Serial: " + challenger.serial + ")");
    print("Desafiado:  " + opponent.name + " (Serial: " + opponent.serial + ")");
    print("Rodada:     " + game.current_round + " de " + TOTAL_ROUNDS);
    print("===========================================================");
    
    // 1. Preparação 
    PlayPreparacaoEmote(challenger);
    PlayPreparacaoEmote(opponent);
    sleepms(3000);
    PlayPreparacaoEmote(challenger);
    PlayPreparacaoEmote(opponent);
    sleepms(3000);

    // 2. Contagem regressiva
    for i := 1 to 4
        PlayContagemEmote(challenger, i);
        sleepms(2000);
    endfor
    
// Calcula valores base
    var stats := CalculateStats(game, challenger_move, opponent_move);
    ValidateStats(stats);
    PrintStatsLog(stats, challenger, opponent, challenger_move, opponent_move);
    
    // 3. Mostra força
    PlayForcaEmote(challenger, stats.challenger_str);
    PlayForcaEmote(opponent, stats.opponent_str);
    sleepms(3000);
    
    // 4. Mostra fortitude
    PlayFortitudeEmote(challenger, stats.challenger_fort);
    PlayFortitudeEmote(opponent, stats.opponent_fort);
    sleepms(3000);
    
    // 5. Execução dos movimentos
    PlayMovimentoEmote(challenger, challenger_move);
    sleepms(3000);
    PlayMovimentoEmote(opponent, opponent_move);
    sleepms(3000);

// 6. Sorte e Críticos
    var challenger_luck_result := ProcessLuck(stats.challenger_luck);
    var opponent_luck_result := ProcessLuck(stats.opponent_luck);
    
    // Adicionar aqui os emotes de sorte
    PlaySorteEmote(challenger, stats.challenger_luck, challenger_luck_result.is_crit, challenger_luck_result.crit_type == "success");
    sleepms(2000);
    PlaySorteEmote(opponent, stats.opponent_luck, opponent_luck_result.is_crit, opponent_luck_result.crit_type == "success");
    sleepms(2000);
    
    // Verifica críticos para vitória/derrota automática
    if (challenger_luck_result.is_crit || opponent_luck_result.is_crit)
        var winner;
        
        // Caso ambos tenham crítico, o crítico de sucesso tem prioridade
        if (challenger_luck_result.is_crit && opponent_luck_result.is_crit)
            if (challenger_luck_result.crit_type == "success")
                winner := challenger;
            elseif (opponent_luck_result.crit_type == "success")
                winner := opponent;
            elseif (challenger_luck_result.crit_type == "fail")
                winner := opponent;
            else
                winner := challenger;
            endif
        elseif (challenger_luck_result.is_crit)
            winner := (challenger_luck_result.crit_type == "success") ? challenger : opponent;
        elseif (opponent_luck_result.is_crit)
            winner := (opponent_luck_result.crit_type == "success") ? opponent : challenger;
        endif
        
        // Emotes e efeitos de crítico
        if (winner)
            if (winner == challenger)
                if (challenger_luck_result.crit_type == "success")
                    PrintTextAbove(challenger, "*Movimento perfeito!*");
                else
                    PrintTextAbove(opponent, "*Falha desastrosa!*");
                endif
            else
                if (opponent_luck_result.crit_type == "success")
                    PrintTextAbove(opponent, "*Movimento perfeito!*");
                else
                    PrintTextAbove(challenger, "*Falha desastrosa!*");
                endif
            endif
            
            PlaySoundEffect(winner, 0x3B2);
            return winner;
        endif
    endif
    
    // Se não houve crítico, continua com o cálculo normal usando o novo modelo
// Se não houve crítico, continua com o cálculo normal usando o novo modelo
var totals := CalculateTotals(stats);
PrintTotalsLog(totals, challenger, opponent);

// Determina vencedor baseado nos totais
var winner := (totals.challenger_total > totals.opponent_total) ? challenger : opponent;
var loser := (winner == challenger) ? opponent : challenger;
var difference := Abs(totals.challenger_total - totals.opponent_total);

// Log do resultado
print("RESULTADO:");
print("Vencedor: " + winner.name);
print("Diferença: " + difference);
print("===========================================================");

// Exibe emotes de resultado
PlayResultadoEmote(winner, 1);
PlayResultadoEmote(loser, 0);
sleepms(1000);

// Efeitos de vitória
PerformAction(winner, 0x10);
PlaySoundEffect(winner, 0x4B);

UpdateGameScore(game, winner);
SetGameState(game, STATE_ROUND_END);
return winner;
endfunction

// Em CalculateStats - remover a primeira atribuição de sorte
function CalculateStats(game, challenger_move, opponent_move)
    if (!game || !game.challenger || !game.opponent)
        print("ERROR: Game or player objects missing");
        return 0;
    endif
    
    var stats := struct{
        // Referências aos objetos dos jogadores
        "challenger"         := game.challenger,
        "opponent"          := game.opponent,
        
        // Stats base
        "challenger_str"    := AP_GetTrueStat(game.challenger, "Strength"),
        "challenger_fort"   := GetResist(game.challenger, FORTITUDE),  // Modificado
        "opponent_str"      := AP_GetTrueStat(game.opponent, "Strength"),
        "opponent_fort"     := GetResist(game.opponent, FORTITUDE),    // Modificado
        
        // Resto do struct permanece igual...
        "challenger_str_mod" := GetMoveStrMod(challenger_move),
        "challenger_fort_mod" := GetMoveFortMod(challenger_move),
        "opponent_str_mod"   := GetMoveStrMod(opponent_move),
        "opponent_fort_mod"  := GetMoveFortMod(opponent_move),
        
        "challenger_crit_success" := 0,
        "challenger_crit_fail"    := 0,
        "opponent_crit_success"   := 0,
        "opponent_crit_fail"      := 0
    };
    
    // Validações apenas para força
    if (!stats.challenger_str || stats.challenger_str < 1)
        stats.challenger_str := 10;
    endif
    if (!stats.opponent_str || stats.opponent_str < 1)
        stats.opponent_str := 10;  
    endif
    
    print("Challenger Stats - Str: " + stats.challenger_str + " Fort: " + stats.challenger_fort);
    print("Opponent Stats - Str: " + stats.opponent_str + " Fort: " + stats.opponent_fort);
    
    // Calcula sorte (0-100)
    stats.challenger_luck := CalculateLuck();
    stats.opponent_luck := CalculateLuck();
    
    print("=== Sorte (0-100) ===");
    print("Desafiante: " + stats.challenger_luck + " - " + GetCriticalStatus(stats.challenger_luck));
    print("Oponente: " + stats.opponent_luck + " - " + GetCriticalStatus(stats.opponent_luck));
    
    ValidateStats(stats);
    return stats;
endfunction

// Função auxiliar para mostrar status de crítico
function GetCriticalStatus(luck)
    if (luck >= LUCK_CRIT_SUCCESS)
        return "CRÍTICO DE SUCESSO";
    elseif (luck <= LUCK_CRIT_FAIL)
        return "CRÍTICO DE FALHA";
    endif
    return "Normal";
endfunction

// Também precisamos ajustar o consumo de Stamina após cada rodada
function ConsumeFortitude(who, amount)
    if (!who)
        return 0;
    endif
    
    var current_stam := AP_GetVital(who, "Stamina");
    if (current_stam < amount)
        amount := current_stam; // Consome o que puder
    endif
    
    return AP_ConsumeVital(who, "Stamina", amount);
endfunction

function GetMoveStrMod(move)
    case (move)
        CONST_MOVE_HOOK:     return 1.3;  // +30% força
        CONST_MOVE_COVER:    return 1.15; // +15% força (aumentado de 5% para 15%)
        CONST_MOVE_PRESSURE: return 0.7;  // -30% força
        default:       return 1.0;
    endcase
endfunction

function GetMoveFortMod(move)
    case (move)
        CONST_MOVE_HOOK:     return 0.7;  // -30% fortitude
        CONST_MOVE_COVER:    return 1.15; // +15% fortitude (aumentado de 5% para 15%)
        CONST_MOVE_PRESSURE: return 1.3;  // +30% fortitude
        default:       return 1.0;
    endcase
endfunction

// Função para processar críticos com novo sistema
function ProcessCriticals(byref stats)
    // Crítico do desafiante
    if (stats.challenger_luck >= LUCK_CRIT_SUCCESS)
        stats.challenger_str *= 1.3;
        stats.challenger_crit_success := 1;
        print("CRÍTICO DE SUCESSO - Desafiante: +" + (stats.challenger_str * 0.3) + " (Sorte: " + stats.challenger_luck + ")");
    elseif (stats.challenger_luck <= LUCK_CRIT_FAIL)
        stats.challenger_str *= 0.7;
        stats.challenger_crit_fail := 1;
        print("CRÍTICO DE FALHA - Desafiante: -" + (stats.challenger_str * 0.3) + " (Sorte: " + stats.challenger_luck + ")");
    endif
    
    // Crítico do alvo
    if (stats.opponent_luck >= LUCK_CRIT_SUCCESS)
        stats.opponent_str *= 1.3;
        stats.opponent_crit_success := 1;
        print("CRÍTICO DE SUCESSO - Oponente: +" + (stats.opponent_str * 0.3) + " (Sorte: " + stats.opponent_luck + ")");
    elseif (stats.opponent_luck <= LUCK_CRIT_FAIL)
        stats.opponent_str *= 0.7;
        stats.opponent_crit_fail := 1;
        print("CRÍTICO DE FALHA - Oponente: -" + (stats.opponent_str * 0.3) + " (Sorte: " + stats.opponent_luck + ")");
    endif
endfunction


// Modificação na função CalculateTotals para usar nova escala
function CalculateTotals(stats)
    var totals := struct;
    
    print("=== Debug CalculateTotals ===");
    
    // Challenger
    var challenger_base_str := stats.challenger_str * stats.challenger_str_mod;
    var challenger_fort := CalculateFortitude(stats, 0);
    
    print("Challenger:");
    print("  Base Strength: " + challenger_base_str);
    print("  Fortitude Total: " + challenger_fort);
    print("  Sorte: " + stats.challenger_luck);
    
    // Aplica as porcentagens para o challenger
    totals.+challenger_str_component := challenger_base_str * PERCENT_STRENGTH;
    totals.+challenger_fort_component := challenger_fort * PERCENT_FORTITUDE;
    totals.+challenger_luck_component := stats.challenger_luck * PERCENT_LUCK;
    totals.+challenger_racial_mod := CalculateRacialMod(stats, 0) * PERCENT_RACIAL;
    
    // Opponent
    var opponent_base_str := stats.opponent_str * stats.opponent_str_mod;
    var opponent_fort := CalculateFortitude(stats, 1);
    
    print("Opponent:");
    print("  Base Strength: " + opponent_base_str);
    print("  Fortitude Total: " + opponent_fort);
    print("  Sorte: " + stats.opponent_luck);
    
    // Aplica as porcentagens para o opponent
    totals.+opponent_str_component := opponent_base_str * PERCENT_STRENGTH;
    totals.+opponent_fort_component := opponent_fort * PERCENT_FORTITUDE;
    totals.+opponent_luck_component := stats.opponent_luck * PERCENT_LUCK;
    totals.+opponent_racial_mod := CalculateRacialMod(stats, 1) * PERCENT_RACIAL;
    
    // Calcula totais finais
    totals.+challenger_total := totals.challenger_str_component + 
                               totals.challenger_fort_component + 
                               totals.challenger_luck_component +
                               totals.challenger_racial_mod;
                               
    totals.+opponent_total := totals.opponent_str_component + 
                             totals.opponent_fort_component + 
                             totals.opponent_luck_component +
                             totals.opponent_racial_mod;
    
    print("=== Totais Finais ===");
    print("Challenger:");
    print("  Força (42%): " + totals.challenger_str_component);
    print("  Fortitude (42%): " + totals.challenger_fort_component);
    print("  Sorte (14%): " + totals.challenger_luck_component);
    print("  Racial (2%): " + totals.challenger_racial_mod);
    print("  TOTAL: " + totals.challenger_total);
    
    print("Opponent:");
    print("  Força (42%): " + totals.opponent_str_component);
    print("  Fortitude (42%): " + totals.opponent_fort_component);
    print("  Sorte (14%): " + totals.opponent_luck_component);
    print("  Racial (2%): " + totals.opponent_racial_mod);
    print("  TOTAL: " + totals.opponent_total);
    
    return totals;
endfunction

// Função PrintPlayerTotals corrigida
function PrintPlayerTotals(who, totals, prefix)
    print(who.name + " Total: " + totals[prefix+"_total"]);
    print("  * Força (42%): " + totals[prefix+"_str_component"]);
    print("  * Fortitude (42%): " + totals[prefix+"_fort_component"]);
    print("  * Racial (2%): " + totals[prefix+"_racial_mod"]);
    print("  * Sorte (14%): " + totals[prefix+"_luck_component"]);
endfunction

function CalculateBaseStrength(stats, is_opponent := 0)
    if (is_opponent)
        return stats.opponent_str * stats.opponent_str_mod;
    else
        return stats.challenger_str * stats.challenger_str_mod;
    endif
endfunction

function UpdateGameScore(byref game, winner)
    print("DEBUG: Atualizando pontuação");
    print("Placar anterior: " + game.rounds_won_challenger + " x " + game.rounds_won_opponent);
    
    if (!game)
        print("ERROR: Game struct is null in UpdateGameScore");
        return 0;
    endif
    
    // Atualiza apenas 1 ponto por vitória
    if (winner == game.challenger)
        game.rounds_won_challenger += 1;
        print("DEBUG: Ponto para o desafiante");
    else
        game.rounds_won_opponent += 1;
        print("DEBUG: Ponto para o oponente");
    endif
    
    print("Novo placar: " + game.rounds_won_challenger + " x " + game.rounds_won_opponent);
    return 1;
endfunction


function PrintStatsLog(stats, challenger, opponent, challenger_move, opponent_move)
    print("MOVIMENTOS ESCOLHIDOS:");
    print("  " + challenger.name + ": " + GetMoveName(challenger_move));
    print("  " + opponent.name + ": " + GetMoveName(opponent_move));
    print("");
    
    print("VALORES BASE E MODIFICADORES:");
    // Imprime stats do challenger
    PrintPlayerDetailedStats(challenger, stats, challenger_move, 0);
    // Imprime stats do opponent
    PrintPlayerDetailedStats(opponent, stats, opponent_move, 1);

    // Debug de Sorte
    print("ROLAGENS DE SORTE:");
    print("  " + challenger.name + ": " + stats.challenger_luck);
    print("  " + opponent.name + ": " + stats.opponent_luck);
    print("");
endfunction

// Nova função auxiliar para evitar código duplicado
function PrintPlayerDetailedStats(who, stats, move, is_opponent)
    var prefix := is_opponent ? "opponent" : "challenger";
    
    print("=== " + who.name + " ===");
    
    // Stats Base
    print("  Força Base: " + stats[prefix+"_str"]);
    print("  Fortitude Base: " + stats[prefix+"_fort"]);
    print("  Modificadores de Movimento:");
    print("    Força: " + GetMoveStrMod(move));
    print("    Fortitude: " + GetMoveFortMod(move));
    
    // Debug Racial
    var chardata := GetObjProperty(who, "chardata");
    print("  Info Racial:");
    print("    Race: " + who.title_race);
    if (chardata && chardata.culture)
        print("    Culture: " + chardata.culture);
    endif
    print("    Race Modifier: " + CalculateRacialMod(stats, is_opponent));

    // Debug Fortitude
    print("  Info Fortitude:");
    print("    GetResist Value: " + GetResist(who, FORTITUDE));
    print("    Calculated Fortitude: " + CalculateFortitude(stats, is_opponent));
    print("");
endfunction


function PrintPlayerStats(who, str, fort, move, prefix)
    print(who.name + ":");
    print("  Força Base: " + str);
    print("  Fortitude Base: " + fort);
    print("  Modificadores:");
    print("    Força: " + GetMoveStrMod(move));
    print("    Fortitude: " + GetMoveFortMod(move));
endfunction

function PrintCriticalsLog(stats, challenger, opponent)
    print("ROLAGENS DE SORTE E CRÍTICOS:");
    PrintPlayerCriticals(challenger, stats.challenger_luck, stats.challenger_crit_success, stats.challenger_crit_fail);
    print("");
    PrintPlayerCriticals(opponent, stats.opponent_luck, stats.opponent_crit_success, stats.opponent_crit_fail);
    print("");
endfunction


// Atualizar função PrintPlayerCriticals para usar escala correta
function PrintPlayerCriticals(who, luck, crit_success, crit_fail)
    print(who.name + ":");
    print("  Sorte: " + luck + " (0-100)");
    print("  Crítico Sucesso: " + YesNo(crit_success));
    print("  Crítico Falha: " + YesNo(crit_fail));
endfunction

function PrintTotalsLog(totals, challenger, opponent)
    print("CÁLCULO FINAL:");
    PrintPlayerTotals(challenger, totals, "challenger");
    print("");
    PrintPlayerTotals(opponent, totals, "opponent");
    print("");
endfunction


function GetMoveName(move)
    case (move)
        CONST_MOVE_HOOK:     return "Gancho (+Força -Fortitude)";
        CONST_MOVE_COVER:    return "Alavanca (+Força +Fortitude)";
        CONST_MOVE_PRESSURE: return "Pressão (-Força +Fortitude)";
        default:       return "Nenhum";
    endcase
endfunction

// Função CalculateFortitude melhorada
function CalculateFortitude(stats, is_opponent := 0)
    var who := is_opponent ? stats.opponent : stats.challenger;
    if (!who)
        print("ERROR: Player object is null in CalculateFortitude");
        return 0;
    endif
    
    // Pega a resistência base
    var resist_fort := GetResist(who, FORTITUDE);
    if (!resist_fort)
        resist_fort := 0;
    endif
    
    // Fator de escala para equilibrar com a Força
    const FORT_SCALE := 25; // Cada ponto de Fortitude vale 15 pontos
    
    // Pega o modificador do movimento
    var fort_mod := is_opponent ? stats.opponent_fort_mod : stats.challenger_fort_mod;
    
    print("=== Debug Fortitude para " + who.name + " ===");
    print("  Resist Fort Base: " + resist_fort);
    print("  Movement Mod: " + fort_mod);
    
    // Calcula o total aplicando o modificador do movimento e a escala
    var scaled_fort := resist_fort * FORT_SCALE;
    var total_fort := scaled_fort * fort_mod;
    
    print("  Scaled Fort: " + scaled_fort);
    print("  Total Fort (após modificador): " + total_fort);
    return total_fort;
endfunction

// Função CalculateRacialMod revisada
function CalculateRacialMod(stats, is_opponent := 0)
    var who := is_opponent ? stats.opponent : stats.challenger;
    if (!who)
        print("ERROR: Player object is null in CalculateRacialMod");
        return 0;
    endif
    
    var raca := who.title_race;
    print("=== Debug CalculateRacialMod para " + who.name + " ===");
    print("  Title Race: " + raca);
    
    // Base para cálculo racial (100 pontos base)
    const BASE_RACIAL := 100;
    var racial_mod := 1.0; // Valor padrão (humano)
    
    // Verifica a raça e define o modificador
    if (raca == "Anão" || raca == "Anao")
        racial_mod := 1.2;    // 20% mais forte
    elseif (raca == "Orc")
        racial_mod := 1.2;    // 20% mais forte
    elseif (raca == "Humano")
        racial_mod := 1.0;    // Base
    elseif (raca == "Elfo")
        racial_mod := 0.9;    // 10% mais fraco
    elseif (raca == "Drow")
        racial_mod := 0.9;    // 10% mais fraco
    elseif (raca == "Polski")
        racial_mod := 0.8;    // 20% mais fraco
    endif
    
    // Calcula pontos raciais
    var racial_points := CInt(BASE_RACIAL * racial_mod);
    
    print("  Modificador Racial: " + racial_mod);
    print("  Pontos Raciais: " + racial_points);
    
    return racial_points;
endfunction

// Função para calcular sorte com distribuição mais equilibrada (0-100)
function CalculateLuck()
    // Crítico forçado (5% de chance)
    var crit_roll := RandomInt(100) + 1;
    if (crit_roll <= 5)
        if (RandomInt(2) == 1)
            print("CRÍTICO FORÇADO: Sucesso Natural!");
            return LUCK_MAX;
        else
            print("CRÍTICO FORÇADO: Falha Natural!");
            return LUCK_MIN;
        endif
    endif
    
    // Rolagem normal
    var variance := RandomInt(LUCK_VARIANCE * 2 + 1) - LUCK_VARIANCE;
    var luck := LUCK_BASE + variance;
    
    // Garante limites
    luck := Cint(Max(LUCK_MIN, Min(LUCK_MAX, luck)));
    
    return luck;
endfunction


// Função atualizada para processar críticos com sons apropriados
function ProcessLuck(luck_value)
    var result := struct{
        "value"      := luck_value,
        "is_crit"    := 0,
        "crit_type"  := "",
        "message"    := "",
        "sound"      := 0
    };
    
    if (luck_value >= LUCK_CRIT_SUCCESS)
        result.is_crit := 1;
        result.crit_type := "success";
        result.message := "CRÍTICO DE SUCESSO - Vitória Automática!";
        result.sound := CRITICO_SUCESSO_SOUNDS[RandomInt(CRITICO_SUCESSO_SOUNDS.size()) + 1];
    elseif (luck_value <= LUCK_CRIT_FAIL)
        result.is_crit := 1;
        result.crit_type := "fail";
        result.message := "CRÍTICO DE FALHA - Derrota Automática!";
        result.sound := CRITICO_FALHA_SOUNDS[RandomInt(CRITICO_FALHA_SOUNDS.size()) + 1];
    endif
    
    return result;
endfunction

function ProcessLuckResult(luck_value, who)
    var result := struct{
        "value"         := luck_value,
        "is_crit"       := 0,
        "crit_type"     := "",
        "description"   := "",
        "modifier"      := 1.0
    };
    
    if (luck_value >= LUCK_CRIT_SUCCESS)
        result.is_crit := 1;
        result.crit_type := "success";
        result.description := "CRÍTICO DE SUCESSO";
        result.modifier := 1.3;
    elseif (luck_value <= LUCK_CRIT_FAIL)
        result.is_crit := 1;
        result.crit_type := "fail";
        result.description := "CRÍTICO DE FALHA";
        result.modifier := 0.7;
        
        // Efeito especial de falha crítica
        if (who)
            PrintTextAbove(who, "*se caga nas calças*");
            PlaySoundEffect(who, 0x246);
            
            var fezes := CreateItemAtLocation(who.x, who.y, who.z, 0x122E, 1, who.realm);
            if (fezes)
                fezes.name := "fezes";
                fezes.color := 1036;
                fezes.movable := 0;
                
                SetObjProperty(fezes, "#DeleteTime", ReadGameClock() + 30);
                start_script(":quedadebraco:cleanup", fezes);
            endif
        endif
    else
        // Resultados não-críticos
        if (luck_value > LUCK_BASE + 20)
            result.description := "Muito Sortudo";
            result.modifier := 1.15; // +15%
        elseif (luck_value > LUCK_BASE)
            result.description := "Sortudo";
            result.modifier := 1.07; // +7%
        elseif (luck_value < LUCK_BASE - 20)
            result.description := "Muito Azarado";
            result.modifier := 0.85; // -15%
        elseif (luck_value < LUCK_BASE)
            result.description := "Azarado";
            result.modifier := 0.93; // -7%
        else
            result.description := "Normal";
            result.modifier := 1.0; // Sem modificador
        endif
    endif
    
    return result;
endfunction


function PrintLuckDetails(who, luck_result)
    print("=== Resultado de Sorte para " + who.name + " ===");
    print("Valor: " + luck_result.value);
    print("Resultado: " + luck_result.description);
    if (luck_result.is_crit)
        print("CRÍTICO! Tipo: " + luck_result.crit_type);
    endif
    print("Modificador: " + ((luck_result.modifier - 1) * 100) + "%");
endfunction


function GetLuckBonus(luck_value)
    // Sistema de bônus escalonado baseado na sorte
    if (luck_value >= LUCK_CRIT_SUCCESS)
        return 2.0; // Dobro do valor normal para críticos
    elseif (luck_value <= LUCK_CRIT_FAIL)
        return 0.5; // Metade do valor normal para falhas
    else
        // Escala linear entre 0.75 e 1.25 para valores normais
        var scale := 0.75 + ((luck_value - LUCK_CRIT_FAIL) * 0.5 / (LUCK_CRIT_SUCCESS - LUCK_CRIT_FAIL));
        return scale;
    endif
endfunction

// Função auxiliar para pontos raciais
function case_racial_points(raca)
    case (raca)
        ANAO: return 60;      // 1.2 pontos
        ORC: return 60;       // 1.2 pontos
        HUMANO: return 50;    // 1.0 ponto
        ELFO: return 45;      // 0.9 pontos
        DROW: return 45;      // 0.9 pontos
        POLSKI: return 40;    // 0.8 pontos
        default: return 0;
    endcase
endfunction

function ValidateStats(byref stats)
    if (!stats)
        print("ERROR: Stats object is null");
        return 0;
    endif
    
    if (!stats.challenger || !stats.opponent)
        print("ERROR: Player objects missing");
        return 0;
    endif
    
    print("=== Validando Stats ===");
    
    // Validar valores mínimos
    stats.challenger_str := Max(1, CInt(stats.challenger_str));
    stats.challenger_fort := Max(1, CInt(stats.challenger_fort));
    stats.opponent_str := Max(1, CInt(stats.opponent_str));
    stats.opponent_fort := Max(1, CInt(stats.opponent_fort));
    
    // Validar modificadores
    stats.challenger_str_mod := Max(0.1, CDbl(stats.challenger_str_mod));
    stats.challenger_fort_mod := Max(0.1, CDbl(stats.challenger_fort_mod));
    stats.opponent_str_mod := Max(0.1, CDbl(stats.opponent_str_mod));
    stats.opponent_fort_mod := Max(0.1, CDbl(stats.opponent_fort_mod));
    
    // Validar sorte
    stats.challenger_luck := Cint(Max(0, Min(100, stats.challenger_luck)));
    stats.opponent_luck := Cint(Max(0, Min(100, stats.opponent_luck)));
    
    return 1;
endfunction