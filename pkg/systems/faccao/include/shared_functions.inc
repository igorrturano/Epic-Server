// Marcador de inclusão para evitar problemas de redefinição
const SHARED_FUNCTIONS_INCLUDED := 1;

use uo;
use os;
use cfgfile;
use datafile;
use guilds;
use math; // Para Abs

include ":datafile:datafile";
include ":faccao:faccao_constants";
include ":timeutils:time"; // Se usado para formatação de tempo
include ":faccao:include/logging";         // ESSENCIAL
include ":faccao:include/datafile_helpers"; // ESSENCIAL
include ":faccao:include/permissions";      // Para GetFactionOfChest
include ":faccao:include/safe_guild_utils"; // Funções seguras para propriedades de guild
include ":faccao:include/territory_management"; // Para TM_ funções
include ":faccao:include/safe_guild_utils";
include ":faccao:include/guild_xp";

// =============================================================================
// SEÇÃO 1: FUNÇÕES SIMPLES PARA ACESSO SEGURO (FASE 1.1 KISS)
// =============================================================================

/*
 * GetGuildFaction(guild)
 * 
 * Obtém a facção de uma guilda - SOLUÇÃO KISS
 * Tenta "faction" primeiro, depois "Faction" como fallback
 */
function GetGuildFaction(guild)
    if (!guild)
        return DEFAULT_FACTION;
    endif
    
    // Tentar propriedade padrão primeiro
    var faction := guild.GetProp(FACTION_PROP_NAME);
    if (faction != error)
        return faction;
    endif
    
    // Fallback simples para propriedade legada
    faction := guild.GetProp("Faction");
    if (faction != error)
        return faction;
    endif
    
    return DEFAULT_FACTION;
endfunction

/*
 * SetGuildFaction(guild, faction_name)
 * 
 * Define a facção de uma guilda - SOLUÇÃO KISS
 * Define a nova e remove a antiga
 */
function SetGuildFaction(guild, faction_name)
    if (!guild)
        return 0;
    endif
    
    if (!faction_name)
        faction_name := DEFAULT_FACTION;
    endif
    
    // Definir a nova propriedade
    guild.SetProp(FACTION_PROP_NAME, faction_name);
    
    // Remover a propriedade legada se existir
    if (guild.GetProp("Faction") != error)
        guild.EraseProp("Faction");
    endif
    
    return 1;
endfunction
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
    
    // MÉTODO 2: Tentar buscar diretamente no datafile de registro
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
    
    // MÉTODO 3: Busca tradicional usando ListGuilds
    var guilds := ListGuilds();
    if (guilds && guilds != error)
        foreach guild in guilds
            var prop := guild.GetProp(FACTION_PROP_NAME);
            if (prop != error && prop == name)
                return guild.guildid;
            endif            // Verificar outras propriedades de nome
            prop := guild.GetProp("name"); // Propriedade legada
            if (prop != error && prop == name)
                return guild.guildid;
            endif
            
            // Verificar o nome direto da guilda usando SafeGetGuildName
            var guild_name := SafeGetGuildName(guild);
            if (guild_name == name)
                return guild.guildid;
            endif
            
            sleepms(2);
        endforeach
    endif
    
    // MÉTODO 4: Backup - procurar por jogadores online
    foreach player in (EnumerateOnlineCharacters())
        if (player.guildid && player.guild.GetProp(FACTION_PROP_NAME) == name)
            return player.guild.guildid;
        endif
        
        if (player.guildid && player.guild.GetProp(FACTION_PROP_NAME) == name)
            return player.guild.guildid;
        elseif (player.guildid && player.guild.GetProp("name") == name) // Propriedade legada            return player.guild.guildid;
        endif
        
        if (player.guildid && SafeGetGuildName(player.guild) == name)
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
            guild_name := guild.GetProp("name"); // Propriedade legada
            endif
            if (!guild_name || guild_name == error)
                guild_name := SafeGetGuildName(guild);
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
 * GetAllFactions_KISS()
 *
 * Retorna todas as facções existentes (guildas), mesmo sem território.
 * Retorna array de structs: { name, guild_id }
 */
function GetAllFactions_KISS()
    var all_factions := array{};
    var guilds := ListGuilds();
    if (!guilds || guilds == error)
        return all_factions;
    endif
    var seen := dictionary{};
    foreach guild in guilds
        var name := GetGuildFaction(guild);
        if (name && !seen.exists(name))
            all_factions.append(struct{
                "name" := name,
                "guild_id" := guild.guildid
            });
            seen[name] := 1;
        endif
        sleepms(1);
    endforeach
    return all_factions;
endfunction



// Flag global para indicar se uma sincronização está em andamento
var GLOBAL_SYNC_IN_PROGRESS := 0;

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

/*
 * SubtractAmount(item, amount_to_subtract)
 *
 * Purpose: Remove uma quantidade de um item.
 * Parameters:
 *   item - O item do qual remover.
 *   amount_to_subtract - A quantidade a remover.
 * Return:
 *   1 se bem-sucedido, 0 se falha.
 */
function HelpSubtractAmount(item, amount_to_subtract)
    if (!item || !item.isA(POLCLASS_ITEM) || amount_to_subtract <= 0)
        LogError("shared_HelpSubtract", "Parâmetros inválidos para HelpSubtractAmount. Item: " + (item ? item.serial : "NULL") + ", Amount: " + amount_to_subtract);
        return 0;
    endif;

    if (item.amount < amount_to_subtract)
        LogWarning("shared_HelpSubtract", "Tentativa de remover " + amount_to_subtract + " de item " + item.serial + " que só tem " + item.amount);
        return 0; // Não tem o suficiente
    endif;

    var container_of_item := item.container; 

    // In POL, we need to manually handle item amount reduction
    var new_amount := item.amount - amount_to_subtract;
    
    if (new_amount <= 0)
        // If the new amount is 0 or less, destroy the item
        var result := DestroyItem(item);
        if (result)
            LogInfo("shared_HelpSubtract", "Item destruído após remover " + amount_to_subtract + " unidades.");
            if (container_of_item && container_of_item.isA(POLCLASS_CONTAINER))
                MarcarRecursosModificados(container_of_item); 
            endif;
            return 1;
        else
            LogError("shared_HelpSubtract", "Falha ao destruir item " + item.serial);
            return 0;
        endif;
    else
        // Set the new amount
        item.amount := new_amount;
        
        // Verify the change took effect
        if (item.amount == new_amount)
            LogInfo("shared_HelpSubtract", amount_to_subtract + " unidades removidas do item " + item.serial + ". Nova quantidade: " + item.amount);
            if (container_of_item && container_of_item.isA(POLCLASS_CONTAINER))
                MarcarRecursosModificados(container_of_item); 
            endif;
            return 1;
        else
            LogError("shared_HelpSubtract", "Falha ao atualizar quantidade do item " + item.serial);
            return 0;
        endif;
    endif;
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


/*
 * NormalizarNomeElementoFaccao(nome_faccao_bruto)
 *
 * Purpose: Normaliza o nome de uma facção para uso em elementos do datafile
 * Parameters:
 *   nome_faccao_bruto - Nome da facção a normalizar
 * Return:
 *   String normalizada no formato "faction_nome" ou um error
 */
function NormalizarNomeElementoFaccao(nome_faccao_bruto)
    if (!nome_faccao_bruto)
        LogError("recursos", "NormalizarNomeElementoFaccao: Nome de facção nulo ou vazio");
        return error{"errortext" := "Nome de facção inválido"};
    endif;
    
    var nome_base := Lower(CStr(nome_faccao_bruto));
    nome_base := StrReplace(nome_base, " ", "_"); 
    
    if (nome_base.find("faction_") == 1) // Corrigido para .find() == 1 (início da string)
        nome_base := nome_base[9, nome_base.len()-8]; // Ajuste para pegar o restante da string
    endif;
    
    if (nome_base == "")
        LogError("recursos", "NormalizarNomeElementoFaccao: Nome de facção ficou vazio após normalização: " + nome_faccao_bruto);
        return error{"errortext" := "Nome de facção resultou em string vazia"};
    endif;
    
    return "faction_" + nome_base;
endfunction

// Função para atualizar o datafile de recursos
// Em pkg/systems/faccao/include/shared_functions.inc

function AtualizarDatafileRecursos(faction_name, recursos_novos_para_salvar)
    var log_message := ""; // Variável para construir as mensagens de log
    var debug_category := "AtualizarDatafile"; // Categoria de log para esta função

    LogInfo(debug_category, $"Iniciando para Facção: '{faction_name}'. Recursos a salvar fornecidos (Tipo: {TypeOf(recursos_novos_para_salvar)}).");

    // Validação dos parâmetros de entrada
    if (!faction_name)
        LogError(debug_category, "Nome da facção não fornecido. Abortando.");
        return 0;
    endif

    if (!recursos_novos_para_salvar || TypeOf(recursos_novos_para_salvar) != "Struct")
        LogError(debug_category, $"'recursos_novos_para_salvar' não é um struct válido para facção '{faction_name}'. Tipo: {TypeOf(recursos_novos_para_salvar)}. Abortando.");
        return 0;
    endif

    // Log detalhado do struct recebido
    LogDebug(debug_category, $"Recursos recebidos para '{faction_name}': " + StructToString(recursos_novos_para_salvar));
    LogCritical(debug_category, $"[RASTREIO MOEDAS] '{faction_name}' - Moedas recebidas em 'recursos_novos_para_salvar': {recursos_novos_para_salvar.moedas}");


    // Abrir o datafile
    var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE, DF_CREATE);
    if (!df)
        LogError(debug_category, $"Falha ao abrir/criar datafile '{KINGDOM_RESOURCE_DATAFILE}'. Abortando.");
        return 0;
    endif

    // Determinar o nome do elemento no datafile
    var elemento_alvo_nome := "";
    if (faction_name == KINGDOM_FACTION || faction_name == "Reino")
        elemento_alvo_nome := KINGDOM_RESOURCE_ELEM;
    else
        elemento_alvo_nome := NormalizarNomeElementoFaccao(faction_name);
        if (elemento_alvo_nome == error)
            LogError(debug_category, $"Nome de facção '{faction_name}' resultou em nome de elemento inválido: {elemento_alvo_nome.errortext}. Abortando.");
            return 0;
        endif
    endif
    LogInfo(debug_category, $"Elemento alvo no datafile para facção '{faction_name}' é: '{elemento_alvo_nome}'.");

    // Encontrar ou criar o elemento no datafile
    var elem := SafeFindElement(df, elemento_alvo_nome, DF_CREATE);
    if (!elem)
        LogError(debug_category, $"Falha ao encontrar/criar elemento '{elemento_alvo_nome}' no datafile. Abortando.");
        return 0;
    endif

    // 1. Ler o struct de recursos ATUAL do datafile para este elemento
    var recursos_existentes_no_df := GetElementProp(elem, "recursos");
    if (!recursos_existentes_no_df || TypeOf(recursos_existentes_no_df) != "Struct")
        LogWarning(debug_category, $"Elemento '{elemento_alvo_nome}' não continha um struct 'recursos' válido ou estava vazio. Inicializando com struct zerado.");
        recursos_existentes_no_df := CreateEmptyResourceStruct();
    endif
    LogDebug(debug_category, $"Recursos existentes no datafile para '{elemento_alvo_nome}': " + StructToString(recursos_existentes_no_df));
    LogCritical(debug_category, $"[RASTREIO MOEDAS] '{elemento_alvo_nome}' - Moedas existentes no datafile: {recursos_existentes_no_df.moedas}");


    // 2. Garantir que o struct lido e o novo tenham todas as chaves esperadas (completar com 0 se faltar)
    var chaves_padrao := CreateEmptyResourceStruct().keys();

    foreach chave in chaves_padrao
        if (!recursos_existentes_no_df.exists(chave))
            LogWarning(debug_category, $"Chave padrão '{chave}' ausente em 'recursos_existentes_no_df' para '{elemento_alvo_nome}'. Adicionando com valor 0.");
            recursos_existentes_no_df[chave] := 0;
        endif
        if (!recursos_novos_para_salvar.exists(chave))
            LogWarning(debug_category, $"Chave padrão '{chave}' ausente em 'recursos_novos_para_salvar' para '{faction_name}'. Adicionando com valor 0.");
            recursos_novos_para_salvar[chave] := 0;
        endif
    endforeach

    // 3. Comparar e determinar se houve mudança real
    var alguma_mudanca_real := 0;
    var struct_final_para_salvar := CreateEmptyResourceStruct(); // Começa zerado, vai ser preenchido

    foreach chave in chaves_padrao
        var valor_existente_df := CInt(recursos_existentes_no_df[chave]);
        var valor_novo_recebido := CInt(recursos_novos_para_salvar[chave]);

        // A lógica aqui é que 'recursos_novos_para_salvar' representa o ESTADO FINAL desejado.
        // Não estamos somando, estamos definindo o valor total.
        struct_final_para_salvar[chave] := valor_novo_recebido;

        if (valor_novo_recebido != valor_existente_df)
            alguma_mudanca_real := 1;
            LogInfo(debug_category, $"Mudança detectada para '{elemento_alvo_nome}', Recurso '{chave}': Datafile={valor_existente_df}, Novo Total={valor_novo_recebido}.");
            if (chave == "moedas")
                 LogCritical(debug_category, $"[RASTREIO MOEDAS] '{elemento_alvo_nome}' - Mudança para MOEDAS: Datafile={valor_existente_df}, Novo Total={valor_novo_recebido}.");
            endif
        endif
    endforeach

    // 4. Salvar o struct final APENAS se houve alguma mudança real
    if (alguma_mudanca_real)
        LogCritical(debug_category, $"PRESTES A SALVAR para '{elemento_alvo_nome}'. Struct Final: {StructToString(struct_final_para_salvar)}");
        LogCritical(debug_category, $"[RASTREIO MOEDAS] '{elemento_alvo_nome}' - Moedas a serem salvas no datafile: {struct_final_para_salvar.moedas}");

        var setResult := SetElementProp(elem, "recursos", struct_final_para_salvar);

        if (setResult)
            LogInfo(debug_category, $"SUCESSO ao salvar struct 'recursos' atualizado para o elemento '{elemento_alvo_nome}'.");
            // Log adicional para confirmar a leitura após salvar (para debug extremo)
            // var recursos_apos_salvar := GetElementProp(elem, "recursos");
            // LogCritical(debug_category, $"[RASTREIO MOEDAS] '{elemento_alvo_nome}' - Moedas LIDAS DO DATAFIL IMEDIATAMENTE APÓS SALVAR: {recursos_apos_salvar.moedas}");
            return 1;
        else
            LogError(debug_category, $"FALHA ao salvar struct 'recursos' para o elemento '{elemento_alvo_nome}'.");
            return 0;
        endif
    else
        LogInfo(debug_category, $"Nenhuma mudança real nos valores dos recursos para '{elemento_alvo_nome}'. Datafile não modificado.");
        return 1; // Sucesso, pois não havia nada a mudar.
    endif
