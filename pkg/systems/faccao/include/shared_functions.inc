use uo;
use os;
use cfgfile;
use datafile;
use guilds;

include ":datafile:datafile";
include ":faccao:faccao_constants";
include ":faccao:include/faccao_resources";
include ":timeutils:time";


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
    if (!container)
        Print("ERRO: ContarItensDoadosNoBau chamada com container nulo");
        return CreateEmptyResourceStruct();
    endif
    
    // Registrar início para depuração
    Print("DEBUG: ContarItensDoadosNoBau() iniciada para baú: " + container.serial);
    
    // Verificar se o cache está "sujo" (marcado para atualização)
    var cache_dirty := GetObjProperty(container, CACHE_DIRTY_FLAG);
    
// Verificar se tem cache válido e recente
var ultimo_update := GetObjProperty(container, CACHE_UPDATE_PROP);
var current_time := polcore().systime;
    
    // Usar cache se: 
    // 1. Não estamos forçando contagem
    // 2. O cache não está marcado como sujo
    // 3. O cache foi atualizado recentemente
if (!force_count && !cache_dirty && ultimo_update != error && ultimo_update && (current_time - ultimo_update < CACHE_EXPIRY_TIME))
        Print("DEBUG: Usando cache - última atualização há " + (current_time - ultimo_update) + " segundos");
        var recursos_cache := GetObjProperty(container, CACHE_PROP_NAME);
        if (recursos_cache)
            return recursos_cache;
        endif
    endif
    
    // Se chegou aqui, precisamos fazer uma contagem completa
    // Chamando a função centralizada em faccao_resources.inc
    Print("DEBUG: Realizando contagem completa dos recursos no baú");
    var recursos := ContarRecursosReais(container);
    
    // Comparar com o cache anterior para detectar inconsistências
    VerificarInconsistencias(container, recursos);
    
    // Atualizar o cache
    SetObjProperty(container, CACHE_PROP_NAME, recursos);
    SetObjProperty(container, CACHE_UPDATE_PROP, current_time);
    EraseObjProperty(container, CACHE_DIRTY_FLAG); // Limpar flag de sujeira
    
    Print("DEBUG: Cache atualizado no baú");
    
    // Verificar se precisamos atualizar o datafile global
    var ultima_sync := GetObjProperty(container, DATAFILE_SYNC_PROP);
    if (!ultima_sync || (current_time - ultima_sync > DATAFILE_UPDATE_INTERVAL) || force_count)
        // Usar a função de sincronização centralizada
        SincronizarComDataFile(container, recursos);
        SetObjProperty(container, DATAFILE_SYNC_PROP, current_time);
        Print("DEBUG: Datafile global atualizado");
    else
        Print("DEBUG: Atualizando datafile apenas no fechamento - última sync há " + (current_time - ultima_sync) + " segundos");
        AgendarAtualizacaoDataFile(container); // Agendar atualização futura
    endif
    
    return recursos;
endfunction


function GetFactionOfChest(chest)
    Print("DEBUG-FACTION: Iniciando verificação de facção para o baú: " + chest.desc);
    
    // CORREÇÃO 1: Forçar reset de cache se estiver com problema
    if (GetObjProperty(chest, "reset_faction_cache"))
        EraseObjProperty(chest, "cache_faction");
        EraseObjProperty(chest, "last_accessed");
        EraseObjProperty(chest, "reset_faction_cache");
        Print("DEBUG-FACTION: Cache resetado forçadamente");
    endif
    
    // Verificar cache primeiro para melhor performance
var cached_faction := GetObjProperty(chest, "cache_faction");
var last_accessed := GetObjProperty(chest, "last_accessed");
    
    // Se tiver cache recente, usar
if (cached_faction != error && cached_faction && last_accessed != error && last_accessed && (ReadGameClock() - last_accessed < 300))
        Print("DEBUG-FACTION: Usando cache: " + cached_faction);
        return cached_faction;
    endif
    
    // Tentar obter a facção do baú de várias formas
    var faction := "";
    
    // 1. Da propriedade OBJ_FACTION_PROP (definida no faccao_constants.inc)
    faction := GetObjProperty(chest, OBJ_FACTION_PROP);
    if (faction && faction != error)
        Print("DEBUG-FACTION: Encontrado via OBJ_FACTION_PROP: " + faction);
    else
        // 2. Da propriedade "FactionName" (suporte para compatibilidade)
        faction := GetObjProperty(chest, "FactionName");
        if (faction && faction != error)
            Print("DEBUG-FACTION: Encontrado via FactionName: " + faction);
        else
            // 3. Do ID único do baú (formato: facção:tipo:território)
            var unique_id := GetObjProperty(chest, UNIQUE_ID_PROP);
            if (unique_id)
                var parts := SplitWords(unique_id, ":");
                if (parts.size() >= 3)
                    faction := parts[1];
                    Print("DEBUG-FACTION: Encontrado via UNIQUE_ID_PROP: " + faction);
                endif
            endif
        endif
    endif
    
    // 4. Do faction_chest_id (formato: facção_território)
    if (!faction || faction == error)
        var chest_id := GetObjProperty(chest, CHEST_ID_PROP);
        if (chest_id)
            var parts := SplitWords(chest_id, "_");
            if (parts.size() >= 2)
                faction := parts[1];
                Print("DEBUG-FACTION: Encontrado via CHEST_ID_PROP: " + faction);
            endif
        endif
    endif
    
 // 5. Verificar seriais no datafile
