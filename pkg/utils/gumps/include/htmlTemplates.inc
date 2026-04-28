/*
 * HTMLTEMPLATES.INC
 *
 * Templates HTML prontos para casos de uso comuns no EpicUO.
 * Todos geram HTML5 completo para uso com SendWebPanel().
 *
 * Dependências: htmlBuilder.inc, cssStyles.inc, webPanel.inc
 *
 * Templates disponíveis:
 *   JornalTemplate(titulo, colunas_array, rodape)
 *   RelatorioTemplate(titulo, dados, opcoesColunas, rodape)
 *   DialogoNarratorTemplate(nomeNpc, texto, opcoes)
 *   ItemInfoTemplate(nome, descricao, atributos)
 *   AnuncioTemplate(titulo, mensagem, tipo)
 *   PerfilJogadorTemplate(nome, raca, classe, atributos, historico)
 */

use basic;

include ":gumps:htmlBuilder";
include ":gumps:cssStyles";

// ─────────────────────────────────────────────
//  Jornal / Boletim
// ─────────────────────────────────────────────

/*
 * JornalTemplate(titulo, artigos, rodape)
 *
 * Cria um jornal/boletim com layout de duas colunas.
 *
 * Parâmetros:
 *   titulo:   Nome do jornal (ex: "O Arauto de Britain")
 *   artigos:  Array de structs com membros:
 *               .titulo   string
 *               .corpo    string HTML (pode usar HtmlBold, HtmlItalic, etc.)
 *               .imagem   string (URL opcional, deixe "" para omitir)
 *   rodape:   Texto do rodapé (ex: "Edição nº 42 — Ultima Online")
 *
 * Retorno: string HTML do corpo do jornal (sem o wrapper do documento).
 *          Passe para SendWebPanel() ou BuildWebDocument().
 */
function JornalTemplate(titulo, artigos, rodape := "")
    var html := HtmlBuilder();

    html.Append("<div style=\"font-size:11px;color:#888;text-align:center;margin-bottom:8px\">");
    html.Append($"— {titulo} —");
    html.Append("</div>");

    html.Append("<div class=\"two-cols\">");

    foreach artigo in artigos
        html.Append("<div style=\"break-inside:avoid;margin-bottom:14px\">");

        if (artigo.imagem)
            html.Append(HtmlImage(artigo.imagem, 80, 0, artigo.titulo));
        endif

        html.Append(HtmlHeading(3, artigo.titulo));
        html.Append(HtmlParagraph(artigo.corpo));
        html.Append(HtmlSeparator());
        html.Append("</div>");

        SleepMs(1);
    endforeach

    html.Append("</div>");

    if (rodape)
        html.Append($"<p class=\"caption\" style=\"text-align:center;margin-top:10px\">{rodape}</p>");
    endif

    return html.Build();
endfunction

// ─────────────────────────────────────────────
//  Relatório de dados (tabela)
// ─────────────────────────────────────────────

/*
 * RelatorioTemplate(titulo, dados, headers, rodape)
 *
 * Relatório tabelado genérico.
 *
 * Parâmetros:
 *   titulo:   Título do relatório
 *   dados:    Array de arrays (linhas e células)
 *             Exemplo: array{ array{"Thorin", "Guerreiro", "42"}, ... }
 *   headers:  Array de strings para o cabeçalho
 *             Exemplo: array{"Nome", "Classe", "Nível"}
 *   rodape:   Texto do rodapé (opcional)
 *
 * Retorno: string HTML do corpo.
 */
function RelatorioTemplate(titulo, dados, headers := array{}, rodape := "")
    var html := HtmlBuilder();

    if (dados.size() == 0)
        html.Append(CssApply(CSS_CAPTION, "Nenhum dado encontrado."));
        return html.Build();
    endif

    html.Append(HtmlTable(dados, headers));

    html.Append($"<p class=\"caption\" style=\"text-align:right;margin-top:4px\">{dados.size()} registro(s)</p>");

    if (rodape)
        html.Append(HtmlSeparator());
        html.Append($"<p class=\"caption\">{rodape}</p>");
    endif

    return html.Build();
