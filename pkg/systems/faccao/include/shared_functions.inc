use uo;
use os;
use cfgfile;
use datafile;
use guilds;

include ":datafile:datafile";
include ":faccao:faccao_constants";

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
function ContarItensDoadosNoBau(bau)
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
    
    if (!bau)
        return recursos;
    endif
    
    foreach item in EnumerateItemsInContainer(bau)
        if (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_COMIDA))
            recursos.comida += item.amount;
        elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_MOEDAS))
            recursos.moedas += item.amount;
        elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_COURO))
            recursos.couro += item.amount;
        elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_METAL))
            recursos.metal += item.amount;
        elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_MADEIRA))
            recursos.madeira += item.amount;
        elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_PANO))
            recursos.pano += item.amount;
        elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_PEDRA))
            recursos.pedras += item.amount;
        elseif (GetObjProperty(item, DONATION_PREFIX + RESOURCE_TYPE_JOIAS))
            recursos.joias += item.amount;
        endif
        
        sleepms(2);
    endforeach

    return recursos;
endfunction

/* 
 * AtualizarRecursosNoDataFile(recursos)
 * 
 * Purpose: Atualiza os valores de recursos no datafile do reino
 * Parameters: 
 *   recursos - Struct com contagens de recursos
 * Return:
 *   1 para sucesso, 0 para falha
 */
function AtualizarRecursosNoDataFile(recursos)
    var data_file := DFOpenDataFile(KINGDOM_RESOURCE_DATAFILE, DF_CREATE);
    var elem := DFFindElement(data_file, KINGDOM_RESOURCE_ELEM, DF_CREATE);
    
    if (!elem)
        return 0;
    endif
    
    // Atualizar cada recurso no datafile
    elem.SetProp("comida", recursos.comida);
    elem.SetProp("moedas", recursos.moedas);
    elem.SetProp("couro", recursos.couro);
    elem.SetProp("metal", recursos.metal);
    elem.SetProp("madeira", recursos.madeira);
    elem.SetProp("pano", recursos.pano);
    elem.SetProp("pedras", recursos.pedras);
    elem.SetProp("joias", recursos.joias);
    
    return 1;
endfunction

/* 
 * ObterRecursosDoReino()
 * 
 * Purpose: Obtém os valores atuais dos recursos do datafile
 * Return:
 *   Struct com os valores dos recursos
 */
function ObterRecursosDoReino()
    var data_file := DFOpenDataFile(KINGDOM_RESOURCE_DATAFILE, DF_CREATE);
    var elem := DFFindElement(data_file, KINGDOM_RESOURCE_ELEM, DF_CREATE);
    
    var recursos := struct{
        comida := elem.GetProp("comida") ? elem.GetProp("comida") : 0,
        moedas := elem.GetProp("moedas") ? elem.GetProp("moedas") : 0,
        couro := elem.GetProp("couro") ? elem.GetProp("couro") : 0,
        metal := elem.GetProp("metal") ? elem.GetProp("metal") : 0,
        madeira := elem.GetProp("madeira") ? elem.GetProp("madeira") : 0,
        pano := elem.GetProp("pano") ? elem.GetProp("pano") : 0,
        pedras := elem.GetProp("pedras") ? elem.GetProp("pedras") : 0,
        joias := elem.GetProp("joias") ? elem.GetProp("joias") : 0
    };
    
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
    if (cached_faction && last_accessed && (ReadGameClock() - last_accessed < 300))
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
            if (territorios_seriais)
                foreach territorio in (territorios_seriais.keys())
                    var info := territorios_seriais[territorio];
                    if (info.bau == chest.serial || info.mesa == chest.serial || info.mural == chest.serial)
                        faction := info.faction;
                        Print("DEBUG-FACTION: Encontrado via datafile de seriais: " + faction);
                        break;
                    endif
                    sleepms(1);
                endforeach
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

function FindGuildByName(name)
    // Primeiro tenta usar ListGuilds (mais completo)
    var guilds := ListGuilds();
    if (guilds)
        foreach guild in guilds
            if (guild.GetProp(FACTION_PROP_NAME) == name)
                return guild.guildid;
            endif
            sleepms(2);
        endforeach
    endif
    
    // Backup: procura por jogadores online se ListGuilds falhar
    foreach player in (EnumerateOnlineCharacters())
        if (player.guildid && player.guild.GetProp(FACTION_PROP_NAME) == name)
            return player.guild.guildid;
        endif
        sleepms(2);
    endforeach
    
    return 0;
endfunction


function ForEachInArray(array_to_iterate, callback)
    if (!array_to_iterate || array_to_iterate.size() < 1)
        return;
    endif
    
    // POL usa índices baseados em 1, não 0
    for i := 1 to array_to_iterate.size()
        callback.call(array_to_iterate[i], i);
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
    if (!text || text == "")
        return array{};
    endif
    
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
                
                // Se territórios específicos foram solicitados
                if (territory_id && territorio_id != territory_id)
                    continue;
                endif
                
                // Verificar se a facção corresponde
                if (info.faction != faction_name)
                    continue;
                endif
                
                // Adicionar itens conforme o tipo solicitado
                if (!item_type || item_type == ITEM_TYPE_BAU)
                    var bau := SystemFindObjectBySerial(info.bau);
                    if (bau)
                        matching_items.append(bau);
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
    foreach elem_name in (rect_df.keys())
        var elem := rect_df.FindElement(elem_name);
        var faction := elem.GetProp(TERRITORY_FACTION_PROP);
        if (!faction)
            faction := elem.GetProp("Faction"); // Compatibilidade
        endif
        
        if (faction == faction_name && elem.GetProp(TERRITORY_IS_CAPITAL_PROP))
            elem.EraseProp(TERRITORY_IS_CAPITAL_PROP);
        endif
        sleepms(2);
    endforeach
    
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

