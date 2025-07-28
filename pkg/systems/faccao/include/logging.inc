include ":faccao:include/permissions";

// Constantes de nível de log
const LOG_CRITICAL := 1;  // Erros graves que afetam o funcionamento
const LOG_ERROR    := 2;  // Erros que não interrompem o sistema
const LOG_WARNING  := 3;  // Avisos importantes 
const LOG_INFO     := 4;  // Informações normais
const LOG_DEBUG    := 5;  // Informação detalhada para depuração

// Variável global para controlar nível de log
var GLOBAL_LOG_LEVEL := 3;  // Por padrão, mostrar até WARNING

// Flag para desativar completamente o logging (útil em produção)
var LOGGING_ENABLED := 1;  // 1 = ativado, 0 = desativado

// Variável para controle por categoria
var LOG_CATEGORIES := dictionary{
    "recursos" := 3,
    "permissao" := 3,
    "territorio" := 3,
    "bau" := 3,
    "CompraMapeador" := 5, 
    "CompraFerramentas" := 5,
    "FeudoRent_Ctrl" := 3,
    "FeudoRent_Payment" := 3,
    "FeudoRent_Debit_Robusto" := 3,
    "FeudoRent_Debit" := 3
};

function EnableLogging(enable := 1)
    LOGGING_ENABLED := enable;
    return LOGGING_ENABLED;
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
    var timestamp := FormatTimeString(polcore().systime);
    var level_text := "";
    case (level)
        LOG_CRITICAL: level_text := "CRÍTICO";
        LOG_ERROR:    level_text := "ERRO";
        LOG_WARNING:  level_text := "AVISO";
        LOG_INFO:     level_text := "INFO";
        LOG_DEBUG:    level_text := "DEBUG";
    endcase
    
    var formatted_message := "[" + timestamp + "] [" + level_text + "] [" + category + "] " + message;
    
    Print(formatted_message);
    
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
    
    // Criar pasta se não existir
    if (!CreateDirectory(":faccao:logs"))
        // Se falhar, tentar outro local
        logfile := "logs/faccao.log";
    endif
    
    AppendToFile(logfile, message + "\n");
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

function PadStr(str, length_needed, padchar := "0") // Certifique-se que esta função auxiliar está disponível
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