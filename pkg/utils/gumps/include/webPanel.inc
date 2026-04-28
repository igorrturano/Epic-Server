/*
 * WEBPANEL.INC
 *
 * Envia painéis HTML/CSS completos para clientes EpicUO que suportam WebView2.
 * O cliente renderiza o conteúdo via Edge Chromium (WebView2), permitindo
 * HTML5/CSS3 completo: tabelas, flexbox, colunas, imagens, etc.
 *
 * Protocolo de comunicação:
 *   Servidor → Cliente: Packet 0xBF, SubCommand 0x00FA
 *     [0xBF:1][size:2][0x00FA:2][width:2][height:2][html:utf8][0x00:1]
 *
 *   Cliente → Servidor: Packet 0xD7, SubCommand 0x00FB
 *     [0xD7:1][size:2][serial:4][0x00FB:2][action:1][data:utf8][0x00:1]
 *     action: WP_ACTION_CLOSE (0), WP_ACTION_CLICK (1), WP_ACTION_DATA (2)
 *
 * Dependências: htmlBuilder.inc, cssStyles.inc
 *
 * Funções públicas:
 *   SendWebPanel(who, title, htmlBody, width, height)
 *   SendWebPanelRaw(who, fullHtml, width, height)
 *   BuildWebDocument(title, htmlBody, extraCss)
 *   CloseWebPanel(who)
 *   WaitForWebPanelEvent(who, timeoutMs) — bloqueia até o cliente
 *                                          devolver close/click/data
 *   _WpStoreEvent(who, action, data)     — usado pelo packetHook
 *
 * Constantes de ação (retorno do cliente):
 *   WP_ACTION_CLOSE  (0x00)
 *   WP_ACTION_CLICK  (0x01)
 *   WP_ACTION_DATA   (0x02)
 *
 * Packet IDs:
 *   WP_PACKET_SEND    (0xBF) / SubCmd WP_SUB_OPEN  (0x00FA)
 *   WP_PACKET_RECEIVE (0xD7) / SubCmd WP_SUB_EVENT (0x00FB)
 */

use uo;
use os;
use basic;
use polsys;

include ":gumps:htmlBuilder";
include ":gumps:cssStyles";

// ─────────────────────────────────────────────
//  Constantes de protocolo
// ─────────────────────────────────────────────

const WP_PACKET_SEND    := 0xBF;
const WP_SUB_OPEN       := 0x00FA;
const WP_SUB_CLOSE      := 0x00FC;

const WP_PACKET_RECEIVE := 0xD7;
const WP_SUB_EVENT      := 0x00FB;

const WP_ACTION_CLOSE   := 0x00;
const WP_ACTION_CLICK   := 0x01;
const WP_ACTION_DATA    := 0x02;

// Dimensões padrão
const WP_DEFAULT_WIDTH  := 640;
const WP_DEFAULT_HEIGHT := 520;

// ─────────────────────────────────────────────
//  CSS base do documento EpicUO
// ─────────────────────────────────────────────

