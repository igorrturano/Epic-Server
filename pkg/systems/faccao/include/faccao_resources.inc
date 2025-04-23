use uo;
use os;
use cfgfile;
use datafile;
use math; // Adicionado para usar a função Abs

include ":datafile:datafile";
include ":faccao:faccao_constants";
include ":faccao:include/shared_functions";
include ":faccao:include/permissions";
include ":faccao:include/datafile_helpers";


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

/*
 * ContarRecursosReais(container)
 *
 * Purpose: Conta os recursos físicos em um container
 * Parameters: container - O baú a ser contado
 * Return: Struct com as contagens de recursos
 */
function ContarRecursosReais(container)
    var recursos := CreateEmptyResourceStruct();
    
    if (!container)
        return recursos;
    endif
    
    LogDebug("recursos", "Iniciando contagem física de recursos em " + container.desc);

    
    foreach item in EnumerateItemsInContainer(container)
        // Verificar se o item é uma doação válida
        if (IsKingdomDonation(item))
            var type := GetDonationType(item);
            ProcessResourceItem(item, type, recursos);
        endif
        sleepms(1);
    endforeach
    
    return recursos;
endfunction

/*
 * IsKingdomDonation(item)
 *
 * Purpose: Verifica se um item é uma doação válida para o reino/facção
 * Parameters: item - O item a verificar
 * Return: 1 se for doação, 0 caso contrário
 */
function IsKingdomDonation(item)
    // Verificar a propriedade específica
    if (GetObjProperty(item, DONATION_PREFIX))
        return 1;
    endif
    
    // Verificar propriedades específicas para cada tipo
    if (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_COMIDA) ||
        GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_MOEDAS) ||
        GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_COURO) ||
        GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_METAL) ||
        GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_MADEIRA) ||
        GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_PANO) ||
        GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_PEDRA) ||
        GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_JOIAS))
        return 1;
    endif
    
    return 0;
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


function SincronizarComDataFile(container, recursos)
    if (!container || !recursos)
        LogError("recursos", "Parâmetros inválidos para sincronização");
        return 0;
    endif
    
    // Obter facção do container com tratamento robusto
    var faction_name := GetObjProperty(container, OBJ_FACTION_PROP);
    if (faction_name == error || !faction_name)
        faction_name := GetObjProperty(container, "FactionName");
        if (faction_name == error || !faction_name)
            faction_name := GetFactionOfChest(container);
        endif
    endif
    
    if (faction_name == error || !faction_name)
        LogError("recursos", "Não foi possível determinar a facção para sincronização");
        faction_name := DEFAULT_FACTION; // Usar valor padrão se não encontrar
    endif
    
    // Registrar início da sincronização
    LogDebug("recursos", "Iniciando sincronização de recursos para facção: " + faction_name);
    
    // Verificar se os recursos são válidos antes de salvar
    recursos := ValidarRecursos(recursos);
    if (!recursos)
        LogError("recursos", "Recursos inválidos após validação");
        return 0;
    endif
    
    // SINCRONIZAÇÃO 1: Datafile global
    var df_success := AtualizarDatafileRecursos(faction_name, recursos);
    
    // SINCRONIZAÇÃO 2: Propriedades da guilda
    var guild_success := AtualizarPropriedadesGuildaRecursos(faction_name, recursos);
    
    // Registrar resultado
    LogDebug("recursos", "Sincronização concluída - Datafile: " + (df_success ? "Sucesso" : "Falha") + 
             ", Guilda: " + (guild_success ? "Sucesso" : "Falha"));
    
    // Retornar 1 se pelo menos uma das sincronizações funcionou
    return (df_success || guild_success) ? 1 : 0;
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


