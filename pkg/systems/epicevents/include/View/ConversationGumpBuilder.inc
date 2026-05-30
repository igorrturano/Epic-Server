use uo;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:epicGumps";
include ":epicevents:Controller/ConversationController";
include ":message:model/Message";
include ":text:Color";

// ============================================================================
// CONVERSATION GUMP CONSTANTS (500-699 range to avoid EventGumpBuilder collisions)
// ============================================================================

// Node editor buttons
const BTN_CONV_EDIT_TEXT := 500;
const BTN_CONV_ADD_ANSWER := 501;
const BTN_CONV_DONE := 502;
const BTN_CONV_TREE_VIEW := 503;
const BTN_CONV_GOTO_NODE := 504;

// Answer edit/delete base (supports up to 10 answers)
const BTN_CONV_ANS_EDIT_BASE := 510;   // 510-519
const BTN_CONV_ANS_DELETE_BASE := 520; // 520-529
const BTN_CONV_ANS_GOTO_BASE := 530;   // 530-539 (navigate to linked node)

// Answer editor buttons
const BTN_CONV_ANS_SAVE := 540;
const BTN_CONV_ANS_CANCEL := 541;
const BTN_CONV_ANS_SET_CONDITION := 542;
const BTN_CONV_ANS_ADD_ACTION := 543;
const BTN_CONV_ANS_CLEAR_ACTIONS := 544;

// Answer editor fields
const ID_CONV_ANS_TEXT := 560;

// Answer editor - next node radio buttons
const RADIO_CONV_NEXT_NODE_BASE := 570;  // 570-579 for existing nodes
const RADIO_CONV_NEXT_NEW_NODE := 580;
const RADIO_CONV_NEXT_END := 581;

// Tree view buttons
const BTN_CONV_TREE_NODE_BASE := 590;  // 590-599 for navigating to nodes

// NPC Setup buttons/fields
const BTN_NPC_USE_EXISTING := 700;
const BTN_NPC_CREATE_NEW := 701;
const BTN_NPC_COPY_GRAPHIC := 702;
const ID_NPC_GRAPHIC := 710;
const ID_NPC_NAME := 711;

// NPC Profile editor buttons/fields
const BTN_CONV_EDIT_PROFILE := 505;
const BTN_CONV_PROFILE_SAVE := 506;
const BTN_CONV_PROFILE_CANCEL := 507;
const BTN_CONV_CREATE_NODE := 508;
const ID_NPC_PROFILE_URL := 712;
const ID_NPC_PROFILE_DESC := 713;

// NPC Profile display constants
const NPC_PROFILE_IMG_WIDTH := 200;
const NPC_PROFILE_IMG_HEIGHT := 200;

// Player dialogue buttons
const BTN_PLAYER_END_CONVERSATION := 650;
const BTN_PLAYER_ANS_BASE := 600;  // 600+ for answer buttons

// Internal background
const CONV_INTERNAL_BG := 9350;

// ============================================================================
// BOOK-BASED TEXT EDITOR (supports paste + pre-fill)
// ============================================================================

