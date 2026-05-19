// naturezas/cifrado.inc
// Natureza Cifrado: 3 camadas de leitura por lingua.
//
// Camada "lingua"  -  jogador conhece a lingua moderna equivalente
// Camada "magia"  -  Magery >= 90
// Camada "arq"  -  Habilidade Arqueologo (3608) ou Desbravador Nato (1100)
//
// DataFile cifrado_textos:
//   c_<lingua>_lingua    -  texto para quem conhece a lingua moderna
//   c_<lingua>_magia     -  texto para quem tem alta magia
//   c_<lingua>_arq       -  texto completo para arqueologos

include ":livros_lacrados:scripts/ler";

//  Equivalencia: lingua cifrada  lingua moderna 

function GetLinguaModerna(lingua)
    case (lingua)
 "aglabita_antigo": return "Aglab";
 "khar_runico": return "Phalk";
 "bjorske_cursivo": return "Bjor";
 "ahruwen": return "Ahruwen";
 "idrith_har": return "Idrith'har";
 "necro_cifrado": return "Idrith'har";
 "tsu_ngathar": return "Grumnak";
 "primeiro_lacre": return "Imperialis";
 "vael_glifo": return "Haiei";
    endcase
 return "";
endfunction

//  Camadas acessiveis 

// Retorna struct{ lingua, magia, arq } com 1/0 por camada
function GetCamadasAcessiveis(who, lingua_cifrada)
    var result := struct{
        lingua := 0,
        magia  := 0,
        arq    := 0
    };

    // Camada lingua: jogador conhece o equivalente moderno
    var moderna := GetLinguaModerna(lingua_cifrada);
 var conhecidas := GetObjProperty(who, "linguas_conhecidas");
    if (conhecidas && !conhecidas.errortext)
        foreach l in conhecidas
            if (l == moderna)
                result.lingua := 1;
                break;
            endif
        endforeach
    endif

    // Camada magia
 var magery := AP_GetSkill(who, "Magery");
    if (magery >= 90) result.magia := 1; endif

    // Camada arqueologia
    if (TemHabilidade(who, 3608) || TemHabilidade(who, 1100))
        result.arq := 1;
    endif

    return result;
endfunction

//  Desbloqueio persistente no livro 
// Props no item: "camada_lingua", "camada_magia", "camada_arq" (0/1)
// Uma vez desbloqueada por qualquer leitor, a camada fica visivel para todos.

function GetCamadasDesbloqueadas(book)
    return struct{
 lingua := CInt(GetObjProperty(book, "camada_lingua")) > 0 ? 1 : 0,
 magia := CInt(GetObjProperty(book, "camada_magia")) > 0 ? 1 : 0,
 arq := CInt(GetObjProperty(book, "camada_arq")) > 0 ? 1 : 0
    };
endfunction

function SetCamadasDesbloqueadas(book, camadas)
 if (camadas.lingua) SetObjProperty(book, "camada_lingua", 1); endif
 if (camadas.magia) SetObjProperty(book, "camada_magia", 1); endif
 if (camadas.arq) SetObjProperty(book, "camada_arq", 1); endif
endfunction

//  Execucao principal 

function ExecutarCifrado(who, book, nivel, afinidade, lugar)
 var lingua_raw := GetObjProperty(book, "lingua");
    var lingua;
 if (!lingua_raw.errortext && lingua_raw != "")
        lingua := CStr(lingua_raw);
    else
        lingua := DesenharLingua(nivel);
 SetObjProperty(book, "lingua", lingua);
    endif

    var do_player := GetCamadasAcessiveis(who, lingua);
    var do_livro  := GetCamadasDesbloqueadas(book);

    // Camadas visiveis = o que o jogador acessa agora + o que ja foi desbloqueado antes
    var camadas := struct{
        lingua := (do_player.lingua || do_livro.lingua) ? 1 : 0,
        magia  := (do_player.magia  || do_livro.magia)  ? 1 : 0,
        arq    := (do_player.arq    || do_livro.arq)    ? 1 : 0
    };

    // O que ESTE leitor desbloqueou de novo (nao estava no livro antes)
    var novas := struct{
        lingua := (do_player.lingua && !do_livro.lingua) ? 1 : 0,
        magia  := (do_player.magia  && !do_livro.magia)  ? 1 : 0,
        arq    := (do_player.arq    && !do_livro.arq)    ? 1 : 0
    };

    // Persistir o estado atualizado no livro
    SetCamadasDesbloqueadas(book, camadas);

    var titulo;
    var conteudo;
    var tem_alguma := camadas.lingua || camadas.magia || camadas.arq;

    if (tem_alguma)
 titulo := "Texto Parcialmente Decifrado";
        conteudo := MontarConteudoCifrado(lingua, camadas, book);
 SetObjProperty(book, "decifrado_por", who.serial);
        if (novas.lingua || novas.magia || novas.arq)
            _MensagemNovasCamadas(who, novas);
        else
 SendSysMessageEX(who, "Voce rele o livro  -  nada de novo se revela para voce.", SSM_INFO);
        endif
    else
 titulo := "Lingua Desconhecida";
        conteudo := GerarGarbled(nivel, lingua);
 SendSysMessageEX(who, "O texto esta escrito em uma lingua que nao reconheces.", SSM_INFO);
    endif

 SetObjProperty(book, "titulo_gerado", titulo);
 SetObjProperty(book, "conteudo_gerado", conteudo);
 SetObjProperty(book, "tipo_inerte", lingua);

 _PaginarEMostrar(who, titulo, conteudo, lingua, nivel, afinidade, "");
