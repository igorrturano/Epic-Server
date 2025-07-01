use uo;
use os;
use util;
use cfgfile;
use math;

include ":epicspawn:epicBoss";
include ":faccao:include/logging";

function ShowBossTimer(altar)
    LogInfo("ChampSpawn", "Iniciando timer de 100 segundos para o boss no altar " + altar.serial);
    
    // NOVO: Muda a cor do altar para 2709 quando a contagem começa
    altar.color := 2709;
    LogInfo("ChampSpawn", "Altar " + altar.serial + " mudou para cor 2709 - Contagem regressiva iniciada!");
    
    PrintTextAbove(altar, "* Todas as ondas foram derrotadas! *");
    PrintTextAbove(altar, "* O Campeão desperta em 100 segundos... *");
    
    var countdown := 100;
    while (countdown >= 1)
        if (countdown % 10 == 0 || countdown <= 10)
            if (countdown > 10)
                PrintTextAbove(altar, "Campeão desperta em " + countdown + " segundos...");
                LogDebug("ChampSpawn", "Timer do boss: " + countdown + " segundos restantes");
            else
                PrintTextAbove(altar, "* " + countdown + " *");
                PlaySoundEffect(altar, 0x244);
                LogDebug("ChampSpawn", "Contagem final: " + countdown);
            endif
        endif
        
        sleep(1);
        
        var spawn_cfg := GetObjProperty(altar, "ChampionConfig");
        if (CountPlayers(altar, spawn_cfg.player_radius) == 0)
            LogWarning("ChampSpawn", "Timer cancelado - nenhum jogador próximo ao altar " + altar.serial);
            PrintTextAbove(altar, "* Timer cancelado - nenhum jogador por perto *");
            
            // NOVO: Restaura cor original se timer for cancelado
            altar.color := 0;
            LogInfo("ChampSpawn", "Cor do altar restaurada para original (timer cancelado)");
            
            return 0;
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
            
            LogInfo("ChampSpawn", "Vela branca " + progress_level + " criada (progresso: " + (progress_level * 25) + "%)");
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
    LogDebug("ChampSpawn", "Todas as velas brancas removidas do altar " + altar.serial);
endfunction

function ClearBossAreaItems(altar)
    var boss_area_items := GetObjProperty(altar, "boss_area_items");
    if (TypeOf(boss_area_items) == "Array")
        foreach serial in boss_area_items
            var item := SystemFindObjectBySerial(serial);
            if (item)
                DestroyItem(item);
                LogDebug("ChampSpawn", "Fogo 0x6d70 destruído - Serial: " + serial);
            endif
        endforeach
        LogInfo("ChampSpawn", boss_area_items.size() + " fogos especiais removidos do altar " + altar.serial);
    endif
    EraseObjProperty(altar, "boss_area_items");
endfunction

function CleanupOrphanedItems(altar)
    LogInfo("ChampSpawn", "=== LIMPEZA DE ITENS ÓRFÃOS ===");
    LogInfo("ChampSpawn", "Verificando área ao redor do altar " + altar.serial + " para itens órfãos...");
    
    var cleanup_radius := 25; // Raio generoso para capturar tudo
    var items_removed := 0;
    
    // Lista todos os itens próximos ao altar
    var nearby_items := ListItemsNearLocation(altar.x, altar.y, altar.z, cleanup_radius, altar.realm);
    
    foreach item in nearby_items
        var should_remove := 0;
        var item_type := "";
        
        // Verifica se é uma caveira vermelha órfã
        if (item.objtype == 0x1854 && item.color == 0x26)
            should_remove := 1;
            item_type := "caveira vermelha";
        endif
        
        // Verifica se é uma vela branca órfã
        if (item.objtype == 0x142C)
            should_remove := 1;
            item_type := "vela branca";
        endif
        
        // Verifica se é um fogo órfão
        if (item.objtype == 0x6d70)
            should_remove := 1;
            item_type := "fogo especial";
        endif
        
        // Remove itens órfãos encontrados
        if (should_remove)
            LogDebug("ChampSpawn", "Removendo " + item_type + " órfão - Serial: " + item.serial + " em " + item.x + "," + item.y);
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
    
    // NOVO: Restaura cor original do altar
    altar.color := 0;
    LogInfo("ChampSpawn", "Cor do altar restaurada durante limpeza de órfãos");
    
    LogInfo("ChampSpawn", "Limpeza concluída: " + items_removed + " itens órfãos removidos");
    
    if (items_removed > 0)
        PrintTextAbove(altar, "* Energia residual dissipada (" + items_removed + " itens) *");
        PlaySoundEffect(altar, 0x1F8);
    endif
endfunction