function BigRequestGumpWithText(who, question, existing_text)
	// Create temporary book in player's backpack
	var book := CreateItemAtLocation(who.x, who.y, who.z, "book1", 1, who.realm);
	if (!book || book.errortext)
		Message("Erro ao criar livro temporario!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	book.SetTitle(question);
	book.SetAuthor(who.name);
	SetObjProperty(book, "#TempConvBook", 1);

	// Pre-fill book with existing text (split into lines)
	if (existing_text && existing_text != "")
		var lines := GFWordWrap(existing_text, 200);
		var i;
		for (i := 1; i <= lines.size(); i := i + 1)
			book.SetLine(i, lines[i]);
		endfor
	endif

	if (!MoveItemToContainer(book, who.backpack))
		Message("Erro ao mover livro para mochila!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		DestroyItem(book);
		return 0;
	endif

	// Open the book for editing
	SendOpenBook(who, book);

	// Show confirmation gump while player edits
	var confirmGump := GFCreateGump(200, 200);
	GFResizePic(confirmGump, 0, 0, GFCfgConst("Defaults", "BackGround"), 300, 150);
	GFResizePic(confirmGump, 10, 10, GFCfgConst("Defaults", "ForeGround"), 280, 30);
	GFTextMid(confirmGump, 10, 15, 280, 2100, question);

	GFResizePic(confirmGump, 10, 45, GFCfgConst("Backgrounds", "GOLDTRIM"), 280, 60);
	GFTextLine(confirmGump, 20, 55, 1153, "Edite o livro na sua mochila.");
	GFTextLine(confirmGump, 20, 75, 1153, "Voce pode colar texto livremente.");
	GFTextLine(confirmGump, 20, 90, 1153, "Clique em Concluido quando terminar.");

	GFResizePic(confirmGump, 10, 108, GFCfgConst("Defaults", "ForeGround"), 280, 30);
	GFAddButton(confirmGump, 100, 113, 4023, 4024, GF_CLOSE_BTN, 1);
	GFTextLine(confirmGump, 130, 113, 2100, "Concluido");

	var confirm := GFSendGump(who, confirmGump);

	if (!confirm || confirm[0] == 0)
		DestroyConvTempBook(book);
		return 0;
	endif

	// Extract text from the book
	var text := ExtractConvBookText(book);

	// Clean up
	DestroyConvTempBook(book);

	return text;
endfunction

function ExtractConvBookText(book)
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

				// If previous ends with word breaker or current starts with space, add space
				// Otherwise it's a broken word - join directly
				if (IsConvWordBreaker(lastChar) || firstChar == " ")
					text := text + " " + line;
				else
					text := text + line;
				endif
			else
				text := line;
			endif
		endif
	endforeach

	// Clean up double spaces
	text := StrReplace(text, "  ", " ");

	return text;
endfunction

function IsConvWordBreaker(ch)
	if (!ch || ch == "")
		return 1;
	endif
	if (ch == " " || ch == "." || ch == "," || ch == "!" || ch == "?" ||
	    ch == ";" || ch == ":" || ch == ")" || ch == "]" || ch == "}")
		return 1;
	endif
	return 0;
endfunction

function DestroyConvTempBook(book)
	if (!book || book.errortext)
		return;
	endif
	ReleaseItem(book);
	sleepms(50);
	var result := DestroyItem(book);
	if (!result)
		sleepms(200);
		ReleaseItem(book);
		DestroyItem(book);
	endif
endfunction

// ============================================================================
// PLAYER DIALOGUE GUMP
// ============================================================================

function BuildPlayerDialogueGump(npc, dialogue_text, visible_answers)
	var npc_url := GetObjProperty(npc, "url");
	var npc_desc := GetObjProperty(npc, "descricao");
	var has_profile := (npc_url && npc_url != "");

	// Wider gump when profile image is present
	var gump_width := has_profile ? 730 : 500;
	var gump := GFECreateGump("Conversa", gump_width, 500);
	GFDisposable(gump, 0);

	// Profile panel (left side) - only when NPC has a profile image
	var content_x := 15;
	if (has_profile)
		// Profile background
		GFResizePic(gump, 15, 70, CONV_INTERNAL_BG, 220, 415);

		// NPC image
		var img_html := "<img src=\"" + npc_url + "\" width=\"" + NPC_PROFILE_IMG_WIDTH + "\" height=\"" + NPC_PROFILE_IMG_HEIGHT + "\" x=\"0\" y=\"0\">";
		GFHTMLArea(gump, 25, 80, NPC_PROFILE_IMG_WIDTH, NPC_PROFILE_IMG_HEIGHT, img_html, 0, 0);

		// NPC description below image
		if (npc_desc && npc_desc != "")
			GFResizePic(gump, 25, 290, CONV_INTERNAL_BG, 200, 185);
			GFHTMLArea(gump, 30, 295, 190, 175, npc_desc, 0, 1);
		endif

		content_x := 245;
	endif

	// Main dialogue area (right side, or centered if no profile)
	var dialog_width := has_profile ? 470 : 470;
	GFResizePic(gump, content_x, 70, CONV_INTERNAL_BG, dialog_width, 415);

	// NPC name header
	var y := 80;
	GFTextMid(gump, content_x, y, dialog_width, GUMP_COLOR::SECTION_HEADER, npc.name);
	y := y + 25;

	// Divider
	GFResizePic(gump, content_x + 15, y, 3000, dialog_width - 30, 2);
	y := y + 15;

	// Dialogue text (scrollable HTML area)
	GFResizePic(gump, content_x + 10, y, CONV_INTERNAL_BG, dialog_width - 20, 180);
	GFHTMLArea(gump, content_x + 15, y + 5, dialog_width - 30, 170, dialogue_text, 0, 1);
	y := y + 195;

	// Divider
	GFResizePic(gump, content_x + 15, y, 3000, dialog_width - 30, 2);
	y := y + 15;

	// Answer buttons
	var i := 1;
	foreach answer in (visible_answers)
		if (i > 8)
			break;
		endif
		GFAddButton(gump, content_x + 15, y, 0xFAE, 0x1764, GF_CLOSE_BTN, BTN_PLAYER_ANS_BASE + i);
		GFTextLine(gump, content_x + 45, y, GUMP_COLOR::LABEL, answer.text);
		y := y + 22;
		i := i + 1;
	endforeach

	// End conversation button
	y := 455;
	var end_x := content_x + dialog_width - 120;
	GFAddButton(gump, end_x, y, 0xFB1, 0xFB2, GF_CLOSE_BTN, BTN_PLAYER_END_CONVERSATION);
	GFTextLine(gump, end_x - 70, y + 3, GUMP_COLOR::HELP_TEXT, "Encerrar");

	return gump;
endfunction

function ParsePlayerDialogueInput(input, visible_answers)
	if (!input || input[BTN_PLAYER_END_CONVERSATION])
		return 0;
	endif

	var i := 1;
	foreach answer in (visible_answers)
		if (input[BTN_PLAYER_ANS_BASE + i])
			return answer;
		endif
		i := i + 1;
	endforeach

	return 0;
endfunction

// ============================================================================
// GM - NODE EDITOR GUMP
// ============================================================================

function BuildNodeEditorGump(conversation_id, node, npc := 0)
	var gump := GFECreateGump("Editor de Conversa", 600, 650);
	GFDisposable(gump, 0);

	// Internal background
	GFResizePic(gump, 15, 70, CONV_INTERNAL_BG, 570, 565);

	var x := 30;
	var y := 80;

	// Header with node ID
	GFTextMid(gump, 0, y, 600, GUMP_COLOR::SECTION_HEADER, $"DIALOGO: {node.id}");
	y := y + 25;
	GFResizePic(gump, 30, y, 3000, 540, 2);
	y := y + 15;

	// Dialogue text section
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, "TEXTO DO DIALOGO");
	y := y + 20;

	// Display current text in scrollable area
	var display_text := node.text ? node.text : "(vazio)";
	GFResizePic(gump, x, y, CONV_INTERNAL_BG, 540, 100);
	GFHTMLArea(gump, x + 5, y + 5, 530, 90, display_text, 0, 1);
	y := y + 105;

	// Edit text button
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_EDIT_TEXT);
	GFTooltipText(gump, "Abrir editor de texto do dialogo");
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::LABEL, "Editar Texto");

	// Edit NPC profile button (only if NPC is available)
	if (npc)
		GFAddButton(gump, x + 250, y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_EDIT_PROFILE);
		GFTooltipText(gump, "Editar imagem e descricao do NPC");
		var has_npc_profile := (GetObjProperty(npc, "url") && GetObjProperty(npc, "url") != "");
		var profile_label := has_npc_profile ? "Editar Perfil (Configurado)" : "Editar Perfil";
		var profile_color := has_npc_profile ? GUMP_COLOR::HIGHLIGHT : GUMP_COLOR::LABEL;
		GFTextLine(gump, x + 290, y + 2, profile_color, profile_label);
	endif
	y := y + 35;

	// Divider
	GFResizePic(gump, 30, y, 3000, 540, 2);
	y := y + 15;

	// Answers section
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, $"RESPOSTAS ({node.answers.size()})");
	y := y + 25;

	var ans_index := 1;
	foreach answer in (node.answers)
		if (ans_index > 10)
			break;
		endif

		// Answer row with background
		GFResizePic(gump, x, y, 0x2486, 540, 55);

		// Edit button
		GFAddButton(gump, x + 5, y + 5, 0x659, 0x65A, GF_CLOSE_BTN, BTN_CONV_ANS_EDIT_BASE + ans_index);
		GFTooltipText(gump, "Editar esta resposta");

		// Delete button
		GFAddButton(gump, x + 5, y + 30, 30008, 30009, GF_CLOSE_BTN, BTN_CONV_ANS_DELETE_BASE + ans_index);
		GFTooltipText(gump, "Remover esta resposta");

		// Answer text
		var ans_text := answer.text ? answer.text : "(sem texto)";
		GFTextLine(gump, x + 45, y + 3, GUMP_COLOR::LABEL, $"{ans_index}. {ans_text}");

		// Next node info
		var next_label;
		if (!answer.next_node || answer.next_node == "")
			next_label := "[FIM]";
		else
			var next_summary := GetNodeSummary(conversation_id, answer.next_node);
			if (Len(next_summary) > 15)
				next_summary := next_summary[1, 15] + "...";
			endif
			next_label := $"[{next_summary}]";
		endif
		GFTextLine(gump, x + 400, y + 3, GUMP_COLOR::HELP_TEXT, $"> {next_label}");

		// Navigate to linked node button (if has next node)
		if (answer.next_node && answer.next_node != "")
			GFAddButton(gump, x + 520, y + 3, 2117, 2118, GF_CLOSE_BTN, BTN_CONV_ANS_GOTO_BASE + ans_index);
			GFTooltipText(gump, "Ir para o dialogo desta resposta");
		endif

		// Condition and action info
		var info_parts := array{};
		if (answer.conditions)
			info_parts.append("Cond: Sim");
		endif
		if (answer.actions && answer.actions.size() > 0)
			info_parts.append($"Acoes: {answer.actions.size()}");
		endif
		var info_text := "";
		foreach part in (info_parts)
			if (info_text != "")
				info_text := info_text + " | ";
			endif
			info_text := info_text + part;
		endforeach
		if (info_text != "")
			GFTextLine(gump, x + 45, y + 25, GUMP_COLOR::HELP_TEXT, info_text);
		endif

		y := y + 60;
		ans_index := ans_index + 1;
	endforeach

	// Add answer button
	GFAddButton(gump, x, y + 5, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_ADD_ANSWER);
	GFTooltipText(gump, "Criar uma nova opcao de resposta");
	GFTextLine(gump, x + 40, y + 7, GUMP_COLOR::HIGHLIGHT, "+ Adicionar Resposta");

	// Create node button (supports "nodes first" workflow)
	GFAddButton(gump, x + 280, y + 5, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_CREATE_NODE);
	GFTooltipText(gump, "Criar um novo dialogo sem precisar de resposta agora");
	GFTextLine(gump, x + 320, y + 7, GUMP_COLOR::HIGHLIGHT, "+ Criar Dialogo");
	y := y + 35;

	// Bottom buttons
	var bottom_y := 600;
	GFAddButton(gump, 30, bottom_y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_GOTO_NODE);
	GFTooltipText(gump, "Navegar para outro dialogo");
	GFTextLine(gump, 70, bottom_y + 2, GUMP_COLOR::LABEL, "Ir para Dialogo...");

	GFAddButton(gump, 230, bottom_y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_TREE_VIEW);
	GFTooltipText(gump, "Ver todos os dialogos da conversa");
	GFTextLine(gump, 270, bottom_y + 2, GUMP_COLOR::LABEL, "Visao Arvore");

	GFAddButton(gump, 430, bottom_y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_DONE);
	GFTooltipText(gump, "Fechar o editor de conversa");
	GFTextLine(gump, 470, bottom_y + 2, GUMP_COLOR::HIGHLIGHT, "Concluir");

	return gump;