// Função para empilhamento seguro de itens 
function SafeStackItems(container, objtype, amount, properties := 0)
    if (!container || amount <= 0)
        return 0;
    endif
    
    // Verificar se existe um item do mesmo tipo para empilhar
    var existing_item := FindExistingStackableItem(container, objtype);
    
    // Se encontramos um item compatível para empilhar
    if (existing_item)
        // Verificar limite de empilhamento
        var max_stack := 60000; // Limite padrão do POL
        var old_amount := existing_item.amount;
        
        // Se adicionar esta quantidade excede o limite
        if (old_amount + amount > max_stack)
            // Limitar ao máximo permitido
            amount := max_stack - old_amount;
            
            // Se não podemos adicionar nada neste item
            if (amount <= 0)
                // Criar um novo item em vez de empilhar
                existing_item := 0;
            endif
        endif
        
        // Se temos um item válido e quantidade a adicionar
        if (existing_item && amount > 0)
            // Tentativa 1: Usar método direto - aumentar a quantidade
            var old_amount := existing_item.amount;
            var result := TryIncreaseAmount(existing_item, amount);
            
            // Se o aumento direto funcionou
            if (result)
                // Transferir propriedades do item novo para o existente, se fornecidas
                if (properties)
                    MergeItemProperties(existing_item, properties);
                endif
                
                return existing_item;
            else
                LogWarning("recursos", "Falha ao aumentar quantidade diretamente. Tentando método alternativo.");

            endif
        endif
    endif
    
    // Se não conseguimos empilhar em um item existente ou temos quantidade restante
    // Criar um novo item
    var new_item := CreateItemInContainer(container, objtype, amount);
    
    // Verificar se a criação foi bem-sucedida
    if (!new_item)
       LogError("recursos", "Falha ao criar novo item para empilhamento");
        return 0;
    endif
    
    // Adicionar propriedades ao novo item, se fornecidas
    if (properties)
        ApplyPropertiesToItem(new_item, properties);
    endif
    
    return new_item;
endfunction

// Função para encontrar item empilhável existente
function FindExistingStackableItem(container, objtype)
    foreach item in EnumerateItemsInContainer(container)
        if (item.objtype == objtype && item.amount < 60000)
            return item;
        endif
        sleepms(1);
    endforeach
    
    return 0;
endfunction

function TryIncreaseAmount(item, amount)
    // Verificações iniciais com logs
    LogDebug("TryIncrease_DEBUG", "Iniciando TryIncreaseAmount para item " + (item ? item.serial : "NULO") + 
             ", amount = " + amount);
             
    if (!item)
        LogError("TryIncrease_DEBUG", "Erro: Item é nulo");
        return 0;
    endif
    
    if (amount <= 0)
        LogError("TryIncrease_DEBUG", "Erro: Amount inválido (" + amount + ")");
        return 0;
    endif
    
    var original_amount := item.amount;
    var new_amount := original_amount + amount;
    
    // Verificar limites
    if (new_amount > 60000)
        LogWarning("TryIncrease_DEBUG", "Limitando nova quantidade: " + new_amount + " -> 60000");
        new_amount := 60000;
        amount := 60000 - original_amount;
    endif
    
    // Verificar propriedades do item que poderiam bloquear a alteração
    LogDebug("TryIncrease_DEBUG", "Item: " + item.serial + ", ObjType: 0x" + Hex(item.objtype) + 
             ", Quantidade atual: " + original_amount + ", Nova quantidade: " + new_amount);
    
    // Verificar se o item tem lock ou outro atributo que impediria modificação
    var locked := GetObjProperty(item, "Locked");
    if (locked)
        LogWarning("TryIncrease_DEBUG", "Item está bloqueado (Locked). Isto pode impedir alterações.");
    endif
    
    var control_script := GetObjProperty(item, "ControlScript");
    if (control_script)
        LogWarning("TryIncrease_DEBUG", "Item tem ControlScript: " + control_script + ". Pode interferir na alteração.");
    endif
    
    // Verificar se o item foi criado recentemente
    var created_time := GetObjProperty(item, "GeneratedTime");
    if (created_time)
        var time_diff := polcore().systime - CInt(created_time);
        if (time_diff < 5) // menos de 5 segundos
            LogWarning("TryIncrease_DEBUG", "Item foi criado há apenas " + time_diff + " segundos. Isso pode causar problemas.");
        endif
    endif
    
    // Tentar aplicar a nova quantidade - Versão simples sem try/except
    LogDebug("TryIncrease_DEBUG", "Tentando atribuir nova quantidade...");
    var before_change := item.amount;
    item.amount := new_amount;
    
    // Verificar se funcionou
    if (item.amount == new_amount)
        LogDebug("TryIncrease_DEBUG", "SUCESSO! Quantidade atualizada: " + item.amount);
        return amount; // Retorna a quantidade efetivamente adicionada
    else
        LogError("TryIncrease_DEBUG", "FALHA! Quantidade não foi alterada. Original: " + original_amount + ", Atual: " + item.amount);
        
        // Tentar restaurar valor original em caso de falha
        item.amount := original_amount;
        
        // Adicionar uma pequena pausa e tentar novamente uma vez como teste
        LogDebug("TryIncrease_DEBUG", "Tentando novamente após pausa...");
        sleepms(100);
        
        // Segunda tentativa
        item.amount := new_amount;
        if (item.amount == new_amount)
            LogInfo("TryIncrease_DEBUG", "Segunda tentativa TEVE SUCESSO após pausa!");
            return amount;
        else
            LogWarning("TryIncrease_DEBUG", "Segunda tentativa também falhou.");
        endif
        
        return 0;
    endif
