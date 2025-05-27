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
        var new_item := CreateItemInContainer(container, objtype, amount_to_add_or_create);
        if (new_item && new_item != error)
            LogInfo("SafeStackItems", $"Criado novo item {new_item.serial} com {new_item.amount} de 0x{Hex(objtype)}.");
            if (TypeOf(properties) == "Dictionary")
                ApplyPropertiesToItem(new_item, properties);
            endif
            return new_item; // Retorna o novo item
        else
            LogError("SafeStackItems", $"Falha ao criar novo item para 0x{Hex(objtype)}.");
            return 0; // Ou error
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
                sleepms(1);
            endif
        endforeach
    elseif (TypeOf(properties) == "Struct")
        var prop_names := properties.keys();
        foreach prop_name in prop_names
            var prop_value := properties[prop_name];
            if (prop_value != error && prop_name && prop_name != "")
                SetObjProperty(item, prop_name, prop_value);
                applied_count += 1;
                sleepms(1);
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
    endif
    
    var container := old_item.container;
    if (!container)
        LogError("ReplaceStack_DEBUG", "Erro: container do old_item é NULL. Item: " + old_item.serial);
        return 0;
    endif
    
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
        endforeach
    endif
    
    LogDebug("ReplaceStack_DEBUG", "Propriedades capturadas: " + old_properties.size());
    
    // Verificar se é apenas atualização de quantidade (mesmo objtype)
    // Para casos simples onde só aumentamos a quantidade
    if (amount_to_add > 0 && old_item.objtype == objtype)
        LogDebug("ReplaceStack_DEBUG", "Mesmo objtype, tentando atualização simples primeiro");
        
        // Obter posição do item antigo para verificação
        var old_pos_x := old_item.x;
        var old_pos_y := old_item.y;
        
        // Verificar se o container ainda está válido
        if (old_item.container && old_item.container.serial == container.serial)
            old_item.amount := total_amount;
            
            // Adicionar novas propriedades se especificadas
            if (properties && TypeOf(properties) == "Dictionary")
                foreach prop_name in (properties.keys())
                    SetObjProperty(old_item, prop_name, properties[prop_name]);
                    sleepms(1);
                endforeach
            endif
            
            LogInfo("ReplaceStack_DEBUG", "Atualização simples bem-sucedida: " + old_item.serial + " agora com " + old_item.amount);
            return old_item;
        endif
    endif
    
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
        endif
        
        // Mover para o container
        var move_result := MoveItemToContainer(new_item, container, old_x, old_y);
        if (move_result != 1)
            LogError("ReplaceStack_DEBUG", "Falha ao mover item para container: " + move_result);
            DestroyItem(new_item);
            return 0;
        endif
        
        LogDebug("ReplaceStack_DEBUG", "Item movido com sucesso para container");
    endif
    
    LogInfo("ReplaceStack_DEBUG", "SUCESSO: Novo item criado com serial " + new_item.serial);
    
    // Transferir propriedades antigas
    LogDebug("ReplaceStack_DEBUG", "Transferindo propriedades do item antigo para o novo");
    foreach prop_name in (old_properties.keys())
        if (old_properties[prop_name] != error)
            SetObjProperty(new_item, prop_name, old_properties[prop_name]);
        endif
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
    
    LogDebug("ReplaceStack_DEBUG", "Propriedades transferidas");
    
    // Aguardar um momento para garantir que o item foi processado
    sleepms(50);
    
    // Verificar se o novo item está realmente válido e no container
    if (!new_item.container || new_item.container.serial != container.serial)
        LogWarn("ReplaceStack_DEBUG", "Novo item pode não estar corretamente no container");
        // Mas continuar mesmo assim, pois pode ser questão de timing
    endif
    
    // CRÍTICO: Destruir o item antigo com verificações extras
    if (old_item && old_item.serial && old_item.serial != new_item.serial)
        LogDebug("ReplaceStack_DEBUG", "Destruindo item antigo " + old_item.serial);
        
        // Aguardar um pouco mais para garantir estabilidade
        sleepms(25);
        
 var destroy_result := DestroyItem(old_item);
        if (destroy_result)
            LogDebug("ReplaceStack_DEBUG", "Item antigo destruído com sucesso");
        else
            LogWarning("ReplaceStack_DEBUG", "Falha ao destruir item antigo: " + destroy_result);
            LogWarning("ReplaceStack_DEBUG", "Tentando mover item antigo para o void como fallback");
            
            // Tentar mover para uma localização inacessível como fallback
            var move_result := MoveObjectToLocation(old_item, 6000, 6000, 0, "britannia_alt", MOVEOBJECT_FORCELOCATION);
            if (move_result != error)
                LogDebug("ReplaceStack_DEBUG", "Item antigo movido para o void");
            else
                LogError("ReplaceStack_DEBUG", "Falha total ao remover item antigo: " + move_result);
            endif
        endif
    endif
    
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
    resource_map["prataingot"] := 0x6993;  // Lingote de prata
    resource_map["frigutiumingot"] := 0x6460;    // Lingote de frigutium
    resource_map["couroespesso"] := 0xEF12; // Couro espesso
    resource_map["couropolar"] := 0xEF17;   // Couro polar
    resource_map["couroescamoso"] := 0xEF11; // Couro escamoso
    resource_map["courodraconiano"] := 0xED20; // Couro draconiano
    resource_map["escamasazuis"] := 0xEF36; // Escamas azuis
    resource_map["madeirapinheiro"] := 0xEE36; // Madeira de pinheiro
    resource_map["madeirateixo"] := 0xEE3E; // Madeira de teixo
    resource_map["carvao"] := 0x1BDE;      // Carvão
    resource_map["cogumelo"] := 0xd16;    // 
    resource_map["pelebranca"] := 0xEE52;  // Pele branca
    resource_map["ossocinza"] := 0xef32;        // Osso
    resource_map["glass"] := 0x1BE3;       // Vidro 
    resource_map["ossogigante"] := 0xEF3A; // Osso gigante
    resource_map["ossodemoniaco"] := 0xEF29; // Osso demoníaco
    
    
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