endfunction

// ============================================================================
// GM - ANSWER EDITOR GUMP
// ============================================================================

function BuildAnswerEditorGump(conversation_id, answer, node_list)
	var gump := GFECreateGump("Editar Resposta", 550, 600);
	GFDisposable(gump, 0);

	GFResizePic(gump, 15, 70, CONV_INTERNAL_BG, 520, 515);

	var x := 30;
	var y := 85;

	// Header
	GFTextMid(gump, 0, y, 550, GUMP_COLOR::SECTION_HEADER, "EDITAR RESPOSTA");
	y := y + 25;
	GFResizePic(gump, 30, y, 3000, 490, 2);
	y := y + 15;

	// Answer text
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, "TEXTO DA RESPOSTA");
	y := y + 20;
	GFResizePic(gump, x, y, CONV_INTERNAL_BG, 490, 25);
	var ans_text := answer ? (answer.text ? answer.text : "") : "";
	GFTextEntry(gump, x + 5, y + 3, 480, 20, GUMP_COLOR::TEXT_ENTRY, ans_text, ID_CONV_ANS_TEXT);
	y := y + 35;

	// Divider
	GFResizePic(gump, 30, y, 3000, 490, 2);
	y := y + 15;

	// Next node selection
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, "PARA ONDE LEVA ESTA RESPOSTA?");
	y := y + 22;

	GFSetRadioGroup(gump, 1);

	// End conversation option
	var is_end := (!answer || !answer.next_node || answer.next_node == "");
	GFRadioButton(gump, x, y, 0x25F8, 0x25FB, is_end, RADIO_CONV_NEXT_END);
	GFTextLine(gump, x + 35, y + 3, GUMP_COLOR::LABEL, "Encerrar conversa");
	y := y + 22;

	// Create new node option
	GFRadioButton(gump, x, y, 0x25F8, 0x25FB, 0, RADIO_CONV_NEXT_NEW_NODE);
	GFTextLine(gump, x + 35, y + 3, GUMP_COLOR::LABEL, "Criar novo dialogo");
	y := y + 22;

	// Existing nodes
	var radio_index := 0;
	var node_number := 1;
	foreach node_id in (node_list)
		if (radio_index >= 8)
			break;
		endif

		var is_selected := (answer && answer.next_node == node_id) ? 1 : 0;
		var summary := GetNodeSummary(conversation_id, node_id);

		GFRadioButton(gump, x, y, 0x25F8, 0x25FB, is_selected, RADIO_CONV_NEXT_NODE_BASE + radio_index);
		GFTextLine(gump, x + 35, y + 3, GUMP_COLOR::LABEL, $"Dialogo {node_number}: {summary}");
		y := y + 22;

		radio_index := radio_index + 1;
		node_number := node_number + 1;
	endforeach

	y := y + 10;

	// Divider
	GFResizePic(gump, 30, y, 3000, 490, 2);
	y := y + 15;

	// Condition section
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, "CONDICAO");
	var cond_label := "(Nenhuma - todos veem)";
	if (answer && answer.conditions)
		cond_label := "(Configurada)";
	endif
	GFTextLine(gump, x + 120, y, GUMP_COLOR::HELP_TEXT, cond_label);
	y := y + 22;
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_ANS_SET_CONDITION);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::LABEL, "Configurar Condicao");
	y := y + 35;

	// Actions section
	var action_count := 0;
	if (answer && answer.actions)
		action_count := answer.actions.size();
	endif
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, $"ACOES ({action_count})");
	y := y + 22;
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_ANS_ADD_ACTION);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::LABEL, "Adicionar Acao");

	if (action_count > 0)
		GFAddButton(gump, x + 250, y, 2708, 2709, GF_CLOSE_BTN, BTN_CONV_ANS_CLEAR_ACTIONS);
		GFTextLine(gump, x + 280, y + 2, 38, "Limpar Acoes");
	endif
	y := y + 35;

	// Bottom buttons
	var bottom_y := 555;
	GFAddButton(gump, 150, bottom_y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_ANS_SAVE);
	GFTextLine(gump, 190, bottom_y + 2, GUMP_COLOR::HIGHLIGHT, "Salvar");

	GFAddButton(gump, 320, bottom_y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_ANS_CANCEL);
	GFTextLine(gump, 360, bottom_y + 2, GUMP_COLOR::LABEL, "Cancelar");

	return gump;
