use uo;
use os;
use polsys;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:requestGump";
include ":containers:containers";
include ":tn:cooldown";

const MURAL_GUMP_BACKGROUND  := 39929;
const MURAL_INTERNAL_BG      := 9350;

const MURAL_MAX_POSTS        := 20;
const MURAL_POST_COOLDOWN    := 300;
const MURAL_POSTS_PER_PAGE   := 5;
const MURAL_POST_COST        := 1;

const MURAL_BTN_EXIT         := 0;
const MURAL_BTN_NEW          := 1;
const MURAL_BTN_PREV         := 2;
const MURAL_BTN_NEXT         := 3;
const MURAL_BTN_BACK         := 4;
const MURAL_BTN_SAVE         := 5;
const MURAL_BTN_CANCEL       := 6;
const MURAL_BTN_PUBLISH      := 7;
const MURAL_BTN_EDIT         := 8;
const MURAL_BTN_READ_BASE    := 100;
const MURAL_BTN_DELETE_BASE  := 200;

const MURAL_BTN_GFX_UP       := 0x659;
const MURAL_BTN_GFX_DOWN     := 0x65A;
const MURAL_BTN_DEL_UP       := 30008;
const MURAL_BTN_DEL_DOWN     := 30009;

function LoadPosts(board)
	var posts := GetObjProperty(board, "mural_posts");
	if (!posts || TypeOf(posts) != "Array")
		posts := array{};
	endif
	return posts;
endfunction

function SavePosts(board, posts)
	SetObjProperty(board, "mural_posts", posts);
endfunction

function CanModerate(who)
	return (who.cmdlevel >= 3);
endfunction

function CanEditPost(who, post)
	return (who.serial == post.author_serial || CanModerate(who));
endfunction

function FormatRelativeTime(timestamp)
	var diff := PolCore().systime - CInt(timestamp);
	if (diff < 60)
		return "agora";
	elseif (diff < 3600)
		return "há " + CInt(diff / 60) + " min";
	elseif (diff < 86400)
		return "há " + CInt(diff / 3600) + "h";
	else
		return "há " + CInt(diff / 86400) + "d";
	endif
endfunction

function GFAddMuralActionButton(byref gump, x, y, btn_id, label, color := 53)
	GFAddButton(gump, x, y, MURAL_BTN_GFX_UP, MURAL_BTN_GFX_DOWN, GF_CLOSE_BTN, btn_id);
	GFTextLine(gump, x + 20, y, color, label);
endfunction

function GFAddMuralDeleteButton(byref gump, x, y, btn_id, label, color := 33)
	GFAddButton(gump, x, y, MURAL_BTN_DEL_UP, MURAL_BTN_DEL_DOWN, GF_CLOSE_BTN, btn_id);
	GFTextLine(gump, x + 20, y, color, label);
endfunction

function AdicionarMolduraMural(byref gump, width, height)
	var molduras := array{
		struct{ x := 0,         y := 0,          width := 25,         height := 25,         gump := 39925 },
		struct{ x := width-25,  y := 0,          width := 35,         height := 25,         gump := 39927 },
		struct{ x := 25,        y := 0,          width := width-41,   height := 25,         gump := 39926 },
		struct{ x := 25,        y := height-25,  width := width-25,   height := 35,         gump := 39932 },
		struct{ x := 0,         y := 25,         width := 10,         height := height-50,  gump := 39928 },
		struct{ x := width-25,  y := 25,         width := 35,         height := height-50,  gump := 39930 },
		struct{ x := width-25,  y := height-25,  width := 35,         height := 35,         gump := 39933 },
		struct{ x := 0,         y := height-25,  width := 35,         height := 35,         gump := 39931 }
	};
	foreach moldura in molduras
		GFGumpPicTiled(gump, moldura.x, moldura.y, moldura.width, moldura.height, moldura.gump);
	endforeach
endfunction