endfunction

function StructToString(s)
    if (!s || TypeOf(s) != "Struct")
        return "Struct Inválido/Nulo";
    endif
    var str := "{";
    var first := 1;
    foreach key in (s.keys())
        if (!first)
            str := str + ", ";
        endif
        str := str + key + "=" + s[key];
        first := 0;
    endforeach
    str := str + "}";
    return str;
endfunction


/*
 * ContarRecursosReais(container)
 *
 * Purpose: Conta os recursos físicos em um container
 * Parameters: container - O baú a ser contado
 * Return: Struct com as contagens de recursos
 */
function ContarRecursosReais(container)
    var recursos := CreateEmptyResourceStruct(); // Garante que todas as categorias estão presentes e zeradas.
    
    if (!container)
        LogDebug("recursos", "ContarRecursosReais: Container nulo, retornando struct vazio");
        return recursos;
    endif
    
    // LogDebug("recursos", "ContarRecursosReais: Iniciando contagem física em: " + container.desc + " (Serial: " + container.serial + ")");
    
    foreach item_no_bau in EnumerateItemsInContainer(container)
        if (!item_no_bau) continue; endif 

        var amount := CInt(item_no_bau.amount);
        if (amount <= 0) continue; endif

        // IMPORTANTE: Usar item_no_bau aqui para ClassificarRecurso
        var categoria_recurso := ClassificarRecurso(item_no_bau); 
        // ClassificarRecurso deve retornar uma string como "Comida", "Metal", etc. (case-sensitive conforme as chaves do struct)

        // LogDebug("recursos", "ContarRecursosReais: Item " + item_no_bau.desc + " (Objtype: 0x" + Hex(item_no_bau.objtype) + ", Amount: " + amount + ") -> Categoria: " + categoria_recurso);

        // As chaves em CreateEmptyResourceStruct são minúsculas (comida, metal, etc.)
        // As constantes RESOURCE_TYPE_* (Comida, Metal) são com maiúscula inicial.
        // Precisamos garantir consistência aqui.
        // Se ClassificarRecurso retorna "Comida", a chave no struct é "comida".
        var chave_struct := "";
        if (categoria_recurso == RESOURCE_TYPE_COMIDA) chave_struct := "comida";
        elseif (categoria_recurso == RESOURCE_TYPE_MOEDAS) chave_struct := "moedas";
        elseif (categoria_recurso == RESOURCE_TYPE_COURO) chave_struct := "couro";
        elseif (categoria_recurso == RESOURCE_TYPE_METAL) chave_struct := "metal";
        elseif (categoria_recurso == RESOURCE_TYPE_MADEIRA) chave_struct := "madeira";
        elseif (categoria_recurso == RESOURCE_TYPE_PANO) chave_struct := "pano";
        elseif (categoria_recurso == RESOURCE_TYPE_PEDRA) chave_struct := "pedras";
        elseif (categoria_recurso == RESOURCE_TYPE_JOIAS) chave_struct := "joias";
        else chave_struct := Lower(categoria_recurso); // Fallback para minúsculo, caso ClassificarRecurso retorne algo inesperado
        endif

        if (recursos.exists(chave_struct)) 
            recursos[chave_struct] := CInt(recursos[chave_struct]) + amount;
        else
            // Este log é importante para identificar recursos não mapeados para as chaves do struct
            LogWarning("recursos", "ContarRecursosReais: Categoria normalizada '" + chave_struct + "' (de original '" + categoria_recurso + "') para o item " + item_no_bau.desc + " (ObjType: 0x" + Hex(item_no_bau.objtype) + ") não existe no struct de recursos retornado por CreateEmptyResourceStruct. Verifique o case e as definições.");
        endif
        
        sleepms(1); 
    endforeach
    
    LogInfo("recursos", "=== Contagem Finalizada em ContarRecursosReais (Baú: " + container.serial + ") ==="); // Mudado para LogInfo para se destacar mais
    foreach key_recurso in (recursos.keys())
      LogInfo("recursos", "  " + key_recurso + ": " + recursos[key_recurso]);
      sleepms(1);
    endforeach
    
    return recursos;
endfunction


/*
 * MapearRecurso(resource_name)
 *
 * Purpose: Mapeia um nome de recurso para seu objtype correspondente
 * Parameters: resource_name - Nome do recurso a ser mapeado
 * Return: Objtype do recurso ou 0 se não encontrado
 */
function MapearRecurso(resource_name)
    // Verificar entrada
    if (!resource_name || TypeOf(resource_name) != "String") 
        LogError("recursos", "Nome de recurso inválido: " + resource_name);
        return 0;
    endif

    // Normalizar nome
    var original_name := resource_name;
    resource_name := Lower(Trim(resource_name));
    
    LogDebug("recursos", "MapearRecurso: normalizado '" + resource_name + "' (original: '" + original_name + "')");
    
    // Mapeamento completo de recursos
var resource_map := dictionary;
    resource_map["ironore"]         := 0x6300;
    resource_map["prataingot"]      := 0x6993;
    resource_map["frigutiumingot"]  := 0x6460; // Verifique se este objtype está correto para "frigutiumingot"
    resource_map["couroespesso"]    := 0xEF12;
    resource_map["couropolar"]      := 0xEF17;
    resource_map["couroescamoso"]   := 0xEF11;
    resource_map["courodraconiano"] := 0xED20; // Verifique se este objtype está correto para "courodraconiano"
    resource_map["escamasazuis"]    := 0xEF36;
    resource_map["madeirapinheiro"] := 0xEE36; // Verifique se este objtype está correto para "madeirapinheiro"
    resource_map["madeirateixo"]    := 0xEE3E;
    resource_map["carvao"]          := 0x1BDE; // Verifique se este objtype está correto para "carvao"
    resource_map["cogumelo"]        := 0xD16;  // Note: 0xd16 é minúsculo, mas POL geralmente trata hex como case-insensitive. Use 0xD16 para consistência.
    resource_map["pelebranca"]      := 0xEE52;
    resource_map["ossocinza"]       := 0xEF32; // Verifique se este objtype está correto para "ossocinza"
    resource_map["glass"]           := 0x1BE3; // Verifique se este objtype está correto para "glass"
    resource_map["ossogigante"]     := 0xEF3A;
    resource_map["ossodemoniaco"]   := 0xEF29; // Verifique se este objtype está correto para "ossodemoniaco"
    
    
    // Verificar mapeamento direto - ÚNICA TENTATIVA DE MAPEAMENTO
    if (resource_map.exists(resource_name))
        LogDebug("recursos", "MapearRecurso: correspondência direta '" + resource_name + "' -> 0x" + Hex(resource_map[resource_name]));
        return resource_map[resource_name];
    endif
    LogError("recursos", "Recurso NOME '" + resource_name + "' não mapeado para OBJTYPE em MapearRecurso (shared_functions.inc).");
    return 0;
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
        "cache_sujo" := GetObjProperty(container, "recursos_cache_dirty") ? 1 : 0,        "historico_sync" := GetObjProperty(container, "recursos_sync_history"),
        "historico_verificacao" := GetObjProperty(container, "recursos_verify_history"),
        "faction" := GetObjectFaction(container)
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

/*
 * SafeCreateItemInContainer_Refactored(container, objtype, amount_to_create, properties := 0, creator_char := 0)
 * 
 * Cria ou empilha um item em um container de forma segura, aplicando propriedades.
 * properties: Dicionário com CProps a serem aplicadas.
 * creator_char: Opcional, personagem que está realizando a ação (0 para SYSTEM).
 * Retorna: ItemRef do item criado/modificado ou objeto de erro.
 */
function SafeCreateItemInContainer_Refactored(container, objtype, amount_to_create, properties := 0, creator_char := 0)
    LogDebug("SafeCreate_Refactored", $"Iniciando para Container: {(container ? container.serial : "NULL")}, ObjType: 0x{Hex(objtype)}, Amount: {amount_to_create}");
    // === Validações Iniciais ===
    if (!container)
        LogError("SafeCreate_Refactored", "Container inválido (NULL).");
        return error{"errortext" := "Container inválido"};
    endif
    if (TypeOf(container) != "ItemRef" || !container.isA(POLCLASS_CONTAINER)) // POL eScript usa TypeOf(obj) == "ItemRef"
        LogError("SafeCreate_Refactored", $"Parâmetro container não é um container válido. Serial: {(container ? container.serial : "N/A")}");
        return error{"errortext" := "Container inválido"};
    endif
    if (!objtype || CInt(objtype) <= 0)
        LogError("SafeCreate_Refactored", $"ObjType inválido: {objtype}");
        return error{"errortext" := "ObjType inválido"};
    endif
    if (amount_to_create <= 0)
        LogInfo("SafeCreate_Refactored", $"Quantidade para criar é zero ou negativa ({amount_to_create}), nada a fazer.");
        return 0; // Nada a fazer se a quantidade é zero ou negativa
    endif
    // Fim Validações

      var item_resultante := 0;
    var amount_remaining_to_create := amount_to_create;

    LogDebug("SafeCreate_Refactored", "Passo 1: Procurando item existente para empilhar...");
    var existing_item := FindExistingStackableItem_Enhanced(container, objtype, properties); 

    if (existing_item)
        LogDebug("SafeCreate_Refactored", $"Item existente {existing_item.serial} (Amount: {existing_item.amount}) encontrado para possível empilhamento.");
        
        var amount_actually_stacked := TryIncreaseAmount_Refactored(existing_item, amount_remaining_to_create); // Chama a nova TryIncrease
        
        if (amount_actually_stacked > 0)
            LogInfo("SafeCreate_Refactored", $"SUCESSO no empilhamento: {amount_actually_stacked} unidades adicionadas ao item {existing_item.serial}. Novo total: {existing_item.amount}");
            amount_remaining_to_create -= amount_actually_stacked;
            item_resultante := existing_item;
            
            if (TypeOf(properties) == "Dictionary" && properties.Size() > 0)
                LogDebug("SafeCreate_Refactored", "Aplicando/Mesclando propriedades (pós-empilhamento) ao item: " + item_resultante.serial);
                ApplyPropertiesToItem(item_resultante, properties); 
            endif
        else
            LogWarning("SafeCreate_Refactored", $"Falha ao tentar empilhar no item {existing_item.serial}.");
        endif
    else
        LogDebug("SafeCreate_Refactored", "Nenhum item existente compatível encontrado para empilhamento.");
    endif

    if (amount_remaining_to_create > 0)
        LogDebug("SafeCreate_Refactored", $"Passo 2: Criando novo item com {amount_remaining_to_create} unidades.");
        var new_item := CreateItemInContainer(container, objtype, amount_remaining_to_create, creator_char);
        
        if (new_item && new_item != error)
            LogInfo("SafeCreate_Refactored", $"SUCESSO na criação: Novo item {new_item.serial} (Amount: {new_item.amount}).");
            if (TypeOf(properties) == "Dictionary" && properties.Size() > 0)
                LogDebug("SafeCreate_Refactored", "Aplicando propriedades ao novo item: " + new_item.serial);
                ApplyPropertiesToItem(new_item, properties);
            else
                LogDebug("SafeCreate_Refactored", "Nenhuma propriedade adicional para aplicar ao novo item: " + new_item.serial);
            endif
            item_resultante := new_item; 
        else
            LogError("SafeCreate_Refactored", $"FALHA ao criar novo item. Erro: {(new_item && new_item.errortext ? CStr(new_item.errortext) : "Desconhecido/NULL")}");
            if (!item_resultante) 
                return error{"errortext" := "Falha total ao criar ou empilhar o item."};
            endif
        endif
    endif

    if (item_resultante)
        LogInfo("SafeCreate_Refactored", $"Operação concluída. Item retornado: {item_resultante.serial}, Amount: {item_resultante.amount}");
        return item_resultante;
    else
        LogWarning("SafeCreate_Refactored", "Nenhum item resultante. Amount original para criar era: " + amount_to_create);
        if (amount_to_create > 0)
            return error{"errortext" := "Falha inesperada, nenhum item criado ou empilhado."};
        else
            return 0; 
        endif
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

function ValidateAndFixTerritoryConsistency(territory_elem, territory_id)
    if (!territory_elem || !territory_id)
        return 0;
    endif
    
    var stored_name := GetElementProp(territory_elem, "Name");
    var stored_id := GetElementProp(territory_elem, TERRITORY_ID_PROP);
    
    LogDebug("territorio", "Validando consistência: Nome='" + stored_name + "', ID='" + stored_id + "', Esperado ID='" + territory_id + "'");
    
    // Se não há ID armazenado, definir
    if (!stored_id)
        LogInfo("territorio", "Territory_ID não definido, configurando: " + territory_id);
        SetElementProp(territory_elem, TERRITORY_ID_PROP, territory_id);
        return 1;
    endif
    
    // Se IDs não correspondem, há inconsistência
    if (stored_id != territory_id)
        LogWarning("territorio", "INCONSISTÊNCIA DETECTADA: ID armazenado ('" + stored_id + "') != ID esperado ('" + territory_id + "')");
        
        // Tentar resolver usando o ID mais confiável
        var rect_df := SafeOpenDataFile(":architect:areas");
        if (rect_df)
            // Verificar se existe elemento com o ID esperado
            var correct_elem := SafeFindElement(rect_df, territory_id);
            if (correct_elem)
                LogInfo("territorio", "Usando elemento correto com ID: " + territory_id);
                return correct_elem;
            endif
        endif
        
        LogError("territorio", "Não foi possível resolver inconsistência de território");
        return 0;
    endif
    
    LogDebug("territorio", "Consistência validada com sucesso");
    return territory_elem;
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
                        SetObjectFaction(bau, faccao_original);
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
                        SetObjectFaction(mesa, faccao_original);
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
                        SetObjectFaction(mural, faccao_original);
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
                LogInfo("territorio", "Removendo território \"" + territory_name + "\" da facção: " + guild.GetProp(FACTION_PROP_NAME));
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
 * Return: String com a categoria do recurso (ex: "Comida", "Metal", etc., conforme faccao_constants.inc)
 *         ou "desconhecido" se não puder ser classificado.
 */
