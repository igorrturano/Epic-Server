// :faccao:include/territory_management.inc v1.0
//
// Módulo centralizado para gerenciamento de territórios
// Segue o princípio KISS (Keep It Simple, Stupid)

use uo;
use os;
use datafile;

include ":datafile:datafile";
include ":faccao:faccao_constants";
include ":faccao:include/logging";
include ":faccao:include/datafile_helpers";

// =======================================
// Funções de Busca Essenciais
// =======================================

/*
 * TM_FindTerritory(territory_id_or_name)
 *
 * Purpose: Localiza um território pelo ID ou nome
 * Parameters:
 *   territory_id_or_name - ID ou nome do território
 * Return:
 *   Elemento do datafile ou 0 se não encontrado
 */
function TM_FindTerritory(territory_id_or_name)
    if (!territory_id_or_name)
        LogError("territorio", "TM_FindTerritory chamado com parâmetro nulo");
        return 0;
    endif
    
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        LogError("territorio", "Falha ao abrir datafile :architect:areas");
        return 0;
    endif
    
    // 1. Tentar encontrar diretamente pelo nome (caso o ID seja o nome)
    var elem := SafeFindElement(rect_df, territory_id_or_name);
    if (elem)
        return elem;
    endif
    
    // 2. Procurar por ID em todos os elementos
    foreach elem_name in (rect_df.keys())
        elem := SafeFindElement(rect_df, elem_name);
        if (elem)
            var id := GetElementProp(elem, TERRITORY_ID_PROP);
            if (id == territory_id_or_name)
                return elem;
            endif
        endif
        sleepms(2);
    endforeach
    
    LogWarning("territorio", "Território não encontrado: " + territory_id_or_name);
    return 0;
endfunction


/*
 * TM_GetTerritoryInfo(territory_id_or_name)
 *
 * Purpose: Obtém informações de um território
 * Parameters:
 *   territory_id_or_name - ID ou nome do território
 * Return:
 *   Struct com as informações ou 0 se não encontrado
 */
function TM_GetTerritoryInfo(territory_id_or_name)
    var territory_elem := TM_FindTerritory(territory_id_or_name);
    if (!territory_elem)
        return 0;
    endif
    
    var info := struct{
        "id" := GetElementProp(territory_elem, TERRITORY_ID_PROP, territory_id_or_name),
        "name" := GetElementProp(territory_elem, TERRITORY_NAME_PROP, territory_id_or_name),
        "rect" := GetElementProp(territory_elem, TERRITORY_RECT_PROP),
        "realm" := GetElementProp(territory_elem, TERRITORY_REALM_PROP, "britannia"),
        "faction" := GetElementProp(territory_elem, TERRITORY_FACTION_PROP),
        "is_capital" := GetElementProp(territory_elem, TERRITORY_IS_CAPITAL_PROP, 0),
        "resources" := GetElementProp(territory_elem, "Resources")
    };
    
    // Compatibilidade: verificar propriedade alternativa para faction
    if (!info.faction)
        info.faction := GetElementProp(territory_elem, "Faction");
    endif
    
    return info;
endfunction


/*
 * TM_IsPointInTerritory(x, y, z, realm, territory_id_or_name)
 *
 * Purpose: Verifica se um ponto está dentro de um território
 * Parameters:
 *   x, y, z - Coordenadas a verificar
 *   realm - Realm a verificar
 *   territory_id_or_name - ID ou nome do território
 * Return:
 *   1 se o ponto está no território, 0 caso contrário
 */
function TM_IsPointInTerritory(x, y, z, realm, territory_id_or_name)
    var territory_info := TM_GetTerritoryInfo(territory_id_or_name);
    if (!territory_info || !territory_info.rect)
        return 0;
    endif
    
    var rect := SplitWords(territory_info.rect);
    if (rect.size() < 4)
        return 0;
    endif
    
    var x1 := CInt(rect[1]);
    var y1 := CInt(rect[2]);
    var x2 := CInt(rect[3]);
    var y2 := CInt(rect[4]);
    
    // Verificar realm
    if (territory_info.realm && territory_info.realm != realm)
        return 0;
    endif
    
    // Verificar coordenadas
    return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
endfunction


/*
 * TM_IsMobileInTerritory(mobile, territory_id_or_name)
 *
 * Purpose: Verifica se um mobile está dentro de um território
 * Parameters:
 *   mobile - O mobile a verificar
 *   territory_id_or_name - ID ou nome do território
 * Return:
 *   1 se o mobile está no território, 0 caso contrário
 */
function TM_IsMobileInTerritory(mobile, territory_id_or_name)
    if (!mobile)
        return 0;
    endif
    
    return TM_IsPointInTerritory(mobile.x, mobile.y, mobile.z, mobile.realm, territory_id_or_name);
endfunction


// =======================================
// Funções de Modificação Básicas
// =======================================

/*
 * TM_SetTerritoryFaction(territory_id_or_name, faction_name)
 *
 * Purpose: Define a facção controladora de um território
 * Parameters:
 *   territory_id_or_name - ID ou nome do território
 *   faction_name - Nome da facção
 * Return:
 *   1 para sucesso, 0 para falha
 */
function TM_SetTerritoryFaction(territory_id_or_name, faction_name)
    var territory_elem := TM_FindTerritory(territory_id_or_name);
    if (!territory_elem)
        LogError("territorio", "TM_SetTerritoryFaction: Território não encontrado: " + territory_id_or_name);
        return 0;
    endif
    
    // Definir a propriedade principal
    var result := SetElementProp(territory_elem, TERRITORY_FACTION_PROP, faction_name);
    
    // Definir também a propriedade alternativa para compatibilidade
    SetElementProp(territory_elem, "Faction", faction_name);
    
    LogInfo("territorio", "Facção do território " + territory_id_or_name + " definida para " + faction_name);
    
    return result;
