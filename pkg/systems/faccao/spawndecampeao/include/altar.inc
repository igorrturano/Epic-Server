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
        
    var countdown := 00;
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
            var tolerance_result := WaitForPlayersWithTolerance(altar, "Timer do Boss", 60);
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
    PlaySoundEffect(altar, 4);
    PlayStationaryEffect(altar.x, altar.y, altar.z, 0x3728, 15, 0x10, 0, altar.realm);
    
    return 1;
endfunction

function CS_ActivateAltar(altar)
    SetObjProperty(altar, "#active", 1);
    
    // Limpa itens órfãos de reinicializações anteriores
    CleanupOrphanedItems(altar);
    
    PrintTextAbove(altar, "* O altar pulsa com uma energia sinistra *");
    PlaySoundEffect(altar, 0x207);
    return 1;
endfunction

// DeactivateAltar com limpeza SEGURA e ROBUSTA e SINTAXE CORRETA
function CS_DeactivateAltar(altar, cooldown)
    LogInfo("ChampSpawn", "Desativando altar " + altar.serial);
    
    EraseObjProperty(altar, "#active");
    EraseObjProperty(altar, "#completed");
    EraseObjProperty(altar, "#current_wave_kills");
    EraseObjProperty(altar, "#current_level");
    EraseObjProperty(altar, "#last_player_seen");

    LogInfo("ChampSpawn", "Limpando itens visuais pertencentes ao altar " + altar.serial);
    var items_destroyed := 0;
    
    // Procura em uma área ampla para garantir que pegamos tudo
    foreach item in ListItemsNearLocation(altar.x, altar.y, altar.z, 20, altar.realm)
        // VERIFICAÇÃO DE VÍNCULO: O item pertence a ESTE altar?
        // ==========================================================
        // INÍCIO DA CORREÇÃO DE SINTAXE
        // ==========================================================
        if (GetObjProperty(item, "AltarOwner") == altar.serial)
            // O item é nosso. Verificamos se é uma caveira ou vela para destruir.
            if (item.objtype == 0x1854 || item.objtype == 0x142C)
                DestroyItem(item);
                items_destroyed += 1;
            endif
        endif
        // ==========================================================
        // FIM DA CORREÇÃO DE SINTAXE
        // ==========================================================
        sleepms(2);
    endforeach
    
    if (items_destroyed > 0)
        LogInfo("ChampSpawn", items_destroyed + " caveiras/velas foram destruídas.");
    endif
    
    // Apaga as CProps do altar por segurança
    EraseObjProperty(altar, "skulls");
    EraseObjProperty(altar, "white_candles");
    
    // A limpeza dos fogos do boss já tem sua própria função
    ClearBossAreaItems(altar);
    
    altar.color := 0;
    
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
            
            // <-- ADIÇÃO CRÍTICA AQUI -->
            SetObjProperty(white_candle, "AltarOwner", altar.serial);

            PlaySoundEffect(white_candle, 0x48);
            PlayStationaryEffect(white_candle.x, white_candle.y, white_candle.z, 0x3728, 5, 0x10, 0, white_candle.realm);
            
            white_candles.append(white_candle.serial);
            SetObjProperty(altar, "white_candles", white_candles);
            
            LogInfo("ChampSpawn", "Vela branca " + progress_level + " criada para o altar " + altar.serial);
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

