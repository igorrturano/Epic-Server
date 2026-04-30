// ============================================================================
//  EpicUO — HTML Gump  (server → client)
//  Opcode customizado 0xF8
//
//  Uso:
//    include ":htmlgump:htmlgump";
//    SendHtmlGump(quem, 0x1001, "<html>...", 400, 300, 100, 100);
//    CloseHtmlGump(quem, 0x1001);
//
//  Pacote layout:
//   [0]  1  opcode (0xF8)
//   [1]  2  tamanho total do pacote (DEVE ser setado manualmente no POL)
//   [3]  4  gump_id
//   [7]  2  x
//   [9]  2  y
//  [11]  2  width
//  [13]  2  height
//  [15]  2  html_len
//  [17]  N  html (ASCII, sem null terminator)
//
//  Tamanho fixo do header (sem HTML): 17 bytes
//  Tamanho total: 17 + html_len
// ============================================================================

use uo;
use polsys;

const HTMLGUMP_OPCODE := 0xF8;
const HTMLGUMP_HEADER  := 17;   // 1 opcode + 2 len + 4 gump_id + 2x + 2y + 2w + 2h + 2 html_len

// Envia ou atualiza um HTML Gump no cliente.
// html_len == 0 → fecha a janela (equivalente a CloseHtmlGump).
function SendHtmlGump(quem, gump_id, html, w, h, x := 100, y := 100)
    var pkt      := CreatePacket(HTMLGUMP_OPCODE, MSGLEN_VARIABLE);
    var html_str := CStr(html);
    var html_len := len(html_str);

    // OBRIGATÓRIO no POL: setar tamanho total manualmente nos bytes 1-2
    pkt.SetInt16(1,  HTMLGUMP_HEADER + html_len);

    pkt.SetInt32(3,  CInt(gump_id));
    pkt.SetInt16(7,  CInt(x));
    pkt.SetInt16(9,  CInt(y));
    pkt.SetInt16(11, CInt(w));
    pkt.SetInt16(13, CInt(h));
    pkt.SetInt16(15, html_len);

    if (html_len > 0)
        pkt.SetString(17, html_str, 0);
    endif

    pkt.SendPacket(quem);
endfunction

// Fecha um HTML Gump existente no cliente.
function CloseHtmlGump(quem, gump_id)
    var pkt := CreatePacket(HTMLGUMP_OPCODE, MSGLEN_VARIABLE);

    pkt.SetInt16(1,  HTMLGUMP_HEADER);   // tamanho fixo sem HTML
    pkt.SetInt32(3,  CInt(gump_id));
    pkt.SetInt16(7,  0);
    pkt.SetInt16(9,  0);
    pkt.SetInt16(11, 0);
    pkt.SetInt16(13, 0);
    pkt.SetInt16(15, 0);
    pkt.SendPacket(quem);
endfunction
