// scripts/ler.inc
// Renderer compartilhado para todos os Livros Lacrados  - primeira abertura e releitura.

include ":livros_lacrados:include/livrosGumpIds";

//   -- -- Gumps do livro (IDs do Gumpart.mul)   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
const LL_BG_GUMP      := 0xA6A4;   // fundo do livro aberto 781  503
const LL_FRAME_GUMP   := 0xA69C;   // vine frame 265  418 (usado nas duas paginas)
const LL_PORTRAIT_FRM := 0xA69F;   // frame quadrado do portrait 123  122

//   -- -- Layout  - canvas 781  503   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
const LL_FL_X := 103;
const LL_FR_X := 413;
const LL_F_Y  := 42;

const LL_SC_X := 423;
const LL_SC_Y := 52;

const LL_PORT_X     := 421;
const LL_PORT_Y     := 27;
const LL_PORT_IMG_X := 429;
const LL_PORT_IMG_Y := 35;

// Texto  - pagina esquerda
const LL_TX_X       := 121;
const LL_TX_W       := 229;
const LL_TX_Y_TITLE := 66;
const LL_TX_Y_SUB   := 134;
const LL_TX_Y_BODY  := 173;
const LL_TX_H_BODY  := 243;
const LL_TX_Y_NAV   := 416;

// Navegcao
const LL_BTN_PREV := 1;
const LL_BTN_NEXT := 2;

//   -- -- Mapeamento invocacao     tipo de portrait/cena   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
function GetTipoFromInvocacao(inv_id)
    case (inv_id)
        // cat1
 "vexalim": return "vexalim_o_faminto";
 "iskaryon": return "os_sete_nomes";
 "pacto_roto": return "o_pacto_roto";
        // cat2
 "mae_procura": return "a_mae_que_procura";
 "soldado": return "o_soldado_de_aiglano";
 "lirenne": return "lirenne_a_esquecida";
 "tres_irmaos": return "os_tres_irmaos";
 "bibliotecario": return "o_bibliotecario_morto";
        // cat3
 "coisa_quadrupede": return "avistamentos";
        // cat4
 "mensageiro": return "sobre_o_mensageiro";
 "voz_velho": return "a_voz_do_velho_mundo";
 "anuncio": return "registro_do_anuncio";
 "confissor": return "a_confissao_do_tardio";
 "recita_nomes": return "a_que_recita_nomes";
        // cat5
 "coletor_anos": return "o_coletor_de_anos";
 "pede_sangue": return "o_que_pede_sangue";
 "quer_nome": return "a_que_quer_um_nome";
 "veio_memoria": return "o_que_veio_por_memoria";
 "cobrador": return "o_cobrador_da_divida_antiga";
    endcase
    // cat3 sem mapeamento especfico, cat7, cat8 e unicos: tipo == inv_id
    return inv_id;
endfunction

//   -- -- Split de texto em paginas pela marccao <br><br>   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Cada <br><br> vira uma quebra de pagina. Usado para conteudo_gerado (profecias, inertes).
function _SplitEmPaginas(texto)
 var sep := "<br><br>";
    var sep_len := len(sep);
    var txt_len := len(texto);
    var paginas := array{};
    var pos     := 1;

    while (pos <= txt_len)
        var sep_pos  := 0;
        var max_srch := txt_len - sep_len + 1;
        var i := pos;
        while (i <= max_srch && !sep_pos)
            if (texto[i, sep_len] == sep)
                sep_pos := i;
            else
                i += 1;
            endif
        endwhile

        if (sep_pos)
            var chunk_len := sep_pos - pos;
            if (chunk_len > 0)
                paginas.append(texto[pos, chunk_len]);
            endif
            pos := sep_pos + sep_len;
        else
            var remaining := txt_len - pos + 1;
            if (remaining > 0)
                paginas.append(texto[pos, remaining]);
            endif
            pos := txt_len + 1;
        endif
    endwhile

    if (paginas.size() == 0)
        paginas.append(texto);
    endif
    return paginas;
endfunction