if (!faction || faction == error)
    var seriais_df := OpenDataFile(":faccao:seriais");
    if (seriais_df != error)
        var territorios_seriais := seriais_df.GetProp("territorios");
        if (territorios_seriais != error && territorios_seriais)
            var keys := territorios_seriais.keys();
            if (keys && keys != error)
                foreach territorio in keys
                    if (!territorios_seriais.exists(territorio))
                        continue;
                    endif
                    
                    var info := territorios_seriais[territorio];
                    if (!info || TypeOf(info) != "Struct")
                        continue;
                    endif
                    
                    if ((info.exists("bau") && info.bau == chest.serial) || 
                        (info.exists("mesa") && info.mesa == chest.serial) || 
                        (info.exists("mural") && info.mural == chest.serial))
                        if (info.exists("faction"))
                            faction := info.faction;
                            Print("DEBUG-FACTION: Encontrado via datafile de seriais: " + faction);
                        endif
                        break;
                    endif
                    sleepms(1);
                endforeach
            endif
        endif
    endif
endif
    
    // 6. Verificar pelo nome do objeto (último recurso)
    if (!faction || faction == error)
        var nome := Lower(chest.name);
        if (nome.find("baú de doações de ") == 0)
            var resto := nome[18, len(nome)-17];
            var pos_traço := Find(resto, "-", 1);
            if (pos_traço)
                faction := Trim(resto[1, pos_traço-1]);
                Print("DEBUG-FACTION: Encontrado via nome do objeto: " + faction);
            else
                faction := Trim(resto);
                Print("DEBUG-FACTION: Encontrado via nome do objeto (sem traço): " + faction);
            endif
        endif
    endif
    
    // Armazenar em cache para futuras consultas
    if (faction && faction != error)
        SetObjProperty(chest, "cache_faction", faction);
        SetObjProperty(chest, "last_accessed", ReadGameClock());
        return faction;
    endif
    
    Print("DEBUG-FACTION: Não foi possível determinar a facção, usando padrão: " + KINGDOM_FACTION);
    return KINGDOM_FACTION; // Valor padrão se não encontrar
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
    var df := OpenDataFile(":faccao:guild_registry");
    if (!df)
        df := OpenDataFile("guild_registry");
    endif
    
    if (df)
        var elem := df.FindElement("guilds");
        if (elem)
            var registry := elem.GetProp("registry");
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
    var seriais_df := OpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territorios_seriais := seriais_df.GetProp("territorios");
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
    var rect_df := OpenDataFile(":architect:areas");
    if (!rect_df)
        return 0;
    endif
    
// Primeiro, remover flag de capital de todos os territórios desta facção
var keys := rect_df.keys();
if (keys && keys != error)
    foreach elem_name in keys
        var elem := rect_df.FindElement(elem_name);
        if (elem && elem != error)
            var faction := elem.GetProp(TERRITORY_FACTION_PROP);
            if (faction == error || !faction)
                faction := elem.GetProp("Faction"); // Compatibilidade
                if (faction == error)
                    faction := "";
                endif
            endif
            
            var is_capital := elem.GetProp(TERRITORY_IS_CAPITAL_PROP);
            if (faction == faction_name && is_capital != error && is_capital)
                elem.EraseProp(TERRITORY_IS_CAPITAL_PROP);
            endif
        endif
        sleepms(2);
    endforeach
endif
    
    // Encontrar o território alvo e defini-lo como capital
    var territory_elem := FindTerritoryByID(territory_id);
    if (territory_elem)
        territory_elem.SetProp(TERRITORY_IS_CAPITAL_PROP, 1);
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
    var rect_df := OpenDataFile(":architect:areas");
    if (!rect_df)
        return 0;
    endif
    
    // Primeiro tentar encontrar pelo ID direto (se o ID for o nome)
    var elem := rect_df.FindElement(territory_id);
    if (elem)
        return elem;
    endif
    
    // Se não encontrar, procurar em todos os elementos
    foreach elem_name in (rect_df.keys())
        elem := rect_df.FindElement(elem_name);
        var id := elem.GetProp(TERRITORY_ID_PROP);
        if (id == territory_id)
            return elem;
        endif
        sleepms(2);
    endforeach
    
    return 0;
