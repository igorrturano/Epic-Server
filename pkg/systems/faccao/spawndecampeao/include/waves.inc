// WAVES.INC - Funções de gerenciamento de ondas e spawning
// Criado durante refatoração para separar responsabilidades

use uo;
use os;
use math;

include ":faccao:include/shared_functions";
include ":epicspawn:epicBoss";

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

function CountAliveMobs(serials_array)
    var alive_count := 0;
    if (TypeOf(serials_array) != "Array")
        return 0;
    endif
    
    foreach serial in serials_array
        var mob := SystemFindObjectBySerial(serial);
        if (mob && !mob.dead)
            alive_count += 1;
        endif
    endforeach
    
    return alive_count;
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

function SpawnChampion(altar, boss_template)
    var loc := GetSpawnLocation(altar);
    var creature := CreateNpcWithPrefix(boss_template, loc[1], loc[2], loc[3], altar.realm);
    
    if (creature && !creature.errortext)
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
            
            LogInfo("ChampSpawn", "*** BOSS CONFIGURADO COMO TERRITORIAL ***");
            LogInfo("ChampSpawn", "Boss Serial: " + creature.serial);
            LogInfo("ChampSpawn", "Territory: " + territory_name);
            LogInfo("ChampSpawn", "Altar Serial: " + altar.serial);
            LogInfo("ChampSpawn", "Props definidas: IsGuardian=1, TerritoryName=" + territory_name + ", FromChampionSpawn=1, ChampionAltar=" + altar.serial);
            LogInfo("ChampSpawn", "IMPORTANTE: Boss usará DeathScript do npcdesc.cfg para criar totem territorial");
            
            LogInfo("ChampSpawn", "Boss configurado como GUARDIÃO TERRITORIAL de: " + territory_name);
            LogInfo("ChampSpawn", "Morte será processada pelo DeathScript via POL");
            
            // SOLUÇÃO CRÍTICA: Configurar DeathScript territorial
            LogInfo("ChampSpawn", "*** CONFIGURANDO DEATHSCRIPT TERRITORIAL ***");
            creature.script := ":spawndecampeao:scripts/territorial_boss_death";
            LogInfo("ChampSpawn", "✓ DeathScript configurado: " + creature.script);
            LogInfo("ChampSpawn", "*** TOTEM TERRITORIAL SERÁ CRIADO AUTOMATICAMENTE AO MATAR O BOSS ***");
        else
            // Boss normal - apenas configurar propriedade do altar
            SetObjProperty(creature, "#AltarSerial", altar.serial);
            LogInfo("ChampSpawn", "Boss criado como champion NORMAL (não territorial)");
            LogInfo("ChampSpawn", "Usando npcdeath.src do sistema POL para death handling");
        endif
        
        SetupBossDrop(creature);
        
        return creature;
    else
        LogError("ChampSpawn", "Falha ao criar o CHEFE com template '" + boss_template + "' na localização " + loc[1] + "," + loc[2]);
        return 0;
    endif
endfunction

function SetupBossDrop(boss)
    // Configurar drop do boss
    var drop_bag := CreateItemAtLocation(boss.x, boss.y, boss.z, 0x0E76, 1, boss.realm);
    if (drop_bag)
        drop_bag.movable := 0;
        drop_bag.color := 0x497;
        SetObjProperty(boss, "drop_bag", drop_bag.serial);
        LogInfo("ChampSpawn", "Bag de drop criada para o boss");
    endif
endfunction

// FUNÇÃO REMOVIDA: CreateChampionDrop
// Sistema agora suporta apenas boss territorial que cria totem via DeathScript
// Bag de recompensas normais foi removida do sistema

// === SISTEMA TERRITORIAL - CRIAÇÃO DE TOTEM ===

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
    var rect_df := OpenDataFile(":architect:areas");
    if (!rect_df)
        LogCritical("ChampSpawn", "ERRO: Não foi possível abrir datafile :architect:areas");
        return;
    endif
    
    var elem := FindDataElem(rect_df, territory_name);
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
    
    // Configurar propriedades do totem
    var territory_id := GetElementProp(elem, "TerritoryId");
    var territory_display_name := GetElementProp(elem, "Name", territory_name);

    if (!territory_id)
        territory_id := territory_name;
    endif
    
    LogCritical("ChampSpawn", "*** CONFIGURANDO PROPRIEDADES DO TOTEM ***");
    LogCritical("ChampSpawn", "Territory ID: " + territory_id);
    LogCritical("ChampSpawn", "Territory Display Name: " + territory_display_name);
    
    SetObjProperty(totem, "TerritoryId", territory_id);
    SetObjProperty(totem, "TerritoryName", territory_display_name);
    SetObjProperty(totem, "TerritoryRect", GetElementProp(elem, "Rect", ""));
    SetObjProperty(totem, "TerritoryRealm", GetElementProp(elem, "Realm", ""));
    SetObjProperty(totem, "CreationDate", polcore().systime);
    SetObjProperty(totem, "FromBossDefeat", 1);
    
    LogCritical("ChampSpawn", "*** TOTEM CONFIGURADO COM SUCESSO ***");
    
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
            
            // Limpar TODOS os itens relacionados ao altar
            CleanupOrphanedItems(altar);
            
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

// Função para limpeza de itens órfãos
function CleanupOrphanedItems(altar)
    LogCritical("ChampSpawn", "*** INICIANDO CleanupOrphanedItems ***");
    LogCritical("ChampSpawn", "Altar para limpeza: " + altar.serial);
    
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
    endif
    
    LogCritical("ChampSpawn", "*** CleanupOrphanedItems FINALIZADA ***");
endfunction
