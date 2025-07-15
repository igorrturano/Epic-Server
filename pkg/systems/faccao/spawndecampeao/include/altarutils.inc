use uo;
use os;
use util;
use cfgfile;
use math;

include ":faccao:include/shared_functions";
include ":faccao:include/faccao_constants";
include ":epicspawn:epicBoss";

function ShowBossTimer(altar)
    LogInfo("ChampSpawn", "Iniciando timer de 100 segundos para o boss no altar " + altar.serial);
    
    // NOVO: Muda a cor do altar para 2709 quando a contagem começa
    altar.color := 2709;
    
    PrintTextAbove(altar, "* Todas as ondas foram derrotadas! *");
    PrintTextAbove(altar, "* O Campeão desperta em 100 segundos... *");
    
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
    
    // NOVO: Limpa itens órfãos de reinicializações anteriores
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
    
    // NOVO: Restaura a cor original do altar
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
    
    if (white_candles.size() < progress_level && progress_level <= 4) // NOVO: Limita a 4 velas
        var candle_x, candle_y;
        case(progress_level)
            1: candle_x := 0; candle_y := -1;   // Norte
            2: candle_x := 1; candle_y := 0;    // Leste  
            3: candle_x := 0; candle_y := 1;    // Sul
            4: candle_x := -1; candle_y := 0;   // Oeste
            // Removido: 5: candle_x := 0; candle_y := 0; (centro)
        endcase
        
        var white_candle := CreateItemAtLocation(altar.x + candle_x, altar.y + candle_y, altar.z, 0x142C, 1, altar.realm);
        
        if (white_candle)
            white_candle.movable := 0;
            PlaySoundEffect(white_candle, 0x47);
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

function CleanupOrphanedItems(altar)
    var cleanup_radius := 25;
    var items_removed := 0;
    
    var nearby_items := ListItemsNearLocation(altar.x, altar.y, altar.z, cleanup_radius, altar.realm);
    
    foreach item in nearby_items
        var should_remove := 0;
        
        // Verifica se é uma caveira vermelha órfã
        if (item.objtype == 0x1854 && item.color == 0x26)
            should_remove := 1;
        endif
        
        // Verifica se é uma vela branca órfã
        if (item.objtype == 0x142C)
            should_remove := 1;
        endif
        
        // Verifica se é um fogo órfão
        if (item.objtype == 0x6d70)
            should_remove := 1;
        endif
        
        if (should_remove)
            DestroyItem(item);
            items_removed := items_removed + 1;
        endif
    endforeach
    
    // Limpa propriedades órfãs do altar
    EraseObjProperty(altar, "skulls");
    EraseObjProperty(altar, "white_candles");
    EraseObjProperty(altar, "boss_area_items");
    EraseObjProperty(altar, "#current_level");
    EraseObjProperty(altar, "#current_wave_kills");
    
    // Restaura cor original do altar
    altar.color := 0;
    
    if (items_removed > 0)
        LogInfo("ChampSpawn", "Limpeza concluída: " + items_removed + " itens órfãos removidos");
        PrintTextAbove(altar, "* Energia residual dissipada (" + items_removed + " itens) *");
        PlaySoundEffect(altar, 0x1F8);
    endif
endfunction

function RestoreChampionState(altar)
    // Primeiro, limpa tudo para começar limpo
    CleanupOrphanedItems(altar);
    
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

function SpawnWaveWithTracking(altar, mob_templates_array, amount_to_spawn)
    var spawned_serials := array{};
    
    if (mob_templates_array.size() < 1 || !amount_to_spawn)
        LogError("ChampSpawn", "Configuração de Wave inválida");
        return spawned_serials;
    endif
    
    var spawned_creatures := 0;
    var failed_attempts := 0;
    var max_failures := amount_to_spawn * 3;
    
    while (spawned_creatures < amount_to_spawn && failed_attempts < max_failures)
        var loc := GetSpawnLocation(altar);
        var template := mob_templates_array[RandomInt(mob_templates_array.size()) + 1];
        
        var creature := CreateNpcWithPrefix(template, loc[1], loc[2], loc[3], altar.realm);
        
        if (creature && !creature.errortext)
            SetObjProperty(creature, "#AltarSerial", altar.serial);
            spawned_serials.append(creature.serial);
            spawned_creatures += 1;
            
            if (template == "troll" || template == "ettin")
                sleepms(100);
            else
                sleepms(50);
            endif
        else
            failed_attempts += 1;
            sleepms(20);
        endif
    endwhile
    
    if (spawned_creatures < amount_to_spawn)
        LogWarning("ChampSpawn", "Onda incompleta: " + spawned_creatures + "/" + amount_to_spawn + " criaturas");
    endif
    
    return spawned_serials;
