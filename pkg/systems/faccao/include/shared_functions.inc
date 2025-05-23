use uo;
use os;
use cfgfile;
use datafile;
use guilds;

include ":datafile:datafile";
include ":faccao:faccao_constants";
include ":faccao:include/faccao_resources";
include ":timeutils:time";
include ":faccao:include/logging";
include ":faccao:include/datafile_helpers";  // Nova inclusão
include ":faccao:include/territory_management";


// Funções compartilhadas entre o sistema de baú e o sistema de gestão

/* 
 * ContarItensDoadosNoBau(bau)
 * 
 * Purpose: Conta todos os itens doados no baú, classificando-os por tipo
 * Parameters: 
 *   bau - Referência ao container do baú
 * Return:
 *   Struct com contagens de cada tipo de recurso
 */
function ContarItensDoadosNoBau(container, force_count := 0)
    // Validação inicial do container
    if (!container)
        LogError("recursos", "ContarItensDoadosNoBau chamada com container nulo");
        return CreateEmptyResourceStruct();
    endif
    
    LogDebug("recursos", $"Iniciando contagem de itens doados no baú: {container.serial}");
    
    // Verificar condições de cache
    var cache_dirty := GetObjProperty(container, CACHE_DIRTY_FLAG);
    var ultimo_update := GetObjProperty(container, CACHE_UPDATE_PROP);
    var current_time := polcore().systime;
    
    // Lógica de uso de cache otimizada
    if (!force_count && 
        !cache_dirty && 
        ultimo_update && 
        (current_time - ultimo_update < CACHE_EXPIRY_TIME))
        
        LogDebug("recursos", $"Usando cache - última atualização há {current_time - ultimo_update} segundos");
        
        var recursos_cache := GetObjProperty(container, CACHE_PROP_NAME);
        if (recursos_cache)
            return recursos_cache;
        endif
    endif
    
    // Contagem completa de recursos
    LogInfo("recursos", "Realizando contagem completa dos recursos no baú");
    var recursos := ContarRecursosReais(container);
    
    // Verificar inconsistências
    var recursos_cache := GetObjProperty(container, CACHE_PROP_NAME);
    if (recursos_cache)
        VerificarInconsistencias(container, recursos, "local");
    endif
    
    // Atualizar cache
    SetObjProperty(container, CACHE_PROP_NAME, recursos);
    SetObjProperty(container, CACHE_UPDATE_PROP, current_time);
    EraseObjProperty(container, CACHE_DIRTY_FLAG);
    
    LogDebug("recursos", "Cache de recursos atualizado");
    
    // Lógica de sincronização com datafile
    var ultima_sync := GetObjProperty(container, DATAFILE_SYNC_PROP);
    if (!ultima_sync || 
        (current_time - ultima_sync > DATAFILE_UPDATE_INTERVAL) || 
        force_count)
        
        SincronizarComDataFile(container, recursos);
        SetObjProperty(container, DATAFILE_SYNC_PROP, current_time);
        
        LogInfo("recursos", "Datafile global atualizado");
    else
        LogDebug("recursos", $"Atualização de datafile agendada - última sync há {current_time - ultima_sync} segundos");
        AgendarAtualizacaoDataFile(container);
    endif
    
    return recursos;
endfunction

/*
 * FindGuildByName(name)
 *
 * Purpose: Encontrar o ID de uma guilda pelo nome de forma confiável
 * Parameters: 
 *   name - Nome da facção/guilda a ser buscada
 * Return: GuildID da guilda ou 0 se não encontrada
 */
function FindGuildByName(name)
    if (!name)
        return 0;
    endif
    
    // MÉTODO 1: Tentar usar o registro persistente de guildas (mais confiável)
    var registry_pid := GetGlobalProperty("guild_registry_pid");
    if (registry_pid)
        var process := GetProcess(registry_pid);
        if (process)
            // Enviar solicitação para o registro
            var ev := struct{
                "type" := "lookup",
                "name" := name,
                "process" := GetPid()
            };
            process.SendEvent(ev);
            
            // Aguardar resposta por até 2 segundos
            ev := wait_for_event(2);
            if (ev && ev.type == "lookup_result" && ev.guild_id)
                return ev.guild_id;
            endif
        endif
    endif
    
    // MÉTODO 2: Tentar buscar diretamente no datafile de registro (em caso de falha do processo)
    var df := SafeOpenDataFile(":faccao:guild_registry");
    if (!df)
        df := SafeOpenDataFile("guild_registry");
    endif
    
    if (df)
        var elem := SafeFindElement(df, "guilds");
        if (elem)
            var registry := GetElementProp(elem, "registry");
            if (registry && typeof(registry) == "Dictionary")
                // Verificar correspondência exata
                if (registry.exists(name))
                    return registry[name];
                endif
                
                // Verificar case-insensitive
                var lower_name := Lower(name);
                foreach key in (registry.keys())
                    if (Lower(key) == lower_name)
                        return registry[key];
                    endif
                    sleepms(2);
                endforeach
            endif
        endif
    endif
    
    // MÉTODO 3: Busca tradicional usando ListGuilds (menos confiável)
    var guilds := ListGuilds();
    if (guilds && guilds != error)
        foreach guild in guilds
            var prop := guild.GetProp(FACTION_PROP_NAME);
            if (prop != error && prop == name)
                return guild.guildid;
            endif
            
            // Verificar outras propriedades de nome
            prop := guild.GetProp("name");
            if (prop != error && prop == name)
                return guild.guildid;
            endif
            
            // Verificar o nome direto da guilda
            if (guild.guildname == name)
                return guild.guildid;
            endif
            
            sleepms(2);
        endforeach
    endif
    
    // MÉTODO 4: Backup - procurar por jogadores online (último recurso)
    foreach player in (EnumerateOnlineCharacters())
        if (player.guildid && player.guild.GetProp(FACTION_PROP_NAME) == name)
            return player.guild.guildid;
        endif
        
        if (player.guildid && player.guild.GetProp("name") == name)
            return player.guild.guildid;
        endif
        
        if (player.guildid && player.guild.guildname == name)
            return player.guild.guildid;
        endif
        
        sleepms(2);
    endforeach
    
    // Tentar encontrar por nome parcial (recurso de último caso)
    var lower_name := Lower(name);
    if (guilds && guilds != error)
        foreach guild in guilds
            var guild_name := guild.GetProp(FACTION_PROP_NAME);
            if (!guild_name || guild_name == error)
                guild_name := guild.GetProp("name");
            endif
            if (!guild_name || guild_name == error)
                guild_name := guild.guildname;
            endif
            
            if (guild_name && guild_name != error)
                if (Lower(guild_name).find(lower_name) != error || lower_name.find(Lower(guild_name)) != error)
                    return guild.guildid;
                endif
            endif
            
            sleepms(2);
        endforeach
    endif
    
    return 0;
endfunction


function ForEachInArray(array_to_iterate, callback)
    if (!array_to_iterate || array_to_iterate.size() < 1 || !callback)
        return;
    endif
    
    // POL usa índices baseados em 1, não 0
    for i := 1 to array_to_iterate.size()
        if (i <= array_to_iterate.size() && callback.?call)
            callback.call(array_to_iterate[i], i);
        endif
        sleepms(2);
    endfor
endfunction

/*
 * MySplitWords(text, delimiter := " ")
 *
 * Purpose: Versão mais robusta de SplitWords que lida melhor com delimitadores personalizados
 * Parameters:
 *   text - O texto a ser dividido
 *   delimiter - O delimitador a ser usado (padrão: espaço)
 * Return: Array de substrings
 */
function MySplitWords(text, delimiter := " ")
    if (text == error || !text || text == "")
        return array{};
    endif
    
    // Garantir que text é string
    text := CStr(text);
    
    var result := array{};
    var current_pos := 1;
    var delim_pos := Find(text, delimiter, current_pos);
    
    while (delim_pos)
        var part := text[current_pos, delim_pos - current_pos];
        part := Trim(part);
        if (part != "")
            result.append(part);
        endif
        
        current_pos := delim_pos + len(delimiter);
        delim_pos := Find(text, delimiter, current_pos);
        
        sleepms(2);
    endwhile
    
    // Processar a última parte
    if (current_pos <= len(text))
        var last_part := text[current_pos, len(text) - current_pos + 1];
        last_part := Trim(last_part);
        if (last_part != "")
            result.append(last_part);
        endif
    endif
    
    return result;
