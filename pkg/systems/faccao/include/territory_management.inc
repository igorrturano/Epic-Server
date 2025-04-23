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
 *          (VERSÃO CORRIGIDA: Usa elemento por território)
 * Parameters:
 *   territory_id - ID do território
 *   object_serials - Struct com seriais dos objetos (bau, mesa, mural, etc)
 * Return:
 *   1 para sucesso, 0 para falha
 */
function TM_RegisterTerritoryObjects(territory_id, object_serials)
    LogDebug("territorio", "------------------------------------------");
    LogDebug("territorio", "INÍCIO TM_RegisterTerritoryObjects (v2 - elem por terr)");
    LogDebug("territorio", "territory_id: " + territory_id);
    LogDebug("territorio", "object_serials: " + (object_serials ? "Presente" : "NULL"));
    if (object_serials)
        LogDebug("territorio", "Tipo: " + TypeOf(object_serials));
        LogDebug("territorio", "Chaves: " + object_serials.keys());
        foreach key in (object_serials.keys())
            LogDebug("territorio", "  " + key + " = " + object_serials[key]);
        endforeach
    endif
    
    // Validação rigorosa de entrada
    if (!territory_id)
        LogError("territorio", "TM_RegisterTerritoryObjects: ID do território é obrigatório");
        return 0;
    endif
    
    if (!object_serials || TypeOf(object_serials) != "Struct")
        LogError("territorio", "TM_RegisterTerritoryObjects: object_serials deve ser um struct válido");
        return 0;
    endif
    
    // Abrir datafile com tratamento de erro
    LogDebug("territorio", "TM_RegisterTerritoryObjects: Abrindo datafile :faccao:seriais");
    var seriais_df := SafeOpenDataFile(":faccao:seriais", 1);
    LogDebug("territorio", "Resultado da abertura do datafile: " + (seriais_df ? "Sucesso" : "FALHA") + 
             ", Tipo: " + TypeOf(seriais_df));
    
    if (!seriais_df)
        LogError("territorio", "TM_RegisterTerritoryObjects: Falha ao acessar datafile de seriais");
        return 0;
    endif
    
    // Criar/encontrar um ELEMENTO específico para este território_id
    LogDebug("territorio", "TM_RegisterTerritoryObjects: Buscando/Criando elemento para: " + territory_id);
    var territory_elem := SafeFindElement(seriais_df, territory_id, 1); // 1 = Criar se não existir
    
    if (!territory_elem)
        LogError("territorio", "TM_RegisterTerritoryObjects: Falha ao criar/encontrar elemento para " + territory_id);
        return 0;
    endif
    LogDebug("territorio", "Elemento do território encontrado/criado com sucesso.");
    
    // Definir as propriedades DIRETAMENTE no elemento do território
    var props_set_count := 0;
    var failed_props := 0;
    foreach key in (object_serials.keys())
        // Verificar se o valor não é nulo/zero
        if (object_serials[key]) // Salvar apenas se houver valor
            LogDebug("territorio", "TM_RegisterTerritoryObjects: Definindo Propriedade '" + key + "' = " + object_serials[key]);
            // Usar SetElementProp para salvar cada propriedade individualmente
            if (SetElementProp(territory_elem, key, object_serials[key]))
                props_set_count += 1;
            else
                LogError("territorio", "Falha ao definir propriedade '" + key + "' para território " + territory_id);
                failed_props += 1;
            endif
        endif
        sleepms(1);
    endforeach
    
    // Verificar se houve falhas
    if (failed_props > 0)
        LogError("territorio", "TM_RegisterTerritoryObjects: " + failed_props + " propriedades falharam ao ser salvas para " + territory_id);
        LogDebug("territorio", "FIM TM_RegisterTerritoryObjects: FALHA PARCIAL");
        LogDebug("territorio", "------------------------------------------");
        return 0; // Ou retornar sucesso parcial? Depende da necessidade. Retornar 0 é mais seguro.
    else
        LogInfo("territorio", "Objetos registrados para território " + territory_id + ": " + 
                props_set_count + " propriedades salvas.");
        LogDebug("territorio", "FIM TM_RegisterTerritoryObjects: SUCESSO");
        LogDebug("territorio", "------------------------------------------");
        
        // Backup adicional como medida de segurança
        BackupTerritoryData(territory_id, object_serials);
        return 1;
    endif
