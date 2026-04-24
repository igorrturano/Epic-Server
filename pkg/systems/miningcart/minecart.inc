use uo;
use os;
use math;

const SOUTH_ID := 0xBB42;
const EAST_ID := 0xBB43;

function GetGlobalTrackStops()
    var stops := GetGlobalProperty("TrackStopsRegistry");
    if (!stops)
        stops := array{};
        SetGlobalProperty("TrackStopsRegistry", stops);
    endif
    return stops;
endfunction

function RegisterTrackStop(stop)
    var stops := GetGlobalTrackStops();
    if (!(stop.serial in stops))
        stops.append(stop.serial);
        SetGlobalProperty("TrackStopsRegistry", stops);
    endif
endfunction

function UnregisterTrackStop(stop)
    var stops := GetGlobalTrackStops();
    var idx := 0;
    foreach s in stops
        if (s == stop.serial)
            idx := _s_iter;
            break;
        endif
    endforeach
    if (idx)
        stops.erase(idx);
        SetGlobalProperty("TrackStopsRegistry", stops);
    endif
endfunction

function FindNearestMineCart(stop)
    var nearest := 0;
    var best_dist := 9999;
    
    // Busca qualquer carrinho a até 2 tiles de distância, ignorando de qual linha ele "veio"
    foreach item in ListItemsNearLocation(stop.x, stop.y, stop.z, 2, stop.realm)
        if (item.objtype == SOUTH_ID || item.objtype == EAST_ID)
            var d := Distance(stop, item);
            if (d < best_dist)
                best_dist := d;
                nearest := item;
            endif
        endif
        sleepms(2);
    endforeach
    return nearest;
endfunction

function FindTargetStops(stop)
    var lineId := GetObjProperty(stop, "LineId");
    if (!lineId) lineId := "1"; endif
    lineId := CStr(lineId); // Garante que seja tratado como string para permitir textos

    var terminal := CInt(GetObjProperty(stop, "Terminal"));
    var targetTerminal := 1;
    if (terminal == 1) targetTerminal := 2; endif
    
    var stops := GetGlobalTrackStops();
    var valid_stops := array{};
    
    foreach s_serial in stops
        var s := SystemFindObjectBySerial(s_serial);
        if (s && s != stop)
            var sLine := GetObjProperty(s, "LineId");
            if (!sLine) sLine := "1"; endif
            sLine := CStr(sLine);

            var sTerm := CInt(GetObjProperty(s, "Terminal"));
            
            if (sLine == lineId && sTerm == targetTerminal)
                valid_stops.append(s);
            endif
        endif
        sleepms(2);
    endforeach
    return valid_stops;
endfunction

function IsTrackAt(x, y, realm)
    var items := ListItemsNearLocation(x, y, LIST_IGNORE_Z, 0, realm);
    foreach item in items
        var isTrack := GetObjProperty(item, "IsTrackTile") || GetObjProperty(item, "IsTrackStop");
        if (isTrack)
            return item.z; // Não verifica mais a linha do trilho, trilhos conectados formam o caminho.
        endif
    endforeach
    return error;
endfunction

function FindTelepadsAround(x, y, realm)
    var pads := array{};
    // O raio foi reduzido de 2 para 0 para evitar que o carrinho "puxe" telepads próximos (como teleporters de casa) acidentalmente.
    foreach item in ListItemsNearLocation(x, y, LIST_IGNORE_Z, 0, realm)
        if (item.IsA(POLCLASS_ITEM))
            var dest := GetObjProperty(item, "Destination");
            if (dest != error && dest.x != error)
                pads.append(item);
                continue;
            endif
            
            var dX := GetObjProperty(item, "DestX");
            if (dX == error) dX := GetObjProperty(item, "destx"); endif
            
            if (dX != error)
                pads.append(item);
            endif
        endif
    endforeach
    return pads;
endfunction