// ALTAR.INC - Funções que manipulam o altar e seus elementos visuais
// Criado durante refatoração para separar responsabilidades

use uo;
use os;

include ":faccao:include/shared_functions";
include ":faccao:spawndecampeao/include/config";
include ":faccao:spawndecampeao/include/arena";
include ":faccao:spawndecampeao/include/constants";

// === FUNÇÕES DE CONTROLE DO ALTAR ===

function ShowBossTimer(altar)
    LogInfo("ChampSpawn", "Iniciando timer de 100 segundos para o boss no altar " + altar.serial);
    
    // Muda a cor do altar quando a contagem começa
    altar.color := 2709;
    
    PrintTextAbove(altar, "* Todas as ondas foram derrotadas! *");
        
    var countdown := 100;
    while (countdown >= 1)
        if (countdown % 10 == 0 || countdown <= 10)
            if (countdown > 10)
                PrintTextAbove(altar, "Campeão desperta em " + countdown + " segundos...");
            else
                PrintTextAbove(altar, "* " + countdown + " *");
                PlaySoundEffect(altar, 0x244);
            endif
        endif
        
        sleep(1);
        
        var spawn_cfg := GetObjProperty(altar, "ChampionConfig");
        
        // Sistema de tolerância durante timer do boss
        if (CountPlayers(altar, spawn_cfg.player_radius) == 0)
            var tolerance_result := WaitForPlayersWithTolerance(altar, "Timer do Boss", 300);
            if (!tolerance_result)
                // Tolerância expirou - cancela timer
                altar.color := 0;
                return 0;
            endif
        endif
        
        countdown := countdown - 1;
    endwhile
    
    LogInfo("ChampSpawn", "Timer concluído - invocando o campeão!");
    PrintTextAbove(altar, "* O CAMPEÃO DESPERTA! *");
    PlaySoundEffect(altar, 0x2F3);
    PlayStationaryEffect(altar.x, altar.y, altar.z, 0x3728, 15, 0x10, 0, altar.realm);
    
    return 1;
endfunction

function ActivateAltar(altar)
    SetObjProperty(altar, "#active", 1);
    
    // Limpa itens órfãos de reinicializações anteriores
    CleanupOrphanedItems(altar);
    
    PrintTextAbove(altar, "* O altar pulsa com uma energia sinistra *");
    PlaySoundEffect(altar, 0x207);
    return 1;
endfunction

function DeactivateAltar(altar, cooldown)
    EraseObjProperty(altar, "#active");
    EraseObjProperty(altar, "#completed");
    EraseObjProperty(altar, "#current_wave_kills");
    EraseObjProperty(altar, "#current_level");

    var skulls := GetObjProperty(altar, "skulls");
    if (TypeOf(skulls) == "Array")
        foreach serial in skulls
            var skull := SystemFindObjectBySerial(serial);
            if (skull) 
                DestroyItem(skull); 
            endif
        endforeach
    endif
    EraseObjProperty(altar, "skulls");
    
    ClearWhiteCandles(altar);
    ClearBossAreaItems(altar);
    
    // Restaura a cor original do altar
    altar.color := 0;
    LogInfo("ChampSpawn", "Cor do altar " + altar.serial + " restaurada para original");
    
    if (!cooldown) 
        cooldown := 86400; 
    endif
    SetObjProperty(altar, "#NextActivation", ReadGameClock() + cooldown);
    
    PrintTextAbove(altar, "* A energia do altar se dissipa... *");
    PlaySoundEffect(altar, 0x1F8);
    return 1;
endfunction

function CreateWhiteCandle(altar, progress_level)
    var white_candles_prop := GetObjProperty(altar, "white_candles");
    var white_candles := (TypeOf(white_candles_prop) == "Array") ? white_candles_prop : array{};
    
    if (white_candles.size() < progress_level && progress_level <= 4)
        var candle_x, candle_y;
        case(progress_level)
            1: candle_x := 0; candle_y := -1;   // Norte
            2: candle_x := 1; candle_y := 0;    // Leste  
            3: candle_x := 0; candle_y := 1;    // Sul
            4: candle_x := -1; candle_y := 0;   // Oeste
        endcase
        
        var white_candle := CreateItemAtLocation(altar.x + candle_x, altar.y + candle_y, altar.z, 0x142C, 1, altar.realm);
        
        if (white_candle)
            white_candle.movable := 0;
            PlaySoundEffect(white_candle, 0x48);
            PlayStationaryEffect(white_candle.x, white_candle.y, white_candle.z, 0x3728, 5, 0x10, 0, white_candle.realm);
            
            white_candles.append(white_candle.serial);
            SetObjProperty(altar, "white_candles", white_candles);
            
            LogInfo("ChampSpawn", "Vela branca " + progress_level + " criada");
        endif
    endif
endfunction

