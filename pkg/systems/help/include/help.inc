use uo;
use datafile;
use file;
use os;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:requestGump";
include ":gumps:yesNo";
include ":gumps:dropDownGump";
include ":tn:tngumps";
include ":datafile:datafile";
include ":DiscordWebhook:outbound";
include ":attributes:advanceCheck";
include "include/say";


const BUTTON_HELP_CANCEL := 5001;
const BUTTON_HELP_SENDGMPAGE := 5002;
const BUTTON_HELP_REPORTBUG := 5003;
const BTN_BACK := 5004;

// =====================================
// ABRIR INDEX
// =====================================
function Help_OpenIndex(who, opened := "", search := "")

    var df := DFOpenDataFile(":help:index");

    var g := GFCreateGump(10, 10);
  GFMovable(g, 0);
   GFResizePic(g, 0, 25, 3600, 360, 580);
    GFResizePic(g, 15, 40, 0xA3C, 330, 555);
    GFTextMid(g, 20, 50, 300, 1153, "HELP");

    // =========================
    // SEARCH BAR
    // =========================
// =========================
// SEARCH BAR (COM MOLDURA)
// =========================

// fundo externo (moldura)
GFPicTiled(g, 195, 48, 115, 24, 0xBBC);

// fundo interno escuro
GFPicTiled(g, 196, 49, 113, 22, 0xA40);
GFAddAlphaRegion(g, 196, 49, 113, 22);

// campo de texto
GFTextEntry(g, 200, 50, 105, 20, 1153, search);
var ENTRY_SEARCH := g.data_id;

// botão de busca
GFAddButton(g, 315, 50, 1531, 1532, GF_CLOSE_BTN, 9000);
GFTooltipText( g, "Search" );

    var results := array{};

    if (search and Len(search) > 2)
        var search_lower := Lower(search);
        foreach k in DFGetElemNames(df)
            sleepms(2);
            var elem := DFFindElement(df, k);
            if (!elem)
                continue;
            endif
            var name := Lower(elem.GetProp("Name"));
            var cat := Lower(elem.GetProp("Category"));
            var sub := Lower(elem.GetProp("Index"));
            if (Find(name, search_lower, 1) or
               Find(cat, search_lower, 1) or
               Find(sub, search_lower, 1))
                results.append(struct{
                key := k,
                name := elem.GetProp("Name")});
            endif
        endforeach
    endif

    var y := 90;
    var id := 2;
    var topic_map := array{};

    if (results.size() > 0)
        GFTextLine(g, 25, y, 1153, "Resultados:");
        y := y + 20;
        foreach r in results
            GFAddButton(g, 25, y, 2117, 2118, GF_CLOSE_BTN, id);
            GFTextLine(g, 55, y, 1153, r.name);
            topic_map[id] := r.key;
            id := id + 1;
            y := y + 20;
        endforeach
    else
    endif

    // =========================
    // LISTA DE CATEGORIAS
    // =========================
    var categories := array{};
	var cat_count := dictionary{};

 foreach k in DFGetElemNames(df)

    var elem := DFFindElement(df, k);
    if (!elem)
        continue;
    endif

    var cat := elem.GetProp("Category");

    if (!cat)
        cat := "Outros";
    endif

    // conta quantos itens tem
    if (!cat_count[cat])
        cat_count[cat] := 1;
    else
        cat_count[cat] := cat_count[cat] + 1;
    endif

endforeach

foreach cat in (cat_count.Keys())

    categories.append(struct{
        name := cat,
        count := cat_count[cat]
    });

endforeach

// =========================
// SORT MANUAL (MAIOR → MENOR)
// =========================
var i;
var j;

for (i := 1; i <= categories.size(); i := i + 1)

    for (j := i + 1; j <= categories.size(); j := j + 1)

        if (categories[j].count > categories[i].count)

            var temp := categories[i];
            categories[i] := categories[j];
            categories[j] := temp;

        endif

    endfor

