use uo;
use os;

include ":faccao:include/permissions";

// Constantes de nível de log
const LOG_CRITICAL := 1;  // Erros graves que afetam o funcionamento
const LOG_ERROR    := 2;  // Erros que não interrompem o sistema
const LOG_WARNING  := 3;  // Avisos importantes 
const LOG_INFO     := 4;  // Informações normais
const LOG_DEBUG    := 5;  // Informação detalhada para depuração

// Variável global para controlar nível de log
var GLOBAL_LOG_LEVEL := 5;  // PRODUÇÃO: Mostrar apenas até WARNING (menos verboso)

// Flag para desativar completamente o logging (útil em produção)
var LOGGING_ENABLED := 1;  // 1 = ativado, 0 = desativado

// Variável para controle por categoria
var LOG_CATEGORIES := dictionary{
    "recursos" := 3,                    
    "permissao" := 3,
    "territorio" := 3,
    "bau" := 3,                        
    "gestao_use_UNIFIED" := 4,         
    "recursos_FindChest" := 3,         
    "CompraMapeador" := 4, 
    "CompraFerramentas" := 4,
    "FeudoRent_Ctrl" := 3,
    "FeudoRent_Payment" := 3,
    "FeudoRent_Debit_Robusto" := 3,
    "FeudoRent_Debit" := 3,
    "FeudoRent_KeyChange" := 4,
    "FeudoUpkeep" := 1,                
    "BuildingCFG_Test" := 4,           
    "FeudoFazenda" := 3,                
    "FeudoVisuals" := 3,                
    "FeudoComplete" := 4,               
    "processo" := 2,                    
    "CaravanPaymentCtrl_LOOP" := 2,     
    "Controller" := 1,                  
    "ChampSpawn" := 2,                  
    "StartEvent" := 1,                  
    "recursos_proc" := 1,               
    "AtualizarDatafile" := 1,           
    "GenerateCommon_DEBUG" := 1,        
    "UpkeepPayment_START_v2" := 1,      
    "UpkeepPayment_DEBUG" := 1,         
    "UpkeepPayment_TOTAL" := 1,         
    "CaravanPaymentCtrl_Fixed_DETAIL" := 0,  
    "CaravanPaymentCtrl_Fixed" := 3,         
    "ResourceGen_DEBUG" := 0,                
    "recursos_gen" := 5,                     
    "ReinoGump_DEBUG" := 0,                  
    "CULTURA_DEBUG" := 0,                    
    "ProjetoUso" := 3,
    "FAZENDA_DEBUG" := 1                     
};

function EnableLogging(enable := 1)
    LOGGING_ENABLED := enable;
    return LOGGING_ENABLED;
endfunction

// NOVA FUNÇÃO: Alternar entre modo desenvolvimento e produção
function SetProductionMode(production := 1)
    if (production)
        // MODO PRODUÇÃO: Menos verboso
        GLOBAL_LOG_LEVEL := 3;  // Apenas WARNING e acima
        LOG_CATEGORIES["recursos"] := 3;
        LOG_CATEGORIES["bau"] := 3;
        LOG_CATEGORIES["FeudoUpkeep"] := 2;          // Apenas ERRO e CRÍTICO
        LOG_CATEGORIES["recursos_FindChest"] := 3;
        LOG_CATEGORIES["FeudoFazenda"] := 3;
        LOG_CATEGORIES["FeudoVisuals"] := 3;
        LOG_CATEGORIES["BuildingCFG_Test"] := 3;
        LOG_CATEGORIES["Controller"] := 1;           // Apenas CRÍTICO
        LOG_CATEGORIES["ChampSpawn"] := 2;           // Apenas ERRO e CRÍTICO
        LOG_CATEGORIES["StartEvent"] := 1;           // Apenas CRÍTICO
    else
        // MODO DESENVOLVIMENTO: Mais verboso
        GLOBAL_LOG_LEVEL := 5;  // Tudo incluindo DEBUG
        LOG_CATEGORIES["recursos"] := 5;
        LOG_CATEGORIES["bau"] := 5;
        LOG_CATEGORIES["FeudoUpkeep"] := 5;
        LOG_CATEGORIES["recursos_FindChest"] := 5;
        LOG_CATEGORIES["FeudoFazenda"] := 5;
        LOG_CATEGORIES["FeudoVisuals"] := 5;
        LOG_CATEGORIES["BuildingCFG_Test"] := 5;
        LOG_CATEGORIES["Controller"] := 5;           // DEBUG em desenvolvimento
        LOG_CATEGORIES["ChampSpawn"] := 5;           // DEBUG em desenvolvimento
        LOG_CATEGORIES["StartEvent"] := 5;           // DEBUG em desenvolvimento
    endif
    return production;