function ShowMuralListGump(who, board, page := 1)
	var posts := LoadPosts(board);
	var total := posts.size();
	var pages := CInt((total + MURAL_POSTS_PER_PAGE - 1) / MURAL_POSTS_PER_PAGE);
	if (pages < 1)
		pages := 1;
	endif
	if (page < 1)
		page := 1;
	endif
	if (page > pages)
		page := pages;
	endif

	var gump_w := 620;
	var gump_h := 440;

	var gump := GFCreateGump();
	GFClosable(gump, 1);
	GFDisposable(gump, 0);

	GFGumpPicTiled(gump, 0, 0, gump_w, gump_h, MURAL_GUMP_BACKGROUND);
	AdicionarMolduraMural(gump, gump_w, gump_h);

	GFResizePic(gump, 15, 50, MURAL_INTERNAL_BG, gump_w - 30, gump_h - 100);
	GFTextMid(gump, 25, 22, gump_w - 50, 53, "Mural da Cidade");

	if (total == 0)
		GFTextMid(gump, 25, 180, gump_w - 50, 1153, "Ainda não há postagens neste mural.");
	else
		var start := (page - 1) * MURAL_POSTS_PER_PAGE + 1;
		var stop := start + MURAL_POSTS_PER_PAGE - 1;
		if (stop > total)
			stop := total;
		endif

		var y := 65;
		for i := start to stop
			var post := posts[i];
			var title_disp := post.title;
			if (Len(title_disp) > 30)
				title_disp := title_disp[1, 28] + "..";
			endif

			GFTextLine(gump, 35, y, 53, title_disp);
			GFTextLine(gump, 35, y + 18, 1153, "por " + post.author_name + " - " + FormatRelativeTime(post.timestamp));

			GFAddMuralActionButton(gump, gump_w - 200, y + 4, MURAL_BTN_READ_BASE + i, "Ler");

			if (CanEditPost(who, post))
				GFAddMuralDeleteButton(gump, gump_w - 110, y + 4, MURAL_BTN_DELETE_BASE + i, "Excluir");
			endif

			y += 50;
		endfor
	endif

	var footer_y := gump_h - 45;

	if (page > 1)
		GFAddMuralActionButton(gump, 30, footer_y, MURAL_BTN_PREV, "Anterior");
	endif
	GFTextLine(gump, 130, footer_y, 1153, "Pág " + page + "/" + pages);
	if (page < pages)
		GFAddMuralActionButton(gump, 200, footer_y, MURAL_BTN_NEXT, "Próxima");
	endif

	GFAddMuralActionButton(gump, gump_w - 130, footer_y, MURAL_BTN_NEW, "Nova Postagem");

	var input := GFSendGump(who, gump);
	var btn := input[0];

	if (btn == MURAL_BTN_NEW)
		return struct{ "cmd" := "compose", "page" := page };
	elseif (btn == MURAL_BTN_PREV)
		return struct{ "cmd" := "list", "page" := page - 1 };
	elseif (btn == MURAL_BTN_NEXT)
		return struct{ "cmd" := "list", "page" := page + 1 };
	elseif (btn >= MURAL_BTN_DELETE_BASE)
		return struct{ "cmd" := "delete", "index" := btn - MURAL_BTN_DELETE_BASE, "page" := page };
	elseif (btn >= MURAL_BTN_READ_BASE)
		return struct{ "cmd" := "view", "index" := btn - MURAL_BTN_READ_BASE, "page" := page };
	else
		return struct{ "cmd" := "exit" };
	endif
endfunction