endfor

    // =========================
    // DESENHAR MENU
    // =========================
    foreach c in categories
		var cat := c.name;
		sleepms(2);
        // botão categoria

    if (opened == cat)
    // botão invertido (parece pressionado)
        GFAddButton(g, 25, y, 56, 555, GF_CLOSE_BTN, id);
        GFTextLine(g, 50, y, 66, "[" + cat + "]");
    else
        GFAddButton(g, 25, y, 55, 56, GF_CLOSE_BTN, id);
        GFTextLine(g, 50, y, 1153, "[" + cat + "]");
    endif

        topic_map[id] := "CAT|" + cat;

        id := id + 1;
        y := y + 20;

        // =========================
        // SE ABERTO → MOSTRA ITENS
        // =========================
        if (opened == cat)

            foreach k in DFGetElemNames(df)
				sleepms(2);
                var elem := DFFindElement(df, k);
                if (!elem)
                    continue;
                endif

                var cat_elem := elem.GetProp("Category");
                if (!cat_elem)
                    cat_elem := "Outros";
                endif
                if (cat_elem == cat)
                    var name := elem.GetProp("Name");
                    GFAddButton(g, 40, y, 2117, 2118, GF_CLOSE_BTN, id);
                    GFTextLine(g, 70, y, 1153, "- " + name);
                    topic_map[id] := k;
                    id := id + 1;
                    y := y + 20;
                endif
            endforeach

        endif

    endforeach

    // =========================
    // ENVIAR
    // =========================
var response := GFSendGump(who, g);
var button := response[0];

if (!button)
    return;
elseif (button == 9000)
    var new_search := GFExtractData(response, ENTRY_SEARCH);
    if (!new_search)
        new_search := "";
    endif
    Help_OpenIndex(who, "", new_search);
    return;
endif

var action := topic_map[button];

if (!action)
    return;
endif

    // =========================
    // CLICK CATEGORIA
    // =========================
    if (SubStr(action, 1, 4) == "CAT|")

        var cat := SubStr(action, 5, Len(action));

        if (opened == cat)
            Help_OpenIndex(who, "");
        else
            Help_OpenIndex(who, cat);
        endif

        return;

    endif
var pid := GetPid();
    // =========================
    // CLICK TOPICO
    // =========================
Help_OpenTopic(who, action, pid);

endfunction

// =====================================
// ABRIR TÓPICO
// =====================================
function Help_OpenTopic(who, key, pidPai, ddManager := 0)

    var df := DFOpenDataFile(":help:index");

    if (!df)
        SendSysMessage(who, "Erro ao abrir help.");
        return;
    endif

var elemIndex := DFFindElement(df, key);

if (!elemIndex)
    // fallback: não faz nada em vez de quebrar
    return;
endif

var path := elemIndex.GetProp("Path");

// 🔥 CORREÇÃO DO "s" BUGADO
if (path and SubStr(path, 1, 1) == "s")
    path := SubStr(path, 2, Len(path));
endif

if (!path)
    SendSysMessage(who, "Help inválido.");
    return;
endif
var cfg := ReadConfigFile(path);

if (!cfg)
    SendSysMessage(who, "Erro ao abrir cfg.");
    return;
endif

var elem := cfg["Topic"];

if (!elem)
    SendSysMessage(who, "Help não encontrado no cfg.");
    return;
endif


var name := GetConfigString(elem, "Name");
var content := GetConfigString(elem, "Content");

    if (!name)
        name := "Ajuda";
    endif

    if (!content)
        content := "Sem conteúdo.";
    endif

    // =========================
    // CRIA GUMP BASE
    // =========================
    var g := GFCreateGump(10, 10);
  GFMovable(g, 0);
    GFResizePic(g, 0, 25, 3600, 360, 580);
    GFResizePic(g, 15, 40, 0xA3C, 330, 520);

    GFTextMid(g, 20, 50, 300, 1153, name.Upper());
GFAddAlphaRegion(g, 20, 149, 20, 360);
    // conteúdo principal
    GFHTMLArea(g, 20, 95, 320, 450, content, 1, 1);

    // =========================
    // SEE ALSO (BOTÕES)
    // =========================
    var topic_map := dictionary{};
    var BTN_SEEALSO := 2001;

    GFAddButton(g, 20, 570, 2117, 2118, GF_CLOSE_BTN, BUTTON_HELP_REPORTBUG);
    GFTextLine(g, 40, 570, 1153, "Report Bug");

    GFAddButton(g, 120, 570, 2117, 2118, GF_CLOSE_BTN, BUTTON_HELP_SENDGMPAGE);
    GFTextLine(g, 140, 570, 1153, "Enviar GMPage");

    GFAddButton(g, 240, 570, 2117, 2118, GF_CLOSE_BTN, BTN_SEEALSO);
    GFTextLine(g, 260, 570, 1153, "See Also");
	
	GFAddButton(g, 30, 55, 4014, 4015, GF_CLOSE_BTN, BTN_BACK);

    // =========================
    // GERAR RELACIONADOS
    // =========================
