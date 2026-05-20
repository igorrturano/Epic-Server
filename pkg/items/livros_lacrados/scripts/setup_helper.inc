function _Setup(df, key, valores)
    var elem := df.FindElement(key);
    if (elem.errortext)
        elem := df.CreateElement(key);
        if (elem.errortext)
            return;
        endif
    endif
 elem.SetProp("n", valores.size());
    var i := 1;
    while (i <= valores.size())
 elem.SetProp("v" + i, valores[i]);
        i += 1;
    endwhile
endfunction

function _SetupStr(df, key, value)
    var elem := df.FindElement(key);
    if (elem.errortext)
        elem := df.CreateElement(key);
        if (elem.errortext)
            return;
        endif
    endif
 elem.SetProp("v", value);
endfunction

function _SetupPags(df, entity_id, paginas)
 var key := entity_id + "_pags";
    var elem := df.FindElement(key);
    if (elem.errortext)
        elem := df.CreateElement(key);
        if (elem.errortext)
            return;
        endif
    endif
 elem.SetProp("n", paginas.size());
    var i := 1;
    while (i <= paginas.size())
 elem.SetProp("p" + i, paginas[i]);
        i += 1;
    endwhile
endfunction
