// Arquivo: :faccao:include/process_registry.inc
//
// Este include implementa um sistema de registro de processos que impede
// a execução duplicada de controllers críticos do sistema.

use uo;
use os;
use datafile;

// Constantes para configuração
const PROCESS_REGISTRY_DATAFILE := ":system:process_registry";
const HEARTBEAT_TIMEOUT := 300;  // 5 minutos de timeout para processos que não respondem

/*
 * RegisterProcess(process_name, unique_id := "")
 *
 * Purpose: Registra um processo em execução e verifica por duplicatas
 * Parameters:
 *   process_name - Nome do processo a ser registrado
 *   unique_id - Identificador único opcional (útil para múltiplas instâncias legítimas)
 * Return:
 *   1 se o processo foi registrado com sucesso
 *   0 se uma instância já está em execução e o processo atual deve ser terminado
 *   error em caso de falha no registro
 */
function RegisterProcess(process_name, unique_id := "")
    // Log inicial de tentativa de registro
    LogInfo("processo", $"Tentativa de registro do processo: {process_name}" + 
            (unique_id ? $" ({unique_id})" : ""));
    
    // Abrir datafile com tratamento de erro
    var df := SafeOpenDataFile(PROCESS_REGISTRY_DATAFILE);
    if (df == error)
        LogCritical("processo", "Falha ao abrir datafile de registro");
        return error;
    endif
    
    // Criar identificador único do processo
    var process_id := unique_id ? 
        $"{process_name}_{unique_id}" : 
        process_name;
    
    // Encontrar/criar elemento no registro
   var elem := SafeFindElement(df, process_id);
    if (elem == error)
        LogError("processo", $"Falha ao acessar elemento: {process_id}");
        return error;
    endif
    
    var current_time := polcore().systime;
    var registered_pid := elem.GetProp("pid");
    var last_heartbeat := elem.GetProp("last_heartbeat");
    
    // Verificar processo existente
    if (registered_pid)
        var proc := GetProcess(registered_pid);
        
        // Verificar se processo está ativo
        if (proc && last_heartbeat && 
            (current_time - last_heartbeat) < HEARTBEAT_TIMEOUT)
            
            LogWarning("processo", $"Processo {process_name} já em execução (PID: {registered_pid})");
            
            // Registrar tentativa duplicada
            var duplicate_attempts := elem.GetProp("duplicate_attempts") ?: 0;
            SetElementProp(elem, "duplicate_attempts", duplicate_attempts + 1);
            SetElementProp(elem, "last_duplicate_attempt", current_time);
            
            return 0;
        endif
        
        // Processo morto ou timeout
        if (proc)
            LogWarning("processo", $"Finalizando processo zumbi: {registered_pid}");
            proc.kill();
        endif
    endif
    
    // Registrar novo processo
    SetElementProp(elem, "pid", getpid());
    SetElementProp(elem, "registered_time", current_time);
    SetElementProp(elem, "last_heartbeat", current_time);
    SetElementProp(elem, "host_runtime", polcore().systime);
    
    LogInfo("processo", $"Processo {process_name} registrado (PID: {getpid()})");
    return 1;
endfunction

/*
 * UpdateProcessHeartbeat(process_name, unique_id := "")
 *
 * Purpose: Atualiza o heartbeat do processo para indicar que ainda está vivo
 * Parameters:
 *   process_name - Nome do processo
 *   unique_id - Identificador único se aplicável
 * Return:
 *   1 em caso de sucesso, 0 ou error em caso de falha
 */
function UpdateProcessHeartbeat(process_name, unique_id := "")
    var df := DFOpenDataFile(PROCESS_REGISTRY_DATAFILE);
    if (!df)
        return 0;
    endif
    
    var process_id := process_name;
    if (unique_id)
        process_id := process_name + "_" + unique_id;
    endif
    
    var elem := DFFindElement(df, process_id);
    if (!elem)
        return 0;
    endif
    
    // Verificar se o PID corresponde ao processo atual
    var registered_pid := elem.GetProp("pid");
    if (registered_pid != getpid())
        LogWarning("processo", $"PID registrado ({registered_pid}) não corresponde ao processo atual ({getpid()})");

        // Não atualizar se não for o processo registrado
        return 0;
    endif
    
    // Atualizar o heartbeat
    SetElementProp(elem, "last_heartbeat", polcore().systime);
    return 1;
endfunction