function RestoreChampionState(altar)
    LogInfo("ChampSpawn", "=== RESTAURAÇÃO DE ESTADO ===");
    
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
        LogInfo("ChampSpawn", "Encontradas " + red_skulls_found.size() + " caveiras vermelhas - Restaurando progresso...");
        
        // Salva as caveiras encontradas
        SetObjProperty(altar, "skulls", red_skulls_found);
        SetObjProperty(altar, "#current_level", red_skulls_found.size());
        
        PrintTextAbove(altar, "* Estado anterior restaurado: Onda " + red_skulls_found.size() + " *");
        return red_skulls_found.size();
    else
        LogInfo("ChampSpawn", "Nenhum progresso anterior encontrado - Começando do zero");
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
        LogError("ChampSpawn", "CreateRedSkull chamada sem nível válido para altar " + altar.serial);
        return;
    endif

    var skulls_prop := GetObjProperty(altar, "skulls");
    var skulls := (TypeOf(skulls_prop) == "Array") ? skulls_prop : array{};
    
    LogDebug("ChampSpawn", "CreateRedSkull - Nível: " + level + ", Caveiras existentes: " + skulls.size());

    if (skulls.size() < level)
        var location := GetRedSkullLocation(altar, level);
        LogDebug("ChampSpawn", "Criando caveira " + level + " na posição " + location[1] + "," + location[2] + "," + location[3]);
        
        var red_skull := CreateItemAtLocation(location[1], location[2], location[3], 0x1854, 1, altar.realm);
        
        if(red_skull)
            red_skull.color := 0x26;
            red_skull.movable := 0;
            PlaySoundEffect(red_skull, 0x29);
            PlayStationaryEffect(red_skull.x, red_skull.y, red_skull.z, 0x3728, 7, 0x10, 0, red_skull.realm);
            
            skulls.append(red_skull.serial);
            SetObjProperty(altar, "skulls", skulls);
            
            LogInfo("ChampSpawn", "Caveira " + level + " criada com sucesso - Serial: " + red_skull.serial + " na posição " + red_skull.x + "," + red_skull.y);
        else
            LogError("ChampSpawn", "FALHA ao criar caveira " + level + " na posição " + location[1] + "," + location[2] + "," + location[3]);
        endif
    else
        LogDebug("ChampSpawn", "Caveira " + level + " já existe - pulando criação");
    endif
endfunction

function SpawnWaveWithTracking(altar, mob_templates_array, amount_to_spawn)
    var spawned_serials := array{};
    
    if (mob_templates_array.size() < 1 || !amount_to_spawn)
        LogError("ChampSpawn", "Configuração de Wave inválida - Templates: " + mob_templates_array.size() + ", Amount: " + amount_to_spawn);
        return spawned_serials;
    endif
    
    LogInfo("ChampSpawn", "Iniciando spawn de " + amount_to_spawn + " criaturas ao redor do altar " + altar.x + "," + altar.y + "," + altar.z);
    var spawned_creatures := 0;
    var failed_attempts := 0;
    var max_failures := amount_to_spawn * 3; // Aumentei para dar mais tentativas
    
    while (spawned_creatures < amount_to_spawn && failed_attempts < max_failures)
        var loc := GetSpawnLocation(altar);
        LogDebug("ChampSpawn", "GetSpawnLocation retornou: " + loc[1] + "," + loc[2] + "," + loc[3]);
        
        var template := mob_templates_array[RandomInt(mob_templates_array.size()) + 1];
        
        LogDebug("ChampSpawn", "Tentando criar " + template + " na posição " + loc[1] + "," + loc[2] + "," + loc[3]);
        var creature := CreateNpcWithPrefix(template, loc[1], loc[2], loc[3], altar.realm);
        
        if (creature && !creature.errortext)
            SetObjProperty(creature, "#AltarSerial", altar.serial);
            spawned_serials.append(creature.serial);
            spawned_creatures += 1;
            
            LogInfo("ChampSpawn", "Criatura " + spawned_creatures + "/" + amount_to_spawn + " spawnada: " + template + " (" + creature.serial + ") em " + creature.x + "," + creature.y + " (planejado: " + loc[1] + "," + loc[2] + ")");
            
            if (template == "troll" || template == "ettin")
                sleepms(100);
            else
                sleepms(50);
            endif
        else
            failed_attempts += 1;
            LogError("ChampSpawn", "Falha ao criar NPC com template '" + template + "' na localização " + loc[1] + "," + loc[2] + " (tentativa " + failed_attempts + "/" + max_failures + ")");
            sleepms(20);
        endif
    endwhile
    
    if (spawned_creatures < amount_to_spawn)
        LogWarning("ChampSpawn", "Onda incompleta: " + spawned_creatures + "/" + amount_to_spawn + " criaturas spawnadas após " + failed_attempts + " falhas");
    else
        LogInfo("ChampSpawn", spawned_creatures + " criaturas spawnadas para tracking no altar " + altar.serial);
    endif
    
    return spawned_serials;