var subcat_str := GetConfigString(elem, "SubCategory");

if (!subcat_str)
    subcat_str := "";
endif

var subcats := SplitWords(subcat_str);

var related := array{};

foreach k in DFGetElemNames(df)
    sleepms(2);

    if (k == key)
        continue;
    endif

    var e := DFFindElement(df, k);
    if (!e)
        continue;
    endif

    var sc := e.GetProp("SubCategory");

    if (!sc)
        continue;
    endif

    foreach s in subcats
        sleepms(1);

        if (Find(Lower(sc), Lower(s), 1))
            related.append(struct{
                key := k,
                name := e.GetProp("Name")
            });
            break;
        endif
    endforeach
endforeach

    // =========================
    // DROPDOWN SETUP (COM ESTADO)
    // =========================
    if (!ddManager)
        ddManager := GerenciadorDropDown(10, 2000, 3000, 5);
    endif

    var dd := ddManager.ObtemDropDown(1, 200);
    foreach r in related
        sleepms(2);
dd.AdicionaOpcao(r.name, _r_iter, r.key);
    endforeach



    // =========================
    // ENVIAR GUMP
    // =========================
pidPai := GetPid();  
 var response := GFSendGump(who, g);
    var button := response[0];

    if (!button)
        return;

    elseif (button == BUTTON_HELP_SENDGMPAGE)
        var sendpage := YesNo(who, "Só envie se seu problema só puder ser resolvido por GMs. Atenção: você só pode ter uma page ativa — uma nova substituirá a anterior. Continuar?",
            "Sim, quero.", "Não, não quero enviar a msg.");
        if (sendpage)
            var msg := SendPageDialog(who);
            var discord_user := RequestGump(who, "Seu usuário no Discord (opcional, deixe vazio para pular)", "", "", 1);
            SendSysMessage(who, "Sua msg foi enviada aos GMs.");
            SendGmPage(who, msg, discord_user);
        endif
        return;

    elseif (button == BUTTON_HELP_REPORTBUG)
        var sendpage3 := YesNo(who, "Um Bug só pode ser resolvido por Scripters. Detalhe o máximo possível como você descobriu o bug. Continuar?",
            "Sim, quero.", "Não, não tenho bug para reportar.");
        if (sendpage3)
            var title := RequestGump(who, "Digite um título para o bug (Até 5 Palavras)", "", "", 1);
            var msg3 := SendPageDialog(who);
            var discord_user3 := RequestGump(who, "Seu usuário no Discord (opcional, deixe vazio para pular)", "", "", 1);
            SendSysMessage(who, "O bug foi reportado para a staff.");
            ReportBug(who, msg3, title, discord_user3);
        endif
        return;
    elseif (button == BTN_BACK)
		Help_OpenIndex(who);
		return;
    endif

        // =========================
    // CLICK SEE ALSO
    // =========================
if (button == dd.PegaIdBottao())

dd.AtribuiX(360);
dd.AtribuiY(370);

    dd.SendGump(who, pidPai);

    // REABRE O GUMP PRINCIPAL
    Help_OpenTopic(who, key, pidPai, ddManager);

    return;
endif

    // =========================
    // CLICK OPÇÃO DO DROPDOWN
    // =========================
var selected := ddManager.DefineValorEscolhido(button);

if (selected)
    var valor := selected.PegaValorEscolhido();

    if (valor)
        Help_OpenTopic(who, valor, pidPai, ddManager);
        return;
    endif
endif

endfunction

// ========================PARTE DO PAGE E REPORT===========================

function GetCharacterName(who)
	var nome := CStr(GetObjProperty(who, "nome_inicial"));
	if (!nome || nome == "0")
		nome := CStr(who.name);
	endif
	return nome;
endfunction

function BuildDiscordMessage(label, nome, preview, discord_user := "")
	var msg := "***[" + label + "]*** ***" + nome + "*** -> " + preview;
	if (discord_user && discord_user != "0")
		msg := msg + " | Discord: @" + discord_user;
	endif
	return msg;
endfunction