function ClassificarRecurso(resource)
    if (!resource)
        return "desconhecido";
    endif

    if (TypeOf(resource) == "ItemRef")
        // Verificar CProps explícitas primeiro (IsKingdomDonation<Categoria>)
        // Esta lógica pode ser expandida ou simplificada dependendo de como você define
        // as CProps nos itens gerados.
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

        // Se não encontrou CProp explícita, classificar pelo objtype do item
        return ClassificarPorObjtype(resource.objtype);
    endif

    if (TypeOf(resource) == "String")
        // Se for string, tentar classificar pelo nome
        return ClassificarPorNome(resource);
    endif

    if (TypeOf(resource) == "Integer")
        // Se for integer, assumir que é um objtype
        return ClassificarPorObjtype(resource);
    endif

    LogWarning("ClassificarRecurso", "Tipo de recurso não suportado para classificação: " + TypeOf(resource));
    return "desconhecido";
endfunction

function ClassificarPorNome(resource_name_input)
    var original_resource_name := resource_name_input;
    var resource_name := Lower(Trim(CStr(resource_name_input)));

    // MAPEAMENTO EXPANDIDO - Chaves são as constantes RESOURCE_TYPE_*
    // A ordem aqui pode importar se um nome de recurso puder cair em múltiplas categorias
    // por exemplo, "osso demoniaco" - é couro ou pedra? Decida e coloque na categoria correta.
    var categorias := dictionary{
        RESOURCE_TYPE_MOEDAS := array{ // Moedas primeiro, pois "gold" pode aparecer em outros
            "moeda", "sestertius", "dinheiro", "ouro", "gold piece", "coin", "currency"
        },
        RESOURCE_TYPE_METAL := array{
            "ironore", "frigutiumingot", "prataingot", "metal", "minerio", "barra", "lingote",
            "minerio de ferro", "barra de frigutium", "barra de prata",
            "minério de ferro", "lingote de prata", "lingote de frigutium", "iron ore"
        },
        RESOURCE_TYPE_COURO := array{
            "couro", "pele", "escama",
            "couroespesso", "couroescamoso", "couropolar", "courodraconiano", "escamasazuis",
            "couro espesso", "couro escamoso", "couro polar", "couro draconiano", "escamas azuis",
            "osso demoniaco" // Se "osso demoniaco" é couro
        },
        RESOURCE_TYPE_MADEIRA := array{
        "madeira", "tora", "lenha", "carvao", "carvão", // Confirmado
        "madeirapinheiro", "madeirateixo", "toras",
        "madeira de pinheiro", "madeira de teixo"
    },
        RESOURCE_TYPE_COMIDA := array{
            "comida", "alimento", "cogumelo"
        },
        RESOURCE_TYPE_PANO := array{
            "pano", "tecido", "linho", "algodao", "algodão",
            "pelebranca", "pele branca" // Se "pele branca" é tecido
        },
        RESOURCE_TYPE_PEDRA := array{
            "pedra", "rocha", "mineral", "vidro", "osso",
            "glass", "ossogigante", "ossos", "ossocinza",
            "osso gigante", "osso cinza"
            // Se "osso demoniaco" for pedra, mova para cá.
        },
        RESOURCE_TYPE_JOIAS := array{
            "joia", "gema", "cristal",
            "jewel", "gem", "crystal", "rubi", "diamante", "esmeralda", "safira"
        }
    };

    foreach categoria_constante in (categorias.keys())
        foreach palavra_chave in (categorias[categoria_constante])
            if (resource_name.find(palavra_chave) != error)
                return categoria_constante;
            endif
        endforeach
    endforeach

    LogWarning("ClassificarPorNome", "Recurso '" + original_resource_name + "' (normalizado: '"+resource_name+"') não classificado por nome em shared_functions.inc.");
    return "desconhecido";
endfunction

function ClassificarPorObjtype(objtype_param)
    var objtype_to_check := CInt(objtype_param);
    if (!objtype_to_check || objtype_to_check <= 0)
        // Adicionei um log aqui para quando o objtype_param original não era 0,
        // mas se tornou 0 após CInt, o que indicaria um problema de entrada.
        if (objtype_param != 0 && objtype_to_check == 0) 
            LogWarning("ClassificarPorObjtype", "Objtype original '" + objtype_param + "' resultou em 0 após CInt().");
        endif
        return "desconhecido";
    endif
    
    // Criar mapa com chaves garantidamente inteiras
    var objtype_map := dictionary{};
    
    // Metais
    objtype_map[CInt(0x6300)] := RESOURCE_TYPE_METAL;     // Iron ore
    objtype_map[CInt(0x19B9)] := RESOURCE_TYPE_METAL;
    objtype_map[CInt(0x6993)] := RESOURCE_TYPE_METAL;     // Prata ingot
    objtype_map[CInt(0x6460)] := RESOURCE_TYPE_METAL;     // Frigutium ingot
    
    // Couro
    objtype_map[CInt(0xEF12)] := RESOURCE_TYPE_COURO;     // Couro espesso
    objtype_map[CInt(0xEF17)] := RESOURCE_TYPE_COURO;     // Couro polar
    objtype_map[CInt(0xEF11)] := RESOURCE_TYPE_COURO;     // Couro escamoso
    objtype_map[CInt(0xED20)] := RESOURCE_TYPE_COURO;     // Couro draconiano
    objtype_map[CInt(0xEF36)] := RESOURCE_TYPE_COURO;     // Escamas azuis
    objtype_map[CInt(0xEF29)] := RESOURCE_TYPE_COURO;     // Osso demoníaco
    
    // Madeira
    objtype_map[CInt(0xEE36)] := RESOURCE_TYPE_MADEIRA;   // Madeira de pinheiro
    objtype_map[CInt(0xEE3E)] := RESOURCE_TYPE_MADEIRA;   // Madeira de teixo
    objtype_map[CInt(0x1BDE)] := RESOURCE_TYPE_MADEIRA;   // Carvão
    objtype_map[CInt(0x1BD7)] := RESOURCE_TYPE_MADEIRA;
    
    // Comida
    objtype_map[CInt(0xD16)]  := RESOURCE_TYPE_COMIDA;    // Cogumelo
    objtype_map[CInt(0x097B)] := RESOURCE_TYPE_COMIDA;
    
    // Moedas
    objtype_map[CInt(0xBA63)] := RESOURCE_TYPE_MOEDAS;
    
    // Pano
    objtype_map[CInt(0xEE52)] := RESOURCE_TYPE_PANO;      // Pele branca
    objtype_map[CInt(0x1767)] := RESOURCE_TYPE_PANO;
    
    // Pedras
    objtype_map[CInt(0xEF32)] := RESOURCE_TYPE_PEDRA;     // Osso cinza
    objtype_map[CInt(0xEF3A)] := RESOURCE_TYPE_PEDRA;     // Osso gigante
    objtype_map[CInt(0x1BE3)] := RESOURCE_TYPE_PEDRA;     // Glass/Vidro
    objtype_map[CInt(0x177A)] := RESOURCE_TYPE_PEDRA;
    
    // Joias
    objtype_map[CInt(0xF13)]  := RESOURCE_TYPE_JOIAS;
    
    // Agora tentar acesso direto que deve funcionar
    if (objtype_map.exists(objtype_to_check))
        // LogDebug("ClassificarPorObjtype", "Objtype 0x" + Hex(objtype_to_check) + " MAPEADO para -> " + objtype_map[objtype_to_check] + " via .exists()");
        return objtype_map[objtype_to_check];
    endif
    
    // Se ainda falhar (o que não deveria acontecer se as chaves são inteiras),
    // tentar a iteração como um fallback de depuração,
    // mas a intenção é que o .exists() acima resolva.
    // Este loop de fallback pode ser removido se o .exists() funcionar consistentemente.
    /*
    LogWarning("ClassificarPorObjtype", "Fallback para iteração de chaves para objtype 0x" + Hex(objtype_to_check) + ". Isso não deveria ser necessário.");
    foreach key_from_map in (objtype_map.keys()) // key_from_map será o inteiro aqui
        if (key_from_map == objtype_to_check)
            // LogDebug("ClassificarPorObjtype", "Objtype 0x" + Hex(objtype_to_check) + " MAPEADO para -> " + objtype_map[key_from_map] + " via iteração de chaves (fallback).");
            return objtype_map[key_from_map];
        endif
    endforeach
    */
    
    LogWarning("ClassificarPorObjtype", "Objtype 0x" + Hex(objtype_to_check) + 
               " (" + objtype_to_check + ") não mapeado. Verifique se está presente em objtype_map e se RESOURCE_TYPE_ constants estão corretas.");
    return "desconhecido";
endfunction

function FindFactionChest(faction_name, territory_identifier := "", create_if_missing := 0)
    if (!faction_name)
        LogError("recursos_FindChest", "FindFactionChest() chamado sem nome de facção");
        return 0;
    endif; // Adicionado ponto e vírgula
    
    LogDebug("recursos_FindChest", "Buscando baú para facção '" + faction_name + 
             (territory_identifier ? "', território/ID '" + territory_identifier + "'" : "'"));
    
    var chest := 0;
    var territory_id := "";
    
    if (territory_identifier)
        if (territory_identifier.find("TER_") == 1)
            territory_id := territory_identifier;
            LogDebug("recursos_FindChest", "Identificador é um territory_id: " + territory_id);
        else
            LogDebug("recursos_FindChest", "Identificador é um nome, buscando territory_id correspondente: '" + territory_identifier + "'");
            territory_id := FindTerritoryIdByName(territory_identifier);
            if (territory_id)
                LogInfo("recursos_FindChest", "Territory_id encontrado para '" + territory_identifier + "': " + territory_id);
            else
                LogWarning("recursos_FindChest", "Não foi possível encontrar territory_id para nome: '" + territory_identifier + "'. A busca continuará sem ID específico de território.");
            endif;
        endif;
    endif;
    
    if (territory_id)
        var seriais_df_s1 := SafeOpenDataFile(":faccao:seriais"); 
        if (seriais_df_s1)
            var territory_elem_s1 := SafeFindElement(seriais_df_s1, territory_id);
            if (territory_elem_s1)
                LogDebug("recursos_FindChest", "Elemento do território '" + territory_id + "' encontrado no datafile de seriais (Estratégia 1).");
                var elem_faction_s1 := GetTerritoryFaction(territory_elem_s1);
                if (elem_faction_s1 == faction_name)
                    var chest_serial_s1 := GetElementProp(territory_elem_s1, "bau");
                    if (chest_serial_s1)
                        chest := SystemFindObjectBySerial(CInt(chest_serial_s1)); 
                        if (chest && IsValidContainer(chest) && GetObjectFaction(chest) == faction_name)
                            LogDebug("recursos_FindChest", "Baú " + chest.serial + " encontrado e validado via datafile (novo formato) para território " + territory_id);
                            return chest;
                        else
                            LogWarning("recursos_FindChest", "Serial " + chest_serial_s1 + " para baú do terr " + territory_id + " encontrado, mas objeto inválido, não encontrado no mundo ou facção do objeto não bate. Chest: " + (chest ? chest.serial : "NULL"));
                            chest := 0; 
                        endif;
                    endif
                else
                    LogDebug("recursos_FindChest", "Elemento de seriais para '" + territory_id + "' encontrado, mas facção no datafile de seriais ('" + elem_faction_s1 + "') não corresponde à facção buscada ('" + faction_name + "').");
                endif;
            endif; 
            
           var territorios_seriais_s2 := GetElementProp(seriais_df_s1, "territorios");
            if (territorios_seriais_s2 && territorios_seriais_s2[territory_id]) 
                var info_s2 := territorios_seriais_s2[territory_id];
                if (info_s2.faction == faction_name && info_s2.bau)
                    chest := SystemFindObjectBySerial(CInt(info_s2.bau)); 
                    if (chest && IsValidContainer(chest))
                        LogInfo("recursos_FindChest", "Baú " + chest.serial + " encontrado via datafile (formato legado) para terr " + territory_id + ". Migrando registro.");
                        var territory_elem_migrate := SafeFindElement(seriais_df_s1, territory_id, DF_CREATE); 
                        if (territory_elem_migrate)
                            SetElementProp(territory_elem_migrate, "bau", info_s2.bau);
                            SetElementProp(territory_elem_migrate, "faction", info_s2.faction);
                            if (info_s2.mesa) SetElementProp(territory_elem_migrate, "mesa", info_s2.mesa); endif;
                            if (info_s2.mural) SetElementProp(territory_elem_migrate, "mural", info_s2.mural); endif;
                        endif;
                        return chest;
                    else
                        LogWarning("recursos_FindChest", "Serial " + info_s2.bau + " (legado) para baú do terr " + territory_id + " encontrado, mas objeto inválido ou não encontrado no mundo.");
                        chest := 0; 
                    endif;
                endif;
            endif; 
        endif; 
    endif; 
    
    LogDebug("recursos_FindChest", "Estratégia 3: Buscando baús no mundo para facção '" + faction_name + "'" + (territory_id ? " e ID '" + territory_id + "'" : " (sem ID específico)"));
    
    foreach item_s3 in EnumerateItemsInContainer(0)
        if (item_s3.objtype == 0xFF11) 
            var item_faction_prop_s3 := GetObjectFaction(item_s3); 
            if (item_faction_prop_s3 == faction_name)
                if (territory_id)
                    var item_territory_id_prop_s3 := GetObjProperty(item_s3, "territory_id");
                    if (item_territory_id_prop_s3 == territory_id)
                        LogInfo("recursos_FindChest", "Baú " + item_s3.serial + " da facção '" + faction_name + "' E território '" + territory_id + "' encontrado no mundo. Atualizando registro.");
                        var seriais_df_s3_update := SafeOpenDataFile(":faccao:seriais", DF_CREATE);
                        if (seriais_df_s3_update)
                            var territory_elem_s3_update := SafeFindElement(seriais_df_s3_update, territory_id, DF_CREATE);
                            if (territory_elem_s3_update)
                                SetElementProp(territory_elem_s3_update, "bau", item_s3.serial);
                                SetElementProp(territory_elem_s3_update, "faction", faction_name);
                            endif;
                        endif;
                        return item_s3;
                    endif; 
                else 
                    LogInfo("recursos_FindChest", "Baú " + item_s3.serial + " da facção '" + faction_name + "' (sem ID de território específico) encontrado no mundo.");
                    return item_s3;
                endif; 
            endif; 
        endif; 
        SleepMS(1); 
    endforeach;
    
    if (create_if_missing && !chest)
        LogWarning("recursos_FindChest", "Criando baú de emergência para '" + faction_name + "'" + (territory_id ? " (associado ao Território ID: " + territory_id + ")" : " (sem associação específica de território)"));
        chest := CreateEmergencyDonationChest(faction_name);
        
        if (chest && territory_id) 
            SetObjProperty(chest, "territory_id", territory_id); 
            SetObjProperty(chest, "emergency_for_territory", territory_id); 
            var seriais_df_emerg := SafeOpenDataFile(":faccao:seriais", DF_CREATE);
            if (seriais_df_emerg)
                var territory_elem_emerg := SafeFindElement(seriais_df_emerg, territory_id, DF_CREATE);
                if (territory_elem_emerg)
                    SetElementProp(territory_elem_emerg, "bau", chest.serial);
                    SetElementProp(territory_elem_emerg, "faction", faction_name); 
                    LogInfo("recursos_FindChest", "Baú de emergência " + chest.serial + " registrado para território ID '" + territory_id + "'.");
                else
                    LogError("recursos_FindChest", "Falha ao encontrar/criar elemento para registrar baú de emergência para Território ID '" + territory_id + "'.");
                endif;
            else
                LogError("recursos_FindChest", "Falha ao abrir datafile de seriais para registrar baú de emergência.");
            endif;
        elseif (chest && !territory_id)
             LogInfo("recursos_FindChest", "Baú de emergência " + chest.serial + " criado para facção '" + faction_name + "' (sem ID de território específico).");
        endif;
        return chest; 
    endif; 
    
    if (!chest)
        LogError("recursos_FindChest", "Nenhum baú encontrado para facção: '" + faction_name + "'" + 
                 (territory_identifier ? " (Território/ID original: '" + territory_identifier + "', ID processado: '" + territory_id + "')" : ""));
    else
        LogInfo("recursos_FindChest", "Baú final retornado (após todas as estratégias) para facção '" + faction_name + "': " + chest.serial);
    endif;

    return chest; 
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
    SetObjectFaction(chest, faction_name);
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

