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
 * SetTerritoryAsCapital(territory_id, faction_name)
 *
 * Purpose: Define um território como capital de uma facção
 * Parameters:
 *   territory_id - ID do território
 *   faction_name - Nome da facção
 * Return:
 *   1 para sucesso, 0 para falha
 */
function SetTerritoryAsCapital(territory_id, faction_name)
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        return 0;
    endif
    
    // Primeiro, remover flag de capital de todos os territórios desta facção
    var keys := rect_df.keys();
    if (keys && keys != error)
        foreach elem_name in keys
            var elem := SafeFindElement(rect_df, elem_name);
            if (elem)
                var faction := GetElementProp(elem, TERRITORY_FACTION_PROP);
                if (!faction)
                    faction := GetElementProp(elem, "Faction"); // Compatibilidade
                endif
                
                var is_capital := GetElementProp(elem, TERRITORY_IS_CAPITAL_PROP);
                if (faction == faction_name && is_capital)
                    elem.EraseProp(TERRITORY_IS_CAPITAL_PROP);
                endif
            endif
            sleepms(2);
        endforeach
    endif
    
    // Encontrar o território alvo e defini-lo como capital
    var territory_elem := FindTerritoryByID(territory_id);
    if (territory_elem)
        SetElementProp(territory_elem, TERRITORY_IS_CAPITAL_PROP, 1);
        return 1;
    endif
    
    return 0;
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

function IsTerritoryCapital(territory_id)
    var territory_elem := FindTerritoryByID(territory_id);
    if (territory_elem)
        var prop := GetElementProp(territory_elem, TERRITORY_IS_CAPITAL_PROP);
        if (prop)
            return 1;
        endif
    endif
    
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


// Função auxiliar para criação segura de itens
function SafeCreateItemInContainer(container, objtype, amount, properties := 0)
    if (!container || !objtype || amount <= 0)
        return 0;
    endif
    
    LogDebug("criacao_item", $"Tentando criar item 0x{Hex(objtype)} x{amount} em contêiner {container.serial}");
    
    // ABORDAGEM 1: Verificar se já existe um item do mesmo tipo para empilhar
    var existing_item := FindExistingStackableItem(container, objtype);
    
    if (existing_item)
        LogDebug("criacao_item", "Item existente encontrado, tentando empilhar");
        
        // Verificar limite de empilhamento
        var old_amount := existing_item.amount;
        var new_amount := old_amount + amount;
        
        if (new_amount <= 60000) // Limite máximo de pilha
            // Tentar aumentar a quantidade diretamente
            var before_amount := existing_item.amount;
            existing_item.amount := new_amount;
            
            // Verificar se o aumento foi bem-sucedido
            if (existing_item.amount == new_amount)
                // Atualizar propriedades se fornecidas
                if (properties)
                    foreach prop_name in (properties.keys())
                        SetObjProperty(existing_item, prop_name, properties[prop_name]);
                        sleepms(1);
                    endforeach
                endif
                
                LogDebug("criacao_item", $"Item empilhado com sucesso, nova quantidade: {existing_item.amount}");
                return existing_item;
            else
                // Restaurar quantidade original se falhou
                existing_item.amount := before_amount;
                LogWarning("criacao_item", "Falha ao aumentar quantidade, tentando método alternativo");
            endif
        endif
        
        // ABORDAGEM 2: Se não conseguiu empilhar diretamente, criar um item substituto
        var replacement := ReplaceStackWithNew(existing_item, amount, properties);
        if (replacement)
            LogDebug("criacao_item", "Item substituído com sucesso");
            return replacement;
        endif
    endif
    
    // ABORDAGEM 3: Criar novo item
    LogDebug("criacao_item", "Criando novo item");
    var new_item := CreateItemInContainer(container, objtype, amount);
    
    if (new_item)
        // Aplicar propriedades se fornecidas
        if (properties)
            foreach prop_name in (properties.keys())
                SetObjProperty(new_item, prop_name, properties[prop_name]);
                sleepms(1);
            endforeach
        endif
        
        LogDebug("criacao_item", $"Item criado com sucesso: {new_item.serial}");
        return new_item;
    endif
    
    LogError("criacao_item", "Falha ao criar item");
    return 0;
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
 * Desfaz uma transferência de território incompleta
 */