endfunction


function BackupTerritoryData(territory_id, object_serials)
    // Create backup using datafile instead of raw file
    var backup_df := SafeOpenDataFile(":faccao:backup_seriais", DF_CREATE);
    if (backup_df)
        var timestamp := polcore().systime;
        var backup_elem := SafeFindElement(backup_df, "backup_" + timestamp + "_" + territory_id, DF_CREATE);
        
        if (backup_elem)
            SetElementProp(backup_elem, "territory_id", territory_id);
            SetElementProp(backup_elem, "timestamp", timestamp);
            
            foreach key in (object_serials.keys())
                SetElementProp(backup_elem, key, object_serials[key]);
            endforeach
            
            return 1;
        endif
    endif
    return 0;
endfunction

/*
 * ArrayToString(arr) - Função auxiliar para logging
 * Converte um array para uma string formatada para logs
 */
function ArrayToString(arr)
    if (!arr)
        return "NULL";
    endif
    
    if (TypeOf(arr) != "Array")
        return "Não é um array: " + TypeOf(arr);
    endif
    
    if (arr.size() == 0)
        return "[]";
    endif
    
    var result := "[";
    for i := 1 to arr.size()
        if (i > 1)
            result += ", ";
        endif
        result += CStr(arr[i]);
    endfor
    result += "]";
    
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
    
    // Estratégia 1: Buscar pelo elemento específico no datafile
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territory_elem := SafeFindElement(seriais_df, territory_id);
        if (territory_elem)
            var elem_faction := GetElementProp(territory_elem, "faction");
            
            // Verificar se a facção corresponde, se especificada
            if (!faction_name || !elem_faction || elem_faction == faction_name)
                // Adicionar os objetos encontrados conforme o tipo solicitado
                if (!item_type || item_type == ITEM_TYPE_BAU)
                    var serial_bau := GetElementProp(territory_elem, "bau");
                    if (serial_bau)
                        result.bau := SystemFindObjectBySerial(serial_bau);
                    endif
                endif
                
                if (!item_type || item_type == ITEM_TYPE_MESA)
                    var serial_mesa := GetElementProp(territory_elem, "mesa");
                    if (serial_mesa)
                        result.mesa := SystemFindObjectBySerial(serial_mesa);
                    endif
                endif
                
                if (!item_type || item_type == ITEM_TYPE_MURAL)
                    var serial_mural := GetElementProp(territory_elem, "mural");
                    if (serial_mural)
                        result.mural := SystemFindObjectBySerial(serial_mural);
                    endif
                endif
                
                // Se encontrou pelo menos um objeto, retornar
                if (result.bau || result.mesa || result.mural)
                    return result;
                endif
            else
                LogDebug("territorio", "Facção não corresponde: " + elem_faction + " vs " + faction_name);
            endif
        else
            // LEGADO: Verificar no dicionário antigo (para compatibilidade durante a transição)
            var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
            if (territorios_seriais && territorios_seriais[territory_id])
                var info := territorios_seriais[territory_id];
                
                // Verificar se a facção corresponde, se especificada
                if (!faction_name || !info.faction || info.faction == faction_name)
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
                    
                    // Se encontrou pelo menos um objeto, retornar
                    if (result.bau || result.mesa || result.mural)
                        // Migrar para o novo formato enquanto estamos aqui
                        MigrarParaNovoFormato(territory_id, info, seriais_df);
                        return result;
                    endif
                endif
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
    
    // Se encontrou objetos no mundo, atualizar o registro
    if (result.bau || result.mesa || result.mural)
        // Cria um struct com os objetos encontrados
        var novos_seriais := struct{};
        if (result.bau) 
            novos_seriais.bau := result.bau.serial; 
        endif
        if (result.mesa) 
            novos_seriais.mesa := result.mesa.serial; 
        endif
        if (result.mural) 
            novos_seriais.mural := result.mural.serial; 
        endif
        
        // Adicionar a facção
        if (faction_name) 
            novos_seriais.faction := faction_name; 
        endif
        
        // Registrar no datafile
        if (novos_seriais.keys().size() > 0)
            TM_RegisterTerritoryObjects(territory_id, novos_seriais);
        endif
    endif
    
    return result;