function GetMaxStackSize(objtype)
    // Exemplo simples, idealmente isso viria de uma config ou itemdesc.cfg
    // (itemdesc.cfg tem a tag StackLimit, mas não é acessível diretamente via eScript padrão)
    var item_cfg := ReadConfigFile(":*:itemdesc"); // Caminho pode variar
    if (item_cfg)
        var elem := item_cfg[Hex(objtype)]; // Objtypes em itemdesc são geralmente hexadecimais
        if (elem)
            var stack_limit := CInt(elem.StackLimit);
            if (stack_limit > 0)
                return stack_limit;
            endif
        endif
    endif
    return 60000; // Padrão genérico do POL
endfunction

// FUNÇÃO AUXILIAR: Encontrar territory_id por nome
function FindTerritoryIdByName(territory_name)
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        return "";
    endif
    
    foreach elem_name in (rect_df.keys())
        var elem := SafeFindElement(rect_df, elem_name);
        if (!elem)
            continue;
        endif
        
        var name := GetElementProp(elem, "Name");
        if (!name)
            name := GetElementProp(elem, TERRITORY_NAME_PROP);
        endif
        
        if (name == territory_name)
            var territory_id := GetElementProp(elem, TERRITORY_ID_PROP);
            if (!territory_id)
                territory_id := elem_name; // Usar o próprio nome do elemento como fallback
            endif
            return territory_id;
        endif
        sleepms(2);
    endforeach
    
    return "";
endfunction

/*
 * CreateEmptyResourceStruct()
 *
 * Purpose: Cria um struct vazio para recurso com todos os valores zerados
 * Parameters: Nenhum
 * Return: Struct com todos os tipos de recursos zerados
 */
// Versão correta (usando a sintaxe adequada de eScript)
function CreateEmptyResourceStruct()
    var recursos := struct{
        comida := 0,
        moedas := 0,    
        couro := 0,
        metal := 0,
        madeira := 0,
        pano := 0,
        pedras := 0,
        joias := 0     
    };
    return recursos;
endfunction
    
    // Add this function to force refresh the gump
function RefreshResourceGump(who, container)
    // Force close any existing gump
    SendSysMessageEX(who, "Atualizando interface...", SSM_INFO);
    
    // Small delay to ensure clean state
    Sleepms(100);
    
    // Create and send fresh gump
    var gump := BuildResourceGump(who, container);
    return GFSendGump(who, gump);
endfunction

/*
 * ProcessResourceChange(who, container)
 *
 * Purpose: Processa mudanças nos recursos e atualiza a interface
 * Parameters: 
 *   who - O jogador que está vendo a interface
 *   container - O baú de recursos sendo atualizado 
 * Return: 1 para sucesso, 0 para falha
 */
function ProcessResourceChange(who, container)
    if (!who || !container)
        return 0;
    endif
    
    // Atualizar contagem de recursos
    var recursos := ContarRecursosReais(container);
    
    // Sincronizar com datafile
    SincronizarComDataFile(container, recursos);
    
    // Limpar duplicatas
    CleanDuplicateItems(container);
    
    // Forçar atualização da interface
    RefreshResourceGump(who, container);
    
    return 1;
endfunction

/*
 * GetDonationType(item)
 *
 * Purpose: Determina o tipo de doação de um item
 * Parameters: item - O item a analisar
 * Return: String com o tipo de recurso
 */
function GetDonationType(item)
    if (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_COMIDA))
        return RESOURCE_TYPE_COMIDA;
    elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_MOEDAS))
        return RESOURCE_TYPE_MOEDAS;
    elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_COURO))
        return RESOURCE_TYPE_COURO;
    elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_METAL))
        return RESOURCE_TYPE_METAL;
    elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_MADEIRA))
        return RESOURCE_TYPE_MADEIRA;
    elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_PANO))
        return RESOURCE_TYPE_PANO;
    elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_PEDRA))
        return RESOURCE_TYPE_PEDRA;
    elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_JOIAS))
        return RESOURCE_TYPE_JOIAS;
    else
        // Determinação automática baseada no objtype
        return DeterminarTipoRecursoAutomatico(item);
    endif
endfunction
 
/*
 * ProcessResourceItem(item, type, byref recursos)
 *
 * Purpose: Processa um item e atualiza a contagem no struct de recursos
 * Parameters: 
 *   item - O item sendo processado
 *   type - O tipo de recurso
 *   recursos - O struct de recursos a ser atualizado
 * Return: Nada, modifica o struct recursos diretamente
 */
function ProcessResourceItem(item, type, byref recursos)
    var amount := 1;
    
    if (item.amount)
        amount := item.amount;
    endif
    
    case (type)
        RESOURCE_TYPE_COMIDA:
            recursos.comida += amount;
            break;
        RESOURCE_TYPE_MOEDAS:
            recursos.moedas += amount;
            break;
        RESOURCE_TYPE_COURO:
            recursos.couro += amount;
            break;
        RESOURCE_TYPE_METAL:
            recursos.metal += amount;
            break;
        RESOURCE_TYPE_MADEIRA:
            recursos.madeira += amount;
            break;
        RESOURCE_TYPE_PANO:
            recursos.pano += amount;
            break;
        RESOURCE_TYPE_PEDRA:
            recursos.pedras += amount;
            break;
        RESOURCE_TYPE_JOIAS:
            recursos.joias += amount;
            break;

    endcase
endfunction


function SincronizarComDataFile(container_ou_faction_name, recursos_totais_agregados)
    var log_message := ""; // Para construir a mensagem de log
    if ((TypeOf(container_ou_faction_name) == "ItemRef" && !container_ou_faction_name) || !recursos_totais_agregados)
        LogError("recursos", "SincronizarComDataFile: Parâmetros inválidos. Container/FactionName ou RecursosTotais nulos.");
        return 0;
    endif

    var faction_name := "";

    if (TypeOf(container_ou_faction_name) == "ItemRef")
        faction_name := GetObjProperty(container_ou_faction_name, OBJ_FACTION_PROP);
        if (faction_name == error || !faction_name)
            faction_name := GetObjectFaction(container_ou_faction_name);
            if (faction_name == error || !faction_name)
                faction_name := GetFactionOfChest(container_ou_faction_name);
            endif
        endif
    elseif (TypeOf(container_ou_faction_name) == "String")
        faction_name := container_ou_faction_name;
    else
        log_message := "SincronizarComDataFile: Primeiro parâmetro inválido, esperado ItemRef ou String. Recebido: " + TypeOf(container_ou_faction_name);
        LogError("recursos", log_message);
        return 0;
    endif

    if (faction_name == error || !faction_name)
        LogError("recursos", "SincronizarComDataFile: Não foi possível determinar a facção para sincronização.");
        return 0;
    endif

    log_message := "SincronizarComDataFile: Iniciando para facção '" + faction_name + "' com recursos totais fornecidos.";
    LogDebug("recursos", log_message);

    var recursos_validados := ValidarRecursos(recursos_totais_agregados);
    if (!recursos_validados)
        log_message := "SincronizarComDataFile: Recursos totais fornecidos são inválidos após validação para facção '" + faction_name + "'.";
        LogError("recursos", log_message);
        return 0;
    endif

    var df_success := AtualizarDatafileRecursos(faction_name, recursos_validados);
    var guild_success := AtualizarPropriedadesGuildaRecursos(faction_name, recursos_validados);

    if (df_success || guild_success)
        log_message := "SincronizarComDataFile para facção '" + faction_name + "' concluída. Datafile: " + (df_success ? "Sucesso" : "Falha") + ", Guilda: " + (guild_success ? "Sucesso" : "Falha");
        LogInfo("recursos", log_message);
        return 1;
    else
        log_message := "SincronizarComDataFile: FALHA TOTAL na sincronização para facção '" + faction_name + "'. Datafile e Guilda falharam.";
        LogError("recursos", log_message);
        return 0;
    endif
endfunction

/*
 * AgendarAtualizacaoDataFile(container)
 *
 * Purpose: Agenda uma atualização futura do datafile para este container
 * Parameters: container - O baú para agendar atualização
 * Return: 1 para sucesso, 0 para falha
 */
function AgendarAtualizacaoDataFile(container)
    if (!container)
        return 0;
    endif
    
    SetObjProperty(container, "recursos_update_scheduled", polcore().systime);
    return 1;
endfunction

/*
 * ObterRecursosFaccao(faction_name := "")
 *
 * Purpose: Obtém os recursos totais de uma facção do datafile global
 * Parameters: faction_name - Nome da facção (opcional)
 * Return: Struct com os recursos
 */
function ObterRecursosFaccao(faction_name := "")
    if (!faction_name)
        faction_name := KINGDOM_FACTION;
    endif
    
    var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE);
    if (!df)
        return CreateEmptyResourceStruct();
    endif
    
    var elem := SafeFindElement(df, faction_name);
    if (!elem)
        return CreateEmptyResourceStruct();
    endif
    
    var recursos := GetElementProp(elem, KINGDOM_RESOURCE_ELEM, CreateEmptyResourceStruct());
    return recursos;
endfunction

/*
 * AtualizarRecursosFaccao(faction_name, recursos)
 *
 * Purpose: Atualiza os recursos de uma facção no datafile global
 * Parameters: 
 *   faction_name - Nome da facção
 *   recursos - Struct com os recursos a serem atualizados
 * Return: 1 para sucesso, 0 para falha
 */
function AtualizarRecursosFaccao(faction_name, recursos)
    var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE, 1); // 1 = criar se não existir
    if (!df)
        LogError("recursos", "Não foi possível criar o datafile de recursos");
        return 0;
    endif
    
    var elem := SafeFindElement(df, faction_name, 1); // 1 = criar se não existir
    if (!elem)
        LogError("recursos", "Não foi possível criar/encontrar o elemento para " + faction_name);
        return 0;
    endif
    
    return SetElementProp(elem, KINGDOM_RESOURCE_ELEM, recursos);
endfunction

   
// Função para obter contagem de recursos agrupados por categoria
function ContarRecursosPorCategoria(container)
    var contagem := dictionary{
        "comida" := 0,
        "moedas" := 0,
        "couro" := 0,
        "metal" := 0,
        "madeira" := 0,
        "pano" := 0,
        "pedras" := 0,
        "joias" := 0,
        "desconhecido" := 0
    };
    
    if (!container)
        return contagem;
    endif
    
    // Contar itens por categoria
    foreach item in EnumerateItemsInContainer(container)
        // Classificar o item
        var categoria := ClassificarRecurso(item);
        
        // Adicionar à contagem apropriada
        if (contagem.exists(categoria))
            contagem[categoria] += item.amount;
        else
            contagem[categoria] := item.amount;
        endif
        
        sleepms(1);
    endforeach
    
    return contagem;
endfunction