endfunction

/*
 * TM_GetTerritoryCenter(territory_id_or_name)
 *
 * Purpose: Obtém o ponto central de um território
 * Parameters:
 *   territory_id_or_name - ID ou nome do território
 * Return:
 *   Struct com {x, y, z, realm} ou 0 se não encontrado
 */
function TM_GetTerritoryCenter(territory_id_or_name)
    var territory_info := TM_GetTerritoryInfo(territory_id_or_name);
    if (!territory_info || !territory_info.rect)
        return 0;
    endif
    
    var rect := SplitWords(territory_info.rect);
    if (rect.size() < 4)
        return 0;
    endif
    
    var x1 := CInt(rect[1]);
    var y1 := CInt(rect[2]);
    var x2 := CInt(rect[3]);
    var y2 := CInt(rect[4]);
    
    var center := struct{
        "x" := CInt((x1 + x2) / 2),
        "y" := CInt((y1 + y2) / 2),
        "z" := GetWorldHeight(CInt((x1 + x2) / 2), CInt((y1 + y2) / 2), territory_info.realm),
        "realm" := territory_info.realm
    };
    
    return center;
endfunction

// =======================================
// Funções de Verificação Chave
// =======================================

/*
 * TM_CreateUniqueItemID(faction_name, item_type, territory_id)
 *
 * Purpose: Cria um ID único padronizado para itens de facção
 * Parameters:
 *   faction_name - Nome da facção
 *   item_type - Tipo do item (BAU, MESA, MURAL)
 *   territory_id - ID do território
 * Return:
 *   String formatada como FactionName:ItemType:TerritoryID
 */
function TM_CreateUniqueItemID(faction_name, item_type, territory_id)
    return faction_name + ":" + item_type + ":" + territory_id;
endfunction

// Renomear a função "interna" para evitar recursão
function TM_DoRectanglesOverlap(rect1, rect2, realm1 := "", realm2 := "")
    // Se realms são diferentes, não há sobreposição
    if (realm1 && realm2 && realm1 != realm2)
        return 0;
    endif
    
    // Converter strings para arrays
    if (TypeOf(rect1) == "String")
        rect1 := SplitWords(rect1);
    endif
    
    if (TypeOf(rect2) == "String")
        rect2 := SplitWords(rect2);
    endif
    
    // Verificar se os arrays têm tamanho correto
    if (rect1.size() < 4 || rect2.size() < 4)
        return 0;
    endif
    
    // Extrair coordenadas
    var r1_x1 := CInt(rect1[1]);
    var r1_y1 := CInt(rect1[2]);
    var r1_x2 := CInt(rect1[3]);
    var r1_y2 := CInt(rect1[4]);
    
    var r2_x1 := CInt(rect2[1]);
    var r2_y1 := CInt(rect2[2]);
    var r2_x2 := CInt(rect2[3]);
    var r2_y2 := CInt(rect2[4]);
    
    // Ordenar coordenadas (garantir que x1 < x2 e y1 < y2)
    if (r1_x1 > r1_x2)
        var temp := r1_x1;
        r1_x1 := r1_x2;
        r1_x2 := temp;
    endif
    
    if (r1_y1 > r1_y2)
        var temp := r1_y1;
        r1_y1 := r1_y2;
        r1_y2 := temp;
    endif
    
    if (r2_x1 > r2_x2)
        var temp := r2_x1;
        r2_x1 := r2_x2;
        r2_x2 := temp;
    endif
    
    if (r2_y1 > r2_y2)
        var temp := r2_y1;
        r2_y1 := r2_y2;
        r2_y2 := temp;
    endif
    
    // Verificar sobreposição
    return (r1_x1 <= r2_x2 && r1_x2 >= r2_x1 && 
            r1_y1 <= r2_y2 && r1_y2 >= r2_y1);
endfunction

// Função principal que verifica no datafile
function TM_CheckRectangleOverlap(rect_df, x1, y1, x2, y2, realm, exclude_name := "")
    foreach rect_name in (rect_df.keys())
        // Pular a verificação se for o próprio retângulo que estamos editando
        if (rect_name == exclude_name)
            continue;
        endif
        
        // Usar TM_GetTerritoryInfo para obter informações do território
        var territory_info := TM_GetTerritoryInfo(rect_name);
        if (!territory_info)
            continue;
        endif
        
        var rect_realm := territory_info.realm;
        
        // Se não estiver no mesmo realm, não há sobreposição
        if (rect_realm != realm)
            continue; 
        endif
        
        var rect_str := territory_info.rect;
        if (!rect_str)
            continue;
        endif
        
        // Usar a função interna para verificar sobreposição
        var rect1 := array{x1, y1, x2, y2};
        var rect2 := SplitWords(rect_str);
        
        if (TM_DoRectanglesOverlap(rect1, rect2, realm, rect_realm))
            return 1; // Há sobreposição
        endif
    endforeach
    
    return 0; // Não há sobreposição
endfunction

// =======================================
// Funções de Compatibilidade (proxies para funções existentes)
// =======================================

/*
 * TM_SetTerritoryAsCapital(territory_id, faction_name)
 *
 * Purpose: Define um território como capital de uma facção
 * Parameters:
 *   territory_id - ID do território
 *   faction_name - Nome da facção
 * Return:
 *   1 para sucesso, 0 para falha
 */