function ClearWhiteCandles(altar)
    var white_candles := GetObjProperty(altar, "white_candles");
    if (TypeOf(white_candles) == "Array")
        foreach serial in white_candles
            var candle := SystemFindObjectBySerial(serial);
            if (candle)
                DestroyItem(candle);
            endif
        endforeach
    endif
    EraseObjProperty(altar, "white_candles");
endfunction

function ClearBossAreaItems(altar)
    var boss_area_items := GetObjProperty(altar, "boss_area_items");
    if (TypeOf(boss_area_items) == "Array")
        var items_removed := 0;
        foreach serial in boss_area_items
            var item := SystemFindObjectBySerial(serial);
            if (item)
                DestroyItem(item);
                items_removed := items_removed + 1;
            endif
        endforeach
        if (items_removed > 0)
            LogInfo("ChampSpawn", items_removed + " fogos especiais removidos");
        endif
    endif
    EraseObjProperty(altar, "boss_area_items");
endfunction

function RestoreChampionState(altar)
    // Primeiro, limpa tudo para começar limpo
    // CleanupOrphanedItems(altar); // Removida - usar versão do waves.inc se necessário
    
    // Verifica se há caveiras próximas que indiquem progresso anterior
    var nearby_items := ListItemsNearLocation(altar.x, altar.y, altar.z, 10, altar.realm);
    var red_skulls_found := array{};
    
    foreach item in nearby_items
        if (item.objtype == 0x1854 && item.color == 0x26)
            red_skulls_found.append(item.serial);
        endif
    endforeach
    
    if (red_skulls_found.size() > 0)
        SetObjProperty(altar, "skulls", red_skulls_found);
        SetObjProperty(altar, "#current_level", red_skulls_found.size());
        
        PrintTextAbove(altar, "* Estado anterior restaurado: Onda " + red_skulls_found.size() + " *");
        LogInfo("ChampSpawn", "Progresso restaurado: Onda " + red_skulls_found.size());
        return red_skulls_found.size();
    else
        return 0;
    endif
endfunction

function RemoveLastRedSkull(altar)
    var skulls := GetObjProperty(altar, "skulls");
    if (TypeOf(skulls) == "Array" && skulls.size() > 0)
        var last_skull_serial := skulls[skulls.size()];
        var last_skull := SystemFindObjectBySerial(last_skull_serial);
        if (last_skull)
            DestroyItem(last_skull);
            skulls.erase(skulls.size());
            SetObjProperty(altar, "skulls", skulls);
            LogWarning("ChampSpawn", "Caveira vermelha removida - retrocesso para nível " + skulls.size());
        endif
    endif
endfunction

function CreateBossAreaItems(altar)
    var boss_area_positions := array{
        array{-1, -1}, array{0, -1}, array{1, -1},
        array{-1,  0},               array{1,  0},
        array{-1,  1}, array{0,  1}, array{1,  1}
    };
    
    var created_items := array{};
    
    foreach pos in boss_area_positions
        var x := altar.x + pos[1];
        var y := altar.y + pos[2];
        var z := altar.z;
        
        var boss_item := CreateItemAtLocation(x, y, z, 0x6d70, 1, altar.realm);
        
        if (boss_item)
            boss_item.movable := 0;
            created_items.append(boss_item.serial);
        endif
    endforeach
    
    SetObjProperty(altar, "boss_area_items", created_items);
    
    if (created_items.size() > 0)
        LogInfo("ChampSpawn", created_items.size() + " itens especiais criados na área do boss");
    endif
endfunction

function VerifyAllSkulls(altar, total_waves)
    LogInfo("ChampSpawn", "Verificando todas as caveiras - deveriam existir " + total_waves + " caveiras");
    
    var skulls_prop := GetObjProperty(altar, "skulls");
    var skulls := (TypeOf(skulls_prop) == "Array") ? skulls_prop : array{};
    
    LogInfo("ChampSpawn", "Caveiras atualmente existentes: " + skulls.size() + "/" + total_waves);
    
    for wave := 1 to total_waves
        if (skulls.size() < wave)
            LogWarning("ChampSpawn", "Caveira " + wave + " está faltando! Criando agora...");
            SetObjProperty(altar, "#current_level", wave);
            CreateRedSkull(altar);
            
            skulls_prop := GetObjProperty(altar, "skulls");
            skulls := (TypeOf(skulls_prop) == "Array") ? skulls_prop : array{};
        endif
    endfor
    
    LogInfo("ChampSpawn", "Verificação concluída - Total de caveiras: " + skulls.size());
endfunction