// Função auxiliar para verificar a categoria de um recurso
function VerificarCategoriaRecurso(resource)
    var categoria := ClassificarRecurso(resource);
    
    LogDebug("recursos", "Recurso: " + resource + " => Categoria: " + categoria);
    
    return categoria;
endfunction


// Em :faccao:include:faccao_resources.src ou shared_functions.src

function SafeStackItems(container, objtype, amount_to_add_or_create, properties := 0)
    // (Validações iniciais de container, objtype, amount_to_add_or_create)
    
    var existing_item := FindExistingStackableItem_Enhanced(container, objtype, properties); // Passar properties para checagem de CProps
    
    if (existing_item)
        var max_stack := GetMaxStackSize(objtype);
        if (!max_stack) max_stack := 60000; endif
        
        var current_amount_in_stack := existing_item.amount;
        var space_in_stack := max_stack - current_amount_in_stack;
        
        if (space_in_stack > 0)
            var amount_to_attempt_stacking := (amount_to_add_or_create <= space_in_stack) ? amount_to_add_or_create : space_in_stack;
            
            var stacked_quantity := TryIncreaseAmount_Refactored(existing_item, amount_to_attempt_stacking);
            
            if (stacked_quantity > 0)
                LogInfo("SafeStackItems", $"Empilhado {stacked_quantity} de 0x{Hex(objtype)} em {existing_item.serial}.");
                amount_to_add_or_create -= stacked_quantity; // Reduz a quantidade que ainda precisa ser criada
                
                // Se houver propriedades que DEVEM ser atualizadas no stack existente após empilhar
                // (Ex: um timestamp de última modificação, mas não GeneratedForTerritory)
                // if (TypeOf(properties) == "Dictionary") {
                //     var stack_update_props := dictionary{};
                //     if (properties.exists("GeneratedTime")) { // Exemplo
                //         stack_update_props["GeneratedTime"] := properties["GeneratedTime"];
                //     }
                //     if (stack_update_props.keys().size() > 0) {
                //         ApplyPropertiesToItem(existing_item, stack_update_props);
                //     }
                // }
                
                // Se toda a quantidade foi empilhada, retornamos o item existente.
                if (amount_to_add_or_create == 0)
                    return existing_item; 
                endif
                // Se sobrou quantidade, ela será usada para criar um novo item abaixo.
            endif
        endif
    endif

    // Se não conseguiu empilhar tudo (ou nada) em um item existente, criar um novo.
    if (amount_to_add_or_create > 0)
        LogDebug("SafeStackItems", $"Passo 2: Criando novo item com {amount_to_add_or_create} unidades.");
        var new_item := CreateItemInContainer(container, objtype, amount_to_add_or_create, creator_char);
        
        if (new_item && new_item != error)
            LogInfo("SafeStackItems", $"SUCESSO na criação: Novo item {new_item.serial} (Amount: {new_item.amount}).");
            if (TypeOf(properties) == "Dictionary" && properties.Size() > 0)
                LogDebug("SafeStackItems", "Aplicando propriedades ao novo item: " + new_item.serial);
                ApplyPropertiesToItem(new_item, properties);
            else
                LogDebug("SafeStackItems", "Nenhuma propriedade adicional para aplicar ao novo item: " + new_item.serial);
            endif
            item_resultante := new_item; 
        else
            LogError("SafeStackItems", $"FALHA ao criar novo item. Erro: {(new_item && new_item.errortext ? CStr(new_item.errortext) : "Desconhecido/NULL")}");
            if (!item_resultante) 
                return error{"errortext" := "Falha total ao criar ou empilhar o item."};
            endif
        endif
    elseif (existing_item) // Se amount_to_add_or_create chegou a zero e tínhamos um existing_item
        return existing_item; // Retorna o item que foi empilhado
    endif

    return 0; // Falha geral
endfunction

/* 
 * FindExistingStackableItem_Enhanced(container, objtype, properties_to_match := 0)
 *
 * Encontra um item existente no container que pode ser empilhado,
 * considerando objtype E compatibilidade de CProps críticas.
 * properties_to_match: Dicionário de CProps que o item existente DEVE ter para ser considerado um match.
 * Retorna: ItemRef ou 0.
 */
function FindExistingStackableItem_Enhanced(container, objtype, properties_to_match := 0)
    LogDebug("FindStackable_Enhanced", $"Buscando em {container.serial} por ObjType 0x{Hex(objtype)}");
    if (properties_to_match && TypeOf(properties_to_match) == "Dictionary")
        LogDebug("FindStackable_Enhanced", "  Verificando CProps para compatibilidade: " + CStr(properties_to_match.keys()));
    endif

    foreach item_in_cont in EnumerateItemsInContainer(container)
        if (item_in_cont && item_in_cont.objtype == objtype)
            LogDebug("FindStackable_Enhanced", $"  Candidato: {item_in_cont.serial}, Amount: {item_in_cont.amount}");
            
            var max_stack := GetMaxStackSize(objtype);
            if (!max_stack) max_stack := 60000; endif

            if (item_in_cont.amount < max_stack)
                // Verificar compatibilidade de CProps
                var cprops_compatible := 1; // Assumir compatível até provar o contrário
                if (TypeOf(properties_to_match) == "Dictionary")
                    // CProps que DEVEM ser iguais para empilhar.
                    // Excluir CProps que mudam com o tempo (GeneratedTime) ou que são genéricas.
                    var critical_cprops_for_stacking := dictionary{
                        "GeneratedByFaction" := 1, // Exemplo
                        "GeneratedForTerritory" := 1, // Exemplo
                        "IsKingdomDonationMetal" := 1, // Se for específico para um tipo de doação
                        "IsKingdomDonationComida" := 1
                        // Adicionar outras CProps que definem a "unicidade" de um stack
                    };
                    // Não incluir "CreatedByResourceController" ou "GeneratedTime" na comparação para empilhamento,
                    // pois "GeneratedTime" muda e "CreatedByResourceController" será igual para todos os itens do sistema.

                    foreach prop_name in (properties_to_match.keys())
                        // Apenas verificar CProps que são consideradas críticas para a identidade do stack
                        if (critical_cprops_for_stacking.exists(prop_name))
                            var new_value := properties_to_match[prop_name];
                            var existing_value := GetObjProperty(item_in_cont, prop_name);

                            // Tratar caso onde a CProp não existe no item existente como incompatível
                            if (existing_value == error || existing_value != new_value)
                                LogDebug("FindStackable_Enhanced", $"    Incompatibilidade de CProp '{prop_name}'. Existente: '{existing_value}', Novo: '{new_value}'.");
                                cprops_compatible := 0;
                                break;
                            endif
                        endif
                    endforeach
                endif

                if (cprops_compatible)
                    LogDebug("FindStackable_Enhanced", $"  Item {item_in_cont.serial} é compatível e tem espaço. Selecionado.");
                    return item_in_cont;
                else
                    LogDebug("FindStackable_Enhanced", $"  Item {item_in_cont.serial} tem ObjType igual, mas CProps incompatíveis.");
                endif
            else
                LogDebug("FindStackable_Enhanced", $"  Item {item_in_cont.serial} está cheio (Amount: {item_in_cont.amount}).");
            endif
        endif
        Sleepms(1);
    endforeach
    LogDebug("FindStackable_Enhanced", "Nenhum item compatível encontrado.");
    return 0;
endfunction

// Arquivo: :faccao:include:faccao_resources.inc

/*
 * TryIncreaseAmount_Refactored(item, amount_to_add)
 *
 * Tenta aumentar a quantidade de um item stackable de forma robusta,
 * priorizando a função AddAmount() do sistema, se disponível.
 * Retorna: A quantidade efetivamente adicionada, ou 0 em caso de falha total.
 */
function TryIncreaseAmount_Refactored(item, amount_to_add)
    LogDebug("TryIncrease_Refactored", $"Iniciando para Item: {(item ? item.serial : "NULL")}, Add: {amount_to_add}");

    // (Validações Iniciais - como na versão anterior)
    if (!item || TypeOf(item) != "ItemRef" || amount_to_add <= 0 || !item.isA(POLCLASS_ITEM) || !item.container || !item.serial || item.serial <= 0)
        // Logs de erro específicos já estão nas validações detalhadas da versão anterior
        LogError("TryIncrease_Refactored", "Parâmetros inválidos ou item inválido.");
        return 0;
    endif
    var test_item := SystemFindObjectBySerial(CInt(item.serial));
    if (!test_item)
        LogError("TryIncrease_Refactored", $"Item {item.serial} não encontrado no sistema.");
        return 0;
    endif

    // (Diagnóstico do Item - como na versão anterior, incluindo verificação de ControlScript)
    // ...

    var original_amount := CInt(item.amount);
    var max_stack_size := GetMaxStackSize(item.objtype);
    if (!max_stack_size || max_stack_size <=0) max_stack_size := 60000; endif

    var can_add_to_stack := max_stack_size - original_amount;
    if (can_add_to_stack <= 0)
        LogInfo("TryIncrease_Refactored", $"Item {item.serial} já está no máximo ({original_amount}/{max_stack_size}).");
        return 0;
    endif

    var actual_amount_to_add := amount_to_add;
    if (amount_to_add > can_add_to_stack)
        actual_amount_to_add := can_add_to_stack;
        LogDebug("TryIncrease_Refactored", $"Ajustando quantidade para adicionar de {amount_to_add} para {actual_amount_to_add}.");
    endif

    if (actual_amount_to_add <= 0)
        LogInfo("TryIncrease_Refactored", "Após ajuste, nada a adicionar.");
        return 0;
    endif

    var target_amount := original_amount + actual_amount_to_add;
    LogDebug("TryIncrease_Refactored", $"Tentando aumentar de {original_amount} para {target_amount} (+{actual_amount_to_add})");

    // --- TENTATIVA 1: Usar a função AddAmount() do sistema ---
    // Assumindo que AddAmount() existe e foi modificada para retornar ItemRef em sucesso
    // e que ela espera a QUANTIDADE A ADICIONAR, não a quantidade final.
    var add_amount_result;
    var success_via_addamount := 0;

    // Precisamos verificar se AddAmount existe antes de chamá-la para evitar erro de "função não definida"
    // A forma de fazer isso em eScript é um pouco indireta, geralmente se coloca em um include e assume-se que está lá.
    // Para robustez, poderíamos tentar um 'eval', mas é complexo.
    // Por agora, vamos assumir que se você a usa em tantos lugares, ela existe.
    // Se ela não existir, esta chamada causará um erro de script.

    LogDebug("TryIncrease_Refactored", "--- TENTATIVA 1: Usando AddAmount(item, actual_amount_to_add) do sistema ---");
    // try // eScript não tem try-catch nativo como outros
        add_amount_result := AddAmount(item, actual_amount_to_add); // AQUI USAMOS A SUA FUNÇÃO
    // except //
    //     LogWarning("TryIncrease_Refactored", "Função AddAmount() não encontrada ou causou erro. Tentando atribuição direta.");
    //     add_amount_result := error; // Sinaliza que falhou
    // endtry

    Sleepms(50);

    // Agora AddAmount retorna ItemRef. Se sucesso, add_amount_result será o item.
    if (add_amount_result && TypeOf(add_amount_result) == "ItemRef" && add_amount_result.serial == item.serial)
        if (item.amount == target_amount)
            LogInfo("TryIncrease_Refactored", $"SUCESSO com AddAmount()! Item {item.serial} agora com {item.amount} unidades.");
            success_via_addamount := 1;
            return actual_amount_to_add;
        else
            LogError("TryIncrease_Refactored", $"AddAmount() retornou ItemRef, mas amount incorreto. Esperado: {target_amount}, Atual: {item.amount}");
            // Considerar falha se a quantidade não bate
        endif
    elseif (add_amount_result == error) // Se AddAmount retornou um erro explícito
        LogError("TryIncrease_Refactored", $"AddAmount() retornou um erro: {add_amount_result.errortext}");
    else // Se AddAmount retornou 0, ou algo que não é ItemRef (indicando falha pelo contrato antigo/novo)
        LogWarning("TryIncrease_Refactored", $"AddAmount() não retornou ItemRef esperado ou indicou falha (Retorno: {add_amount_result}, Tipo: {TypeOf(add_amount_result)}).");
    endif


    // --- TENTATIVA 2: Atribuição Direta (Fallback se AddAmount falhou ou não é a principal) ---
    // Só tentar se AddAmount não teve sucesso
    if (!success_via_addamount)
        LogDebug("TryIncrease_Refactored", "--- TENTATIVA 2: Atribuição Direta (Fallback) ---");
        item.amount := target_amount;
        Sleepms(50); 

        if (item.amount == target_amount)
            LogInfo("TryIncrease_Refactored", $"SUCESSO com Atribuição Direta (Fallback)! Item {item.serial} agora com {item.amount} unidades.");
            return actual_amount_to_add;
        else
            LogError("TryIncrease_Refactored", $"Atribuição Direta (Fallback) falhou. Esperado: {target_amount}, Atual: {item.amount}.");
        endif
    endif
    
    // --- Falha Total ---
    LogCritical("TryIncrease_Refactored", $"!!! FALHA TOTAL NO EMPILHAMENTO para item {item.serial} (ObjType: 0x{Hex(item.objtype)}) !!!");
    // ... (log de falha como antes)
    return 0; 
endfunction