//   -- -- Renderizador de uma pagina   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
function _MostrarGumpLivro(who, titulo, conteudo, tipo, nivel, afinidade, subtitulo, cur_pag, n_pags)
    var port_id := GetPortraitGumpId(tipo);
    var ill_id  := GetIlustracaoGumpId(tipo, nivel, afinidade);

    var gump := GFCreateGump();
    GFClosable(gump, 1);
    GFMovable(gump, 1);

    GFGumpPic(gump, 0,        0,        LL_BG_GUMP);
    GFGumpPic(gump, LL_FL_X,  LL_F_Y,   LL_FRAME_GUMP);
    GFGumpPic(gump, LL_SC_X,  LL_SC_Y,  ill_id);
    GFGumpPic(gump, LL_FR_X,  LL_F_Y,   LL_FRAME_GUMP);

    if (port_id != LL_GUMP_DEFAULT)
        GFGumpPic(gump, LL_PORT_IMG_X, LL_PORT_IMG_Y, port_id);
        GFGumpPic(gump, LL_PORT_X,     LL_PORT_Y,     LL_PORTRAIT_FRM);
    endif

    GFHTMLArea(gump, LL_TX_X, LL_TX_Y_TITLE, LL_TX_W, 68,
 "<center><basefont color=#371405>" + titulo + "</basefont></center>", 0, 0);

    var sub := CStr(subtitulo);
 if (sub != "" && sub != "0")
        GFHTMLArea(gump, LL_TX_X, LL_TX_Y_SUB, LL_TX_W, 39,
 "<center><basefont color=#0A32 " + sub + "</basefont></center>", 0, 0);
    endif

    // scroll=0: sem scrollbar, sem bug de posi  o e sem tracao preto nas bordas
    GFHTMLArea(gump, LL_TX_X, LL_TX_Y_BODY, LL_TX_W, LL_TX_H_BODY,
 "<basefont color=#1E0F05>" + conteudo + "</basefont>", 0, 0);

    // Nav: [prev] < N / M > [next]
    if (cur_pag > 1)
        GFAddButton(gump, LL_TX_X, LL_TX_Y_NAV + 3, 2704, 2705, GF_CLOSE_BTN, LL_BTN_PREV);
    endif

 var nav_label := "";
    if (n_pags > 1)
 nav_label := "< " + cur_pag + " / " + n_pags + " >";
    else
 nav_label := "< 1 >";
    endif
    GFHTMLArea(gump, LL_TX_X + 18, LL_TX_Y_NAV, LL_TX_W - 36, 20,
 "<center><basefont color=#371405>" + nav_label + "</basefont></center>", 0, 0);

    if (cur_pag < n_pags)
        GFAddButton(gump, LL_TX_X + LL_TX_W - 16, LL_TX_Y_NAV + 3, 2706, 2707, GF_CLOSE_BTN, LL_BTN_NEXT);
    endif

    var resp := GFSendGump(who, gump);
    return resp[0];
endfunction

//   -- -- Loop de pagincao (uso interno)   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
function _LoopPaginas(who, titulo, paginas, tipo, nivel, afinidade, subtitulo)
    var n_pags  := paginas.size();
    var cur_pag := 1;

    while (1)
        var btn := _MostrarGumpLivro(who, titulo, paginas[cur_pag], tipo, nivel, afinidade, subtitulo, cur_pag, n_pags);
        if (!btn)
            return;
        elseif (btn == LL_BTN_PREV && cur_pag > 1)
            cur_pag -= 1;
        elseif (btn == LL_BTN_NEXT && cur_pag < n_pags)
            cur_pag += 1;
        endif
    endwhile
endfunction

//   -- -- Entry point para scripts de primeira abertura   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// Substitui a chamada direta a _MostrarGumpLivro(7 args) nos scripts de natureza.
// Divide conteudo em paginas via <br><br> e entra no loop de pagincao.
function _PaginarEMostrar(who, titulo, conteudo, tipo, nivel, afinidade, subtitulo)
    _LoopPaginas(who, titulo, _SplitEmPaginas(conteudo), tipo, nivel, afinidade, subtitulo);
endfunction