function CS_GetRedSkullLocation(altar, index)
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
    var players_validos := 0;
    LogDebug("CountPlayers", "Iniciando contagem de jogadores no raio " + radius + " ao redor do altar " + altar.serial);
    
    foreach mob in ListMobilesNearLocation(altar.x, altar.y, altar.z, radius, altar.realm)
        // Pula o loop se o mob for inválido
        if (!mob) continue; endif
        
        var is_counted := 0;
        var reason := "";
        
        // Critério 1: Tem que ser um jogador (ter uma conta)
        if (mob.acctname)
            // Critério 2: Tem que estar vivo
            if (!mob.dead)
                // Critério 3: Não pode ser um membro da staff (GM, etc.)
                if (mob.cmdlevel == 0)
                    // Critério 4: Não pode estar invisível (GM observando)
                    if (!mob.hidden)
                        players_validos += 1;
                        is_counted := 1;
                    else
                        reason := "ignorado (GM/staff invisível)";
                    endif
                else
                    reason := "ignorado (é um membro da staff, cmdlevel > 0)";
                endif
            else
                reason := "ignorado (está morto/fantasma)";
            endif
        else
            // Não é um jogador, é um NPC/monstro
            // Não precisamos logar isso para não poluir o console.
            // A menos que você queira, então descomente a linha abaixo:
            // reason := "ignorado (não é um jogador)";
        endif
        
        // Logar apenas se for um jogador, para facilitar a depuração
        if (mob.acctname)
            LogDebug("CountPlayers", "  -> Verificando: " + mob.name + " (Serial: " + mob.serial + ") - " + (is_counted ? "CONTADO" : reason));
        endif
        
    endforeach
    
    LogInfo("CountPlayers", "Contagem final: " + players_validos + " jogadores válidos encontrados.");
    return players_validos;
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
        var location := CS_GetRedSkullLocation(altar, level);
        
        var red_skull := CreateItemAtLocation(location[1], location[2], location[3], 0x1854, 1, altar.realm);
        
        if(red_skull)
            red_skull.color := 0x26;
            red_skull.movable := 0;
            
            // <-- ADIÇÃO CRÍTICA AQUI -->
            SetObjProperty(red_skull, "AltarOwner", altar.serial);
            
            PlaySoundEffect(red_skull, 0x29);
            PlayStationaryEffect(red_skull.x, red_skull.y, red_skull.z, 0x3728, 7, 0x10, 0, red_skull.realm);
            
            skulls.append(red_skull.serial);
            SetObjProperty(altar, "skulls", skulls);
            
            LogInfo("ChampSpawn", "Caveira " + level + " criada para o altar " + altar.serial);
        else
            LogError("ChampSpawn", "FALHA ao criar caveira " + level);
        endif
    endif
endfunction

function FinalCleanup(altar, spawn_cfg, reason := "completed")
    LogCritical("FinalCleanup", "==========================================================");
    LogCritical("FinalCleanup", "FUNÇÃO INICIADA (Motivo: " + reason + ")");

    // ===== BLOCO DE SEGURANÇA (A ÚNICA VERIFICAÇÃO NECESSÁRIA) =====
    if (!altar || altar.errortext)
        LogError("FinalCleanup", "ERRO FATAL: Altar inválido ou não inicializado. Abortando limpeza.");
        return;
    endif
    // ===== FIM DO BLOCO DE SEGURANÇA =====

    var altar_serial_para_log := altar.serial;
    var territory_name := GetObjProperty(altar, "TerritoryName");

    // ====================================================================
    // LÓGICA DE ABANDONO/FALHA (Reconstrução Agendada)
    // ====================================================================
    if (reason == "abandoned" || reason == "failed" || reason == "timer_cancelled")
        LogCritical("FinalCleanup", "Motivo '" + reason + "' detectado. Iniciando processo de RECONSTRUÇÃO TOTAL.");
        
        if (territory_name)
            var territory_elem := TM_FindTerritory(territory_name); // Corrigido para usar a função de busca
            if (territory_elem)
                var territory_id := GetElementProp(territory_elem, "TerritoryID");
                if (territory_id)
                    // Definir território como Selvagem
                    LogInfo("FinalCleanup", "--> PASSO A: Definindo território " + territory_id + " como Selvagem.");
                    TM_SetTerritoryFaction(territory_id, DEFAULT_FACTION);

                    // Agendar a reconstrução para daqui a 50 segundos
                    LogInfo("FinalCleanup", "--> PASSO B: Agendando reconstrução com delay de 50 segundos.");
                    var params := struct{ "territory_id" := territory_id, "delay_seconds" := 50 };
                    start_script(":faccao:spawndecampeao/scripts/rebuild_champion_task", params);
                endif
            endif
        endif
        
        // DEMOLIR a arena e o altar ATUAIS IMEDIATAMENTE
        LogInfo("FinalCleanup", "--> PASSO C: Demolindo arena e altar atuais...");
        var items_removed := CleanupArenaItems(altar.x, altar.y, altar.z, altar.realm);
        LogInfo("FinalCleanup", "    " + items_removed + " itens da arena e altar foram removidos.");
    
    // ====================================================================
    // LÓGICA DE VITÓRIA (Destruição Completa SEM Reconstrução Agendada)
    // ====================================================================
    elseif (reason == "completed")
        LogCritical("FinalCleanup", "Motivo 'completed' (vitória) detectado. Iniciando processo de DEMOLIÇÃO FINAL.");
        
        // O totem já foi criado pela função TerritorialBossDeath.
        // Nossa única tarefa aqui é demolir a arena e o altar para limpar o local.
        LogInfo("FinalCleanup", "--> Demolindo arena e altar para finalizar o evento...");
        var items_removed := CleanupArenaItems(altar.x, altar.y, altar.z, altar.realm);
        LogInfo("FinalCleanup", "    " + items_removed + " itens da arena e altar foram removidos.");
    
    // ====================================================================
    // CASO DE FALLBACK (Se um motivo desconhecido for passado)
    // ====================================================================
    else
        LogWarning("FinalCleanup", "Motivo desconhecido '" + reason + "'. Executando limpeza padrão com cooldown.");
        var cooldown_time := spawn_cfg.cooldown;
        CS_DeactivateAltar(altar, cooldown_time);
    endif

    LogInfo("FinalCleanup", "--> PASSO FINAL: Liberando a trava de PID do controlador");
    var lock_name_final := "ChampSpawn.Lock." + altar_serial_para_log;
    EraseGlobalProperty(lock_name_final);
    
    LogCritical("FinalCleanup", "Limpeza concluída. O script do controlador para o altar " + altar_serial_para_log + " será finalizado.");
    LogCritical("FinalCleanup", "==========================================================");