// Função para mesclar propriedades entre itens
function MergeItemProperties(item, properties)
    if (!item || !properties)
        return 0;
    endif;
    
    LogDebug("MergeProps_DEBUG", "Iniciando mesclagem de propriedades para item " + item.serial);
    var merged := 0;
    
    // Se propriedades é um dicionário/struct, aplicar diretamente
    if (TypeOf(properties) == "Dictionary" || TypeOf(properties) == "Struct")
        LogDebug("MergeProps_DEBUG", "Mesclando de Dictionary/Struct com " + properties.keys().size() + " propriedades");
        foreach prop_name in (properties.keys())
            LogDebug("MergeProps_DEBUG", "  -> SetProp: " + prop_name + " = " + properties[prop_name]);
            var result := SetObjProperty(item, prop_name, properties[prop_name]);
            if (result != error)
                merged += 1;
            else
                LogError("MergeProps_DEBUG", "  -> FALHA ao definir " + prop_name + "!");
            endif;
            sleepms(1);
        endforeach;
    endif;
    
    // Se propriedades é outro item, copiar suas propriedades
    if (TypeOf(properties) == "ItemRef")
        var source_item := properties;
        var prop_names := GetObjPropertyNames(source_item);
        
        LogDebug("MergeProps_DEBUG", "Mesclando de ItemRef " + source_item.serial + " com " + prop_names.size() + " propriedades");
        foreach prop_name in prop_names
            var value := GetObjProperty(source_item, prop_name);
            LogDebug("MergeProps_DEBUG", "  -> CopyProp: " + prop_name);
            var result := SetObjProperty(item, prop_name, value);
            if (result != error)
                merged += 1;
            else
                LogError("MergeProps_DEBUG", "  -> FALHA ao copiar " + prop_name + "!");
            endif;
            sleepms(1);
        endforeach;
    else
        LogWarning("MergeProps_DEBUG", "Tipo de propriedades não suportado: " + TypeOf(properties));
        return 0;
    endif
    
    LogDebug("MergeProps_DEBUG", "Mesclagem concluída: " + merged + " propriedades aplicadas");
    return merged;
endfunction


// Função para aplicar propriedades a um item
function ApplyPropertiesToItem(item, properties)
    if (!item || !properties)
        return 0;
    endif
    
    var applied_count := 0;
    
    if (TypeOf(properties) == "Dictionary")
        foreach prop_name in (properties.keys())
            var prop_value := properties[prop_name];
            if (prop_value != error && prop_name && prop_name != "")
                SetObjProperty(item, prop_name, prop_value);
                applied_count += 1;
            endif
        endforeach
    elseif (TypeOf(properties) == "Struct")
        var prop_names := properties.keys();
        foreach prop_name in prop_names
            var prop_value := properties[prop_name];
            if (prop_value != error && prop_name && prop_name != "")
                SetObjProperty(item, prop_name, prop_value);
                applied_count += 1;
            endif
        endforeach
    else
        LogWarning("ApplyPropertiesToItem", "Tipo de propriedades não suportado: " + TypeOf(properties));
        return 0;
    endif
    
    LogDebug("ApplyPropertiesToItem", "Aplicadas " + applied_count + " propriedades ao item " + item.serial);
    return applied_count;
endfunction

// Função para substituir um item existente por um novo com quantidade acumulada
function SafeReplaceStackWithNew(old_item, amount_to_add, properties := 0)
    if (!old_item)
        LogError("ReplaceStack_DEBUG", "Erro: old_item é NULL");
        return 0;
    endif;
    
    var container := old_item.container;
    if (!container)
        LogError("ReplaceStack_DEBUG", "Erro: container do old_item é NULL. Item: " + old_item.serial);
        return 0;
    endif;
    
    LogDebug("ReplaceStack_DEBUG", "Iniciando SafeReplaceStackWithNew para item " + old_item.serial + " em container " + container.serial);
    LogDebug("ReplaceStack_DEBUG", "Quantidade atual: " + old_item.amount + ", Quantidade a adicionar: " + amount_to_add);
    
    var objtype := old_item.objtype;
    var old_amount := old_item.amount;
    var total_amount := old_amount + amount_to_add;
    
    // Verificar limite
    if (total_amount > 60000)
        LogDebug("ReplaceStack_DEBUG", "Limite de 60000 excedido. Ajustando de " + total_amount + " para 60000");
        total_amount := 60000;
    endif
    
    // Capturar propriedades do item antigo ANTES de fazer qualquer alteração
    var old_properties := dictionary{};
    var prop_names := GetObjPropertyNames(old_item);
    
    LogDebug("ReplaceStack_DEBUG", "Salvando " + prop_names.size() + " propriedades do item antigo");
    if (prop_names && TypeOf(prop_names) == "Array")
        foreach prop_name in prop_names
            old_properties[prop_name] := GetObjProperty(old_item, prop_name);
            sleepms(1);
        endforeach;
    endif;
    
    LogDebug("ReplaceStack_DEBUG", "Propriedades capturadas: " + old_properties.size());
    
    // Verificar se é apenas atualização de quantidade (mesmo objtype)
    if (amount_to_add > 0 && old_item.objtype == objtype)
        LogDebug("ReplaceStack_DEBUG", "Mesmo objtype, tentando atualização simples primeiro");
        
        // Verificar se o container ainda está válido
        if (old_item.container && old_item.container.serial == container.serial)
            old_item.amount := total_amount;
            
            // Adicionar novas propriedades se especificadas
            if (properties && TypeOf(properties) == "Dictionary")
                foreach prop_name in (properties.keys())
                    SetObjProperty(old_item, prop_name, properties[prop_name]);
                    sleepms(1);
                endforeach;
            endif;
            
            LogInfo("ReplaceStack_DEBUG", "Atualização simples bem-sucedida: " + old_item.serial + " agora com " + old_item.amount);
            return old_item;
        endif;
    endif;
    
    // Se chegou aqui, precisa criar um novo item
    LogDebug("ReplaceStack_DEBUG", "Criando novo item para substituição completa");
    
    // Obter posição do item antigo para tentar manter a mesma posição
    var old_x := old_item.x;
    var old_y := old_item.y;
    
    // Método 1: Tentar CreateItemInContainer primeiro (mais direto)
    LogDebug("ReplaceStack_DEBUG", "Criando novo item de tipo 0x" + Hex(objtype) + " com quantidade " + total_amount);
    var new_item := CreateItemInContainer(container, objtype, total_amount);
    
    if (!new_item)
        LogError("ReplaceStack_DEBUG", "FALHA na criação do novo item com CreateItemInContainer");
        
        // Método 2: Criar fora e mover (fallback)
        LogDebug("ReplaceStack_DEBUG", "Tentando método alternativo: criar fora e mover");
        new_item := CreateItemAtLocation(1000, 1000, 0, objtype, total_amount, "britannia");
        
        if (!new_item)
            LogError("ReplaceStack_DEBUG", "FALHA total na criação do novo item");
            return 0;
        endif;
        
        // Mover para o container
        var move_result := MoveItemToContainer(new_item, container, old_x, old_y);
        if (move_result != 1)
            LogError("ReplaceStack_DEBUG", "Falha ao mover item para container: " + move_result);
            DestroyItem(new_item);
            return 0;
        endif;
        
        LogDebug("ReplaceStack_DEBUG", "Item movido com sucesso para container");
    endif;
    
    LogInfo("ReplaceStack_DEBUG", "SUCESSO: Novo item criado com serial " + new_item.serial);
    
    // Transferir propriedades antigas
    LogDebug("ReplaceStack_DEBUG", "Transferindo propriedades do item antigo para o novo");
    foreach prop_name in (old_properties.keys())
        if (old_properties[prop_name] != error)
            SetObjProperty(new_item, prop_name, old_properties[prop_name]);
        endif;
        sleepms(1);
    endforeach;
    
    // Adicionar novas propriedades
    if (properties)
        LogDebug("ReplaceStack_DEBUG", "Adicionando novas propriedades ao item");
        if (TypeOf(properties) == "Dictionary")
            foreach prop_name in (properties.keys())
                SetObjProperty(new_item, prop_name, properties[prop_name]);
                sleepms(1);
            endforeach;
        else
            LogWarning("ReplaceStack_DEBUG", "Propriedades fornecidas não são um Dictionary: " + TypeOf(properties));
        endif;
    endif;
    
    LogDebug("ReplaceStack_DEBUG", "Propriedades transferidas");
    
    sleepms(50);
    
    // Verificar se o novo item está realmente válido e no container
    if (!new_item.container || new_item.container.serial != container.serial)
        LogWarn("ReplaceStack_DEBUG", "Novo item pode não estar corretamente no container");
    endif;
    
    // CRÍTICO: Destruir o item antigo com verificações extras
    if (old_item && old_item.serial && old_item.serial != new_item.serial)
        LogDebug("ReplaceStack_DEBUG", "Destruindo item antigo " + old_item.serial);
        
        sleepms(25);
        
        var destroy_result := DestroyItem(old_item);
        if (destroy_result)
            LogDebug("ReplaceStack_DEBUG", "Item antigo destruído com sucesso");
        else
            LogWarning("ReplaceStack_DEBUG", "Falha ao destruir item antigo: " + destroy_result);
            LogWarning("ReplaceStack_DEBUG", "Tentando mover item antigo para o void como fallback");
            
            var move_result := MoveObjectToLocation(old_item, 6000, 6000, 0, "britannia_alt", MOVEOBJECT_FORCELOCATION);
            if (move_result != error)
                LogDebug("ReplaceStack_DEBUG", "Item antigo movido para o void");
            else
                LogError("ReplaceStack_DEBUG", "Falha total ao remover item antigo: " + move_result);
            endif;
        endif;
    endif;
    
    LogInfo("ReplaceStack_DEBUG", "SafeReplaceStackWithNew concluído. Novo item: " + new_item.serial + " com quantidade " + new_item.amount);
    return new_item;
endfunction



/*
 * VerificarInconsistencias(container, recursos_atuais := 0, nivel := "local")
 *
 * Purpose: Verifica inconsistências entre a contagem atual e o cache/datafile
 * Parameters: 
 *   container - O baú sendo verificado
 *   recursos_atuais - A contagem atual de recursos (opcional se nivel = "global")
 *   nivel - "local" (apenas este baú) ou "global" (todos os baús da facção)
 * Return: Array de inconsistências encontradas ou contagem de inconsistências
 */
function VerificarInconsistencias(container, recursos_atuais := 0, nivel := "local")
    var inconsistencias := array{};
    
    if (nivel == "local")
        // Verificação local (um único baú)
        if (!recursos_atuais)
            recursos_atuais := ContarRecursosReais(container);
        endif
        
        var recursos_cache := GetObjProperty(container, CACHE_PROP_NAME);
        if (!recursos_cache)
            return inconsistencias; // Não há cache para comparar
        endif
        
        // Verificar cada tipo de recurso
        if (recursos_cache.comida != recursos_atuais.comida)
            inconsistencias.append(struct{
                "tipo" := "comida",
                "antigo" := recursos_cache.comida,
                "novo" := recursos_atuais.comida,
                "diferenca" := recursos_atuais.comida - recursos_cache.comida
            });
            LogDebug("recursos", "Inconsistência em comida: " + recursos_cache.comida + " vs " + recursos_atuais.comida);
        endif
             if (recursos_cache.moedas != recursos_atuais.moedas)
            inconsistencias.append(struct{
                "tipo" := "moedas",
                "antigo" := recursos_cache.moedas,
                "novo" := recursos_atuais.moedas,
                "diferenca" := recursos_atuais.moedas - recursos_cache.moedas
            });
            LogDebug("recursos", "Inconsistência em moedas: " + recursos_cache.moedas + " vs " + recursos_atuais.moedas);
        endif
        
        if (recursos_cache.couro != recursos_atuais.couro)
            inconsistencias.append(struct{
                "tipo" := "couro",
                "antigo" := recursos_cache.couro,
                "novo" := recursos_atuais.couro,
                "diferenca" := recursos_atuais.couro - recursos_cache.couro
            });
            LogDebug("recursos", "Inconsistência em couro: " + recursos_cache.couro + " vs " + recursos_atuais.couro);
        endif
        
        if (recursos_cache.metal != recursos_atuais.metal)
            inconsistencias.append(struct{
                "tipo" := "metal",
                "antigo" := recursos_cache.metal,
                "novo" := recursos_atuais.metal,
                "diferenca" := recursos_atuais.metal - recursos_cache.metal
            });
            LogDebug("recursos", "Inconsistência em metal: " + recursos_cache.metal + " vs " + recursos_atuais.metal);
        endif
        
        if (recursos_cache.madeira != recursos_atuais.madeira)
            inconsistencias.append(struct{
                "tipo" := "madeira",
                "antigo" := recursos_cache.madeira,
                "novo" := recursos_atuais.madeira,
                "diferenca" := recursos_atuais.madeira - recursos_cache.madeira
            });
            LogDebug("recursos", "Inconsistência em madeira: " + recursos_cache.madeira + " vs " + recursos_atuais.madeira);
        endif
        
        if (recursos_cache.pano != recursos_atuais.pano)
            inconsistencias.append(struct{
                "tipo" := "pano",
                "antigo" := recursos_cache.pano,
                "novo" := recursos_atuais.pano,
                "diferenca" := recursos_atuais.pano - recursos_cache.pano
            });
            LogDebug("recursos", "Inconsistência em pano: " + recursos_cache.pano + " vs " + recursos_atuais.pano);
        endif
        
        if (recursos_cache.pedras != recursos_atuais.pedras)
            inconsistencias.append(struct{
                "tipo" := "pedras",
                "antigo" := recursos_cache.pedras,
                "novo" := recursos_atuais.pedras,
                "diferenca" := recursos_atuais.pedras - recursos_cache.pedras
            });
            LogDebug("recursos", "Inconsistência em pedras: " + recursos_cache.pedras + " vs " + recursos_atuais.pedras);
        endif
             if (recursos_cache.joias != recursos_atuais.joias)
            inconsistencias.append(struct{
                "tipo" := "joias",
                "antigo" := recursos_cache.joias,
                "novo" := recursos_atuais.joias,
                "diferenca" := recursos_atuais.joias - recursos_cache.joias
            });
            LogDebug("recursos", "Inconsistência em joias: " + recursos_cache.joias + " vs " + recursos_atuais.joias);
        endif

        
        // Registrar inconsistência se encontrada
        if (inconsistencias.size() > 0)
            var historico := GetObjProperty(container, "recursos_verify_history");
            if (!historico)
                historico := array{};
            endif
            
            historico.append(struct{
                "timestamp" := polcore().systime,
                "diff" := 1
            });
            
            // Limitar o tamanho do histórico
            while (historico.size() > 20)
                historico.Erase(1);
            endwhile
            
            SetObjProperty(container, "recursos_verify_history", historico);
            RegistrarInconsistencia(container, recursos_cache, recursos_atuais, inconsistencias);
        endif
        
        return inconsistencias;
     else
        // Verificação global (todos os baús da facção)
        var faction_name := GetObjProperty(container, OBJ_FACTION_PROP);
        if (!faction_name)
            return 0;
        endif
        
        // Obter recursos do datafile
        var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE, 1); // 1 = criar
        if (!df)
            return 0;
        endif

        var elem := SafeFindElement(df, "faction_" + faction_name, 1); // 1 = criar
        var recursos_datafile := GetElementProp(elem, "recursos", CreateEmptyResourceStruct());
        
        if (!recursos_datafile)
            recursos_datafile := CreateEmptyResourceStruct();
            SetElementProp(elem, "recursos", recursos_datafile);
        endif
        
        // Contar recursos reais em todos os baús da facção
           var recursos_reais := CreateEmptyResourceStruct(); // Agora inclui moedas e joias
        var baus_contados := 0;
        
        foreach item in EnumerateItemsInContainer(0)
            if (item.objtype == 0xFF11 && GetObjProperty(item, OBJ_FACTION_PROP) == faction_name)
                baus_contados += 1;
                var bau_recursos := ContarRecursosReais(item);
                
                // Somar aos recursos totais
                recursos_reais.comida += bau_recursos.comida;
                recursos_reais.moedas += bau_recursos.moedas;     // ADICIONADO
                recursos_reais.couro += bau_recursos.couro;
                recursos_reais.metal += bau_recursos.metal;
                recursos_reais.madeira += bau_recursos.madeira;
                recursos_reais.pano += bau_recursos.pano;
                recursos_reais.pedras += bau_recursos.pedras;
                recursos_reais.joias += bau_recursos.joias;      // ADICIONADO
            endif
            Sleep(1);
        endforeach
        
        // Verificar diferenças significativas
        var tolerancia := (faction_name == KINGDOM_FACTION || faction_name == "Reino") ? 20 : 10;
        var inconsistencias_count := 0;
        
        if (Abs(recursos_reais.comida - recursos_datafile.comida) > tolerancia ||
            Abs(recursos_reais.moedas - recursos_datafile.moedas) > tolerancia ||   // ADICIONADO
            Abs(recursos_reais.couro - recursos_datafile.couro) > tolerancia ||
            Abs(recursos_reais.metal - recursos_datafile.metal) > tolerancia ||
            Abs(recursos_reais.madeira - recursos_datafile.madeira) > tolerancia ||
            Abs(recursos_reais.pano - recursos_datafile.pano) > tolerancia ||
            Abs(recursos_reais.pedras - recursos_datafile.pedras) > tolerancia ||
            Abs(recursos_reais.joias - recursos_datafile.joias) > tolerancia) // REMOVIDO || extra
            
            // Atualizar o datafile com os recursos reais
            SetElementProp(elem, "recursos", recursos_reais);
            inconsistencias_count += 1;
            
            LogWarning("recursos", "Corrigida inconsistência global nos recursos da facção: " + faction_name);
            LogInfo("recursos", "  - Baús encontrados: " + baus_contados);
            
            // Atualizar também na guilda
            var guild_id := FindGuildByName(faction_name);
            if (guild_id)
                var guild := FindGuild(guild_id);
                if (guild)
                    guild.SetProp("recursos", recursos_reais);
                    guild.SetProp("recursos_timestamp", polcore().systime);
                endif
            endif
        endif
        
        return inconsistencias_count;
    endif
