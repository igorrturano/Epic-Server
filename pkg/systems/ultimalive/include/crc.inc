// crc.inc — Checksum Fletcher-16 para blocos de mapa UltimaLive.
// Include path: ":ultimalive:include/crc"

use uo;
use basic;

include ":ultimalive:include/ultimalive";

// Calcula Fletcher-16 de um array de inteiros (0–255).
// Usa mod 255 — é o que o cliente (ClassicUO/UltimaLive.cs) implementa.
// Retorna uint16: (sum2 << 8) | sum1.
function UL_Fletcher16(byte_array)
    var sum1 := 0;
    var sum2 := 0;
    foreach b in byte_array
        sum1 := (sum1 + b) % 255;
        sum2 := (sum2 + sum1) % 255;
    endforeach
    return (sum2 << 8) | sum1;
endfunction

// Calcula o hash dos 64 land tiles de um bloco 8×8, produzindo os mesmos
// 192 bytes que o cliente lê do map.mul:
//   - column-major (x outer 0..7, y inner 0..7): é como o .mul armazena blocos
//   - por tile: low byte de tile_id, high byte de tile_id (little-endian), byte de z
//   - z signed: se z < 0, converte para unsigned via (1 << 8) + z
// Statics não incluídos — decisão sub-fase 1.A, apenas land por enquanto.
function UL_ComputeBlockHash(realm, blockX, blockY)
    var bytes := array{};
    var x := 0;
    while (x < 8)
        var y := 0;
        while (y < 8)
            var info := GetMapInfo(blockX * 8 + x, blockY * 8 + y, realm);
            var tile_id := CInt(info.landtile);
            var z := CInt(info.z);

            // Aplica mutação de land se registrada; z=0 é convenção UltimaLive pra land.
            var mutation := UL_GetMutation(realm, blockX * 8 + x, blockY * 8 + y, 0, 0);
            if (mutation != error && mutation.MutationType == "land_modify")
                tile_id := mutation.Graphic;
            endif

            bytes.append(tile_id % 256);
            bytes.append(tile_id / 256);
            if (z < 0)
                z := (1 << 8) + z;
            endif
            bytes.append(z);
            y += 1;
        endwhile
        x += 1;
    endwhile
    return UL_Fletcher16(bytes);
endfunction
