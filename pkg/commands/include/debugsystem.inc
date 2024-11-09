/*
 * debusystem.inc - Sistema de Depuração (Debug)
 * 
 * Este arquivo implementa um sistema de debug personalizável para o servidor POL,
 * permitindo que diferentes tipos de mensagens de depuração sejam exibidas 
 * apenas para jogadores ou usuários que ativaram os debugs correspondentes.
 * 
 * Cada tipo de debug é associado a um nome e a um nível de comando (cmdlevel).
 * O sistema permite ativar e desativar debugs dinamicamente, ajudando os 
 * desenvolvedores e administradores a monitorar eventos específicos no servidor,
 * como mudanças de clima, dano recebido, aumento de habilidades, etc.
 * 
 * Exemplo de uso:
 * - O jogador pode ativar o debug digitando ".debug up" para monitorar o aumento
 *   de habilidades.
 * - As mensagens de debug aparecerão no formato: "DEBUG <nome>: <mensagem>".
 * 
 * Para adicionar novos tipos de debug:
 * 1. Adicione um novo nome à enumeração DEBUGNAMES.
 * 2. Inclua o novo debug na lista "listDebugs" com o nível de acesso desejado.
 * 3. Utilize a função `DebugMsg` ou `DebugMsgEvent` para enviar mensagens
 *    de depuração relacionadas ao novo debug.
 * 
 * Documentação adicional pode ser encontrada em: debug.src
 * 
 * Autor: [Seu Nome]
 * Data: [Data de criação]
 */

use uo;

include "include/say";

const PROPTAG := "#debug_";

// Adicionar novos debugs aqui, no mesmo modelo.
enum DEBUGNAMES
	DEBUG_ALL := "all",
	DEBUG_CLIMA := "clima",
	DEBUG_DAMAGE := "damage",
	DEBUG_SMELTING := "smelting",
    DEBUG_UP := "up",
	DEBUG_EQUIP := "equip"
endenum

var listDebugs := array{ // Novos debugs precisam ser adicionados aqui com a tag e o cmdlevel
    struct { name := DEBUG_ALL, "level" := 0 },
    struct { name := DEBUG_CLIMA, "level" := 0 }, 
    struct { name := DEBUG_DAMAGE, "level" := 0 },
	struct { name := DEBUG_SMELTING, "level" := 0 },
	struct { name := DEBUG_UP, "level" := 0 },
	struct { name := DEBUG_EQUIP, "level" := 0 }
};

function CheckDebug(who, debugName)
    var prop := getDebugProp(who, debugName);

    if (!prop)
        return 0;
    endif

    return 1;
endfunction

function getDebugProp(who, debugName)
    return GetObjProperty(who, PROPTAG + debugName);
endfunction

function DebugMsg(who, debugName, msg)
    if (checkDebug(who, debugName) || checkDebug(who, "all"))
        SendSysMessageEx(who, $"DEBUG {debugName}: {msg}", SSM_INFO);
    endif
endfunction

function DebugMsgEvent(debugName, msg) 
    foreach who in EnumerateOnlineCharacters()
        if (checkDebug(who, debugName) || checkDebug(who, "all"))
            SendSysMessageEx(who, $"DEBUG {debugName}: {msg}", SSM_INFO);
        endif
    endforeach
endfunction