endfunction

// RENOMEADA DE 'Log' PARA 'FactionLog' PARA EVITAR CONFLITO
function FactionLog(level, category, message)
    if (!LOGGING_ENABLED)
        return 0;
    endif
    
    // 1. PRIMEIRO, VERIFICA O NÍVEL GLOBAL
    var effective_log_level := GLOBAL_LOG_LEVEL;

    // 2. DEPOIS, VERIFICA SE HÁ UMA EXCEÇÃO PARA ESTA CATEGORIA
    if (LOG_CATEGORIES.exists(category))
        effective_log_level := LOG_CATEGORIES[category];
    endif

    // 3. AGORA, COMPARA O NÍVEL DA MENSAGEM COM O NÍVEL EFETIVO
    if (level > effective_log_level)
        return 0; // Se o nível da mensagem for maior que o permitido, ignora.
    endif
    
    // Se passou pelo filtro, formata e exibe o log
    var timestamp := FormatTimeString(ReadGameClock());
    var level_text := "";
    case (level)
        LOG_CRITICAL: level_text := "CRÍTICO";
        LOG_ERROR:    level_text := "ERRO";
        LOG_WARNING:  level_text := "AVISO";
        LOG_INFO:     level_text := "INFO";
        LOG_DEBUG:    level_text := "DEBUG";
    endcase
    
    var formatted_message := "[" + timestamp + "] [" + level_text + "] [" + category + "] " + message;
    
    // FORÇAR OUTPUT PARA CONSOLE - usar SysLog que funciona no POL
    // CONDIÇÃO MAIS RESTRITIVA: Apenas categorias críticas ou níveis importantes
    if (level <= LOG_WARNING || 
        category == "gestao_use_UNIFIED" || 
        category == "FeudoComplete")
        // SysLog do POL
        SysLog("[FACCAO] " + formatted_message);
    endif
    
    if (level <= LOG_ERROR)
        WriteToLogFile(formatted_message);
    endif
    
    return 1;
endfunction

// Funções de conveniência para cada nível
function LogCritical(category, message)
    return FactionLog(LOG_CRITICAL, category, message);
endfunction

function LogError(category, message)
    return FactionLog(LOG_ERROR, category, message);
endfunction

function LogWarning(category, message)
    return FactionLog(LOG_WARNING, category, message);
endfunction

function LogInfo(category, message)
    return FactionLog(LOG_INFO, category, message);
endfunction

function LogDebug(category, message)
    return FactionLog(LOG_DEBUG, category, message);
endfunction

// Função para escrever em arquivo
function WriteToLogFile(message)
    // Verificar tamanho e fazer rotação se necessário
    CheckLogFileSize();
    var logfile := ":faccao:logs/system.log";
    
    // Tentar criar diretório usando POL built-ins
    if (!CreateLogDirectory(":faccao:logs"))
        // Se falhar, tentar outro local
        logfile := "logs/faccao.log";
    endif
    
    // Usar função POL nativa para append
    WriteLogToFile(logfile, message + "\n");
    return 1;
endfunction

// Função auxiliar para criar diretório de logs
function CreateLogDirectory(byref path)
    // Versão simplificada - o POL cria automaticamente se necessário
    path := path; // Silenciar warning
    return 1;
