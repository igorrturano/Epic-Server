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
            
            // SOLUÇÃO CRÍTICA: Iniciar guardian_death_monitor como backup/alternativa
            LogInfo("ChampSpawn", "*** INICIANDO GUARDIAN_DEATH_MONITOR ***");
            var monitor_result := start_script(":faccao:spawndecampeao:guardian_death_monitor", creature);
            if (monitor_result.errortext)
                LogInfo("ChampSpawn", "AVISO: Falha ao iniciar guardian_death_monitor: " + monitor_result.errortext);
            else
                LogInfo("ChampSpawn", "✓ Guardian_death_monitor iniciado com sucesso!");
                LogInfo("ChampSpawn", "*** TOTEM TERRITORIAL SERÁ CRIADO AUTOMATICAMENTE AO MATAR O BOSS ***");
            endif
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

function CreateChampionDrop(altar, realm)
    var drop_bag := CreateItemAtLocation(altar.x, altar.y, altar.z, 0x0E76, 1, realm);
    if (!drop_bag)
        LogError("ChampSpawn", "Falha ao criar bag de drop do Champion");
        return 0;
    endif
    
    drop_bag.movable := 0;
    drop_bag.color := 0x497;
    drop_bag.name := "Champion Spawn Rewards";
    
    // Adicionar itens de recompensa
    var reward_items := array{
        struct{ objtype := 0x1F3D, amount := 1, color := 0x497 },  // Kryss especial
        struct{ objtype := 0x0F51, amount := 1, color := 0x497 },  // Dagger especial  
        struct{ objtype := 0x0EED, amount := RandomInt(50) + 50 }   // Gold
    };
    
    foreach reward in reward_items
        var item := CreateItemInContainer(drop_bag, reward.objtype, reward.amount);
        if (item && reward.color)
            item.color := reward.color;
        endif
    endforeach
    
    LogInfo("ChampSpawn", "Bag de recompensas criada no altar");
    return drop_bag;
endfunction