// CSS base aplicado a todos os documentos WebPanel — definido como constante
// para evitar reconstruir uma string de ~2KB a cada BuildWebDocument().
const WP_BASE_CSS := "
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            background-color: #1a1208;
            color: #dddddd;
            font-family: 'Palatino Linotype', 'Book Antiqua', Palatino, serif;
            font-size: 14px;
            padding: 16px 20px;
            line-height: 1.5;
        }
        h1, h2, h3, h4, h5, h6 {
            color: #C8A040;
            margin: 12px 0 6px 0;
            font-weight: bold;
        }
        h1 { font-size: 22px; border-bottom: 1px solid #5a4010; padding-bottom: 6px; }
        h2 { font-size: 18px; }
        h3 { font-size: 15px; color: #AAAAAA; }
        p  { margin: 6px 0; }
        a  { color: #4499FF; text-decoration: none; }
        a:hover { text-decoration: underline; }
        hr { border: none; border-top: 1px solid #5a4010; margin: 10px 0; }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 8px 0;
            font-size: 13px;
        }
        th {
            background-color: #2a1f0a;
            color: #C8A040;
            padding: 6px 10px;
            text-align: left;
            border: 1px solid #5a4010;
        }
        td {
            padding: 5px 10px;
            border: 1px solid #3a2c0a;
            vertical-align: top;
        }
        tr:nth-child(even) td { background-color: #211808; }
        ul, ol { padding-left: 20px; margin: 6px 0; }
        li { margin: 3px 0; }
        img { max-width: 100%; border: 1px solid #5a4010; }
        .close-btn {
            display: block;
            margin: 16px auto 4px auto;
            padding: 6px 24px;
            background: linear-gradient(180deg, #5a3d10 0%, #3a2508 100%);
            color: #C8A040;
            border: 1px solid #C8A040;
            font-family: inherit;
            font-size: 13px;
            cursor: pointer;
            letter-spacing: 1px;
        }
        .close-btn:hover { background: linear-gradient(180deg, #7a5518 0%, #5a3d10 100%); }
        .alert  { color: #FFD700; font-weight: bold; }
        .error  { color: #FF4444; font-weight: bold; }
        .success { color: #44CC77; }
        .info   { color: #4499FF; }
        .caption { color: #666666; font-style: italic; font-size: 11px; }
        .narrator { color: #DDBB88; font-style: italic; text-align: center; }
        .two-cols { columns: 2; column-gap: 20px; }
    ";

// ─────────────────────────────────────────────
//  Construção do documento HTML completo
// ─────────────────────────────────────────────

/*
 * BuildWebDocument(title, htmlBody, extraCss)
 * Monta o documento HTML5 completo com o template EpicUO.
 */
function BuildWebDocument(title, htmlBody, extraCss := "")
    var css := WP_BASE_CSS;
    if (extraCss)
        css := css + extraCss;
    endif

    return "<!DOCTYPE html><html lang=\"pt-BR\"><head>"
        + "<meta charset=\"utf-8\">"
        + "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        + $"<title>{title}</title>"
        + $"<style>{css}</style>"
        + "</head><body>"
        + $"<h1>{title}</h1>"
        + CStr(htmlBody)
        + "<button class=\"close-btn\" onclick=\"window.chrome.webview.postMessage('close')\">Fechar</button>"
        + "</body></html>";
endfunction

// ─────────────────────────────────────────────
//  Envio de packets
// ─────────────────────────────────────────────

/*
 * _WpSendPacket(who, width, height, fullHtml)
 * Monta e envia o packet 0xBF/0x00FA ao cliente.
 *
 * Layout do packet:
 *   [0xBF:1][size:2][sub:2][width:2][height:2][html_len:2][html:utf8]
 */
function _WpSendPacket(who, width, height, fullHtml)
    var htmlStr   := CStr(fullHtml);
    var htmlLen   := Len(htmlStr);
    // header: 1 (id) + 2 (size) + 2 (sub) + 2 (width) + 2 (height) + 2 (html_len) = 11
    var pktSize   := 11 + htmlLen;

    var pkt := CreatePacket(WP_PACKET_SEND, pktSize);
    pkt.SetInt8(0,  WP_PACKET_SEND);
    pkt.SetInt16(1, pktSize);
    pkt.SetInt16(3, WP_SUB_OPEN);
    pkt.SetInt16(5, CInt(width));
    pkt.SetInt16(7, CInt(height));
    pkt.SetInt16(9, htmlLen);
    pkt.SetString(11, htmlStr, 0);

    pkt.SendPacket(who);
endfunction

/*
 * SendWebPanel(who, title, htmlBody, width, height)
 * Alto nível: monta o documento completo e envia ao cliente.
 *
 * Exemplo:
 *   var corpo := HtmlBuilder();
 *   corpo.Heading(2, "Notícia").Paragraph("O torneio começa amanhã.").Br();
 *   SendWebPanel(who, "O Arauto de Britain", corpo.Build());
 */
function SendWebPanel(who, title, htmlBody, width := WP_DEFAULT_WIDTH, height := WP_DEFAULT_HEIGHT)
    var fullHtml := BuildWebDocument(title, htmlBody);
    _WpSendPacket(who, width, height, fullHtml);
endfunction

/*
 * SendWebPanelRaw(who, fullHtml, width, height)
 * Envia HTML já montado diretamente (sem aplicar o template base).
 * Use quando precisar de controle total sobre o documento.
 */
function SendWebPanelRaw(who, fullHtml, width := WP_DEFAULT_WIDTH, height := WP_DEFAULT_HEIGHT)
    _WpSendPacket(who, width, height, fullHtml);
endfunction

/*
 * CloseWebPanel(who)
 * Envia comando para fechar o painel WebView2 no cliente.
 */
function CloseWebPanel(who)
    var pkt := CreatePacket(WP_PACKET_SEND, 5);
    pkt.SetInt8(0,  WP_PACKET_SEND);
    pkt.SetInt16(1, 5);
    pkt.SetInt16(3, WP_SUB_CLOSE);
    pkt.SendPacket(who);
endfunction

// ─────────────────────────────────────────────
//  Storage de eventos / espera bloqueante
// ─────────────────────────────────────────────

// Prefixo de storage por serial — compartilhado entre webPanel.inc e o
// packetHook em pkg/packetHooks/webPanel/webPanel.src.
const WP_STORAGE_PREFIX := "webpanel_event_";

/*
 * _WpStoreEvent(who, action, data)
 * Armazena o evento mais recente no storage global, indexado por serial.
 * Chamado pelo packetHook (webPanel.src) ao receber 0xD7/0x00FB.
 */
function _WpStoreEvent(who, action, data)
    var key  := WP_STORAGE_PREFIX + CStr(who.serial);
    var evt  := struct;
    evt.+action := action;
    evt.+data   := data;
    evt.+time   := ReadGameClock();

    SetGlobalProperty(key, evt);
endfunction

/*
 * WaitForWebPanelEvent(who, timeoutMs)
 *
 * Bloqueia o script atual até o jogador interagir com o painel WebView2
 * ou até o timeout expirar.
 *
 * Parâmetros:
 *   who:       O mobile que tem o painel aberto
 *   timeoutMs: Timeout em milissegundos (padrão: 60000 = 1 minuto)
 *
 * Retorno:
 *   struct{ action, data, time } — ação e dado do evento
 *   error { errortext := "timeout" } se expirar
 *
 * Exemplo:
 *   SendWebPanel(who, "Jornal", corpo);
 *   var evt := WaitForWebPanelEvent(who);
 *   if (evt.action == WP_ACTION_CLICK)
 *       // processa evt.data (id do botão)
 *   endif
 */
function WaitForWebPanelEvent(who, timeoutMs := 60000)
    var key      := WP_STORAGE_PREFIX + CStr(who.serial);
    var elapsed  := 0;
    var interval := 100;

    // Limpa qualquer evento antigo antes de aguardar.
    EraseGlobalProperty(key);

    while (elapsed < timeoutMs)
        SleepMs(interval);
        elapsed := elapsed + interval;

        var evt := GetGlobalProperty(key);
        if (evt && !evt.errortext)
            EraseGlobalProperty(key);
            return evt;
        endif
    endwhile

    return error{ errortext := "timeout" };
endfunction
