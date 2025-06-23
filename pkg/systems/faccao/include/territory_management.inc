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
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        return 0;
    endif
    
    var elem := TM_FindTerritory(territory_id_or_name);
    if (!elem)
        return 0;
    endif
      var info := struct{
        "name" := GetElementProp(elem, TERRITORY_NAME_PROP, territory_id_or_name),
        "id" := GetElementProp(elem, TERRITORY_ID_PROP, territory_id_or_name),
        "faction" := GetTerritoryFaction(elem),
        "rect" := GetElementProp(elem, TERRITORY_RECT_PROP, ""),
        "realm" := GetElementProp(elem, TERRITORY_REALM_PROP, "britannia"),
        "is_capital" := GetElementProp(elem, TERRITORY_IS_CAPITAL_PROP, 0) // Adicionar status de capital aqui
    };
      // Compatibilidade retroativa
    if (!info.faction)
        info.faction := GetElementProp(elem, TERRITORY_FACTION_PROP, "");
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
    var territory_elem := TM_FindTerritory(territory_id_or_name);    if (!territory_elem)
        LogError("territorio", "TM_SetTerritoryFaction: Território não encontrado: " + territory_id_or_name);
        return 0;
    endif
    
    // Obter a facção antiga antes de mudar
    var old_faction := GetElementProp(territory_elem, TERRITORY_FACTION_PROP);
    if (!old_faction)
        old_faction := GetElementProp(territory_elem, LEGACY_FACTION_PROP); // Tentar propriedade legada
    endif
    
    // Definir a propriedade principal
    var result := SetTerritoryFaction(territory_elem, faction_name);
    
    LogInfo("territorio", "Facção do território " + territory_id_or_name + " definida para " + faction_name);
    
    // --- SINCRONIZAÇÃO AUTOMÁTICA DA CPROP TERRITORIES ---
    // Atualizar CProp da guilda ANTIGA (remover o território)
    if (old_faction && old_faction != DEFAULT_FACTION)
        LogDebug("TM_SetTerritoryFaction", "Removendo território da lista da facção antiga: " + old_faction);
        var old_guild_id := FindGuildByName(old_faction);
        if (old_guild_id)
            var old_guild_obj := FindGuild(old_guild_id);
            if (old_guild_obj)
                var old_territories := old_guild_obj.GetProp("territories");
                if (old_territories && TypeOf(old_territories) == "Array")
                    var new_old_territories := array{};
                    foreach terr in old_territories
                        // Adicionar apenas se não for este território
                        if (CStr(terr) != CStr(territory_id_or_name))
                            new_old_territories.append(terr);
                        endif
                        sleepms(2);
                    endforeach
                    
                    old_guild_obj.SetProp("territories", new_old_territories);
                    LogDebug("TM_SetTerritoryFaction", "Território removido da facção " + old_faction + ". Territórios restantes: " + new_old_territories.size());
                endif
            endif
        endif
    endif

    // Atualizar CProp da guilda NOVA (adicionar o território)
    if (faction_name && faction_name != DEFAULT_FACTION)
        LogDebug("TM_SetTerritoryFaction", "Adicionando território à lista da nova facção: " + faction_name);
        var new_guild_id := FindGuildByName(faction_name);
        if (new_guild_id)
            var new_guild_obj := FindGuild(new_guild_id);
            if (new_guild_obj)
                var new_territories := new_guild_obj.GetProp("territories");
                if (!new_territories || TypeOf(new_territories) != "Array")
                    new_territories := array{};
                endif
                
                // Verificar se já não existe na lista
                var exists := 0;
                foreach terr in new_territories
                    if (CStr(terr) == CStr(territory_id_or_name))
                        exists := 1;
                        break;
                    endif
                    sleepms(2);
                endforeach
                
                if (!exists)
                    new_territories.append(territory_id_or_name);
                    new_guild_obj.SetProp("territories", new_territories);
                    LogDebug("TM_SetTerritoryFaction", "Território adicionado à facção " + faction_name + ". Total de territórios: " + new_territories.size());
                else
                    LogDebug("TM_SetTerritoryFaction", "Território já existe na lista da facção " + faction_name);
                endif
            endif
        endif
    endif
    // --- FIM DA SINCRONIZAÇÃO ---
    
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
    // Primeiro, verificar se já existe uma capital para esta facção
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        return 0;
    endif
    
    // Procurar por territórios existentes marcados como capital
    foreach elem_name in (rect_df.keys())
        var elem := rect_df.FindElement(elem_name);
        if (!elem)
            continue;        endif
        
        // Verificar se é da mesma facção
        var elem_faction := GetTerritoryFaction(elem);
        
        // Se encontrar uma capital existente da mesma facção
        if (elem_faction == faction_name && GetElementProp(elem, TERRITORY_IS_CAPITAL_PROP))
            // Remover status de capital do território antigo
            elem.EraseProp(TERRITORY_IS_CAPITAL_PROP);
            LogDebug("territorio", "Removido status de capital do território: " + elem_name);
        endif
        sleepms(2);
    endforeach
    
    // Agora definir o novo território como capital
    var territory_elem := TM_FindTerritory(territory_id);
    if (!territory_elem)
        return 0;
    endif
    
    SetElementProp(territory_elem, TERRITORY_IS_CAPITAL_PROP, 1);
    return 1;
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
    // Usar TM_GetTerritoryInfo para manter consistência
    var territory_info := TM_GetTerritoryInfo(territory_id);
    if (!territory_info)
        return 0;
    endif
    
    return territory_info.is_capital;
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
        // TODO: Usar polcore().systime quando incluído em arquivo .src
        var timestamp := 0; // polcore().systime;
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
function TM_FindTerritoryObjects(territory_id, faction_name := "", item_type := "")    var result := struct{
        "bau" := 0,
        "mesa" := 0,
        "mesa_inscricao" := 0,
        "mural" := 0,
        "bandeira" := 0
    };
    
    if (!territory_id)
        LogError("territorio", "TM_FindTerritoryObjects: Território não especificado");
        return result;
    endif
    
    // Estratégia 1: Buscar pelo elemento específico no datafile
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        var territory_elem := SafeFindElement(seriais_df, territory_id);        if (territory_elem)
            var elem_faction := GetTerritoryFaction(territory_elem);
            
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
                
                if (!item_type || item_type == ITEM_TYPE_MESA_INSCRICAO)
                    var serial_mesa_inscricao := GetElementProp(territory_elem, "mesa_inscricao");
                    if (serial_mesa_inscricao)
                        result.mesa_inscricao := SystemFindObjectBySerial(serial_mesa_inscricao);
                    endif
                endif
                  if (!item_type || item_type == ITEM_TYPE_MURAL)
                    var serial_mural := GetElementProp(territory_elem, "mural");
                    if (serial_mural)
                        result.mural := SystemFindObjectBySerial(serial_mural);
                    endif
                endif
                
                if (!item_type || item_type == "BANDEIRA")
                    var serial_bandeira := GetElementProp(territory_elem, "bandeira");
                    if (serial_bandeira)
                        result.bandeira := SystemFindObjectBySerial(serial_bandeira);
                    endif
                endif
                  
                // Se encontrou pelo menos um objeto, retornar
                if (result.bau || result.mesa || result.mesa_inscricao || result.mural || result.bandeira)
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
                    
                    if (!item_type || item_type == "BANDEIRA")
                        if (info.bandeira)
                            result.bandeira := SystemFindObjectBySerial(info.bandeira);
                        endif
                    endif
                    
                    // Se encontrou pelo menos um objeto, retornar
                    if (result.bau || result.mesa || result.mesa_inscricao || result.mural || result.bandeira)
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
            
            if (!faction_name || faction_obj == faction_name)                if (!item_type || 
                    (item_type == ITEM_TYPE_BAU && item.objtype == 0xFF11) ||
                    (item_type == ITEM_TYPE_MESA && item.objtype == 0xFE5C) ||
                    (item_type == ITEM_TYPE_MESA_INSCRICAO && item.objtype == 0xF8A8) ||
                    (item_type == ITEM_TYPE_MURAL && item.objtype == 0x46AE))
                    
                    if (item.objtype == 0xFF11)
                        result.bau := item;
                    elseif (item.objtype == 0xFE5C)
                        result.mesa := item;
                    elseif (item.objtype == 0xF8A8)
                        result.mesa_inscricao := item;
                    elseif (item.objtype == 0x46AE)
                        result.mural := item;
                    endif
                endif
            endif
        endif
        sleepms(1);
    endforeach
      // Se encontrou objetos no mundo, atualizar o registro
    if (result.bau || result.mesa || result.mesa_inscricao || result.mural)
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
 *   faction_name - Nome da facção (opcional, padrão: DEFAULT_FACTION)
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
    
    // Definir a facção controladora (padrão: DEFAULT_FACTION)
    if (!faction_name)
        faction_name := DEFAULT_FACTION;
    endif
      SetElementProp(elem, TERRITORY_FACTION_PROP, faction_name);
    SetElementProp(elem, TERRITORY_FACTION_PROP, faction_name); // Para compatibilidade
    
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
    
    // Verificar permissões do jogador
    var territory_faction := GetTerritoryFaction(elem);
    
    // Verificar se o jogador pertence à facção ou é GM
    var has_permission := 0;
    if (who.cmdlevel >= 1)
        has_permission := 1;
    else
        var player_guild := FindGuild(who.guildid);
        // Verificar se o jogador tem cargo na facção
        var is_leader := 0;
        if (player_guild && player_guild.guildname == territory_faction)
            var player_rank := player_guild.GetGuildRank(who);
            if (player_rank >= 8) // Rank 8 ou maior = líder
                is_leader := 1;
                has_permission := 1;
            endif
        endif
    endif


    // Criar o gump base
    var gump := GFCreateGump(50, 50, width, height);
    GFClosable(gump, 1);
    GFResizePic(gump, 0, 0, 9200, width, height);
    
    var y := 20;
    
    // Título
    GFTextLine(gump, width/2-80, y, 1153, "Informações do Território");
    y += 30;
    
    // Nome
    var name := GetElementProp(elem, "Name", "Sem Nome");
    GFTextLine(gump, 20, y, 2100, "Nome:");
    GFTextLine(gump, 120, y, 63, name);
    y += 25;
    
    // ID
    var territory_id := GetElementProp(elem, TERRITORY_ID_PROP, "");
    if (territory_id)
        GFTextLine(gump, 20, y, 2100, "ID:");
        GFTextLine(gump, 120, y, 63, territory_id);
        y += 25;
    endif
    
    // Facção
    GFTextLine(gump, 20, y, 2100, "Facção:");
    GFTextLine(gump, 120, y, 63, territory_faction);
    y += 25;
    
    // Coordenadas
    var rect := GetElementProp(elem, "Rect", "");
    if (rect)
        GFTextLine(gump, 20, y, 2100, "Coordenadas:");
        GFTextLine(gump, 120, y, 63, rect);
        y += 25;
    endif
    
    // Realm
    var realm := GetElementProp(elem, "Realm", "britannia");
    GFTextLine(gump, 20, y, 2100, "Realm:");
    GFTextLine(gump, 120, y, 63, realm);
    y += 40;

    // Seção de Ações Administrativas (visível para GMs e líderes da facção)
    if (has_permission && (who.cmdlevel >= 1 || is_leader))
        GFTextLine(gump, 20, y, 38, "Ações Administrativas:");
        y += 25;
        
        // Botão para remover controle
        GFAddButton(gump, 30, y, 2117, 2118, GF_CLOSE_BTN, 100);
        GFTextLine(gump, 50, y, 63, "Remover controle da facção");
        y += 25;
        
        // Botão para transferir
        GFAddButton(gump, 30, y, 2117, 2118, GF_CLOSE_BTN, 200);
        GFTextLine(gump, 50, y, 63, "Transferir para outra facção");
        y += 25;
        
        // Botão para definir como capital
        GFAddButton(gump, 30, y, 2117, 2118, GF_CLOSE_BTN, 300);
        GFTextLine(gump, 50, y, 63, "Definir como Capital");
        y += 25;
    endif

    // Botão OKAY no final
    y := height - 40;
    GFAddButton(gump, width/2-30, y, 2128, 2129, GF_CLOSE_BTN, 0);
    
    var input := GFSendGump(who, gump);
    var button := input[0];
    
    if (!has_permission && button != 0)
        SendSysMessage(who, "Você não tem permissão para executar esta ação.");
        return;
    endif

    case (button)
        100: // Remover controle
            if (YesNo(who, "Tem certeza que deseja remover o controle da facção?"))
                TM_SetTerritoryFaction(territory_id, DEFAULT_FACTION);
                SendSysMessage(who, "Controle removido com sucesso.");
            endif
        200: // Transferir
            var new_faction := RequestGump(who, "Digite o nome da nova facção:");
            if (new_faction)
                TM_TransferTerritoryFaction(territory_id, new_faction, who);
            endif
        300: // Definir como Capital
            if (YesNo(who, "Definir este território como capital?"))
                TM_SetTerritoryAsCapital(territory_id, territory_faction);
                SendSysMessage(who, "Território definido como capital.");
            endif
    endcase
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
    
    // Remover baú (com verificação)
    if (objetos.bau && objetos.bau.serial) // <-- ADICIONADA VERIFICAÇÃO .serial
        var result := DestroyItem(objetos.bau);
        if (result)
            objetos_removidos += 1;
            LogDebug("territorio", "Baú removido: " + objetos.bau.serial);
        else
            LogError("territorio", "Falha ao remover baú: " + objetos.bau.serial + ", Resultado: " + result);
        endif
    endif; // Adicionado ponto e vírgula

    // Remover mesa (com verificação)
    if (objetos.mesa && objetos.mesa.serial) // <-- ADICIONADA VERIFICAÇÃO .serial
        var result := DestroyItem(objetos.mesa);
        if (result)
            objetos_removidos += 1;
            LogDebug("territorio", "Mesa removida: " + objetos.mesa.serial);
        else
            LogError("territorio", "Falha ao remover mesa: " + objetos.mesa.serial + ", Resultado: " + result);
        endif
    endif;
    
    // Remover mesa de inscrição
    if (objetos.mesa_inscricao)
        var result := DestroyItem(objetos.mesa_inscricao);
        if (result)
            objetos_removidos += 1;
            LogDebug("territorio", "Mesa de inscrição removida: " + objetos.mesa_inscricao.serial);
        else
            LogError("territorio", "Falha ao remover mesa de inscrição: " + objetos.mesa_inscricao.serial + ", Resultado: " + result);
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
    
    // Remover estandarte/bandeira
    if (objetos.bandeira)
        var result := DestroyItem(objetos.bandeira);
        if (result)
            objetos_removidos += 1;
            LogDebug("territorio", "Estandarte/bandeira removido: " + objetos.bandeira.serial);
        else
            LogError("territorio", "Falha ao remover estandarte/bandeira: " + objetos.bandeira.serial + ", Resultado: " + result);
        endif
    endif
    
    // Remover registros de seriais no datafile
    var seriais_df := SafeOpenDataFile(":faccao:seriais");
    if (seriais_df)
        // MÉTODO 1: Remover do elemento específico
        var territory_elem := SafeFindElement(seriais_df, territory_id);
        if (territory_elem)
            // Salvar informações de fação para logs
            var old_faction := GetElementProp(territory_elem, "faction");              // Limpar propriedades