endfunction

function IsTerritoryCapital(territory_id)
    var territory_elem := FindTerritoryByID(territory_id);
    if (territory_elem && territory_elem != error)
        var prop := territory_elem.GetProp(TERRITORY_IS_CAPITAL_PROP);
        if (prop != error && prop)
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
    else
        // Adicionar à fila se não existe
        updates_pending[container.serial] := polcore().systime;
        
        // Log de depuração
        Print("DEBUG: Baú #" + container.serial + " agendado para atualização");
    endif
    
    // Atualizar a propriedade global
    var result := SetGlobalProperty("recursos_updates_pending", updates_pending);
    if (result != 1)
        Print("ERRO ao agendar baú: " + result);
        return 0;
    endif
    
    // Verificar se o processador está rodando e iniciá-lo se necessário
    var pid := GetGlobalProperty("recursos_update_processor_pid");
    if (!pid || !GetProcess(pid))
        var script := start_script(":faccao:process_recursos_updates");
        if (script && script.errortext)
            Print("ERRO ao iniciar processador: " + script.errortext);
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
        Print("AVISO: Sincronização já em andamento, agendando para posterior");
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
    
    var df := DFOpenDataFile(KINGDOM_RESOURCE_DATAFILE, DF_CREATE);
    if (df == error)
        Print("ERRO: Não foi possível abrir/criar o datafile de recursos");
        return 0;
    endif
    
    // Abordagem diferente dependendo se é Reino ou facção específica
    if (faction_name == KINGDOM_FACTION || faction_name == "Reino")
        // Reino: usar o elemento principal
        var elem := DFFindElement(df, KINGDOM_RESOURCE_ELEM, DF_CREATE);
        elem.SetProp("recursos", recursos);
    else
        // Facção específica: criar elemento dedicado
        var elem := DFFindElement(df, "faction_" + faction_name, DF_CREATE);
        elem.SetProp("recursos", recursos);
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
        Print("AVISO: Encontradas " + inconsistencias + " inconsistências no baú #" + container.serial);
        
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


/*
 * AddSpecialPermission(who, faction_name)
 *
 * Purpose: Adiciona permissão especial para um membro da facção
 * Parameters: 
 *   who - O jogador que está concedendo a permissão
 *   faction_name - O nome da facção
 * Return: 1 para sucesso, 0 para falha
 */
function AddSpecialPermission(who, faction_name)
    SendSysMessage(who, "Selecione o membro para conceder permissão especial:");
    var target := Target(who);
    
    if (!target || !target.isA(POLCLASS_MOBILE))
        SendSysMessage(who, "Você deve selecionar um jogador.");
        return 0;
    endif
    
    // Verificar se o alvo pertence à facção
    var target_faction := GetPlayerFaction(target);
    if (target_faction != faction_name)
        SendSysMessage(who, "Este jogador não pertence à sua facção.");
        return 0;
    endif
    
    // Solicitar o tipo de permissão
    var gump := GFCreateGump();
    GFResizePic(gump, 0, 0, GFCfgConst("Defaults", "BackGround"), 300, 200);
    GFResizePic(gump, 15, 15, GFCfgConst("Defaults", "ForeGround"), 270, 170);
    
    GFTextMid(gump, 10, 25, 280, 2100, "Tipo de Permissão");
    
    var y := 60;
    GFAddButton(gump, 30, y, 2117, 2118, GF_CLOSE_BTN, 1);
    GFTextLine(gump, 55, y, 2103, "Ver e Depositar");
    y += 25;
    
    GFAddButton(gump, 30, y, 2117, 2118, GF_CLOSE_BTN, 2);
    GFTextLine(gump, 55, y, 2103, "Ver, Depositar e Remover");
    y += 25;
    
    GFAddButton(gump, 30, y, 2117, 2118, GF_CLOSE_BTN, 3);
    GFTextLine(gump, 55, y, 38, "Cancelar");
    
    var input := GFSendGump(who, gump);
    
    var permission := "";
    case (input[0])
        1: permission := "depositar";
        2: permission := "remover";
        default: return 0;
    endcase
    
    // Salvar a permissão especial
    var guild := FindGuildByName(faction_name);
    if (guild)
        var guild_obj := FindGuild(guild);
        if (guild_obj)
            var special_permissions := guild_obj.GetProp("chest_permissions");
            if (!special_permissions)
                special_permissions := dictionary{};
            endif
            
            special_permissions[target.serial] := permission;
            guild_obj.SetProp("chest_permissions", special_permissions);
            
            SendSysMessage(who, "Permissão especial concedida para " + target.name + ".");
            return 1;
        endif
    endif
    
    SendSysMessage(who, "Erro ao salvar permissão especial.");
    return 0;
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
    
    Print("DEBUG: Tentando criar item 0x" + Hex(objtype) + " x" + amount + " em contêiner " + container.serial);
    
    // ABORDAGEM 1: Verificar se já existe um item do mesmo tipo para empilhar
    var existing_item := FindExistingStackableItem(container, objtype);
    
    if (existing_item)
        Print("DEBUG: Item existente encontrado, tentando empilhar");
        
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
                
                Print("DEBUG: Item empilhado com sucesso, nova quantidade: " + existing_item.amount);
                return existing_item;
            else
                // Restaurar quantidade original se falhou
                existing_item.amount := before_amount;
                Print("AVISO: Falha ao aumentar quantidade, tentando método alternativo");
            endif
        endif
        
        // ABORDAGEM 2: Se não conseguiu empilhar diretamente, criar um item substituto
        var replacement := ReplaceStackWithNew(existing_item, amount, properties);
        if (replacement)
            Print("DEBUG: Item substituído com sucesso");
            return replacement;
        endif
    endif
    
    // ABORDAGEM 3: Criar novo item
    Print("DEBUG: Criando novo item");
    var new_item := CreateItemInContainer(container, objtype, amount);
    
    if (new_item)
        // Aplicar propriedades se fornecidas
        if (properties)
            foreach prop_name in (properties.keys())
                SetObjProperty(new_item, prop_name, properties[prop_name]);
                sleepms(1);
            endforeach
        endif
        
        Print("DEBUG: Item criado com sucesso: " + new_item.serial);
        return new_item;
    endif
    
    Print("ERRO: Falha ao criar item");
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
        Print("ERRO: Falha ao aumentar quantidade do item " + item.serial + ": " + item.amount + " != " + new_amount);
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
 * UnifiedCheckInsertPermission(who, container)
 *
 * Purpose: Verificação unificada de permissão para inserir itens no baú
 * Parameters:
 *   who - O jogador que está tentando inserir itens
 *   container - O baú onde o jogador está tentando inserir
 * Return:
 *   1 se tem permissão, 0 se não tem
 */
