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
    Print("Tentativa de registro do processo: " + process_name + (unique_id ? " (" + unique_id + ")" : ""));
    
    var df := DFOpenDataFile(PROCESS_REGISTRY_DATAFILE, DF_CREATE);
    if (!df)
        Print("ERRO CRÍTICO: Não foi possível abrir/criar registry datafile");
        return error;
    endif
    
    var process_id := process_name;
    if (unique_id)
        process_id := process_name + "_" + unique_id;
    endif
    
    var elem := DFFindElement(df, process_id, DF_CREATE);
    if (!elem)
        Print("ERRO: Falha ao criar/encontrar elemento no registry");
        return error;
    endif
    
    // Verificar se há um processo registrado e se ainda está ativo
    var registered_pid := elem.GetProp("pid");
    var registered_time := elem.GetProp("registered_time");
    var last_heartbeat := elem.GetProp("last_heartbeat");
    var current_time := polcore().systime;
    
    // Se já existe um processo registrado
    if (registered_pid)
        // Verificar se o processo ainda está ativo
        var proc := GetProcess(registered_pid);
        
        // Se o processo existe e seu último heartbeat é recente
        if (proc && last_heartbeat && (current_time - last_heartbeat) < HEARTBEAT_TIMEOUT)
            Print("AVISO: Processo " + process_name + " já está em execução (PID: " + registered_pid + ")");
            // Registrar tentativa de inicialização duplicada
            var duplicate_attempts := elem.GetProp("duplicate_attempts");
            if (!duplicate_attempts)
                duplicate_attempts := 0;
            endif
            elem.SetProp("duplicate_attempts", duplicate_attempts + 1);
            elem.SetProp("last_duplicate_attempt", current_time);
            
            // Este processo deve ser encerrado
            return 0;
        else
            // Processo morto ou timeout, registrar este como novo
            Print("Encontrado registro de processo obsoleto. Substituindo...");
            if (proc)
                // Se o processo existe mas não deu heartbeat, tentar finalizá-lo
                Print("Tentando finalizar processo zumbi: " + registered_pid);
                proc.kill();
            endif
        endif
    endif
    
    // Registrar este processo
    elem.SetProp("pid", getpid());
    elem.SetProp("registered_time", current_time);
    elem.SetProp("last_heartbeat", current_time);
    elem.SetProp("host_runtime", polcore().systime);
    
    Print("Processo " + process_name + " registrado com sucesso (PID: " + getpid() + ")");
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
        Print("AVISO: PID registrado (" + registered_pid + ") não corresponde ao processo atual (" + getpid() + ")");
        // Não atualizar se não for o processo registrado
        return 0;
    endif
    
    // Atualizar o heartbeat
    elem.SetProp("last_heartbeat", polcore().systime);
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
        Print("AVISO: Tentativa de desregistro de outro processo. PID registrado: " + registered_pid + ", PID atual: " + getpid());
        return 0;
    endif
    
    // Não apagar o elemento, apenas marcar como inativo
    elem.SetProp("pid", 0);
    elem.SetProp("deregistered_time", polcore().systime);
    elem.SetProp("status", "stopped");
    
    Print("Processo " + process_name + " desregistrado com sucesso");
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
                Print("Limpando registro obsoleto: " + key + " (PID: " + registered_pid + ")");
                
                // Marcar como inativo
                elem.SetProp("pid", 0);
                elem.SetProp("status", "expired");
                elem.SetProp("expired_time", current_time);
                
                // Tentar matar o processo se ele existe mas está unresponsive
                if (proc)
                    Print("Tentando encerrar processo zumbi: " + registered_pid);
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