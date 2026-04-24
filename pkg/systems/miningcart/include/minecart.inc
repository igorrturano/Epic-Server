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
    var lineId := CInt(GetObjProperty(stop, "LineId"));
    if (!lineId) lineId := 1; endif
    
    var nearest := 0;
    var best_dist := 9999;
    
    foreach item in ListItemsNearLocation(stop.x, stop.y, stop.z, 8, stop.realm)
        if (item.objtype == SOUTH_ID || item.objtype == EAST_ID)
            var cartLine := CInt(GetObjProperty(item, "LineId"));
            if (!cartLine) cartLine := 1; endif
            
            if (cartLine == lineId)
                var d := Distance(stop, item);
                if (d < best_dist)
                    best_dist := d;
                    nearest := item;
                endif
            endif
        endif
        sleepms(2);
    endforeach
    return nearest;
endfunction

function FindTargetStop(stop)
    var lineId := CInt(GetObjProperty(stop, "LineId"));
    var terminal := CInt(GetObjProperty(stop, "Terminal"));
    var targetTerminal := 1;
    if (terminal == 1) targetTerminal := 2; endif
    
    var stops := GetGlobalTrackStops();
    var best_stop := 0;
    var best_dist := 9999999;
    
    foreach s_serial in stops
        var s := SystemFindObjectBySerial(s_serial);
        if (s && s != stop)
            var sLine := CInt(GetObjProperty(s, "LineId"));
            var sTerm := CInt(GetObjProperty(s, "Terminal"));
            
            if (sLine == lineId && sTerm == targetTerminal)
                var d := 9999999; // Alta penalidade para outros mapas
                if (s.realm == stop.realm)
                    d := Distance(stop, s);
                endif
                if (d < best_dist)
                    best_dist := d;
                    best_stop := s;
                endif
            endif
        endif
        sleepms(2);
    endforeach
    return best_stop;
endfunction

function IsTrackAt(x, y, realm, lineId)
    var items := ListItemsNearLocation(x, y, LIST_IGNORE_Z, 0, realm);
    foreach item in items
        var isTrack := GetObjProperty(item, "IsTrackTile") || GetObjProperty(item, "IsTrackStop");
        if (isTrack)
            var tLine := CInt(GetObjProperty(item, "LineId"));
            if (!tLine) tLine := 1; endif
            if (tLine == lineId)
                return item.z;
            endif
        endif
    endforeach
    return error;
endfunction

function FindTelepadAround(x, y, realm)
    foreach item in ListItemsNearLocation(x, y, LIST_IGNORE_Z, 1, realm)
        if (item.IsA(POLCLASS_ITEM))
            // Verifica se o item possui a CProp de destino típica de teleporters
            if (GetObjProperty(item, "DestX") != error)
                return item;
            endif
        endif
    endforeach
    return 0;
endfunction

function FindTelepadsAround(x, y, realm)
    var result := array{};
    foreach item in ListItemsNearLocation(x, y, LIST_IGNORE_Z, 1, realm)
        if (item.IsA(POLCLASS_ITEM))
            if (GetObjProperty(item, "DestX") != error || GetObjProperty(item, "Destination") != error)
                result.append(item);
            endif
        endif
    endforeach
    return result;
endfunction