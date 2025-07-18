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

// Função CORRIGIDA - Substitua a versão antiga por esta
// Função CORRIGIDA - Substitua a versão antiga por esta
function CreateTotemOnBossDeath(territory_id, x, y, z, realm)
    LogCritical("CreateTotem_LOG", "========================================================");
    LogCritical("CreateTotem_LOG", "INICIANDO CreateTotemOnBossDeath (v2 COM REGISTRO)");
    LogCritical("CreateTotem_LOG", "  -> Recebido Territory_ID: " + territory_id);

    if (!territory_id)
        LogError("CreateTotem_LOG", "FALHA CRÍTICA: Função chamada sem um territory_id. Abortando.");
        return;
    endif

    var elem := TM_FindTerritory(territory_id);
    if (!elem)
        LogError("CreateTotem_LOG", "FALHA: Não foi possível encontrar o elemento para o ID '" + territory_id + "'.");
        return;
    endif

    var territory_name := GetElementProp(elem, "Name", territory_id);
    var totem := CreateItemAtLocation(x, y, z, 0xA102, 1, realm);

    if (!totem)
        LogError("CreateTotem_LOG", "FALHA CRÍTICA: CreateItemAtLocation retornou nulo. O totem não foi criado.");
        return;
    endif
    
    // Define um tempo de expiração de 1 hora (3600 segundos)
    var expiration_timestamp := polcore().systime + 3600;

    LogInfo("CreateTotem_LOG", "SUCESSO: Totem criado (Serial: " + totem.serial + "). Configurando CProps...");
    SetObjProperty(totem, "TerritoryID", territory_id);
    SetObjProperty(totem, "Name", territory_name);
    SetObjProperty(totem, "Rect", GetElementProp(elem, "Rect"));
    SetObjProperty(totem, "Realm", GetElementProp(elem, "Realm"));
    SetObjProperty(totem, "FromBossDefeat", 1);
    SetObjProperty(totem, "ClaimByTimestamp", expiration_timestamp);

    // ==========================================================
    // >>>>> INÍCIO DA ADIÇÃO: REGISTRAR O TOTEM NO VIGILANTE <<<<<
    // ==========================================================
    var totem_registry_df := SafeOpenDataFile(":faccao:totem_registry", DF_CREATE);
    if (totem_registry_df)
        var totem_elem := SafeFindElement(totem_registry_df, CStr(totem.serial), DF_CREATE);
        if (totem_elem)
            SetElementProp(totem_elem, "territory_id", territory_id);
            SetElementProp(totem_elem, "expiration", expiration_timestamp);
            LogInfo("TotemCreation", "Totem " + totem.serial + " registrado no vigilante para expirar em " + expiration_timestamp);
        endif
    endif
    // ==========================================================
    // >>>>> FIM DA ADIÇÃO <<<<<
    // ==========================================================

    LogCritical("CreateTotem_LOG", "FUNÇÃO CONCLUÍDA COM SUCESSO. O totem está no jogo e registrado.");
    LogCritical("CreateTotem_LOG", "========================================================");
endfunction