function NotifyOnlineGMs(tipo, nome)
	var ppl := EnumerateOnlineCharacters();
	foreach x in ppl
		if (x.cmdlevel > 0)
			Start_script(":help:gump_notif_gm", {x, tipo, nome});
		endif
	endforeach
endfunction

function SendGmPage(who, msg, discord_user := "")
	var nome := GetCharacterName(who);

	var df := DFOpenDataFile(":help:gmpages", DF_CREATE);
	var elem := DFFindElement(df, who.serial, DF_CREATE);
	elem.SetProp("page", msg);
	elem.SetProp("date", polcore().systime);

	var preview := SubStr(msg, 1, 100);
	PresetDiscordMessage(BuildDiscordMessage("GMPage", nome, preview, discord_user));
	NotifyOnlineGMs("gmpage", nome);
endfunction

function SendCounselorPage(who, msg)
	var df := DFOpenDataFile(":help:counpages", DF_CREATE);
	var elem := DFFindElement(df, who.serial, DF_CREATE);
	elem.SetProp( "page", msg  );
	GMBcast("Nova page para counselor recebida!");
	GMBcast("Enviada por " + who.name);
	GMBcast("Texto " + msg);
	CounBcast("Nova page para counselor recebida!");
	CounBcast("Enviada por " + who.name);
	CounBcast("Texto " + msg);
endfunction

function ReportBug(who, msg, title, discord_user := "")
	var nome := GetCharacterName(who);

	var df := DFOpenDataFile(":help:bugs", DF_CREATE, DF_KEYTYPE_INTEGER);
	var total := df.Keys();
	var num := cint(total.size()) + 1;
	var elem := df.CreateElement(num);
	elem.SetProp("page", msg);
	elem.SetProp("ReporterSerial", who.serial);
	elem.SetProp("ReporterCharacter", who.name);
	elem.SetProp("ReporterAccount", who.acctname);
	elem.SetProp("Title", title);

	var preview := SubStr(msg, 1, 100) + " | Titulo: " + title;
	PresetDiscordMessage(BuildDiscordMessage("BugReport", nome, preview, discord_user));
	NotifyOnlineGMs("bug", nome);
endfunction

function GMBcast(msg)
	var ppl := EnumerateOnlineCharacters();
	foreach x in ppl
		if (x.cmdlevel > 0)
			SendSysMessage(x, ""+msg);
		endif
	endforeach
endfunction

function CounBcast(msg)
	var ppl := EnumerateOnlineCharacters();
	foreach x in ppl
		if (GetObjProperty(x, "counselor") == 1)
			SendSysMessage(x, ""+msg);
		endif
	endforeach
endfunction

//pagefille: gmpages, counpages, bugs
function GetPageIndexByPageNum(num, pagefile)
	var df := DFOpenDataFile(pagefile, DF_CREATE, DF_KEYTYPE_INTEGER);
	var pages := df.Keys();
	if (pages[num])
		return pages[num];
	else
		return 0;
	endif
endfunction

function PageDescGump(who, elem)
	var chargump := GFCreateGump();
	var input;

	GFDisposable(chargump, 0);
	GFResizePic(chargump, 0, 25, 3600, 300, 380);
	GFResizePic(chargump, 15, 40, 0xA3C, 270, 350);
	//GFTextMid(chargump, 15, 35, 600, 1153, "Descricao da page");

	var lines := elem.GetProp("page");

	GFHTMLArea(chargump, 22, 61, 259, 303, lines, 0, 1);
	input := GFSendGump(who, chargump);
	input := input[0];
endfunction

function DeletePage(serial, pagetype)

	if ( serial )
		var df := DFOpenDataFile(pagetype);
		df.DeleteElement(serial);
		return 1;
	endif
	return 0;
endfunction