endfunction

function ParseAnswerEditorInput(input, node_list)
	if (!input)
		return 0;
	endif

	if (input[BTN_CONV_ANS_CANCEL])
		return struct{ action := "cancel" };
	endif

	if (input[BTN_CONV_ANS_SET_CONDITION])
		return struct{ action := "set_condition" };
	endif

	if (input[BTN_CONV_ANS_ADD_ACTION])
		return struct{ action := "add_action" };
	endif

	if (input[BTN_CONV_ANS_CLEAR_ACTIONS])
		return struct{ action := "clear_actions" };
	endif

	if (input[BTN_CONV_ANS_SAVE])
		var text := Trim(GFExtractData(input, ID_CONV_ANS_TEXT));

		// Determine next node
		var next_node := "";

		if (input[RADIO_CONV_NEXT_END])
			next_node := "";
		elseif (input[RADIO_CONV_NEXT_NEW_NODE])
			next_node := "__NEW__";
		else
			var radio_index := 0;
			foreach node_id in (node_list)
				if (input[RADIO_CONV_NEXT_NODE_BASE + radio_index])
					next_node := node_id;
					break;
				endif
				radio_index := radio_index + 1;
			endforeach
		endif

		return struct{
			action := "save",
			text := text,
			next_node := next_node
		};
	endif

	return 0;
