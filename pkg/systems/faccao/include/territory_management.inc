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
 * SetTerritoryAsCapital(territory_id, faction_name)
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