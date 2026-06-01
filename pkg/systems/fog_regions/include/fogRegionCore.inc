// fogRegionCore.inc — API do sistema fog regions: datafile CRUD, packets, preview.
// Include path: ":fog_regions:include/fogRegionCore"

use uo;
use os;
use util;
use datafile;
use polsys;

// ── Constantes ────────────────────────────────────────────────────────────────

const FOG_REGION_DATA_FILE      := ":fog_regions:regions";
const FOG_REGION_STATUS_ACTIVE  := 1;
const FOG_REGION_STATUS_STOPPED := 0;
const FOG_REGION_MODE_PERMANENT := "permanent";
const FOG_REGION_MODE_TIMED     := "timed";

// ── Helpers ───────────────────────────────────────────────────────────────────

// Retorna 1 se (x, y) está dentro do retângulo [x1,y1]→[x2,y2].
function InRect(x, y, x1, y1, x2, y2)
    return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
endfunction

// Abre (ou cria) o datafile de regiões.
function _OpenRegionsDF()
    var df := OpenDataFile(FOG_REGION_DATA_FILE);
    if (!df)
        df := CreateDataFile(FOG_REGION_DATA_FILE, DF_KEYTYPE_STRING);
    endif
    return df;
endfunction

// Converte elemento do datafile em struct de região.
function _ElemToStruct(elem, name)
    var r := struct;
    r.+Name       := name;
    r.+X1         := CInt(elem.GetProp("X1"));
    r.+Y1         := CInt(elem.GetProp("Y1"));
    r.+X2         := CInt(elem.GetProp("X2"));
    r.+Y2         := CInt(elem.GetProp("Y2"));
    r.+Realm      := CStr(elem.GetProp("Realm"));
    r.+Status     := CInt(elem.GetProp("Status"));
    r.+Mode       := CStr(elem.GetProp("Mode"));
    r.+DensityInt := CInt(elem.GetProp("DensityInt"));
    r.+Radius     := CInt(elem.GetProp("Radius"));
    r.+Falloff    := CInt(elem.GetProp("Falloff"));
    r.+VMinInt    := CInt(elem.GetProp("VMinInt"));
    r.+VMaxInt    := CInt(elem.GetProp("VMaxInt"));
    r.+NoiseInt   := CInt(elem.GetProp("NoiseInt"));
    r.+ScaleInt   := CInt(elem.GetProp("ScaleInt"));
    r.+SpeedInt   := CInt(elem.GetProp("SpeedInt"));
    r.+DarkInt    := CInt(elem.GetProp("DarkInt"));
    r.+ColorR     := CInt(elem.GetProp("ColorR"));
    r.+ColorG     := CInt(elem.GetProp("ColorG"));
    r.+ColorB     := CInt(elem.GetProp("ColorB"));
    r.+DesatInt   := CInt(elem.GetProp("DesatInt"));
    r.+PresetName := CStr(elem.GetProp("PresetName"));
    r.+TimerEnd   := CInt(elem.GetProp("TimerEnd"));
    // FOG-FASE5/F: fallback para regiões criadas antes do suporte a Z-height.
    var zmin_prop := elem.GetProp("WorldZMin");
    if (zmin_prop.errortext)
        r.+WorldZMin := -128;
    else
        r.+WorldZMin := CInt(zmin_prop);
    endif
    var zmax_prop := elem.GetProp("WorldZMax");
    if (zmax_prop.errortext)
        r.+WorldZMax := 127;
    else
        r.+WorldZMax := CInt(zmax_prop);
    endif
    var zfade_prop := elem.GetProp("FalloffZ"); // FOG-FASE6/G
    if (zfade_prop.errortext)
        r.+FalloffZ := 10;
    else
        r.+FalloffZ := CInt(zfade_prop);
    endif
    return r;
endfunction

// ── API pública ───────────────────────────────────────────────────────────────

// Carrega todas as regiões do data store. Retorna array de structs (pode ser vazio).
function LoadRegions()
    var df := _OpenRegionsDF();
    if (!df)
        return array{};
    endif
    var result := array{};
    var keys := df.Keys();
    foreach key in keys
        var elem := df.FindElement(key);
        if (elem)
            result.append(_ElemToStruct(elem, CStr(key)));
        endif
    endforeach
    return result;
endfunction

// Busca região pelo nome. Retorna struct ou error se não encontrada.
function GetRegionByName(name)
    var df := _OpenRegionsDF();
    if (!df)
        return error{"errortext" := "data file não encontrado"};
    endif
    var elem := df.FindElement(name);
    if (!elem)
        return error{"errortext" := "região '" + name + "' não encontrada"};
    endif
    return _ElemToStruct(elem, name);
endfunction

// Retorna 1 se região está logicamente ativa.
// Retorna 0 se Status=STOPPED ou se é timed e TimerEnd já passou.
function IsRegionLogicallyActive(region)
    if (region.Status != FOG_REGION_STATUS_ACTIVE)
        return 0;
    endif
    if (region.Mode == FOG_REGION_MODE_TIMED)
        if (polcore().systime >= region.TimerEnd)
            return 0;
        endif
    endif
    return 1;