endfunction

// Função auxiliar para migrar do formato antigo para o novo
function MigrarParaNovoFormato(territory_id, info, seriais_df)
    if (!territory_id || !info || !seriais_df)
        return 0;
    endif
    
    LogInfo("territorio", "Migrando território " + territory_id + " para o novo formato de armazenamento");
    
    // Criar elemento individual para o território
    var territory_elem := SafeFindElement(seriais_df, territory_id, 1); // 1 = criar
if (!territory_elem)
    LogError("territorio", "Falha ao criar elemento para registro de seriais do território: " + territory_id);
endif
    
    // Definir propriedades no elemento
    var migration_success := 1;
    foreach key in (info.keys())
        if (info[key]) // Copiar apenas valores não nulos
            if (!SetElementProp(territory_elem, key, info[key]))
                LogError("territorio", "Falha ao definir " + key + " durante migração do território " + territory_id);
                migration_success := 0;
            endif
        endif
    endforeach
    
    // Se a migração foi bem-sucedida, tentar remover a entrada antiga
    if (migration_success)
        var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
        if (territorios_seriais && territorios_seriais.exists(territory_id))
            territorios_seriais.erase(territory_id);
            var set_result := SetElementProp(seriais_df, "territorios", territorios_seriais);
            
            if (set_result)
                LogInfo("territorio", "Território " + territory_id + " migrado com sucesso para o novo formato");
            else
                LogWarning("territorio", "Território " + territory_id + " migrado, mas falha ao remover entrada antiga");
            endif
        endif
    endif
    
    return migration_success;
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

