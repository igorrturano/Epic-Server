function IsScriptRunning(obj, prop_key)
    var pid := CInt(GetObjProperty(obj, prop_key));
    if (!pid) return 0; endif
    var proc := GetProcess(pid);
    return (proc && proc.pid) ? 1 : 0;
endfunction

function TryAcquirePID(obj, prop_key)
    if (IsScriptRunning(obj, prop_key)) return 0; endif
    SetObjProperty(obj, prop_key, GetPid());
    return 1;
endfunction

function ReleasePID(obj, prop_key)
    EraseObjProperty(obj, prop_key);
endfunction