function UnifiedCheckInsertPermission(who, container)
    // Debug
    Print("DEBUG-PERM: Verificando permissão para INSERIR - Player: " + who.name);
    
    // GMs sempre podem inserir
    if (who.cmdlevel >= 3)
        Print("DEBUG-PERM: Permissão concedida - GM");
        return 1;
    endif
    
    // Se não tiver container, usar comportamento permissivo (para compatibilidade)
    if (!container)
        Print("DEBUG-PERM: Container não especificado, permitindo inserção");
        return 1;
    endif
    
    // Verificar facção do baú
    var chest_faction := GetFactionOfChest(container);
    Print("DEBUG-PERM: Facção do baú: " + chest_faction);
    
    // Obter facção do jogador
    var player_faction := GetPlayerFaction(who);
    Print("DEBUG-PERM: Facção do jogador: " + player_faction);
    
    // Regras para baús do Reino - qualquer um pode inserir
    if (chest_faction == KINGDOM_FACTION || chest_faction == "Reino")
        Print("DEBUG-PERM: Baú do Reino - inserção permitida para todos");
        return 1;
    endif
    
    // Para baús de facções específicas
    
    // Se o jogador for da mesma facção que o baú, pode inserir
    if (player_faction == chest_faction)
        Print("DEBUG-PERM: Jogador é membro da facção - inserção permitida");
        return 1;
    endif
    
    // Verificar permissões especiais
    if (HasSpecialPermission(who, container, "insert"))
        Print("DEBUG-PERM: Jogador tem permissão especial - inserção permitida");
        return 1;
    endif
    
    // Verificar títulos de nobreza para acesso especial
    var chardata := GetObjProperty(who, "chardata");
    if (chardata)
        if (chardata.current_nobility_title in {"Rei", "Rainha"} || 
            chardata.current_professional_position in {"Mão do Rei", "Imperador"})
            Print("DEBUG-PERM: Jogador tem título de nobreza/cargo especial - inserção permitida");
            return 1;
        endif
    endif
    
    // Se chegou aqui, não tem permissão
    Print("DEBUG-PERM: Permissão negada para inserção");
    return 0;
endfunction

/*
 * UnifiedCheckRemovePermission(who, container)
 *
 * Purpose: Verificação unificada de permissão para remover itens do baú
 * Parameters:
 *   who - O jogador que está tentando remover itens
 *   container - O baú de onde o jogador está tentando remover
 * Return:
 *   1 se tem permissão, 0 se não tem
 */
