// :faccao:construcao/include/placa_controller.inc
//
// CONTROLADOR CENTRAL PARA TODAS AS PLACAS DE CONSTRUÇÃO
// Sistema escalável que gerencia automaticamente todos os tipos de construção
//
// Versão 1.0 - Agosto 2025

use uo;
use os;
use guilds;

include ":faccao:include/logging";
include ":faccao:include/permissions";
include ":faccao:include/safe_guild_utils";
include ":faccao:construcao/include/feudo_buildings";

// ========================================================================
// CONSTANTES DO SISTEMA
// ========================================================================

// Níveis de acesso para cada tipo de usuário
const ACCESS_LEVEL_OWNER      := 4;  // Líder da guilda proprietária
const ACCESS_LEVEL_RENTER     := 3;  // Inquilino com contrato
const ACCESS_LEVEL_CITIZEN    := 2;  // Cidadão da guilda proprietária  
const ACCESS_LEVEL_OUTSIDER   := 1;  // Forasteiro

// ========================================================================
// FUNÇÕES UTILITÁRIAS PARA MAPEAMENTO DE TIPOS
// ========================================================================

/*
 * PLACA_GetBuildingInfo(building_type)
 * 
 * Retorna todas as informações de uma construção (módulo e nome de exibição).
 * Esta função centraliza todos os mapeamentos para evitar duplicação.
 */
function PLACA_GetBuildingInfo(building_type)
    var info := struct{};
    var type_key := Lower(building_type);
    
    // Definir informações para cada tipo de construção
    case (type_key)
        // === NÍVEL 1 (BÁSICO) ===
        "casa":
            info.+module_name := "Casa";
            info.+display_name := "Casa";
        "lotedefazenda":
            info.+module_name := "LoteDeFazenda";
            info.+display_name := "Lote de Fazenda";
        "casadolenhador":
            info.+module_name := "CasaDoLenhador";
            info.+display_name := "Casa do Lenhador";
        "pocodemineracao":
            info.+module_name := "PocoMineracao";
            info.+display_name := "Poço de Mineração";
        "palicada":
            info.+module_name := "Palicada";
            info.+display_name := "Paliçada de Madeira";
            
        // === NÍVEL 2+ (FUTUROS) ===
        "biblioteca":
            info.+module_name := "Biblioteca";
            info.+display_name := "Biblioteca";
        "ferreiro":
            info.+module_name := "Ferreiro";
            info.+display_name := "Ferreiro";
        "marcenaria":
            info.+module_name := "Marcenaria";
            info.+display_name := "Marcenaria";
        "taverna":
            info.+module_name := "Taverna";
            info.+display_name := "Taverna";
        "salaoconselh":
            info.+module_name := "SalaoConselho";
            info.+display_name := "Salão do Conselho";
        "quartel":
            info.+module_name := "Quartel";
            info.+display_name := "Quartel";
        "hospital":
            info.+module_name := "Hospital";
            info.+display_name := "Hospital";
        "labalquimia":
            info.+module_name := "LabAlquimia";
            info.+display_name := "Laboratório de Alquimia";
        "mercado":
            info.+module_name := "Mercado";
            info.+display_name := "Mercado";
        "oficinacerco":
            info.+module_name := "OficinaCerco";
            info.+display_name := "Oficina de Cerco";
        "torremagica":
            info.+module_name := "TorreMagica";
            info.+display_name := "Torre Mágica";
        "arena":
            info.+module_name := "Arena";
            info.+display_name := "Arena";
        "prisao":
            info.+module_name := "Prisao";
            info.+display_name := "Prisão";
        "casacomercio":
            info.+module_name := "CasaComercio";
            info.+display_name := "Casa de Comércio";
        "observatorio":
            info.+module_name := "Observatorio";
            info.+display_name := "Observatório";
        "monumento":
            info.+module_name := "Monumento";
            info.+display_name := "Monumento";
        default:
            info.+module_name := "";
            info.+display_name := building_type;
    endcase
    
    return info;
endfunction

/*
 * PLACA_GetModuleName(building_type)
 * 
 * Função legada mantida para compatibilidade.
 */
function PLACA_GetModuleName(building_type)
    var info := PLACA_GetBuildingInfo(building_type);
    return info.module_name;
endfunction

// ========================================================================
// FUNÇÃO PRINCIPAL DO CONTROLADOR
// ========================================================================

/*
 * PLACA_ProcessUsage(who, placa)
 * 
 * Função principal que detecta o tipo de construção e roteia para o módulo apropriado.
 * Esta é a única função que deve ser chamada externamente.
 */