function TM_SetTerritoryAsCapital(territory_id, faction_name)
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
    var territory_elem := TM_FindTerritory(territory_id);
    if (territory_elem)
        SetElementProp(territory_elem, TERRITORY_IS_CAPITAL_PROP, 1);
        LogInfo("territorio", "Território " + territory_id + " definido como capital de " + faction_name);
        return 1;
    endif
    
    LogError("territorio", "TM_SetTerritoryAsCapital: Território não encontrado: " + territory_id);
    return 0;
endfunction

/*
 * TM_IsTerritoryCapital(territory_id)
 *
 * Purpose: Verifica se um território é capital
 * Parameters:
 *   territory_id - ID do território
 * Return:
 *   1 se for capital, 0 caso contrário
 */
function TM_IsTerritoryCapital(territory_id)
    var territory_elem := TM_FindTerritory(territory_id);
    if (territory_elem)
        var prop := GetElementProp(territory_elem, TERRITORY_IS_CAPITAL_PROP);
        if (prop)
            return 1;
        endif
    endif
    
    return 0;
endfunction


/*
 * Compatibilidade com architect - versão TM
 */
function TM_AT_InRect(who, rect)
    var x1 := CInt(rect[1]);
    var y1 := CInt(rect[2]);
    var x2 := CInt(rect[3]);
    var y2 := CInt(rect[4]);
    var realm := rect[5];

    return (who.realm == realm && (CInt(who.x) >= x1 && CInt(who.x) <= x2) && (CInt(who.y) >= y1 && CInt(who.y) <= y2));
endfunction


// =======================================
// Funções para Gerenciamento de Objetos Territoriais
// =======================================

/*
 * TM_RegisterTerritoryObjects(territory_id, object_serials)
 *
 * Purpose: Registra objetos associados a um território no datafile central
 * Parameters:
 *   territory_id - ID do território
 *   object_serials - Struct com seriais dos objetos (bau, mesa, mural, etc)
 * Return:
 *   1 para sucesso, 0 para falha
 */
function TM_RegisterTerritoryObjects(territory_id, object_serials)
    if (!territory_id || !object_serials)
        LogError("territorio", "TM_RegisterTerritoryObjects: parâmetros inválidos");
        return 0;
    endif
    
    var seriais_df := SafeOpenDataFile(":faccao:seriais", 1); // 1 = criar se não existir
    if (!seriais_df)
        LogError("territorio", "TM_RegisterTerritoryObjects: Não foi possível acessar o datafile de seriais");
        return 0;
    endif
    
    var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
    if (!territorios_seriais)
        territorios_seriais := dictionary{};
    endif
    
    // Combinar com dados existentes, se houver
    var existing_data := territorios_seriais[territory_id];
    if (!existing_data)
        existing_data := struct{};
    endif
    
    // Adicionar/atualizar cada serial fornecido
    foreach key in (object_serials.keys())
        existing_data[key] := object_serials[key];
        sleepms(1);
    endforeach
    
    // Salvar no datafile
    territorios_seriais[territory_id] := existing_data;
    var result := SetElementProp(seriais_df, "territorios", territorios_seriais);
    
    LogInfo("territorio", "Objetos registrados para território " + territory_id + ": " + 
            object_serials.keys().size() + " objetos");
    
    return result;
endfunction


/*
 * TM_FindTerritoryObjects(territory_id, faction_name := "", item_type := "")
 *
 * Purpose: Encontra objetos associados a um território
 * Parameters:
 *   territory_id - ID do território
 *   faction_name - Nome da facção (opcional)
 *   item_type - Tipo do objeto (BAU, MESA, MURAL - opcional)
 * Return:
 *   Struct com objetos encontrados ou struct vazio se não encontrado
 */
function TM_FindTerritoryObjects(territory_id, faction_name := "", item_type := "")
    var result := struct{
        "bau" := 0,
        "mesa" := 0,
        "mural" := 0
    };
    
    if (!territory_id)
        LogError("territorio", "TM_FindTerritoryObjects: Território não especificado");
        return result;
    endif
    
    // Estratégia 1: Buscar pelos registros no datafile
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
        if (territorios_seriais && territorios_seriais[territory_id])
            var info := territorios_seriais[territory_id];
            
            // Verificar se a facção corresponde, se especificada
            if (faction_name && info.faction && info.faction != faction_name)
                LogDebug("territorio", "Facção não corresponde: " + info.faction + " vs " + faction_name);
            else
                // Adicionar os objetos encontrados conforme o tipo solicitado
                if (!item_type || item_type == ITEM_TYPE_BAU)
                    if (info.bau)
                        result.bau := SystemFindObjectBySerial(info.bau);
                    endif
                endif
                
                if (!item_type || item_type == ITEM_TYPE_MESA)
                    if (info.mesa)
                        result.mesa := SystemFindObjectBySerial(info.mesa);
                    endif
                endif
                
                if (!item_type || item_type == ITEM_TYPE_MURAL)
                    if (info.mural)
                        result.mural := SystemFindObjectBySerial(info.mural);
                    endif
                endif
                
                return result;
            endif
        endif
    endif
    
    // Estratégia 2: Busca no mundo se não encontrou no datafile
    var territory_info := TM_GetTerritoryInfo(territory_id);
    if (!territory_info)
        LogError("territorio", "TM_FindTerritoryObjects: Informações do território não encontradas");
        return result;
    endif
    
    // Se a facção não foi especificada, usar a do território
    if (!faction_name)
        faction_name := territory_info.faction;
    endif
    
    // Buscar objetos por propriedades específicas
    foreach item in EnumerateItemsInContainer(0) // 0 = raiz do mundo
        if (GetObjProperty(item, TERRITORY_ID_PROP) == territory_id)
            var faction_obj := GetObjProperty(item, OBJ_FACTION_PROP);
            
            if (!faction_name || faction_obj == faction_name)
                if (!item_type || 
                    (item_type == ITEM_TYPE_BAU && item.objtype == 0xFF11) ||
                    (item_type == ITEM_TYPE_MESA && item.objtype == 0xFE5C) ||
                    (item_type == ITEM_TYPE_MURAL && item.objtype == 0x46AE))
                    
                    if (item.objtype == 0xFF11)
                        result.bau := item;
                    elseif (item.objtype == 0xFE5C)
                        result.mesa := item;
                    elseif (item.objtype == 0x46AE)
                        result.mural := item;
                    endif
                endif
            endif
        endif
        sleepms(1);
    endforeach
    
    return result;