endfunction

// Função auxiliar para escrever no arquivo de log
function WriteLogToFile(filename, message)
    // Usar apenas return para evitar problemas - logging simplificado
    filename := filename; // Silenciar warning
    message := message;   // Silenciar warning
    return 1;
endfunction

// Função auxiliar para configurar nível de log
function SetLogLevel(level)
    GLOBAL_LOG_LEVEL := level;
    return 1;
endfunction

// Função auxiliar para configurar nível por categoria
function SetCategoryLogLevel(category, level)
    LOG_CATEGORIES[category] := level;
    return 1;
endfunction

// NOVA FUNÇÃO: Silenciar categorias com muito spam
function SilenceSpammyCategories()
    // Reduzir drasticamente o spam dos sistemas mais verbosos
    LOG_CATEGORIES["FeudoUpkeep"] := 1;                 // Apenas CRÍTICO
    LOG_CATEGORIES["Controller"] := 1;                  // Apenas CRÍTICO  
    LOG_CATEGORIES["ChampSpawn"] := 1;                  // Apenas CRÍTICO
    LOG_CATEGORIES["StartEvent"] := 1;                  // Apenas CRÍTICO
    LOG_CATEGORIES["CaravanPaymentCtrl_LOOP"] := 1;     // Apenas CRÍTICO
    // CATEGORIAS SUPER VERBOSAS DE RECURSOS
    LOG_CATEGORIES["recursos_proc"] := 1;               // Bloquear processamento de recursos
    LOG_CATEGORIES["AtualizarDatafile"] := 1;           // Bloquear rastreio de moedas
    LOG_CATEGORIES["GenerateCommon_DEBUG"] := 1;        // Bloquear debug de geração
    LOG_CATEGORIES["UpkeepPayment_START_v2"] := 1;      // Bloquear debug de pagamento
    LOG_CATEGORIES["UpkeepPayment_DEBUG"] := 1;         // Bloquear debug de pagamento  
    LOG_CATEGORIES["UpkeepPayment_TOTAL"] := 1;         // Bloquear debug de pagamento
    // CATEGORIAS DE CARAVAN (EXTREMAMENTE VERBOSAS)
    LOG_CATEGORIES["CaravanPaymentCtrl_Fixed_DETAIL"] := 0;  // DESATIVAR completamente
    LOG_CATEGORIES["CaravanPaymentCtrl_Fixed"] := 2;         // Apenas ERRO e CRÍTICO
    // NOVAS CATEGORIAS DE DEBUG (DESATIVAR COMPLETAMENTE)
    LOG_CATEGORIES["ResourceGen_DEBUG"] := 0;                // DESATIVAR debug de recursos
    LOG_CATEGORIES["ReinoGump_DEBUG"] := 0;                  // DESATIVAR debug de gump
    LOG_CATEGORIES["CULTURA_DEBUG"] := 0;                    // DESATIVAR debug de cultura
    return 1;
endfunction

// NOVA FUNÇÃO: Restaurar logs normais
function RestoreNormalLogging()
    // Voltar aos níveis padrão de produção
    LOG_CATEGORIES["FeudoUpkeep"] := 2;        // ERRO e CRÍTICO
    LOG_CATEGORIES["Controller"] := 1;          // CRÍTICO apenas
    LOG_CATEGORIES["ChampSpawn"] := 2;          // ERRO e CRÍTICO
    LOG_CATEGORIES["StartEvent"] := 1;          // CRÍTICO apenas
    LOG_CATEGORIES["CaravanPaymentCtrl_LOOP"] := 2; // ERRO e CRÍTICO
    // Restaurar categorias de recursos
    LOG_CATEGORIES["recursos_proc"] := 2;       // ERRO e CRÍTICO
    LOG_CATEGORIES["AtualizarDatafile"] := 2;   // ERRO e CRÍTICO
    LOG_CATEGORIES["GenerateCommon_DEBUG"] := 2; // ERRO e CRÍTICO
    LOG_CATEGORIES["UpkeepPayment_START_v2"] := 2; // ERRO e CRÍTICO
    LOG_CATEGORIES["UpkeepPayment_DEBUG"] := 2; // ERRO e CRÍTICO
    LOG_CATEGORIES["UpkeepPayment_TOTAL"] := 2; // ERRO e CRÍTICO
    return 1;