function PLACA_ProcessUsage(who, placa)
    var LOG_CATEGORY := "PlacaController";
    LogCritical(LOG_CATEGORY, "=== PROCESSAMENTO DE PLACA INICIADO ===");
    LogCritical(LOG_CATEGORY, "Player: " + who.name + " (Serial: " + who.serial + ")");
    LogCritical(LOG_CATEGORY, "Placa: Serial " + placa.serial + ", ObjType: 0x" + Hex(placa.objtype));
    
    // === ETAPA 1: VALIDAÇÃO INICIAL ===
    if (!who || !placa)
        LogCritical(LOG_CATEGORY, "ERRO CRÍTICO: Parâmetros inválidos - who ou placa nulos");
        return 0;
    endif
    
    // === ETAPA 2: OBTER INFORMAÇÕES DA CONSTRUÇÃO ===
    var build_info := GetObjProperty(placa, "BuildingInfo");
    if (!build_info)
        LogCritical(LOG_CATEGORY, "ERRO CRÍTICO: Placa " + placa.serial + " sem BuildingInfo!");
        LogCritical(LOG_CATEGORY, "Esta placa foi criada mas não recebeu os dados de construção!");
        SendSysMessage(who, "Esta placa está corrompida (dados de construção ausentes).");
        return 0;
    endif
    
    var building_type := Lower(build_info.type);
    LogCritical(LOG_CATEGORY, "Tipo detectado: '" + building_type + "' (Nível: " + build_info.level + ")");
    
    // === ETAPA 3: DETERMINAR NÍVEL DE ACESSO ===
    var access_level := PLACA_GetAccessLevel(who, placa);
    LogCritical(LOG_CATEGORY, "Nível de acesso determinado: " + access_level);
    
    // === ETAPA 4: ROTEAMENTO PARA MÓDULO ESPECÍFICO ===
    var module_name := PLACA_GetModuleName(building_type);
    if (!module_name)
        LogCritical(LOG_CATEGORY, "ERRO CRÍTICO: Tipo de construção desconhecido: '" + building_type + "'");
        LogCritical(LOG_CATEGORY, "Tipos válidos: casa, lotedefazenda, casadolenhador, etc.");
        SendSysMessage(who, "Tipo de construção não reconhecido: " + building_type);
        SendSysMessage(who, "Contate um administrador sobre este erro.");
        return 0;
    endif
    LogCritical(LOG_CATEGORY, "Redirecionando para módulo: " + module_name);
    
    // Carregar e executar o módulo específico
    LogCritical(LOG_CATEGORY, "Chamando PLACA_LoadAndExecuteModule...");
    var result := PLACA_LoadAndExecuteModule(module_name, who, placa, access_level);
    LogCritical(LOG_CATEGORY, "PLACA_LoadAndExecuteModule retornou: " + result);
    
    LogCritical(LOG_CATEGORY, "=== PROCESSAMENTO CONCLUÍDO ===");
    return result;
endfunction

// ========================================================================
// FUNÇÕES DE APOIO
// ========================================================================

/*
 * PLACA_GetAccessLevel(who, placa)
 * 
 * Determina o nível de acesso do jogador em relação à construção.
 */
function PLACA_GetAccessLevel(who, placa)
    var owner_guild_id := GetObjProperty(placa, "GuildOwner");
    var contrato := GetObjProperty(placa, "ContratoAluguel");
    
    // Líder da guilda proprietária ou Admin
    if (who.guildid == owner_guild_id && (IsGuildLeader(who) || who.cmdlevel >= 1))
        return ACCESS_LEVEL_OWNER;
    endif
    
    // Inquilino com contrato ativo
    if (contrato && CInt(contrato.InquilinoSerial) == who.serial)
        return ACCESS_LEVEL_RENTER;
    endif
    
    // Cidadão da guilda proprietária
    if (who.guildid == owner_guild_id)
        return ACCESS_LEVEL_CITIZEN;
    endif
    
    // Forasteiro
    return ACCESS_LEVEL_OUTSIDER;
endfunction

/*
 * PLACA_LoadAndExecuteModule(module_name, who, placa, access_level)
 * 
 * Carrega dinamicamente o módulo específico e executa sua função principal.
 */