function SendPageDialog(who)
	var gump := GFCreateGump();
	var num_lines := 15;
	var msg_array := array;
	var y_pos := 80;
	var i;

	GFResizePic(gump, 0, 0, 3600, 420, 405);
	GFResizePic(gump, 15, 15, 0x13BE, 390, 55);
	GFTextMid(gump, 15, 20, 390, 1153, "Descreva seu problema");
	GFAddButton(gump, 325, 45, 2128, 2129, GF_CLOSE_BTN, 20);
	GFResizePic(gump, 15, 70, 0xA3C, 390, 320);
	for (i := 1; i <= num_lines; i := i + 1)
		msg_array.Append(GFTextEntry(gump, 20, y_pos, 396, 20, 1153, ""));
		y_pos := y_pos + 19;
	endfor

	var input := GFSendGump(who, gump);

	if (input[0] == 20)
		var i;
		for (i := 1; i <= msg_array.Size(); i := i + 1)
			var line := GFExtractData(input, msg_array[i]);
			// sendsysmessage(who, "extraindo linha " + line);
			if (line)
				line := line + " ";
				msg_array[i] := line;
			else
				msg_array[i] := "";
			endif
		// sendsysmessage(who, "linha final " + line);
		endfor

		// Clear off empty lines.
		for (i := msg_array.Size(); i >= 1; i := i - 1)
			if (Len(msg_array[i]) <= 0)
				msg_array.Erase(i);
			endif
			SleepMS(2);
		endfor

		var text := "";
		for (i := 1; i <= msg_array.Size(); i := i + 1)
			text := text + msg_array[i];
			SleepMS(2);
		endfor
		return text;
	endif
endfunction

function GMPageGump(who)

	var pagegump := GFCreateGump();
	var input;

	GFDisposable(pagegump, 0);
	GFResizePic(pagegump, 0, 25, 3600, 525, 480);
	GFResizePic(pagegump, 15, 40, 0x13BE, 495, 450);
	GFTextMid(pagegump, 20, 50, 370, 1153, "Menu de Pages");

	GFTextLine(pagegump, 20, 70, 1153, "Nome");
	GFTextLine(pagegump, 160, 70, 1153, "Goto");
	GFTextLine(pagegump, 200, 70, 1153, "Msg");
	GFTextLine(pagegump, 240, 70, 1153, "Summon");
	GFTextLine(pagegump, 300, 70, 1153, "Descricao");
	GFTextLine(pagegump, 370, 70, 1153, "Deletar");
	GFTextLine(pagegump, 445, 70, 1153, "Tempo");

	var df := DFOpenDataFile("gmpages", DF_CREATE);
	var pages := df.Keys();

	var y := 90;
	var i := 1;
	var pagenum := 0;
	var elem;
	var player;
	var fontcolor := 1153;
	foreach page in pages
		pagenum := pagenum + 1;
		GFPage(pagegump, i);
		elem := df.FindElement(page);
		player := SystemFindObjectBySerial(Cint(page));
		var getdate := DFFindElement(DFOpenDataFile("gmpages", DF_CREATE, DF_KEYTYPE_INTEGER), cint(page), DF_CREATE);
		GFTextLine(pagegump, 20, y, fontcolor, player.name + " (" + player.acctname + ")");
		GFAddButton(pagegump, 165, y, 2117, 2118, 1, 200 + pagenum);
		GFAddButton(pagegump, 205, y, 2117, 2118, 1, 400 + pagenum);
		GFAddButton(pagegump, 255, y, 2117, 2118, 1, 600 + pagenum);
		GFAddButton(pagegump, 320, y, 2117, 2118, 1, 800 + pagenum);
		GFAddButton(pagegump, 380, y, 2117, 2118, 1, 1000 + pagenum);
		GFTextLine(pagegump, 442, y, fontcolor, CalculateDaysInReal(getdate.GetProp("date")) + " dias");
		y := y + 20;
		if (Cdbl(pagenum / 18) == i)
			i := i + 1;
			y := 90;
			GFAddButton(pagegump, 445, 460, 2648, 2649, GF_PAGE_BTN, i);
			GFPage(pagegump, i);
			GFAddButton(pagegump, 445, 50, 2650, 2651, GF_PAGE_BTN, i - 1);
		endif
	endforeach

	input := GFSendGump(who, pagegump);
	input := input[0];

	if ((input > 200) &&
		input < 400)
		var serial := GetPageIndexByPageNum(input - 200, "gmpages");
		if (serial)
			var player := SystemFindObjectBySerial(Cint(serial));
			if (player == error)
				SendSysMessage(who, "Esse personagem foi deletado!");
			else
				MoveObjectToLocation(who, player.x, player.y, player.z, player.realm);
			endif
		else
			SendSysMessage(who, "Essa page foi deletada.");
		endif
		GMPageGump(who);
	elseif ((input > 400) &&
		input < 600)
		var serial := GetPageIndexByPageNum(input - 400, "gmpages");
		if (serial)
			var player := SystemFindObjectBySerial(Cint(serial));
			if (player == error)
				SendSysMessage(who, "Esse personagem foi deletado!");
			else
				var message_txt := RequestInput(who, who.backpack, "Digite sua mensagem: ");
				if (!message_txt)
					SendSysMessage(who, "Cancelado.");
					return 0;
				else
					SendSysMessage(who, "Resposta enviada para " + player.name + ".");
					SendSysMessage(player, "Staff: " + message_txt);
				endif
			endif
		else
			SendSysMessage(who, "Essa page foi deletada.");
		endif
		GMPageGump(who);
	elseif ((input > 600) &&
		input < 800)
		var serial := GetPageIndexByPageNum(input - 600, "gmpages");
		if (serial)
			var player := SystemFindObjectBySerial(Cint(serial));
			if (player == error)
				SendSysMessage(who, "Esse personagem foi deletado!");
			else
				MoveObjectToLocation(player, who.x, who.y, who.z, who.realm);
			endif
		else
			SendSysMessage(who, "Essa page foi deletada.");
		endif
		GMPageGump(who);
	elseif ((input > 800) &&
		input < 1000)
		var serial := GetPageIndexByPageNum(input - 800, "gmpages");
		if (serial)
			var player := SystemFindObjectBySerial(Cint(serial));
			var df2 := DFOpenDataFile("gmpages", DF_CREATE, DF_KEYTYPE_INTEGER);
			var elem2 := DFFindElement(df2, cint(serial), DF_CREATE);
			if (elem2 == error)
				SendSysMessage(who, "Essa page foi deletada.");
			else
				PageDescGump(who, elem2);
			endif
		else
			SendSysMessage(who, "Essa page foi deletada.");
		endif
		GMPageGump(who);
	elseif (input > 1000)
		var serial := GetPageIndexByPageNum(input - 1000, "gmpages");
		DeletePage(serial, "gmpages");
		GMPageGump(who);
	endif