endfunction


/*
 * CreateUniqueItemID(faction_name, item_type, territory_id)
 *
 * Purpose: Cria um ID único padronizado para itens de facção
 * Parameters:
 *   faction_name - Nome da facção
 *   item_type - Tipo do item (BAU, MESA, MURAL)
 *   territory_id - ID do território
 * Return:
 *   String formatada como FactionName:ItemType:TerritoryID
 */
function CreateUniqueItemID(faction_name, item_type, territory_id)
    return faction_name + ":" + item_type + ":" + territory_id;
endfunction


/*
 * FindItemsByFactionAndType(faction_name, item_type := "", territory_id := "")
 *
 * Purpose: Encontra todos os itens associados a uma facção, opcionalmente filtrando por tipo e território
 * Parameters:
 *   faction_name - Nome da facção
 *   item_type - Tipo do item (opcional)
 *   territory_id - ID do território (opcional)
 * Return:
 *   Array de referências para os itens encontrados
 */
function FindItemsByFactionAndType(faction_name, item_type := "", territory_id := "")
    var matching_items := array{};
    var search_pattern := faction_name;
    
    if (item_type)
        search_pattern += ":" + item_type;
    endif
    
    if (territory_id)
        search_pattern += ":" + territory_id;
    endif
    
    // Primeiro tentar pelos registros no datafile
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territorios_seriais := GetElementProp(seriais_df, "territorios");
        if (territorios_seriais)
            foreach territorio_id in (territorios_seriais.keys())
                  var info := territorios_seriais[territorio_id];
                  if (!info || TypeOf(info) != "Struct")
                      continue;
                  endif
                
                // Se territórios específicos foram solicitados
                if (territory_id && territorio_id != territory_id)
                    continue;
                endif
                
                // Verificar se a facção corresponde
                if (!info.?faction || info.faction != faction_name)
                    continue;
                endif
                
                // Adicionar itens conforme o tipo solicitado
                if (!item_type || item_type == ITEM_TYPE_BAU)
                    if (info.?bau)
                        var bau := SystemFindObjectBySerial(info.bau);
                        if (bau)
                            matching_items.append(bau);
                        endif
                    endif
                endif
                
                if (!item_type || item_type == ITEM_TYPE_MESA)
                    var mesa := SystemFindObjectBySerial(info.mesa);
                    if (mesa)
                        matching_items.append(mesa);
                    endif
                endif
                
                if (!item_type || item_type == ITEM_TYPE_MURAL)
                    var mural := SystemFindObjectBySerial(info.mural);
                    if (mural)
                        matching_items.append(mural);
                    endif
                endif
                
                sleepms(2);
            endforeach
        endif
    endif
    
    // Se não encontrou ou ainda precisa procurar mais
    if (matching_items.size() < 1)
        foreach item in EnumerateItemsInContainer(0)
            var unique_id := GetObjProperty(item, UNIQUE_ID_PROP);
            if (unique_id && unique_id.find(search_pattern) == 0)
                matching_items.append(item);
            else
                // Verificar também por propriedades individuais para compatibilidade
                var item_faction := GetObjProperty(item, OBJ_FACTION_PROP);
                var item_territory := GetObjProperty(item, TERRITORY_ID_PROP);
                
                if (item_faction == faction_name)
                    if (!territory_id || item_territory == territory_id)
                        if (!item_type || 
                            (item_type == ITEM_TYPE_BAU && item.objtype == 0xff11) ||
                            (item_type == ITEM_TYPE_MESA && item.objtype == 0xFE5C) ||
                            (item_type == ITEM_TYPE_MURAL && item.objtype == 0x46AE))
                            matching_items.append(item);
                        endif
                    endif
                endif
            endif
            sleepms(1);
        endforeach
    endif
    
    return matching_items;
endfunction


/*
 * FindTerritoryByID(territory_id)
 *
 * Purpose: Localiza o elemento de território pelo ID
 * Parameters:
 *   territory_id - ID do território
 * Return:
 *   Elemento do datafile ou 0 se não encontrado
 */
function FindTerritoryByID(territory_id)
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        return 0;
    endif
    
    // Primeiro tentar encontrar pelo ID direto (se o ID for o nome)
    var elem := SafeFindElement(rect_df, territory_id);
    if (elem)
        return elem;
    endif
    
    // Se não encontrar, procurar em todos os elementos
    foreach elem_name in (rect_df.keys())
        elem := SafeFindElement(rect_df, elem_name);
        var id := GetElementProp(elem, TERRITORY_ID_PROP);
        if (id == territory_id)
            return elem;
        endif
        sleepms(2);
    endforeach
    
    return 0;
endfunction



// Flag global para indicar se uma sincronização está em andamento
var GLOBAL_SYNC_IN_PROGRESS := 0;

// Função centralizada para marcar recursos como modificados
function MarcarRecursosModificados(container)
    if (!container)
        return 0;
    endif
    
    // Marcar que o container precisa atualização
    SetObjProperty(container, CACHE_DIRTY_FLAG, 1);
    
    // Registrar o timestamp da modificação
    SetObjProperty(container, "recursos_last_modified", polcore().systime);
    
    // Agendar atualização assíncrona
    return AgendarAtualizacaoBau(container);
endfunction


// Função centralizada para agendar atualizações
function AgendarAtualizacaoBau(container)
    if (!container)
        LogWarning("recursos", "Tentativa de agendar atualização para container nulo");
        return 0;
    endif
    
    // Obter lista global de atualizações pendentes
    var updates_pending := GetGlobalProperty("recursos_updates_pending");
    if (!updates_pending || updates_pending == error)
        updates_pending := dictionary{};
    endif
    
    // Verificar se este container já está na fila
    if (updates_pending.exists(container.serial))
        // Apenas atualizar o timestamp se já existe
        updates_pending[container.serial] := polcore().systime;
        LogDebug("recursos", $"Timestamp atualizado para baú #{container.serial}");
    else
        // Adicionar à fila se não existe
        updates_pending[container.serial] := polcore().systime;
        LogInfo("recursos", $"Baú #{container.serial} agendado para atualização");
    endif
    
    // Atualizar a propriedade global
    var result := SetGlobalProperty("recursos_updates_pending", updates_pending);
    if (result != 1)
        LogError("recursos", $"Falha ao agendar baú: {result}");
        return 0;
    endif
    
    // Verificar se o processador está rodando e iniciá-lo se necessário
    var pid := GetGlobalProperty("recursos_update_processor_pid");
    if (!pid || !GetProcess(pid))
        var script := start_script(":faccao:process_recursos_updates");
        if (script && script.errortext)
            LogCritical("recursos", $"Erro ao iniciar processador de recursos: {script.errortext}");
        else
            LogInfo("recursos", "Processador de recursos iniciado");
        endif
    endif
    
    return 1;
endfunction



// Função para forçar sincronização imediata de forma segura
function ForcarSincronizacaoBau(container)
    if (!container)
        return 0;
    endif
    
    // Verificar se já existe uma sincronização em andamento
    if (GLOBAL_SYNC_IN_PROGRESS)
        LogWarning("sincronizacao", "Sincronização já em andamento, agendando para posterior");

        return AgendarAtualizacaoBau(container);
    endif
    
    // Indicar que uma sincronização está começando
    GLOBAL_SYNC_IN_PROGRESS := 1;
    
    // Forçar contagem completa e atualização do datafile
    var recursos := ContarItensDoadosNoBau(container, 1); // 1 = force_count
    
    // Sincronizar com o datafile
    var sync_result := SincronizarComDataFile(container, recursos);
    
    // Registrar histórico de sincronização
    var historico := GetObjProperty(container, "recursos_sync_history");
    if (!historico)
        historico := array{};
    endif
    
    historico.append(struct{
        "timestamp" := polcore().systime,
        "forced" := 1,
        "success" := sync_result
    });
    
    // Limitar o tamanho do histórico
    while (historico.size() > 20)
        historico.erase(1);
    endwhile
    
    SetObjProperty(container, "recursos_sync_history", historico);
    
    // Atualizar timestamps
    SetObjProperty(container, DATAFILE_SYNC_PROP, polcore().systime);
    
    // Limpar flag de sujeira
    EraseObjProperty(container, CACHE_DIRTY_FLAG);
    
    // Terminou a sincronização
    GLOBAL_SYNC_IN_PROGRESS := 0;
    
    return sync_result;