endfunction


// =======================================
// Funções para Gerenciamento de Recursos
// =======================================

/*
 * TM_UpdateTerritoryResources(territory_id, resources, interval, amount)
 *
 * Purpose: Atualiza configurações de recursos de um território
 * Parameters:
 *   territory_id - ID do território
 *   resources - String com recursos separados por vírgula ou array de recursos
 *   interval - Intervalo de geração em minutos
 *   amount - Quantidade de cada recurso gerado
 * Return:
 *   1 para sucesso, 0 para falha
 */
function TM_UpdateTerritoryResources(territory_id, resources, interval, amount)
    var territory_elem := TM_FindTerritory(territory_id);
    if (!territory_elem)
        LogError("territorio", "TM_UpdateTerritoryResources: Território não encontrado: " + territory_id);
        return 0;
    endif
    
    // Formatar recursos como string se for um array
    var resources_str;
    if (TypeOf(resources) == "Array")
        resources_str := "";
        for i := 1 to resources.size()
            if (i > 1)
                resources_str += ", ";
            endif
            resources_str += resources[i];
        endfor
    else
        resources_str := CStr(resources);
    endif
    
    // Salvar as configurações
    SetElementProp(territory_elem, "Resources", resources_str);
    SetElementProp(territory_elem, "ResourceInterval", CInt(interval));
    SetElementProp(territory_elem, "ResourceAmount", CInt(amount));
    
    // Configurar próxima geração
    var next_time := polcore().systime + (interval * 60); // Converter minutos para segundos
    SetElementProp(territory_elem, "NextResourceTime", next_time);
    
    LogInfo("territorio", "Recursos atualizados para território " + territory_id + ": " + 
            resources_str + " (cada " + interval + " minutos, " + amount + " unidades)");
    
    return 1;
endfunction

/*
 * TM_GetTerritoryResourceInfo(territory_id)
 *
 * Purpose: Obtém informações sobre recursos de um território
 * Parameters:
 *   territory_id - ID do território
 * Return:
 *   Struct com informações de recursos ou 0 se não encontrado
 */
function TM_GetTerritoryResourceInfo(territory_id)
    var territory_elem := TM_FindTerritory(territory_id);
    if (!territory_elem)
        LogError("territorio", "TM_GetTerritoryResourceInfo: Território não encontrado: " + territory_id);
        return 0;
    endif
    
    var resources_str := GetElementProp(territory_elem, "Resources", "");
    var interval := GetElementProp(territory_elem, "ResourceInterval", 0);
    var amount := GetElementProp(territory_elem, "ResourceAmount", 0);
    var next_time := GetElementProp(territory_elem, "NextResourceTime", 0);
    
    // Converter string de recursos para array
    var resources_array := array{};
    if (resources_str)
        resources_array := SplitWords(resources_str, ",");
        
        // Limpar espaços extras
        for i := 1 to resources_array.size()
            resources_array[i] := TrimString(resources_array[i]);
        endfor
    endif
    
    var info := struct{
        "resources_str" := resources_str,
        "resources" := resources_array,
        "interval" := interval,
        "amount" := amount,
        "next_time" := next_time,
        "time_left" := next_time ? (next_time - polcore().systime) : 0
    };
    
    return info;
endfunction

// =======================================
// NOVAS FUNÇÕES ADICIONADAS NA ETAPA 3
// =======================================
/*
 * TM_CreateNewTerritory(territory_name, x1, y1, x2, y2, realm, faction_name := "")
 *
 * Purpose: Cria um novo território de forma centralizada
 * Parameters:
 *   territory_name - Nome do território
 *   x1, y1, x2, y2 - Coordenadas do retângulo
 *   realm - Realm do território
 *   faction_name - Nome da facção (opcional, padrão: Selvagem)
 * Return:
 *   ID do território criado ou 0 em caso de falha
 */
