use uo;
use guilds;
use datafile;

include ":faccao:faccao_constants";
include ":datafile:datafile";

// Controle de depuração global - 0 = desligado, 1 = ligado
var DEBUG_PERM := 0;
// =======================================
// Funções Principais de Permissão
// =======================================

/*
 * CheckPermission(who, container, action := "access")
 *
 * Purpose: Função única para verificar qualquer tipo de permissão
 * Parameters:
 *   who - O jogador sendo verificado
 *   container - O container (baú) sendo acessado
 *   action - Tipo de ação: "access", "insert", "remove", "open"
 * Return:
 *   1 se tem permissão, 0 se não tem
 */
function CheckPermission(who, container, action := "access")
    case (action)
        "access": return UnifiedCheckOpenPermission(who, container);
        "open": return UnifiedCheckOpenPermission(who, container);
        "insert": return UnifiedCheckInsertPermission(who, container);
        "remove": return UnifiedCheckRemovePermission(who, container);
        default: return 0;
    endcase
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
function UnifiedCheckInsertPermission(who, container)
    LogDebug("permissao", "Verificando permissão de INSERÇÃO para " + who.name);
    
    // GMs sempre podem inserir
    if (who.cmdlevel >= 3)
        LogInfo("permissao", "GM " + who.name + " tem permissão total de inserção");
        return 1;
    endif
    
    // Comportamento permissivo para container não especificado
    if (!container)
        LogWarning("permissao", "Container não especificado - inserção permitida por compatibilidade");
        return 1;
    endif
    
    // Obter informações de facção
    var chest_faction := GetFactionOfChest(container);
    var player_faction := GetPlayerFaction(who);
    
    LogDebug("permissao", $"Facção do baú: {chest_faction}, Facção do jogador: {player_faction}");
    
    // Baús do Reino: inserção livre
    if (chest_faction == KINGDOM_FACTION || chest_faction == "Reino")
        LogInfo("permissao", "Baú do Reino - inserção permitida para todos");
        return 1;
    endif
    
    // Membros da mesma facção podem inserir
    if (player_faction == chest_faction)
        LogInfo("permissao", who.name + " membro da facção - inserção permitida");
        return 1;
    endif
    
    // Permissões especiais
    if (HasSpecialPermission(who, container, "insert"))
        LogInfo("permissao", who.name + " tem permissão especial de inserção");
        return 1;
    endif
    
    // Verificação de títulos nobres
    var chardata := GetObjProperty(who, "chardata");
    if (chardata && 
        (chardata.current_nobility_title in {"Rei", "Rainha"} || 
         chardata.current_professional_position in {"Mão do Rei", "Imperador"}))
        LogInfo("permissao", who.name + " tem título real - inserção permitida");
        return 1;
    endif
    
    // Negação final
    LogWarning("permissao", who.name + " sem permissão para inserção");
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
    // Log inicial de verificação de permissão
    LogDebug("permissao", "Verificando permissão de remoção para " + who.name);
    
    // GMs sempre podem remover
    if (who.cmdlevel >= 3)
        LogInfo("permissao", "GM " + who.name + " tem permissão total");
        return 1;
    endif
    
    // Verificação sem container - compatibilidade com títulos de nobreza
    if (!container)
        var chardata := GetObjProperty(who, "chardata");
        if (!chardata)
            LogWarning("permissao", "Sem dados de personagem para " + who.name);
            return 0;
        endif
    
        // Verificação simplificada de títulos reais
        if (chardata.current_nobility_title in {"Rei", "Rainha"} || 
            chardata.current_professional_position == "Mão do Rei")
            LogInfo("permissao", who.name + " tem título real");
            return 1;
        endif
    
        return 0;
    endif
    
    // Obter informações de facção
    var chest_faction := GetFactionOfChest(container);
    var player_faction := GetPlayerFaction(who);
    
    LogDebug("permissao", "Facção do baú: " + chest_faction + ", Facção do jogador: " + player_faction);
    
    // Regras para baús do Reino
    if (chest_faction == KINGDOM_FACTION || chest_faction == "Reino")
        var chardata := GetObjProperty(who, "chardata");
        if (chardata && 
            (chardata.current_nobility_title in {"Rei", "Rainha"} || 
             chardata.current_professional_position in {"Mão do Rei", "Imperador"}))
            LogInfo("permissao", who.name + " tem cargo real - acesso ao baú do Reino permitido");
            return 1;
        endif
        
        LogWarning("permissao", who.name + " sem permissão para baú do Reino");
        return 0;
    endif
    
    // Verificações para baús de facções específicas
    if (player_faction == chest_faction)
        // Verificações de cargo simplificadas
        var player_rank := GetObjProperty(who, "guild_rank");
        
        if (IsGuildLeader(who) || 
            player_rank in {"Lider", "Capitao", "Leader", "Captain", "Tenente", "Lieutenant"})
            LogInfo("permissao", who.name + " tem cargo de liderança na facção");
            return 1;
        endif
        
        // Verificar permissões especiais
        if (HasSpecialPermission(who, container, "remove"))
            LogInfo("permissao", who.name + " tem permissão especial");
            return 1;
        endif
        
        LogWarning("permissao", who.name + " sem permissão - membro regular da facção");
        return 0;
    endif
    
    // Verificação final de nobreza para acesso a baús de outras facções
    var chardata := GetObjProperty(who, "chardata");
    if (chardata && 
        (chardata.current_nobility_title in {"Rei", "Rainha"} || 
         chardata.current_professional_position in {"Mão do Rei", "Imperador"}))
        LogInfo("permissao", who.name + " tem título real - acesso especial");
        return 1;
    endif
    
    // Negação final de permissão
    LogWarning("permissao", who.name + " sem permissão para remover do baú");
    return 0;
