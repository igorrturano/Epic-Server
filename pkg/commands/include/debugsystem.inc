/* 
 * Documentado em: debug.src
 */

use uo;

include "include/say";

const PROPTAG := "#debug_";

// Adicionar novos debugs aqui, no mesmo modelo.
enum DEBUGNAMES
	DEBUG_ALL := "all",
	DEBUG_CLIMA := "clima",
	DEBUG_DAMAGE := "damage",
	DEBUG_SMELTING := "smelting"
endenum

var listDebugs := array{ // Novos debugs precisam ser adicionados aqui com o cmdlevel
    struct { name := DEBUG_ALL, "level" := 1 },
    struct { name := DEBUG_CLIMA, "level" := 0 }, 
    struct { name := DEBUG_DAMAGE, "level" := 0 },
	struct { name := DEBUG_SMELTING, "level" := 0 }
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