endfunction

function SpawnChampion(altar, boss_template)
    LogCritical("ChampSpawn", "*** INICIANDO SpawnChampion ***");
    LogCritical("ChampSpawn", "Altar: " + altar.serial + ", Boss Template: " + boss_template);
    
    var loc := GetSpawnLocation(altar);
    LogCritical("ChampSpawn", "Localização do boss: " + loc[1] + "," + loc[2] + "," + loc[3]);
    
    var creature := CreateNpcWithPrefix(boss_template, loc[1], loc[2], loc[3], altar.realm);

    if (creature && !creature.errortext)
        LogCritical("ChampSpawn", "*** BOSS CRIADO COM SUCESSO ***");
        LogCritical("ChampSpawn", "Boss Serial: " + creature.serial);
        LogCritical("ChampSpawn", "Boss Name: " + creature.name);
        LogCritical("ChampSpawn", "Boss Template: " + boss_template);
        LogCritical("ChampSpawn", "Boss Position: " + creature.x + "," + creature.y + "," + creature.z);
        
        PlaySoundEffect(creature, 0x2F3);
        SetObjProperty(altar, "champ_serial", creature.serial);
        
        LogInfo("ChampSpawn", "Boss criado - Serial: " + creature.serial + ", Template: " + boss_template + ", Local: " + creature.x + "," + creature.y + "," + creature.z);
        
        // NOVO: Verificar se altar é territorial
        var territory_name := GetObjProperty(altar, "TerritoryName");
        LogInfo("ChampSpawn", "Verificando altar territorial - TerritoryName: " + territory_name);
        
        if (territory_name)
            // Boss territorial - DeathScript já está configurado no npcdesc.cfg
            LogInfo("ChampSpawn", "*** BOSS TERRITORIAL DETECTADO ***");
            LogInfo("ChampSpawn", "Template: " + boss_template + " (deve ter '_territorial' no nome)");
            LogInfo("ChampSpawn", "DeathScript configurado via npcdesc.cfg, não via creature.script");
            
            // Configurar boss como guardião territorial
            SetObjProperty(creature, "IsGuardian", 1);
            SetObjProperty(creature, "TerritoryName", territory_name);
            SetObjProperty(creature, "FromChampionSpawn", 1);
            SetObjProperty(creature, "ChampionAltar", altar.serial);
            SetObjProperty(creature, "NoLoot", 1);  // CRÍTICO: Desabilitar loot normal
            
            // Configurar DeathScript especial para boss territorial
            creature.script := ":faccao:spawndecampeao:scripts/territorial_boss_death";
            LogInfo("ChampSpawn", "DeathScript configurado: " + creature.script);
            
            LogInfo("ChampSpawn", "*** BOSS CONFIGURADO COMO TERRITORIAL ***");
            LogInfo("ChampSpawn", "Boss Serial: " + creature.serial);
            LogInfo("ChampSpawn", "Territory: " + territory_name);
            LogInfo("ChampSpawn", "Altar Serial: " + altar.serial);
            LogInfo("ChampSpawn", "Props definidas: IsGuardian=1, TerritoryName=" + territory_name + ", FromChampionSpawn=1, ChampionAltar=" + altar.serial);
            LogInfo("ChampSpawn", "IMPORTANTE: Boss usará DeathScript do npcdesc.cfg para criar totem territorial");
            
            LogInfo("ChampSpawn", "Boss configurado como GUARDIÃO TERRITORIAL de: " + territory_name);
            LogInfo("ChampSpawn", "Morte será processada pelo DeathScript via POL");
            
            // CRÍTICO: Boss territorial NÃO deve ter loot normal
            LogInfo("ChampSpawn", "*** BOSS TERRITORIAL - SEM LOOT NORMAL ***");
            LogInfo("ChampSpawn", "Boss territorial não receberá SetupBossDrop()");
            LogInfo("ChampSpawn", "Apenas totem territorial será criado");
            
            // SOLUÇÃO CRÍTICA: Iniciar guardian_death_monitor como backup/alternativa
            LogInfo("ChampSpawn", "*** INICIANDO GUARDIAN_DEATH_MONITOR ***");
            var monitor_result := start_script(":faccao:spawndecampeao:scripts/guardian_death_monitor", creature);
            if (monitor_result.errortext)
                LogCritical("ChampSpawn", "ERRO: Falha ao iniciar guardian_death_monitor: " + monitor_result.errortext);
                LogCritical("ChampSpawn", "PATH TENTADO: :faccao:spawndecampeao:scripts/guardian_death_monitor");
                LogCritical("ChampSpawn", "TOTEM TERRITORIAL PODE NÃO SER CRIADO!");
            else
                LogCritical("ChampSpawn", "✓ Guardian_death_monitor iniciado com sucesso!");
                LogCritical("ChampSpawn", "*** TOTEM TERRITORIAL SERÁ CRIADO AUTOMATICAMENTE AO MATAR O BOSS ***");
            endif
        else
            // Boss normal - apenas configurar propriedade do altar
            SetObjProperty(creature, "#AltarSerial", altar.serial);
            LogInfo("ChampSpawn", "Boss criado como champion NORMAL (não territorial)");
            LogInfo("ChampSpawn", "Usando npcdeath.src do sistema POL para death handling");
            
            // Configurar loot apenas para boss NORMAL
            SetupBossDrop(creature);
            LogInfo("ChampSpawn", "Boss normal configurado com loot");
        endif
        
        // Adiciona os itens 0x6d70 na área interna do altar quando o boss aparece
        CreateBossAreaItems(altar);
        
        return creature;
    else
        LogError("ChampSpawn", "Falha ao criar o CHEFE com template '" + boss_template + "' na localização " + loc[1] + "," + loc[2]);
        return 0;
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