endfunction

// Função para atualizar o datafile de recursos
function AtualizarDatafileRecursos(faction_name, recursos)
    if (!faction_name || !recursos)
        return 0;
    endif
    
    var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE, 1);
    if (!df)
        LogError("recursos", "Não foi possível abrir/criar o datafile de recursos");
        return 0;
    endif
    
    // Abordagem diferente dependendo se é Reino ou facção específica
    if (faction_name == KINGDOM_FACTION || faction_name == "Reino")
        // Reino: usar o elemento principal
        var elem := SafeFindElement(df, KINGDOM_RESOURCE_ELEM, 1);
        SetElementProp(elem, "recursos", recursos);
    else
        // Facção específica: criar elemento dedicado
        var elem := SafeFindElement(df, "faction_" + faction_name, 1);
        SetElementProp(elem, "recursos", recursos);
    endif
    
    return 1;
endfunction

// Função para verificar e resolver inconsistências em um baú
function VerificarEResolverInconsistencias(container)
    if (!container)
        return 0;
    endif
    
    // Obter recursos do cache atual
    var recursos_cache := GetObjProperty(container, CACHE_PROP_NAME);
    
    // Contar recursos reais
    var recursos_reais := ContarRecursosReais(container);
    
    // Se não há cache, apenas criar
    if (!recursos_cache)
        SetObjProperty(container, CACHE_PROP_NAME, recursos_reais);
        SetObjProperty(container, CACHE_UPDATE_PROP, polcore().systime);
        return 0;
    endif
    
    // Verificar se há diferenças significativas
    var inconsistencias := 0;
    var diff_details := array{};
    
    // Verificar cada tipo de recurso
    if (recursos_reais.exists("comida") && recursos_cache.exists("comida") && MyAbs(recursos_reais.comida - recursos_cache.comida) > 5)
        inconsistencias += 1;
        diff_details.append(struct{
            "tipo" := "comida",
            "cache" := recursos_cache.comida,
            "real" := recursos_reais.comida,
            "diff" := recursos_reais.comida - recursos_cache.comida
        });
    endif
    
    if (recursos_reais.exists("moedas") && recursos_cache.exists("moedas") && MyAbs(recursos_reais.moedas - recursos_cache.moedas) > 5)
        inconsistencias += 1;
        diff_details.append(struct{
            "tipo" := "moedas",
            "cache" := recursos_cache.moedas,
            "real" := recursos_reais.moedas,
            "diff" := recursos_reais.moedas - recursos_cache.moedas
        });
    endif
    
    if (recursos_reais.exists("couro") && recursos_cache.exists("couro") && MyAbs(recursos_reais.couro - recursos_cache.couro) > 5)
        inconsistencias += 1;
        diff_details.append(struct{
            "tipo" := "couro",
            "cache" := recursos_cache.couro,
            "real" := recursos_reais.couro,
            "diff" := recursos_reais.couro - recursos_cache.couro
        });
    endif
    
    if (recursos_reais.exists("metal") && recursos_cache.exists("metal") && MyAbs(recursos_reais.metal - recursos_cache.metal) > 5)
        inconsistencias += 1;
        diff_details.append(struct{
            "tipo" := "metal",
            "cache" := recursos_cache.metal,
            "real" := recursos_reais.metal,
            "diff" := recursos_reais.metal - recursos_cache.metal
        });
    endif
    
    if (recursos_reais.exists("madeira") && recursos_cache.exists("madeira") && MyAbs(recursos_reais.madeira - recursos_cache.madeira) > 5)
        inconsistencias += 1;
        diff_details.append(struct{
            "tipo" := "madeira",
            "cache" := recursos_cache.madeira,
            "real" := recursos_reais.madeira,
            "diff" := recursos_reais.madeira - recursos_cache.madeira
        });
    endif
    
    if (recursos_reais.exists("pano") && recursos_cache.exists("pano") && MyAbs(recursos_reais.pano - recursos_cache.pano) > 5)
        inconsistencias += 1;
        diff_details.append(struct{
            "tipo" := "pano",
            "cache" := recursos_cache.pano,
            "real" := recursos_reais.pano,
            "diff" := recursos_reais.pano - recursos_cache.pano
        });
    endif
    
    if (recursos_reais.exists("pedras") && recursos_cache.exists("pedras") && MyAbs(recursos_reais.pedras - recursos_cache.pedras) > 5)
        inconsistencias += 1;
        diff_details.append(struct{
            "tipo" := "pedras",
            "cache" := recursos_cache.pedras,
            "real" := recursos_reais.pedras,
            "diff" := recursos_reais.pedras - recursos_cache.pedras
        });
    endif
    
    if (recursos_reais.exists("joias") && recursos_cache.exists("joias") && MyAbs(recursos_reais.joias - recursos_cache.joias) > 5)
        inconsistencias += 1;
        diff_details.append(struct{
            "tipo" := "joias",
            "cache" := recursos_cache.joias,
            "real" := recursos_reais.joias,
            "diff" := recursos_reais.joias - recursos_cache.joias
        });
    endif
    
    // Se encontrou inconsistências
    if (inconsistencias > 0)
        // Atualizar o cache com valores reais
        SetObjProperty(container, CACHE_PROP_NAME, recursos_reais);
        SetObjProperty(container, CACHE_UPDATE_PROP, polcore().systime);
        
        // Registrar histórico de inconsistências
        var historico := GetObjProperty(container, "recursos_inconsistency_history");
        if (!historico)
            historico := array{};
        endif
        
        historico.append(struct{
            "timestamp" := polcore().systime,
            "inconsistencias" := inconsistencias,
            "detalhes" := diff_details
        });
        
        // Limitar o tamanho do histórico
        while (historico.size() > 10)
            historico.erase(1);
        endwhile
        
        SetObjProperty(container, "recursos_inconsistency_history", historico);
        
        // Log de depuração
        LogWarning("recursos", $"Encontradas {inconsistencias} inconsistências no baú #{container.serial}");

        
        // Sincronizar com datafile
        SincronizarComDataFile(container, recursos_reais);
        
        return inconsistencias;
    endif
    
    return 0;
endfunction

// Função auxiliar para valor absoluto
function MyAbs(value)
    if (value < 0)
        return -value;
    endif
    return value;
endfunction

// Função para atualizar propriedades da guilda
function AtualizarPropriedadesGuildaRecursos(faction_name, recursos)
    if (faction_name == KINGDOM_FACTION || faction_name == "Reino")
        // Reino não tem guilda, apenas datafile
        return 1;
    endif
    
    var guild_id := FindGuildByName(faction_name);
    if (!guild_id)
        return 0;
    endif
    
    var guild := FindGuild(guild_id);
    if (!guild)
        return 0;
    endif
    
    // Atualizar recursos na guilda
    guild.SetProp("recursos", recursos);
    guild.SetProp("recursos_timestamp", polcore().systime);
    
    return 1;
endfunction



// Função para obter estatísticas de um baú específico
function ObterEstatisticasBau(container)
    if (!container)
        return struct{};
    endif
    
    var stats := struct{
        "ultima_contagem" := GetObjProperty(container, "recursos_ultimo_update"),
        "ultima_sync" := GetObjProperty(container, "recursos_ultima_sync"),
        "ultima_modificacao" := GetObjProperty(container, "recursos_last_modified"),
        "cache_sujo" := GetObjProperty(container, "recursos_cache_dirty") ? 1 : 0,
        "historico_sync" := GetObjProperty(container, "recursos_sync_history"),
        "historico_verificacao" := GetObjProperty(container, "recursos_verify_history"),
        "faction" := GetObjProperty(container, "faction")
    };
    
    return stats;