territory_elem.EraseProp("bau");
territory_elem.EraseProp("mesa");
territory_elem.EraseProp("mesa_inscricao");
territory_elem.EraseProp("mural");
territory_elem.EraseProp("bandeira");
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
    var is_capital := territory_info.is_capital; // Capturar status de capital
    
    // Verificar se a facção mudou
    if (old_faction == new_faction)
        if (who)
            SendSysMessage(who, "O território já pertence à facção " + new_faction);
        endif
        LogWarning("territorio", "TM_TransferTerritoryFaction: Território já pertence à facção: " + new_faction);
        return 0;
    endif

    // Se for capital, remover status antes da transferência
    // Se for capital, remover status antes da transferência
    if (is_capital)
        LogInfo("CapitalDebug", $"Território {territory_id} é capital, removendo status antes da transferência");
        
        // Remover a propriedade de capital do elemento atual
        var elem := TM_FindTerritory(territory_id);
        if (elem)
            // Garantir que o status de capital seja removido
            elem.EraseProp(TERRITORY_IS_CAPITAL_PROP);
            
            // Garantir que a propriedade antiga também seja removida (compatibilidade)
            elem.EraseProp("IsCapital");
            
            LogInfo("CapitalDebug", $"Status de capital removido do território {territory_id}");
            if (who)
                SendSysMessage(who, "O território perdeu o status de capital devido à transferência.");
            endif
        endif
    endif

    // Verificar todos os territórios da nova facção para garantir que não haja outro capital
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (rect_df)
        foreach elem_name in (rect_df.keys())
            var elem := SafeFindElement(rect_df, elem_name);
            if (elem)
                var faction := GetElementProp(elem, TERRITORY_FACTION_PROP);
                if (!faction)
                    faction := GetElementProp(elem, "Faction");
                endif
                
                if (faction == new_faction)
                    // Se encontrar outro território como capital da nova facção, remover
                    if (GetElementProp(elem, TERRITORY_IS_CAPITAL_PROP))
                        elem.EraseProp(TERRITORY_IS_CAPITAL_PROP);
                        elem.EraseProp("IsCapital");
                        LogInfo("CapitalDebug", $"Removido status de capital de {elem_name} da facção {new_faction}");
                    endif
                endif
            endif
            sleepms(2);
        endforeach
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
    

    // ====================================================================