endfunction

// ─────────────────────────────────────────────
//  Diálogo de NPC / Narrador
// ─────────────────────────────────────────────

/*
 * DialogoNarratorTemplate(nomeNpc, texto, opcoes)
 *
 * Painel de diálogo com fala de NPC e opções de resposta.
 * As opções são botões que enviam postMessage ao servidor via cliente.
 *
 * Parâmetros:
 *   nomeNpc:  Nome do personagem que fala
 *   texto:    Fala do personagem (pode conter HTML)
 *   opcoes:   Array de structs com membros .label e .id
 *             Exemplo: array{ struct{label:="Aceitar",id:=1}, struct{label:="Recusar",id:=2} }
 *
 * Retorno: string HTML do corpo.
 */
function DialogoNarratorTemplate(nomeNpc, texto, opcoes := array{})
    var html := HtmlBuilder();

    html.Append("<div style=\"border:1px solid #5a4010;padding:14px;background:#1f1708;margin-bottom:12px\">");
    html.Append($"<p style=\"color:#C8A040;font-size:13px;margin-bottom:8px\">— {nomeNpc} diz —</p>");
    html.Append($"<p class=\"narrator\">{texto}</p>");
    html.Append("</div>");

    if (opcoes.size() > 0)
        html.Append("<div style=\"display:flex;gap:10px;justify-content:center;flex-wrap:wrap;margin-top:12px\">");
        foreach opcao in opcoes
            var id := CStr(opcao.id);
            var label := CStr(opcao.label);
            html.Append($"<button style=\"padding:6px 18px;background:#2a1f0a;color:#C8A040;border:1px solid #5a4010;font-family:inherit;cursor:pointer\" onclick=\"window.chrome.webview.postMessage('click:{id}')\">{label}</button>");
            SleepMs(1);
        endforeach
        html.Append("</div>");
    endif

    return html.Build();
endfunction

// ─────────────────────────────────────────────
//  Informações de item
// ─────────────────────────────────────────────

/*
 * ItemInfoTemplate(nome, descricao, atributos, raridade)
 *
 * Painel de detalhes de um item com atributos em tabela.
 *
 * Parâmetros:
 *   nome:       Nome do item
 *   descricao:  Texto de lore/descrição (pode conter HTML)
 *   atributos:  Array de structs com membros .nome e .valor
 *               Exemplo: array{ struct{nome:="Dano",valor:="20-35"}, ... }
 *   raridade:   String opcional ("Comum", "Raro", "Épico", "Lendário")
 *
 * Retorno: string HTML do corpo.
 */
function ItemInfoTemplate(nome, descricao, atributos := array{}, raridade := "")
    var html := HtmlBuilder();

    // Cor da raridade
    var rarColor := CSS_HEX_WHITE;
    case (Lower(CStr(raridade)))
        "raro":     rarColor := "#4499FF";
        "épico":    rarColor := "#AA44FF";
        "épico":    rarColor := "#AA44FF";
        "epico":    rarColor := "#AA44FF";
        "lendário": rarColor := "#FF8C00";
        "lendario": rarColor := "#FF8C00";
        default:    rarColor := CSS_HEX_GRAY;
    endcase

    if (raridade)
        html.Append(HtmlSpan(raridade, $"color:{rarColor};font-size:11px;font-weight:bold;letter-spacing:1px"));
        html.Br();
    endif

    html.Append(HtmlSeparator());
    html.Append($"<p style=\"color:#BBBBBB;margin:8px 0;font-style:italic\">{descricao}</p>");
    html.Append(HtmlSeparator());

    if (atributos.size() > 0)
        html.Append("<table style=\"margin-top:8px\">");
        foreach attr in atributos
            html.Append($"<tr><td style=\"color:#AAAAAA;width:50%\">{attr.nome}</td><td style=\"color:#EEEEEE;font-weight:bold\">{attr.valor}</td></tr>");
            SleepMs(1);
        endforeach
        html.Append("</table>");
    endif

    return html.Build();