function ShowMuralViewGump(who, post)
	var has_image := (post.url && post.url != "");

	var img_w := 520;
	var img_h := 768;

	var gump_w;
	var gump_h;
	if (has_image)
		gump_w := img_w + 70;
		gump_h := img_h + 360;
	else
		gump_w := 700;
		gump_h := 560;
	endif

	var gump := GFCreateGump();
	GFClosable(gump, 1);
	GFDisposable(gump, 0);

	GFGumpPicTiled(gump, 0, 0, gump_w, gump_h, MURAL_GUMP_BACKGROUND);
	AdicionarMolduraMural(gump, gump_w, gump_h);

	GFResizePic(gump, 15, 50, MURAL_INTERNAL_BG, gump_w - 30, gump_h - 100);

	GFTextMid(gump, 25, 22, gump_w - 50, 53, post.title);
	GFTextMid(gump, 25, 60, gump_w - 50, 1153, "por " + post.author_name + " - " + FormatRelativeTime(post.timestamp));

	var content_y := 90;

	if (has_image)
		var img_x := (gump_w - img_w) / 2;
		GFHTMLArea(gump, img_x, content_y, img_w, img_h, "<img src=\"" + post.url + "\" width=\"" + img_w + "\" height=\"" + img_h + "\" x=\"0\" y=\"0\">");
		content_y += img_h + 20;
	endif

	var body_h := gump_h - content_y - 70;
	GFHTMLArea(gump, 35, content_y, gump_w - 70, body_h, post.body, 0, 1);

	GFAddMuralActionButton(gump, gump_w - 100, gump_h - 38, MURAL_BTN_BACK, "Voltar");

	GFSendGump(who, gump);
endfunction

function IsMuralWordBreaker(ch)
	if (!ch || ch == "")
		return 1;
	endif
	if (ch == " " || ch == "." || ch == "," || ch == "!" || ch == "?" ||
	    ch == ";" || ch == ":" || ch == ")" || ch == "]" || ch == "}")
		return 1;
	endif
	return 0;
endfunction

function ExtractMuralBookText(book)
	var contents := book.GetContents();
	if (!contents || contents.size() == 0)
		return "";
	endif

	var text := "";
	foreach line in contents
		if (line && line != "")
			if (text != "")
				var lastChar := text[len(text), 1];
				var firstChar := line[1, 1];
				if (IsMuralWordBreaker(lastChar) || firstChar == " ")
					text := text + " " + line;
				else
					text := text + line;
				endif
			else
				text := line;
			endif
		endif
	endforeach

	text := StrReplace(text, "  ", " ");
	return text;
endfunction

function CreateMuralTempBook(player)
	var book := CreateItemAtLocation(player.x, player.y, player.z, "book5", 1, player.realm);
	if (!book || book.errortext)
		return error;
	endif
	book.SetTitle("Postagem do Mural");
	book.SetAuthor(player.name);
	SetObjProperty(book, "#TempMuralBook", 1);
	return book;
endfunction

function DestroyMuralTempBook(book)
	if (!book || book.errortext)
		return;
	endif
	ReleaseItem(book);
	sleepms(50);
	if (!DestroyItem(book))
		sleepms(200);
		ReleaseItem(book);
		DestroyItem(book);
	endif
endfunction

function CleanupMuralTempBooks(player)
	var backpack := player.backpack;
	if (!backpack)
		return;
	endif
	foreach item in EnumerateItemsInContainer(backpack)
		if (GetObjProperty(item, "#TempMuralBook"))
			DestroyMuralTempBook(item);
		endif
	endforeach
endfunction