/*
 * DeregisterProcess(process_name, unique_id := "")
 *
 * Purpose: Remove o registro de um processo quando ele termina normalmente
 * Parameters:
 *   process_name - Nome do processo
 *   unique_id - Identificador único se aplicável
 * Return:
 *   1 em caso de sucesso, 0 ou error em caso de falha
 */
function DeregisterProcess(process_name, unique_id := "")
    var df := DFOpenDataFile(PROCESS_REGISTRY_DATAFILE);
    if (!df)
        return 0;
    endif
    
    var process_id := process_name;
    if (unique_id)
        process_id := process_name + "_" + unique_id;
    endif
    
    var elem := DFFindElement(df, process_id);
    if (!elem)
        return 0;
    endif
    
    // Verificar se o PID corresponde ao processo atual
    var registered_pid := elem.GetProp("pid");
    if (registered_pid != getpid())
       LogWarning("processo", $"Tentativa de desregistro de outro processo. PID registrado: {registered_pid}, PID atual: {getpid()}");

        return 0;
    endif
    
    // Não apagar o elemento, apenas marcar como inativo
    SetElementProp(elem, "pid", 0);
    SetElementProp(elem, "deregistered_time", polcore().systime);
    SetElementProp(elem, "status", "stopped");
    
    LogInfo("processo", $"Processo {process_name} desregistrado com sucesso");

    return 1;
endfunction

/*
 * CheckAndCleanupProcesses()
 *
 * Purpose: Verificar todos os processos registrados e limpar registros obsoletos
 * Return:
 *   Número de processos limpos
 */
function CheckAndCleanupProcesses()
    var df := DFOpenDataFile(PROCESS_REGISTRY_DATAFILE);
    if (!df)
        return 0;
    endif
    
    var cleanup_count := 0;
    var current_time := polcore().systime;
    
    foreach key in (df.Keys())
        var elem := DFFindElement(df, key);
        if (!elem)
            continue;
        endif
        
        var registered_pid := elem.GetProp("pid");
        var last_heartbeat := elem.GetProp("last_heartbeat");
        
        // Verificar se é um processo ativo
        if (registered_pid && registered_pid != 0)
            var proc := GetProcess(registered_pid);
            
            // Se o processo não existe ou excedeu o timeout
            if (!proc || (last_heartbeat && (current_time - last_heartbeat) > HEARTBEAT_TIMEOUT))
                LogDebug("processo", $"Limpando registro obsoleto: {key} (PID: {registered_pid})");
                
                // Marcar como inativo
                SetElementProp(elem, "pid", 0);
                SetElementProp(elem, "status", "expired");
                SetElementProp(elem, "expired_time", current_time);
                
                // Tentar matar o processo se ele existe mas está unresponsive
                if (proc)
                    LogWarning("processo", $"Tentando encerrar processo zumbi: {registered_pid}");

                    proc.kill();
                endif
                
                cleanup_count += 1;
            endif
        endif
        
        sleepms(2); // Prevenir congelamento do servidor
    endforeach
    
    return cleanup_count;
endfunction

/*
 * IsProcessRunning(process_name, unique_id := "")
 *
 * Purpose: Verificar se um processo específico está em execução
 * Parameters:
 *   process_name - Nome do processo
 *   unique_id - Identificador único se aplicável
 * Return:
 *   1 se o processo está ativo, 0 se não está ou error em caso de falha
 */
function IsProcessRunning(process_name, unique_id := "")
    var df := DFOpenDataFile(PROCESS_REGISTRY_DATAFILE);
    if (!df)
        return error;
    endif
    
    var process_id := process_name;
    if (unique_id)
        process_id := process_name + "_" + unique_id;
    endif
    
    var elem := DFFindElement(df, process_id);
    if (!elem)
        return 0;
    endif
    
    var registered_pid := elem.GetProp("pid");
    var last_heartbeat := elem.GetProp("last_heartbeat");
    var current_time := polcore().systime;
    
    // Se não há PID registrado ou é 0, não está rodando
    if (!registered_pid || registered_pid == 0)
        return 0;
    endif
    
    // Verificar se o processo existe
    var proc := GetProcess(registered_pid);
    if (!proc)
        return 0;
    endif
    
    // Verificar se o processo está respondendo (heartbeat recente)
    if (last_heartbeat && (current_time - last_heartbeat) > HEARTBEAT_TIMEOUT)
        return 0;
    endif
    
    return 1;
endfunction