function TM_CreateNewTerritory(territory_name, x1, y1, x2, y2, realm, faction_name := "")
    if (!territory_name)
        LogError("territorio", "TM_CreateNewTerritory: Nome do território não fornecido");
        return 0;
    endif
    
    // Abrir datafile de territórios
    var rect_df := SafeOpenDataFile(":architect:areas", 1); // 1 = criar se não existir
    if (!rect_df)
        LogError("territorio", "TM_CreateNewTerritory: Falha ao abrir datafile de áreas");
        return 0;
    endif
    
    // Verificar se o nome já existe
    var existing_elem := SafeFindElement(rect_df, territory_name);
    if (existing_elem)
        LogError("territorio", "TM_CreateNewTerritory: Território com este nome já existe");
        return 0;
    endif
    
    // Ordenar coordenadas para garantir que x1 < x2 e y1 < y2
    if (x1 > x2)
        var temp := x1;
        x1 := x2;
        x2 := temp;
    endif
    
    if (y1 > y2)
        var temp := y1;
        y1 := y2;
        y2 := temp;
    endif
    
    // Verificar sobreposição
    if (TM_CheckRectangleOverlap(rect_df, x1, y1, x2, y2, realm))
        LogError("territorio", "TM_CreateNewTerritory: Este território se sobrepõe a outro existente");
        return 0;
    endif
    
    // Criar o elemento para o território
    var elem := rect_df.CreateElement(territory_name);
    if (!elem)
        LogError("territorio", "TM_CreateNewTerritory: Falha ao criar elemento do território");
        return 0;
    endif
    
    // Gerar ID único para o território
    var territory_id := "TER_" + polcore().systime + "_" + RandomInt(10000);
    
    // Definir as propriedades básicas
    var rect_str := x1 + " " + y1 + " " + x2 + " " + y2;
    SetElementProp(elem, "Rect", rect_str);
    SetElementProp(elem, "Realm", realm);
    SetElementProp(elem, "Name", territory_name);
    SetElementProp(elem, TERRITORY_ID_PROP, territory_id);
    
    // Definir a facção controladora (padrão: Selvagem)
    if (!faction_name)
        faction_name := DEFAULT_FACTION;
    endif
    
    SetElementProp(elem, TERRITORY_FACTION_PROP, faction_name);
    SetElementProp(elem, "Faction", faction_name); // Para compatibilidade
    
    // Configurar recursos padrão
    SetElementProp(elem, "Resources", "");
    SetElementProp(elem, "ResourceInterval", 60); // 60 minutos
    SetElementProp(elem, "ResourceAmount", 5); // 5 unidades por vez
    
    LogInfo("territorio", "Território criado: " + territory_name + " (ID: " + territory_id + ")");
    
    // Notificar sistema de eventos sobre a criação
    TM_NotifyTerritoryChange(territory_id, "created", struct{
        "name" := territory_name,
        "faction" := faction_name
    });
    
    return territory_id;
endfunction

/*
 * TM_DeleteTerritory(territory_id)
 *
 * Purpose: Remove completamente um território e seus objetos
 * Parameters:
 *   territory_id - ID ou nome do território
 * Return:
 *   1 para sucesso, 0 para falha
 */
function TM_DeleteTerritory(territory_id)
    var territory_elem := TM_FindTerritory(territory_id);
    if (!territory_elem)
        LogError("territorio", "TM_DeleteTerritory: Território não encontrado: " + territory_id);
        return 0;
    endif
    
    // Obter informações do território antes de removê-lo
    var territory_info := TM_GetTerritoryInfo(territory_id);
    if (!territory_info)
        LogError("territorio", "TM_DeleteTerritory: Não foi possível obter informações do território");
        return 0;
    endif
    
    var territory_name := territory_info.name;
    var faction := territory_info.faction;
    
    // 1. Remover objetos físicos do território
    TM_DeleteTerritoryObjects(territory_id, faction);
    
    // 2. Remover referências nas guildas
    TM_RemoveTerritoryFromGuilds(territory_id, territory_name);
    
    // 3. Remover entrada no registro de seriais
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
        if (territorios_seriais && territorios_seriais.exists(territory_id))
            territorios_seriais.erase(territory_id);
            SetElementProp(seriais_df, "territorios", territorios_seriais);
        endif
    endif
    
    // 4. Remover o território do datafile
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (rect_df)
        if (DeleteDatafileEntry(":architect:areas", territory_name))
            // Notificar sistema de eventos sobre a remoção
            TM_NotifyTerritoryChange(territory_id, "deleted", struct{
                "name" := territory_name,
                "faction" := faction
            });
            
            LogInfo("territorio", "Território removido: " + territory_name + " (ID: " + territory_id + ")");
            return 1;
        endif
    endif
    
    LogError("territorio", "TM_DeleteTerritory: Falha ao remover território do datafile");
    return 0;
endfunction

/*
 * TM_DeleteTerritoryObjects(territory_id, faction_name := "")
 *
 * Purpose: Remove os objetos físicos de um território
 * Parameters:
 *   territory_id - ID do território
 *   faction_name - Nome da facção (opcional)
 * Return:
 *   Número de objetos removidos
 */