endfunction

// Função para verificar se um container é válido e acessível
function IsValidContainer(container)
    if (!container)
        return 0;
    endif
    
    // Verificar se é realmente um container
    if (!container.isA(POLCLASS_CONTAINER))
        return 0;
    endif
    
    // Verificar se não está trancado
    if (container.locked)
        return 0;
    endif
    
    // Verificar se não está em um contexto inválido
    if (container.container == 0xFFFFFFFF)
        return 0;
    endif
    
    return 1;
endfunction

// Função para verificar se um container tem espaço para novos itens
function HasContainerSpace(container, amount := 1)
    if (!IsValidContainer(container))
        return 0;
    endif
    
// Verificar limite de itens (assumindo um limite máximo padrão)
var item_count := 0;
var items := EnumerateItemsInContainer(container);
if (items != error && items)
    foreach item in items
        if (item)
            item_count += 1;
        endif
        sleepms(1); // Para evitar congelamento do servidor
    endforeach
endif
    
    // Definir um limite razoável (pode ser ajustado conforme necessário)
    var max_items := 125;
    
    return (item_count + amount <= max_items);
endfunction


// Modificação da função SafeCreateItemInContainer com logs de depuração
function SafeCreateItemInContainer(container, objtype, amount, properties := 0)
    LogDebug("SafeCreate_DEBUG", "Entrando em SafeCreate: Cont=" + container.serial + ", Obj=0x" + Hex(objtype) + ", Amt=" + amount);

    if (!container || !objtype || amount <= 0) 
        return 0;
    endif

    var existing_item := FindExistingStackableItem(container, objtype);
    if (existing_item)
        LogDebug("SafeCreate_DEBUG", "ExistingItem encontrado: " + existing_item.serial);
    else
        LogDebug("SafeCreate_DEBUG", "ExistingItem encontrado: Nenhum");
    endif

    if (existing_item)
        var max_stack := 60000;
        var old_amount := existing_item.amount;
        var amount_to_add_now := amount;

        if (old_amount + amount_to_add_now > max_stack)
            amount_to_add_now := max_stack - old_amount;
            if (amount_to_add_now <= 0) 
                existing_item := 0; // Não pode empilhar, força criação de novo
            endif
        endif

        if (existing_item && amount_to_add_now > 0)
            LogDebug("SafeCreate_DEBUG", "Tentando TryIncreaseAmount no item " + existing_item.serial + " com +" + amount_to_add_now);
            var added_amount := TryIncreaseAmount(existing_item, amount_to_add_now);

            if (added_amount > 0)
                LogDebug("SafeCreate_DEBUG", "TryIncreaseAmount SUCESSO. Adicionado: " + added_amount + ". Aplicando props...");
                if (properties) 
                    ApplyPropertiesToItem(existing_item, properties);
                endif
                LogDebug("SafeCreate_DEBUG", "Propriedades aplicadas (ou nenhuma). Retornando item existente " + existing_item.serial);
                return existing_item;
            else
                LogWarning("SafeCreate_DEBUG", "TryIncreaseAmount FALHOU. Tentando ReplaceStackWithNew...");
                var replacement := ReplaceStackWithNew(existing_item, amount, properties); // Tenta com o amount original
                if (replacement)
                    LogDebug("SafeCreate_DEBUG", "Resultado ReplaceStackWithNew: " + replacement.serial);
                else
                    LogDebug("SafeCreate_DEBUG", "Resultado ReplaceStackWithNew: Falha");
                endif
                return replacement; // Retorna o novo item ou 0 se falhar
            endif
        endif
    endif

    // Criar novo item
    LogDebug("SafeCreate_DEBUG", "Criando novo item via CreateItemInContainer...");
    var new_item := CreateItemInContainer(container, objtype, amount);

    if (new_item)
        LogDebug("SafeCreate_DEBUG", "CreateItemInContainer SUCESSO. Novo serial: " + new_item.serial + ". Aplicando props...");
        if (properties) 
            ApplyPropertiesToItem(new_item, properties);
        endif
        LogDebug("SafeCreate_DEBUG", "Propriedades aplicadas (ou nenhuma). Retornando novo item " + new_item.serial);
        return new_item;
    else
        LogError("SafeCreate_DEBUG", "CreateItemInContainer FALHOU.");
        return 0;
    endif
endfunction

// Função para encontrar um item do mesmo tipo para empilhar
function FindStackableItem(container, objtype)
    if (!container || !objtype)
        return 0;
    endif
    
    var items := EnumerateItemsInContainer(container);
    if (items != error && items)
        foreach item in items
            if (item && item.objtype == objtype && item.amount < 60000)
                return item;
            endif
            sleepms(1);
        endforeach
    endif
    
    return 0;
endfunction

// Função segura para aumentar a quantidade de um item (versão sem try/except)
function SafeIncreaseAmount(item, amount)
    if (!item || amount <= 0)
        return 0;
    endif
    
    var old_amount := item.amount;
    var new_amount := old_amount + amount;
    
    // Verificar se excede o limite
    if (new_amount > 60000)
        new_amount := 60000;
    endif
    
    // Tentar definir a nova quantidade (sem try/except)
    var before_change := item.amount;
    item.amount := new_amount;
    
    // Verificar se a mudança foi efetiva
    if (item.amount == new_amount)
        return 1;
    else
LogError("criacao_item", $"Falha ao aumentar quantidade do item {item.serial}: {item.amount} != {new_amount}");
        // Tentar restaurar valor original
        item.amount := before_change;
        return 0;
    endif
endfunction

// Função para transferir propriedades de um item para outro
function TransferItemProperties(old_item, new_item)
    if (!old_item || !new_item)
        return 0;
    endif
    
    var properties := GetObjPropertyNames(old_item);
    var transferred := 0;
    
    foreach prop_name in properties
        var prop_value := GetObjProperty(old_item, prop_name);
        SetObjProperty(new_item, prop_name, prop_value);
        transferred += 1;
        sleepms(1);
    endforeach
    
    return transferred;
endfunction



// Função para validar recursos antes de salvar (que estava faltando)
function ValidarRecursos(recursos)
    if (!recursos || TypeOf(recursos) != "Struct")
        return CreateEmptyResourceStruct();
    endif
    
    var valid_recursos := CreateEmptyResourceStruct();
    
    // Copiar valores válidos (convertendo para inteiros)
    valid_recursos.comida := CInt(recursos.comida) >= 0 ? CInt(recursos.comida) : 0;
    valid_recursos.moedas := CInt(recursos.moedas) >= 0 ? CInt(recursos.moedas) : 0;
    valid_recursos.couro := CInt(recursos.couro) >= 0 ? CInt(recursos.couro) : 0;
    valid_recursos.metal := CInt(recursos.metal) >= 0 ? CInt(recursos.metal) : 0;
    valid_recursos.madeira := CInt(recursos.madeira) >= 0 ? CInt(recursos.madeira) : 0;
    valid_recursos.pano := CInt(recursos.pano) >= 0 ? CInt(recursos.pano) : 0;
    valid_recursos.pedras := CInt(recursos.pedras) >= 0 ? CInt(recursos.pedras) : 0;
    valid_recursos.joias := CInt(recursos.joias) >= 0 ? CInt(recursos.joias) : 0;
    
    return valid_recursos;
endfunction



/*
 * IsGuildMember(who, faction_name)
 *
 * Purpose: Verificar se um jogador é membro de uma facção específica
 * Parameters:
 *   who - O jogador a ser verificado
 *   faction_name - O nome da facção a ser verificada
 * Return:
 *   1 se for membro, 0 se não for
 */
function IsGuildMember(who, faction_name)
    if (!who || !faction_name)
        return 0;
    endif
    
    var player_faction := GetPlayerFaction(who);
    return (player_faction == faction_name);
endfunction