endfunction

// Retorna a primeira região logicamente ativa que contém (x, y, realm).
// Retorna error se nenhuma encontrada.
function GetRegionAtPosition(x, y, realm)
    var regions := LoadRegions();
    foreach r in regions
        if (!IsRegionLogicallyActive(r))
            continue;
        endif
        if (Lower(r.Realm) != Lower(realm))
            continue;
        endif
        if (InRect(x, y, r.X1, r.Y1, r.X2, r.Y2))
            return r;
        endif
    endforeach
    return error{"errortext" := "nenhuma fog region nesta posição"};
endfunction

// Salva (cria ou sobrescreve) uma região no data store.
function SaveRegion(region_struct)
    var df := _OpenRegionsDF();
    if (!df)
        return error{"errortext" := "data file não encontrado"};
    endif
    var elem := df.FindElement(region_struct.Name);
    if (!elem)
        elem := df.CreateElement(region_struct.Name);
    endif
    if (elem.errortext)
        return elem;
    endif
    elem.SetProp("X1",         region_struct.X1);
    elem.SetProp("Y1",         region_struct.Y1);
    elem.SetProp("X2",         region_struct.X2);
    elem.SetProp("Y2",         region_struct.Y2);
    elem.SetProp("Realm",      region_struct.Realm);
    elem.SetProp("Status",     region_struct.Status);
    elem.SetProp("Mode",       region_struct.Mode);
    elem.SetProp("DensityInt", region_struct.DensityInt);
    elem.SetProp("Radius",     region_struct.Radius);
    elem.SetProp("Falloff",    region_struct.Falloff);
    elem.SetProp("VMinInt",    region_struct.VMinInt);
    elem.SetProp("VMaxInt",    region_struct.VMaxInt);
    elem.SetProp("NoiseInt",   region_struct.NoiseInt);
    elem.SetProp("ScaleInt",   region_struct.ScaleInt);
    elem.SetProp("SpeedInt",   region_struct.SpeedInt);
    elem.SetProp("DarkInt",    region_struct.DarkInt);
    elem.SetProp("ColorR",     region_struct.ColorR);
    elem.SetProp("ColorG",     region_struct.ColorG);
    elem.SetProp("ColorB",     region_struct.ColorB);
    elem.SetProp("DesatInt",   region_struct.DesatInt);
    elem.SetProp("PresetName", region_struct.PresetName);
    elem.SetProp("TimerEnd",   region_struct.TimerEnd);
    elem.SetProp("WorldZMin",  region_struct.WorldZMin); // FOG-FASE5/F
    elem.SetProp("WorldZMax",  region_struct.WorldZMax); // FOG-FASE5/F
    elem.SetProp("FalloffZ",   region_struct.FalloffZ);  // FOG-FASE6/G
    return 1;
endfunction

// Remove uma região do data store pelo nome.
function DeleteRegion(name)
    var df := _OpenRegionsDF();
    if (!df)
        return error{"errortext" := "data file não encontrado"};
    endif
    return df.DeleteElement(name);
endfunction

// ── Packet helpers ────────────────────────────────────────────────────────────

function _Fog_SetInt16BE(byref packet, offset, value)
    packet.SetInt8(offset,     CInt(value / 256));
    packet.SetInt8(offset + 1, value % 256);
endfunction

// FOG-FASE5/F: escreve Int16 BE com suporte a valores negativos (two's complement).
function _Fog_SetInt16BESigned(byref packet, offset, value)
    var uval := value;
    if (uval < 0)
        uval := uval + 65536;
    endif
    packet.SetInt8(offset,     CInt(uval / 256));
    packet.SetInt8(offset + 1, uval % 256);
endfunction

// Envia packet 0x0070 v6 com parâmetros customizados para o GM (não persiste).
function SendFogTestPacket(who, params)
    var pkt := CreatePacket(0xD7, 35); // FOG-FASE6/G: v6 = 35 bytes
    pkt.SetInt8(0, 0xD7);
    _Fog_SetInt16BE(pkt, 1,  35); // FOG-FASE6/G
    _Fog_SetInt16BE(pkt, 3,  0x0070);
    pkt.SetInt8(5, 1);
    _Fog_SetInt16BE(pkt, 6,  params.density_int);
    _Fog_SetInt16BE(pkt, 8,  params.radius);
    _Fog_SetInt16BE(pkt, 10, params.falloff);
    _Fog_SetInt16BE(pkt, 12, params.vmin_int);
    _Fog_SetInt16BE(pkt, 14, params.vmax_int);
    _Fog_SetInt16BE(pkt, 16, params.noise_int);
    _Fog_SetInt16BE(pkt, 18, params.scale_int);
    _Fog_SetInt16BE(pkt, 20, params.speed_int);
    _Fog_SetInt16BE(pkt, 22, params.dark_int);
    pkt.SetInt8(24, params.color_r);
    pkt.SetInt8(25, params.color_g);
    pkt.SetInt8(26, params.color_b);
    _Fog_SetInt16BE(pkt, 27, params.desat_int);
    _Fog_SetInt16BESigned(pkt, 29, params.world_z_min);  // FOG-FASE5/F
    _Fog_SetInt16BESigned(pkt, 31, params.world_z_max);  // FOG-FASE5/F
    _Fog_SetInt16BE(pkt, 33, params.world_z_fade);        // FOG-FASE6/G
    pkt.SendPacket(who);