function TM_DeleteTerritoryObjects(territory_id, faction_name := "")
    // Tipos de objetos do território
    var tipos_objetos := array{0xFF11, 0xFE5C, 0x46AE}; // Baú, Mesa, Mural
    var objetos_destruidos := 0;
    
    // Verificar se temos informação de facção
    if (!faction_name)
        var territory_info := TM_GetTerritoryInfo(territory_id);
        if (territory_info)
            faction_name := territory_info.faction;
        endif
    endif
    
    LogDebug("territorio", "TM_DeleteTerritoryObjects: Removendo objetos do território " + territory_id);
    
    // Primeiro, verificar os seriais registrados
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
        if (territorios_seriais && territorios_seriais[territory_id])
            var info := territorios_seriais[territory_id];
            
            // Remover baú
            if (info.bau)
                var bau := SystemFindObjectBySerial(info.bau);
                if (bau)
                    PrintTextAbove(bau, "*Este objeto será removido*");
                    bau.movable := 1;
                    Sleep(1);
                    if (DestroyItem(bau))
                        objetos_destruidos += 1;
                    else
                        // Tentar método alternativo se falhar
                        MoveObjectToLocation(bau, 6000, 6000, 0, "britannia_alt", MOVEOBJECT_FORCELOCATION);
                        objetos_destruidos += 1;
                    endif
                endif
            endif
            
            // Remover mesa
            if (info.mesa)
                var mesa := SystemFindObjectBySerial(info.mesa);
                if (mesa)
                    PrintTextAbove(mesa, "*Este objeto será removido*");
                    mesa.movable := 1;
                    Sleep(1);
                    if (DestroyItem(mesa))
                        objetos_destruidos += 1;
                    else
                        // Tentar método alternativo se falhar
                        MoveObjectToLocation(mesa, 6000, 6000, 0, "britannia_alt", MOVEOBJECT_FORCELOCATION);
                        objetos_destruidos += 1;
                    endif
                endif
            endif
            
            // Remover mural
            if (info.mural)
                var mural := SystemFindObjectBySerial(info.mural);
                if (mural)
                    PrintTextAbove(mural, "*Este objeto será removido*");
                    mural.movable := 1;
                    Sleep(1);
                    if (DestroyItem(mural))
                        objetos_destruidos += 1;
                    else
                        // Tentar método alternativo se falhar
                        MoveObjectToLocation(mural, 6000, 6000, 0, "britannia_alt", MOVEOBJECT_FORCELOCATION);
                        objetos_destruidos += 1;
                    endif
                endif
            endif
            
            // Remover entrada do datafile
            territorios_seriais.erase(territory_id);
            SetElementProp(seriais_df, "territorios", territorios_seriais);
        endif
    endif
    
    // Se não encontrou pelos seriais, ou não conseguiu remover todos,
    // buscar todos os objetos no mundo que possam pertencer a este território
    if (objetos_destruidos < 3)
        foreach item in EnumerateItemsInContainer(0) // 0 = raiz do mundo
            if (item.objtype in tipos_objetos)
                var item_territory_id := GetObjProperty(item, TERRITORY_ID_PROP);
                var item_faction := GetObjProperty(item, OBJ_FACTION_PROP);
                var item_name := item.name;
                
                // Verificar por ID, facção ou nome
                if ((item_territory_id && item_territory_id == territory_id) || 
                    (faction_name && item_faction == faction_name && item_name.find(territory_id) != error) ||
                    (item_name && (item_name.find(territory_id) != error)))
                    
                    PrintTextAbove(item, "*Este objeto será removido*");
                    item.movable := 1;
                    Sleep(1);
                    
                    if (DestroyItem(item))
                        objetos_destruidos += 1;
                    else
                        // Tentar método alternativo se falhar
                        MoveObjectToLocation(item, 6000, 6000, 0, "britannia_alt", MOVEOBJECT_FORCELOCATION);
                        objetos_destruidos += 1;
                    endif
                endif
            endif
            sleepms(1);
        endforeach
    endif
    
    LogInfo("territorio", "TM_DeleteTerritoryObjects: " + objetos_destruidos + " objetos removidos");
    return objetos_destruidos;
endfunction

/*
 * TM_RemoveTerritoryFromGuilds(territory_id, territory_name := "")
 *
 * Purpose: Remove um território das listas de territórios de todas as guildas
 * Parameters:
 *   territory_id - ID do território
 *   territory_name - Nome do território (opcional)
 * Return:
 *   Número de guildas atualizadas
 */
function TM_RemoveTerritoryFromGuilds(territory_id, territory_name := "")
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
        
        var changed := 0;
        var new_territories := array{};
        
        foreach terr in territories
            // Verificar diferentes formatos de comparação (nome, ID, estrutura)
            var skip := 0;
            
            if (typeof(terr) == "String")
                if (terr == territory_id || (territory_name && terr == territory_name))
                    skip := 1;
                endif
            elseif (typeof(terr) == "Struct")
                if ((terr.id && terr.id == territory_id) || 
                    (territory_name && terr.name && terr.name == territory_name))
                    skip := 1;
                endif
            endif
            
            if (!skip)
                new_territories.append(terr);
            else
                changed := 1;
            endif
        endforeach
        
        if (changed)
            guild.SetProp("territories", new_territories);
            guilds_updated += 1;
        endif
        
        sleepms(2);
    endforeach
    
    return guilds_updated;
endfunction

/*
 * TM_TransferTerritoryFaction(territory_id, new_faction, who := 0)
 *
 * Purpose: Transfere um território de uma facção para outra
 * Parameters:
 *   territory_id - ID do território
 *   new_faction - Nome da nova facção
 *   who - Referência ao jogador para envio de mensagens (opcional)
 * Return:
 *   1 para sucesso, 0 para falha
 */