///////////////////////////////////////////////////////////////
// Sistema de Transações Atômicas para Operações Críticas
///////////////////////////////////////////////////////////////

/*
 * IniciarTransacao(operacao, dados_chave)
 * 
 * Purpose: Registra o início de uma operação crítica
 * Parameters:
 *   operacao - Tipo de operação (ex: "transferir_territorio")
 *   dados_chave - Dados essenciais para identificação e reversão
 * Return: ID da transação ou error em caso de falha
 */
function IniciarTransacao(operacao, dados_chave)
    var df := SafeOpenDataFile(":faccao:transacoes", 1); // 1 = criar se não existir
    if (!df)
        return error; // Erro já logado pela função SafeOpenDataFile
    endif
    
    // ID simplificado baseado em operação e timestamp
    var transaction_id := operacao + "_" + CStr(polcore().systime);
    var elem := SafeFindElement(df, transaction_id, 1); // 1 = criar se não existir
    if (!elem)
        return error; // Erro já logado pela função SafeFindElement
    endif
    
    // Armazenar apenas dados essenciais
    SetElementProp(elem, "operacao", operacao);
    SetElementProp(elem, "dados", dados_chave);
    SetElementProp(elem, "status", "iniciada");
    SetElementProp(elem, "timestamp", polcore().systime);
    
    LogInfo("transacao", "Transação iniciada: " + transaction_id);
    
    return transaction_id;
endfunction


/*
 * FinalizarTransacao(transaction_id)
 * 
 * Purpose: Marca uma transação como concluída
 * Parameters:
 *   transaction_id - ID da transação
 * Return: 1 para sucesso, 0 para falha
 */
function FinalizarTransacao(transaction_id)
    var df := SafeOpenDataFile(":faccao:transacoes");
    if (!df)
        return 0; // Erro já logado pela função SafeOpenDataFile
    endif
    
    var elem := SafeFindElement(df, transaction_id);
    if (!elem)
        return 0; // Erro já logado pela função SafeFindElement
    endif
    
    // Apenas marcar como concluída
    SetElementProp(elem, "status", "concluida");
    SetElementProp(elem, "timestamp_conclusao", polcore().systime);
    
    LogInfo("transacao", "Transação finalizada: " + transaction_id);
    
    // Usar um contador global em vez de aleatoriedade
    var cleanup_counter := GetGlobalProperty("transaction_cleanup_counter");
    if (!cleanup_counter)
        cleanup_counter := 0;
    endif
    
    cleanup_counter += 1;
    if (cleanup_counter >= 20)  // A cada 20 transações
        LimparTransacoesAntigas(df);
        cleanup_counter := 0;
    endif
    
    SetGlobalProperty("transaction_cleanup_counter", cleanup_counter);
    
    return 1;
endfunction


/*
 * DesfazerTransacao(transaction_id)
 * 
 * Purpose: Desfaz uma transação em caso de falha
 * Parameters:
 *   transaction_id - ID da transação a ser desfeita
 * Return: 1 para sucesso, 0 para falha
 */
function DesfazerTransacao(transaction_id)
    var df := SafeOpenDataFile(":faccao:transacoes");
    if (!df)
        return 0; // Erro já logado pela função SafeOpenDataFile
    endif
    
    var elem := SafeFindElement(df, transaction_id);
    if (!elem)
        return 0; // Erro já logado pela função SafeFindElement
    endif
    
    var operacao := GetElementProp(elem, "operacao");
    var dados := GetElementProp(elem, "dados");
    
    LogInfo("transacao", "Desfazendo transação: " + transaction_id);
    
    // Apenas suportar operações críticas específicas
    var resultado := 0;
    case (operacao)
        "transferir_territorio": 
            resultado := DesfazerTransferenciaTerritorio(dados);
        default: 
            LogWarning("transacao", "Tipo de operação desconhecido: " + operacao);
    endcase
    
    // Marcar status
    SetElementProp(elem, "status", "desfeita");
    SetElementProp(elem, "timestamp_desfazer", polcore().systime);
    
    return resultado;
endfunction

/*
 * DesfazerTransferenciaTerritorio(dados)
 *
 * Purpose: Desfaz a transferência de território em caso de falha
 * Parameters:
 *   dados - Dados da transação salva
 * Return:
 *   1 para sucesso, 0 para falha
 */
function DesfazerTransferenciaTerritorio(dados)
    if (!dados)
        LogError("territorio", "DesfazerTransferenciaTerritorio: Dados de transação não fornecidos");
        return 0;
    endif
    
    LogWarning("territorio", "Iniciando rollback de transferência de território: " + dados.territorio_id);
    
    var territorio_id := dados.territorio_id;
    var faccao_original := dados.faccao_original;
    
    // 1. Restaurar a facção original no datafile
    if (!TM_SetTerritoryFaction(territorio_id, faccao_original))
        LogError("territorio", "DesfazerTransferenciaTerritorio: Falha ao restaurar facção original no datafile");
        return 0;
    endif
    
    // 2. Remover objetos criados para a nova facção
    TM_DeleteTerritoryObjects(territorio_id, dados.faccao_nova);
    
    // 3. Restaurar os dados originais de seriais, se existirem
    if (dados.seriais_originais)
        // NOVA VERSÃO: Restaurar no elemento específico do território
        var seriais_df := SafeOpenDataFile(":faccao:seriais", 1);
        if (seriais_df)
            // Criar/Encontrar o elemento para este território
            var territory_elem := SafeFindElement(seriais_df, territorio_id, 1); // 1 = criar
            if (territory_elem)
                // Restaurar as propriedades do elemento
                foreach key in (dados.seriais_originais.keys())
                    SetElementProp(territory_elem, key, dados.seriais_originais[key]);
                    sleepms(1);
                endforeach
                
                LogInfo("territorio", "Seriais originais restaurados para território: " + territorio_id);
            else
                LogError("territorio", "Falha ao criar/encontrar elemento para restaurar seriais: " + territorio_id);
            endif
            
            // LEGADO: Também restaurar no formato antigo
            var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
            if (!territorios_seriais)
                territorios_seriais := dictionary{};
            endif
            
            territorios_seriais[territorio_id] := dados.seriais_originais;
            SetElementProp(seriais_df, "territorios", territorios_seriais);
            LogDebug("territorio", "Registro de seriais legado restaurado para: " + territorio_id);
        endif
    endif
    
    // 4. Restaurar objetos originais, se possível
    if (dados.seriais_originais)
        var center := TM_GetTerritoryCenter(territorio_id);
        if (center)
            // Se havia um baú original, tentar recriar
            if (dados.seriais_originais.bau)
                var orig_bau := SystemFindObjectBySerial(dados.seriais_originais.bau);
                if (!orig_bau)
                    // Recriar o baú
                    var bau := CreateItemAtLocation(center.x, center.y, center.z, 0xFF11, 1, center.realm);
                    if (bau)
                        bau.movable := 0;
                        SetObjProperty(bau, OBJ_FACTION_PROP, faccao_original);
                        SetObjProperty(bau, "FactionName", faccao_original);
                        SetObjProperty(bau, TERRITORY_ID_PROP, territorio_id);
                        
                        var chest_id := faccao_original + "_" + territorio_id;
                        SetObjProperty(bau, CHEST_ID_PROP, chest_id);
                        
                        var nome_bau := "Baú de Doações de " + faccao_original;
                        if (territorio_id)
                            nome_bau += " - " + territorio_id;
                        endif
                        SetName(bau, nome_bau);
                        
                        bau.usescript := ":faccao:baudoacao/use";
                        SetObjProperty(bau, "OnInsertScript", ":faccao:baudoacao/onInsert");
                        SetObjProperty(bau, "OnRemoveScript", ":faccao:baudoacao/onRemove");
                        SetObjProperty(bau, "CanInsertScript", ":faccao:baudoacao/canInsert");
                        SetObjProperty(bau, "CanRemoveScript", ":faccao:baudoacao/canRemoveDonation");
                        
                        LogInfo("territorio", "Baú recriado para território: " + territorio_id);
                    endif
                endif
            endif
            
            // Se havia uma mesa original, tentar recriar
            if (dados.seriais_originais.mesa)
                var orig_mesa := SystemFindObjectBySerial(dados.seriais_originais.mesa);
                if (!orig_mesa)
                    // Recriar a mesa
                    var mesa := CreateItemAtLocation(center.x+1, center.y, center.z, 0xFE5C, 1, center.realm);
                    if (mesa)
                        mesa.movable := 0;
                        SetObjProperty(mesa, OBJ_FACTION_PROP, faccao_original);
                        SetObjProperty(mesa, "FactionName", faccao_original);
                        SetObjProperty(mesa, TERRITORY_ID_PROP, territorio_id);
                        
                        var nome_mesa := "Mesa de Gestão de " + faccao_original;
                        if (territorio_id)
                            nome_mesa += " - " + territorio_id;
                        endif
                        SetName(mesa, nome_mesa);
                        
                        mesa.usescript := ":faccao:gestao_use";
                        
                        LogInfo("territorio", "Mesa recriada para território: " + territorio_id);
                    endif
                endif
            endif
            
            // Se havia um mural original, tentar recriar
            if (dados.seriais_originais.mural)
                var orig_mural := SystemFindObjectBySerial(dados.seriais_originais.mural);
                if (!orig_mural)
                    // Recriar o mural
                    var mural := CreateItemAtLocation(center.x, center.y+1, center.z, 0x46AE, 1, center.realm);
                    if (mural)
                        mural.movable := 0;
                        SetObjProperty(mural, OBJ_FACTION_PROP, faccao_original);
                        SetObjProperty(mural, "FactionName", faccao_original);
                        SetObjProperty(mural, TERRITORY_ID_PROP, territorio_id);
                        
                        var nome_mural := "Mural de Recrutamento de " + faccao_original;
                        if (territorio_id)
                            nome_mural += " - " + territorio_id;
                        endif
                        SetName(mural, nome_mural);
                        
                        mural.usescript := ":faccao:faccao";
                        
                        LogInfo("territorio", "Mural recriado para território: " + territorio_id);
                    endif
                endif
            endif
        endif
    endif
    
    // 5. Notificar o controlador de recursos sobre o rollback
    TM_NotifyTerritoryChange(territorio_id, "rollback", struct{
        "old_faction" := dados.faccao_nova,
        "restored_faction" := faccao_original
    });
    
    LogInfo("territorio", "Rollback concluído. Território " + territorio_id + " restaurado para facção original: " + 
                           faccao_original);
    
    return 1;