//   -- -- Re-leitura do livro (double-click aps abertura inicial)   -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
function LerLivroLacrado(who, book)
 var natureza := CStr(GetObjProperty(book, "natureza"));
 var afinidade := CStr(GetObjProperty(book, "afinidade"));
 var nivel := CInt(GetObjProperty(book, "nivel_bau"));
 var invocacao := CStr(GetObjProperty(book, "invocacao"));

 var _titulo_raw := GetObjProperty(book, "titulo_gerado");
 var titulo := "";
    if (!_titulo_raw.errortext) titulo := CStr(_titulo_raw); endif

 var _sub_raw := GetObjProperty(book, "subtitulo_gerado");
 var subtitulo := "";
    if (!_sub_raw.errortext) subtitulo := CStr(_sub_raw); endif

 var _conteudo_raw := GetObjProperty(book, "conteudo_gerado");
 var conteudo := "";
    if (!_conteudo_raw.errortext) conteudo := CStr(_conteudo_raw); endif

 var _tipo_raw := GetObjProperty(book, "tipo_inerte");
 var tipo := "";
    if (!_tipo_raw.errortext) tipo := CStr(_tipo_raw); endif


    var paginas := array{};

 if (conteudo != "" && conteudo != "0")
 if (tipo == "" || tipo == "0")
            tipo := GetTipoFromInvocacao(invocacao);
        endif
        // split por <br><br> em paginas (profecia tem 1 separador     2 pgs)
        paginas := _SplitEmPaginas(conteudo);
 elseif (invocacao != "" && invocacao != "0")
        tipo    := GetTipoFromInvocacao(invocacao);
        paginas := _BuscarPaginasEntidade(invocacao);
 if (titulo == "" || titulo == "0")
            titulo := _BuscarTituloEntidade(invocacao);
 if (titulo != "" && titulo != "0")
 SetObjProperty(book, "titulo_gerado", titulo);
            endif
        endif
    else
        paginas.append(_GetConteudoGenerico(natureza));
    endif

 if (titulo == "" || titulo == "0")
 titulo := "Livro Lacrado";
    endif

    if (paginas.size() == 0)
 paginas.append("As paginas esto em branco.");
    endif

    _LoopPaginas(who, titulo, paginas, tipo, nivel, afinidade, subtitulo);
endfunction

//   -- -- DataFile: busca paginas de uma entidade   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
function _BuscarPaginasEntidade(entity_id)
 var df := OpenDataFile(":livros_lacrados:lore_entidades");
    if (df.errortext)
 return array{"O contedo esta alem da compreenso."};
    endif

 var pags_key := entity_id + "_pags";
    var pags_elem := df.FindElement(pags_key);
    if (pags_elem.errortext)
 return array{"As paginas esto em branco."};
    endif

 var n_pags := CInt(pags_elem.GetProp("n"));
    // Cada chunk armazenado pode conter <br><br> como separador de pargrafo.
    // Aplicar _SplitEmPaginas em cada chunk transforma esses separadores em
    // quebras de pagina, evitando overflow de texto sem scrollbar.
    var result := array{};
    var i := 1;
    while (i <= n_pags)
 var pg := CStr(pags_elem.GetProp("p" + i));
        var sub := _SplitEmPaginas(pg);
        var j := 1;
        while (j <= sub.size())
            result.append(sub[j]);
            j += 1;
        endwhile
        i += 1;
    endwhile

    if (result.size() == 0)
 return array{"As paginas esto em branco."};
    endif
    return result;
endfunction

//   -- -- DataFile: busca titulo de uma entidade   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
function _BuscarTituloEntidade(entity_id)
 var df := OpenDataFile(":livros_lacrados:lore_entidades");
 if (df.errortext) return ""; endif
 var elem := df.FindElement(entity_id + "_titulo");
 if (elem.errortext) return ""; endif
 var v := elem.GetProp("v");
 if (v.errortext) return ""; endif
    return CStr(v);
endfunction

//   -- -- Fallback para livros sem contedo nem invoccao   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
function _GetConteudoGenerico(natureza)
    case (natureza)
        NAT_CIFRADO:
 return "As palavras se movem ao olhar fixo. Nao h como decifrar.";
        NAT_MANCHADO:
 return "A tinta manchou as paginas. Fragmentos de algo ecoam.";
        NAT_PROFETICO:
 return "Versos sem autoria. Cada leitura revela palavras diferentes.";
        NAT_CONVOCADOR:
 return "O nome que havia aqui no existe mais.";
        NAT_DESPERTADOR:
 return "O chamado j foi feito. A pagina esta em silencio.";
    endcase
 return "As paginas esto ilegveis.";
endfunction