endfunction

// Função para mesclar propriedades entre itens
function MergeItemProperties(item, properties)
    if (!item || !properties)
        return 0;
    endif
    
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
            endif
            sleepms(1);
        endforeach
    endif
    
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
            endif
            sleepms(1);
        endforeach
    endif
    
    LogDebug("MergeProps_DEBUG", "Mesclagem concluída: " + merged + " propriedades aplicadas");
    return merged;
endfunction


// Função para aplicar propriedades a um item
function ApplyPropertiesToItem(item, properties)
    LogDebug("ApplyProps_DEBUG", "Aplicando propriedades ao item " + (item ? item.serial : "NULL"));
    
    if (!item)
        LogError("ApplyProps_DEBUG", "Item inválido (NULL)");
        return 0;
    endif
    
    if (!properties)
        LogDebug("ApplyProps_DEBUG", "Nenhuma propriedade para aplicar");
        return 1; // Sucesso - não havia nada para fazer
    endif
    
    var result := MergeItemProperties(item, properties);
    LogDebug("ApplyProps_DEBUG", "Aplicação concluída: " + result + " propriedades aplicadas");
    return result;
endfunction



// Função para substituir um item existente por um novo com quantidade acumulada
function ReplaceStackWithNew(old_item, amount_to_add, properties := 0)
    if (!old_item)
        LogError("ReplaceStack_DEBUG", "Erro: old_item é NULL");
        return 0;
    endif
    
    var container := old_item.container;
    if (!container)
        LogError("ReplaceStack_DEBUG", "Erro: container do old_item é NULL. Item: " + old_item.serial);
        return 0;
    endif
    
    LogDebug("ReplaceStack_DEBUG", "Iniciando ReplaceStackWithNew para item " + old_item.serial + " em container " + container.serial);
    LogDebug("ReplaceStack_DEBUG", "Quantidade atual: " + old_item.amount + ", Quantidade a adicionar: " + amount_to_add);
    
    var objtype := old_item.objtype;
    var old_amount := old_item.amount;
    var total_amount := old_amount + amount_to_add;
    
    // Verificar limite
    if (total_amount > 60000)
        LogDebug("ReplaceStack_DEBUG", "Limite de 60000 excedido. Ajustando de " + total_amount + " para 60000");
        total_amount := 60000;
    endif
    
    // Salvar propriedades do item antigo
    var old_properties := dictionary{};
    var prop_names := GetObjPropertyNames(old_item);
    
    LogDebug("ReplaceStack_DEBUG", "Salvando " + prop_names.size() + " propriedades do item antigo");
    foreach prop_name in prop_names
        old_properties[prop_name] := GetObjProperty(old_item, prop_name);
        sleepms(1);
    endforeach
    
    // Criar novo item com quantidade total
    LogDebug("ReplaceStack_DEBUG", "Criando novo item de tipo 0x" + Hex(objtype) + " com quantidade " + total_amount);
    var new_item := CreateItemInContainer(container, objtype, total_amount);
    
    if (!new_item)
        LogError("ReplaceStack_DEBUG", "FALHA na criação do novo item. Retornando 0");
        return 0;
    endif
    
    LogInfo("ReplaceStack_DEBUG", "SUCESSO: Novo item criado com serial " + new_item.serial);
    
    // Transferir propriedades antigas
    LogDebug("ReplaceStack_DEBUG", "Transferindo propriedades do item antigo para o novo");
    foreach prop_name in (old_properties.keys())
        SetObjProperty(new_item, prop_name, old_properties[prop_name]);
        sleepms(1);
    endforeach
    
    // Adicionar novas propriedades
    if (properties)
        LogDebug("ReplaceStack_DEBUG", "Adicionando novas propriedades ao item");
        if (TypeOf(properties) == "Dictionary")
            foreach prop_name in (properties.keys())
                SetObjProperty(new_item, prop_name, properties[prop_name]);
                sleepms(1);
            endforeach
        else
            LogWarning("ReplaceStack_DEBUG", "Propriedades fornecidas não são um Dictionary: " + TypeOf(properties));
        endif
    endif
    
    // Destruir item antigo
    LogDebug("ReplaceStack_DEBUG", "Destruindo item antigo " + old_item.serial);
    var destroy_result := DestroyItem(old_item);
    if (destroy_result)
        LogDebug("ReplaceStack_DEBUG", "Item antigo destruído com sucesso");
    else
        LogWarning("ReplaceStack_DEBUG", "Falha ao destruir item antigo: " + destroy_result);
    endif
    
    LogInfo("ReplaceStack_DEBUG", "ReplaceStackWithNew concluído. Novo item: " + new_item.serial + " com quantidade " + new_item.amount);
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
            
            if (historico.size() > 20)
                historico.Erase(1); // Manter apenas os 20 mais recentes
            endif
            
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
        var recursos_reais := CreateEmptyResourceStruct();
        var baus_contados := 0;
        
        foreach item in EnumerateItemsInContainer(0)
            if (item.objtype == 0xFF11 && GetObjProperty(item, OBJ_FACTION_PROP) == faction_name)
                baus_contados += 1;
                var bau_recursos := ContarRecursosReais(item);
                
                // Somar aos recursos totais
                recursos_reais.comida += bau_recursos.comida;
                recursos_reais.moedas += bau_recursos.moedas;
                recursos_reais.couro += bau_recursos.couro;
                recursos_reais.metal += bau_recursos.metal;
                recursos_reais.madeira += bau_recursos.madeira;
                recursos_reais.pano += bau_recursos.pano;
                recursos_reais.pedras += bau_recursos.pedras;
                recursos_reais.joias += bau_recursos.joias;
            endif
            Sleep(1);
        endforeach
        
        // Verificar diferenças significativas
        var tolerancia := (faction_name == KINGDOM_FACTION || faction_name == "Reino") ? 20 : 10;
        var inconsistencias_count := 0;
        
        if (Abs(recursos_reais.comida - recursos_datafile.comida) > tolerancia ||
            Abs(recursos_reais.moedas - recursos_datafile.moedas) > tolerancia ||
            Abs(recursos_reais.couro - recursos_datafile.couro) > tolerancia ||
            Abs(recursos_reais.metal - recursos_datafile.metal) > tolerancia ||
            Abs(recursos_reais.madeira - recursos_datafile.madeira) > tolerancia ||
            Abs(recursos_reais.pano - recursos_datafile.pano) > tolerancia ||
            Abs(recursos_reais.pedras - recursos_datafile.pedras) > tolerancia ||
            Abs(recursos_reais.joias - recursos_datafile.joias) > tolerancia)
            
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