function SetupBossDrop(boss)
    SetObjProperty(boss, "#ChampionBoss", 1);
    SetObjProperty(boss, "#DropItem", 0xA102);
    
    var is_champ := GetObjProperty(boss, "#ChampionBoss");
    var drop_item := GetObjProperty(boss, "#DropItem");
    
    LogDebug("ChampSpawn", "Boss " + boss.serial + " (" + boss.name + ") configurado para drop:");
    LogDebug("ChampSpawn", "  - ChampionBoss: " + is_champ + ", DropItem: 0x" + Hex(drop_item));
endfunction

function CreateChampionDrop(altar, realm)
    // Verificar se o altar é territorial
    var territory_name := GetObjProperty(altar, "TerritoryName");
    var is_territorial := GetObjProperty(altar, "IsTerritorChampion");
    
    if (territory_name && is_territorial)
        LogInfo("ChampSpawn", "Altar territorial - Totem será criado pelo monitor territorial");
        return 1;
    endif
    
    var totem := CreateItemAtLocation(altar.x, altar.y, altar.z, ":faccao:0xA102", 1, realm);
    
    if (totem)
        totem.movable := 1;
        totem.weight := 10;
        
        PrintTextAbove(totem, "* Um Totem de Aquisição aparece! *");
        PlaySoundEffect(totem, 0x1F5);
        PlayStationaryEffect(totem.x, totem.y, totem.z, 0x3728, 10, 0x10, 0, realm);
        
        LogInfo("ChampSpawn", "Totem criado - Serial: " + totem.serial);
        return totem;
    else
        var fallback_totem := CreateItemAtLocation(altar.x, altar.y, altar.z, 0xA102, 1, realm);
        
        if (fallback_totem)
            fallback_totem.movable := 1;
            fallback_totem.weight := 10;
            PrintTextAbove(fallback_totem, "* Um Totem de Aquisição aparece! *");
            PlaySoundEffect(fallback_totem, 0x1F5);
            LogInfo("ChampSpawn", "Totem criado via fallback - Serial: " + fallback_totem.serial);
            return fallback_totem;
        else
            LogError("ChampSpawn", "FALHA: Não foi possível criar totem");
            return 0;
        endif
    endif
endfunction

function CountAliveMobs(serials_array)
    var alive_count := 0;
    foreach serial in serials_array
        var mob := SystemFindObjectBySerial(serial);
        if (mob && !mob.dead)
            alive_count += 1;
        endif
    endforeach
    return alive_count;
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
    
    // Lógica CORRIGIDA para um quadrado 5x5 perfeito (16 caveiras)
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

