use uo;
use os;
use cfgfile;
use datafile;

include ":datafile:datafile";
include ":faccao:faccao_constants";
include ":faccao:include/shared_functions";

/*
 * CreateEmptyResourceStruct()
 *
 * Purpose: Cria um struct vazio para recurso com todos os valores zerados
 * Parameters: Nenhum
 * Return: Struct com todos os tipos de recursos zerados
 */
function CreateEmptyResourceStruct()
    var recursos := struct{
        "comida" := 0,
        "moedas" := 0,
        "couro" := 0,
        "metal" := 0,
        "madeira" := 0,
        "pano" := 0,
        "pedras" := 0,
        "joias" := 0
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
    
    Print("DEBUG: Iniciando contagem física de recursos em " + container.desc);
    
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
 * DeterminarTipoRecursoAutomatico(item)
 *
 * Purpose: Tenta determinar automaticamente o tipo de recurso baseado no objtype
 * Parameters: item - O item a analisar
 * Return: String com o tipo de recurso determinado
 */
function DeterminarTipoRecursoAutomatico(item)
    var objtype := item.objtype;
    
    // Categorias comuns de itens
    case (objtype)
        // Comida
        0x097B: // Peixe
        0x097C: // Peixe
        0x09B7: // Carne
        0x09C0: // Carne
        0x09D2: // Queijo
        0x09D0: // Bolo
        0x09EB: // Maçã
            return RESOURCE_TYPE_COMIDA;
            
        // Moedas
        0x0EED: // Moedas de ouro
        0x0EEA: // Moedas de prata
        0x0EE8: // Moedas de cobre
            return RESOURCE_TYPE_MOEDAS;
            
        // Couro
        0x1078: // Couro
        0x1079: // Peles
            return RESOURCE_TYPE_COURO;
            
        // Metal
        0x1BEF: // Lingote de ferro
        0x1BF2: // Lingote de cobre
        0x1BEE: // Lingote de ouro
            return RESOURCE_TYPE_METAL;
            
        // Madeira
        0x1BD7: // Toras
        0x1BD8: // Toras
        0x1BD9: // Toras
            return RESOURCE_TYPE_MADEIRA;
            
        // Pano
        0x1766: // Pano
        0x1767: // Pano
        0x1768: // Pano
            return RESOURCE_TYPE_PANO;
            
        // Pedras
        0x1BDD: // Pedra
        0x1BDC: // Pedra
            return RESOURCE_TYPE_PEDRA;
            
        // Joias
        0x1F13: // Joia
        0x1F15: // Joia
        0x1F11: // Joia
            return RESOURCE_TYPE_JOIAS;
            
        default:
            return RESOURCE_TYPE_COMIDA; // Valor padrão se não conseguir determinar
    endcase
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

/*
 * VerificarInconsistencias(container, recursos_atuais)
 *
 * Purpose: Compara a contagem atual com o cache para detectar inconsistências
 * Parameters: 
 *   container - O baú sendo verificado
 *   recursos_atuais - A contagem atual de recursos
 * Return: 1 se detectou inconsistências, 0 caso contrário
 */
function VerificarInconsistencias(container, recursos_atuais)
    var recursos_cache := GetObjProperty(container, CACHE_PROP_NAME);
    if (!recursos_cache)
        return 0; // Não tem cache para comparar
    endif
    
    var diferenca := 0;
    
    // Comparar cada tipo de recurso
    if (recursos_cache.comida != recursos_atuais.comida)
        Print("DEBUG: Inconsistência em comida: " + recursos_cache.comida + " vs " + recursos_atuais.comida);
        diferenca := 1;
    endif
    
    if (recursos_cache.moedas != recursos_atuais.moedas)
        Print("DEBUG: Inconsistência em moedas: " + recursos_cache.moedas + " vs " + recursos_atuais.moedas);
        diferenca := 1;
    endif
    
    if (recursos_cache.couro != recursos_atuais.couro)
        Print("DEBUG: Inconsistência em couro: " + recursos_cache.couro + " vs " + recursos_atuais.couro);
        diferenca := 1;
    endif
    
    if (recursos_cache.metal != recursos_atuais.metal)
        Print("DEBUG: Inconsistência em metal: " + recursos_cache.metal + " vs " + recursos_atuais.metal);
        diferenca := 1;
    endif
    
    if (recursos_cache.madeira != recursos_atuais.madeira)
        Print("DEBUG: Inconsistência em madeira: " + recursos_cache.madeira + " vs " + recursos_atuais.madeira);
        diferenca := 1;
    endif
    
    if (recursos_cache.pano != recursos_atuais.pano)
        Print("DEBUG: Inconsistência em pano: " + recursos_cache.pano + " vs " + recursos_atuais.pano);
        diferenca := 1;
    endif
    
    if (recursos_cache.pedras != recursos_atuais.pedras)
        Print("DEBUG: Inconsistência em pedras: " + recursos_cache.pedras + " vs " + recursos_atuais.pedras);
        diferenca := 1;
    endif
    
    if (recursos_cache.joias != recursos_atuais.joias)
        Print("DEBUG: Inconsistência em joias: " + recursos_cache.joias + " vs " + recursos_atuais.joias);
        diferenca := 1;
    endif
    
    // Registrar inconsistência se encontrada
    if (diferenca)
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
    endif
    
    return diferenca;
endfunction

function SincronizarComDataFile(container, recursos)
    if (!container || !recursos)
        Print("ERRO: Parâmetros inválidos para sincronização");
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
        Print("ERRO: Não foi possível determinar a facção para sincronização");
        faction_name := DEFAULT_FACTION; // Usar valor padrão se não encontrar
    endif
    
    // Registrar início da sincronização
    Print("DEBUG: Iniciando sincronização de recursos para facção: " + faction_name);
    
    // Verificar se os recursos são válidos antes de salvar
    recursos := ValidarRecursos(recursos);
    if (!recursos)
        Print("ERRO: Recursos inválidos após validação");
        return 0;
    endif
    
    // SINCRONIZAÇÃO 1: Datafile global
    var df_success := AtualizarDatafileRecursos(faction_name, recursos);
    
    // SINCRONIZAÇÃO 2: Propriedades da guilda
    var guild_success := AtualizarPropriedadesGuildaRecursos(faction_name, recursos);
    
    // Registrar resultado
    Print("DEBUG: Sincronização concluída - Datafile: " + (df_success ? "Sucesso" : "Falha") + 
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
    
    var df := OpenDataFile(KINGDOM_RESOURCE_DATAFILE);
    if (df == error)
        return CreateEmptyResourceStruct();
    endif
    
    var elem := DFFindElement(df, faction_name);
    if (elem == error)
        return CreateEmptyResourceStruct();
    endif
    
    var recursos := elem.GetProp(KINGDOM_RESOURCE_ELEM);
    if (!recursos)
        return CreateEmptyResourceStruct();
    endif
    
    return recursos;
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
 * AtualizarRecursosFaccao(faction_name, recursos)
 *
 * Purpose: Atualiza os recursos de uma facção no datafile global
 * Parameters: 
 *   faction_name - Nome da facção
 *   recursos - Struct com os recursos a serem atualizados
 * Return: 1 para sucesso, 0 para falha
 */
function AtualizarRecursosFaccao(faction_name, recursos)
    var df := OpenDataFile(KINGDOM_RESOURCE_DATAFILE);
    if (df == error)
        df := CreateDataFile(KINGDOM_RESOURCE_DATAFILE);
        if (df == error)
            Print("ERROR: Não foi possível criar o datafile de recursos");
            return 0;
        endif
    endif
    
    var elem := DFFindElement(df, faction_name, DF_CREATE);
    if (elem == error)
        Print("ERROR: Não foi possível criar/encontrar o elemento para " + faction_name);
        return 0;
    endif
    
    elem.SetProp(KINGDOM_RESOURCE_ELEM, recursos);
    return 1;
endfunction


// ==========================================================================
// FUNÇÕES PARA CLASSIFICAÇÃO DE RECURSOS
// ==========================================================================

// Uma função unificada para classificar recursos em categorias padronizadas
// Chama GetResourceCategory em ResourceController.src, mas adiciona validação
function ClassificarRecurso(resource)
    // Verificar o tipo do parâmetro
    if (!resource)
        return "desconhecido";
    endif
    
    // Se for um item/objeto
    if (TypeOf(resource) == "ItemRef")
        // Verificar primeiro por propriedades explícitas
        if (GetObjProperty(resource, "IsKingdomDonationComida"))
            return "comida";
        elseif (GetObjProperty(resource, "IsKingdomDonationMoedas"))
            return "moedas";
        elseif (GetObjProperty(resource, "IsKingdomDonationCouro"))
            return "couro";
        elseif (GetObjProperty(resource, "IsKingdomDonationMetal"))
            return "metal";
        elseif (GetObjProperty(resource, "IsKingdomDonationMadeira"))
            return "madeira";
        elseif (GetObjProperty(resource, "IsKingdomDonationPano"))
            return "pano";
        elseif (GetObjProperty(resource, "IsKingdomDonationPedra"))
            return "pedras";
        elseif (GetObjProperty(resource, "IsKingdomDonationJoias"))
            return "joias";
        endif
        
        // Se não tem propriedade explícita, usar a categorização pelo objtype
        return CategoriaRecursoPorObjtype(resource.objtype);
    endif
    
    // Se for uma string
    if (TypeOf(resource) == "String")
        // Usar a categorização baseada em nome
        return CategoriaRecursoPorNome(resource);
    endif
    
    // Se for um número (objtype direto)
    if (TypeOf(resource) == "Integer")
        return CategoriaRecursoPorObjtype(resource);
    endif
    
    // Não conseguimos classificar
    return "desconhecido";
endfunction

// Função para categorizar recursos pelo nome
function CategoriaRecursoPorNome(resource_name)
    // Normalizar o nome do recurso
    resource_name := Trim(Lower(resource_name));
    
    // Mapeamento extenso de nomes para categorias
    // Inclui várias formas de escrever o mesmo recurso
    var category_map := dictionary{
        // Metais
        "ironore"           := "metal",
        "iron ore"          := "metal",
        "ferro"             := "metal",
        "minério de ferro"  := "metal",
        "prataingot"        := "metal",
        "prata"             := "metal",
        "silver"            := "metal",
        "acoingot"          := "metal",
        "aço"               := "metal",
        "steel"             := "metal",
        "cobre"             := "metal",
        "copper"            := "metal",
        "bronze"            := "metal",
        "ouro"              := "metal",
        "gold"              := "metal",
        "metal"             := "metal",
        "minério"           := "metal",
        "ore"               := "metal",
        "lingote"           := "metal",
        "ingot"             := "metal",
        
        // Couro
        "couroespesso"      := "couro",
        "couro espesso"     := "couro",
        "couropolar"        := "couro",
        "couro polar"       := "couro",
        "courodraconiano"   := "couro",
        "couro draconiano"  := "couro",
        "couroescamoso"     := "couro",
        "couro escamoso"    := "couro",
        "couro"             := "couro",
        "leather"           := "couro",
        "hide"              := "couro",
        "pele"              := "couro",
        "skin"              := "couro",
        
        // Madeira
        "madeirapinheiro"   := "madeira",
        "madeira pinheiro"  := "madeira",
        "madeirateixo"      := "madeira",
        "madeira teixo"     := "madeira",
        "madeira"           := "madeira",
        "wood"              := "madeira",
        "pinewood"          := "madeira",
        "yewwood"           := "madeira",
        "log"               := "madeira",
        "tora"              := "madeira",
        "oak"               := "madeira",
        "carvalho"          := "madeira",
        "carvao"            := "madeira",
        "carvão"            := "madeira",
        "coal"              := "madeira",
        "lumber"            := "madeira",
        
        // Comida
        "cogumelo"          := "comida",
        "mushroom"          := "comida",
        "peixe"             := "comida",
        "fish"              := "comida",
        "carne"             := "comida",
        "meat"              := "comida",
        "fruta"             := "comida",
        "fruit"             := "comida",
        "vegetal"           := "comida",
        "vegetable"         := "comida",
        "pão"               := "comida",
        "bread"             := "comida",
        "cereal"            := "comida",
        "bolo"              := "comida",
        "cake"              := "comida",
        "food"              := "comida",
        "comida"            := "comida",
        "alimento"          := "comida",
        
        // Tecidos/Panos
        "pelebranca"        := "pano",
        "pele branca"       := "pano",
        "escamasazuis"      := "pano",
        "escamas azuis"     := "pano",
        "escamas"           := "pano",
        "scales"            := "pano",
        "cloth"             := "pano",
        "tecido"            := "pano",
        "pano"              := "pano",
        "silk"              := "pano",
        "seda"              := "pano",
        "cotton"            := "pano",
        "algodão"           := "pano",
        "wool"              := "pano",
        "lã"                := "pano",
        "fabric"            := "pano",
        
        // Pedras
        "ossogigante"       := "pedras",
        "osso gigante"      := "pedras",
        "ossodemoniaco"     := "pedras",
        "osso demoniaco"    := "pedras",
        "osso"              := "pedras",
        "bone"              := "pedras",
        "stone"             := "pedras",
        "pedra"             := "pedras",
        "rock"              := "pedras",
        "rocha"             := "pedras",
        "granite"           := "pedras",
        "granito"           := "pedras",
        "marble"            := "pedras",
        "mármore"           := "pedras",
        "sand"              := "pedras",
        "areia"             := "pedras",
        
        // Joias
        "vidro"             := "joias",
        "glass"             := "joias",
        "gem"               := "joias",
        "gema"              := "joias",
        "joia"              := "joias",
        "jewel"             := "joias",
        "ruby"              := "joias",
        "rubi"              := "joias",
        "diamond"           := "joias",
        "diamante"          := "joias",
        "pearl"             := "joias",
        "pérola"            := "joias",
        "emerald"           := "joias",
        "esmeralda"         := "joias",
        "sapphire"          := "joias",
        "safira"            := "joias",
        "crystal"           := "joias",
        "cristal"           := "joias",
        
        // Moedas
        "moeda"             := "moedas",
        "coin"              := "moedas",
        "copper coin"       := "moedas",
        "silver coin"       := "moedas",
        "gold coin"         := "moedas",
        "moeda de cobre"    := "moedas",
        "moeda de prata"    := "moedas",
        "moeda de ouro"     := "moedas",
        "dinheiro"          := "moedas",
        "money"             := "moedas",
        "currency"          := "moedas"
    };
    
    // Primeiro verificar correspondência direta
    if (category_map.exists(resource_name))
        return category_map[resource_name];
    endif
    
    // Tentar correspondência parcial - verificando palavras-chave na string
    foreach key in (category_map.keys())
        if (resource_name.find(key) != error)
            return category_map[key];
        endif
        sleepms(1);
    endforeach
    
    // Categorização por sufixos/prefixos comuns
    if (resource_name.find("ore") != error || resource_name.find("ingot") != error)
        return "metal";
    elseif (resource_name.find("log") != error || resource_name.find("wood") != error)
        return "madeira";
    elseif (resource_name.find("hide") != error || resource_name.find("leather") != error)
        return "couro";
    endif
    
    // Não conseguimos categorizar pelo nome
    return "desconhecido";
endfunction

// Função para categorizar recursos pelo objtype
function CategoriaRecursoPorObjtype(objtype)
    // Mapeamento detalhado de objtypes para categorias
    var objtype_map := dictionary{
        // Metais
        0x6300 := "metal",     // Iron ore
        0x6993 := "metal",     // Silver ingot
        0x6995 := "metal",     // Steel ingot
        0x1BEF := "metal",     // Iron ingot
        0x1BF2 := "metal",     // Copper ingot
        0x1BEE := "metal",     // Gold ingot
        
        // Couro
        0xef12 := "couro",     // Couro espesso
        0xef17 := "couro",     // Couro polar
        0xed20 := "couro",     // Couro draconiano
        0xef11 := "couro",     // Couro escamoso
        0x1078 := "couro",     // Couro comum
        0x1079 := "couro",     // Peles
        
        // Madeira
        0xee36 := "madeira",   // Madeira de pinheiro
        0xee3e := "madeira",   // Madeira de teixo
        0x1bde := "madeira",   // Carvão
        0x1BD7 := "madeira",   // Toras
        0x1BD8 := "madeira",   // Toras
        0x1BD9 := "madeira",   // Toras
        
        // Comida
        0x0D16 := "comida",    // Cogumelo
        0x097B := "comida",    // Peixe
        0x097C := "comida",    // Peixe
        0x09B7 := "comida",    // Carne
        0x09C0 := "comida",    // Carne
        0x09D2 := "comida",    // Queijo
        0x09D0 := "comida",    // Bolo
        0x09EB := "comida",    // Maçã
        
        // Tecidos/Panos
        0xee52 := "pano",      // Pele branca
        0xef36 := "pano",      // Escamas azuis
        0x1766 := "pano",      // Pano
        0x1767 := "pano",      // Pano
        0x1768 := "pano",      // Pano
        
        // Pedras
        0xef3A := "pedras",    // Osso gigante
        0xef29 := "pedras",    // Osso demoníaco
        0x0F7E := "pedras",    // Osso comum
        0x1BDD := "pedras",    // Pedra
        0x1BDC := "pedras",    // Pedra
        
        // Joias
        0x1BE3 := "joias",     // Vidro
        0x1F13 := "joias",     // Joia
        0x1F15 := "joias",     // Joia
        0x1F11 := "joias",     // Joia
        
        // Moedas
        0x0EED := "moedas",    // Moedas de ouro
        0x0EEA := "moedas",    // Moedas de prata
        0x0EE8 := "moedas",    // Moedas de cobre
        0xBA63 := "moedas",    // Moedas novas
        0xBA64 := "moedas",    // Moedas novas
        0xBA65 := "moedas"     // Moedas novas
    };
    
    // Verificar mapeamento direto
    if (objtype_map.exists(objtype))
        return objtype_map[objtype];
    endif
    
    // Categorização por faixas de objtype (baseado em tendências observadas)
    if (objtype >= 0x1BEE && objtype <= 0x1BF5)
        return "metal"; // Faixa de ingots
    elseif (objtype >= 0x1078 && objtype <= 0x1079)
        return "couro"; // Faixa de couros
    elseif (objtype >= 0x1BD7 && objtype <= 0x1BDD)
        return "madeira"; // Faixa de madeiras
    elseif (objtype >= 0x097B && objtype <= 0x09EB)
        return "comida"; // Faixa de comidas
    elseif (objtype >= 0x1766 && objtype <= 0x1768)
        return "pano"; // Faixa de panos
    elseif (objtype >= 0x1F11 && objtype <= 0x1F15)
        return "joias"; // Faixa de joias
    elseif (objtype >= 0x0EE8 && objtype <= 0x0EED)
        return "moedas"; // Faixa de moedas antigas
    elseif (objtype >= 0xBA63 && objtype <= 0xBA65)
        return "moedas"; // Faixa de moedas novas
    endif
    
    // Não conseguimos categorizar pelo objtype
    return "desconhecido";
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
    
    Print("Recurso: " + resource + " => Categoria: " + categoria);
    
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
                Print("AVISO: Falha ao aumentar quantidade diretamente. Tentando método alternativo.");
            endif
        endif
    endif
    
    // Se não conseguimos empilhar em um item existente ou temos quantidade restante
    // Criar um novo item
    var new_item := CreateItemInContainer(container, objtype, amount);
    
    // Verificar se a criação foi bem-sucedida
    if (!new_item)
        Print("ERRO: Falha ao criar novo item para empilhamento");
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


// Tenta aumentar a quantidade diretamente
function TryIncreaseAmount(item, amount)
    if (!item || amount <= 0)
        return 0;
    endif
    
    var original_amount := item.amount;
    var new_amount := original_amount + amount;
    
    // Verificar limites
    if (new_amount > 60000)
        new_amount := 60000;
        amount := 60000 - original_amount;
    endif
    
    // Tentar aplicar a nova quantidade
    item.amount := new_amount;
    
    // Verificar se funcionou
    if (item.amount == new_amount)
        return amount; // Retorna a quantidade efetivamente adicionada
    else
        // Tentar restaurar valor original em caso de falha
        item.amount := original_amount;
        return 0;
    endif
endfunction

// Função para mesclar propriedades entre itens
function MergeItemProperties(item, properties)
    if (!item || !properties)
        return 0;
    endif
    
    var merged := 0;
    
    // Se propriedades é um dicionário/struct, aplicar diretamente
    if (TypeOf(properties) == "Dictionary" || TypeOf(properties) == "Struct")
        foreach prop_name in (properties.keys())
            SetObjProperty(item, prop_name, properties[prop_name]);
            merged += 1;
            sleepms(1);
        endforeach
    endif
    
    // Se propriedades é outro item, copiar suas propriedades
    if (TypeOf(properties) == "ItemRef")
        var source_item := properties;
        var prop_names := GetObjPropertyNames(source_item);
        
        foreach prop_name in prop_names
            var value := GetObjProperty(source_item, prop_name);
            SetObjProperty(item, prop_name, value);
            merged += 1;
            sleepms(1);
        endforeach
    endif
    
    return merged;
endfunction

// Função para aplicar propriedades a um item
function ApplyPropertiesToItem(item, properties)
    if (!item)
        return 0;
    endif
    
    return MergeItemProperties(item, properties);
endfunction


// Função para substituir um item existente por um novo com quantidade acumulada
function ReplaceStackWithNew(old_item, amount_to_add, properties := 0)
    if (!old_item)
        return 0;
    endif
    
    var container := old_item.container;
    if (!container)
        return 0;
    endif
    
    Print("DEBUG: Tentando substituir item " + old_item.serial + " com qtd adicional " + amount_to_add);
    
    var objtype := old_item.objtype;
    var old_amount := old_item.amount;
    var total_amount := old_amount + amount_to_add;
    
    // Verificar limite
    if (total_amount > 60000)
        total_amount := 60000;
    endif
    
    // Salvar propriedades do item antigo
    var old_properties := dictionary{};
    var prop_names := GetObjPropertyNames(old_item);
    
    foreach prop_name in prop_names
        old_properties[prop_name] := GetObjProperty(old_item, prop_name);
        sleepms(1);
    endforeach
    
    // Criar novo item com quantidade total
    var new_item := CreateItemInContainer(container, objtype, total_amount);
    if (!new_item)
        Print("ERRO: Falha ao criar novo item para substituição");
        return 0;
    endif
    
    // Transferir propriedades antigas
    foreach prop_name in (old_properties.keys())
        SetObjProperty(new_item, prop_name, old_properties[prop_name]);
        sleepms(1);
    endforeach
    
    // Adicionar novas propriedades
    if (properties)
        foreach prop_name in (properties.keys())
            SetObjProperty(new_item, prop_name, properties[prop_name]);
            sleepms(1);
        endforeach
    endif
    
    // Destruir item antigo
    DestroyItem(old_item);
    
    Print("DEBUG: Item substituído com sucesso, nova quantidade: " + new_item.amount);
    return new_item;
endfunction