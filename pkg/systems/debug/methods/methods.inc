include ":debug:main";

print("INSTALLING: Methods for Debug System... OK!");

exported function getDebugProp(who, debugName)
    return who.GetProp(PROPTAG + debugName);
endfunction

exported function setDebugProp(who, debugName)
	var propName := PROPTAG + debugName;
    var prop := who.GetProp(propName);

    if (!prop)
        who.SetProp(propName, 1);
        SendSysMessageEx(who, $"DEBUG {debugName}: ativado!");
    else
        EraseObjProperty(who, propName);
        SendSysMessageEx(who, $"DEBUG {debugName}: desativado!");
    endif
endfunction