endfunction

function SpawnChampion(altar, boss_template)
    var loc := GetSpawnLocation(altar);
    var creature := CreateNpcWithPrefix(boss_template, loc[1], loc[2], loc[3], altar.realm);
    
    if (creature && !creature.errortext)
        PlaySoundEffect(creature, 0x2F3);
        SetObjProperty(altar, "champ_serial", creature.serial);
        
        LogInfo("ChampSpawn", "Boss criado - Serial: " + creature.serial + ", Template: " + boss_template + ", Local: " + creature.x + "," + creature.y + "," + creature.z);
        
        SetupBossDrop(creature);
        
        // Adiciona os itens 0x6d70 na área interna do altar quando o boss aparece
        CreateBossAreaItems(altar);
        
        return creature;
    else
        LogError("ChampSpawn", "Falha ao criar o CHEFE com template '" + boss_template + "' na localização " + loc[1] + "," + loc[2]);
        return 0;
    endif
endfunction

function CreateBossAreaItems(altar)
    LogInfo("ChampSpawn", "Criando itens especiais na área do boss (0x6d70)...");
    
    // Área interna 3x3 ao redor do altar, excluindo o centro (0,0)
    var boss_area_positions := array{
        array{-1, -1}, array{0, -1}, array{1, -1},  // Linha superior
        array{-1,  0},               array{1,  0},  // Linha central (sem centro)
        array{-1,  1}, array{0,  1}, array{1,  1}   // Linha inferior
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
            LogDebug("ChampSpawn", "Item 0x6d70 criado em " + x + "," + y + "," + z);
        else
            LogError("ChampSpawn", "Falha ao criar item 0x6d70 em " + x + "," + y + "," + z);
        endif
    endforeach
    
    // Salva os serials dos itens criados para limpeza posterior
    SetObjProperty(altar, "boss_area_items", created_items);
    
    LogInfo("ChampSpawn", created_items.size() + " itens especiais criados na área do boss");
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
    LogInfo("ChampSpawn", "=== INICIANDO CRIAÇÃO DO TOTEM ===");
    LogInfo("ChampSpawn", "Local do altar: " + altar.x + "," + altar.y + "," + altar.z);
    LogInfo("ChampSpawn", "Realm: " + realm);
    
    LogInfo("ChampSpawn", "Tentativa 1: Criando via :faccao:0xA102");
    var totem := CreateItemAtLocation(altar.x, altar.y, altar.z, ":faccao:0xA102", 1, realm);
    
    if (totem)
        totem.movable := 1;
        totem.weight := 10;
        
        PrintTextAbove(totem, "* Um Totem de Aquisição aparece! *");
        PlaySoundEffect(totem, 0x1F5);
        PlayStationaryEffect(totem.x, totem.y, totem.z, 0x3728, 10, 0x10, 0, realm);
        
        LogInfo("ChampSpawn", "SUCCESS: Totem criado via :faccao: - Serial: " + totem.serial);
        return totem;
    else
        LogError("ChampSpawn", "FALHA: Tentativa 1 falhou");
        
        LogInfo("ChampSpawn", "Tentativa 2: Criando via 0xA102 direto");
        var fallback_totem := CreateItemAtLocation(altar.x, altar.y, altar.z, 0xA102, 1, realm);
        
        if (fallback_totem)
            LogInfo("ChampSpawn", "SUCCESS: Totem criado via fallback - Serial: " + fallback_totem.serial);
            fallback_totem.movable := 1;
            fallback_totem.weight := 10;
            PrintTextAbove(fallback_totem, "* Um Totem de Aquisição aparece! *");
            PlaySoundEffect(fallback_totem, 0x1F5);
            return fallback_totem;
        else
            LogCritical("ChampSpawn", "FALHA TOTAL: Nenhum método funcionou!");
            
            LogInfo("ChampSpawn", "Tentativa 3: Testando com item genérico");
            var test_item := CreateItemAtLocation(altar.x, altar.y, altar.z, 0x0F3F, 1, realm);
            if (test_item)
                LogInfo("ChampSpawn", "Item de teste criado - problema específico com o ItemID 0xA102");
                PrintTextAbove(test_item, "* TESTE: Item genérico criado *");
                return test_item;
            else
                LogCritical("ChampSpawn", "Nem item de teste foi criado - problema de localização!");
                return 0;
            endif
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