endfunction


/*
 * KillAndVanish(mobile)
 * 
 * Purpose: Remove um NPC do jogo de forma 100% segura, com logs detalhados
 *          para cada etapa do processo.
 */
function KillAndVanish(mobile)
    // ====================================================================
    // LOG: INÍCIO DA FUNÇÃO E VALIDAÇÃO DE PARÂMETROS
    // ====================================================================
    LogCritical("KillAndVanish", "==========================================================");
    LogCritical("KillAndVanish", "FUNÇÃO INICIADA");
    
    if (!mobile)
        LogError("KillAndVanish", "ERRO FATAL: Parâmetro 'mobile' é nulo. Abortando.");
        LogCritical("KillAndVanish", "==========================================================");
        return 0;
    endif
    if (mobile.errortext)
        LogError("KillAndVanish", "ERRO FATAL: Parâmetro 'mobile' é um erro: " + mobile.errortext + ". Abortando.");
        LogCritical("KillAndVanish", "==========================================================");
        return 0;
    endif
    if (mobile.dead)
        LogWarning("KillAndVanish", "AVISO: Mobile já está morto. Ação não necessária. Abortando.");
        LogCritical("KillAndVanish", "==========================================================");
        return 0;
    endif
    
    var mobile_serial := mobile.serial;
    LogInfo("KillAndVanish", "Alvo válido: " + mobile.name + " (Serial: " + mobile_serial + ", HP: " + mobile.hp + ")");
    LogInfo("KillAndVanish", "Alvo válido: " + mobile.name + " (Serial: " + mobile_serial + ")");

    // ==========================================================
    // >>>>> INÍCIO DA CORREÇÃO <<<<<
    // ==========================================================
    
    // ANTES de matar, nós definimos uma "bandeira" no chefe
    // para que o DeathScript saiba que isso é um abandono.
    LogCritical("KillAndVanish", "Sinalizando chefe " + mobile.serial + " como ABANDONADO antes de chamar .kill()");
    SetObjProperty(mobile, "IsBeingAbandoned", 1);
    
    // ==========================================================
    // >>>>> FIM DA CORREÇÃO <<<<<
    // ============
    // ====================================================================
    // PASSO 1: MOVER O NPC PARA O LIMBO
    // ====================================================================
    LogInfo("KillAndVanish", "--> PASSO 1: Mover para o limbo (10,10,10)");
    var move_result := MoveObjectToLocation(mobile, 10, 10, 10, mobile.realm, MOVEOBJECT_FORCELOCATION);
    if (move_result.errortext)
        LogError("KillAndVanish", "    FALHA ao mover: " + move_result.errortext + ". O processo continuará mesmo assim.");
    else
        LogInfo("KillAndVanish", "    SUCESSO ao mover.");
    endif

    // ====================================================================
    // PASSO 2: INICIAR O PROCESSO DE MORTE
    // ====================================================================
    LogInfo("KillAndVanish", "--> PASSO 2: Ordenar morte com a função de motor de jogo .kill()");
    mobile.kill();
    LogInfo("KillAndVanish", "    Comando .kill() executado. O processo de morte foi iniciado pelo motor do jogo.");

    // ====================================================================
    // PASSO 3: ESPERA ATIVA PELA TRANSFORMAÇÃO EM CORPO (ITEM)
    // ====================================================================
    LogInfo("KillAndVanish", "--> PASSO 3: Espera ativa pela transformação em corpo (item)");
    var corpse := 0;
    var attempts := 15; // Tentar por no máximo 3 segundos (15 * 200ms)
    var i;
    for (i := 1; i <= attempts; i := i + 1)
        LogDebug("KillAndVanish", "    [Tentativa " + i + "/" + attempts + "]");
        var obj_after_death := SystemFindObjectBySerial(mobile_serial);
        
        if (!obj_after_death)
            LogInfo("KillAndVanish", "    VERIFICAÇÃO: Objeto não encontrado. Provavelmente já foi limpo pelo sistema. SUCESSO.");
            corpse := 0; // Garante que não tentaremos destruir
            break;
        endif
        
        LogDebug("KillAndVanish", "    OBJETO ENCONTRADO: Serial=" + obj_after_death.serial + ", ObjType=" + Hex(obj_after_death.objtype) + ", Graphic=" + Hex(obj_after_death.graphic));
        
        if (obj_after_death.IsA(POLCLASS_ITEM))
            LogInfo("KillAndVanish", "    VERIFICAÇÃO: Objeto é um ITEM. SUCESSO! É um corpo. Prosseguindo para a destruição.");
            corpse := obj_after_death;
            break;
        else
            LogDebug("KillAndVanish", "    VERIFICAÇÃO: Objeto ainda NÃO é um item (é um " + TypeOf(obj_after_death) + "). Esperando mais 200ms...");
        endif
        
        sleepms(200);
    endfor

    // ====================================================================
    // PASSO 4: DESTRUIR O CORPO, SE ENCONTRADO
    // ====================================================================
    if (corpse)
        LogInfo("KillAndVanish", "--> PASSO 4: Destruir o corpo");
        var destroy_result := DestroyItem(corpse);
        if (destroy_result.errortext)
            LogError("KillAndVanish", "    FALHA ao destruir: " + destroy_result.errortext);
        else
            LogInfo("KillAndVanish", "    SUCESSO ao destruir.");
        endif
    elseif (i > attempts)
        LogError("KillAndVanish", "    FALHA: Loop de verificação terminou sem encontrar um corpo (item). O objeto pode ter ficado preso no estado 'morrendo'.");
    endif

    // ====================================================================
    // PASSO 5: VERIFICAÇÃO FINAL
    // ====================================================================
    LogInfo("KillAndVanish", "--> PASSO 5: Verificação final no mundo");
    var final_check := SystemFindObjectBySerial(mobile_serial);
    if (!final_check)
        LogInfo("KillAndVanish", "    SUCESSO: Objeto com serial " + mobile_serial + " não existe mais.");
        LogCritical("KillAndVanish", "FUNÇÃO FINALIZADA - Resultado: SUCESSO");
        LogCritical("KillAndVanish", "==========================================================");
        return 1;
    else
        LogError("KillAndVanish", "    FALHA: Objeto com serial " + mobile_serial + " ainda existe.");
        LogCritical("KillAndVanish", "FUNÇÃO FINALIZADA - Resultado: FALHA");
        LogCritical("KillAndVanish", "==========================================================");
        return 0;
    endif