function DisplayRectangleDetailsComplete(who, elem)
    var width := 350;
    var height := 300;
    
    // ---- OBTER DADOS BÁSICOS COM VALIDAÇÃO RIGOROSA ----
    if (!elem)
        SendSysMessage(who, "Erro: Elemento inválido.");
        return;
    endif
    
    // Obter nome com segurança
    var name := "";
    name := GetElementProp(elem, "Name", "");
    if (name == error || !name || name == "")
        name := "Sem Nome";
    endif
    
    // Obter informações básicas com segurança máxima
    var rect_str := "";
    var realm := "britannia";
    var faction := DEFAULT_FACTION;
    var territory_id := "";
    
    rect_str := GetElementProp(elem, "Rect", "");
    if (rect_str == error) 
        rect_str := "";
    endif
    
    realm := GetElementProp(elem, "Realm", "britannia");
    if (realm == error) 
        realm := "britannia";
    endif
    
    faction := GetElementProp(elem, "Faction", DEFAULT_FACTION);
    if (faction == error) 
        faction := DEFAULT_FACTION;
    endif
    
    if (!faction)
        faction := GetElementProp(elem, TERRITORY_FACTION_PROP, DEFAULT_FACTION);
        if (faction == error) 
            faction := DEFAULT_FACTION;
        endif
    endif
    
    territory_id := GetElementProp(elem, TERRITORY_ID_PROP, "");
    if (territory_id == error) 
        territory_id := "";
    endif
    
    // ---- CRIAR O GUMP SUPER SIMPLES ----
    var gump := GFCreateGump(50, 50, width, height);
    GFClosable(gump, 1);
    GFResizePic(gump, 0, 0, 9200, width, height);
    
    // ---- ADICIONAR INFORMAÇÕES BÁSICAS ----
    var y := 20;
    
    // Título
    GFTextLine(gump, width/2-80, y, 1153, "Território: " + name);
    y += 30;
    
    // Coordenadas (apenas se existirem)
    if (rect_str && rect_str != "")
        var coords := SplitWords(rect_str);
        if (coords && TypeOf(coords) == "Array" && coords.size() >= 4)
            GFTextLine(gump, 20, y, 2100, "Coordenadas:");
            GFTextLine(gump, 120, y, 63, "(" + coords[1] + "," + coords[2] + ") a (" + coords[3] + "," + coords[4] + ")");
        else
            GFTextLine(gump, 20, y, 2100, "Coordenadas:");
            GFTextLine(gump, 120, y, 63, CStr(rect_str));
        endif
        y += 25;
    endif
    
    // Realm
    GFTextLine(gump, 20, y, 2100, "Realm:");
    GFTextLine(gump, 120, y, 63, realm);
    y += 25;
    
    // Facção
    GFTextLine(gump, 20, y, 2100, "Controlado por:");
    var control_color := (faction == DEFAULT_FACTION) ? 38 : 63; // Vermelho ou verde
    GFTextLine(gump, 120, y, control_color, faction);
    y += 40;
    
    // ---- BOTÕES DE AÇÃO ----
    y := height - 40; // Posiciona na parte inferior
    
    // Botão Ir para Território
    GFAddButton(gump, 20, y, 2361, 2361, GF_CLOSE_BTN, 300);
    GFTextLine(gump, 45, y, 63, "Ir Para");
    
    // Botões apenas para GMs
    if (who.cmdlevel >= 1)
        // Botão Configurar Recursos
        GFAddButton(gump, width/2-50, y, 2361, 2361, GF_CLOSE_BTN, 500);
        GFTextLine(gump, width/2-25, y, 63, "Recursos");
        
        // Botão Deletar
        GFAddButton(gump, width-80, y, 2361, 2361, GF_CLOSE_BTN, 100);
        GFTextLine(gump, width-55, y, 38, "Deletar");
    endif
    
    // ---- PROCESSAR RESPOSTA ----
    var input := GFSendGump(who, gump);
    var button := input[0];
    
    if (button == 100 && who.cmdlevel >= 1) // Deletar
        if (YesNo(who, "Tem certeza que deseja deletar o território '" + name + "'?"))
            // Tentar deletar pelo nome diretamente - mais seguro
            var df := SafeOpenDataFile(":architect:areas");
            if (df && df.DeleteElement(name))
                SendSysMessage(who, "Território deletado pelo nome!");
                return;
            endif
            
            // Se não conseguiu, tentar pelo ID
            if (territory_id && TM_DeleteTerritory(territory_id))
                SendSysMessage(who, "Território deletado com sucesso!");
            else
                SendSysMessage(who, "Falha ao deletar território.");
            endif
        endif
    elseif (button == 300) // Ir para território
        // Calcular centro manualmente
        if (rect_str)
            var coords := SplitWords(rect_str);
            if (coords && TypeOf(coords) == "Array" && coords.size() >= 4)
                var x1 := CInt(coords[1]);
                var y1 := CInt(coords[2]);
                var x2 := CInt(coords[3]);
                var y2 := CInt(coords[4]);
                
                var center_x := CInt((x1 + x2) / 2);
                var center_y := CInt((y1 + y2) / 2);
                var center_z := GetWorldHeight(center_x, center_y, realm);
                
                MoveObjectToLocation(who, center_x, center_y, center_z, realm, MOVEOBJECT_FORCELOCATION);
                SendSysMessage(who, "Teleportado para o centro do território!");
            else
                SendSysMessage(who, "Coordenadas inválidas!");
            endif
        else
            SendSysMessage(who, "Coordenadas não encontradas para este território!");
        endif
    elseif (button == 500 && who.cmdlevel >= 1) // Configurar Recursos
        SetupResourcesForTerritory(who, elem);
    endif
endfunction

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
    
    // 3. Remover entrada no registro de seriais - NOVA VERSÃO
    // Remover diretamente o elemento do território
    DeleteDatafileEntry(":faccao:seriais", territory_id);
    
    // LEGADO: Verificar também no formato antigo 
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
        if (territorios_seriais && territorios_seriais.exists(territory_id))
            territorios_seriais.erase(territory_id);
            SetElementProp(seriais_df, "territorios", territorios_seriais);
        endif
    endif
    
    // 4. Remover o território do datafile - IMPORTANTE: usar o nome como chave
    if (DeleteDatafileEntry(":architect:areas", territory_name))
        // Notificar sistema de eventos sobre a remoção
        TM_NotifyTerritoryChange(territory_id, "deleted", struct{
            "name" := territory_name,
            "faction" := faction
        });
        
        LogInfo("territorio", "Território removido: " + territory_name + " (ID: " + territory_id + ")");
        return 1;
    endif
    
    // Segunda tentativa: tentar excluir pelo ID
    if (DeleteDatafileEntry(":architect:areas", territory_id))
        TM_NotifyTerritoryChange(territory_id, "deleted", struct{
            "name" := territory_name,
            "faction" := faction
        });
        
        LogInfo("territorio", "Território removido por ID: " + territory_id);
        return 1;
    endif
    
    LogError("territorio", "TM_DeleteTerritory: Falha ao remover território do datafile");
    return 0;