endfunction

// Função auxiliar para remover espaços em branco no início e fim
function TruncateWhiteSpace(str)
    var start := 1;
    var end := Len(str);

    while (start <= end && str[start] == " ")
        start := start + 1;
    endwhile

    while (end >= start && str[end] == " ")
        end := end - 1;
    endwhile

    return str[start, end];
endfunction
// Função auxiliar para processar a string de recursos de forma segura
function ProcessResourceString(resources_string)
    // Delegar para a função centralizada em faccao_resources.inc
    return ParseResourceList(resources_string);
endfunction


/*
 * RegistrarInconsistencia(container, recursos_antigos, recursos_novos, inconsistencias)
 *
 * Purpose: Registra inconsistências encontradas entre recursos esperados e atuais
 * Parameters: 
 *   container - O baú onde a inconsistência foi encontrada
 *   recursos_antigos - Struct com os recursos esperados/anteriores
 *   recursos_novos - Struct com os recursos atuais encontrados
 *   inconsistencias - Array de structs com detalhes das inconsistências
 * Return: 1 para sucesso, 0 para falha
 */
function RegistrarInconsistencia(container, recursos_antigos, recursos_novos, inconsistencias)
    var timestamp := polcore().systime;    var container_faction := GetObjectFaction(container);
    
    // Criar mensagem detalhada para o log
    var log_msg := "[" + timestamp + "] Inconsistência no baú " + container.serial + " (Facção: " + container_faction + ")";
    
    foreach inc in inconsistencias
        log_msg += " | " + inc.tipo + ": " + inc.antigo + " -> " + inc.novo + " (Δ" + inc.diferenca + ")";
    endforeach
    
    // Registrar no log do sistema
    LogWarning("recursos", log_msg);
    
    // Salvar no datafile de inconsistências para análise posterior
var df := SafeOpenDataFile(":faccao:recursos_inconsistencias", 1);
if (df)
    var elem := SafeFindElement(df, "registros", 1);
    
    var registros := GetElementProp(elem, "list", array{});

        
        // Adicionar novo registro
        registros.append(struct{
            "tempo" := timestamp,
            "container" := container.serial,
            "faction" := container_faction,
            "antigos" := recursos_antigos,
            "novos" := recursos_novos,
            "inconsistencias" := inconsistencias
        });
        
        // Limitar o tamanho do array para não crescer indefinidamente
        while (registros.size() > 100)
            registros.erase(1);
        endwhile
        
        SetElementProp(elem, "list", registros);
    endif
    
    // Também atualizar o histórico de verificação no próprio baú
    var historico := GetObjProperty(container, "recursos_verify_history");
    if (!historico)
        historico := array{};
    endif
    
    historico.append(struct{
        "timestamp" := timestamp,
        "diff" := 1
    });
    
    // Limitar tamanho do histórico
    while (historico.size() > 20)
        historico.Erase(1);
    endwhile
    
    SetObjProperty(container, "recursos_verify_history", historico);
    
    return 1;
endfunction


/*
 * GetObjectCulture(obj)
 * Obtém a cultura de um objeto através de sua guilda
 */
function GetObjectCulture(obj)
    if (!obj)
        return DEFAULT_FACTION;
    endif
    
    var guild_id := GetObjProperty(obj, "guild_id");
    if (!guild_id)
        return DEFAULT_FACTION;
    endif
    
    // Obter objeto da guild
    var guild_obj := FindGuild(guild_id);
    if (!guild_obj)
        return DEFAULT_FACTION;
    endif
    
    // Retornar cultura da guild
    return SafeGetGuildProperty(guild_obj, "culture", DEFAULT_FACTION);
endfunction


// Funções de compatibilidade para código legado
function GetResourceCategory(resource)
    return ClassificarRecurso(resource);
endfunction

function GetResourceObjtype(resource_name)
    return MapearRecurso(resource_name);
endfunction

/*
 */
function DeterminarTipoRecursoAutomatico(item)
    if (!item)
        return "desconhecido";
    endif
    
    var objtype := 0;
    if (TypeOf(item) == "ItemRef")
        objtype := item.objtype;
    else
        objtype := CInt(item);
    endif
      // Usar ClassificarRecurso para determinar o tipo
    return ClassificarRecurso(objtype);
endfunction


function CleanDuplicateItems(container)
    if (!container)
        return 0;
    endif
    
    LogInfo("CleanDuplicates", "Limpando duplicatas no container: " + container.serial);
    
    var items_by_type := dictionary{};
    var items_to_remove := array{};
    var cleaned_count := 0;
    
    // Agrupar itens por objtype
    foreach item in EnumerateItemsInContainer(container)
        if (!item) continue; endif
        
        var key := item.objtype;
        
        if (!items_by_type.exists(key))
            items_by_type[key] := array{};
        endif
        
        items_by_type[key].append(item);
        sleepms(1);
    endforeach
    
    // Processar cada grupo
    foreach objtype in (items_by_type.keys())
        var items_list := items_by_type[objtype];
        
        if (items_list.size() > 1)
            LogDebug("CleanDuplicates", "Processando " + items_list.size() + " itens duplicados do tipo 0x" + Hex(objtype));
            
            // Encontrar item com maior quantidade para manter
            var main_item := items_list[1];
            var total_amount := main_item.amount;
            
            // Somar quantidades e marcar outros para remoção
            for i := 2 to items_list.size()
                var duplicate_item := items_list[i];
                total_amount += duplicate_item.amount;
                items_to_remove.append(duplicate_item);
                sleepms(1);
            endfor
            
            // Limitar ao máximo
            if (total_amount > 60000)
                total_amount := 60000;
            endif
            
            // Atualizar quantidade do item principal
            main_item.amount := total_amount;
            LogDebug("CleanDuplicates", "Consolidado: " + main_item.serial + " com " + total_amount + " unidades");
        endif
        
        sleepms(2);
    endforeach
    
    // Remover duplicatas
    foreach item in items_to_remove
        if (DestroyItem(item))
            cleaned_count += 1;
        endif
        sleepms(5);
    endforeach
    
    LogInfo("CleanDuplicates", "Limpeza concluída: " + cleaned_count + " duplicatas removidas");
    return cleaned_count;
endfunction


/*
 * ProcessarRecursosDeUmaFaccao(faction_name, byref updates_pending_dict_ref_dummy)
 *
 * Calcula o total de recursos para TODOS os baús de uma facção específica
 * e sincroniza esse total com o datafile e CProps da guilda.
 * Também atualiza o cache individual de cada baú processado da facção.
 *
 * MODIFICADO PARA SER CHAMADO DE FORMA SÍNCRONA E ATUALIZAR TODOS OS BAÚS DA FACÇÃO.
 * A variável 'updates_pending_dict_ref_dummy' não é ativamente usada nesta versão,
 * mas mantida para compatibilidade caso o script process_recursos_updates.src ainda a passe.
 */
