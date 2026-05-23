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

            var mutation := UL_GetMutation(realm, tile_x, tile_y, 0);
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

function UL_BuildHashResponsePacket(realm, centerBlockX, centerBlockY)
    return error{"errortext" := "STUB - sub-fase 1.E"};
endfunction

function UL_HandleHashQuery(player, packet)
    return error{"errortext" := "STUB - sub-fase 1.E"};
endfunction