// >>>>> INÍCIO DA CORREÇÃO <<<<<
// ====================================================================

// 1. Remover objetos existentes - CHAMADA DIRETA DA FUNÇÃO, NÃO START_SCRIPT
LogInfo("territorio", "Transferência: Chamando TM_DeleteTerritoryObjects para limpar " + old_faction);
TM_DeleteTerritoryObjects(territory_id, old_faction);
LogInfo("territorio", "Transferência: Limpeza de objetos antigos concluída.");

// ====================================================================
// >>>>> FIM DA CORREÇÃO <<<<<
// ====================================================================

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

    // ETAPA 4: CRIAR NOVOS OBJETOS PARA A NOVA FACÇÃO
    if (who)
        SendSysMessage(who, "Processando: Criando novos objetos para a facção...");
    endif
    
    var center := TM_GetTerritoryCenter(territory_id);
    if (!center)
        DesfazerTransacao(transaction_id);
        if (who) SendSysMessage(who, "Erro ao obter centro do território. Transferência cancelada."); endif
        LogError("territorio", "TM_TransferTerritoryFaction: Falha ao obter centro do território");
        return 0;
    endif;

    // --- Declaração de variáveis para os objetos ---
    var chest := 0;
    var table := 0;
    var mural := 0;
    var inscricao_table := 0;
    var error_msg := "";

    // --- Bloco de criação de itens com verificação manual ---
    
    // 4.1 Criar o baú
    chest := CreateItemAtLocation(center.x, center.y, center.z, TERRITORIO_BAU, 1, center.realm);
    if (!chest)
        error_msg := "Falha ao criar o baú.";
    else
        chest.movable := 0;
        SetObjectFaction(chest, new_faction);
        SetObjProperty(chest, TERRITORY_ID_PROP, territory_id);
        SetName(chest, "Baú de Doações de " + new_faction + " - " + territory_id);
        chest.usescript := ":faccao:baudoacao/use";
        SetObjProperty(chest, "OnInsertScript", ":faccao:baudoacao/onInsert");
    endif;

    // 4.2 Criar a mesa de gestão
    if (!error_msg)
        table := CreateItemAtLocation(center.x + 1, center.y, center.z, TERRITORIO_MESA, 1, center.realm);
        if (!table)
            error_msg := "Falha ao criar a mesa de gestão.";
        else
            table.movable := 0;
            SetObjectFaction(table, new_faction);
            SetObjProperty(table, TERRITORY_ID_PROP, territory_id);
            SetName(table, "Mesa de Gestão de " + new_faction + " - " + territory_id);
            table.usescript := ":faccao:gestao_use";
        endif;
    endif;

    // 4.3 Criar o mural de recrutamento
    if (!error_msg)
        mural := CreateItemAtLocation(center.x, center.y + 1, center.z, TERRITORIO_MURAL, 1, center.realm);
        if (!mural)
            error_msg := "Falha ao criar o mural de recrutamento.";
        else
            mural.movable := 0;
            SetObjectFaction(mural, new_faction);
            SetObjProperty(mural, TERRITORY_ID_PROP, territory_id);
            SetName(mural, "Mural de Recrutamento de " + new_faction + " - " + territory_id);
            mural.usescript := ":faccao:faccao";
        endif;
    endif;
    
    // 4.4 Criar a mesa de inscrição
    if (!error_msg)
        inscricao_table := CreateItemAtLocation(center.x - 1, center.y + 1, center.z, TERRITORIO_MESA_INSCRICAO, 1, center.realm);
        if (!inscricao_table)
            error_msg := "Falha ao criar a mesa de inscrição.";
        else
            inscricao_table.movable := 0;
            SetObjectFaction(inscricao_table, new_faction);
            SetObjProperty(inscricao_table, TERRITORY_ID_PROP, territory_id);
            SetName(inscricao_table, "Mesa de Inscrição [" + new_faction + "]");
            inscricao_table.usescript := ":faccao:mesainscricao";
        endif;
    endif;

    // --- Verificação de Falhas ---
    if (error_msg != "")
        LogError("territorio", "TM_TransferTerritoryFaction: " + error_msg);
        if (who) SendSysMessage(who, "Erro na criação de objetos: " + error_msg + ". Revertendo."); endif
        
        if (chest) DestroyItem(chest); endif;
        if (table) DestroyItem(table); endif;
        if (mural) DestroyItem(mural); endif;
        if (inscricao_table) DestroyItem(inscricao_table); endif;
        
        DesfazerTransacao(transaction_id);
        return 0;
    endif;

    // --- 4.5 Salvar os seriais no datafile (VERSÃO SEGURA) ---
    var object_serials_struct := struct;
    object_serials_struct.+faction := new_faction;
    
    // Adiciona o serial de cada objeto APENAS se ele foi criado com sucesso
    if(chest) object_serials_struct.+bau := chest.serial; endif;
    if(table) object_serials_struct.+mesa := table.serial; endif;
    if(mural) object_serials_struct.+mural := mural.serial; endif;
    if(inscricao_table) object_serials_struct.+mesa_inscricao := inscricao_table.serial; endif;
    
    var register_result := TM_RegisterTerritoryObjects(territory_id, object_serials_struct);
    
    if (!register_result)
        LogError("territorio", "FALHA ao registrar seriais usando TM_RegisterTerritoryObjects na transferência.");
        DesfazerTransacao(transaction_id);
        if (who) SendSysMessage(who, "Erro ao registrar seriais. Transferência revertida."); endif
        return 0;
    endif;
    
    // Etapa 5: Notificar os sistemas sobre a mudança.
    TM_NotifyTerritoryChange(territory_id, "transfer", struct{"old_faction" := old_faction, "new_faction" := new_faction});
    
    // Etapa 6: Finalizar a transação e notificar o jogador.
    FinalizarTransacao(transaction_id);
    
    if (who)
        SendSysMessage(who, "Território transferido com sucesso para " + new_faction + "!");
    endif
    
    LogInfo("territorio", "Transferência concluída. Território " + territory_id + " transferido de " + old_faction + " para " + new_faction);
    
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
    
    // TODO: Usar polcore().systime quando incluído em arquivo .src
    var elem_name := "territory_faction_" + territory_id + "_0"; // + polcore().systime;
    var backup_elem := SafeFindElement(backup_df, elem_name, DF_CREATE);
    if (!backup_elem)
        LogError("territorio", "BackupTerritoryFactionRelation: Failed to create backup element");
        return 0;
    endif
    
    SetElementProp(backup_elem, "territory_id", territory_id);
    SetElementProp(backup_elem, "faction", faction_name);
    // TODO: Usar polcore().systime quando incluído em arquivo .src
    SetElementProp(backup_elem, "timestamp", 0); // polcore().systime;
    
    LogInfo("territorio", "Backup created for territory " + territory_id + " with faction " + faction_name);
    return 1;
