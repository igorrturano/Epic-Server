// trees_meta.inc — Mapeamento estático de tipos de árvore (trunk/foliage) para UltimaLive.
// Include path: ":ultimalive:include/trees_meta"

use basic;

function UL_GetTreeMeta(graphic)
    var trunk     := 0;
    var foliage   := 0;
    var type_name := "";
    var log_type  := 0;

    case (CInt(graphic))
        0xCD6:
        0xCD7:
            trunk     := 0xCD6;
            foliage   := 0xCD7;
            type_name := "Pinheiro";
            log_type  := 0xEE36;
        0xCD8:
        0xCD9:
            trunk     := 0xCD8;
            foliage   := 0xCD9;
            type_name := "Pinheiro";
            log_type  := 0xEE36;
        0xCDA:
        0xCDB:
            trunk     := 0xCDA;
            foliage   := 0xCDB;
            type_name := "Carvalho";
            log_type  := 0xEE37;
        0xCDD:
        0xCDE:
            trunk     := 0xCDD;
            foliage   := 0xCDE;
            type_name := "Carvalho";
            log_type  := 0xEE37;
        0xCE0:
        0xCE1:
            trunk     := 0xCE0;
            foliage   := 0xCE1;
            type_name := "Nogueira";
            log_type  := 0xEE38;
        0xCE3:
        0xCE4:
            trunk     := 0xCE3;
            foliage   := 0xCE4;
            type_name := "Nogueira";
            log_type  := 0xEE38;
        0xCE6:
        0xCE7:
            trunk     := 0xCE6;
            foliage   := 0xCE7;
            type_name := "Salgueiro";
            log_type  := 0xEE39;
        0xCF8:
        0xCF9:
            trunk     := 0xCF8;
            foliage   := 0xCF9;
            type_name := "Cipreste";
            log_type  := 0xEE3A;
        0xCFB:
        0xCFC:
            trunk     := 0xCFB;
            foliage   := 0xCFC;
            type_name := "Cipreste";
            log_type  := 0xEE3A;
        0xCFE:
        0xCFF:
            trunk     := 0xCFE;
            foliage   := 0xCFF;
            type_name := "Cipreste";
            log_type  := 0xEE3A;
        0xD01:
        0xD02:
            trunk     := 0xD01;
            foliage   := 0xD02;
            type_name := "Cipreste";
            log_type  := 0xEE3A;
        0x309C:
        0x30A6:
            trunk     := 0x309C;
            foliage   := 0x30A6;
            type_name := "Mogno";
            log_type  := 0xEE40;
        0x247D:
        0x2478:
            trunk     := 0x247D;
            foliage   := 0x2478;
            type_name := "Polar";
            log_type  := 0xEE3D;
        0x12B9:
        0x12C0:
            trunk     := 0x12B9;
            foliage   := 0x12C0;
            type_name := "Teixo";
            log_type  := 0xEE3E;
        0xA5CE:
        0xA5CF:
            trunk     := 0xA5CE;
            foliage   := 0xA5CF;
            type_name := "Imbuia";
            log_type  := 0xEE3F;
        0x30BD:
        0x30C8:
            trunk     := 0x30BD;
            foliage   := 0x30C8;
            type_name := "Ebano";
            log_type  := 0xEE3B;
    endcase

    if (!trunk)
        return error{"errortext" := "UL_GetTreeMeta: nao mapeado: " + Hex(CInt(graphic))};
    endif

    var meta := struct{};
    meta.+trunk     := trunk;
    meta.+foliage   := foliage;
    meta.+type_name := type_name;
    meta.+log_type  := log_type;
    return meta;
endfunction