function PLACA_LoadAndExecuteModule(module_name, who, placa, access_level)
    var LOG_CATEGORY := "PlacaController";
    LogCritical(LOG_CATEGORY, "=== PLACA_LoadAndExecuteModule INICIADO ===");
    LogCritical(LOG_CATEGORY, "Module: " + module_name);
    LogCritical(LOG_CATEGORY, "Access Level: " + access_level);
    
    // Tenta carregar o módulo específico
    var module_path := ":faccao:construcao/include/construcoes/" + Lower(module_name);
    LogCritical(LOG_CATEGORY, "Tentando carregar módulo: " + module_path);
    
    // Verifica se o módulo existe tentando incluí-lo
    var module_exists := 1;
    
    // Por enquanto, vamos usar um sistema de fallback baseado em tipos conhecidos
    LogCritical(LOG_CATEGORY, "Verificando tipo de módulo: '" + Lower(module_name) + "'");
    case (Lower(module_name))
        "casa":
            LogCritical(LOG_CATEGORY, "CASA detectada - chamando script casa_uso existente");
            return start_script(":faccao:construcao/construcoes_todas/casa_uso", array{who, placa});
            
        "lotedefazenda":
            LogCritical(LOG_CATEGORY, "FAZENDA detectada - chamando script fazenda_uso existente");
            return start_script(":faccao:construcao/construcoes_todas/fazenda_uso", array{who, placa});
            
        "casadolenhador":
            LogCritical(LOG_CATEGORY, "LENHADOR detectada - chamando PLACA_ExecuteLenhador");
            return PLACA_ExecuteLenhador(who, placa, access_level);
            
        "pocomineracao":
            LogCritical(LOG_CATEGORY, "MINERAÇÃO detectada - chamando PLACA_ExecuteMineracao");
            return PLACA_ExecuteMineracao(who, placa, access_level);
            
        "palicada":
            LogCritical(LOG_CATEGORY, "PALIÇADA detectada - chamando PLACA_ExecutePalicada");
            return PLACA_ExecutePalicada(who, placa, access_level);
            
        default:
            LogCritical(LOG_CATEGORY, "ERRO: Módulo ainda não implementado: " + module_name);
            SendSysMessage(who, "Este tipo de construção ainda não foi implementado.");
            SendSysMessage(who, "Módulo necessário: " + module_name);
            return 0;
    endcase
endfunction

// ========================================================================
// FUNÇÕES DE EXECUÇÃO ESPECÍFICAS (TEMPORÁRIAS - SERÃO MOVIDAS)
// ========================================================================

/*
 * PLACA_ExecuteCasa(who, placa, access_level)
 * 
 * Executa a lógica específica para casas.
 * NOTA: Esta função será movida para o módulo específico casa.inc
 */
function PLACA_ExecuteCasa(who, placa, access_level)
    LogCritical("PlacaCasa", "=== PLACA_ExecuteCasa INICIADO ===");
    LogCritical("PlacaCasa", "Player: " + who.name);
    LogCritical("PlacaCasa", "Access Level: " + access_level);
    
    // ABORDAGEM SIMPLIFICADA: Executar diretamente a lógica da casa aqui
    LogCritical("PlacaCasa", "Verificando permissões...");
    
    var owner_guild_id := GetObjProperty(placa, "GuildOwner");
    var contrato := GetObjProperty(placa, "ContratoAluguel");
    
    LogCritical("PlacaCasa", "GuildOwner: " + owner_guild_id + ", Player Guild: " + who.guildid);
    
    if (who.guildid == owner_guild_id && (IsGuildLeader(who) || who.cmdlevel >= 1))
        LogCritical("PlacaCasa", "Player é líder - redirecionando para casa_uso.src...");
        // Por enquanto, executar o script diretamente com run_script_to_completion
        var script_ref := start_script(":faccao:construcao/construcoes_todas/casa_uso", array{who, placa});
        if (script_ref.errortext)
            LogCritical("PlacaCasa", "ERRO start_script: " + script_ref.errortext);
            // Fallback: exibir mensagem simples
            SendSysMessage(who, "🏠 Gestão da Casa (Líder)");
            SendSysMessage(who, "Sistema temporariamente indisponível - tente novamente.");
        endif
        return script_ref;
    elseif (contrato && CInt(contrato.InquilinoSerial) == who.serial)
        LogCritical("PlacaCasa", "Player é inquilino");
        SendSysMessage(who, "🏠 Casa alugada - funcionalidade de inquilino em desenvolvimento");
        return 1;
    elseif (who.guildid == owner_guild_id)
        LogCritical("PlacaCasa", "Player é cidadão");
        SendSysMessage(who, "🏠 Casa da nossa guilda");
        SendSysMessage(who, "PopCap +8 para o feudo");
        return 1;
    else
        LogCritical("PlacaCasa", "Player é forasteiro");
        SendSysMessage(who, "🏠 Casa Simples");
        SendSysMessage(who, "Uma residência básica.");
        return 1;
    endif
endfunction

/*
 * PLACA_ExecuteFazenda(who, placa, access_level)
 * 
 * Executa a lógica específica para fazendas.
 * NOTA: Esta função será movida para o módulo específico fazenda.inc
 */