function UnifiedCheckRemovePermission(who, container)
    // Debug
    Print("DEBUG-PERM: Verificando permissão para REMOVER - Player: " + who.name);
    
    // GMs sempre podem remover
    if (who.cmdlevel >= 3)
        Print("DEBUG-PERM: Permissão concedida - GM");
        return 1;
    endif
    
    // Se não tiver container, verificar se tem título de nobreza (compatibilidade)
    if (!container)
        var chardata := GetObjProperty(who, "chardata");
        if (!chardata)
            return 0;
        endif
    
        // Verificar se o personagem é Rei, Rainha ou Mão do Rei
        if (chardata.current_nobility_title == "Rei" || chardata.current_nobility_title == "Rainha")
            return 1;
        endif
    
        if (chardata.current_professional_position == "Mão do Rei")
            return 1;
        endif
    
        return 0;
    endif
    
    // Verificar facção do baú
    var chest_faction := GetFactionOfChest(container);
    Print("DEBUG-PERM: Facção do baú: " + chest_faction);
    
    // Obter facção do jogador
    var player_faction := GetPlayerFaction(who);
    Print("DEBUG-PERM: Facção do jogador: " + player_faction);
    
    // Regras para baús do Reino
    if (chest_faction == KINGDOM_FACTION || chest_faction == "Reino")
        // Apenas cargos de alto escalão podem remover itens do Reino
        var chardata := GetObjProperty(who, "chardata");
        if (chardata)
            if (chardata.current_nobility_title in {"Rei", "Rainha"} || 
                chardata.current_professional_position in {"Mão do Rei", "Imperador"})
                Print("DEBUG-PERM: Jogador tem título de nobreza/cargo real - remoção permitida");
                return 1;
            endif
        endif
        
        // Outros jogadores não podem remover
        Print("DEBUG-PERM: Permissão negada - não tem cargo real necessário");
        return 0;
    endif
    
    // Para baús de facções específicas
    
    // Se o jogador for da mesma facção, verificar cargo
    if (player_faction == chest_faction)
        // Verificar se é líder da guilda
        if (IsGuildLeader(who))
            Print("DEBUG-PERM: Jogador é líder da facção - remoção permitida");
            return 1;
        endif
        
        // Verificar cargo na facção
        var player_rank := GetObjProperty(who, "guild_rank");
        if (player_rank in {"Lider", "Capitao", "Leader", "Captain"})
            Print("DEBUG-PERM: Jogador tem cargo alto na facção - remoção permitida");
            return 1;
        endif
        
        if (player_rank in {"Tenente", "Lieutenant"})
            Print("DEBUG-PERM: Tenente da facção - remoção permitida");
            return 1;
        endif
        
        // Verificar permissões especiais
        if (HasSpecialPermission(who, container, "remove"))
            Print("DEBUG-PERM: Jogador tem permissão especial - remoção permitida");
            return 1;
        endif
        
        // Membros regulares não podem remover
        Print("DEBUG-PERM: Membro regular da facção - remoção negada");
        return 0;
    endif
    
    // Verificar nobreza do reino para acesso a baús de facções
    var chardata := GetObjProperty(who, "chardata");
    if (chardata)
        if (chardata.current_nobility_title in {"Rei", "Rainha"} || 
            chardata.current_professional_position in {"Mão do Rei", "Imperador"})
            Print("DEBUG-PERM: Jogador tem título de nobreza/cargo real - remoção permitida");
            return 1;
        endif
    endif
    
    // Se chegou aqui, não tem permissão
    Print("DEBUG-PERM: Permissão negada para remoção");
    return 0;
endfunction

/*
 * UnifiedCheckOpenPermission(who, container)
 *
 * Purpose: Verificação unificada de permissão para abrir o baú
 * Parameters:
 *   who - O jogador que está tentando abrir o baú
 *   container - O baú que o jogador está tentando abrir
 * Return:
 *   1 se tem permissão, 0 se não tem
 */