endfunction




function GetSpawnLocation(altar)
    var spawn_radius := 15;
    var min_dist := 3;
    
    // Primeiro, tenta encontrar posições válidas usando verificação mais inteligente
    for attempt := 1 to 100
        var angle := (RandomInt(628) / 100.0); // 0 a 2π radianos
        var dist := min_dist + (RandomInt(spawn_radius - min_dist));
        
        var x_offset := CInt(dist * Sin(angle));
        var y_offset := CInt(dist * Cos(angle));
        
        var x := altar.x + x_offset;
        var y := altar.y + y_offset;
        var z := altar.z - 5; // Altura do chão da arena
        
        // Verificação mais rápida e eficiente
        if (IsValidSpawnLocationFast(x, y, z, altar.realm))
            LogDebug("ChampSpawn", "Local de spawn encontrado em " + attempt + " tentativas: " + x + "," + y + "," + z);
            return array{x, y, z};
        endif
        
        sleepms(2);
    endfor
    
    // Se não encontrou após 100 tentativas, usa grid sistemático
    LogWarning("ChampSpawn", "Tentativas aleatórias falharam, usando busca em grid...");
    
    for radius := 3 to 20
        for angle := 0 to 359
            if (angle % 15 != 0)
                continue;
            endif
            
            var rad := angle * 0.0174533; // Converte para radianos
            var x := altar.x + CInt(radius * Sin(rad));
            var y := altar.y + CInt(radius * Cos(rad));
            var z := altar.z - 5;
            
            if (IsValidSpawnLocationFast(x, y, z, altar.realm))
                LogDebug("ChampSpawn", "Local encontrado via grid: " + x + "," + y + "," + z);
                return array{x, y, z};
            endif
        endfor
    endfor
    
    // Último recurso: posições fixas conhecidas como válidas
    LogWarning("ChampSpawn", "Usando posição de fallback final...");
    var fallback_positions := array{
        array{altar.x + 10, altar.y, altar.z - 5},
        array{altar.x - 10, altar.y, altar.z - 5},
        array{altar.x, altar.y + 10, altar.z - 5},
        array{altar.x, altar.y - 10, altar.z - 5},
        array{altar.x + 8, altar.y + 8, altar.z - 5},
        array{altar.x - 8, altar.y - 8, altar.z - 5}
    };
    
    foreach pos in fallback_positions
        if (IsValidSpawnLocationFast(pos[1], pos[2], pos[3], altar.realm))
            LogDebug("ChampSpawn", "Usando fallback válido: " + pos[1] + "," + pos[2] + "," + pos[3]);
            return pos;
        endif
    endforeach
    
    // Se absolutamente nada funcionou, força uma posição
    LogCritical("ChampSpawn", "FORÇANDO posição de spawn - pode haver problemas!");
    return array{altar.x + 15, altar.y + 15, altar.z - 5};