endfunction

/*
 * VerificarTransacoesPendentes()
 * 
 * Purpose: Verifica e processa transações pendentes
 * Return: Número de transações processadas
 */
function VerificarTransacoesPendentes()
    var df := SafeOpenDataFile(":faccao:transacoes");
    if (!df)
        return 0; // Erro já logado pela função SafeOpenDataFile
    endif
    
    var now := polcore().systime;
    var count := 0;
    var timeout := 300;  // 5 minutos
    
    foreach key in (df.Keys())
        var elem := SafeFindElement(df, key);
        if (!elem)
            continue;
        endif
        
        var status := GetElementProp(elem, "status");
        var timestamp := GetElementProp(elem, "timestamp");
        
        // Processar apenas transações iniciadas e expiradas
        if (status == "iniciada" && (now - timestamp) > timeout)
            LogWarning("transacao", "Transação pendente expirada: " + key);
            
            // Tentar desfazer a transação
            DesfazerTransacao(key);
            count += 1;
        endif
        
        sleepms(2);
    endforeach
    
    // Contador global para limpeza
    var verify_counter := GetGlobalProperty("transaction_verify_counter");
    if (!verify_counter)
        verify_counter := 0;
    endif
    
    verify_counter += 1;
    if (verify_counter >= 5)  // A cada 5 verificações
        LimparTransacoesAntigas(df);
        verify_counter := 0;
    endif
    
    SetGlobalProperty("transaction_verify_counter", verify_counter);
    
    return count;
endfunction

/*
 * LimparTransacoesAntigas(df)
 * 
 * Purpose: Remove transações concluídas/desfeitas antigas
 * Parameters:
 *   df - Datafile de transações
 * Return: Número de transações removidas
 */
function LimparTransacoesAntigas(df)
    if (!df)
        return 0;
    endif
    
    var now := polcore().systime;
    var count := 0;
    var limite_idade := 604800;  // 7 dias em segundos
    
    foreach key in (df.Keys())
        var elem := SafeFindElement(df, key);
        if (!elem)
            continue;
        endif
        
        var status := GetElementProp(elem, "status");
        var timestamp := GetElementProp(elem, "timestamp");
        
        // Remover apenas transações concluídas ou desfeitas antigas
        if ((status == "concluida" || status == "desfeita") && 
            (now - timestamp) > limite_idade)
            if (DeleteDatafileEntry(":faccao:transacoes", key))
                count += 1;
            endif
        endif
        
        sleepms(2);
    endforeach
    
    if (count > 0)
        LogInfo("transacao", "Limpeza: removidas " + count + " transações antigas");
    endif
    
    return count;
endfunction
/*
 * RemoveTerritoryFromGuilds(territory_name, territory_id)
 *
 * Purpose: Remover um território deletado de todas as guildas que possam referenciá-lo
 * Parameters:
 *   territory_name - Nome do território
 *   territory_id - ID do território (opcional)
 * Return: Número de guildas atualizadas
 */
function RemoveTerritoryFromGuilds(territory_name, territory_id := "")
    LogDebug("territorio", "Iniciando limpeza de referências ao território: " + territory_name);
    
    var guilds_updated := 0;
    var guilds := ListGuilds();
    
    foreach guild_id in guilds
        var guild := FindGuild(guild_id);
        if (!guild)
            continue;
        endif
        
       var territories := guild.GetProp("territories");
if (!territories || territories == error || TypeOf(territories) != "Array")
    territories := array{};
endif

// Garantir que seja um array limpo
var cleaned_territories := array{};
foreach terr in territories
    // Apenas adicionar itens válidos
    if (TypeOf(terr) == "String" && terr != "")
        cleaned_territories.append(terr);
    endif
    sleepms(2);
endforeach
territories := cleaned_territories;
        if (!territories || typeof(territories) != "Array")
            continue;
        endif
        
        // Indicar se houve alteração na lista
        var changed := 0;
        
        // Criar uma nova lista sem o território removido
        var new_territories := array{};
        
        foreach terr in territories
            // Verificar diferentes formatos de comparação (nome, ID, estrutura)
            var skip := 0;
            
            if (typeof(terr) == "String" && (terr == territory_name || (territory_id && terr == territory_id)))
                skip := 1;
            elseif (typeof(terr) == "Struct" && ((terr.name && terr.name == territory_name) || (territory_id && terr.id && terr.id == territory_id)))
                skip := 1;
            endif
            
            // Se não deve pular, manter na nova lista
            if (!skip)
                new_territories.append(terr);
            else
                changed := 1;
                LogInfo("territorio", "Removendo território \"" + territory_name + "\" da facção: " + guild.GetProp("name"));
            endif
            
            sleepms(2);
        endforeach
        
        // Se houve alteração, atualizar a lista
        if (changed)
            guild.SetProp("territories", new_territories);
            guilds_updated += 1;
        endif
        
        sleepms(2);
    endforeach
    
    LogInfo("territorio", "Limpeza concluída. Atualizadas " + guilds_updated + " guildas.");

    return guilds_updated;
endfunction


/*
 * ClassificarRecurso(resource)
 *
 * Purpose: Função unificada para categorizar recursos (string, objtype ou item)
 * Parameters: resource - Pode ser um ItemRef, String ou Integer (objtype)
 * Return: String com a categoria do recurso (comida, metal, etc.)
 */