endfunction

// ============================================================================
// GM - TREE OVERVIEW GUMP
// ============================================================================

function BuildTreeOverviewGump(conversation_id)
	var node_list := GetNodeList(conversation_id);
	var root_id := GetRootNodeId(conversation_id);

	var num_nodes := node_list.size();
	var height := 150 + (num_nodes * 80);
	if (height > 650)
		height := 650;
	endif

	var gump := GFECreateGump("Visao da Arvore", 600, height);
	GFDisposable(gump, 0);

	GFResizePic(gump, 15, 70, CONV_INTERNAL_BG, 570, height - 85);

	var x := 30;
	var y := 85;

	GFTextMid(gump, 0, y, 600, GUMP_COLOR::SECTION_HEADER, $"ARVORE DE CONVERSA ({num_nodes} dialogos)");
	y := y + 25;
	GFResizePic(gump, 30, y, 3000, 540, 2);
	y := y + 15;

	var node_index := 0;
	var node_number := 1;
	foreach node_id in (node_list)
		if (node_index >= 10)
			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, $"... e mais {num_nodes - 10} dialogo(s)");
			break;
		endif

		var node := LoadConversationNode(conversation_id, node_id);
		if (!node)
			node_index := node_index + 1;
			node_number := node_number + 1;
			continue;
		endif

		// Node entry
		var is_root := (node_id == root_id);
		var label_color := is_root ? GUMP_COLOR::HIGHLIGHT : GUMP_COLOR::LABEL;
		var root_tag := is_root ? " (INICIO)" : "";

		// Navigate button
		GFAddButton(gump, x, y, 2117, 2118, GF_CLOSE_BTN, BTN_CONV_TREE_NODE_BASE + node_index);

		// Node info
		var summary := node.text;
		if (Len(summary) > 40)
			summary := summary[1, 40] + "...";
		endif
		GFTextLine(gump, x + 25, y, label_color, $"Dialogo {node_number}{root_tag}");
		y := y + 18;
		GFTextLine(gump, x + 40, y, GUMP_COLOR::HELP_TEXT, $"\"{summary}\"");
		y := y + 18;

		// Show answer connections
		foreach answer in (node.answers)
			var next_label := (!answer.next_node || answer.next_node == "") ? "[FIM]" : "[Dialogo]";
			var ans_preview := answer.text;
			if (Len(ans_preview) > 25)
				ans_preview := ans_preview[1, 25] + "...";
			endif
			GFTextLine(gump, x + 55, y, GUMP_COLOR::HELP_TEXT, $"- \"{ans_preview}\" > {next_label}");
			y := y + 16;
		endforeach

		y := y + 10;
		node_index := node_index + 1;
		node_number := node_number + 1;
	endforeach

	// Back button
	var bottom_y := height - 50;
	GFAddButton(gump, 250, bottom_y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_DONE);
	GFTextLine(gump, 290, bottom_y + 2, GUMP_COLOR::LABEL, "Voltar ao Editor");

	return gump;