function UnifiedCheckOpenPermission(who, container)
    // Debug
    Print("DEBUG-PERM: Verificando permissão para ABRIR - Player: " + who.name);
    
    // GMs sempre podem abrir
    if (who.cmdlevel >= 3)
        Print("DEBUG-PERM: Permissão concedida - GM");
        return 1;
    endif
    
    // Se não tiver container, não permitir (segurança)
    if (!container)
        Print("DEBUG-PERM: Container não especificado, negando acesso");
        return 0;
    endif
    
    // Verificar facção do baú
    var chest_faction := GetFactionOfChest(container);
    Print("DEBUG-PERM: Facção do baú: " + chest_faction);
    
    // Verificar facção do jogador
    var player_faction := GetPlayerFaction(who);
    Print("DEBUG-PERM: Facção do jogador: " + player_faction);
    
    // REGRA PRINCIPAL: Facções devem ser idênticas para acesso
    if (player_faction == chest_faction)
        Print("DEBUG-PERM: Jogador é membro da facção - acesso permitido");
        return 1;
    endif
    
    // EXCEÇÃO 1: Baús do Reino podem ser acessados por jogadores com títulos nobres
    if (chest_faction == KINGDOM_FACTION || chest_faction == "Reino")
        var chardata := GetObjProperty(who, "chardata");
        if (chardata)
            if (chardata.current_nobility_title in {"Rei", "Rainha"} || 
                chardata.current_professional_position in {"Mão do Rei", "Imperador"})
                Print("DEBUG-PERM: Jogador tem título de nobreza/cargo real - acesso a baú do Reino permitido");
                return 1;
            endif
        endif
    endif
    
    // EXCEÇÃO 2: Verificar permissões especiais específicas
    if (HasSpecialPermission(who, container, "open") || 
        HasSpecialPermission(who, container, "insert") || 
        HasSpecialPermission(who, container, "remove"))
        Print("DEBUG-PERM: Jogador tem permissão especial - acesso permitido");
        return 1;
    endif
    
    // EXCEÇÃO 3: Permissões de debug para testes (apenas para desenvolvimento)
    if (GetObjProperty(who, "AllowFullChestAccess") || 
        GetObjProperty(who, "AllowAllChestAccess"))
        Print("DEBUG-PERM: Jogador tem permissão de debug - acesso permitido");
        return 1;
    endif
    
    // Negação padrão: se nenhuma regra ou exceção permitir acesso
    Print("DEBUG-PERM: Permissão negada para abrir o baú");
    return 0;
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

/*
 * GetPlayerRankInGuild(who)
 *
 * Purpose: Obter o cargo/rank do jogador na guilda de forma padronizada
 * Parameters:
 *   who - O jogador a ser verificado
 * Return:
 *   String com o cargo do jogador, ou string vazia se não tiver
 */
function GetPlayerRankInGuild(who)
    if (!who)
        return "";
    endif
    
    // Primeiro tentar obter da propriedade armazenada
    var player_rank := GetObjProperty(who, "guild_rank");
    if (player_rank)
        return player_rank;
    endif
    
    // Se não encontrou, tentar obter diretamente da guilda
    var player_guild := who.guild;
    if (!player_guild)
        return "";
    endif
    
    // Verificar nos membros da guilda
    foreach member in (player_guild.members)
        if (member.serial == who.serial && member.guild_title)
            // Atualizar a propriedade para futuras consultas
            SetObjProperty(who, "guild_rank", member.guild_title);
            return member.guild_title;
        endif
        sleepms(1);
    endforeach
    
    return "";
endfunction

/*
 * LogPermissionCheck(who, container, action, result)
 *
 * Purpose: Função padronizada para registrar verificações de permissão
 * Parameters:
 *   who - O jogador que está sendo verificado
 *   container - O container envolvido
 *   action - A ação que está sendo verificada ("OPEN", "INSERT", "REMOVE")
 *   result - O resultado da verificação (1 para permitido, 0 para negado)
 */
function LogPermissionCheck(who, container, action, result)
    if (!who || !container)
        return;
    endif
    
    // Timestamp para registros
    var timestamp := polcore().systime;
    var time_str := CStr(timestamp); // Fallback para o timestamp cru
    
    // Formatar o timestamp de forma segura
    var formatted_time := FormatTimeString(timestamp);
    if (formatted_time && formatted_time != error)
        time_str := formatted_time;
    endif
    
    // Obter informações relevantes com verificações de segurança
    var chest_faction := "Desconhecida";
    var player_faction := "Desconhecida";
    var player_rank := "";
    
    // Obter facção do baú com verificação de erro
    var temp_faction := GetFactionOfChest(container);
    if (temp_faction && temp_faction != error)
        chest_faction := temp_faction;
    endif
    
    // Obter facção do jogador com verificação de erro
    temp_faction := GetPlayerFaction(who);
    if (temp_faction && temp_faction != error)
        player_faction := temp_faction;
    endif
    
    // Obter rank do jogador com verificação de erro
    var temp_rank := GetPlayerRankInGuild(who);
    if (temp_rank && temp_rank != error)
        player_rank := temp_rank;
    endif
    
    // Registrar no console do servidor (log básico)
    var log_message := "[" + time_str + "] PERM-CHECK: " + who.name + " [" + player_faction + "/" + player_rank + "] " + 
          action + " " + (result ? "PERMITIDO" : "NEGADO") + " para baú " + container.serial + 
          " (Facção: " + chest_faction + ")";
    
    Print(log_message);
    
    // Registrar no datafile para análise posterior (log detalhado)
    var df := DFOpenDataFile(":faccao:chest_access_log");
    if (df != error && df)
        var elem := DFFindElement(df, "logs", DF_CREATE);
        if (elem != error && elem)
            var logs := elem.GetProp("entries");
            if (!logs || logs == error)
                logs := array{};
            endif
            
            // Criar entrada de log com verificações de tipo
            var log_entry := struct{
                "timestamp" := timestamp,
                "player" := who.name,
                "player_serial" := who.serial,
                "player_faction" := player_faction,
                "player_rank" := player_rank,
                "chest_serial" := container.serial,
                "chest_faction" := chest_faction,
                "action" := action,
                "success" := result ? 1 : 0,
                "cmdlevel" := who.cmdlevel
            };
            
            logs.append(log_entry);
            
            // Limitar o tamanho do log para evitar crescimento excessivo
            while (logs.size() > 200)
                logs.erase(1);
            endwhile
            
            elem.SetProp("entries", logs);
        endif
    endif