endfunction

/*
 * TM_DeleteTerritoryObjects(territory_id, faction_name)
 *
 * Purpose: Remove objetos associados a um território
 * Parameters:
 *   territory_id - ID do território
 *   faction_name - Nome da facção
 * Return:
 *   Número de objetos removidos
 */
function TM_DeleteTerritoryObjects(territory_id, faction_name)
    LogInfo("territorio", "Removendo objetos do território " + territory_id + " da facção " + faction_name);
    
    var objetos_removidos := 0;
    
    // Encontrar objetos associados ao território
    var objetos := TM_FindTerritoryObjects(territory_id, faction_name);
    
    // Remover baú
    if (objetos.bau)
        var result := DestroyItem(objetos.bau);
        if (result)
            objetos_removidos += 1;
            LogDebug("territorio", "Baú removido: " + objetos.bau.serial);
        else
            LogError("territorio", "Falha ao remover baú: " + objetos.bau.serial + ", Resultado: " + result);
        endif
    endif
    
    // Remover mesa
    if (objetos.mesa)
        var result := DestroyItem(objetos.mesa);
        if (result)
            objetos_removidos += 1;
            LogDebug("territorio", "Mesa removida: " + objetos.mesa.serial);
        else
            LogError("territorio", "Falha ao remover mesa: " + objetos.mesa.serial + ", Resultado: " + result);
        endif
    endif
    
    // Remover mural
    if (objetos.mural)
        var result := DestroyItem(objetos.mural);
        if (result)
            objetos_removidos += 1;
            LogDebug("territorio", "Mural removido: " + objetos.mural.serial);
        else
            LogError("territorio", "Falha ao remover mural: " + objetos.mural.serial + ", Resultado: " + result);
        endif
    endif
    
    // Remover registros de seriais no datafile
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        // MÉTODO 1: Remover do elemento específico
        var territory_elem := SafeFindElement(seriais_df, territory_id);
        if (territory_elem)
            // Salvar informações de fação para logs
            var old_faction := GetElementProp(territory_elem, "faction");
            
            // Limpar propriedades