function ClassificarRecurso(resource)
    // Verificar tipo do parâmetro para determinar abordagem
    if (!resource)
        return "desconhecido";
    endif
    
    // Se for um item/objeto
    if (TypeOf(resource) == "ItemRef")
        // Verificar propriedades explícitas primeiro
        if (GetObjProperty(resource, DONATION_PREFIX + RESOURCE_TYPE_COMIDA))
            return RESOURCE_TYPE_COMIDA;
        elseif (GetObjProperty(resource, DONATION_PREFIX + RESOURCE_TYPE_MOEDAS))
            return RESOURCE_TYPE_MOEDAS;
        elseif (GetObjProperty(resource, DONATION_PREFIX + RESOURCE_TYPE_COURO))
            return RESOURCE_TYPE_COURO;
        elseif (GetObjProperty(resource, DONATION_PREFIX + RESOURCE_TYPE_METAL))
            return RESOURCE_TYPE_METAL;
        elseif (GetObjProperty(resource, DONATION_PREFIX + RESOURCE_TYPE_MADEIRA))
            return RESOURCE_TYPE_MADEIRA;
        elseif (GetObjProperty(resource, DONATION_PREFIX + RESOURCE_TYPE_PANO))
            return RESOURCE_TYPE_PANO;
        elseif (GetObjProperty(resource, DONATION_PREFIX + RESOURCE_TYPE_PEDRA))
            return RESOURCE_TYPE_PEDRA;
        elseif (GetObjProperty(resource, DONATION_PREFIX + RESOURCE_TYPE_JOIAS))
            return RESOURCE_TYPE_JOIAS;
        endif
        
        // Se não tem propriedade explícita, usar a categorização pelo objtype
        return ClassificarPorObjtype(resource.objtype);
    endif
    
    // Se for uma string
    if (TypeOf(resource) == "String")
        return ClassificarPorNome(resource);
    endif
    
    // Se for um número (objtype)
    if (TypeOf(resource) == "Integer")
        return ClassificarPorObjtype(resource);
    endif
    
    return "desconhecido";
endfunction

// Funções auxiliares privadas para a categorização
function ClassificarPorNome(resource_name)
    // Normalizar o nome
    resource_name := Lower(Trim(resource_name));
    
    var categorias := dictionary{
        "metal" := array{"ferro", "prata", "ouro", "cobre", "bronze", "metal", "ingot", "ore", "ironore", "prataingot", "acoingot"},
        "couro" := array{"couro", "leather", "hide", "pele", "escamoso", "polar", "couroespesso", "couroescamoso", "couropolar", "courodraconiano", "escamasazuis"},
        "madeira" := array{"madeira", "wood", "log", "tora", "carvao", "carvão", "coal", "madeirapinheiro", "madeirateixo"},
        "comida" := array{"comida", "food", "peixe", "fish", "carne", "meat", "pão", "bread", "cogumelo"},
        "pano" := array{"pano", "cloth", "tecido", "escamas", "scales", "seda", "silk", "pelebranca"},
        "pedras" := array{"pedra", "stone", "rock", "osso", "bone", "areia", "sand", "ossogigante", "ossodemoniaco"},
        "joias" := array{"joia", "jewel", "gem", "vidro", "glass", "crystal"},
        "moedas" := array{"moeda", "coin", "dinheiro", "money", "currency"}
    };
    
    // Verificar cada categoria
    foreach categoria in (categorias.keys())
        foreach palavra in (categorias[categoria])
            if (resource_name.find(palavra) != error)
                return categoria;
            endif
        endforeach
    endforeach
    
    return "desconhecido";
endfunction


function ClassificarPorObjtype(objtype)
    // Mapeamento direto de objtypes comuns
    var objtype_map := dictionary{
        // Metais
        0x6300 := "metal",     // Iron ore
        0x6993 := "metal",     // Silver ingot
        0x6995 := "metal",     // Steel ingot
        0x1BEF := "metal",     // Iron ingot
        0x1BF2 := "metal",     // Aço
        
        // Couro
        0xef12 := "couro",     // Couro espesso
        0xef17 := "couro",     // Couro polar
        0x1078 := "couro",     // Couro comum
        0xef11 := "couro",     // Couro escamoso
        0xed20 := "couro",     // Couro draconiano
        0xef36 := "couro",     // Escamas azuis
        
        // Madeira
        0xee36 := "madeira",   // Madeira de pinheiro
        0xee3e := "madeira",   // Madeira de teixo
        0x1BD7 := "madeira",   // Toras
        0x1BDE := "madeira",   // Carvão
        
        // Comida
        0x097B := "comida",    // Peixe
        0x09B7 := "comida",    // Carne
        0x09D2 := "comida",    // Queijo
        0x0D16 := "comida",    // Cogumelo
        
        // Tecidos
        0x1766 := "pano",      // Pano
        0xee52 := "pano",      // Pele branca
        
        // Pedras/Ossos
        0x1BDD := "pedras",    // Pedra
        0x0F7E := "pedras",    // Osso
        
        // Joias/Vidro
        0x1BE3 := "joias",     // Vidro
        0x1F13 := "joias",     // Joia
        
        // Moedas
        0x0EED := "moedas"     // Moedas de ouro
    };
    
    // Verificar mapeamento direto
    if (objtype_map.exists(objtype))
        return objtype_map[objtype];
    endif
    
    // Categorização por faixas de objtype
    if (objtype >= 0x1BEE && objtype <= 0x1BF5)
        return "metal"; // Faixa de ingots
    elseif (objtype >= 0x1078 && objtype <= 0x1079)
        return "couro"; // Faixa de couros
    elseif (objtype >= 0x097B && objtype <= 0x09EB)
        return "comida"; // Faixa de comidas
    endif
    
    return "desconhecido";
endfunction