// FUNÇÃO CORRIGIDA COM MÚLTIPLAS MELHORIAS
function GetSpawnLocation(altar)
    var spawn_radius := 15; // Aumentei o raio
    var min_dist := 3;      // Diminui distância mínima
    
    // Primeiro, tenta encontrar posições válidas usando verificação mais inteligente
    for attempt := 1 to 100
        var angle := (RandomInt(628) / 100.0); // 0 a 2π radianos (approximadamente)
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
        
        sleepms(2); // Pequeno delay para não sobrecarregar
    endfor
    
    // Se não encontrou após 100 tentativas, usa grid sistemático
    LogWarning("ChampSpawn", "Tentativas aleatórias falharam, usando busca em grid...");
    
    for radius := 3 to 20
        for angle := 0 to 359
            if (angle % 15 != 0) // Verifica apenas a cada 15 graus
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

// Nova função de validação mais rápida e eficiente
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

// Mantém a função original como backup
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

// ============================================================================
// SISTEMA DE TOLERÂNCIA UNIFICADO
// ============================================================================

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

// FUNÇÃO CENTRALIZADA: Carrega configurações de spawn do settings.cfg
function LoadSpawnConfig(spawn_type)
    var cfg := ReadConfigFile(":faccao:spawndecampeao:config/settings");
    if (!cfg)
        LogError("ChampSpawn", "Erro: Não foi possível carregar settings.cfg");
        return error;
    endif
    
    var spawn_name;
    case (Lower(spawn_type))
        "cavernadosgoblins":
            spawn_name := "CavernaDosGoblins";
        "covildasaranhas":
            spawn_name := "CovilDasAranhas";
        default:
            LogError("ChampSpawn", "Tipo de spawn desconhecido: " + spawn_type);
            return error;
    endcase
    
    var spawn_cfg := FindConfigElem(cfg, spawn_name);
    if (!spawn_cfg)
        LogError("ChampSpawn", "Configuração não encontrada para: " + spawn_name);
        return error;
    endif
    
    var config := struct;
    config.+name := GetConfigString(spawn_cfg, "Name");
    config.+boss_template := GetConfigString(spawn_cfg, "BossTemplate");
    config.+cooldown := GetConfigInt(spawn_cfg, "ActivationCooldown");
    config.+player_radius := GetConfigInt(spawn_cfg, "PlayerCheckRadius");
    
    // Carregar waves
    var waves := array{};
    var i;
    for (i := 1; i <= 4; i := i + 1)
        var wave_mobs := GetConfigString(spawn_cfg, "Wave" + i + " Mobs");
        var wave_amount := GetConfigInt(spawn_cfg, "Wave" + i + " Amount");
        
        if (wave_mobs && wave_amount)
            var mob_list := SplitWords(wave_mobs);
            waves.append(struct{"mobs" := mob_list, "amount" := wave_amount});
        endif
    endfor
    
    config.+waves := waves;
    
    LogInfo("ChampSpawn", "Configuração carregada do settings.cfg - Spawn: " + spawn_name + 
           ", Boss: " + config.boss_template + ", Waves: " + len(waves));
    
    return config;
endfunction

// ============================================================================
// SISTEMA TERRITORIAL - CRIAÇÃO DE TOTEM
// ============================================================================

