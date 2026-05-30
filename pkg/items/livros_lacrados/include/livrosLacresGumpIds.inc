// livrosLacresGumpIds.inc
// IDs dos livros lacrados (selados + rachados) no Gumpart.mul

const LL_GUMP_TRANSP     := 0xA95D;
const LL_GUMP_BTN_LACRE  := 0xA95E;

function GetLivroLacreGumpId(lacre_img)
    case (lacre_img)
        1: return 0xA937;
        2: return 0xA938;
        3: return 0xA939;
        4: return 0xA93A;
        5: return 0xA93B;
        6: return 0xA93C;
        7: return 0xA93D;
        8: return 0xA93E;
        10: return 0xA93F;
        11: return 0xA940;
        13: return 0xA942;
        14: return 0xA943;
        15: return 0xA944;
        16: return 0xA945;
        17: return 0xA946;
        18: return 0xA947;
        19: return 0xA948;
        20: return 0xA949;
    endcase
    return 0xA937;
endfunction