endfunction



function UnifiedCheckOpenPermission(who, container)
    LogDebug("permissao", "Verificando permissão para ABRIR - Player: " + who.name);
    
    // GMs sempre podem abrir
    if (who.cmdlevel >= 3)
        LogInfo("permissao", "GM " + who.name + " tem permissão total");
        return 1;
    endif
    
    // Validação básica de container
    if (!container)
        LogWarning("permissao", "Container não especificado para " + who.name);
        return 0;
    endif
    
    // Obter informações de facção
    var chest_faction := GetFactionOfChest(container);
    var player_faction := GetPlayerFaction(who);
    
    LogDebug("permissao", "Facção do baú: " + chest_faction + ", Facção do jogador: " + player_faction);
    
    // REGRA PRINCIPAL: Facções idênticas
    if (player_faction == chest_faction)
        LogInfo("permissao", who.name + " membro da facção - acesso permitido");
        return 1;
    endif
    
    // EXCEÇÃO 1: Baús do Reino para nobres
    if (chest_faction == KINGDOM_FACTION || chest_faction == "Reino")
        var chardata := GetObjProperty(who, "chardata");
        if (chardata && 
            (chardata.current_nobility_title in {"Rei", "Rainha"} || 
             chardata.current_professional_position in {"Mão do Rei", "Imperador"}))
            LogInfo("permissao", who.name + " tem título real - acesso ao Reino permitido");
            return 1;
        endif
    endif
    
    // EXCEÇÃO 2: Permissões especiais
    if (HasSpecialPermission(who, container, "open") || 
        HasSpecialPermission(who, container, "insert") || 
        HasSpecialPermission(who, container, "remove"))
        LogInfo("permissao", who.name + " tem permissão especial - acesso permitido");
        return 1;
    endif
    
    // EXCEÇÃO 3: Permissões de desenvolvimento
    if (GetObjProperty(who, "AllowFullChestAccess") || 
        GetObjProperty(who, "AllowAllChestAccess"))
        LogInfo("permissao", who.name + " tem permissão de desenvolvimento - acesso permitido");
        return 1;
    endif
    
    // Negação padrão
    LogWarning("permissao", who.name + " sem permissão para abrir baú");
    return 0;
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
    // Validações básicas de entrada
    if (!who || !container)
        return;
    endif
    
    // Usar novo sistema de logging para padronização
    var log_level := result ? LOG_INFO : LOG_WARNING;
    
    // Obter informações com fallback seguro
    var chest_faction := GetFactionOfChest(container) ?: "Desconhecida";
    var player_faction := GetPlayerFaction(who) ?: "Desconhecida";
    var player_rank := GetPlayerRankInGuild(who) ?: "Sem Rank";
    
    // Mensagem de log unificada
    var log_message := $"{who.name} [{player_faction}/{player_rank}] {action} " + 
                       (result ? "PERMITIDO" : "NEGADO") + 
                       $" para baú {container.serial} (Facção: {chest_faction})";
    
    // Log centralizado
    Log(log_level, "permissao", log_message);
    
    // Registro simplificado em datafile
    LogPermissionToDatafile(who, container, action, result, 
                             player_faction, player_rank, chest_faction);
endfunction

/*
 * LogPermissionToDatafile(who, item, action, result, player_faction, player_rank, chest_faction)
 *
 * Registra operações de permissão no datafile para auditoria
 */
