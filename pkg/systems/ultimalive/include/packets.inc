// packets.inc — Serialização dos packets UltimaLive (0x3F e 0x40).
// Include path: ":ultimalive:include/packets"

use uo;
use polsys;
use basic;

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

function UL_BuildHashResponsePacket(realm, centerBlockX, centerBlockY)
    return error{"errortext" := "STUB - sub-fase 1.E"};
endfunction

function UL_HandleHashQuery(player, packet)
    return error{"errortext" := "STUB - sub-fase 1.E"};
endfunction