// Função CORRIGIDA - Substitua a versão antiga por esta
function TerritorialBossDeath(corpse_or_mobile)
    LogCritical("BossDeath_LOG", "========================================================");
    LogCritical("BossDeath_LOG", "INICIANDO TerritorialBossDeath");

    if (!corpse_or_mobile)
        LogError("BossDeath_LOG", "FALHA CRÍTICA (PASSO 0): Parâmetro 'corpse_or_mobile' é nulo. Abortando.");
        LogCritical("BossDeath_LOG", "==================== FUNÇÃO ENCERRADA (ERRO) ====================");
        return;
    endif
    LogInfo("BossDeath_LOG", "  -> Objeto gatilho: Serial " + corpse_or_mobile.serial + " (Tipo: " + TypeOf(corpse_or_mobile) + ")");

    LogInfo("BossDeath_LOG", "--> PASSO 1: Lendo CProp 'ChampionAltar' do objeto...");
    var altar_serial := GetObjProperty(corpse_or_mobile, "ChampionAltar");

    if (!altar_serial)
        LogError("BossDeath_LOG", "    FALHA (PASSO 1): CProp 'ChampionAltar' não encontrada. Impossível continuar.");
        LogCritical("BossDeath_LOG", "==================== FUNÇÃO ENCERRADA (ERRO) ====================");
        return;
    endif
    LogInfo("BossDeath_LOG", "    SUCESSO (PASSO 1): Serial do altar encontrado: " + altar_serial);

    LogInfo("BossDeath_LOG", "--> PASSO 2: Buscando o objeto do Altar no mundo...");
    var altar := SystemFindObjectBySerial(CInt(altar_serial));

    if (!altar)
        LogError("BossDeath_LOG", "    FALHA (PASSO 2): Objeto do Altar (Serial: " + altar_serial + ") não encontrado.");
        LogCritical("BossDeath_LOG", "==================== FUNÇÃO ENCERRADA (ERRO) ====================");
        return;
    endif
    LogInfo("BossDeath_LOG", "    SUCESSO (PASSO 2): Objeto do Altar encontrado.");

    LogInfo("BossDeath_LOG", "--> PASSO 3: Lendo CProps 'TerritoryName' e 'TerritoryID' do Altar...");
    var territory_name := GetObjProperty(altar, "TerritoryName");
    var territory_id := GetObjProperty(altar, "TerritoryID");

    if (!territory_name || !territory_id)
        LogError("BossDeath_LOG", "    FALHA (PASSO 3): Altar " + altar.serial + " não possui 'TerritoryName' ou 'TerritoryID'.");
        LogCritical("BossDeath_LOG", "==================== FUNÇÃO ENCERRADA (ERRO) ====================");
        return;
    endif
    LogInfo("BossDeath_LOG", "    SUCESSO (PASSO 3): Nome='" + territory_name + "', ID='" + territory_id + "'.");

    LogInfo("BossDeath_LOG", "--> PASSO 4: Determinando coordenadas para o totem...");
    var center_info := TM_GetTerritoryCenter(territory_id);
    var spawn_x, spawn_y, spawn_z, spawn_realm;

    if (center_info)
        spawn_x := center_info.x;
        spawn_y := center_info.y;
        spawn_z := center_info.z;
        spawn_realm := center_info.realm;
        LogInfo("BossDeath_LOG", "    -> Usando centro do território: " + spawn_x + "," + spawn_y + "," + spawn_z);
    else
        spawn_x := corpse_or_mobile.x;
        spawn_y := corpse_or_mobile.y;
        spawn_z := corpse_or_mobile.z;
        spawn_realm := corpse_or_mobile.realm;
        LogWarning("BossDeath_LOG", "    -> AVISO: Não foi possível obter o centro do território. Usando a localização do corpo como fallback: " + spawn_x + "," + spawn_y + "," + spawn_z);
    endif

    LogInfo("BossDeath_LOG", "--> PASSO 5: Delegando a criação do totem para a função CreateTotemOnBossDeath...");
    CreateTotemOnBossDeath(territory_id, spawn_x, spawn_y, spawn_z, spawn_realm);

    LogCritical("BossDeath_LOG", "FUNÇÃO CONCLUÍDA COM SUCESSO.");
    LogCritical("BossDeath_LOG", "========================================================");
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
    
    if (creature && !creature.errotext)
        PlaySoundEffect(creature, 0x2F4);
        SetObjProperty(altar, "champ_serial", creature.serial);
        
        LogCritical("SpawnChampion_LOG", "----------------------------------------------------");
        LogCritical("SpawnChampion_LOG", "Boss criado - Serial: " + creature.serial + ", Template: " + boss_template);
        
        // LER TODAS AS INFORMAÇÕES DO ALTAR
        var territory_name := GetObjProperty(altar, "TerritoryName");
        var territory_id := GetObjProperty(altar, "TerritoryID");
        
        if (territory_name && territory_id)
            LogCritical("SpawnChampion_LOG", "PASSO 1: Altar é territorial. Nome: '" + territory_name + "', ID: '" + territory_id + "'.");
            
            // Configura o boss como guardião territorial
            SetObjProperty(creature, "IsGuardian", 1);
            SetObjProperty(creature, "TerritoryName", territory_name);
            SetObjProperty(creature, "TerritoryID", territory_id); // Passa o ID para o boss
            SetObjProperty(creature, "FromChampionSpawn", 1);
            SetObjProperty(creature, "ChampionAltar", altar.serial);
            
            LogCritical("SpawnChampion_LOG", "PASSO 2: Propriedades 'TerritoryName' e 'TerritoryID' definidas no boss " + creature.serial + ".");
        else
            // Boss normal
            SetObjProperty(creature, "#AltarSerial", altar.serial);
            LogCritical("SpawnChampion_LOG", "Boss criado como champion NORMAL (não territorial).");
        endif
        
        return creature;
    else
        LogError("ChampSpawn", "Falha ao criar o CHEFE com template '" + boss_template + "'.");
        return 0;
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