function TM_TransferTerritoryFaction(territory_id, new_faction, who := 0)
    // Verificar existência do território
    var territory_info := TM_GetTerritoryInfo(territory_id);
    if (!territory_info)
        if (who)
            SendSysMessage(who, "Território não encontrado");
        endif
        LogError("territorio", "TM_TransferTerritoryFaction: Território não encontrado: " + territory_id);
        return 0;
    endif
    
    var territory_name := territory_info.name;
    var old_faction := territory_info.faction;
    
    // Verificar se a facção mudou
    if (old_faction == new_faction)
        if (who)
            SendSysMessage(who, "O território já pertence à facção " + new_faction);
        endif
        LogWarning("territorio", "TM_TransferTerritoryFaction: Território já pertence à facção: " + new_faction);
        return 0;
    endif
    
    // Iniciar uma transação para garantir atomicidade
    var transaction_data := struct{
        "territorio_id" := territory_id,
        "territorio_nome" := territory_name,
        "faccao_original" := old_faction,
        "faccao_nova" := new_faction
    };
    
    // Buscar informações sobre seriais existentes
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
        if (territorios_seriais && territorios_seriais[territory_id])
            transaction_data.seriais_originais := territorios_seriais[territory_id];
        endif
    endif
    
    var transaction_id := IniciarTransacao("transferir_territorio", transaction_data);
    if (transaction_id == error)
        if (who)
            SendSysMessage(who, "Erro ao iniciar transação. Transferência cancelada.");
        endif
        LogError("territorio", "TM_TransferTerritoryFaction: Falha ao iniciar transação");
        return 0;
    endif
    
    if (who)
        SendSysMessage(who, "Processando: Iniciando transferência de território...");
    endif
    
    // 1. Remover objetos existentes
    TM_DeleteTerritoryObjects(territory_id, old_faction);
    
    // 2. Atualizar o datafile
    if (!TM_SetTerritoryFaction(territory_id, new_faction))
        DesfazerTransacao(transaction_id);
        if (who)
            SendSysMessage(who, "Erro ao definir nova facção. Transferência cancelada.");
        endif
        LogError("territorio", "TM_TransferTerritoryFaction: Falha ao atualizar facção no datafile");
        return 0;
    endif
    
    // 3. Atualizar as guildas
    
    // 3.1 Remover da facção antiga
    var old_guild := FindGuildByName(old_faction);
    if (old_guild)
        var old_guild_obj := FindGuild(old_guild);
        if (old_guild_obj)
            var old_territories := old_guild_obj.GetProp("territories");
            if (old_territories)
                var new_old_territories := array{};
                foreach terr in old_territories
                    if (terr != territory_id && terr != territory_name)
                        new_old_territories.append(terr);
                    endif
                    sleepms(2);
                endforeach
                
                old_guild_obj.SetProp("territories", new_old_territories);
            endif
        endif
    endif
    
    // 3.2 Adicionar à nova facção
    var new_guild := FindGuildByName(new_faction);
    if (new_guild)
        var new_guild_obj := FindGuild(new_guild);
        if (new_guild_obj)
            var new_territories := new_guild_obj.GetProp("territories");
            if (!new_territories)
                new_territories := array{};
            endif
            
            // Verificar se já existe
            var exists := 0;
            foreach terr in new_territories
                if (terr == territory_id || terr == territory_name)
                    exists := 1;
                    break;
                endif
                sleepms(2);
            endforeach
            
            if (!exists)
                new_territories.append(territory_id);
            endif
            
            new_guild_obj.SetProp("territories", new_territories);
        endif
    endif
    
    // 4. Criar novos objetos para a nova facção
    if (who)
        SendSysMessage(who, "Processando: Criando novos objetos para a facção...");
    endif
    
    var center := TM_GetTerritoryCenter(territory_id);
    if (!center)
        DesfazerTransacao(transaction_id);
        if (who)
            SendSysMessage(who, "Erro ao obter centro do território. Transferência cancelada.");
        endif
        LogError("territorio", "TM_TransferTerritoryFaction: Falha ao obter centro do território");
        return 0;
    endif
    
    // 4.1 Criar o baú
    var chest := CreateItemAtLocation(center.x, center.y, center.z, 0xFF11, 1, center.realm);
    if (!chest)
        DesfazerTransacao(transaction_id);
        if (who)
            SendSysMessage(who, "Erro ao criar baú. Transferência revertida.");
        endif
        LogError("territorio", "TM_TransferTerritoryFaction: Falha ao criar baú");
        return 0;
    endif
    
    chest.movable := 0;
    SetObjProperty(chest, OBJ_FACTION_PROP, new_faction);
    SetObjProperty(chest, "FactionName", new_faction);
    SetObjProperty(chest, TERRITORY_ID_PROP, territory_id);
    
    var chest_id := new_faction + "_" + territory_id;
    SetObjProperty(chest, CHEST_ID_PROP, chest_id);
    
    var nome_bau := "Baú de Doações de " + new_faction;
    if (territory_id)
        nome_bau += " - " + territory_id;
    endif
    SetName(chest, nome_bau);
    
    chest.usescript := ":faccao:baudoacao/use";
    SetObjProperty(chest, "OnInsertScript", ":faccao:baudoacao/onInsert");
    SetObjProperty(chest, "OnRemoveScript", ":faccao:baudoacao/onRemove");
    SetObjProperty(chest, "CanInsertScript", ":faccao:baudoacao/canInsert");
    SetObjProperty(chest, "CanRemoveScript", ":faccao:baudoacao/canRemoveDonation");
    
    // 4.2 Criar a mesa de gestão
    var table := CreateItemAtLocation(center.x+1, center.y, center.z, 0xFE5C, 1, center.realm);
    if (!table)
        DesfazerTransacao(transaction_id);
        DestroyItem(chest); // Limpar o baú criado
        if (who)
            SendSysMessage(who, "Erro ao criar mesa de gestão. Transferência revertida.");
        endif
        LogError("territorio", "TM_TransferTerritoryFaction: Falha ao criar mesa");
        return 0;
    endif
    
    table.movable := 0;
    SetObjProperty(table, OBJ_FACTION_PROP, new_faction);
    SetObjProperty(table, "FactionName", new_faction);
    SetObjProperty(table, TERRITORY_ID_PROP, territory_id);
    
    var nome_mesa := "Mesa de Gestão de " + new_faction;
    if (territory_id)
        nome_mesa += " - " + territory_id;
    endif
    SetName(table, nome_mesa);
    
    table.usescript := ":faccao:gestao_use";
    
    // 4.3 Criar o mural de recrutamento
    var mural := CreateItemAtLocation(center.x, center.y+1, center.z, 0x46AE, 1, center.realm);
    if (!mural)
        DesfazerTransacao(transaction_id);
        DestroyItem(chest);
        DestroyItem(table);
        if (who)
            SendSysMessage(who, "Erro ao criar mural. Transferência revertida.");
        endif
        LogError("territorio", "TM_TransferTerritoryFaction: Falha ao criar mural");
        return 0;
    endif
    
    mural.movable := 0;
    SetObjProperty(mural, OBJ_FACTION_PROP, new_faction);
    SetObjProperty(mural, "FactionName", new_faction);
    SetObjProperty(mural, TERRITORY_ID_PROP, territory_id);
    
    var nome_mural := "Mural de Recrutamento de " + new_faction;
    if (territory_id)
        nome_mural += " - " + territory_id;
    endif
    SetName(mural, nome_mural);
    
    mural.usescript := ":faccao:faccao";
    
    // 4.4 Salvar os seriais no datafile
    if (seriais_df)
        var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
        if (!territorios_seriais)
            territorios_seriais := dictionary{};
        endif
        
        territorios_seriais[territory_id] := struct{
            "bau" := chest ? chest.serial : 0,
            "mesa" := table ? table.serial : 0,
            "mural" := mural ? mural.serial : 0,
            "faction" := new_faction
        };
        
        SetElementProp(seriais_df, "territorios", territorios_seriais);
    endif
    
    // 5. Notificar o controlador de recursos
    TM_NotifyTerritoryChange(territory_id, "transfer", struct{
        "old_faction" := old_faction,
        "new_faction" := new_faction
    });
    
    // Finalizar a transação com sucesso
    FinalizarTransacao(transaction_id);
    
    if (who)
        SendSysMessage(who, "Território transferido com sucesso para " + new_faction + "!");
    endif
    
    LogInfo("territorio", "Transferência concluída. Território " + territory_id + " transferido de " + 
                           old_faction + " para " + new_faction);
    
    return 1;
