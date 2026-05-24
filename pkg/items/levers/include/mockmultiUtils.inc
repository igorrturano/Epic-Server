use uo;
use os;
use util;

const VEL_LENTO  := 0;
const VEL_NORMAL := 1;
const VEL_RAPIDO := 2;

const CLOUD_Z_THRESHOLD := 30;
const CLOUD_SPACING     := 4;

function CarregarSerials(groupId)
    var gdata := GetGlobalProperty("mmGroup_" + groupId);
    if (!gdata || gdata.errortext) return array{}; endif
    if (!gdata.serials)           return array{}; endif
    return gdata.serials;
endfunction

function BaseZAtual(serials)
    var minZ := 32767;
    foreach serial in serials
        var item := SystemFindObjectBySerial(serial);
        if (item && item.z < minZ) minZ := item.z; endif
    endforeach
    if (minZ == 32767) minZ := 0; endif
    return minZ;
endfunction

function VelStep(vel)
    if (vel == VEL_LENTO)  return 1; endif
    if (vel == VEL_RAPIDO) return 5; endif
    return 2;
endfunction

function VelDelay(vel)
    if (vel == VEL_LENTO)  return 200; endif
    if (vel == VEL_RAPIDO) return 50;  endif
    return 100;
endfunction

function _AnimarSerials(serials, shift, vel_step, delay)
    if (shift == 0) return; endif
    var dir      := shift > 0 ? 1 : -1;
    var restante := shift > 0 ? shift : -shift;
    while (restante > 0)
        var passo := restante < vel_step ? restante : vel_step;
        foreach serial in serials
            var tile := SystemFindObjectBySerial(serial);
            if (tile)
                MoveObjectToLocation(tile, tile.x, tile.y, tile.z + (passo * dir), tile.realm, MOVEOBJECT_FORCELOCATION);
            endif
        endforeach
        restante -= passo;
        SleepMS(delay);
    endwhile
endfunction

function _BoundingBox(serials)
    var min_x := 32767; var max_x := -32767;
    var min_y := 32767; var max_y := -32767;
    var min_z := 32767;
    foreach serial in serials
        var t := SystemFindObjectBySerial(serial);
        if (t)
            if (t.x < min_x) min_x := t.x; endif
            if (t.x > max_x) max_x := t.x; endif
            if (t.y < min_y) min_y := t.y; endif
            if (t.y > max_y) max_y := t.y; endif
            if (t.z < min_z) min_z := t.z; endif
        endif
    endforeach
    if (min_x == 32767) return 0; endif
    return struct{ min_x := min_x, max_x := max_x, min_y := min_y, max_y := max_y, min_z := min_z };
endfunction

function _CriarNuvens(serials)
    var bb := _BoundingBox(serials);
    if (!bb) return array{}; endif

    var realm := 0;
    foreach serial in serials
        var t := SystemFindObjectBySerial(serial);
        if (t) realm := t.realm; break; endif
    endforeach

    var gz       := bb.min_z - 2;
    var graficos := array{ 0xc21c, 0xc21d, 0xc21e, 0xc223, 0xd4a4 };
    var nuvens   := array{};

    var x := bb.min_x;
    while (x <= bb.max_x)
        var n1 := CreateItemAtLocation(x, bb.min_y, gz, graficos[RandomInt(graficos.size()) + 1], 1, realm);
        var n2 := CreateItemAtLocation(x, bb.max_y, gz, graficos[RandomInt(graficos.size()) + 1], 1, realm);
        if (n1 && !n1.errortext) n1.movable := 0; nuvens.append(n1.serial); endif
        if (n2 && !n2.errortext) n2.movable := 0; nuvens.append(n2.serial); endif
        x += CLOUD_SPACING;
    endwhile

    var y := bb.min_y + CLOUD_SPACING;
    while (y < bb.max_y)
        var n3 := CreateItemAtLocation(bb.min_x, y, gz, graficos[RandomInt(graficos.size()) + 1], 1, realm);
        var n4 := CreateItemAtLocation(bb.max_x, y, gz, graficos[RandomInt(graficos.size()) + 1], 1, realm);
        if (n3 && !n3.errortext) n3.movable := 0; nuvens.append(n3.serial); endif
        if (n4 && !n4.errortext) n4.movable := 0; nuvens.append(n4.serial); endif
        y += CLOUD_SPACING;
    endwhile

    return nuvens;
endfunction

function _DestruirNuvens(cloud_serials)
    if (!cloud_serials) return; endif
    foreach serial in cloud_serials
        var n := SystemFindObjectBySerial(serial);
        if (n) DestroyItem(n); endif
    endforeach
endfunction

function _AtualizarDestino(serial, shift)
    if (!serial) return; endif
    var it := SystemFindObjectBySerial(serial);
    if (!it) return; endif
    var dest := GetObjProperty(it, "Destination");
    if (!dest || dest.errortext) return; endif
    dest.z := dest.z + shift;
    SetObjProperty(it, "Destination", dest);
endfunction

function _CriarCordaNoMundo(who, volta)
    var tipo := volta ? "Corda de Volta" : "Corda de Subir";

    SendSysMessage(who, tipo + ": clique onde a corda vai spawnar.");
    var spawn := TargetCoordinates(who);
    if (!spawn)
        SendSysMessage(who, "Cancelado.");
        return 0;
    endif

    SendSysMessage(who, tipo + ": clique onde o jogador vai aparecer.");
    var destino := TargetCoordinates(who);
    if (!destino)
        SendSysMessage(who, "Cancelado.");
        return 0;
    endif

    var corda := CreateItemAtLocation(spawn.x, spawn.y, spawn.z, 0xfc0, 1, spawn.realm);
    if (!corda || corda.errortext)
        SendSysMessage(who, "Falha ao criar " + tipo + ".");
        return 0;
    endif

    corda.movable   := 0;
    corda.invisible := 0;
    SetObjProperty(corda, "Destination", struct{ x := destino.x, y := destino.y, z := destino.z, realm := destino.realm });
    SetObjProperty(corda, "direcao", volta ? "down" : "up");

    SendSysMessage(who, tipo + " criada: Z=" + spawn.z + " -> Z=" + destino.z + ".");
    return struct{ corda := corda, spawn := spawn, destino := destino };
endfunction