function DesfazerTransferenciaTerritorio(dados)
    if (!dados)
        LogWarning("transacao", "Tentativa de desfazer transferência com dados nulos");
        return 0;
    endif
    
    LogDebug("transacao", $"Desfazendo transferência de território");
    LogDebug("transacao", $"Território: {dados.territorio_id}");
    LogDebug("transacao", $"Facção original: {dados.faccao_original}");
    
    var territorio_id := dados.territorio_id;
    var faccao_original := dados.faccao_original;
    
    // 1. Restaurar facção original no território
    var elem := FindTerritoryByID(territorio_id);
    if (!elem)
        LogError("transacao", $"Território {territorio_id} não encontrado");
        return 0;
    endif
    
    SetElementProp(elem, TERRITORY_FACTION_PROP, faccao_original);
    SetElementProp(elem, "Faction", faccao_original); // Para compatibilidade
    
    // 2. Restaurar objetos originais se informação disponível
    if (dados.seriais_originais)
        var seriais_df := SafeOpenDataFile(":faccao:seriais");
        if (seriais_df)
            var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
            if (territorios_seriais)
                territorios_seriais[territorio_id] := dados.seriais_originais;
                SetElementProp(seriais_df, "territorios", territorios_seriais);
                LogInfo("transacao", $"Seriais originais restaurados para território {territorio_id}");
            endif
        endif
    endif
    
    // 3. Restaurar lista de territórios da facção original
    var guild_original := FindGuildByName(faccao_original);
    if (guild_original)
        var guild_obj := FindGuild(guild_original);
        if (guild_obj)
            var territories := guild_obj.GetProp("territories") ?: array{};
            
            if (!(territorio_id in territories))
                territories.append(territorio_id);
                guild_obj.SetProp("territories", territories);
                LogInfo("transacao", $"Território {territorio_id} restaurado para facção {faccao_original}");
            endif
        endif
    endif
    
    // 4. Remover da nova facção (se aplicável)
    if (dados.faccao_nova)
        var guild_nova := FindGuildByName(dados.faccao_nova);
        if (guild_nova)
            var guild_obj := FindGuild(guild_nova);
            if (guild_obj)
                var territories := guild_obj.GetProp("territories");
                if (territories)
                    var new_territories := array{};
                    foreach terr in territories
                        if (terr != territorio_id)
                            new_territories.append(terr);
                        endif
                        sleepms(2);
                    endforeach
                    
                    guild_obj.SetProp("territories", new_territories);
                    LogInfo("transacao", $"Território {territorio_id} removido da facção {dados.faccao_nova}");
                endif
            endif
        endif
    endif
    
    LogInfo("transacao", $"Transferência de território {territorio_id} desfeita com sucesso");
    
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
        "metal" := array{"ferro", "prata", "ouro", "cobre", "bronze", "metal", "ingot", "ore"},
        "couro" := array{"couro", "leather", "hide", "pele", "escamoso", "polar"},
        "madeira" := array{"madeira", "wood", "log", "tora", "carvao", "carvão", "coal"},
        "comida" := array{"comida", "food", "peixe", "fish", "carne", "meat", "pão", "bread"},
        "pano" := array{"pano", "cloth", "tecido", "escamas", "scales", "seda", "silk"},
        "pedras" := array{"pedra", "stone", "rock", "osso", "bone", "areia", "sand"},
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
        
        // Couro
        0xef12 := "couro",     // Couro espesso
        0xef17 := "couro",     // Couro polar
        0x1078 := "couro",     // Couro comum
        
        // Madeira
        0xee36 := "madeira",   // Madeira de pinheiro
        0xee3e := "madeira",   // Madeira de teixo
        0x1BD7 := "madeira",   // Toras
        
        // Comida
        0x097B := "comida",    // Peixe
        0x09B7 := "comida",    // Carne
        0x09D2 := "comida",    // Queijo
        
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

/*
 * FindFactionChest(faction_name, territory_id := "", create_if_missing := 0)
 *
 * Purpose: Função unificada para encontrar baús de facção usando múltiplas estratégias
 * 
 * Parameters:
 *   faction_name - Nome da facção dona do baú
 *   territory_id - ID específico do território (opcional)
 *   create_if_missing - Se deve criar um baú de emergência caso não encontre (0 = não, 1 = sim)
 *
 * Return:
 *   Referência para o baú encontrado ou 0 se não encontrar
 */