endfunction

/*
 * TM_NotifyTerritoryChange(territory_id, change_type, change_data := 0)
 *
 * Purpose: Notifica sobre mudanças em um território
 * Parameters:
 *   territory_id - ID do território
 *   change_type - Tipo de mudança ("created", "deleted", "transfer", etc.)
 *   change_data - Dados adicionais sobre a mudança (opcional)
 * Return:
 *   1 para sucesso, 0 para falha
 */
function TM_NotifyTerritoryChange(territory_id, change_type, change_data := 0)
    // 1. Registrar no datafile de mensagens
    var df := SafeOpenDataFile(":system:messages", 1);
    if (df)
        var elem := SafeFindElement(df, "territory_changes", 1);
        if (elem)
            var changes := GetElementProp(elem, "changes", array{});
            
            // Criar entrada com timestamp
            var change_entry := struct{
                "territory_id" := territory_id,
                "type" := change_type,
                "data" := change_data,
                "timestamp" := polcore().systime
            };
            
            // Adicionar no início do array para processamento mais rápido
            changes.insert(1, change_entry);
            
            // Limitar tamanho do array (manter apenas os 20 mais recentes)
            while (changes.size() > 20)
                changes.erase(changes.size());
            endwhile
            
            SetElementProp(elem, "changes", changes);
        endif
    endif
    
    // 2. Notificar diretamente o controlador de recursos via events
    var pid := GetGlobalProperty("ResourceControllerPID");
    if (pid)
        var process := GetProcess(pid);
        if (process)
            var ev := struct{
                "type" := "TerritoryChange",
                "territory_id" := territory_id,
                "change_type" := change_type,
                "change_data" := change_data,
                "timestamp" := polcore().systime
            };
            
            process.SendEvent(ev);
        endif
    endif
    
    // 3. Registrar no log
    LogInfo("territorio", "Território " + territory_id + " - " + change_type + 
                           " - Dados: " + change_data);
    
    return 1;
endfunction

/*
 * TM_GetAllTerritories(faction_name := "")
 *
 * Purpose: Obtém todos os territórios, opcionalmente filtrando por facção
 * Parameters:
 *   faction_name - Nome da facção para filtrar (opcional)
 * Return:
 *   Array de território_ids ou array vazio
 */
function TM_GetAllTerritories(faction_name := "")
    var territories := array{};
    var rect_df := SafeOpenDataFile(":architect:areas");
    
    if (!rect_df)
        LogError("territorio", "TM_GetAllTerritories: Falha ao abrir datafile de áreas");
        return territories;
    endif
    
    foreach elem_name in (rect_df.keys())
        var elem := SafeFindElement(rect_df, elem_name);
        if (!elem)
            continue;
        endif
        
        // Obter o ID do território
        var territory_id := GetElementProp(elem, TERRITORY_ID_PROP, elem_name);
        
        // Se um filtro de facção foi especificado
        if (faction_name)
            var territory_faction := GetElementProp(elem, TERRITORY_FACTION_PROP);
            if (!territory_faction)
                territory_faction := GetElementProp(elem, "Faction"); // Compatibilidade
            endif
            
            if (territory_faction != faction_name)
                continue; // Pular territórios que não pertencem à facção
            endif
        endif
        
        territories.append(territory_id);
        sleepms(2);
    endforeach
    
    return territories;
endfunction

// Função utilitária para limpar espaços em strings
function TrimString(str)
    if (!str)
        return "";
    endif
    
    // Remover espaços no início
    while (str.length() > 0 && str[1] == " ")
        str := str[2, str.length()];
    endwhile
    
    // Remover espaços no fim
    while (str.length() > 0 && str[str.length()] == " ")
        str := str[1, str.length()-1];
    endwhile
    
    return str;
endfunction
    