function GetRedSkullLocation(altar, index)
    var x, y;
    
    // Lógica para um quadrado 5x5 perfeito (16 caveiras)
    if(index >= 1 && index <= 5)        // Lado Superior (5 caveiras, incluindo cantos)
        x := (index - 1) - 2;
        y := -2; 
    elseif(index >= 6 && index <= 8)    // Lado Direito (3 caveiras, sem cantos)
        x := 2; 
        y := (index - 6) - 1;
    elseif(index >= 9 && index <= 13)   // Lado Inferior (5 caveiras, incluindo cantos)
        x := 2 - (index - 9); 
        y := 2; 
    elseif(index >= 14 && index <= 16)  // Lado Esquerdo (3 caveiras, sem cantos)
        x := -2; 
        y := 1 - (index - 14);
    else
        // Posições de fallback caso o número de ondas seja diferente
        case(index)
            17: x := -1; y := -1;
            18: x := 1; y := -1;
            19: x := 1; y := 1;
            20: x := -1; y := 1;
            default: x:=0; y:=0;
        endcase
    endif
    
    // Altura ajustada para os degraus da pirâmide (Z+1 relativo ao altar)
    var final_z := altar.z + 1;
    
    LogDebug("ChampSpawn", "Caveira " + index + " calculada para offset (" + x + "," + y + ") = posição final (" + (altar.x + x) + "," + (altar.y + y) + "," + final_z + ")");
    return array(altar.x + x, altar.y + y, final_z);
endfunction

function CountPlayers(altar, radius)
    var players := 0;
    foreach mob in ListMobilesNearLocation(altar.x, altar.y, altar.z, radius, altar.realm)
        if (mob.acctname)
            players += 1;
        endif
    endforeach
    return players;
endfunction

// === SISTEMA DE TOLERÂNCIA ===

// Função unificada para gerenciar tolerância de jogadores
// Retorna: 1 se jogadores retornaram, 0 se tolerância expirou
function WaitForPlayersWithTolerance(altar, context, tolerance_seconds)
    if (!tolerance_seconds)
        tolerance_seconds := 300; // Default: 5 minutos
    endif
    
    var spawn_cfg := GetObjProperty(altar, "ChampionConfig");
    if (!spawn_cfg || !spawn_cfg.player_radius)
        LogError("ChampSpawn", "Configuração de spawn inválida para tolerância");
        return 0;
    endif
    
    if (CountPlayers(altar, spawn_cfg.player_radius) > 0)
        return 1; // Já tem jogadores, não precisa de tolerância
    endif
    
    LogWarning("ChampSpawn", "Nenhum jogador detectado (" + context + ") - Iniciando tolerância de " + (tolerance_seconds/60) + " minutos");
    PrintTextAbove(altar, "* Aguardando jogadores retornarem... *");
    
    var tolerance_start := ReadGameClock();
    var last_warning := 0;
    var warning_interval := 120; // Aviso a cada 2 minutos
    
    while (CountPlayers(altar, spawn_cfg.player_radius) == 0)
        sleep(10); // Verifica a cada 10 segundos
        
        var elapsed := ReadGameClock() - tolerance_start;
        var remaining := tolerance_seconds - elapsed;
        
        if (remaining <= 0)
            LogWarning("ChampSpawn", "Tolerância expirada (" + context + ") - " + (tolerance_seconds/60) + " minutos sem jogadores");
            PrintTextAbove(altar, "* Tolerância expirada - " + context + " cancelado *");
            return 0;
        endif
        
        // Avisos periódicos para não fazer spam
        if (elapsed - last_warning >= warning_interval)
            var remaining_minutes := CInt(remaining / 60);
            if (remaining_minutes > 0)
                PrintTextAbove(altar, "* " + remaining_minutes + " minutos restantes de tolerância *");
                last_warning := elapsed;
            endif
        endif
    endwhile
    
    LogInfo("ChampSpawn", "Jogadores retornaram (" + context + ") - Continuando...");
    PrintTextAbove(altar, "* Jogadores retornaram! *");
    return 1;
endfunction

function CreateRedSkull(altar)
    var level := CInt(GetObjProperty(altar, "#current_level"));
    if (!level)
        LogError("ChampSpawn", "CreateRedSkull chamada sem nível válido");
        return;
    endif

    var skulls_prop := GetObjProperty(altar, "skulls");
    var skulls := (TypeOf(skulls_prop) == "Array") ? skulls_prop : array{};

    if (skulls.size() < level)
        var location := GetRedSkullLocation(altar, level);
        
        var red_skull := CreateItemAtLocation(location[1], location[2], location[3], 0x1854, 1, altar.realm);
        
        if(red_skull)
            red_skull.color := 0x26;
            red_skull.movable := 0;
            PlaySoundEffect(red_skull, 0x29);
            PlayStationaryEffect(red_skull.x, red_skull.y, red_skull.z, 0x3728, 7, 0x10, 0, red_skull.realm);
            
            skulls.append(red_skull.serial);
            SetObjProperty(altar, "skulls", skulls);
            
            LogInfo("ChampSpawn", "Caveira " + level + " criada");
        else
            LogError("ChampSpawn", "FALHA ao criar caveira " + level);
        endif
    endif
endfunction