territory_elem.EraseProp("bau");
territory_elem.EraseProp("mesa");
territory_elem.EraseProp("mural");
territory_elem.EraseProp("faction");
            
            LogInfo("territorio", "Registros de seriais limpos para território " + territory_id + 
                              " (antiga facção: " + old_faction + ")");
        endif
        
        // MÉTODO 2: Compatibilidade com formato antigo
        var territorios_seriais := GetElementProp(seriais_df, "territorios", dictionary{});
        if (territorios_seriais && territorios_seriais.exists(territory_id))
            territorios_seriais.erase(territory_id);
            SetElementProp(seriais_df, "territorios", territorios_seriais);
            LogInfo("territorio", "Registros legados de seriais removidos para território " + territory_id);
        endif
    endif
    
    LogInfo("territorio", "Remoção de objetos concluída. Objetos removidos: " + objetos_removidos);
    return objetos_removidos;
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
        // NOVA LÓGICA: Buscar pelo elemento específico do território
        var territory_elem := SafeFindElement(seriais_df, territory_id);
        if (territory_elem)
            // Capturar os seriais originais
            transaction_data.seriais_originais := struct{
                "bau" := GetElementProp(territory_elem, "bau"),
                "mesa" := GetElementProp(territory_elem, "mesa"),
                "mural" := GetElementProp(territory_elem, "mural"),
                "faction" := GetElementProp(territory_elem, "faction")
            };
        endif
        
        // LEGADO: Verificar no formato antigo (dicionário)
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
    LogDebug("territorio", "3.2: Adicionando à nova facção: " + new_faction);
    var new_guild := FindGuildByName(new_faction);
    LogDebug("territorio", "Resultado FindGuildByName para nova facção: " + new_guild);

    if (new_guild)
        var new_guild_obj := FindGuild(new_guild);
        LogDebug("territorio", "Resultado FindGuild para nova facção: " + (new_guild_obj ? "OK" : "FALHA"));
        LogDebug("territorio", "Tipo do objeto guild: " + TypeOf(new_guild_obj) + ", Serial: " + new_guild_obj);
        
        if (new_guild_obj)
            var new_territories := new_guild_obj.GetProp("territories");
            LogDebug("territorio", "Territories da nova facção: " + (new_territories ? "Encontrados" : "NULL") + 
                     ", Tipo: " + TypeOf(new_territories));
            
            // Aprimoramento da manipulação de array
            if (!new_territories || new_territories == error || TypeOf(new_territories) != "Array")
                LogDebug("territorio", "Criando novo array para territories (devido a tipo inválido)");
                new_territories := array{};
            endif

            // Garantir formato consistente para itens
            var new_territories_clean := array{};
            foreach terr in new_territories
                if (TypeOf(terr) == "String" && terr != "")
                    new_territories_clean.append(terr);
                endif
                sleepms(2);
            endforeach
            new_territories := new_territories_clean;
            
            // Log detalhado ANTES da adição do território
            LogDebug("territorio", "Estado do array 'territories' ANTES: Size=" + new_territories.size() + 
                     ", TypeOf=" + TypeOf(new_territories) + 
                     ", Conteúdo=" + ArrayToString(new_territories));
            
            // Verificar se já existe
            var exists := 0;
            LogDebug("territorio", "Verificando se território já existe na lista");
            
            foreach terr in new_territories
                LogDebug("territorio", "Território existente: " + terr + ", Tipo: " + TypeOf(terr));
                if (terr == territory_id || terr == territory_name)
                    exists := 1;
                    LogDebug("territorio", "Território já existe na lista");
                    break;
                endif
                sleepms(2);
            endforeach
            
            if (!exists)
                LogDebug("territorio", "Adicionando território à lista: " + territory_id);
                new_territories.append(territory_id);
                LogDebug("territorio", "Novo tamanho da lista: " + new_territories.size());
            endif
            
            // Log detalhado IMEDIATAMENTE ANTES do SetProp crítico
            LogDebug("territorio", "CRÍTICO - Prestes a executar SetProp('territories')");
            LogDebug("territorio", "Guild: " + new_guild_obj.guildid + ", Nome: " + new_guild_obj.guildname);
            LogDebug("territorio", "Territories array: TypeOf=" + TypeOf(new_territories) + 
                     ", Size=" + new_territories.size() + 
                     ", Conteúdo=" + ArrayToString(new_territories));
            
            // Implementar retentativas para o SetProp da nova facção
            var max_retries := 3;
            var retry_count := 0;
            var success := 0;

            while (!success && retry_count < max_retries)
                retry_count += 1;
                var set_result := new_guild_obj.SetProp("territories", new_territories);
                
                LogDebug("territorio", "CRÍTICO - SetProp('territories') tentativa #" + retry_count + ", resultado: " + set_result);
                
                if (set_result != error)
                    success := 1;
                    LogDebug("territorio", "SetProp da nova facção bem-sucedido na tentativa #" + retry_count);
                else
                    LogWarning("territorio", "Falha no SetProp da nova facção, tentativa #" + retry_count + ": " + set_result);
                    
                    if (retry_count == max_retries)
                        // Log detalhado para erros na última tentativa
                        LogError("territorio", "FALHA ao atualizar territories na nova facção: " + set_result);
                        LogError("territorio", "Guild ID: " + new_guild_obj.guildid + ", Nome: " + new_guild_obj.guildname);
                        LogError("territorio", "Territories: Tipo=" + TypeOf(new_territories) + ", Size=" + new_territories.size());
                    endif
                    
                    sleep(1); // Esperar um segundo antes de tentar novamente
                endif
            endwhile

            if (!success)
                LogError("territorio", "Todas as tentativas de SetProp da nova facção falharam");
                // Considerar usar o sistema de backup aqui
                BackupTerritoryFactionRelation(territory_id, new_faction);
            endif
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
        // NOVA VERSÃO: Criar um elemento específico para este território
        var territory_elem := SafeFindElement(seriais_df, territory_id, 1); // 1 = criar
        if (territory_elem)
            // Definir as propriedades diretamente no elemento
            SetElementProp(territory_elem, "bau", chest ? chest.serial : 0);
            SetElementProp(territory_elem, "mesa", table ? table.serial : 0);
            SetElementProp(territory_elem, "mural", mural ? mural.serial : 0);
            SetElementProp(territory_elem, "faction", new_faction);
            
            LogInfo("territorio", "Seriais registrados no novo formato para território: " + territory_id);
        endif
        
        // LEGADO: Também manter o formato antigo temporariamente
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
        
        // Tentar salvar, mas não bloquear se falhar
        var result := SetElementProp(seriais_df, "territorios", territorios_seriais);
        if (result != 1)
            LogWarning("territorio", "Aviso: Falha ao salvar no formato antigo, mas o novo formato foi utilizado.");
        endif
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
 * BackupTerritoryFactionRelation(territory_id, faction_name)
 *
 * Purpose: Creates a backup of the territory-faction relationship 
 * Parameters:
 *   territory_id - ID of the territory
 *   faction_name - Name of the faction
 * Return:
 *   1 for success, 0 for failure
 */
