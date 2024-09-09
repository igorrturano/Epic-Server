use uo;

const PROPTAG := "#debug_";

function checkDebug(who, debugName)
    var prop := getDebugProp(who, debugName);

    if (!prop)
        return false;
    endif

    return true;
endfunction

function getDebugProp(who, debugName)
    return GetObjProperty(who, PROPTAG + debugName);
endfunction

function debugMsg(who, debugName, msg)
    if (checkDebug(who, debugName) || checkDebug(who, "all"))
        msg := "DEBUG "+debugName+": "+msg;

        SendSysMessageEx(who, msg, SSM_INFO);
    endif
endfunction

function debugMsgEvent(debugName, msg) 
    foreach who in EnumerateOnlineCharacters()
        if (checkDebug(who, debugName) || checkDebug(who, "all"))
            msg := "DEBUG "+debugName+": "+msg;

            SendSysMessageEx(who, msg, SSM_INFO);
        endif
    endforeach
endfunction