endfunction


/*
 * FormatTimeString(timestamp)
 *
 * Purpose: Formatar timestamp para uso em logs
 * Parameters:
 *   timestamp - Timestamp do polcore().systime
 * Return:
 *   String formatada com data e hora
 */
function FormatTimeString(timestamp)
    // Verificação inicial para prevenir erros
    if (!timestamp || timestamp == error)
        return CStr(ReadGameClock());
    endif
    
    // Método simples que não usa systime_precise
    var seconds_per_day := 86400;
    var seconds_per_hour := 3600;
    var seconds_per_minute := 60;
    
    var days := CInt(timestamp / seconds_per_day) % 365;
    var hours := CInt((timestamp % seconds_per_day) / seconds_per_hour);
    var minutes := CInt((timestamp % seconds_per_hour) / seconds_per_minute);
    var seconds := CInt(timestamp % seconds_per_minute);
    
    // Formatar simples (ano fixo - mais para exibição do que precisão)
    var formatted := CStr(2023) + "-" + 
                   PadStr(CStr(1), 2, "0") + "-" + 
                   PadStr(CStr(days+1), 2, "0") + " " + 
                   PadStr(CStr(hours), 2, "0") + ":" + 
                   PadStr(CStr(minutes), 2, "0") + ":" + 
                   PadStr(CStr(seconds), 2, "0");
    
    return formatted;
endfunction

function PadStr(str, length_needed, padchar := " ")
    str := CStr(str);
    while (length_needed > len(str))
        str := padchar + str;
    endwhile
    
    return str;
endfunction

// Função para compatibilidade com código existente
function CanAccessChest(who, chest)
    return UnifiedCheckOpenPermission(who, chest);
endfunction

// Função para compatibilidade com código existente
function CanInsertItems(who, container := 0)
    return UnifiedCheckInsertPermission(who, container);
endfunction

// Função para compatibilidade com código existente
function CanRemoveItems(who, container := 0)
    return UnifiedCheckRemovePermission(who, container);
endfunction


/*
 * HasSpecialPermission(who, container, permission_type)
 *
 * Purpose: Verificar se um jogador tem permissão especial para um container
 * Parameters:
 *   who - O jogador a ser verificado
 *   container - O container a ser verificado
 *   permission_type - O tipo de permissão ("open", "insert", "remove")
 * Return:
 *   1 se tem permissão, 0 se não tem
 */
function HasSpecialPermission(who, container, permission_type)
    // Verificar se o jogador tem permissão específica
    var container_permissions := GetObjProperty(container, "special_permissions");
    if (!container_permissions)
        return 0;
    endif
    
    // Procurar permissão específica para este jogador
    if (container_permissions[who.serial])
        // Se tiver entrada para este jogador, verificar permissão específica
        var player_permissions := container_permissions[who.serial];
        if (TypeOf(player_permissions) == "Struct" && player_permissions[permission_type])
            return 1;
        endif
    endif
    
    // Verificar permissões de debug
    if (GetObjProperty(who, "AllowFullChestAccess"))
        return 1;
    endif
    
    return 0;
endfunction



///////////////////////////////////////////////////////////////
// Sistema de Transações Atômicas para Operações Críticas
///////////////////////////////////////////////////////////////

/*
 * IniciarTransacao(operacao, dados)
 * Inicia uma transação e registra em um datafile para rastreamento
 */
function IniciarTransacao(operacao, dados)
    var df := OpenDataFile(":faccao:transacoes");
    if (!df)
        df := CreateDataFile(":faccao:transacoes");
        if (!df)
            Print("Erro ao criar datafile de transações");
            return error;
        endif
    endif
    
    var transaction_id := CStr(PolCore().systime) + "_" + CStr(RandomInt(1000));
    var elem := df.CreateElement(transaction_id);
    
    elem.SetProp("operacao", operacao);
    elem.SetProp("dados", dados);
    elem.SetProp("status", "iniciada");
    elem.SetProp("timestamp", PolCore().systime);
    
    Print("DEBUG-TRANS: Transação " + transaction_id + " iniciada para operação: " + operacao);
    
    return transaction_id;
endfunction