endfunction

// ─────────────────────────────────────────────
//  Anúncio do sistema
// ─────────────────────────────────────────────

/*
 * AnuncioTemplate(mensagem, tipo)
 *
 * Painel de anúncio/alerta simples para o jogador.
 *
 * Parâmetros:
 *   mensagem: Texto do anúncio (pode conter HTML)
 *   tipo:     "info" | "alerta" | "erro" | "sucesso" (define a cor)
 *
 * Retorno: string HTML do corpo.
 */
function AnuncioTemplate(mensagem, tipo := "info")
    var borderColor, iconText;
    case (Lower(CStr(tipo)))
        "alerta":  borderColor := CSS_HEX_YELLOW;  iconText := "⚠";
        "erro":    borderColor := CSS_HEX_RED;      iconText := "✖";
        "sucesso": borderColor := CSS_HEX_GREEN;    iconText := "✔";
        default:   borderColor := CSS_HEX_BLUE;     iconText := "ℹ";
    endcase

    return $"<div style=\"border-left:4px solid {borderColor};padding:12px 16px;background:#1f1708;margin:8px 0\">"
        + $"<span style=\"color:{borderColor};font-size:18px;margin-right:8px\">{iconText}</span>"
        + $"<span>{mensagem}</span>"
        + "</div>";
endfunction

// ─────────────────────────────────────────────
//  Perfil de personagem
// ─────────────────────────────────────────────

/*
 * PerfilJogadorTemplate(nome, raca, classe, atributos, historico)
 *
 * Painel de ficha de personagem com foto, atributos e histórico.
 *
 * Parâmetros:
 *   nome:       Nome do personagem
 *   raca:       Raça
 *   classe:     Classe/profissão
 *   atributos:  Array de structs { .nome, .valor, .max } para barra de progresso
 *   historico:  Texto livre de história do personagem
 *
 * Retorno: string HTML do corpo.
 */
function PerfilJogadorTemplate(nome, raca, classe, atributos := array{}, historico := "")
    var html := HtmlBuilder();

    // Cabeçalho
    html.Append("<div style=\"display:flex;gap:16px;align-items:flex-start;margin-bottom:12px\">");
    html.Append("<div style=\"flex:1\">");
    html.Append($"<p style=\"color:#AAAAAA;font-size:12px\">{raca} · {classe}</p>");

    // Atributos com barras visuais ASCII
    if (atributos.size() > 0)
        html.Append("<table style=\"margin-top:10px;font-size:12px\">");
        foreach attr in atributos
            var pct := 0;
            if (attr.max > 0)
                pct := CInt((CDbl(attr.valor) / CDbl(attr.max)) * 100);
            endif
            var barFilled := CInt(pct / 5);   // barra de 20 blocos
            var barEmpty  := 20 - barFilled;
            var bar := "";
            var i := 0;
            while (i < barFilled)
                bar := bar + "█";
                i += 1;
            endwhile
            while (i < 20)
                bar := bar + "░";
                i += 1;
            endwhile
            html.Append($"<tr><td style=\"color:#AAAAAA;width:80px\">{attr.nome}</td>");
            html.Append($"<td style=\"color:#5a8a5a;font-family:monospace;font-size:10px\">{bar}</td>");
            html.Append($"<td style=\"color:#EEEEEE;padding-left:6px\">{attr.valor}/{attr.max}</td></tr>");
            SleepMs(1);
        endforeach
        html.Append("</table>");
    endif

    html.Append("</div></div>");

    // Histórico
    if (historico)
        html.Append(HtmlSeparator());
        html.Append(HtmlHeading(3, "História"));
        html.Append($"<p style=\"color:#BBBBBB;font-style:italic;line-height:1.6\">{historico}</p>");
    endif

    return html.Build();
endfunction
