// naturezas/inerte.inc
// Natureza Inerte (58%): conteudo carregado do DataFile :livros_lacrados:inerte_textos

include ":livros_lacrados:scripts/ler";

function ExecutarInerte(who, book, nivel, afinidade, lugar)
 var fixo_raw := GetObjProperty(book, "tipo_inerte_fixo");
    var tipo;
 if (!fixo_raw.errortext && fixo_raw != "")
        tipo := CStr(fixo_raw);
    else
        tipo := SortearTipoInerte();
    endif

    var titulo   := GerarTituloInerte(tipo);
    var conteudo := GerarConteudoInerte(tipo, nivel);

 SetObjProperty(book, "tipo_inerte", tipo);
 SetObjProperty(book, "titulo_gerado", titulo);
 SetObjProperty(book, "conteudo_gerado", conteudo);

 _PaginarEMostrar(who, titulo, conteudo, tipo, nivel, afinidade, "");
 SendSysMessageEX(who, "Voce le o livro com atencao.", SSM_INFO);
endfunction

function SortearTipoInerte()
    var tipos := array{
 "diario_puxado",
 "carta_aiglano",
 "registro_anao_khar",
 "canto_bjorske",
 "nota_senhor_guerra",
 "tomo_leste",
 "confissao_drow",
 "herbario_pequilho",
 "cronica_orc",
 "diario_catedral",
 "relato_mercador",
 "nota_espiao_liga",
 "testemunho_retornado",
 "diario_necromante",
 "fragmento_limiar"
    };
    return tipos[RandomInt(15) + 1];
endfunction

function _LerInerte(chave, fallback)
 var df := OpenDataFile(":livros_lacrados:inerte_textos");
    if (df.errortext)
        return fallback;
    endif
    var elem := df.FindElement(chave);
    if (elem.errortext)
        return fallback;
    endif
 var n := CInt(elem.GetProp("n"));
    if (n < 1) n := 1; endif
 var v := elem.GetProp("v" + (RandomInt(n) + 1));
    if (v.errortext) return fallback; endif
    return CStr(v);
endfunction

function GerarTituloInerte(tipo)
 return _LerInerte("t_" + tipo, "Um livro velho");
endfunction

function GerarConteudoInerte(tipo, nivel)
 return _LerInerte("c_" + tipo + "_" + nivel, "As paginas estao ilegiveis.");
endfunction