/*
 * FinalizarTransacao(transaction_id)
 * Marca uma transação como concluída
 */
function FinalizarTransacao(transaction_id)
    var df := OpenDataFile(":faccao:transacoes");
    if (!df)
        Print("Erro ao abrir datafile de transações");
        return error;
    endif
    
    var elem := df.FindElement(transaction_id);
    if (!elem)
        Print("Transação " + transaction_id + " não encontrada");
        return error;
    endif
    
    elem.SetProp("status", "concluida");
    elem.SetProp("timestamp_conclusao", PolCore().systime);
    
    Print("DEBUG-TRANS: Transação " + transaction_id + " finalizada com sucesso");
    
    return 1;
endfunction

/*
 * DesfazerTransacao(transaction_id)
 * Desfaz uma transação em caso de falha
 */
function DesfazerTransacao(transaction_id)
    var df := OpenDataFile(":faccao:transacoes");
    if (!df)
        Print("Erro ao abrir datafile de transações");
        return error;
    endif
    
    var elem := df.FindElement(transaction_id);
    if (!elem)
        Print("Transação " + transaction_id + " não encontrada");
        return error;
    endif
    
    var operacao := elem.GetProp("operacao");
    var dados := elem.GetProp("dados");
    
    Print("DEBUG-TRANS: Desfazendo transação " + transaction_id + " para operação: " + operacao);
    
    // Executar lógica de reversão com base no tipo de operação
    var resultado := 0;
    case (operacao)
        "transferir_territorio": resultado := DesfazerTransferenciaTerritorio(dados);
        default: Print("Tipo de operação desconhecido: " + operacao);
    endcase
    
    elem.SetProp("status", "desfeita");
    elem.SetProp("timestamp_desfazer", PolCore().systime);
    
    return resultado;
endfunction

/*
 * DesfazerTransferenciaTerritorio(dados)
 * Desfaz uma transferência de território incompleta
 */
function DesfazerTransferenciaTerritorio(dados)
    if (!dados)
        return 0;
    endif
    
    Print("DEBUG-TRANS: Desfazendo transferência de território");
    Print("DEBUG-TRANS: Território: " + dados.territorio_id);
    Print("DEBUG-TRANS: Facção original: " + dados.faccao_original);
    
    var territorio_id := dados.territorio_id;
    var faccao_original := dados.faccao_original;
    
    // 1. Restaurar facção original no território
    var elem := FindTerritoryByID(territorio_id);
    if (!elem)
        Print("Território " + territorio_id + " não encontrado");
        return 0;
    endif
    
    elem.SetProp(TERRITORY_FACTION_PROP, faccao_original);
    elem.SetProp("Faction", faccao_original); // Para compatibilidade
    
    // 2. Restaurar objetos originais se informação disponível
    if (dados.seriais_originais)
        var seriais_df := OpenDataFile(":faccao:seriais");
        if (seriais_df)
            var territorios_seriais := seriais_df.GetProp("territorios");
            if (territorios_seriais)
                territorios_seriais[territorio_id] := dados.seriais_originais;
                seriais_df.SetProp("territorios", territorios_seriais);
            endif
        endif
    endif
    
    // 3. Restaurar lista de territórios da facção original
    var guild_original := FindGuildByName(faccao_original);
    if (guild_original)
        var guild_obj := FindGuild(guild_original);
        if (guild_obj)
            var territories := guild_obj.GetProp("territories");
            if (!territories)
                territories := array{};
            endif
            
            if (!(territorio_id in territories))
                territories.append(territorio_id);
                guild_obj.SetProp("territories", territories);
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
                endif
            endif
        endif
    endif
    
    Print("DEBUG-TRANS: Transferência de território desfeita com sucesso");
    
    return 1;
endfunction

/*
 * VerificarTransacoesPendentes()
 * Verifica transações pendentes e tenta recuperar ou finalizar
 */
function VerificarTransacoesPendentes()
    var df := OpenDataFile(":faccao:transacoes");
    if (!df)
        return error;
    endif
    
    var now := PolCore().systime;
    var count := 0;
    
    foreach key in (df.Keys())
        var elem := df.FindElement(key);
        var status := elem.GetProp("status");
        var timestamp := elem.GetProp("timestamp");
        
        if (status == "iniciada" && (now - timestamp) > 300) // 5 minutos de timeout
            Print("Transação " + key + " pendente há muito tempo. Tentando desfazer...");
            DesfazerTransacao(key);
            count += 1;
        endif
        
        sleepms(2);
    endforeach
    
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
    Print("Iniciando limpeza de referências ao território: " + territory_name);
    
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
                Print("Removendo território '" + territory_name + "' da facção: " + guild.GetProp("name"));
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
    
    Print("Limpeza concluída. Atualizadas " + guilds_updated + " guildas.");
    return guilds_updated;
endfunction