endfunction

// ============================================================================
// GM - NODE SELECTION GUMP (for "Ir para No...")
// ============================================================================

function BuildNodeSelectionGump(conversation_id)
	var node_list := GetNodeList(conversation_id);
	var height := 150 + (node_list.size() * 25);
	if (height > 500)
		height := 500;
	endif

	var gump := GFECreateGump("Selecionar No", 450, height);
	GFDisposable(gump, 0);
	GFResizePic(gump, 15, 70, CONV_INTERNAL_BG, 420, height - 85);

	var x := 30;
	var y := 85;

	GFTextMid(gump, 0, y, 450, GUMP_COLOR::SECTION_HEADER, "SELECIONE UM DIALOGO");
	y := y + 25;
	GFResizePic(gump, 30, y, 3000, 390, 2);
	y := y + 15;

	var node_index := 0;
	var node_number := 1;
	foreach node_id in (node_list)
		if (node_index >= 10)
			break;
		endif

		var summary := GetNodeSummary(conversation_id, node_id);
		GFAddButton(gump, x, y, 2117, 2118, GF_CLOSE_BTN, BTN_CONV_TREE_NODE_BASE + node_index);
		GFTextLine(gump, x + 25, y, GUMP_COLOR::LABEL, $"Dialogo {node_number}: {summary}");
		y := y + 22;

		node_index := node_index + 1;
		node_number := node_number + 1;
	endforeach

	return gump;
