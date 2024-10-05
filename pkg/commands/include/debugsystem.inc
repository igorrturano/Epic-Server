/* 
 * Documentado em: debug.src
 */

use uo;

include "include/say";

const PROPTAG := "#debug_";

// Adicionar novos debugs aqui, no mesmo modelo. O valor da String deve coincidir com o var name do listDebugs no debug.src (conferir lá).
const DEBUG_NAME_ALL := "all";
const DEBUG_NAME_CLIMA := "clima";
const DEBUG_NAME_DAMAGE := "damage";
const DEBUG_NAME_SMELTING := "smelting";

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