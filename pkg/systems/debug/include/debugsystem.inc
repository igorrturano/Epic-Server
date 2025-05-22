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


/**
 * Enumeração DEBUGNAMES
 * 
 * Lista os nomes de debug disponíveis que podem ser ativados pelos jogadores utilizando o comando `.debug <nome>`.
 * Cada valor representa uma categoria específica de debug para facilitar o diagnóstico e desenvolvimento do sistema.
 * 
 * Valores:
 * - DEBUG_ALL: Ativa todos os debugs.
 * - DEBUG_CLIMA: Debug relacionado ao sistema de clima.
 * - DEBUG_DAMAGE: Debug relacionado ao sistema de dano.
 * - DEBUG_SMELTING: Debug relacionado ao sistema de fundição.
 * - DEBUG_UP: Debug relacionado a melhorias/upgrade.
 * - DEBUG_EQUIP: Debug relacionado a equipamentos.
 * - DEBUG_HIT: Debug relacionado a acertos/golpes.
 * - DEBUG_HAB: Debug relacionado a habilidades.
 * - DEBUG_MASTERY: Debug relacionado à maestria.
 * - DEBUG_ACHIEVEMENT: Debug relacionado a conquistas.
 */
enum DEBUGNAMES
	DEBUG_ALL := "all",
	DEBUG_CLIMA := "clima",
	DEBUG_DAMAGE := "damage",
	DEBUG_SMELTING := "smelting",
    DEBUG_UP := "up",
	DEBUG_EQUIP := "equip",
    DEBUG_HIT := "hit",
	DEBUG_HAB := "habilidades",
	DEBUG_MASTERY := "maestria",
	DEBUG_ACHIEVEMENT := "conquistas",
endenum

/**
 * Lista de estruturas que definem os diferentes tipos de debugs disponíveis no sistema.
 * Cada item contém:
 *   - name: Enum que identifica o tipo de debug.
 *   - "level": Nível mínimo de permissão necessário para ativar o debug.
 * 
 * Novos tipos de debug devem ser adicionados nesta lista, utilizando o enum correspondente
 * e especificando o nível de permissão adequado.
 */
var listDebugs := array{
    struct { name := DEBUG_ALL, "level" := 0 },
    struct { name := DEBUG_CLIMA, "level" := 0 }, 
    struct { name := DEBUG_DAMAGE, "level" := 0 },
	struct { name := DEBUG_SMELTING, "level" := 0 },
	struct { name := DEBUG_UP, "level" := 0 },
	struct { name := DEBUG_EQUIP, "level" := 0 },
    struct { name := DEBUG_HIT, "level" := 0 },
	struct { name := DEBUG_HAB, "level" := 0 },
	struct { name := DEBUG_MASTERY, "level" := 0 },
	struct { name := DEBUG_ACHIEVEMENT, "level" := 0 }
};

// function CheckDebug(who, debugName)
//     var prop := who.getDebugProp(debugName);

//     if (!prop)
//         return 0;
//     endif

//     return 1;
// endfunction

function DebugMsg(who, debugName, msg)
    if (who.getDebugProp(debugName) || who.getDebugProp(DEBUG_ALL))
        SendSysMessageEx(who, $"DEBUG {debugName}: {msg}", SSM_INFO);
    endif
endfunction

function DebugMsgEvent(debugName, msg) 
    foreach who in EnumerateOnlineCharacters()
        if (who.getDebugProp(debugName) || who.getDebugProp(DEBUG_ALL))
            SendSysMessageEx(who, $"DEBUG {debugName}: {msg}", SSM_INFO);
        endif
    endforeach
endfunction