// packets.inc — Serialização dos packets UltimaLive (0x3F e 0x40).
// Include path: ":ultimalive:include/packets"

use uo;

// Constrói packet 0x40 com land tiles e statics de um bloco.
function UL_BuildBlockUpdatePacket(realm, blockX, blockY)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction

// Constrói resposta 0x3F com os hashes de uma grade 5×5 centrada em centerBlockX/Y.
function UL_BuildHashResponsePacket(realm, centerBlockX, centerBlockY)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction

// Processa packet 0x3F recebido do cliente.
function UL_HandleHashQuery(player, packet)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction
