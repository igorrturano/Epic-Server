// :faccao:spawndecampeao/include/waves.inc

// WAVES.INC - Funções de gerenciamento de ondas e spawning
// Criado durante refatoração para separar responsabilidades

use uo;
use os;
use math;

// ==========================================================
// >>>>> INÍCIO DA CORREÇÃO <<<<<
// ==========================================================
// Adiciona os includes necessários para as funções DENTRO deste arquivo.
include ":faccao:include/shared_functions";
include ":epicspawn:epicBoss";
include ":faccao:spawndecampeao/include/altar"; // ESSENCIAL: Torna GetSpawnLocation visível

// ==========================================================
// >>>>> FIM DA CORREÇÃO <<<<<
// ==========================================================


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
        var loc := GetSpawnLocation(altar); // <--- Agora esta função é reconhecida
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
    
    // Define um tempo de expiração de 60 segundos (1 minuto)
    var expiration_timestamp := polcore().systime + 60;
    LogCritical("TotemLifecycle", "PASSO 1: Totem " + totem.serial + " criado. Definindo expiração para " + expiration_timestamp + " (" + (expiration_timestamp - polcore().systime) + " segundos a partir de agora).");

    LogInfo("CreateTotem_LOG", "SUCESSO: Totem criado (Serial: " + totem.serial + "). Configurando CProps...");
    SetObjProperty(totem, "TerritoryID", territory_id);
    SetObjProperty(totem, "Name", territory_name);
    SetObjProperty(totem, "Rect", GetElementProp(elem, "Rect"));
    SetObjProperty(totem, "Realm", GetElementProp(elem, "Realm"));
    SetObjProperty(totem, "FromBossDefeat", 1);
    SetObjProperty(totem, "ClaimByTimestamp", expiration_timestamp);

  var totem_registry_df := SafeOpenDataFile(":faccao:totem_registry", DF_CREATE);
    if (totem_registry_df)
        var totem_elem := SafeFindElement(totem_registry_df, CStr(totem.serial), DF_CREATE);
        if (totem_elem)
            // Agora salvamos a informação mais importante junto com a expiração
            SetElementProp(totem_elem, "territory_id", territory_id); // <-- LINHA ADICIONADA
            SetElementProp(totem_elem, "expiration", expiration_timestamp);
            LogCritical("TotemLifecycle", "PASSO 2: Totem " + totem.serial + " (Território: " + territory_id + ") registrado no datafile do vigilante.");
        endif
    endif

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


function SpawnChampion(altar, boss_template)
    var loc := GetSpawnLocation(altar); // <--- Agora esta função é reconhecida
    var creature := CreateNpcWithPrefix(boss_template, loc[1], loc[2], loc[3], altar.realm);
    
    if (creature && !creature.errotext)
        PlaySoundEffect(creature, 0x2F4);
        SetObjProperty(altar, "champ_serial", creature.serial);
        
        LogCritical("SpawnChampion_LOG", "----------------------------------------------------");
        LogCritical("SpawnChampion_LOG", "Boss criado - Serial: " + creature.serial + ", Template: " + boss_template);
        
        var territory_name := GetObjProperty(altar, "TerritoryName");
        var territory_id := GetObjProperty(altar, "TerritoryID");
        
        if (territory_name && territory_id)
            LogCritical("SpawnChampion_LOG", "PASSO 1: Altar é territorial. Nome: '" + territory_name + "', ID: '" + territory_id + "'.");
            
            SetObjProperty(creature, "IsGuardian", 1);
            SetObjProperty(creature, "TerritoryName", territory_name);
            SetObjProperty(creature, "TerritoryID", territory_id);
            SetObjProperty(creature, "FromChampionSpawn", 1);
            SetObjProperty(creature, "ChampionAltar", altar.serial);
            
            LogCritical("SpawnChampion_LOG", "PASSO 2: Propriedades 'TerritoryName' e 'TerritoryID' definidas no boss " + creature.serial + ".");
        else
            SetObjProperty(creature, "#AltarSerial", altar.serial);
            LogCritical("SpawnChampion_LOG", "Boss criado como champion NORMAL (não territorial).");
        endif
        
        return creature;
    else
        LogError("ChampSpawn", "Falha ao criar o CHEFE com template '" + boss_template + "'.");
        return 0;
    endif
endfunction


function CleanupOrphanedItems(altar)
    LogCritical("ChampSpawn", "*** INICIANDO CleanupOrphanedItems ***");
    LogCritical("ChampSpawn", "Altar para limpeza: " + altar.serial);
    
    var cleanup_radius := 25;
    var items_removed := 0;
    
    var nearby_items := ListItemsNearLocation(altar.x, altar.y, altar.z, cleanup_radius, altar.realm);
    
    foreach item in nearby_items
        var should_remove := 0;
        
        if (item.objtype == 0x1854 && item.color == 0x26)
            should_remove := 1;
        endif
        
        if (item.objtype == 0x142C)
            should_remove := 1;
        endif
        
        if (item.objtype == 0x6d70)
            should_remove := 1;
        endif
        
        if (should_remove)
            DestroyItem(item);
            items_removed := items_removed + 1;
        endif
    endforeach
    
    EraseObjProperty(altar, "skulls");
    EraseObjProperty(altar, "white_candles");
    EraseObjProperty(altar, "boss_area_items");
    EraseObjProperty(altar, "#current_level");
    EraseObjProperty(altar, "#current_wave_kills");
    
    altar.color := 0;
    
    if (items_removed > 0)
        LogInfo("ChampSpawn", "Limpeza concluída: " + items_removed + " itens órfãos removidos");
    endif
    
    LogCritical("ChampSpawn", "*** CleanupOrphanedItems FINALIZADA ***");
endfunction