endfunction

// Desliga fog para o GM (usado ao fechar editor manual).
function SendFogOffToGm(who)
    var pkt := CreatePacket(0xD7, 29);
    pkt.SetInt8(0, 0xD7);
    _Fog_SetInt16BE(pkt, 1, 29);
    _Fog_SetInt16BE(pkt, 3, 0x0070);
    pkt.SetInt8(5, 0);
    pkt.SendPacket(who);
endfunction

// Envia packet 0x0070 v6 com parâmetros de uma região para o player (watcher).
function SendFogPacketForRegion(who, region)
    var pkt := CreatePacket(0xD7, 35); // FOG-FASE6/G: v6 = 35 bytes
    pkt.SetInt8(0, 0xD7);
    _Fog_SetInt16BE(pkt, 1,  35); // FOG-FASE6/G
    _Fog_SetInt16BE(pkt, 3,  0x0070);
    pkt.SetInt8(5, 1);
    _Fog_SetInt16BE(pkt, 6,  region.DensityInt);
    _Fog_SetInt16BE(pkt, 8,  region.Radius);
    _Fog_SetInt16BE(pkt, 10, region.Falloff);
    _Fog_SetInt16BE(pkt, 12, region.VMinInt);
    _Fog_SetInt16BE(pkt, 14, region.VMaxInt);
    _Fog_SetInt16BE(pkt, 16, region.NoiseInt);
    _Fog_SetInt16BE(pkt, 18, region.ScaleInt);
    _Fog_SetInt16BE(pkt, 20, region.SpeedInt);
    _Fog_SetInt16BE(pkt, 22, region.DarkInt);
    pkt.SetInt8(24, region.ColorR);
    pkt.SetInt8(25, region.ColorG);
    pkt.SetInt8(26, region.ColorB);
    _Fog_SetInt16BE(pkt, 27, region.DesatInt);
    _Fog_SetInt16BESigned(pkt, 29, region.WorldZMin); // FOG-FASE5/F
    _Fog_SetInt16BESigned(pkt, 31, region.WorldZMax); // FOG-FASE5/F
    _Fog_SetInt16BE(pkt, 33, region.FalloffZ);        // FOG-FASE6/G
    pkt.SendPacket(who);
endfunction

// Desliga fog do player (watcher, não o editor GM).
function SendFogOff(who)
    var pkt := CreatePacket(0xD7, 29);
    pkt.SetInt8(0, 0xD7);
    _Fog_SetInt16BE(pkt, 1, 29);
    _Fog_SetInt16BE(pkt, 3, 0x0070);
    pkt.SetInt8(5, 0);
    pkt.SendPacket(who);
endfunction

// ── Preview de retângulo ──────────────────────────────────────────────────────

// Marca as 4 bordas do retângulo com marcadores visuais (objtype 0xF240).
function _FRPreviewRect(x1, y1, x2, y2, realm)
    var markers := array{};
    var z;
    var m;

    var x := x1;
    while (x <= x2)
        z := GetWorldHeight(x, y1, realm);
        m := CreateItemAtLocation(x, y1, z, 0xF240, 1, realm);
        if (m) markers.append(m); endif
        x += 1;
        sleepms(1);
    endwhile

    x := x1;
    while (x <= x2)
        z := GetWorldHeight(x, y2, realm);
        m := CreateItemAtLocation(x, y2, z, 0xF240, 1, realm);
        if (m) markers.append(m); endif
        x += 1;
        sleepms(1);
    endwhile

    var y := y1 + 1;
    while (y < y2)
        z := GetWorldHeight(x1, y, realm);
        m := CreateItemAtLocation(x1, y, z, 0xF240, 1, realm);
        if (m) markers.append(m); endif
        y += 1;
        sleepms(1);
    endwhile

    y := y1 + 1;
    while (y < y2)
        z := GetWorldHeight(x2, y, realm);
        m := CreateItemAtLocation(x2, y, z, 0xF240, 1, realm);
        if (m) markers.append(m); endif
        y += 1;
        sleepms(1);
    endwhile

    return markers;
endfunction

function _FRClearPreview(markers)
    foreach item in markers
        DestroyItem(item);
        sleepms(1);
    endforeach
endfunction

// Limpa marcadores de preview armazenados em ObjProperty do player.
// Chamada no logon (crash recovery) e logoff.
function CleanupFogPreviewMarkers(who)
    var serials := GetObjProperty(who, "#fogPreviewMarkers");
    if (!serials)
        return;
    endif
    foreach serial in serials
        var item := SystemFindObjectBySerial(serial);
        if (item)
            DestroyItem(item);
            sleepms(1);
        endif
    endforeach
    EraseObjProperty(who, "#fogPreviewMarkers");
endfunction