function FindFactionChest(faction_name, territory_id := "", create_if_missing := 0)
    if (!faction_name)
        LogError("recursos", "FindFactionChest() chamado sem nome de facção");
        return 0;
    endif
    
    LogDebug("recursos", "FindFactionChest: Buscando baú para facção '" + faction_name + 
             (territory_id ? "', território '" + territory_id + "'" : "'"));
    
    var chest := 0;
    
    // ESTRATÉGIA 1: Busca específica pelo território no formato novo (elemento específico)
    if (territory_id)
        var seriais_df := SafeOpenDataFile(":faccao:seriais");
        if (seriais_df)
            // Buscar pelo elemento específico do território (formato novo)
            var territory_elem := SafeFindElement(seriais_df, territory_id);
            if (territory_elem)
                LogDebug("recursos", "Elemento do território encontrado no datafile");
                
                // Verificar se a facção corresponde
                var elem_faction := GetElementProp(territory_elem, "faction");
                if (elem_faction == faction_name)
                    LogDebug("recursos", "Facção corresponde, verificando serial do baú");
                    
                    // Obter serial do baú e verificá-lo
                    var chest_serial := GetElementProp(territory_elem, "bau");
                    if (chest_serial)
                        LogDebug("recursos", "Serial do baú encontrado: " + chest_serial);
                        
                        chest := SystemFindObjectBySerial(chest_serial);
                        if (chest)
                            LogDebug("recursos", "Baú encontrado com sucesso usando elemento de território");
                            // Verificar se o baú está acessível e pertence à facção correta
                            if (IsValidContainer(chest) && GetObjProperty(chest, "faction") == faction_name)
                                return chest;
                            else
                                LogWarning("recursos", "Baú encontrado, mas é inválido ou facção não corresponde");
                                chest := 0; // Resetar para continuar com outras estratégias
                            endif
                        else
                            LogWarning("recursos", "Serial do baú válido, mas objeto não encontrado");
                        endif
                    endif
                else
                    LogDebug("recursos", "Facção não corresponde. Esperado: " + faction_name + ", Encontrado: " + elem_faction);
                endif
            else
                LogDebug("recursos", "Elemento do território não encontrado no datafile");
            endif
            
            // ESTRATÉGIA 2: Verificar no formato antigo (dicionário territorios)
            var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
            if (territorios_seriais && territorios_seriais[territory_id])
                LogDebug("recursos", "Registro encontrado no formato antigo");
                
                var info := territorios_seriais[territory_id];
                if (info.faction == faction_name && info.bau)
                    LogDebug("recursos", "Facção corresponde, verificando serial do baú (formato antigo)");
                    
                    chest := SystemFindObjectBySerial(info.bau);
                    if (chest && IsValidContainer(chest))
                        LogDebug("recursos", "Baú encontrado com sucesso usando formato antigo");
                        
                        // Atualizar para o novo formato enquanto estamos aqui
                        var territory_elem := SafeFindElement(seriais_df, territory_id, 1); // 1 = criar
                        if (territory_elem)
                            SetElementProp(territory_elem, "bau", info.bau);
                            SetElementProp(territory_elem, "faction", info.faction);
                            if (info.mesa) 
                                SetElementProp(territory_elem, "mesa", info.mesa);
                            endif
                            if (info.mural) 
                                SetElementProp(territory_elem, "mural", info.mural);
                            endif
                            LogInfo("recursos", "Migrado registro de baú para novo formato");
                        endif
                        
                        return chest;
                    endif
                endif
            endif
        endif
    endif
    
    // ESTRATÉGIA 3: Busca no mundo por qualquer baú da facção e território
    LogDebug("recursos", "Buscando baús no mundo");
    
    // 3.1: Primeiro, buscar baú que corresponda exatamente à facção e território
    if (territory_id)
        foreach item in EnumerateItemsInContainer(0) // 0 = raiz do mundo
            if ((item.objtype == 0xFF11 || item.objtype == 0xBA63) && 
                GetObjProperty(item, "faction") == faction_name &&
                GetObjProperty(item, "territory_id") == territory_id)
                
                LogInfo("recursos", "Baú com facção e território correspondentes encontrado no mundo: " + item.serial);
                
                // Atualizar registro no datafile
                var seriais_df := SafeOpenDataFile(":faccao:seriais", 1);
                if (seriais_df)
                    var territory_elem := SafeFindElement(seriais_df, territory_id, 1);
                    if (territory_elem)
                        SetElementProp(territory_elem, "bau", item.serial);
                        SetElementProp(territory_elem, "faction", faction_name);
                    endif
                endif
                
                return item;
            endif
            sleepms(1);
        endforeach
    endif
    
    // 3.2: Buscar qualquer baú da facção
    var generic_chest := 0;
    var neutral_chest := 0; // Baú sem território atribuído
    
    foreach item in EnumerateItemsInContainer(0)
        if ((item.objtype == 0xFF11 || item.objtype == 0xBA63) && 
            GetObjProperty(item, "faction") == faction_name)
            
            generic_chest := item; // Guardar qualquer baú da facção
            
            // Verificar se não tem território atribuído
            if (!GetObjProperty(item, "territory_id"))
                neutral_chest := item;
                LogDebug("recursos", "Baú neutro (sem território) encontrado: " + item.serial);
                break; // Preferir um baú neutro
            endif
            
            sleepms(1);
        endif
    endforeach
    
    // Preferir baú neutro se encontrado
    if (neutral_chest)
        LogInfo("recursos", "Usando baú neutro encontrado: " + neutral_chest.serial);
        
        // Se fornecido um território, atribuir ao baú
        if (territory_id)
            SetObjProperty(neutral_chest, "territory_id", territory_id);
            
            // Atualizar registro no datafile
            var seriais_df := SafeOpenDataFile(":faccao:seriais", 1);
            if (seriais_df)
                var territory_elem := SafeFindElement(seriais_df, territory_id, 1);
                if (territory_elem)
                    SetElementProp(territory_elem, "bau", neutral_chest.serial);
                    SetElementProp(territory_elem, "faction", faction_name);
                endif
            endif
            
            LogInfo("recursos", "Território " + territory_id + " atribuído ao baú neutro");
        endif
        
        return neutral_chest;
    endif
    
    // Se não encontrou baú neutro, usar qualquer baú da facção
    if (generic_chest)
        LogInfo("recursos", "Usando baú genérico da facção: " + generic_chest.serial);
        
        // Se fornecido um território, atribuir ao baú
        if (territory_id && GetObjProperty(generic_chest, "territory_id") != territory_id)
            LogWarning("recursos", "Baú já possui outro território. Atual: " + 
                      GetObjProperty(generic_chest, "territory_id") + ", Solicitado: " + territory_id);
        endif
        
        return generic_chest;
    endif
    
    // ESTRATÉGIA 4: Criar baú de emergência se solicitado
    if (create_if_missing)
        LogWarning("recursos", "Criando baú de emergência para " + faction_name);
        chest := CreateEmergencyDonationChest(faction_name);
        
        if (chest)
            // Atribuir território ao baú criado, se fornecido
            if (territory_id)
                SetObjProperty(chest, "territory_id", territory_id);
                LogInfo("recursos", "Território " + territory_id + " atribuído ao baú de emergência");
                
                // Registrar no datafile
                var seriais_df := SafeOpenDataFile(":faccao:seriais", 1);
                if (seriais_df)
                    var territory_elem := SafeFindElement(seriais_df, territory_id, 1);
                    if (territory_elem)
                        SetElementProp(territory_elem, "bau", chest.serial);
                        SetElementProp(territory_elem, "faction", faction_name);
                    endif
                endif
            endif
            
            return chest;
        else
            LogCritical("recursos", "Falha ao criar baú de emergência");
        endif
    endif
    
    LogError("recursos", "Nenhum baú encontrado para facção: " + faction_name + 
            (territory_id ? " (Território: " + territory_id + ")" : ""));
    
    return 0;
endfunction


// Função para criar um baú de emergência se nenhum for encontrado
function CreateEmergencyDonationChest(faction_name)
    LogWarning("recursos", "Tentando criar baú de emergência para " + faction_name);

    
    // Local de emergência para criar o baú (geralmente um lugar seguro)
    var x := 5288;
    var y := 1176;
    var z := 0;
    var realm := "britannia";
    
    // Criar o baú
    var chest := CreateItemAtLocation(x, y, z, 0xff11, 1, realm);
    if (!chest)
        LogError("recursos", "Falha ao criar baú de emergência");
        return 0;
    endif
    
    // Configurar o baú
    chest.name := "Baú de Doações de " + faction_name + " [Emergência]";
    SetObjProperty(chest, "faction", faction_name);
    SetObjProperty(chest, "faction_chest_id", faction_name + "_emergency_" + polcore().systime);
    SetObjProperty(chest, "is_emergency_chest", 1);
    SetObjProperty(chest, "created_time", polcore().systime);
    
    // Definir scripts
    chest.usescript := ":faccao:baudoacao/use";
    SetObjProperty(chest, "OnInsertScript", ":faccao:baudoacao/onInsert");
    SetObjProperty(chest, "OnRemoveScript", ":faccao:baudoacao/onRemove");
    SetObjProperty(chest, "CanInsertScript", ":faccao:baudoacao/canInsert");
    SetObjProperty(chest, "CanRemoveScript", ":faccao:baudoacao/canRemoveDonation");
    
    // Registrar o baú
    UpdateGuildChestRecord(faction_name, chest);
    
    // Registrar um aviso no log
    LogWarning("recursos", "Baú de emergência criado para " + faction_name + " em " + x + "," + y + "," + z + " " + realm);

    
    return chest;
endfunction


// Função auxiliar para registrar baú no datafile
function RegisterDonationChest(faction_name, chest)
    var df := SafeOpenDataFile("faction_chests", 1);
    var elem := SafeFindElement(df, faction_name, 1);
    
    SetElementProp(elem, "chest_serial", chest.serial);
    SetElementProp(elem, "location", struct{
        "x" := chest.x,
        "y" := chest.y,
        "z" := chest.z,
        "realm" := chest.realm
    });
    SetElementProp(elem, "last_update", polcore().systime);
    
    return 1;
endfunction

// Função auxiliar para atualizar registro na guilda
function UpdateGuildChestRecord(faction_name, chest)
    var guild := FindGuildByName(faction_name);
    if (!guild)
        return 0;
    endif
    
    var guild_obj := FindGuild(guild);
    if (!guild_obj)
        return 0;
    endif
    
    guild_obj.SetProp("donation_chest", chest.serial);
    return 1;
endfunction