// Função para criar totem quando boss territorial morre
function CreateTotemOnBossDeath(territory_name, x, y, z, realm, altar_serial)
    LogCritical("ChampSpawn", "*** INICIANDO CreateTotemOnBossDeath ***");
    LogCritical("ChampSpawn", "Territory: " + territory_name);
    LogCritical("ChampSpawn", "Posição: " + x + "," + y + "," + z);
    LogCritical("ChampSpawn", "Realm: " + realm);
    LogCritical("ChampSpawn", "Altar Serial: " + altar_serial);
    
    if (!territory_name)
        LogCritical("ChampSpawn", "ERRO: Territory name é null - abortando");
        return;
    endif
    
    // Buscar o território no datafile
    LogCritical("ChampSpawn", "*** BUSCANDO TERRITÓRIO NO DATAFILE ***");
    var rect_df := SafeOpenDataFile(":architect:areas", 1);
    if (!rect_df)
        LogCritical("ChampSpawn", "ERRO: Não foi possível abrir datafile :architect:areas");
        return;
    endif
    
    var elem := SafeFindElement(rect_df, territory_name);
    if (!elem)
        LogCritical("ChampSpawn", "ERRO: Território não encontrado: " + territory_name);
        return;
    endif
    
    LogCritical("ChampSpawn", "✓ Território encontrado no datafile: " + territory_name);
    
    // Criar totem no chão
    LogCritical("ChampSpawn", "*** CRIANDO TOTEM TERRITORIAL ***");
    var totem := CreateItemAtLocation(x, y, z, 0xA102, 1, realm);
    if (!totem)
        LogCritical("ChampSpawn", "ERRO: Falha ao criar totem territorial");
        return;
    endif
    
    LogCritical("ChampSpawn", "✓ TOTEM TERRITORIAL CRIADO!");
    LogCritical("ChampSpawn", "Totem Serial: " + totem.serial);
    LogCritical("ChampSpawn", "Totem Position: " + totem.x + "," + totem.y + "," + totem.z);
    LogCritical("ChampSpawn", "Totem Objtype: 0x" + Hex(totem.objtype));
    
    // Obter ID e Nome do elemento
    var territory_id := GetElementProp(elem, TERRITORY_ID_PROP);
    var territory_display_name := GetElementProp(elem, "Name", territory_name);

    if (!territory_id)
        territory_id := territory_name;
    endif
    
    LogCritical("ChampSpawn", "*** CONFIGURANDO PROPRIEDADES DO TOTEM ***");
    LogCritical("ChampSpawn", "Territory ID: " + territory_id);
    LogCritical("ChampSpawn", "Territory Display Name: " + territory_display_name);
    
    // Configurar propriedades do totem
    SetObjProperty(totem, TERRITORY_ID_PROP, territory_id);
    SetObjProperty(totem, TERRITORY_NAME_PROP, territory_display_name);
    SetObjProperty(totem, TERRITORY_RECT_PROP, GetElementProp(elem, "Rect", ""));
    SetObjProperty(totem, TERRITORY_REALM_PROP, GetElementProp(elem, "Realm", ""));
    SetObjProperty(totem, "CreationDate", polcore().systime);
    SetObjProperty(totem, "FromBossDefeat", 1);
    
    LogCritical("ChampSpawn", "*** TOTEM CONFIGURADO COM SUCESSO ***");
    LogCritical("ChampSpawn", "Todas as propriedades territoriais definidas");
    
    // AGUARDAR 30 SEGUNDOS ANTES DA LIMPEZA COMPLETA
    LogCritical("ChampSpawn", "*** INICIANDO CONTAGEM REGRESSIVA DE 30 SEGUNDOS ***");
    for countdown := 30 to 1
        if (countdown % 10 == 0 || countdown <= 5)
            LogCritical("ChampSpawn", "Limpeza do altar em " + countdown + " segundos...");
        endif
        sleep(1);
    endfor
    
    LogCritical("ChampSpawn", "*** INICIANDO LIMPEZA COMPLETA DO ALTAR ***");
    
    // LIMPEZA COMPLETA: Deletar altar temporário E todos os itens relacionados
    if (altar_serial)
        var altar := SystemFindObjectBySerial(altar_serial);
        if (altar)
            LogCritical("ChampSpawn", "Altar encontrado: " + altar.serial + " - Iniciando limpeza");
            
            // Limpar TODOS os itens relacionados ao altar (incluindo decorativos)
            CleanupCompleteTemporarySpawn(altar);
            
            // Deletar o próprio altar por último
            LogCritical("ChampSpawn", "Deletando altar: " + altar.serial);
            DestroyItem(altar);
            
            LogCritical("ChampSpawn", "*** LIMPEZA COMPLETA FINALIZADA ***");
        else
            LogCritical("ChampSpawn", "ERRO: Altar não encontrado para limpeza: " + altar_serial);
        endif
    else
        LogCritical("ChampSpawn", "ERRO: Altar serial é null - não foi possível fazer limpeza");
    endif
endfunction

// Função para limpeza COMPLETA do spawn temporário (TODOS os itens)
function CleanupCompleteTemporarySpawn(altar)
    LogCritical("ChampSpawn", "*** INICIANDO CleanupCompleteTemporarySpawn ***");
    LogCritical("ChampSpawn", "Altar para limpeza: " + altar.serial);
    
    // Usar função centralizada do altarutils.inc
    CleanupOrphanedItems(altar);
    
    LogCritical("ChampSpawn", "*** CleanupCompleteTemporarySpawn FINALIZADA ***");
endfunction

// ============================================================================