endfunction

function TM_HasCapital(faction_name)
    if (!faction_name)
        return 0;
    endif
    
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        return 0;
    endif
    
    foreach elem_name in (rect_df.keys())
        var elem := SafeFindElement(rect_df, elem_name);
        if (!elem)
            continue;
        endif
          // Verificar a facção atual
        var territory_faction := GetTerritoryFaction(elem);

        // Se for da facção procurada e for capital
        if (territory_faction == faction_name && GetElementProp(elem, TERRITORY_IS_CAPITAL_PROP))
            return 1;
        endif
        
        sleepms(2);
    endforeach
    
    return 0;
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
                 // TODO: Usar polcore().systime quando incluído em arquivo .src
                 "timestamp" := 0 // polcore().systime
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
            var territory_faction := GetTerritoryFaction(elem);
            
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


/*
 * SyncGuildTerritories(faction_name)
 *
 * Purpose: Sincroniza a lista de territórios da guilda com o datafile
 * Parameters: faction_name - Nome da facção
 * Return: Número de territórios sincronizados
 */
function SyncGuildTerritories(faction_name)
    if (!faction_name)
        return 0;
    endif
    
    LogInfo("sync_territories", "Iniciando sincronização de territórios para: " + faction_name);
    
    // Obter a guilda
    var guild_id := FindGuildByName(faction_name);
    if (!guild_id)
        LogError("sync_territories", "Guilda não encontrada: " + faction_name);
        return 0;
    endif
    
    var guild := FindGuild(guild_id);
    if (!guild)
        LogError("sync_territories", "Objeto da guilda não encontrado");
        return 0;
    endif
    
    // Buscar territórios no datafile
    var rect_df := SafeOpenDataFile(":architect:areas");
    if (!rect_df)
        LogError("sync_territories", "Não foi possível abrir datafile de áreas");
        return 0;
    endif
    
    var territory_list := array{};
    
    foreach elem_name in (rect_df.keys())        var elem := SafeFindElement(rect_df, elem_name);
        if (!elem)
            continue;
        endif
        
        var terr_faction := GetTerritoryFaction(elem);
        
        if (terr_faction == faction_name)
            var terr_id := GetElementProp(elem, TERRITORY_ID_PROP);
            if (!terr_id)
                terr_id := elem_name;
            endif
            
            territory_list.append(terr_id);
            LogDebug("sync_territories", "Território encontrado: " + terr_id);
        endif
        sleepms(2);
    endforeach
    
    // Atualizar a lista na guilda
    guild.SetProp("territories", territory_list);
    
    LogInfo("sync_territories", "Sincronizados " + territory_list.size() + " territórios para " + faction_name);
    
    return territory_list.size();