function ShowMuralBookConfirmGump(who)
	var gump := GFCreateGump(200, 200);
	GFClosable(gump, 1);
	GFDisposable(gump, 0);
	GFResizePic(gump, 0, 0, GFCfgConst("Defaults", "BackGround"), 380, 220);
	GFResizePic(gump, 10, 10, GFCfgConst("Defaults", "ForeGround"), 360, 30);
	GFTextMid(gump, 10, 15, 360, 2100, "Nova Postagem");

	GFResizePic(gump, 10, 45, GFCfgConst("Backgrounds", "GOLDTRIM"), 360, 125);
	GFTextLine(gump, 20, 55, 1153, "Edite o livro na sua mochila.");
	GFTextLine(gump, 20, 75, 33, "ATENÇÃO: feche o livro antes de");
	GFTextLine(gump, 20, 90, 33, "clicar em Concluído, senão o texto");
	GFTextLine(gump, 20, 105, 33, "não será salvo.");
	GFTextLine(gump, 20, 130, 1153, "Concluído: pré-visualiza a postagem.");
	GFTextLine(gump, 20, 148, 1153, "Cancelar: descarta tudo.");

	GFResizePic(gump, 10, 175, GFCfgConst("Defaults", "ForeGround"), 360, 35);
	GFAddMuralActionButton(gump, 30, 182, MURAL_BTN_SAVE, "Concluído", 2100);
	GFAddMuralDeleteButton(gump, 230, 182, MURAL_BTN_CANCEL, "Cancelar", 2100);

	return GFSendGump(who, gump);
endfunction

function ShowMuralPreviewGump(who, draft)
	var has_image := (draft.url && draft.url != "");
	var img_w := 520;
	var img_h := 768;

	var gump_w;
	var gump_h;
	if (has_image)
		gump_w := img_w + 70;
		gump_h := img_h + 380;
	else
		gump_w := 700;
		gump_h := 580;
	endif

	var gump := GFCreateGump();
	GFClosable(gump, 1);
	GFDisposable(gump, 0);

	GFGumpPicTiled(gump, 0, 0, gump_w, gump_h, MURAL_GUMP_BACKGROUND);
	AdicionarMolduraMural(gump, gump_w, gump_h);

	GFResizePic(gump, 15, 50, MURAL_INTERNAL_BG, gump_w - 30, gump_h - 100);

	GFTextMid(gump, 25, 22, gump_w - 50, 53, "Pré-visualização");
	GFTextMid(gump, 25, 60, gump_w - 50, 53, draft.title);
	GFTextMid(gump, 25, 78, gump_w - 50, 1153, "por " + who.name);

	var content_y := 105;
	if (has_image)
		var img_x := (gump_w - img_w) / 2;
		GFHTMLArea(gump, img_x, content_y, img_w, img_h, "<img src=\"" + draft.url + "\" width=\"" + img_w + "\" height=\"" + img_h + "\" x=\"0\" y=\"0\">");
		content_y += img_h + 20;
	endif

	var body_h := gump_h - content_y - 80;
	GFHTMLArea(gump, 35, content_y, gump_w - 70, body_h, draft.body, 0, 1);

	var footer_y := gump_h - 38;
	GFAddButton(gump, 30, footer_y, MURAL_BTN_GFX_UP, MURAL_BTN_GFX_DOWN, GF_CLOSE_BTN, MURAL_BTN_PUBLISH);
	GFTooltip(gump, 1042971, "Custa " + MURAL_POST_COST + " moeda de cobre.");
	GFTextLine(gump, 50, footer_y, 53, "Publicar");

	GFAddMuralActionButton(gump, 150, footer_y, MURAL_BTN_EDIT, "Editar");
	GFAddMuralDeleteButton(gump, gump_w - 110, footer_y, MURAL_BTN_CANCEL, "Cancelar");

	var input := GFSendGump(who, gump);
	var btn := input[0];

	if (btn == MURAL_BTN_PUBLISH)
		return "publish";
	elseif (btn == MURAL_BTN_EDIT)
		return "edit";
	else
		return "cancel";
	endif
endfunction