exported function ProcessarRecursosDeUmaFaccao(faction_name, byref updates_pending_dict_ref_dummy)
    // Marcar parâmetro como utilizado para evitar warning
    updates_pending_dict_ref_dummy := updates_pending_dict_ref_dummy;

    if (!faction_name || faction_name == error)
        LogError("recursos", "ProcessarRecursosDeUmaFaccao_SHARED: Nome da facção inválido.");
        return 0;
    endif;

    LogInfo("recursos", "ProcessarRecursosDeUmaFaccao_SHARED: Iniciando processamento agregado para facção '" + faction_name + "'.");
    var total_recursos_da_faccao := CreateEmptyResourceStruct();
    var baus_da_faccao_processados_neste_ciclo := 0;
    var current_processing_timestamp := polcore().systime; // Usar um timestamp consistente para este ciclo

    // Passo 1: Identificar todos os baús conhecidos desta facção
    var todos_os_baus_da_faccao := array{};

    // Sub-Passo 1.1: Verificar baús registrados no datafile :faccao:seriais
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        // Formato Novo (elemento por territory_id)
        var all_territory_ids := seriais_df.keys();
        foreach terr_id_key in all_territory_ids
            if (terr_id_key == "territorios") continue; endif; // Pular o elemento legado
            var terr_elem := SafeFindElement(seriais_df, terr_id_key);
            if (terr_elem)
                var elem_faction_name := GetTerritoryFaction(terr_elem);
                if (elem_faction_name == faction_name)
                    var bau_serial := GetElementProp(terr_elem, "bau");
                    if (bau_serial)
                        var bau_obj := SystemFindObjectBySerial(CInt(bau_serial));
                        if (bau_obj && !(bau_obj in todos_os_baus_da_faccao))
                            todos_os_baus_da_faccao.append(bau_obj);
                            LogDebug("recursos_proc", "Baú " + bau_serial + " adicionado à lista (via datafile novo formato).");
                        endif;
                    endif;
                endif;
            endif;
            SleepMS(1);
        endforeach;

        // Formato Legado (dicionário "territorios")
        var territorios_seriais_legado := GetElementProp(seriais_df, "territorios", dictionary{});
        if (TypeOf(territorios_seriais_legado) == "Dictionary")
            foreach terr_id_legado in (territorios_seriais_legado.keys())
                var info_legado := territorios_seriais_legado[terr_id_legado];
                if (info_legado && info_legado.faction == faction_name && info_legado.bau)
                    var bau_obj_legado := SystemFindObjectBySerial(CInt(info_legado.bau));
                    if (bau_obj_legado && !(bau_obj_legado in todos_os_baus_da_faccao))
                        todos_os_baus_da_faccao.append(bau_obj_legado);
                        LogDebug("recursos_proc", "Baú " + info_legado.bau + " adicionado à lista (via datafile formato legado).");
                    endif;
                endif;
                SleepMS(1);
            endforeach;
        endif;
    endif; // Closes if (seriais_df)

    // Sub-Passo 1.2: Iterar sobre itens no mundo para encontrar baús da facção não registrados
    var items_scanned_world := 0;
    var max_items_to_scan_world := 10000; // Ajustável
    LogDebug("recursos_proc", "Procurando baús adicionais da facção '" + faction_name + "' no mundo...");
    foreach item_no_mundo in EnumerateItemsInContainer(0) // 0 para root (mundo)
        items_scanned_world +=1;
        if (items_scanned_world > max_items_to_scan_world)
            LogWarning("recursos_proc", "ProcessarRecursosDeUmaFaccao: Limite de scan no mundo ("+max_items_to_scan_world+") atingido para facção " + faction_name);
            break;
        endif;

        if (item_no_mundo.objtype == 0xFF11) // ObjType do baú de doações
            var chest_faction_prop := GetObjProperty(item_no_mundo, OBJ_FACTION_PROP);
            if (!chest_faction_prop) chest_faction_prop := GetObjectFaction(item_no_mundo); endif; // Fallback com função utilitária

            if (chest_faction_prop == faction_name)
                if (!(item_no_mundo in todos_os_baus_da_faccao))
                    todos_os_baus_da_faccao.append(item_no_mundo);
                    LogDebug("recursos_proc", "Baú " + item_no_mundo.serial + " adicionado à lista (encontrado no mundo).");
                endif;
            endif;
        endif;
        if (items_scanned_world % 1000 == 0) SleepMS(1); endif;
    endforeach;
    LogInfo("recursos_proc", "Total de baús identificados para facção '" + faction_name + "': " + todos_os_baus_da_faccao.size());


    // Passo 2: Processar cada baú identificado
    if (todos_os_baus_da_faccao.size() == 0)
        LogInfo("recursos", "ProcessarRecursosDeUmaFaccao_SHARED: Nenhum baú encontrado para a facção '" + faction_name + "'. Sincronizando com zero no datafile.");
        // Se não há baús, o total de recursos é zero. Sincronizar isso.
        AtualizarDatafileRecursos(faction_name, total_recursos_da_faccao);
        AtualizarPropriedadesGuildaRecursos(faction_name, total_recursos_da_faccao);
        return 1;
    endif;

    foreach bau_individual in todos_os_baus_da_faccao
        if (bau_individual && bau_individual.isA(POLCLASS_CONTAINER))
            if (GetObjProperty(bau_individual, "being_processed_by_aggregator") && (current_processing_timestamp - CInt(GetObjProperty(bau_individual, "being_processed_by_aggregator")) < 60) )
                LogWarning("recursos_proc", "Baú #" + bau_individual.serial + " pulado, já 'being_processed_by_aggregator' recentemente.");
                var cache_bau_individual_skip := GetObjProperty(bau_individual, CACHE_PROP_NAME);
                var cache_sujo_skip := GetObjProperty(bau_individual, CACHE_DIRTY_FLAG);
                if (cache_bau_individual_skip && TypeOf(cache_bau_individual_skip) == "Struct" && !cache_sujo_skip)
                     foreach key_recurso_cache_skip in (cache_bau_individual_skip.keys())
                        if (total_recursos_da_faccao.exists(key_recurso_cache_skip))
                            total_recursos_da_faccao[key_recurso_cache_skip] := total_recursos_da_faccao[key_recurso_cache_skip] + CInt(cache_bau_individual_skip[key_recurso_cache_skip]);
                        endif;
                    endforeach;
                endif;
                continue;
            endif;

            SetObjProperty(bau_individual, "being_processed_by_aggregator", current_processing_timestamp);
            var recursos_deste_bau := ContarRecursosReais(bau_individual);
            baus_da_faccao_processados_neste_ciclo += 1;
            SetObjProperty(bau_individual, CACHE_PROP_NAME, recursos_deste_bau);
            SetObjProperty(bau_individual, CACHE_UPDATE_PROP, current_processing_timestamp);
            EraseObjProperty(bau_individual, CACHE_DIRTY_FLAG);
            LogDebug("recursos_proc", "Cache individual do baú #" + bau_individual.serial + " atualizado.");

            foreach key_recurso in (recursos_deste_bau.keys())
                if (total_recursos_da_faccao.exists(key_recurso))
                    total_recursos_da_faccao[key_recurso] := total_recursos_da_faccao[key_recurso] + CInt(recursos_deste_bau[key_recurso]);
                endif;
            endforeach;
            EraseObjProperty(bau_individual, "being_processed_by_aggregator");
        else
            LogWarning("recursos_proc", "Item inválido ou não é container na lista de baús da facção " + faction_name + ". Serial (se disponível): " + (bau_individual ? bau_individual.serial : "N/A"));
        endif;
        SleepMS(50);
    endforeach;

    LogInfo("recursos_proc", "Processamento agregado para facção '" + faction_name + "' concluído.");
    LogInfo("recursos_proc", "  Baús processados neste ciclo: " + baus_da_faccao_processados_neste_ciclo);
    LogCritical("recursos_proc", "  Total de MOEDAS AGREGADO para '" + faction_name + "': " + total_recursos_da_faccao.moedas);

    // Passo 3: Sincronizar o total agregado com o Datafile e CProps da Guilda
    var sucesso_sincronizacao_agregada := SincronizarComDataFile(faction_name, total_recursos_da_faccao);

    if (sucesso_sincronizacao_agregada)
        LogInfo("recursos", "ProcessarRecursosDeUmaFaccao_SHARED: Sincronização do total para facção '" + faction_name + "' realizada com sucesso.");
    else
        LogError("recursos", "ProcessarRecursosDeUmaFaccao_SHARED: FALHA na sincronização do total para facção '" + faction_name + "'.");
    endif;

    return sucesso_sincronizacao_agregada;
endfunction

/*
 * GetActiveFactionsFromTerritories()
 * 
 * MÉTODO KISS: Encontra facções ativas baseado em territórios
 * Substitui ListGuilds() que pode ser não confiável
 * 
 * Return: Array de structs {name, guild_id, territory_id}
 */
function GetActiveFactionsFromTerritories()
    var active_factions := array{};
    var seen_factions := dictionary{};
      // Buscar via datafile de seriais (método que funciona no upkeep)
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var elem_names := seriais_df.keys();
        foreach elem_name in elem_names
            if (elem_name.find("TER_") == 1) // É um território
                var territory_elem := SafeFindElement(seriais_df, elem_name);
                if (territory_elem)
                    var faction_name := GetTerritoryFaction(territory_elem);
                    if (faction_name && !seen_factions.exists(faction_name))
                        var guild_id := FindGuildByName(faction_name); // Usar função robusta existente
                        if (guild_id)
                            var faction_info := struct{
                                "name" := faction_name,
                                "guild_id" := guild_id,
                                "territory_id" := elem_name
                            };
                            active_factions.append(faction_info);
                            seen_factions[faction_name] := 1;
                        endif
                    endif
                endif
            endif
            sleepms(1);
        endforeach
    endif
    
    return active_factions;
endfunction

function GetGuildTerritoryCount(guild_name)
    if (!guild_name)
        LogWarning("shared_functions", "GetGuildTerritoryCount chamada com guild_name vazio");  // ✅ Validação
        return 0;
    endif
    
    var count := 0;
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        LogError("shared_functions", "Falha ao abrir datafile :architect:areas em GetGuildTerritoryCount");  // ✅ Logging
        return 0;
    endif
    
    foreach rect_name in (rect_df.keys())
        var elem := SafeFindElement(rect_df, rect_name);
        if (elem)            var faction := GetTerritoryFaction(elem);
            
            if (faction == guild_name)
                count += 1;  // ✅ Incrementa corretamente
            endif
        endif
        sleepms(2);  // ✅ Anti-lag
    endforeach
    
    LogDebug("shared_functions", "GetGuildTerritoryCount(" + guild_name + ") = " + count);  // ✅ Debug detalhado
    return count;
endfunction

/*
 * GetTerritoryFaction(elem)
 * Obtém a facção de um território de forma padronizada (case-insensitive)
 */
function GetTerritoryFaction(elem)
    if (!elem)
        return "";
    endif
    var faction := GetElementProp(elem, TERRITORY_FACTION_PROP);
    if (!faction)
        faction := GetElementProp(elem, "Faction");
    endif
    if (!faction)
        faction := "";
    endif
    return faction;
endfunction

/*
 * SetTerritoryFaction(elem, faction_name)
 * Define a facção de um território de forma padronizada (ambas as props)
 */
function SetTerritoryFaction(elem, faction_name)
    if (!elem)
        return 0;
    endif
    if (!faction_name)
        faction_name := "";
    endif
    var result1 := SetElementProp(elem, TERRITORY_FACTION_PROP, faction_name);
    var result2 := SetElementProp(elem, "Faction", faction_name);
    return (result1 && result2);
endfunction

/*
 * RemoveTerritoryFaction(elem)
 * Remove a facção de um território (ambas as propriedades)
 */
function RemoveTerritoryFaction(elem)
    if (!elem)
        return 0;
    endif
    var result1 := EraseElementProp(elem, TERRITORY_FACTION_PROP);
    var result2 := EraseElementProp(elem, "Faction");
    return (result1 && result2);
endfunction

// =======================================
// FUNÇÕES UTILITÁRIAS PARA OBJETOS
// =======================================

/*
 * GetObjectFaction(obj)
 * 
 * Obtém a facção associada a um objeto, com fallback para propriedades legadas.
 * Retorna: string com o nome da facção ou "" se não houver.
 */
function GetObjectFaction(obj)
    if (!obj)
        return "";
    endif

    // Padrão novo
    var faction := GetObjProperty(obj, OBJ_FACTION_PROP);
    if (faction && faction != error)
        return faction;
    endif

    // Fallback legado
    faction := GetObjProperty(obj, "FactionName");
    if (faction && faction != error)
        return faction;
    endif    // Fallback adicional (caso alguém use "Faction" com F maiúsculo)
    faction := GetObjProperty(obj, "Faction"); // Propriedade legada
    if (faction && faction != error)
        return faction;
    endif

    return "";
endfunction

/*
 * SetObjectFaction(obj, faction_name)
 * 
 * Define a facção de um objeto de forma padronizada.
 * Remove propriedades legadas para evitar inconsistências.
 */
function SetObjectFaction(obj, faction_name)
    if (!obj)
        return 0;
    endif    // Primeiro, remover propriedades legadas para evitar inconsistências
    if (GetObjProperty(obj, "FactionName") != error)
        EraseObjProperty(obj, "FactionName");
    endif
    if (GetObjProperty(obj, "Faction") != error) // Propriedade legada
        EraseObjProperty(obj, "Faction");
    endif

    // Depois, definir a nova propriedade padrão
    SetObjProperty(obj, OBJ_FACTION_PROP, faction_name);

    return 1;
endfunction

/*
 * RemoveObjectFaction(obj)
 * 
 * Remove todas as propriedades de facção do objeto (atual e legadas).
 */
function RemoveObjectFaction(obj)
    if (!obj)
        return 0;
    endif

    if (GetObjProperty(obj, OBJ_FACTION_PROP) != error)
        EraseObjProperty(obj, OBJ_FACTION_PROP);
    endif    if (GetObjProperty(obj, "FactionName") != error)
        EraseObjProperty(obj, "FactionName");
    endif
    if (GetObjProperty(obj, "Faction") != error) // Propriedade legada
        EraseObjProperty(obj, "Faction");
    endif

    return 1;
endfunction

function GetSimpleGuildLevel(guild)
    if (!guild)
        return 1;
    endif
    
    // Tentar obter do sistema de XP da guild primeiro
    var guild_xp_info := GetGuildXPInfo(guild);
    if (guild_xp_info)
        // Tentar ambas as propriedades para compatibilidade
        if (guild_xp_info.current_level)
            return guild_xp_info.current_level;
        elseif (guild_xp_info.level)
            return guild_xp_info.level;
        endif
    endif
    
    // Fallback: tentar propriedade direta
    var level := SafeGetGuildProperty(guild, "level", 1);
    if (level && level > 0)
        return level;
    endif
    
    return 1; // Nível padrão
endfunction

function FormatNumber(number)
    var str := CStr(number);
    var result := "";
    var count := 0;
    
    // Adicionar pontos a cada 3 dígitos (da direita para esquerda)
    var i := str.size();
    while (i >= 1)

        if (count == 3)
    
            result := "." + result;
            count := 0;
        endif
        result := str[i] + result;  
        count += 1;
        i := i - 1;
       endwhile
    
    return result;

endfunction

// =============================================================================
// FUNÇÃO CENTRALIZADA DE CÁLCULO DE UPKEEP DE GUILDAS/FAÇÕES
// =============================================================================
exported function CalcularCustoUpkeep(guild, territories_info_array)
    if (!guild || !territories_info_array)
        return 0;
    endif

    if (territories_info_array.size() == 0)
        return 0; 
    endif

    var guild_level := GetSimpleGuildLevel(guild);
    var n_membros := SafeGetGuildMembers(guild).size();

    // --- CÁLCULO DO CUSTO DOS TERRITÓRIOS ---
    var custo_total_territorios := 0;
    var rect_df := SafeOpenDataFile(":architect:areas");

    if (rect_df)
        foreach territory_info in (territories_info_array)
            var elem := SafeFindElement(rect_df, territory_info.name);
            custo_total_territorios += GetTerritoryUpkeepCost(elem, guild_level);
        endforeach
    endif

    // --- CÁLCULO DO CUSTO DOS MEMBROS ---
    var custo_membros := n_membros * 150;
    if (n_membros > 10)
        custo_membros := CInt(custo_membros * 1.5);
    endif

    var total_cost := custo_total_territorios + custo_membros;
    
    LogDebug("upkeep", "Cálculo Upkeep (v3): Facção=" + SafeGetGuildName(guild) + 
                       ", Nível=" + guild_level + 
                       ", N_Territórios=" + territories_info_array.size() +
                       ", CustoTerritórios=" + custo_total_territorios +
                       ", CustoMembros=" + custo_membros +
                       ", Total=" + total_cost);

    return total_cost;
endfunction

function SafeGetGuildMembers(guild)
    if (!guild)
        return array{};
    endif
    var members := guild.members;
    if (!members || members == error || TypeOf(members) != "Array")
        return array{};
    endif
    return members;
endfunction

/*
 * GetTerritoryUpkeepCost(territory_elem, guild_level)
 *
 * Purpose: Calcula o custo de upkeep para um único território.
 */
function GetTerritoryUpkeepCost(territory_elem, guild_level)
    if (!territory_elem)
        return 0; // Custo zero se o elemento for inválido
    endif

    var base_cost := 100; // Custo mínimo para qualquer território

    // Territórios expansíveis são mais caros
    if (GetElementProp(territory_elem, "IsExpandable"))
        // O custo escala com o nível do REINO, não com o tamanho físico
        case (guild_level)
            1: base_cost := 200;
            2: base_cost := 450;
            3: base_cost := 900;
            4: base_cost := 2000;
            5: base_cost := 5000;
            default: base_cost := 200;
        endcase
    else
        // Territórios estáticos (customizados) têm um custo fixo baixo
        base_cost := 150; 
    endif

    return base_cost;
endfunction