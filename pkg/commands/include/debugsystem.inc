use uo;

const PROPTAG := "#debug_";

function checkDebug(who, debugName)
    var prop := getProp(who, debugName);

    if (!prop)
        return false;
    endif
endfunction

function getDebugProp(who, debugName)
    return GetObjProperty(who, PROPTAG + debugName);
endfunction

function debugMsg(who, msg)
    msg := "DEBUG: " + msg;

    SendSysMessageEx(who, msg, SSM_INFO);
endfunction