endfunction

function _MensagemNovasCamadas(who, novas)
    var partes := array{};
 if (novas.lingua) partes.append("lingua nativa"); endif
 if (novas.magia) partes.append("magia arcana"); endif
 if (novas.arq) partes.append("conhecimento arqueologico"); endif
 var msg := "Voce revela ";
    var i := 1;
    while (i <= partes.size())
 if (i > 1) msg := msg + (i == partes.size() ? " e " : ", "); endif
        msg := msg + partes[i];
        i := i + 1;
    endwhile
 msg := msg + " do texto.";
    SendSysMessageEX(who, msg, SSM_INFO);
endfunction

//  Montagem do conteudo com separadores de camada 

function MontarConteudoCifrado(lingua, camadas, book)
 var resultado := "";

    if (camadas.lingua)
 var txt := _LerCamada(lingua, "lingua", book);
 if (txt != "")
 resultado := resultado + "<basefont color=#371405>[Fragmento  -  Lingua Nativa]</basefont><br>" + txt;
        endif
    endif

    if (camadas.magia)
 var txt2 := _LerCamada(lingua, "magia", book);
 if (txt2 != "")
 if (resultado != "") resultado := resultado + "<br><br>"; endif
 resultado := resultado + "<basefont color=#0A32 [Leitura Magica]</basefont><br>" + txt2;
        endif
    endif

    if (camadas.arq)
 var txt3 := _LerCamada(lingua, "arq", book);
 if (txt3 != "")
 if (resultado != "") resultado := resultado + "<br><br>"; endif
 resultado := resultado + "<basefont color=#2A6A0A>[Transcricao Arqueologica]</basefont><br>" + txt3;
        endif
    endif

 if (resultado == "") return "O conteudo esta alem da compreensao."; endif
    return resultado;
endfunction

function _LerCamada(lingua, camada, book)
 var df := OpenDataFile(":livros_lacrados:cifrado_textos");
 if (df.errortext) return ""; endif
 var key := "c_" + lingua + "_" + camada;
    var elem := df.FindElement(key);
 if (elem.errortext) return ""; endif
 var n := CInt(elem.GetProp("n"));
 if (n < 1) return ""; endif
    var idx := RandomInt(n) + 1;
    if (book && !book.errortext)
 var fixo := CInt(GetObjProperty(book, "variante_idx"));
        if (fixo >= 1 && fixo <= n) idx := fixo; endif
    endif
 var v := elem.GetProp("v" + idx);
 if (v.errortext) return ""; endif
    return CStr(v);
endfunction

// Fallback: le chave antiga c_<lingua> (retrocompatibilidade)
function GerarConteudoCifrado(lingua, book)
 var df := OpenDataFile(":livros_lacrados:cifrado_textos");
 if (df.errortext) return "Texto corrompido alem do recuperavel."; endif
 var elem := df.FindElement("c_" + lingua);
 if (elem.errortext) return "Texto corrompido alem do recuperavel."; endif
 var n := CInt(elem.GetProp("n"));
    if (n < 1) n := 1; endif
    var idx := RandomInt(n) + 1;
    if (book && !book.errortext)
 var fixo := CInt(GetObjProperty(book, "variante_idx"));
        if (fixo >= 1 && fixo <= n) idx := fixo; endif
    endif
 var v := elem.GetProp("v" + idx);
 if (v.errortext) return "Texto corrompido alem do recuperavel."; endif
    return CStr(v);
endfunction

//  Lingua sorteada por nivel 

function DesenharLingua(nivel)
    case (nivel)
 3: var l3 := array{"aglabita_antigo", "khar_runico", "bjorske_cursivo"}; return l3[RandomInt(3)+1];
 4: var l4 := array{"ahruwen", "idrith_har", "necro_cifrado"}; return l4[RandomInt(3)+1];
 5: var l5 := array{"tsu_ngathar", "primeiro_lacre", "vael_glifo"}; return l5[RandomInt(3)+1];
    endcase
 return "lingua_desconhecida";
endfunction

//  TEXTO GARBLED 

