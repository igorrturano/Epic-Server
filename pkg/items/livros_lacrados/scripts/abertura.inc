// abertura.inc
// Fluxo de abertura: player clica no livro     abre.

//   -- -- Helpers de imagem   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

// Atribui imagem ao livro na cricao (1-19, sem 9)
function AtribuirLacreImg(book)
    var idx := RandomInt(18) + 1;  // 18 lacres vlidos (1-8, 10-11, 13-20)
    if (idx >= 9)  idx += 1; endif // pula 9
    if (idx >= 12) idx += 1; endif // pula 12
 SetObjProperty(book, "lacre_img", idx);
    return idx;
endfunction

// Retorna o ndice de imagem do lacre (atribui se ainda no tiver)
function GetLacreIdx(book)
 var raw := GetObjProperty(book, "lacre_img");
    if (raw.errortext)
        return AtribuirLacreImg(book);
    endif
    var idx := CInt(raw);
    if (idx < 1)
        return AtribuirLacreImg(book);
    endif
    return idx;
endfunction

//   -- -- Gump Step 1  - livro com lacre ntegro   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

function _GumpStep1(who, book)
    var idx     := GetLacreIdx(book);
    var gump_id := GetLivroLacreGumpId(idx);

    var opts := array{HIDE_BORDER, HIDE_TITLE, HIDE_PAGE};
 var gump := Gump(who, "", 300, 300, opts);

    // Fundo tiled invisvel: drag surface para os pixels transparentes do livro
 gump.AddControl(ImageGumpTiled("bg", 0, 0, 300, 300, LL_GUMP_TRANSP));

    // Livro como imagem esttica (no-interativo = draggvel pelo jogador)
 gump.AddControl(ImageGump("book", 0, 0, gump_id));

    // Boto invisvel 100  100 centrado no lacre (regiao central do livro)
 var btnLacre := Button("btnLacre", 100, 100, LL_GUMP_BTN_LACRE, LL_GUMP_BTN_LACRE);
    gump.AddControl(btnLacre);

    var evento := gump.Show();
    gump.Dispose();

    if (evento.GetControl() == EVENT_ACTION::CLOSED)
        return 0;
    endif

    return evento.GetControl().Equals(btnLacre) ? 1 : 0;
endfunction

//   -- -- AbrirLivroLacrado  - ponto de entrada   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

function AbrirLivroLacrado(who, book)
    if (!_GumpStep1(who, book))
 SendSysMessageEX(who, "Voce recua.", SSM_INFO);
        return;
    endif

 if (CInt(GetObjProperty(book, "aberto")))
 SendSysMessageEX(who, "O livro j foi aberto enquanto decidia.", SSM_INFO);
        return;
    endif

    // --- Ler propriedades antes dos efeitos visuais ---
 var natureza := GetObjProperty(book, "natureza");
 var afinidade := GetObjProperty(book, "afinidade");
 var nivel := CInt(GetObjProperty(book, "nivel_bau"));
    var lugar      := GetLugar(who);
 var lugar_prop := GetObjProperty(book, "lugar_teste");
    if (!lugar_prop.errortext) lugar := CStr(lugar_prop); endif

    if (natureza != NAT_INERTE && natureza != NAT_CIFRADO &&
        natureza != NAT_MANCHADO && natureza != NAT_DESPERTADOR &&
        natureza != NAT_CONVOCADOR && natureza != NAT_PROFETICO)
        return;
    endif

    // --- Animcao de abertura ---
    who.frozen    := 1;
    book.movable  := 0;

 PrintTextAbove(who, "*rompe o lacre com cuidado*");
    PlaySoundEffect(who, 0x0056);
    PlayStationaryEffect(who.x, who.y, who.z, 0x374A, 9, 16, 1, who.realm);

    sleep(2);
 PrintTextAbove(who, "*as paginas comecam a se mover sozinhas*");
    PlaySoundEffect(who, 0x05A);
    sleepms(300);
    PlaySoundEffect(who, 0x05A);
    sleepms(300);
    PlaySoundEffect(who, 0x05A);
    sleep(3);

    who.frozen   := 0;
    book.movable := 1;

 SetObjProperty(book, "aberto", 1);
    SetName(book, "Livro Aberto");

    // --- Discord: notificar abertura (ignora livros de teste GM) ---
 if (!CInt(GetObjProperty(book, "teste_gm")))
        var region_ref := who.region;
 var regiao_discord := "Valtheris";
        if (region_ref && !region_ref.errortext)
            regiao_discord := CStr(region_ref.name);
        endif

 var nome_jog := CStr(GetObjProperty(who, "nome_inicial"));
 if (!nome_jog || nome_jog == "0") nome_jog := CStr(who.name); endif

 var msg_tipo := CStr(natureza) + " N" + CStr(nivel);
        if (natureza == NAT_CONVOCADOR)
 var inv_discord := CStr(GetObjProperty(book, "invocacao"));
 if (inv_discord && inv_discord != "0")
 msg_tipo += " [" + inv_discord + "]";
            endif
        endif

 var discord_msg := "**[LIVRO LACRADO]** Um lacre foi rompido em **" + regiao_discord + "** | " + msg_tipo + " ||" + nome_jog + " [" + who.acctname + "] em " + CStr(who.x) + ", " + CStr(who.y) + ", " + CStr(who.z) + "||";
        PresetDiscordMessage(discord_msg);
    endif

    // --- Assinatura sensorial ---
    if (natureza == NAT_CONVOCADOR || natureza == NAT_DESPERTADOR)
 var invocacao := GetObjProperty(book, "invocacao");
        EmitirAssinaturaSensorial(who, GetTipoEntidade(invocacao));
        sleepms(500);
    endif

 start_script(":livros_lacrados:scripts/naturezas/exec_dispatcher",
        array{who, book, nivel, afinidade, lugar, natureza});
endfunction