/*
 * MapearRecurso(resource_name)
 *
 * Purpose: Mapeia um nome de recurso para seu objtype correspondente
 * Parameters: resource_name - Nome do recurso a ser mapeado
 * Return: Objtype do recurso ou 0 se não encontrado
 */
/*
 * MapearRecurso(resource_name)
 *
 * Purpose: Mapeia um nome de recurso para seu objtype correspondente
 * Parameters: resource_name - Nome do recurso a ser mapeado
 * Return: Objtype do recurso ou 0 se não encontrado
 */
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
    // Recursos básicos
    resource_map["ironore"] := 0x6300;     // Minério de ferro
    resource_map["couro"] := 0x1078;       // Couro comum
    resource_map["madeira"] := 0x1BD7;     // Toras
    resource_map["comida"] := 0x097B;      // Peixe
    resource_map["pano"] := 0x1766;        // Pano
    resource_map["pedra"] := 0x1BDD;       // Pedra
    resource_map["joias"] := 0x1F13;       // Joia
    resource_map["moedas"] := 0x0EED;      // Moedas
    
    // Recursos específicos - problemas anteriores
    resource_map["acoingot"] := 0x1BF2;    // Lingote de aço
    resource_map["vidro"] := 0x1BE3;       // Vidro 
    resource_map["osso"] := 0x0F7E;        // Osso
    
    // Recursos adicionais
    resource_map["couroespesso"] := 0xEF12; // Couro espesso
    resource_map["couroescamoso"] := 0xEF11; // Couro escamoso
    resource_map["couropolar"] := 0xEF17;   // Couro polar
    resource_map["courodraconiano"] := 0xED20; // Couro draconiano
    resource_map["pelebranca"] := 0xEE52;  // Pele branca
    resource_map["madeirapinheiro"] := 0xEE36; // Madeira de pinheiro
    resource_map["madeirateixo"] := 0xEE3E; // Madeira de teixo
    resource_map["carvao"] := 0x1BDE;      // Carvão
    resource_map["prataingot"] := 0x6993;  // Lingote de prata
    resource_map["escamasazuis"] := 0xEF36; // Escamas azuis
    resource_map["ossogigante"] := 0xEF3A; // Osso gigante
    resource_map["ossodemoniaco"] := 0xEF29; // Osso demoníaco
    resource_map["cogumelo"] := 0x0D16;    // Cogumelo
    
    // Verificar mapeamento direto - ÚNICA TENTATIVA DE MAPEAMENTO
    if (resource_map.exists(resource_name))
        LogDebug("recursos", "MapearRecurso: correspondência direta '" + resource_name + "' -> 0x" + Hex(resource_map[resource_name]));
        return resource_map[resource_name];
    endif
    
    // Falha no mapeamento - não usar correspondência parcial ou fallback genérico
    LogError("recursos", "Recurso não mapeado: '" + resource_name + "'. Adicione este recurso ao dicionário em MapearRecurso.");
    return 0; // Retorna 0 para indicar falha
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
    var timestamp := polcore().systime;
    var container_faction := GetObjProperty(container, OBJ_FACTION_PROP);
    if (!container_faction)
        container_faction := GetObjProperty(container, "faction");
    endif
    
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