function GerarGarbled(nivel, lingua)
    var palavras := GetPalavrasGarbled(lingua);
    var n := palavras.size();

 var resultado := "";
    var base := 13;
    if (nivel == 4) base := 18; elseif (nivel == 5) base := 24; endif
    var total := base + RandomInt(5);
    var i := 0;
    var na_linha := 0;
    var quebra := 4 + RandomInt(3);

    while (i < total)
        resultado := resultado + palavras[RandomInt(n) + 1];
        i := i + 1;
        na_linha := na_linha + 1;
        if (i < total)
            if (na_linha >= quebra)
 resultado := resultado + "<br>";
                na_linha := 0;
                quebra := 4 + RandomInt(3);
            else
 resultado := resultado + " ";
            endif
        endif
    endwhile

    return resultado;
endfunction

function GetPalavrasGarbled(lingua)
 if (lingua == "aglabita_antigo")
 return array{"kaleth", "bathar", "miraq", "tariq", "hamal", "sefur", "qalath", "zakhr", "haraf", "kothar", "valim", "sarek", "thumal", "zafir", "karath", "kaliram", "mirakhal", "hamalar", "sefarim", "qalathur", "haraqim", "kotharem", "thumalak", "zariqath", "valiqar", "soluqan", "bathirul", "tarkhamal", "seqalifur", "bathariq"};
 elseif (lingua == "khar_runico")
 return array{"groth", "skul", "nord", "sturm", "bram", "durn", "kolt", "vrak", "thrum", "gross", "skarn", "durk", "volk", "kumm", "thruk", "grom", "krath", "durr", "durknot", "thrumkel", "volkrath", "grothul", "nordvak", "sturmkel", "bramvolk", "durknath", "skarnbram", "grothul", "koltdurk", "thrumvolk"};
 elseif (lingua == "bjorske_cursivo")
 return array{"bjorn", "ulv", "fjord", "solv", "mjod", "varg", "strom", "keld", "ork", "fjard", "grod", "svalg", "ylve", "arn", "grimr", "halv", "kveld", "dreng", "svart", "frost", "blod", "eld", "natt", "grimmvard", "fjordalv", "stromkel", "kveldarn", "svartrim", "bjornulv", "fjordgrod"};
 elseif (lingua == "ahruwen")
 return array{"aelith", "syraith", "faeron", "naelis", "sylve", "faeth", "aerith", "thalion", "erendis", "calath", "miriel", "luvien", "anduil", "celeith", "tirith", "galath", "isil", "aluve", "sindel", "ithril", "galve", "sylireth", "thalune", "calaeth", "mirvael", "erendile", "isilthar", "anduirith", "celunith", "sindirael"};
 elseif (lingua == "idrith_har")
 return array{"vel", "zyl", "shar", "draev", "quel", "mith", "vrae", "zyn", "shael", "dris", "velth", "sharath", "mithran", "quelth", "xun", "drel", "ilth", "briv", "xull", "nath", "vist", "relth", "quiv", "dritz", "xundrel", "mithrelv", "shaelquiv", "zynvrath", "quelthraz", "drisvelth"};
 elseif (lingua == "necro_cifrado")
 return array{"kath", "mort", "vorn", "mord", "norv", "grav", "thond", "velt", "krum", "nh", "drend", "gorsk", "pelth", "umrak", "skath", "vrh", "skath", "morth", "velthor", "katharak", "gravund", "morskath", "thondral", "pelthrak", "krumvelt", "gorskath", "nhvorn", "drenvelt", "gravskor", "umrakath"};
 elseif (lingua == "tsu_ngathar")
 return array{"xhat", "zzul", "grik", "thzor", "vrax", "xlith", "thxul", "zzir", "vrath", "xhul", "zzorth", "grith", "thxur", "xlut", "ngor", "thzuth", "zzux", "xhir", "grox", "vraz", "krux", "nguth", "xlurk", "thgrim", "zzurak", "xhurgol", "grizor", "vrathx", "ngorx", "thxirk"};
 elseif (lingua == "primeiro_lacre")
 return array{"calx", "verum", "ductus", "petra", "signum", "arcus", "forma", "vertex", "limen", "corpus", "nexum", "datum", "medium", "orbis", "pactum", "sedes", "tendo", "umbra", "votum", "axis", "cursus", "finis", "gradus", "lapis", "modus", "terminus", "ignis", "caelum", "pontus", "regnum"};
 elseif (lingua == "vael_glifo")
 return array{"vael", "shras", "thul", "raxul", "shaeth", "thulk", "raxur", "vaesh", "thulr", "shrax", "zelve", "prax", "xulor", "thaur", "relve", "praxil", "xuroth", "thaurdis", "relve", "praxuth", "xulorath", "shaelvor", "zelvath", "thauldir", "raxvorn", "vaelthul", "phrul", "xaerv", "phrulath", "zelvuth"};
    endif
 return array{"???", "---", "###", "...", "~~~", "***", "^^^", "|||", ":::", "___", "---", "???", "~~~", "***", "###", "|||", ":::", "^^^", "...", "___"};
endfunction
