use uo;
use os;
use basic;
use polsys;
use cfgfile;

const ED_ACTIONS_CFG := ":epicdungeon:config/actions";

function EDSplitChar(str, ch)
    var result      := array{};
    if (!str || str == "") return result; endif
    var current_pos := 1;
    var delim_pos   := Find(str, ch, 1);
    while (delim_pos)
        result.append(str[current_pos, delim_pos - current_pos]);
        current_pos := delim_pos + 1;
        delim_pos   := Find(str, ch, current_pos);
    endwhile
    result.append(str[current_pos, Len(str) - current_pos + 1]);
    return result;
endfunction

function EDParseParam(param_str)
    if (!param_str || Len(param_str) == 0) return 0; endif
    var p     := EDSplitChar(param_str, ":");
    var param := struct{};
    param.+name  := p[1];
    param.+ptype := p[2];
    if (param.ptype == "slider")
        param.+pmin     := CInt(p[3]);
        param.+pmax     := CInt(p[4]);
        param.+pdefault := CInt(p[5]);
        var raw_step := CInt(p[6]);
        if (raw_step > 0)
            param.+pstep := raw_step;
        else
            param.+pstep := 0;
        endif
    elseif (param.ptype == "text")
        param.+pmaxlen  := CInt(p[3]);
        param.+pdefault := "";
    elseif (param.ptype == "bool")
        param.+pdefault := CInt(p[3]);
    elseif (param.ptype == "dropdown")
        param.+poptions := EDSplitChar(p[3], "|");
        var def_str     := p[4];
        var def_idx     := 0;
        for i := 1 to param.poptions.size()
            if (param.poptions[i] == def_str)
                def_idx := i - 1;
                break;
            endif
        endfor
        param.+pdefault := def_idx;
    elseif (param.ptype == "hue")
        param.+phuemax  := CInt(p[3]);
        param.+pdefault := CInt(p[4]);
    endif
    return param;
endfunction

function EDParseAllParams(params_str)
    var result := array{};
    if (!params_str || Len(params_str) == 0) return result; endif
    var parts := EDSplitChar(params_str, ",");
    foreach pstr in parts
        var p := EDParseParam(pstr);
        if (p)
            result.append(p);
        endif
    endforeach
    return result;
endfunction

function EDGetParamDefaults(action)
    var all_params := EDParseAllParams(action.params);
    var defaults   := array{};
    foreach param in all_params
        if (param.ptype == "slider" || param.ptype == "bool" || param.ptype == "dropdown")
            defaults.append(CInt(param.pdefault));
        elseif (param.ptype == "text")
            defaults.append("");
        elseif (param.ptype == "hue")
            defaults.append(CInt(param.pdefault));
        else
            defaults.append(0);
        endif
    endforeach
    return defaults;
endfunction

function EDLoadActions()
    var cfg := ReadConfigFile(ED_ACTIONS_CFG);
    if (cfg == error) return array{}; endif
    var names   := GetConfigStringKeys(cfg);
    var actions := array{};
    foreach name in names
        var elem   := FindConfigElem(cfg, name);
        var action := struct{};
        action.+id          := name;
        action.+category    := GetConfigString(elem, "Category");
        action.+label       := GetConfigString(elem, "Label");
        action.+description := GetConfigString(elem, "Description");
        action.+scope       := GetConfigString(elem, "Scope");
        action.+params      := GetConfigString(elem, "Params");
        action.+confirm       := GetConfigInt(elem, "Confirm");
        action.+script        := GetConfigString(elem, "Script");
        action.+external_gump := GetConfigInt(elem, "ExternalGump");
        actions.append(action);
        sleepms(2);
    endforeach
    return actions;
endfunction

function EDGetActionsByCategory(category)
    var all      := EDLoadActions();
    var filtered := array{};
    foreach a in all
        if (a.category == category)
            filtered.append(a);
        endif
    endforeach
    return filtered;
endfunction

function EDExecuteAction(dm, action, mob, param_values)
    Start_Script(action.script, array{ dm, mob, param_values });
    SendSysMessage(dm, "[EpicDungeon] " + action.label + " executado.");
    return CInt(action.external_gump);
endfunction