// Funções de compatibilidade para código legado
function GetResourceCategory(resource)
    return ClassificarRecurso(resource);
endfunction

function GetResourceObjtype(resource_name)
    return MapearRecurso(resource_name);
endfunction

/*
 * DeterminarTipoRecursoAutomatico(item)
 *
 * Purpose: Determina automaticamente o tipo de recurso com base no objtype
 * Parameters: item - O item a ser analisado
 * Return: String com o tipo de recurso
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

/*
 * GetActiveFactions()
 *
 * Purpose: Obtém uma lista de facções ativas no servidor
 * Parameters: Nenhum
 * Return: Array com nomes de facções ativas
 */
function GetActiveFactions()
    var active_factions := array{};
    
    // Adicionar o Reino como facção padrão
    active_factions.append(KINGDOM_FACTION);
    
    // Obter facções de guildas
    var guilds := ListGuilds();
    foreach guild_id in guilds
        var guild := FindGuild(guild_id);
        if (guild && guild.members.size() > 0)
            var faction_name := guild.GetProp("name");
            if (!faction_name)
                faction_name := guild.guildname;
            endif
            
            if (faction_name && !(faction_name in active_factions))
                active_factions.append(faction_name);
            endif
        endif
        sleepms(1);
    endforeach
    
    // Verificar também no datafile
var df := SafeOpenDataFile(KINGDOM_RESOURCE_DATAFILE);
if (df)
    foreach key in (df.keys())
        if (key.find("faction_") == 0)
            var faction_name := key[8, key.size()-7]; // Remover o prefixo "faction_"
            if (!(faction_name in active_factions))
                active_factions.append(faction_name);
            endif
        endif
        sleepms(1);
    endforeach
endif
    
    return active_factions;
endfunction