endfunction

function IsValidSpawnLocationFast(x, y, z, realm)
    // Verificação básica de mapa primeiro (mais rápida)
    var mapinfo := GetMapInfo(x, y);
    if (!mapinfo)
        return 0;
    endif
    
    // Verifica se não é água (tiles de água comuns)
    var landtile := mapinfo.landtile;
    if (landtile >= 168 && landtile <= 171) // Água
        return 0;
    endif
    if (landtile >= 310 && landtile <= 311) // Mais tipos de água
        return 0;
    endif
    
    // Verifica itens estáticos próximos (mais eficiente que ListItems)
    var statics := ListStaticsAtLocation(x, y, z);
    if (statics.size() > 0)
        foreach static_item in statics
            // Verifica se é um item que bloqueia movimento
            if (static_item.z >= z && static_item.z <= z + 10)
                return 0;
            endif
        endforeach
    endif
    
    // Verificação rápida de NPCs próximos (raio menor)
    var nearby_npcs := ListMobilesNearLocation(x, y, z, 1, realm);
    if (nearby_npcs.size() > 0)
        return 0;
    endif
    
    return 1;
endfunction

function IsValidSpawnLocation(x, y, z, realm)
    var mapinfo := GetMapInfo(x, y);
    if (!mapinfo || mapinfo.landtile == error)
        return 0;
    endif
    
    if (mapinfo.landtile == 168 || mapinfo.landtile == 169)
        return 0;
    endif
    
    var nearby_npcs := ListMobilesNearLocation(x, y, z, 2, realm);
    foreach npc in nearby_npcs
        if (!npc.acctname)
            return 0;
        endif
    endforeach
    
    var nearby_items := ListItemsNearLocation(x, y, z, 1, realm);
    foreach item in nearby_items
        if (!item.movable)
            return 0;
        endif
    endforeach
    
    return 1;
endfunction