endfunction

// ============================================================================
// GM - NPC SETUP GUMP
// ============================================================================

function BuildNpcSetupGump(working_params := 0)
	var gump := GFECreateGump("Epic Events - Configurar NPC", 500, 400);
	GFDisposable(gump, 0);

	GFResizePic(gump, 15, 70, CONV_INTERNAL_BG, 470, 315);

	var x := 30;
	var y := 85;

	GFTextMid(gump, 0, y, 500, GUMP_COLOR::SECTION_HEADER, "CONFIGURAR NPC DO EVENTO");
	y := y + 25;
	GFResizePic(gump, 30, y, 3000, 440, 2);
	y := y + 20;

	// Option 1: Use existing NPC
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_NPC_USE_EXISTING);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Usar NPC Existente");
	GFTextLine(gump, x + 40, y + 20, GUMP_COLOR::HELP_TEXT, "Clique em um NPC ja existente no mundo");
	y := y + 55;

	// Divider
	GFResizePic(gump, 30, y, 3000, 440, 2);
	y := y + 15;

	// Option 2: Create new NPC
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, "CRIAR NOVO NPC");
	y := y + 25;

	// Graphic
	var graphic_default := "0x0190";
	if (working_params && working_params.graphic)
		graphic_default := working_params.graphic;
	endif
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Graphic (hex):");
	GFResizePic(gump, x + 140, y, CONV_INTERNAL_BG, 120, 25);
	GFTextEntry(gump, x + 145, y + 3, 110, 20, GUMP_COLOR::TEXT_ENTRY, graphic_default, ID_NPC_GRAPHIC);

	// Copy graphic button
	GFAddButton(gump, x + 275, y, 2152, 2153, GF_CLOSE_BTN, BTN_NPC_COPY_GRAPHIC);
	GFTextLine(gump, x + 315, y + 2, GUMP_COLOR::HELP_TEXT, "Copiar de...");
	y := y + 30;

	// Name
	var name_default := "NPC";
	if (working_params && working_params.npc_name)
		name_default := working_params.npc_name;
	endif
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Nome:");
	GFResizePic(gump, x + 140, y, CONV_INTERNAL_BG, 300, 25);
	GFTextEntry(gump, x + 145, y + 3, 290, 20, GUMP_COLOR::TEXT_ENTRY, name_default, ID_NPC_NAME);
	y := y + 40;

	// Create button
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_NPC_CREATE_NEW);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Criar NPC e Continuar");

	return gump;