endfunction

function GMBugGump(who)

	var pagegump := GFCreateGump();
	var input;

	GFDisposable(pagegump, 0);
	GFResizePic(pagegump, 0, 25, 3600, 450, 480);
	GFResizePic(pagegump, 15, 40, 0x13BE, 420, 450);
	GFTextMid(pagegump, 20, 50, 370, 1153, "Menu de Bugs");

	GFTextLine(pagegump, 20, 70, 1153, "Nome");
	GFTextLine(pagegump, 300, 70, 1153, "Descricao");
	GFTextLine(pagegump, 370, 70, 1153, "Deletar");

	var df := DFOpenDataFile("bugs", DF_CREATE, DF_KEYTYPE_INTEGER);
	var pages := df.Keys();
	var y := 90;
	var i := 1;
	var pagenum := 0;
	var elem;
	var player;
	var fontcolor := 1153;
	foreach page in pages
		pagenum := pagenum + 1;
		GFPage(pagegump, i);
		elem := df.FindElement(Cint(page));
		player := elem.GetProp("Title");
		GFTextLine(pagegump, 20, y, fontcolor, player);
		GFAddButton(pagegump, 320, y, 2117, 2118, 1, 800 + pagenum);
		GFAddButton(pagegump, 380, y, 2117, 2118, 1, 1000 + pagenum);
		y := y + 20;
		if (Cdbl(pagenum / 18) == i)
			i := i + 1;
			y := 90;
			GFAddButton(pagegump, 380, 460, 2648, 2649, GF_PAGE_BTN, i);
			GFPage(pagegump, i);
			GFAddButton(pagegump, 380, 50, 2650, 2651, GF_PAGE_BTN, i - 1);
		endif
	endforeach

	input := GFSendGump(who, pagegump);
	input := input[0];

	if ((input > 800) &&
		input < 1000)
		var serial := GetPageIndexByPageNum(input - 800, "bugs");
		if (serial)
			var df2 := DFOpenDataFile("bugs", DF_CREATE, DF_KEYTYPE_INTEGER);
			var elem2 := df2.FindElement(serial);
			if (elem2 == error)
				SendSysMessage(who, "Esse bug foi deletado.");
			else
				PageDescGump(who, elem2);
			endif
		else
			SendSysMessage(who, "Esse bug foi deletado.");
		endif
		GMBugGump(who);
	elseif (input > 1000)
		var serial := GetPageIndexByPageNum(input - 1000, "bugs");
		DeletePage(serial, "bugs");
		GMBugGump(who);
	endif

endfunction