endfunction

// ====================================================================
// AÇÃO 4 (Adição): Nova função para expandir um território
// ====================================================================
/*
 * TM_ExpandTerritory(territory_id, new_level)
 *
 * Purpose: Expande o retângulo de um território para as dimensões de um novo nível.
 * Parameters:
 *   territory_id - ID do território a ser expandido
 *   new_level - O novo nível do reino (para buscar as dimensões corretas)
 * Return:
 *   1 para sucesso, 0 para falha.
 */
function TM_ExpandTerritory(territory_id, new_level)
    // 1. Encontrar o território
    var elem := TM_FindTerritory(territory_id);
    if (!elem) 
        LogError("territorio", "TM_ExpandTerritory: Não foi possível encontrar o território com ID: " + territory_id);
        return 0;
    endif

    // 2. Obter o retângulo MÁXIMO (buffer) para achar o centro verdadeiro
    var max_rect_str := GetElementProp(elem, "MaxRect");
    if (!max_rect_str)
    	// Se não tem MaxRect, não é um território expansível.
    	LogWarning("territorio", "TM_ExpandTerritory: Tentativa de expandir território não-expansível (sem MaxRect): " + territory_id);
    	return 0; 
    endif
    
    var coords := SplitWords(max_rect_str);
    var center_x := CInt((CInt(coords[1]) + CInt(coords[3])) / 2);
    var center_y := CInt((CInt(coords[2]) + CInt(coords[4])) / 2);

    // 3. Obter as NOVAS dimensões para o new_level
    var new_dims := GetDimensionsForLevel(new_level); 
    if (!new_dims)
        LogError("territorio", "TM_ExpandTerritory: Dimensões não encontradas para o nível: " + new_level);
        return 0;
    endif
    var new_width := new_dims[1];
    var new_height := new_dims[2];

    // 4. Calcular o NOVO retângulo a partir do centro
    var new_x1 := center_x - CInt(new_width / 2);
    var new_y1 := center_y - CInt(new_height / 2);
    var new_x2 := new_x1 + new_width;
    var new_y2 := new_y1 + new_height;
    var new_rect_str := new_x1 + " " + new_y1 + " " + new_x2 + " " + new_y2;

    // 5. Salvar o novo retângulo (SEM verificação de overlap, pois confiamos no MaxRect)
    SetElementProp(elem, "Rect", new_rect_str);
    
    LogInfo("territorio", "Território " + territory_id + " expandido para Nível " + new_level + ". Novas Coords: " + new_rect_str);
    return 1; // Sucesso
endfunction


/*
 * GetDimensionsForLevel(level)
 *
 * Purpose: Função auxiliar para obter as dimensões de um território com base no nível.
 * Parameters:
 *   level - O nível do reino.
 * Return:
 *   Array com {largura, altura} ou 0 se o nível for inválido.
 */
function GetDimensionsForLevel(level)
    case (level)
        1: return TERRITORY_DIMS_LEVEL_1;
        2: return TERRITORY_DIMS_LEVEL_2;
        3: return TERRITORY_DIMS_LEVEL_3;
        4: return TERRITORY_DIMS_LEVEL_4;
        5: return TERRITORY_DIMS_LEVEL_5;
        default: return 0;
    endcase
endfunction