/*
 * IsTerritoryCapital(territory_id)
 *
 * Purpose: Verifica se um território é uma capital
 * Parameters:
 *   territory_id - ID do território
 * Return:
 *   1 se for capital, 0 se não for
 */
function IsTerritoryCapital(territory_id)
    var territory_elem := FindTerritoryByID(territory_id);
    if (territory_elem && territory_elem.GetProp(TERRITORY_IS_CAPITAL_PROP))
        return 1;
    endif
    
    return 0;
endfunction


function CanAccessChest(who, chest)
    // Depuração avançada
    Print("DEBUG-ACESS: Iniciando verificação de acesso para " + who.name + " ao baú " + chest.desc);
    
    // GMs sempre podem acessar
    if (who.cmdlevel >= 3)
        Print("DEBUG-ACCESS: Acesso permitido - GM");
        return 1;
    endif
    
    // Obter a facção do baú usando nossa função aprimorada
    var chest_faction := GetFactionOfChest(chest);
    Print("DEBUG-ACCESS: Facção do baú: " + chest_faction);
    
    // ===== VERIFICAÇÃO DE FACÇÃO/GUILDA =====
    var player_guild := who.guild;
    var player_faction := "";
    
    if (player_guild)
        // Tentar obter o nome da facção de várias fontes para garantir
        player_faction := player_guild.GetProp(FACTION_PROP_NAME);
        if (!player_faction)
            player_faction := player_guild.GetProp("name");
        endif
        if (!player_faction)
            player_faction := player_guild.guildname;
        endif
        
        Print("DEBUG-ACCESS: Facção do jogador: " + player_faction);
        
        // CORREÇÃO 1: Verificação mais robusta de liderança
        var is_leader := 0;
        var guild_leader := player_guild.leader;
        if (guild_leader == who.serial)
            is_leader := 1;
            Print("DEBUG-ACCESS: Jogador é líder da guilda");
        endif
        
        // Se pertence à mesma facção que o baú
        if (player_faction == chest_faction)
            // Verificar se é líder da guilda (mais robusto)
            if (is_leader)
                Print("DEBUG-ACCESS: Acesso permitido - Líder da mesma facção");
                return 1;
            endif
            
            // CORREÇÃO 2: Verificação mais robusta de cargo
            var player_rank := GetObjProperty(who, "guild_rank");
            if (player_rank in {"Lider", "Capitao", "Tenente", "Leader", "Captain", "Lieutenant"})
                Print("DEBUG-ACCESS: Acesso permitido - Cargo na guilda: " + player_rank);
                return 1;
            endif
            
// NOVO: Verificação de oficial da guilda
var guild_officer := 0;
var officer_title := "";
foreach member in (player_guild.members)
    if (member.serial == who.serial && member.guild_title in {"Lider", "Capitao", "Tenente", "Leader", "Captain", "Lieutenant"})
        guild_officer := 1;
        officer_title := member.guild_title;
        break;
    endif
    sleepms(1);
endforeach

if (guild_officer)
    // ADICIONADO: Atualizar automaticamente a propriedade guild_rank
    SetObjProperty(who, "guild_rank", officer_title);
    Print("DEBUG-ACCESS: Acesso permitido - Oficial reconhecido pela guilda");
    return 1;
endif
        endif
    endif
    
    // ===== VERIFICAÇÃO ALTERNATIVA DE FACÇÃO =====
    var alt_faction := GetObjProperty(who, "faction");
    if (alt_faction == chest_faction)
        Print("DEBUG-ACCESS: Propriedade alternativa de facção corresponde: " + alt_faction);
        
        // CORREÇÃO 3: Verificação mais robusta de cargo alternativo
        var player_rank := GetObjProperty(who, "guild_rank");
        if (player_rank in {"Lider", "Capitao", "Tenente", "Leader", "Captain", "Lieutenant"})
            Print("DEBUG-ACCESS: Acesso permitido - Cargo na facção alternativa: " + player_rank);
            return 1;
        endif
    endif
    
    // ===== VERIFICAÇÃO DE TÍTULOS DE NOBREZA =====
    var chardata := GetObjProperty(who, "chardata");
    if (chardata)
        Print("DEBUG-ACCESS: Título de nobreza: " + chardata.current_nobility_title);
        Print("DEBUG-ACCESS: Cargo profissional: " + chardata.current_professional_position);
        
        // Baús do Reino - acessíveis por cargos reais
        if (chest_faction == KINGDOM_FACTION || chest_faction == "Reino")
            if (chardata.current_nobility_title in {"Rei", "Rainha"} || 
                chardata.current_professional_position in {"Mão do Rei", "Imperador"})
                Print("DEBUG-ACCESS: Acesso permitido - Título/cargo real para baú do Reino");
                return 1;
            endif
        endif
        
        // NOVO: Acesso de emergência para reis
        if (chardata.current_nobility_title in {"Rei", "Rainha"})
            // Os reis podem acessar qualquer baú, mas apenas com permissão especial
            if (GetObjProperty(who, "AllowKingEmergencyAccess"))
                Print("DEBUG-ACCESS: Acesso permitido - Acesso de emergência do rei");
                return 1;
            endif
        endif
        
        // NOVO: Teste de permissão especial para debug/testes
        if (GetObjProperty(who, "AllowAllChestAccess"))
            Print("DEBUG-ACCESS: Acesso permitido - Permissão especial AllowAllChestAccess");
            return 1;
        endif
    endif
    
    Print("DEBUG-ACCESS: Acesso negado - Nenhuma condição atendida");
    return 0;
endfunction