function LogPermissionToDatafile(who, item, action, result := 0, player_faction := "", player_rank := "", chest_faction := "")
    // Abrir/criar datafile
    var df := SafeOpenDataFile(":faccao:chest_access_log", 1);
    if (!df)
        LogError("permissao", "Falha ao abrir datafile de log de acesso");
        return 0;
    endif
    
    // Encontrar/criar elemento de logs
    var elem := SafeFindElement(df, "logs", 1);
    if (!elem)
        LogError("permissao", "Falha ao criar elemento de logs");
        return 0;
    endif
    
    // Obter registros existentes com segurança
    var logs := GetElementProp(elem, "entries", array{});
    if (!logs)
        logs := array{};
    endif
    
    // Adicionar novo registro
    logs.append(struct{
        "time" := polcore().systime,
        "character" := who.name,
        "serial" := who.serial,
        "item" := item.desc,
        "item_serial" := item.serial,
        "action" := action,
        "result" := result ? 1 : 0,
        "player_faction" := player_faction,
        "player_rank" := player_rank,
        "chest_faction" := chest_faction
    });
    
    // Limitar o tamanho do histórico
    while (logs.size() > 100)
        logs.erase(1);
    endwhile
    
    // Salvar registros atualizados com verificação
    if (!SetElementProp(elem, "entries", logs))
        LogError("permissao", "Falha ao salvar histórico de acesso");
        return 0;
    endif
    
    return 1;
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
function GetFactionOfChest(chest)
    // Log de início da verificação
    LogDebug("permissao", "Verificando facção do baú " + chest.serial);
    
    // Verificar cache com simplificação
    var cached_faction := GetObjProperty(chest, "cache_faction");
    var last_accessed := GetObjProperty(chest, "last_accessed");
    
    if (cached_faction && 
        last_accessed && 
        (ReadGameClock() - last_accessed < 300))
        LogDebug("permissao", "Usando facção em cache: " + cached_faction);
        return cached_faction;
    endif
    
var faction_sources := array{
    OBJ_FACTION_PROP,  // Propriedade principal
    "FactionName"      // Propriedade legada
};

var faction;
    
    foreach source in faction_sources
        faction := GetObjProperty(chest, source);
        if (faction)
            // Atualizar cache
            SetObjProperty(chest, "cache_faction", faction);
            SetObjProperty(chest, "last_accessed", ReadGameClock());
            
            LogInfo("permissao", "Facção encontrada via " + source + ": " + faction);
            return faction;
        endif
    endforeach
    
    // Valor padrão com log
    LogWarning("permissao", "Não foi possível determinar facção do baú " + chest.serial);
    return KINGDOM_FACTION;
endfunction

/*
 * GetPlayerFaction(who)
 *
 * Purpose: Obter o nome da facção de um jogador de forma segura
 * Parameters:
 *   who - O jogador a ser verificado
 * Return:
 *   Nome da facção ou string vazia se não pertencer a nenhuma facção
 */
function GetPlayerFaction(who)
    if (!who)
        return "";
    endif

    // Verificar se o jogador pertence a uma guilda
    if (!who.guildid)
        // Verificar propriedade alternativa como fallback
        var faction := GetObjProperty(who, "faction");
        if (faction && faction != error)
            return faction;
        endif

        return "";
    endif

    var player_guild;

    // Obter o objeto guild sem try/catch
    // Primeiro método: .guild
    player_guild := who.guild;

    // Se falhar, tentar FindGuild
    if (!player_guild)
        player_guild := FindGuild(who.guildid);
    endif

    if (!player_guild)
        // Se não conseguir obter a guilda, verificar propriedade alternativa
        var faction := GetObjProperty(who, "faction");
        if (faction && faction != error)
            return faction;
        endif

        return "";
    endif

    // Tentar obter o nome da facção/guilda com várias tentativas
    var faction_name := "";

    // Método 1: Verificar propriedade FACTION_PROP_NAME
    faction_name := player_guild.GetProp(FACTION_PROP_NAME);
    if (faction_name && faction_name != error)
        return faction_name;
    endif

    // Método 2: Verificar propriedade "name"
    faction_name := player_guild.GetProp("name");
    if (faction_name && faction_name != error)
        return faction_name;
    endif

    // Método 3: Verificar guildname
    if (player_guild.guildname)
        return player_guild.guildname;
    endif

    // Método 4: Verificar propriedade alternativa como último recurso
    var faction := GetObjProperty(who, "faction");
    if (faction && faction != error)
        return faction;
    endif

    return "";
endfunction

/*
 * IsGuildLeader(who)
 *
 * Purpose: Verifica se o jogador é líder de uma guilda
 * Parameters: who - O jogador a verificar
 * Return: 1 se for líder, 0 caso contrário
 */
function IsGuildLeader(who)
    if (!who)
        return 0;
    endif

    // Método 1: Verificar se tem uma guilda e se é o líder
    var player_guild := who.guild;
    if (player_guild)
        if (player_guild.leader == who.serial)
            return 1;
        endif
    endif

    // Método 2: Verificar por cargo armazenado em propriedade
    var player_rank := GetObjProperty(who, "guild_rank");
    if (player_rank in {"Lider", "Leader"})
        return 1;
    endif

    // Não é líder de guilda
    return 0;
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
 * Funções wrapper para manter compatibilidade com código existente
 * Estas funções apenas redirecionam para CheckPermission()
 */


function CanInsertItems(who, container)
    return CheckPermission(who, container, "insert");
endfunction

function CanRemoveItems(who, container)
    return CheckPermission(who, container, "remove");
endfunction

function CanAccessChest(who, container)
    return CheckPermission(who, container, "access");
endfunction