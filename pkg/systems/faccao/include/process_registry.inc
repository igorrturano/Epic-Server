// Arquivo: :faccao:include/process_registry.inc
//
// Este include implementa um sistema de registro de processos que impede
// a execução duplicada de controllers críticos do sistema.

use uo;
use os;
use datafile;

// Constantes para configuração
const PROCESS_REGISTRY_DATAFILE := ":faccao:process_registry";
const HEARTBEAT_TIMEOUT := 300;  // 5 minutos de timeout para processos que não respondem


/*
 * RegisterProcess(process_name, unique_id := "")
 *
 * Purpose: Registra um processo em execução e verifica por duplicatas.
 *          Esta é a função CENTRAL de registro.
 * Parameters:
 *   process_name - Nome do processo (ex: "ResourceController")
 *   unique_id    - Opcional, para instâncias múltiplas legítimas (não usado por ResourceController)
 * Return:
 *   1: Sucesso, esta instância é a mestre.
 *   0: Duplicata, esta instância deve sair.
 *   error: Erro crítico no datafile.
 */
function RegisterProcess(process_name, unique_id := "")
    LogInfo("processo", $"Tentativa de registro do processo: ${process_name}" + (unique_id ? $" (${unique_id})" : ""));

    // DF_CREATE garante que o datafile e o elemento são criados se não existirem.
    var df := SafeOpenDataFile(PROCESS_REGISTRY_DATAFILE, DF_CREATE);
    if (df == error)
        LogCritical("processo", "FALHA CRÍTICA ao abrir/criar datafile de registro: " + df.errortext + " para " + PROCESS_REGISTRY_DATAFILE);
        return error;
    endif
    
    var process_id_key := unique_id ? $"{process_name}_${unique_id}" : process_name;
    var elem := SafeFindElement(df, process_id_key, DF_CREATE);
    if (elem == error)
        LogError("processo", $"FALHA CRÍTICA ao acessar/criar elemento: ${process_id_key}, Erro: " + elem.errortext);
        return error;
    endif
    
    var current_time := polcore().systime;
    var current_pid := GetPid();
    
    var registered_pid_str := elem.GetProp("pid"); // Pode ser string ou integer do datafile
    var registered_pid := 0;
    if (registered_pid_str)
        registered_pid := CInt(registered_pid_str);
    endif
    
    var last_heartbeat := CInt(elem.GetProp("last_heartbeat"));

    // Cenário 1: Há um PID registrado no datafile.
    if (registered_pid && registered_pid != 0)
        // Cenário 1a: O PID registrado é o MESMO que o PID do processo atual.
        if (registered_pid == current_pid)
            LogWarning("processo", $"PID registrado (${registered_pid}) para '${process_id_key}' é o mesmo que o PID atual. Permitindo re-registro/atualização do heartbeat.");
            // A execução continuará para o bloco de registro/atualização no final da função.
        else
            // Cenário 1b: O PID registrado é DIFERENTE do PID do processo atual.
            // Precisamos verificar se o processo com 'registered_pid' ainda está realmente ativo.
            var old_process_obj := GetProcess(registered_pid); // Renomeado para old_process_obj para clareza
            if (old_process_obj) // O processo com o PID antigo AINDA EXISTE no sistema.
                // Verificar se o heartbeat do processo antigo é recente.
                if (last_heartbeat && (current_time - last_heartbeat) < HEARTBEAT_TIMEOUT)
                    // O processo antigo existe E seu heartbeat está OK.
                    // Esta é uma verdadeira instância duplicada tentando iniciar.
                    LogWarning("processo", $"Processo '${process_id_key}' já está em execução com PID ${registered_pid} (Heartbeat OK). PID atual ${current_pid} é uma duplicata e será encerrado.");
                    
                    // Opcional: Registrar tentativa de duplicata
                    var duplicate_attempts := CInt(elem.GetProp("duplicate_attempts")) + 1;
                    SetElementProp(elem, "duplicate_attempts", duplicate_attempts);
                    SetElementProp(elem, "last_duplicate_attempt", current_time);
                    
                    return 0; // Sinaliza para a instância atual sair.
                else
                    // O processo antigo existe, mas seu heartbeat expirou ou não existe.
                    // Consideramos um processo "zumbi" que não está atualizando seu status.
                    var time_diff := current_time - last_heartbeat;
                    LogWarning("processo", $"Processo '${process_id_key}' com PID ${registered_pid} encontrado, mas heartbeat expirou (Last: ${last_heartbeat}, Current: ${current_time}, Diff: ${time_diff}s). Finalizando processo zumbi.");
                    old_process_obj.kill(); // Usar old_process_obj.kill()
                    // A execução continuará para registrar esta nova instância.
                endif // Fim do if (last_heartbeat ...)
            else // O processo com o PID antigo NÃO EXISTE mais no sistema.
                LogInfo("processo", $"Registro para processo '${process_id_key}' com PID ${registered_pid} encontrado, mas o processo não está mais ativo. Limpando registro antigo.");
                // A execução continuará para registrar esta nova instância.
            endif // Fim do if (old_process_obj)
        endif // Fim do if (registered_pid == current_pid)
    endif // Fim do if (registered_pid && registered_pid != 0)
    // Cenário 2: Nenhum PID registrado (ou é 0). Este é o primeiro registro ou o registro anterior foi limpo.
    // A execução continuará para o bloco de registro/atualização.

    // Registrar/Atualizar o processo atual
    if (!SetElementProp(elem, "pid", current_pid))
        LogError("processo", "Falha ao definir PID para " + process_id_key);
        return error;
    endif
    
    // CORREÇÃO APLICADA AQUI:
    var current_process_obj := GetProcess(current_pid);
    if (current_process_obj && current_process_obj.script) // Verifica se o processo e a propriedade script existem
        if (!SetElementProp(elem, "script_path", current_process_obj.script) )
            LogError("processo", "Falha ao definir script_path para " + process_id_key); 
        endif
    elseif (!current_process_obj)
         LogError("processo", "Falha ao obter objeto do processo para PID " + current_pid + " para " + process_id_key);
    else // current_process_obj existe, mas .script não (improvável, mas para segurança)
         LogWarning("processo", "Objeto do processo para PID " + current_pid + " não possui propriedade 'script' para " + process_id_key);
    endif
    
    if (!SetElementProp(elem, "registered_time", current_time))
        LogError("processo", "Falha ao definir registered_time para " + process_id_key);
        return error;
    endif
    
    if (!SetElementProp(elem, "last_heartbeat", current_time))
        LogError("processo", "Falha ao definir last_heartbeat para " + process_id_key);
        return error;
    endif
    
    // EraseObjProperty(elem, "duplicate_attempts"); // Opcional: Limpar contagem de duplicatas no registro bem-sucedido
    // EraseObjProperty(elem, "last_duplicate_attempt");
    
    LogInfo("processo", $"Processo '${process_id_key}' registrado/atualizado com sucesso (PID: ${current_pid}).");
    return 1; // Sucesso no registro
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