function PLACA_ExecuteFazenda(who, placa, access_level)
    LogInfo("PlacaFazenda", "Executando lógica de fazenda para access_level: " + access_level);
    
    // Por enquanto, chama o script existente
    return start_script(":faccao:construcao/construcoes_todas/fazenda_uso", array{who, placa});
endfunction

/*
 * PLACA_ExecuteLenhador(who, placa, access_level)
 * 
 * Executa a lógica específica para casa do lenhador.
 */
function PLACA_ExecuteLenhador(who, placa, access_level)
    LogInfo("PlacaLenhador", "Casa do Lenhador - access_level: " + access_level);
    
    case (access_level)
        ACCESS_LEVEL_OWNER:
            SendSysMessage(who, "🪓 Gestão da Casa do Lenhador (Líder)");
            SendSysMessage(who, "Esta funcionalidade ainda será implementada.");
            break;
            
        ACCESS_LEVEL_CITIZEN:
            SendSysMessage(who, "🪓 Casa do Lenhador do nosso feudo");
            SendSysMessage(who, "Produção de madeira aumentada para o feudo.");
            break;
            
        default:
            SendSysMessage(who, "🪓 Casa do Lenhador");
            SendSysMessage(who, "Uma oficina especializada em trabalhos com madeira.");
            break;
    endcase
    
    return 1;
endfunction

/*
 * PLACA_ExecuteMineracao(who, placa, access_level)
 * 
 * Executa a lógica específica para poço de mineração.
 */
function PLACA_ExecuteMineracao(who, placa, access_level)
    LogInfo("PlacaMineracao", "Poço de Mineração - access_level: " + access_level);
    
    case (access_level)
        ACCESS_LEVEL_OWNER:
            SendSysMessage(who, "⛏️ Gestão do Poço de Mineração (Líder)");
            SendSysMessage(who, "Esta funcionalidade ainda será implementada.");
            break;
            
        ACCESS_LEVEL_CITIZEN:
            SendSysMessage(who, "⛏️ Poço de Mineração do nosso feudo");
            SendSysMessage(who, "Produção de minerais aumentada para o feudo.");
            break;
            
        default:
            SendSysMessage(who, "⛏️ Poço de Mineração");
            SendSysMessage(who, "Uma instalação especializada em extração mineral.");
            break;
    endcase
    
    return 1;
endfunction

/*
 * PLACA_ExecutePalicada(who, placa, access_level)
 * 
 * Executa a lógica específica para paliçada.
 */
function PLACA_ExecutePalicada(who, placa, access_level)
    LogInfo("PlacaPalicada", "Paliçada - access_level: " + access_level);
    
    case (access_level)
        ACCESS_LEVEL_OWNER:
            SendSysMessage(who, "🛡️ Gestão da Paliçada (Líder)");
            SendSysMessage(who, "Esta funcionalidade ainda será implementada.");
            break;
            
        ACCESS_LEVEL_CITIZEN:
            SendSysMessage(who, "🛡️ Paliçada defensiva do nosso feudo");
            SendSysMessage(who, "Aumenta a defesa do território.");
            break;
            
        default:
            SendSysMessage(who, "🛡️ Paliçada Defensiva");
            SendSysMessage(who, "Uma fortificação de madeira para proteção.");
            break;
    endcase
    
    return 1;
endfunction

// ========================================================================
// FUNÇÕES UTILITÁRIAS
// ========================================================================

/*
 * PLACA_GetBuildingDisplayName(building_type)
 * 
 * Função legada mantida para compatibilidade.
 */
function PLACA_GetBuildingDisplayName(building_type)
    var info := PLACA_GetBuildingInfo(building_type);
    return info.display_name ? info.display_name : building_type;
endfunction

/*
 * PLACA_GetAccessLevelName(access_level)
 * 
 * Retorna o nome de exibição para um nível de acesso.
 */
function PLACA_GetAccessLevelName(access_level)
    case (access_level)
        ACCESS_LEVEL_OWNER:   return "Proprietário";
        ACCESS_LEVEL_RENTER:  return "Inquilino";
        ACCESS_LEVEL_CITIZEN: return "Cidadão";
        ACCESS_LEVEL_OUTSIDER: return "Forasteiro";
        default: return "Desconhecido";
    endcase
endfunction

// ========================================================================
// REDIRECIONAR PARA SISTEMAS ESPECÍFICOS JÁ IMPLEMENTADOS
// ========================================================================

// As funções PLACA_ShowCasaLeaderGump, PLACA_GerenciarInquilino e 
// PLACA_DemolirConstrucao foram removidas deste include porque
// o casa_uso.src já tem toda essa funcionalidade implementada e funcionando.
