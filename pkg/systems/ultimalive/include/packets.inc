// packets.inc — Serialização dos packets UltimaLive (0x3F e 0x40).
// Include path: ":ultimalive:include/packets"

use uo;
use polsys;
use basic;
use cfgfile;

include ":ultimalive:include/ultimalive";

function _UL_Hex2(b)
    var nibbles := array{"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};
    return nibbles[(b / 16) + 1] + nibbles[(b % 16) + 1];
endfunction

function UL_BuildBlockUpdatePacket(realm, blockX, blockY)
    var realm_info := Realms()[CStr(realm)];
    if (!realm_info)
        return error{"errortext" := "UL_BuildBlockUpdatePacket: realm nao encontrado: " + realm};
    endif

    var map_height_blocks := CInt(realm_info.height) / 8;
    var block_number      := blockX * map_height_blocks + blockY;
    var map_id            := CInt(realm_info.mapid);

    var pkt := CreatePacket(0x40, 201);
    // opcode 0x40 setado automaticamente no byte 0 pelo CreatePacket

    // bytes 1-4: block number (uint32 big-endian)
    pkt.SetInt32(1, block_number);

    // bytes 5-196: land data column-major (x outer 0..7, y inner 0..7)
    // offset = 5 + (x*8 + y) * 3 — mesma lógica de UL_ComputeBlockHash em crc.inc
    var x := 0;
    while (x < 8)
        var y := 0;
        while (y < 8)
            var tile_x := blockX * 8 + x;
            var tile_y := blockY * 8 + y;
            var info    := GetMapInfo(tile_x, tile_y, realm);
            var tile_id := CInt(info.landtile);
            var z       := CInt(info.z);

            var mutation := UL_GetMutation(realm, tile_x, tile_y, 0, 0);
            if (mutation != error && mutation.MutationType == "land_modify")
                tile_id := mutation.Graphic;
            endif

            var offset := 5 + (x * 8 + y) * 3;
            pkt.SetInt8(offset,     tile_id % 256);
            pkt.SetInt8(offset + 1, tile_id / 256);
            if (z < 0)
                z := (1 << 8) + z;
            endif
            pkt.SetInt8(offset + 2, z);

            y += 1;
        endwhile
        x += 1;
    endwhile

    // bytes 197-199: padding explícito — cliente faz Seek(200) absoluto e ignora esses 3 bytes
    pkt.SetInt8(197, 0);
    pkt.SetInt8(198, 0);
    pkt.SetInt8(199, 0);

    // byte 200: mapId (uomapid do realm.cfg)
    pkt.SetInt8(200, map_id);

    return pkt;
endfunction

function UL_SendBlockUpdate(who, realm, blockX, blockY)
    var pkt := UL_BuildBlockUpdatePacket(realm, blockX, blockY);
    if (pkt == error)
        return pkt;
    endif

    var block_number := pkt.GetInt32(1);
    var map_id       := pkt.GetInt8(200);

    Print("UL_SEND realm=" + realm + " blockX=" + blockX + " blockY=" + blockY +
          " block=" + block_number + " mapId=" + map_id + " size=" + pkt.GetSize());

    var i          := 0;
    var line       := "";
    var line_start := 0;
    while (i < 201)
        line := line + _UL_Hex2(pkt.GetInt8(i)) + " ";
        if ((i % 32) == 31 || i == 200)
            Print("UL_SEND bytes[" + line_start + ".." + i + "] " + line);
            line       := "";
            line_start := i + 1;
        endif
        i += 1;
    endwhile

    pkt.SendPacket(who);
    return 1;
endfunction

function _UL_GetShardIdentifier()
    var cfg := ReadConfigFile(":ultimalive:config/ultimalive");
    if (cfg == error)
        return error{"errortext" := "_UL_GetShardIdentifier: falha ao abrir ultimalive.cfg"};
    endif
    var elem := cfg["Settings"];
    if (elem == error)
        return error{"errortext" := "_UL_GetShardIdentifier: elem Settings nao encontrado"};
    endif
    var id := GetConfigString(elem, "ShardIdentifier");
    if (id == error)
        return error{"errortext" := "_UL_GetShardIdentifier: ShardIdentifier ausente"};
    endif
    return id;
endfunction

function UL_BuildLoginConfirmationPacket(shard_id)
    var pkt := CreatePacket(0x3F, 43);
    pkt.SetInt16(1, 43);
    pkt.SetInt8(13, 0x02);
    if (Len(shard_id) > 27)
        shard_id := SubStr(shard_id, 1, 27);
    endif
    pkt.SetString(15, shard_id, 1);
    return pkt;
endfunction

function UL_BuildMapDefinitionPacket()
    var realms_map := Realms();
    var keys       := realms_map.Keys();
    var n          := Len(keys);
    if (n == 0)
        return error{"errortext" := "UL_BuildMapDefinitionPacket: nenhum realm encontrado"};
    endif
    var v       := (n * 9 + 6) / 7;
    var pkt_len := 15 + n * 9;
    var pkt := CreatePacket(0x3F, pkt_len);
    pkt.SetInt16(1, pkt_len);
    pkt.SetInt32(7, v);
    pkt.SetInt8(13, 0x01);
    var offset := 15;
    foreach realm_name in keys
        var realm_info := realms_map[realm_name];
        var map_id     := CInt(realm_info.mapid);
        var width      := CInt(realm_info.width);
        var height     := CInt(realm_info.height);
        pkt.SetInt8 (offset,     map_id);
        pkt.SetInt16(offset + 1, width);
        pkt.SetInt16(offset + 3, height);
        pkt.SetInt16(offset + 5, width);
        pkt.SetInt16(offset + 7, height);
        offset += 9;
    endforeach
    return pkt;
endfunction

function UL_BuildHashQueryPacket(realm, centerBlockX, centerBlockY)
    var realm_info := Realms()[CStr(realm)];
    if (!realm_info)
        return error{"errortext" := "UL_BuildHashQueryPacket: realm nao encontrado: " + realm};
    endif
    var map_height_blocks := CInt(realm_info.height) / 8;
    var center_block      := centerBlockX * map_height_blocks + centerBlockY;
    var map_id            := CInt(realm_info.mapid);
    var pkt := CreatePacket(0x3F, 15);
    pkt.SetInt16(1, 15);
    pkt.SetInt32(3, center_block);
    pkt.SetInt8(13, 0xFF);
    pkt.SetInt8(14, map_id);
    return pkt;
endfunction

function UL_SendHandshake(who)
    var shard_id := _UL_GetShardIdentifier();
    if (shard_id == error)
        return shard_id;
    endif
    var pkt_login := UL_BuildLoginConfirmationPacket(shard_id);
    if (pkt_login == error)
        return pkt_login;
    endif
    pkt_login.SendPacket(who);
    var pkt_map := UL_BuildMapDefinitionPacket();
    if (pkt_map == error)
        return pkt_map;
    endif
    pkt_map.SendPacket(who);
    var blockX         := CInt(who.x) / 8;
    var blockY         := CInt(who.y) / 8;
    var pkt_hash_query := UL_BuildHashQueryPacket(who.realm, blockX, blockY);
    if (pkt_hash_query == error)
        return pkt_hash_query;
    endif
    pkt_hash_query.SendPacket(who);
    return 1;
endfunction

// SUB-FASE 2.A: itera apenas statics nativos via
// ListStaticsAtLocation. Mutações static_add em tiles SEM
// native static não são incluídas no packet.
//
// DÉBITO TÉCNICO: pra fase 3 da árvore destrutível
// (4 pedaços de fallen log em tiles vizinhos), precisará
// iterar DataFile :ultimalive:mutations e adicionar entries
// com MutationType="static_add" deste bloco. Sub-fase 2.D.
function UL_BuildStaticsUpdatePacket(realm, blockX, blockY)
    var realm_info := Realms()[CStr(realm)];
    if (!realm_info)
        return error{"errortext" := "UL_BuildStaticsUpdatePacket: realm nao encontrado: " + realm};
    endif

    var map_height_blocks := CInt(realm_info.height) / 8;
    var block_number      := blockX * map_height_blocks + blockY;
    var map_id            := CInt(realm_info.mapid);

    var statics_list := array{};
    var x := 0;
    while (x < 8)
        var y := 0;
        while (y < 8)
            var tile_x  := blockX * 8 + x;
            var tile_y  := blockY * 8 + y;
            var natives := ListStaticsAtLocation(tile_x, tile_y, LIST_IGNORE_Z, 0, realm);
            foreach st in natives
                var sz       := CInt(st.z);
                var mutation := UL_GetMutation(realm, tile_x, tile_y, sz, CInt(st.objtype));
                var s        := struct{};
                if (mutation == error)
                    s.+graphic := CInt(st.objtype);
                    s.+x_w     := x;
                    s.+y_w     := y;
                    s.+z       := sz;
                    s.+hue     := 0;
                    statics_list.append(s);
                elseif (mutation.MutationType == "static_delete")
                    // omitir — static removido pela mutação
                elseif (mutation.MutationType == "static_modify")
                    s.+graphic := mutation.Graphic;
                    s.+x_w     := x;
                    s.+y_w     := y;
                    s.+z       := sz;
                    s.+hue     := mutation.Hue;
                    statics_list.append(s);
                else
                    // tipo desconhecido — mantém native por segurança
                    s.+graphic := CInt(st.objtype);
                    s.+x_w     := x;
                    s.+y_w     := y;
                    s.+z       := sz;
                    s.+hue     := 0;
                    statics_list.append(s);
                endif
            endforeach
            y += 1;
        endwhile
        x += 1;
    endwhile

    var n       := Len(statics_list);
    var pkt_len := 15 + n * 7;
    var pkt     := CreatePacket(0x3F, pkt_len);

    pkt.SetInt16(1, pkt_len);
    pkt.SetInt32(3, block_number);
    pkt.SetInt32(7, n);
    // bytes 11-12 ficam zero (CreatePacket inicializa com zeros)
    pkt.SetInt8(13, 0x00);
    pkt.SetInt8(14, map_id);

    var i := 0;
    while (i < n)
        var st   := statics_list[i + 1];
        var base := 15 + i * 7;
        var z    := st.z;
        if (z < 0)
            z := (1 << 8) + z;
        endif
        // StaticsBlock é LE (Pack=1) — SetInt16 normal é BE, então escrevemos 2× SetInt8
        pkt.SetInt8(base,     st.graphic % 256);
        pkt.SetInt8(base + 1, st.graphic / 256);
        pkt.SetInt8(base + 2, st.x_w);
        pkt.SetInt8(base + 3, st.y_w);
        pkt.SetInt8(base + 4, z);
        pkt.SetInt8(base + 5, st.hue % 256);
        pkt.SetInt8(base + 6, st.hue / 256);
        i += 1;
    endwhile

    return pkt;
endfunction

function UL_SendStaticsUpdate(who, realm, blockX, blockY)
    var pkt := UL_BuildStaticsUpdatePacket(realm, blockX, blockY);
    if (pkt == error)
        return pkt;
    endif

    pkt.SendPacket(who);
    return 1;
endfunction

function UL_SendPlayerBlockWindow(who)
    var realm_info := Realms()[CStr(who.realm)];
    if (!realm_info)
        return;
    endif
    var max_bx := CInt(realm_info.width)  / 8 - 1;
    var max_by := CInt(realm_info.height) / 8 - 1;
    var cx     := CInt(who.x) / 8;
    var cy     := CInt(who.y) / 8;
    var dx     := -2;
    while (dx <= 2)
        var dy := -2;
        while (dy <= 2)
            var bx := cx + dx;
            var by := cy + dy;
            if (bx >= 0 && bx <= max_bx && by >= 0 && by <= max_by)
                if (UL_IsBlockMutated(who.realm, bx, by))
                    var r := UL_SendStaticsUpdate(who, who.realm, bx, by);
                    if (r == error)
                        Print("UL block window error [" + who.name + "] block(" + bx + "," + by + "): " + r.errortext);
                    endif
                endif
            endif
            dy += 1;
        endwhile
        dx += 1;
    endwhile
endfunction

function UL_MaybeSendBlock(who, bx, by, max_bx, max_by)
    if (bx < 0 || bx > max_bx || by < 0 || by > max_by)
        return;
    endif
    if (UL_IsBlockMutated(who.realm, bx, by))
        var r := UL_SendStaticsUpdate(who, who.realm, bx, by);
        if (r == error)
            Print("UL delta error [" + who.name + "] block(" + bx + "," + by + "): " + r.errortext);
        endif
    endif
endfunction

function UL_SendBlockDelta(who, bx, by, ddx, ddy)
    var realm_info := Realms()[CStr(who.realm)];
    if (!realm_info)
        return;
    endif
    var max_bx := CInt(realm_info.width)  / 8 - 1;
    var max_by := CInt(realm_info.height) / 8 - 1;

    var abs_ddx := ddx;
    var abs_ddy := ddy;
    if (abs_ddx < 0) abs_ddx := -abs_ddx; endif
    if (abs_ddy < 0) abs_ddy := -abs_ddy; endif
    if (abs_ddx > 2 || abs_ddy > 2)
        UL_SendPlayerBlockWindow(who);
        return;
    endif

    if (ddx > 0)
        var sy := -2;
        while (sy <= 2)
            UL_MaybeSendBlock(who, bx + 2, by + sy, max_bx, max_by);
            sy := sy + 1;
        endwhile
    endif

    if (ddx < 0)
        var sy := -2;
        while (sy <= 2)
            UL_MaybeSendBlock(who, bx - 2, by + sy, max_bx, max_by);
            sy := sy + 1;
        endwhile
    endif

    if (ddy > 0)
        var sx := -2;
        while (sx <= 2)
            UL_MaybeSendBlock(who, bx + sx, by + 2, max_bx, max_by);
            sx := sx + 1;
        endwhile
    endif

    if (ddy < 0)
        var sx := -2;
        while (sx <= 2)
            UL_MaybeSendBlock(who, bx + sx, by - 2, max_bx, max_by);
            sx := sx + 1;
        endwhile
    endif
endfunction

function UL_BuildHashResponsePacket(realm, centerBlockX, centerBlockY)
    return error{"errortext" := "STUB - sub-fase 1.E"};
endfunction

function UL_HandleHashQuery(player, packet)
    return error{"errortext" := "STUB - sub-fase 1.E"};
endfunction

function UL_NotifyNearbyPlayers(realm, bx, by)
    foreach who in EnumerateOnlineCharacters()
        if (CStr(who.realm) == CStr(realm))
            var dx := CInt(who.x) / 8 - bx;
            var dy := CInt(who.y) / 8 - by;
            if (dx < 0) dx := -dx; endif
            if (dy < 0) dy := -dy; endif
            if (dx <= 2 && dy <= 2)
                UL_SendStaticsUpdate(who, realm, bx, by);
            endif
        endif
    endforeach
endfunction
