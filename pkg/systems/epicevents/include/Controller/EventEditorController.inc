use uo;

include ":gumps:gumps";
include ":epicevents:View/EventGumpBuilder";
include ":epicevents:View/ConversationGumpBuilder";
include ":epicevents:Controller/ConversationController";
include ":epicevents:Model/ActionConfig";
include ":epicevents:Model/EventConfig";
include ":message:model/Message";
include ":merchants:item_template";

// ============================================================================
// CONDITION EDITOR
// ============================================================================

// Handle condition editing with support for item targeting and character dropdowns
// Returns: condition struct, 0 (no condition), or -1 (cancelled)
function HandleConditionEdit(who, existing_condition)
	var working_condition := existing_condition;

	// Ensure working_condition has character params initialized
	if (working_condition && working_condition.params)
		if (!working_condition.params.character_race)
			working_condition.params.+character_race := "";
		endif
		if (!working_condition.params.character_culture)
			working_condition.params.+character_culture := "";
		endif
		if (!working_condition.params.character_religion)
			working_condition.params.+character_religion := "";
		endif
	endif

	// Create dropdown manager for race/culture/religion
	var gerenciador := GerenciadorDropDown(
		COND_DD_MAX_OPCOES,
		COND_DD_ID_DROPDOWN,
		COND_DD_ID_OPCOES,
		COND_DD_MAX_DROPDOWN
	);

	while (1)
		gerenciador.FechaTodosDropdowns(who);

		// Setup dropdown options
		SetupCharacterDropdowns(gerenciador);

		var cond_gump := BuildConditionGump(working_condition, gerenciador);
		var cond_input := GFSendGump(who, cond_gump);

		if (!cond_input)
			gerenciador.FechaTodosDropdowns(who);
			return -1;  // Cancelled
		endif

		// Handle dropdown button clicks (open dropdown overlay)
		if (gerenciador.IsIdDentroDropDowns(cond_input[0]))
			// Preserve form data before opening dropdown
			working_condition := PreserveConditionFormData(cond_input, working_condition);

			var posicao := cond_input[0] - gerenciador.PegaAdicionalIdDropDown();
			var dropdown := gerenciador.BuscaDropDownPorPosicao(posicao);
			gerenciador.FechaTodosDropdowns(who);
			dropdown.SendGump(who, GetPid());
			continue;

		// Handle dropdown option selection
		elseif (gerenciador.IsIdDentroOpcoes(cond_input[0]))
			var dropdown := gerenciador.DefineValorEscolhido(cond_input[0]);
			var valor := dropdown.PegaValorEscolhido();
			var pos := dropdown.PegaPosicaoBottao();

			// Ensure working_condition exists with params
			if (!working_condition || !working_condition.params)
				working_condition := struct{ type := "CONDITION", params := struct{} };
				working_condition.params.+character_race := "";
				working_condition.params.+character_culture := "";
				working_condition.params.+character_religion := "";
			endif

			// Update the selected character field
			if (pos == 1)
				working_condition.params.character_race := valor;
			elseif (pos == 2)
				working_condition.params.character_culture := valor;
			elseif (pos == 3)
				working_condition.params.character_religion := valor;
			endif
			continue;
		endif

		var result := ParseConditionInput(cond_input);

		// Check if user clicked target button
		if (result && result.action == "target_item")
			// Preserve current form data
			working_condition := PreserveConditionFormData(cond_input, working_condition);

			Message("Clique no item desejado...", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			var targ := Target(who);

			if (targ && targ.objtype)
				// Update the working condition with targeted item
				if (!working_condition)
					working_condition := struct{ type := "CONDITION", params := struct{} };
					working_condition.params.+has_item_objtype := 0;
					working_condition.params.+has_item_amount := 1;
				endif
				if (!working_condition.params)
					working_condition.params := struct{};
					working_condition.params.+has_item_objtype := 0;
					working_condition.params.+has_item_amount := 1;
				endif

				working_condition.params.has_item_objtype := targ.objtype;
				if (!working_condition.params.has_item_amount)
					working_condition.params.has_item_amount := 1;
				endif

				Message($"Item selecionado: {targ.objtype} ({Hex(targ.objtype)})", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
			else
				Message("Target cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			endif

			continue;  // Show gump again
		endif

		// Normal save/skip result - merge character dropdown values
		if (result && result.params && working_condition && working_condition.params)
			result.params.character_race := working_condition.params.character_race ? working_condition.params.character_race : "";
			result.params.character_culture := working_condition.params.character_culture ? working_condition.params.character_culture : "";
			result.params.character_religion := working_condition.params.character_religion ? working_condition.params.character_religion : "";
		// If ParseConditionInput returned 0 (no item/nature), but character dropdowns have values
		elseif (!result && working_condition && working_condition.params)
			var has_char := (working_condition.params.character_race && working_condition.params.character_race != "") ||
			                (working_condition.params.character_culture && working_condition.params.character_culture != "") ||
			                (working_condition.params.character_religion && working_condition.params.character_religion != "");
			if (has_char && cond_input[CHK_COND_CHARACTER])
				result := struct{ type := "CONDITION", params := struct{} };
				result.params.+character_race := working_condition.params.character_race ? working_condition.params.character_race : "";
				result.params.+character_culture := working_condition.params.character_culture ? working_condition.params.character_culture : "";
				result.params.+character_religion := working_condition.params.character_religion ? working_condition.params.character_religion : "";
				result.params.+has_item_objtype := 0;
				result.params.+has_item_amount := 0;
				result.params.+should_consume_item := 0;
				result.params.+moon_phases := array{};
				result.params.+day_night := "";
				result.params.+seasons := array{};
				result.params.+time_min := 0;
				result.params.+time_max := 0;
			endif
		endif

		gerenciador.FechaTodosDropdowns(who);
		return result;
	endwhile

	return -1;
endfunction

// Preserve condition form data when targeting
function PreserveConditionFormData(input, existing_condition)
	var params := struct{};

	// Preserve HAS_ITEM fields
	var objtype_str := Trim(GFExtractData(input, ID_COND_ITEM_OBJTYPE));
	var amount_str := Trim(GFExtractData(input, ID_COND_ITEM_AMOUNT));

	if (objtype_str && objtype_str != "")
		params.+has_item_objtype := CInt(objtype_str);
	elseif (existing_condition && existing_condition.params && existing_condition.params.has_item_objtype)
		params.+has_item_objtype := existing_condition.params.has_item_objtype;
	else
		params.+has_item_objtype := 0;
	endif

	params.+has_item_amount := (amount_str && amount_str != "") ? CInt(amount_str) : 1;
	params.+should_consume_item := input[CHCK_COND_ITEM_CONSUME] ? 1 : 0;

	// Preserve NATURE fields
	params.+moon_phases := array{};
	if (input[CHK_MOON_KALENDAE])
		params.moon_phases.append("Kalendae");
	endif
	if (input[CHK_MOON_NONAE])
		params.moon_phases.append("Nonae");
	endif
	if (input[CHK_MOON_IDUS])
		params.moon_phases.append("Idus");
	endif
	if (input[CHK_MOON_DECRESCENS])
		params.moon_phases.append("Decrescens");
	endif

	if (input[RADIO_DAYNIGHT_DIA])
		params.+day_night := "dia";
	elseif (input[RADIO_DAYNIGHT_NOITE])
		params.+day_night := "noite";
	else
		params.+day_night := "";
	endif

	params.+seasons := array{};
	if (input[CHK_SEASON_SUMMER])
		params.seasons.append(1);
	endif
	if (input[CHK_SEASON_AUTUMN])
		params.seasons.append(2);
	endif
	if (input[CHK_SEASON_WINTER])
		params.seasons.append(4);
	endif
	if (input[CHK_SEASON_SPRING])
		params.seasons.append(5);
	endif

	var time_min_str := Trim(GFExtractData(input, ID_COND_TIME_MIN));
	var time_max_str := Trim(GFExtractData(input, ID_COND_TIME_MAX));
	params.+time_min := (time_min_str != "") ? CInt(time_min_str) : 0;
	params.+time_max := (time_max_str != "") ? CInt(time_max_str) : 0;

	// Preserve CHARACTER fields (from dropdown selections stored in working_condition)
	params.+character_race := "";
	params.+character_culture := "";
	params.+character_religion := "";

	if (existing_condition && existing_condition.params)
		params.character_race := existing_condition.params.character_race ? existing_condition.params.character_race : "";
		params.character_culture := existing_condition.params.character_culture ? existing_condition.params.character_culture : "";
		params.character_religion := existing_condition.params.character_religion ? existing_condition.params.character_religion : "";
	endif

	return struct{ type := "CONDITION", params := params };
endfunction

// ============================================================================
// ACTION EDITOR
// ============================================================================

function HandleActionEdit(who, action_type, existing_action)
	if (action_type == "CONVERSATION")
		return HandleConversationEdit(who, existing_action);
	endif

	if (action_type == "SPAWN")
		// Reconstruct EventConfig from existing action params for editing
		var existing_spawn_config := 0;
		if (existing_action && existing_action.params)
			existing_spawn_config := EventConfig();
			existing_spawn_config.FromStruct(existing_action.params);
		endif

		var spawn_gump := BuildEventConfigGump(existing_spawn_config);
		var spawn_input := GFSendGump(who, spawn_gump);

		if (!spawn_input || !spawn_input[BTN_OK])
			return 0;
		endif

		var spawn_config := ParseEventConfigInput(spawn_input);
		if (!spawn_config)
			return 0;
		endif

		var spawn_action := ActionConfig("SPAWN");
		spawn_action.params := spawn_config.ToStruct();
		return spawn_action;
	endif

	var editing := 1;
	var working_action := existing_action;

	while (editing)
		var edit_gump := BuildActionEditGump(action_type, working_action, who);
		var edit_input := GFSendGump(who, edit_gump);

		if (edit_input[BTN_TARGET_ITEM])
			PreserveFormDataForTargeting(working_action, edit_input, action_type);

			Message("Target o item desejado...", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			var targ := Target(who);

			if (!targ)
				Message("Target cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				continue;
			endif

			// Preserve existing values or default
			var existing_amount := 1;
			var existing_consume := 0;
			if (working_action.condition && working_action.condition.params)
				if (working_action.condition.params.amount)
					existing_amount := working_action.condition.params.amount;
				endif
				if (working_action.condition.params.should_consume_item)
					existing_consume := working_action.condition.params.should_consume_item;
				endif
			endif

			// Replace condition with new struct to ensure clean update
			working_action.condition := struct{
				type := "PLAYER_HAS_ITEM",
				params := struct{ objtype := targ.objtype, amount := existing_amount, should_consume_item := existing_consume }
			};

			Message($"Item selecionado: {targ.objtype} (0x{Hex(targ.objtype)})", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
			continue;
		endif

		if (edit_input[BTN_TARGET_COORDS])
			PreserveFormDataForTargeting(working_action, edit_input, action_type);

			Message("Selecione a localizacao de destino...", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			var targ := TargetCoordinates(who);

			if (!targ)
				Message("Target cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				continue;
			endif

			// Replace params with new struct to ensure clean update
			working_action.params := struct{ x := targ.x, y := targ.y, z := targ.z, realm := who.realm };

			Message($"Localizacao selecionada: ({targ.x}, {targ.y}, {targ.z}) [{who.realm}]", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
			continue;
		endif

		if (edit_input[BTN_GIVE_ITEM_TARGET])
			PreserveFormDataForTargeting(working_action, edit_input, action_type);

			Message("Target o item modelo (sera clonado para o jogador)...", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			var targ := Target(who);

			if (!targ)
				Message("Target cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				continue;
			endif

			var template := CreateItemTemplate(targ);
			if (template == error)
				Message("Erro ao gerar modelo do item.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				continue;
			endif

			var keep_amount := 0;
			if (working_action.params && working_action.params.amount)
				keep_amount := working_action.params.amount;
			endif

			working_action.params := struct{ template := template, amount := keep_amount };

			var label := targ.desc ? CStr(targ.desc) : (targ.name ? CStr(targ.name) : $"0x{Hex(targ.objtype)}");
			Message($"Modelo capturado: {label}", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
			continue;
		endif

		var parsed_action := ParseActionEditInput(edit_input, action_type, working_action);

		if (!parsed_action)
			return 0;
		endif

		return parsed_action;
	endwhile

	return 0;
endfunction

function PreserveFormDataForTargeting(byref working_action, edit_input, action_type)
	if (!working_action)
		working_action := ActionConfig(action_type);
	endif

	var delay_str := Trim(GFExtractData(edit_input, ID_ACTION_DELAY));
	if (delay_str && delay_str != "")
		working_action.delay := CInt(delay_str);
	endif

	case (action_type)
		"EMOTE":
			var text := Trim(GFExtractData(edit_input, ID_ACTION_EMOTE_TEXT));
			var show_on_source := edit_input[BTN_EMOTE_SHOW_ON_PLAYER] ? 1 : 0;
			working_action.params := struct{ text := text, show_on_source := show_on_source };

		"TELEPORT":
			var x_str := Trim(GFExtractData(edit_input, ID_ACTION_TELEPORT_X));
			var y_str := Trim(GFExtractData(edit_input, ID_ACTION_TELEPORT_Y));
			var z_str := Trim(GFExtractData(edit_input, ID_ACTION_TELEPORT_Z));
			var realm_str := Trim(GFExtractData(edit_input, ID_ACTION_TELEPORT_REALM));

			var new_params := struct{};
			if (x_str && x_str != "")
				new_params.+x := CInt(x_str);
			endif
			if (y_str && y_str != "")
				new_params.+y := CInt(y_str);
			endif
			if (z_str && z_str != "")
				new_params.+z := CInt(z_str);
			endif
			if (realm_str && realm_str != "")
				new_params.+realm := realm_str;
			endif
			working_action.params := new_params;

		"GIVE_ITEM":
			var amount_str := Trim(GFExtractData(edit_input, ID_ACTION_GIVE_ITEM_AMOUNT));
			var existing_template := 0;
			if (working_action.params && working_action.params.template)
				existing_template := working_action.params.template;
			endif
			working_action.params := struct{
				template := existing_template,
				amount := amount_str ? CInt(amount_str) : 0
			};
	endcase

	return 1;
endfunction

// ============================================================================
// CONVERSATION EDITOR
// ============================================================================

function HandleConversationEdit(who, existing_action)
	var conversation_id;

	if (existing_action && existing_action.params && existing_action.params.conversation_id)
		conversation_id := existing_action.params.conversation_id;
	else
		// Create new conversation
		conversation_id := CreateConversation(0, "NPC", who.name);
		if (!conversation_id)
			Message("Erro ao criar conversa!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return 0;
		endif
		Message($"Nova conversa criada: {conversation_id}", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
	endif

	// Try to resolve NPC from conversation metadata
	var npc := 0;
	var meta := GetConversationMeta(conversation_id);
	if (meta && meta.npc_serial)
		npc := SystemFindObjectBySerial(CInt(meta.npc_serial));
	endif

	// Enter the conversation editor loop
	var root_id := GetRootNodeId(conversation_id);
	ConversationEditorLoop(who, conversation_id, root_id, npc);

	// Return the action config
	var action := ActionConfig("CONVERSATION");
	action.params := struct{ conversation_id := conversation_id };
	return action;
endfunction

function ConversationEditorLoop(who, conversation_id, start_node_id, npc := 0)
	var current_node_id := start_node_id;

	while (current_node_id)
		var node := LoadConversationNode(conversation_id, current_node_id);
		if (!node)
			Message("Erro: no nao encontrado!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			break;
		endif

		var editor_gump := BuildNodeEditorGump(conversation_id, node, npc);
		var input := GFSendGump(who, editor_gump);

		if (!input)
			break;
		endif

		// Edit NPC profile
		if (input[BTN_CONV_EDIT_PROFILE] && npc)
			var profile_gump := BuildNpcProfileEditorGump(npc);
			var profile_input := GFSendGump(who, profile_gump);
			var profile_result := ParseNpcProfileEditorInput(profile_input);
			if (profile_result && profile_result.action == "save")
				SetObjProperty(npc, "url", profile_result.url);
				SetObjProperty(npc, "descricao", profile_result.descricao);
				Message("Perfil do NPC atualizado!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
			endif
			continue;

		// Edit dialogue text
		elseif (input[BTN_CONV_EDIT_TEXT])
			var existing_text := node.text ? node.text : "";
			var new_text := BigRequestGumpWithText(who, "Texto do dialogo:", existing_text);
			if (new_text && new_text != "")
				node.text := new_text;
				SaveConversationNode(conversation_id, node);
				Message("Texto atualizado!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
			endif
			continue;

		// Add new answer
		elseif (input[BTN_CONV_ADD_ANSWER])
			var answer_result := AnswerEditorFlow(who, conversation_id, 0);
			if (answer_result)
				node.AddAnswer(answer_result.text, answer_result.next_node, answer_result.conditions, answer_result.actions);
				SaveConversationNode(conversation_id, node);
				Message("Resposta adicionada!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
			endif
			continue;

		// Create a new dialogue node (without forcing immediate linking)
		elseif (input[BTN_CONV_CREATE_NODE])
			var next_dialog_number := GetNodeList(conversation_id).size() + 1;
			var default_dialog_text := $"Dialogo_{next_dialog_number}";
			var created_node_id := CreateNewNode(conversation_id, default_dialog_text);
			if (created_node_id)
				Message($"Novo dialogo criado: {created_node_id}", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
			else
				Message("Erro ao criar novo dialogo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			endif
			continue;

		// Done - exit editor
		elseif (input[BTN_CONV_DONE])
			break;

		// Tree overview
		elseif (input[BTN_CONV_TREE_VIEW])
			var tree_gump := BuildTreeOverviewGump(conversation_id);
			var tree_input := GFSendGump(who, tree_gump);
			if (tree_input)
				// Check if a node was selected in the tree
				var node_list := GetNodeList(conversation_id);
				var ni := 0;
				foreach nid in (node_list)
					if (tree_input[BTN_CONV_TREE_NODE_BASE + ni])
						current_node_id := nid;
						break;
					endif
					ni := ni + 1;
				endforeach
			endif
			continue;

		// Go to node selection
		elseif (input[BTN_CONV_GOTO_NODE])
			var sel_gump := BuildNodeSelectionGump(conversation_id);
			var sel_input := GFSendGump(who, sel_gump);
			if (sel_input)
				var node_list := GetNodeList(conversation_id);
				var ni := 0;
				foreach nid in (node_list)
					if (sel_input[BTN_CONV_TREE_NODE_BASE + ni])
						current_node_id := nid;
						break;
					endif
					ni := ni + 1;
				endforeach
			endif
			continue;
		endif

		// Check for answer edit/delete/goto buttons
		var handled := 0;
		for ans_idx := 1 to 10
			// Edit answer
			if (input[BTN_CONV_ANS_EDIT_BASE + ans_idx])
				if (ans_idx <= node.answers.size())
					var existing_answer := node.answers[ans_idx];
					var answer_result := AnswerEditorFlow(who, conversation_id, existing_answer);
					if (answer_result)
						node.answers[ans_idx].text := answer_result.text;
						node.answers[ans_idx].next_node := answer_result.next_node;
						node.answers[ans_idx].conditions := answer_result.conditions;
						node.answers[ans_idx].actions := answer_result.actions;
						SaveConversationNode(conversation_id, node);
						Message("Resposta atualizada!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
					endif
				endif
				handled := 1;
				break;

			// Delete answer
			elseif (input[BTN_CONV_ANS_DELETE_BASE + ans_idx])
				if (ans_idx <= node.answers.size())
					node.RemoveAnswer(ans_idx);
					SaveConversationNode(conversation_id, node);
					Message("Resposta removida!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
				endif
				handled := 1;
				break;

			// Navigate to linked node
			elseif (input[BTN_CONV_ANS_GOTO_BASE + ans_idx])
				if (ans_idx <= node.answers.size())
					var next := node.answers[ans_idx].next_node;
					if (next && next != "")
						current_node_id := next;
					endif
				endif
				handled := 1;
				break;
			endif
		endfor

		if (!handled)
			break;
		endif
	endwhile
endfunction

function AnswerEditorFlow(who, conversation_id, existing_answer)
	var working_answer := existing_answer;

	while (1)
		var node_list := GetNodeList(conversation_id);
		var ans_gump := BuildAnswerEditorGump(conversation_id, working_answer, node_list);
		var ans_input := GFSendGump(who, ans_gump);

		var result := ParseAnswerEditorInput(ans_input, node_list);
		if (!result)
			return 0;
		endif

		case (result.action)
			"cancel":
				return 0;

			"set_condition":
				var existing_cond := 0;
				if (working_answer && working_answer.conditions)
					existing_cond := working_answer.conditions;
				endif
				var new_cond := HandleConditionEdit(who, existing_cond);
				if (new_cond != -1)
					if (!working_answer)
						working_answer := struct{};
						working_answer.+text := "";
						working_answer.+next_node := "";
						working_answer.+conditions := 0;
						working_answer.+actions := array{};
					endif
					working_answer.conditions := new_cond;
					if (new_cond)
						Message("Condicao configurada!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
					else
						Message("Condicao removida.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
					endif
				endif
				continue;

			"add_action":
				var type_gump := BuildActionTypeSelectionGump();
				var type_input := GFSendGump(who, type_gump);
				var act_type := ParseActionTypeSelectionInput(type_input);

				if (act_type && act_type != "CONVERSATION")
					var temp_action := 0;
					var new_act := HandleActionEdit(who, act_type, temp_action);
					if (new_act)
						if (!working_answer)
							working_answer := struct{};
							working_answer.+text := "";
							working_answer.+next_node := "";
							working_answer.+conditions := 0;
							working_answer.+actions := array{};
						endif
						if (!working_answer.actions)
							working_answer.+actions := array{};
						endif
						working_answer.actions.append(new_act);
						Message($"Acao adicionada! Total: {working_answer.actions.size()}", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
					endif
				elseif (act_type == "CONVERSATION")
					Message("Nao eh possivel aninhar conversas!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				endif
				continue;

			"clear_actions":
				if (working_answer)
					working_answer.actions := array{};
					Message("Acoes removidas.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
				endif
				continue;

			"save":
				var next_node := result.next_node;

				// Handle "create new node"
				if (next_node == "__NEW__")
					var new_node_id := CreateNewNode(conversation_id, "Novo dialogo...");
					if (new_node_id)
						next_node := new_node_id;
						Message($"Novo no criado: {new_node_id}", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
					else
						next_node := "";
					endif
				endif

				var final_answer := struct{};
				final_answer.+text := result.text;
				final_answer.+next_node := next_node;
				final_answer.+conditions := (working_answer && working_answer.conditions) ? working_answer.conditions : 0;
				final_answer.+actions := (working_answer && working_answer.actions) ? working_answer.actions : array{};

				return final_answer;
		endcase
	endwhile

	return 0;
endfunction
