use uo;
use os;
use polsys;

const ED_ROSTER_KEY := "RaidRoster_";

function EDGetRoster(dm)
    var roster := GetGlobalProperty(ED_ROSTER_KEY + dm.serial);
    if (roster == error || !roster)
        return array{};
    endif
    return roster;
endfunction

function EDAddPlayer(dm, serial)
    var roster := EDGetRoster(dm);
    foreach s in roster
        if (s == serial)
            return roster;
        endif
    endforeach
    roster.append(serial);
    SetGlobalProperty(ED_ROSTER_KEY + dm.serial, roster);
    return roster;
endfunction

function EDRemovePlayer(dm, serial)
    var roster  := EDGetRoster(dm);
    var updated := array{};
    foreach s in roster
        if (s != serial)
            updated.append(s);
        endif
    endforeach
    SetGlobalProperty(ED_ROSTER_KEY + dm.serial, updated);
    return updated;
endfunction

function EDClearRoster(dm)
    SetGlobalProperty(ED_ROSTER_KEY + dm.serial, array{});
    return array{};
endfunction

// Retorna array de ObjRefs de players online no roster
function EDResolveMobs(dm)
    var roster := EDGetRoster(dm);
    var mobs   := array{};
    foreach serial in roster
        var mob := SystemFindObjectBySerial(serial, SYSFIND_SEARCH_OFFLINE_MOBILES);
        if (mob != error && mob && mob.connected)
            mobs.append(mob);
        endif
        sleepms(2);
    endforeach
    return mobs;
endfunction

function EDPlayerLabel(mob)
    var nome := GetObjProperty(mob, "nome_inicial");
    if (!nome) nome := mob.name; endif
    return mob.acctname + " - " + mob.title_race + " [" + nome + "]";
endfunction

// Retorna array de structs para renderizar na sidebar
function EDGetRosterEntries(dm)
    var roster  := EDGetRoster(dm);
    var entries := array{};
    foreach serial in roster
        var mob   := SystemFindObjectBySerial(serial, SYSFIND_SEARCH_OFFLINE_MOBILES);
        var entry := struct{};
        entry.+serial := serial;
        if (mob == error || !mob)
            entry.+name   := "#" + serial;
            entry.+label  := "?";
            entry.+col    := 32;
        elseif (!mob.connected)
            entry.+name  := mob.acctname + " - " + mob.title_race;
            var nome_off := GetObjProperty(mob, "nome_inicial");
            if (!nome_off) nome_off := mob.name; endif
            entry.+label := "[" + nome_off + "] OFFLINE";
            entry.+col   := 32;
        elseif (mob.dead)
            entry.+name  := mob.acctname + " - " + mob.title_race;
            var nome_dead := GetObjProperty(mob, "nome_inicial");
            if (!nome_dead) nome_dead := mob.name; endif
            entry.+label := "[" + nome_dead + "] MORTO";
            entry.+col   := 34;
        else
            var maxhp := CInt(mob.maxhits);
            if (maxhp <= 0) maxhp := 1; endif
            var pct := CInt(CInt(mob.hits) * 100 / maxhp);
            entry.+name  := mob.acctname + " - " + mob.title_race;
            var nome_hp  := GetObjProperty(mob, "nome_inicial");
            if (!nome_hp) nome_hp := mob.name; endif
            entry.+label := "[" + nome_hp + "] " + pct + "%";
            if (pct > 50)
                entry.+col := 1153;
            elseif (pct > 25)
                entry.+col := 53;
            else
                entry.+col := 34;
            endif
        endif
        entries.append(entry);
        sleepms(2);
    endforeach
    return entries;
endfunction