endfunction

function ParseNpcSetupInput(input)
	if (!input)
		return 0;
	endif

	if (input[BTN_NPC_USE_EXISTING])
		return struct{ action := "use_existing" };
	endif

	if (input[BTN_NPC_COPY_GRAPHIC])
		var graphic_str := Trim(GFExtractData(input, ID_NPC_GRAPHIC));
		var name_str := Trim(GFExtractData(input, ID_NPC_NAME));
		return struct{
			action := "copy_graphic",
			graphic := graphic_str,
			npc_name := name_str
		};
	endif

	if (input[BTN_NPC_CREATE_NEW])
		var graphic_str := Trim(GFExtractData(input, ID_NPC_GRAPHIC));
		var name_str := Trim(GFExtractData(input, ID_NPC_NAME));
		return struct{
			action := "create_new",
			graphic := graphic_str,
			npc_name := name_str
		};
	endif

	return 0;
endfunction

// ============================================================================
// GM - NPC PROFILE EDITOR GUMP
// ============================================================================

function BuildNpcProfileEditorGump(npc)
	var gump := GFECreateGump("Editar Perfil do NPC", 500, 400);
	GFDisposable(gump, 0);

	GFResizePic(gump, 15, 70, CONV_INTERNAL_BG, 470, 315);

	var x := 30;
	var y := 85;

	GFTextMid(gump, 0, y, 500, GUMP_COLOR::SECTION_HEADER, $"PERFIL: {npc.name}");
	y := y + 25;
	GFResizePic(gump, 30, y, 3000, 440, 2);
	y := y + 15;

	// URL da Imagem
	var current_url := GetObjProperty(npc, "url");
	if (!current_url)
		current_url := "";
	endif
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, "URL DA IMAGEM (210x330)");
	y := y + 20;
	GFResizePic(gump, x, y, CONV_INTERNAL_BG, 440, 25);
	GFTextEntry(gump, x + 5, y + 3, 430, 20, GUMP_COLOR::TEXT_ENTRY, current_url, ID_NPC_PROFILE_URL);
	y := y + 35;

	// Descricao
	var current_desc := GetObjProperty(npc, "descricao");
	if (!current_desc)
		current_desc := "";
	endif
	GFTextLine(gump, x, y, GUMP_COLOR::HIGHLIGHT, "DESCRICAO");
	y := y + 20;
	GFResizePic(gump, x, y, CONV_INTERNAL_BG, 440, 120);
	GFTextEntry(gump, x + 5, y + 3, 430, 115, GUMP_COLOR::TEXT_ENTRY, current_desc, ID_NPC_PROFILE_DESC);
	y := y + 135;

	// Preview hint
	if (current_url != "")
		GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Imagem atual configurada. Salve para atualizar.");
	else
		GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Nenhuma imagem configurada.");
	endif

	// Buttons
	var bottom_y := 350;
	GFAddButton(gump, 150, bottom_y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_PROFILE_SAVE);
	GFTextLine(gump, 190, bottom_y + 2, GUMP_COLOR::HIGHLIGHT, "Salvar");

	GFAddButton(gump, 320, bottom_y, 2152, 2153, GF_CLOSE_BTN, BTN_CONV_PROFILE_CANCEL);
	GFTextLine(gump, 360, bottom_y + 2, GUMP_COLOR::LABEL, "Cancelar");

	return gump;
endfunction

function ParseNpcProfileEditorInput(input)
	if (!input || input[BTN_CONV_PROFILE_CANCEL])
		return 0;
	endif

	if (input[BTN_CONV_PROFILE_SAVE])
		var url := Trim(GFExtractData(input, ID_NPC_PROFILE_URL));
		var desc := Trim(GFExtractData(input, ID_NPC_PROFILE_DESC));
		return struct{
			action := "save",
			url := url,
			descricao := desc
		};
	endif

	return 0;
endfunction