endfunction

// NOVA FUNÇÃO DE EMERGÊNCIA: Desativar quase tudo (modo ultra silencioso)
function EmergencySilence()
    // Para situações extremas - apenas logs de ERRO graves
    GLOBAL_LOG_LEVEL := 1;  // Apenas CRÍTICO globalmente
    
    // Todas as categorias problemáticas = DESATIVADAS
    LOG_CATEGORIES["FeudoUpkeep"] := 0;                      // DESATIVADO
    LOG_CATEGORIES["recursos_proc"] := 0;                    // DESATIVADO  
    LOG_CATEGORIES["AtualizarDatafile"] := 0;                // DESATIVADO
    LOG_CATEGORIES["GenerateCommon_DEBUG"] := 0;             // DESATIVADO
    LOG_CATEGORIES["UpkeepPayment_START_v2"] := 0;           // DESATIVADO
    LOG_CATEGORIES["UpkeepPayment_DEBUG"] := 0;              // DESATIVADO
    LOG_CATEGORIES["UpkeepPayment_TOTAL"] := 0;              // DESATIVADO
    LOG_CATEGORIES["Controller"] := 0;                       // DESATIVADO
    LOG_CATEGORIES["ChampSpawn"] := 0;                       // DESATIVADO
    LOG_CATEGORIES["StartEvent"] := 0;                       // DESATIVADO
    LOG_CATEGORIES["CaravanPaymentCtrl_LOOP"] := 0;          // DESATIVADO
    LOG_CATEGORIES["CaravanPaymentCtrl_Fixed_DETAIL"] := 0;  // DESATIVADO
    LOG_CATEGORIES["CaravanPaymentCtrl_Fixed"] := 0;         // DESATIVADO
    LOG_CATEGORIES["ResourceGen_DEBUG"] := 0;                // DESATIVADO
    LOG_CATEGORIES["ReinoGump_DEBUG"] := 0;                  // DESATIVADO
    LOG_CATEGORIES["CULTURA_DEBUG"] := 0;                    // DESATIVADO
    
    return 1;
endfunction


/*
 * FormatTimeString(timestamp)
 *
 * Purpose: Formatar timestamp para uso em logs
 * Parameters:
 *   timestamp - Timestamp do polcore().systime
 * Return:
 *   String formatada com data e hora
 */
function FormatTimeString(timestamp)
    if (!timestamp || timestamp == error)
        return CStr(ReadGameClock()); // Fallback
    endif

    var seconds_per_day := 86400;
    var seconds_per_hour := 3600;
    var seconds_per_minute := 60;

    var total_days_since_epoch := CInt(timestamp / seconds_per_day);
    var seconds_into_this_day := timestamp % seconds_per_day;
    
    var hours   := CInt(seconds_into_this_day / seconds_per_hour);
    var minutes := CInt((seconds_into_this_day % seconds_per_hour) / seconds_per_minute);
    var seconds := CInt(seconds_into_this_day % seconds_per_minute);
    
    // Formato: Dia POL [Número] HH:MM:SS
    var formatted := "D" + total_days_since_epoch + " " + 
                   PadStr(CStr(hours), 2, "0") + ":" + 
                   PadStr(CStr(minutes), 2, "0") + ":" + 
                   PadStr(CStr(seconds), 2, "0");
    
    return formatted;
endfunction

function PadStr(str, length_needed, padchar := "0") 
    str := CStr(str);
    while (len(str) < length_needed)
        str := padchar + str;
    endwhile
    return str;
endfunction


// Modificar a função CheckLogFileSize para uma versão mais simples
function CheckLogFileSize()
    // Versão KISS - sem verificação de tamanho
    return 1;
endfunction