function BackupTerritoryFactionRelation(territory_id, faction_name)
    var backup_df := SafeOpenDataFile(":faccao:backups", DF_CREATE);
    if (!backup_df)
        LogError("territorio", "BackupTerritoryFactionRelation: Failed to create backup datafile");
        return 0;
    endif
    
    var elem_name := "territory_faction_" + territory_id + "_" + polcore().systime;
    var backup_elem := SafeFindElement(backup_df, elem_name, DF_CREATE);
    if (!backup_elem)
        LogError("territorio", "BackupTerritoryFactionRelation: Failed to create backup element");
        return 0;
    endif
    
    SetElementProp(backup_elem, "territory_id", territory_id);
    SetElementProp(backup_elem, "faction", faction_name);
    SetElementProp(backup_elem, "timestamp", polcore().systime);
    
    LogInfo("territorio", "Backup created for territory " + territory_id + " with faction " + faction_name);
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
    var df := SafeOpenDataFile(":faccao:messages", 1); // Usar o datafile existente
    if (df)
        // Usar uma chave diferente de "territory_changes" para não conflitar, ex: "controller_events"
        var elem := SafeFindElement(df, "controller_events", 1);
        if (elem)
             var events := GetElementProp(elem, "pending", array{});
             if(TypeOf(events) != "Array") 
                 events := array{}; 
             endif // Garantir que é array

             // Criar entrada do evento
             var event_entry := struct{
                 "territory_id" := territory_id,
                 "type" := change_type, // Manter o nome 'type' para consistência
                 "data" := change_data,
                 "timestamp" := polcore().systime
             };

             // Adicionar evento (evita duplicatas se necessário, mas para simplicidade, apenas adiciona)
             events.append(event_entry);

             // Limitar tamanho (opcional, mas recomendado)
             while (events.size() > 50) 
                 events.erase(1); 
             endwhile

             SetElementProp(elem, "pending", events);
             LogInfo("EventoTerritorio", "Evento " + change_type + " para " + territory_id + " registrado no datafile de mensagens.");
             return 1; // Indicar sucesso no registro
        else
             LogError("EventoTerritorio", "Falha ao encontrar/criar elemento 'controller_events' em :faccao:messages");
        endif
    else
        LogError("EventoTerritorio", "Falha ao abrir datafile :faccao:messages para registrar evento.");
    endif
     
    // 2. Registrar no log (manter)
    LogInfo("territorio", "Território " + territory_id + " - " + change_type + " - Dados: " + change_data);
    return 0; // Indicar falha no registro
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