function RunMuralComposeFlow(who)
	var cd := getCooldown(who, "mural_post_cd");
	if (cd > 0)
		SendSysMessage(who, "Aguarde " + cd + " segundos para postar novamente.");
		return 0;
	endif

	CleanupMuralTempBooks(who);

	var title := cstr(RequestGump(who, "Titulo da postagem:"));
	if (!title || title == "")
		SendSysMessage(who, "Postagem cancelada.");
		return 0;
	endif

	var url := cstr(RequestGump(who, "URL da imagem (opcional, deixe vazio para nenhuma):"));
	if (url == error)
		url := "";
	endif

	if (!who.backpack)
		SendSysMessage(who, "Voce precisa de uma mochila para escrever a postagem.");
		return 0;
	endif

	SendSysMessage(who, "Um livro aparecera na sua mochila para escrever o texto.");
	SendSysMessage(who, "Escreva ou cole o conteudo. IMPORTANTE: feche o livro antes de clicar em Concluído, senão o texto não será salvo.");

	var book := CreateMuralTempBook(who);
	if (!book || book.errortext)
		SendSysMessage(who, "Erro ao criar livro temporario.");
		return 0;
	endif

	if (!MoveItemToContainer(book, who.backpack))
		SendSysMessage(who, "Erro ao mover livro para a mochila.");
		DestroyMuralTempBook(book);
		return 0;
	endif

	while (1)
		SendOpenBook(who, book);
		var confirm := ShowMuralBookConfirmGump(who);
		if (!confirm || confirm[0] != MURAL_BTN_SAVE)
			SendSysMessage(who, "Postagem cancelada.");
			DestroyMuralTempBook(book);
			return 0;
		endif

		var body := ExtractMuralBookText(book);
		if (!body || body == "")
			SendSysMessage(who, "Texto vazio. Escreva algo no livro antes de concluir.");
			continue;
		endif

		var draft := struct{ "title" := title, "url" := url, "body" := body };
		var choice := ShowMuralPreviewGump(who, draft);

		if (choice == "publish")
			var paid := who.spendgold(MURAL_POST_COST, 0);
			if (paid == error)
				SendSysMessage(who, "Você não tem " + MURAL_POST_COST + " moeda de cobre para publicar. Adquira moedas e tente novamente, ou cancele.");
				continue;
			endif
			DestroyMuralTempBook(book);
			return draft;
		elseif (choice == "edit")
			SendSysMessage(who, "Voltando para edição. Reabra o livro na sua mochila.");
			continue;
		else
			SendSysMessage(who, "Postagem cancelada.");
			DestroyMuralTempBook(book);
			return 0;
		endif
	endwhile
endfunction

function ValidateAndSubmitPost(who, board, draft)
	var title := draft.title;
	var url := draft.url;
	var body := draft.body;

	if (Len(body) == 0)
		SendSysMessage(who, "O texto da postagem é obrigatório.");
		return 0;
	endif

	if (Len(title) == 0)
		SendSysMessage(who, "O título é obrigatório.");
		return 0;
	endif

	var posts := LoadPosts(board);
	if (posts.size() >= MURAL_MAX_POSTS)
		SendSysMessage(who, "O mural está cheio. Aguarde até que uma postagem seja removida.");
		return 0;
	endif

	var next_id := CInt(GetObjProperty(board, "mural_next_id"));
	if (next_id < 1)
		next_id := 1;
	endif

	var post := struct{
		"id"              := next_id,
		"title"           := title,
		"body"            := body,
		"url"             := url,
		"author_serial"   := who.serial,
		"author_name"     := who.name,
		"author_acctname" := who.acctname,
		"timestamp"       := PolCore().systime
	};

	posts.insert(1, post);

	SavePosts(board, posts);
	SetObjProperty(board, "mural_next_id", next_id + 1);
	setCooldown(who, "mural_post_cd", MURAL_POST_COOLDOWN);

	SendSysMessage(who, "Postagem publicada no mural.");
	return 1;
endfunction

function DeletePostByIndex(who, board, index)
	var posts := LoadPosts(board);
	if (index < 1 || index > posts.size())
		return 0;
	endif
	if (!CanEditPost(who, posts[index]))
		SendSysMessage(who, "Você não tem permissão para remover esta postagem.");
		return 0;
	endif
	posts.erase(index);
	SavePosts(board, posts);
	SendSysMessage(who, "Postagem removida.");
	return 1;
endfunction