function FindFactionChest(faction_name, territory_id := "", create_if_missing := 0)
    if (!faction_name)
        LogError("recursos", "FindFactionChest() chamado sem nome de facção");
        return 0;
    endif
    
    var chest := 0;
    
    // ESTRATÉGIA 1: Busca específica por território
    if (territory_id)
        LogDebug("recursos", "Buscando baú para facção: " + faction_name + ", território: " + territory_id);
        
        // 1.1: Verificar seriais registrados
        var seriais_df := SafeOpenDataFile(":faccao:seriais");
        if (seriais_df)
            var territorios_seriais := GetElementProp(seriais_df, "territorios");
            if (territorios_seriais && territorios_seriais[territory_id])
                var info := territorios_seriais[territory_id];
                if (info.bau)
                    chest := SystemFindObjectBySerial(info.bau);
                    if (chest && GetObjProperty(chest, "faction") == faction_name)
                        LogDebug("recursos", "Baú encontrado via seriais registrados: " + chest.serial);
                        return chest;
                    endif
                endif
            endif
        endif
        
        // 1.2: Buscar pelo território no mundo
        foreach item in (EnumerateItemsInContainer(0))
            if ((item.objtype == 0xff11 || item.objtype == 0xba63) && 
                GetObjProperty(item, "faction") == faction_name && 
                GetObjProperty(item, "territory_id") == territory_id)
                LogDebug("recursos", "Baú encontrado via busca no mundo por território: " + item.serial);
                // Atualizar registro
                RegisterDonationChest(faction_name, item);
                return item;
            endif
            sleepms(1);
        endforeach
    endif
    
    // ESTRATÉGIA 2: Busca pelo registro na guilda
    var guild := FindGuildByName(faction_name);
    if (guild)
        var guild_obj := FindGuild(guild);
        if (guild_obj)
            var chest_serial := guild_obj.GetProp("donation_chest");
            if (!chest_serial)
                chest_serial := guild_obj.GetProp(GUILD_CHEST_PROP); // Nome alternativo
            endif
            
            if (chest_serial)
                chest := SystemFindObjectBySerial(chest_serial);
                if (chest && GetObjProperty(chest, "faction") == faction_name)
                    LogDebug("recursos", "Baú encontrado via registro na guilda: " + chest.serial);
                    // Atualizar território se necessário
                    if (territory_id && !GetObjProperty(chest, "territory_id"))
                        SetObjProperty(chest, "territory_id", territory_id);
                    endif
                    return chest;
                endif
            endif
        endif
    endif
    
    // ESTRATÉGIA 3: Busca direta no datafile
    var df := SafeOpenDataFile("faction_chests");
    if (df)
        var elem := SafeFindElement(df, faction_name);
        if (elem)
            var chest_serial := GetElementProp(elem, "chest_serial");
            if (chest_serial)
                chest := SystemFindObjectBySerial(chest_serial);
                if (chest && GetObjProperty(chest, "faction") == faction_name)
                    LogDebug("recursos", "Baú encontrado via datafile: " + chest.serial);
                    // Atualizar território se necessário
                    if (territory_id && !GetObjProperty(chest, "territory_id"))
                        SetObjProperty(chest, "territory_id", territory_id);
                    endif
                    // Atualizar registro na guilda
                    UpdateGuildChestRecord(faction_name, chest);
                    return chest;
                endif
            endif
            
            // Verificar coordenadas salvas
            var loc := GetElementProp(elem, "location");
            if (loc)
                foreach item in ListItemsAtLocation(loc.x, loc.y, loc.z, loc.realm)
                    if ((item.objtype == 0xff11 || item.objtype == 0xba63) && 
                        GetObjProperty(item, "faction") == faction_name)
                        LogDebug("recursos", "Baú encontrado por localização: " + item.serial);
                        // Atualizar registros
                        SetElementProp(elem, "chest_serial", item.serial);
                        if (territory_id && !GetObjProperty(item, "territory_id"))
                            SetObjProperty(item, "territory_id", territory_id);
                        endif
                        UpdateGuildChestRecord(faction_name, item);
                        return item;
                    endif
                    sleepms(1);
                endforeach
            endif
        endif
    endif
    
    // ESTRATÉGIA 4: Busca completa no mundo
    LogDebug("recursos", "Realizando busca completa no mundo para " + faction_name);
    foreach item in EnumerateItemsInContainer(0)
        if ((item.objtype == 0xff11 || item.objtype == 0xba63) && 
            GetObjProperty(item, "faction") == faction_name)
            // Se temos ID do território e este baú tem outro ID, continuar buscando
            var item_territory := GetObjProperty(item, "territory_id");
            if (territory_id && item_territory && item_territory != territory_id)
                continue;
            endif
            
            LogDebug("recursos", "Baú encontrado via busca completa: " + item.serial);
            // Atualizar registros
            RegisterDonationChest(faction_name, item);
            UpdateGuildChestRecord(faction_name, item);
            return item;
        endif
        sleepms(1);
    endforeach
    
    // ESTRATÉGIA 5: Criar baú de emergência se solicitado
    if (create_if_missing)
        LogInfo("recursos", "Criando baú de emergência para " + faction_name);
        chest := CreateEmergencyDonationChest(faction_name);
        if (chest)
            if (territory_id)
                SetObjProperty(chest, "territory_id", territory_id);
            endif
            RegisterDonationChest(chest, faction_name);
            UpdateGuildChestRecord(faction_name, chest);
            return chest;
        endif
    endif
    
    LogWarning("recursos", "Nenhum baú encontrado para facção " + faction_name);
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