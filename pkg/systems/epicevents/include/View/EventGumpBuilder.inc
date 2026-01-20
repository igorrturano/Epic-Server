use uo;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:epicGumps";
include ":epicevents:Model/EventConfig";
include ":epicevents:Model/ActionConfig";
include ":epicevents:Model/CreatureSpawnInfo";
include ":text:Color";

const DEFAULT_TRIGGER_RADIUS := 8;

const GUMP_BACKGROUND := 39929;
const INTERNAL_BACKGROUND := 9350;
const GUMP_WIDTH := 540;
const GUMP_HEIGHT := 650;

const BTN_OK := 200;
const BTN_NEXT := 201;

// Trigger selection buttons
const BTN_TRIGGER_SPEECH := 210;
const BTN_TRIGGER_ENTEREDAREA := 211;
const BTN_TRIGGER_LEFTAREA := 212;
const BTN_TRIGGER_ITEM_GIVEN := 213;
const BTN_TRIGGER_DOUBLECLICKED := 214;

// Trigger config IDs
const ID_TRIGGER_KEYWORD := 120;
const ID_TRIGGER_RADIUS := 121;

// Action management buttons
const BTN_ACTION_ADD := 220;
const BTN_ACTION_EDIT_BASE := 230;  // 230-239 for editing actions 1-10
const BTN_ACTION_DELETE_BASE := 240;  // 240-249 for deleting actions 1-10
const BTN_ACTION_DONE := 250;

// Action type buttons
const BTN_ACTION_TYPE_EMOTE := 260;
const BTN_ACTION_TYPE_SPAWN := 261;
const BTN_ACTION_TYPE_TELEPORT := 262;

// Action edit field IDs
const ID_ACTION_DELAY := 300;
const ID_ACTION_EMOTE_TEXT := 301;
const BTN_EMOTE_SHOW_ON_PLAYER := 303;
const BTN_TARGET_ITEM := 304;
const BTN_TARGET_COORDS := 305;
const ID_ACTION_TELEPORT_X := 310;
const ID_ACTION_TELEPORT_Y := 311;
const ID_ACTION_TELEPORT_Z := 312;
const ID_ACTION_TELEPORT_REALM := 313;

// Condition field IDs
const ID_CONDITION_OBJTYPE := 320;
const ID_CONDITION_AMOUNT := 321;

// Radio button IDs for condition selection
const RADIO_CONDITION_NONE := 330;
const RADIO_CONDITION_HAS_ITEM := 331;

// Event delay fields
const ID_EVENT_MIN_DELAY := 340;
const ID_EVENT_MAX_DELAY := 341;

// Creature field IDs - base IDs, each creature uses base + (index * 4)
const ID_CREATURE_BASE := 100;
const ID_CREATURE_NAME_OFFSET := 0;
const ID_CREATURE_AMOUNT_OFFSET := 1;
const ID_CREATURE_TIER_OFFSET := 2;
const ID_CREATURE_TEMPLATE_OFFSET := 3;
const MAX_CREATURES := 3;

const ID_COLOR := 112;
const ID_EMOTE := 113;

// Helper to get creature field IDs
function GetCreatureFieldIds(creature_index)
	var base := ID_CREATURE_BASE + (creature_index * 4);
	return struct{
		name := base + ID_CREATURE_NAME_OFFSET,
		amount := base + ID_CREATURE_AMOUNT_OFFSET,
		tier := base + ID_CREATURE_TIER_OFFSET,
		template := base + ID_CREATURE_TEMPLATE_OFFSET
	};
endfunction

// Helper to build a single creature row in the gump
function BuildCreatureRow(byref gump, x, y, ids)
	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Nome:");
	GFResizePic(gump, x + 100, y, 9350, 150, 25);
	GFTextEntry(gump, x + 105, y + 5, 140, 20, GUMP_COLOR::TEXT_ENTRY, "", ids.name);

	GFTextLine(gump, x + 265, y, GUMP_COLOR::HELP_TEXT, "Qtd:");
	GFResizePic(gump, x + 310, y, 9350, 60, 25);
	GFTextEntry(gump, x + 315, y + 5, 50, 20, GUMP_COLOR::TEXT_ENTRY, "", ids.amount);

	GFTextLine(gump, x + 385, y, GUMP_COLOR::HELP_TEXT, "Tier:");
	GFResizePic(gump, x + 425, y, 9350, 60, 25);
	GFTextEntry(gump, x + 430, y + 5, 50, 20, GUMP_COLOR::TEXT_ENTRY, "", ids.tier);
	y := y + 30;

	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Template:");
	GFResizePic(gump, x + 100, y, 9350, 385, 25);
	GFTextEntry(gump, x + 105, y + 5, 375, 20, GUMP_COLOR::TEXT_ENTRY, "", ids.template);

	return y;
endfunction

function BuildEventConfigGump()
	var gump := GFECreateGump("Epic Events - Configuracao de Spawn", GUMP_WIDTH, GUMP_HEIGHT);
	var y := SetupGumpFrame(gump, GUMP_WIDTH, GUMP_HEIGHT, 0);  // No divider, we'll add section headers

	var x := 30;  // Moved 35px to the left (was 50)
	var spacing := 70;

	// Section header
	GFTextMid(gump, 0, y, GUMP_WIDTH, GUMP_COLOR::SECTION_HEADER, "CRIATURAS");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, GUMP_WIDTH - 80, 2);
	y := y + 20;

	// Build creature rows using loop (DRY)
	for i := 0 to (MAX_CREATURES - 1)
		var ids := GetCreatureFieldIds(i);
		y := BuildCreatureRow(gump, x, y, ids);
		if (i < MAX_CREATURES - 1)
			y := y + spacing;
		else
			y := y + 50;
		endif
	endfor

	// Section header
	GFTextMid(gump, 0, y, GUMP_WIDTH, GUMP_COLOR::SECTION_HEADER, "CONFIGURACOES ADICIONAIS");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, GUMP_WIDTH - 80, 2);
	y := y + 15;

	// Shared settings
	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Cor (opcional):");
	GFResizePic(gump, x + 150, y, 9350, 100, 25);
	GFTextEntry(gump, x + 155, y + 5, 90, 20, GUMP_COLOR::TEXT_ENTRY,"", ID_COLOR);
	y := y + 35;

	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Emote (opcional):");
	y := y + 25;
	GFResizePic(gump, x, y, 9350, 480, 25);
	GFTextEntry(gump, x + 5, y + 5, 470, 20, GUMP_COLOR::TEXT_ENTRY,"", ID_EMOTE);
	y := y + 40;

	// Divider before button
	GFResizePic(gump, 40, y, 3000, GUMP_WIDTH - 80, 2);

	// OK Button (moved 20px up - was y + 20)
	GFAddButton(gump, 250, y, 2152, 2153, GF_CLOSE_BTN, BTN_OK);

	return gump;
endfunction

function ParseEventConfigInput(input)
	if (!input || !input[BTN_OK])
		return 0;
	endif

	var config := EventConfig();

	// Parse creatures using loop (DRY)
	for i := 0 to (MAX_CREATURES - 1)
		var ids := GetCreatureFieldIds(i);
		var template := Trim(GFExtractData(input, ids.template));
		if (template && template != "")
			var creature := CreatureSpawnInfo();
			creature.template := template;
			creature.amount := Trim(GFExtractData(input, ids.amount));
			creature.tier := Trim(GFExtractData(input, ids.tier));
			creature.name := Trim(GFExtractData(input, ids.name));
			config.creatures.append(creature);
		endif
	endfor

	// Parse shared
	config.color := Trim(GFExtractData(input, ID_COLOR));
	config.emote := Trim(GFExtractData(input, ID_EMOTE));

	return config;
endfunction

// Helper function to set up common gump structure with frame and grey background
// Returns the starting Y position for content
function SetupGumpFrame(byref gump, width, height, show_divider := 1)
	// Grey background with proper padding from edges (like skillwin)
	var bg_x := 15;
	var bg_y := 70;
	var bg_width := width - 30;
	var bg_height := height - 85;

	GFResizePic(gump, bg_x, bg_y, INTERNAL_BACKGROUND, bg_width, bg_height);

	// Divider below title area (optional)
	if (show_divider)
		GFResizePic(gump, 30, 95, 3000, width - 60, 2);
		return 110;  // Start content below divider
	endif

	return 90;  // Start content without divider
endfunction

// Trigger selection gump
function BuildTriggerSelectionGump()
	var gump := GFECreateGump("Epic Events - Selecione o Gatilho", 600, 550);
	var y := SetupGumpFrame(gump, 600, 550);

	var x := 40;
	var spacing := 80;

	// Instructions
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Como este evento deve ser acionado?");
	y := y + 35;

	// SPEECH trigger
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_TRIGGER_SPEECH);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Fala (Palavra-chave)");
	GFTextLine(gump, x + 40, y + 22, GUMP_COLOR::LABEL, "Jogador diz uma palavra ou frase especifica");
	y := y + spacing;

	// ENTEREDAREA trigger
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_TRIGGER_ENTEREDAREA);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Entrada em Area (Proximidade)");
	GFTextLine(gump, x + 40, y + 22, GUMP_COLOR::LABEL, "Jogador entra na area ao redor do item");
	y := y + spacing;

	// LEFTAREA trigger
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_TRIGGER_LEFTAREA);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Saida de Area");
	GFTextLine(gump, x + 40, y + 22, GUMP_COLOR::LABEL, "Jogador sai da area (punicao por covardia!)");
	y := y + spacing;

	// ITEM_GIVEN trigger - DISABLED (Phase 2)
	// GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_TRIGGER_ITEM_GIVEN);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Entrega de Item (Oferenda) [DESABILITADO]");
	GFTextLine(gump, x + 40, y + 22, GUMP_COLOR::LABEL, "Jogador entrega um item especifico");
	y := y + spacing;

	// DOUBLECLICKED trigger - DISABLED (Phase 2)
	// GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_TRIGGER_DOUBLECLICKED);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Clique Duplo (Interacao) [DESABILITADO]");
	GFTextLine(gump, x + 40, y + 22, GUMP_COLOR::LABEL, "Jogador clica duas vezes no item");

	return gump;
endfunction

// Trigger configuration gump (for triggers that need extra params)
function BuildTriggerConfigGump(trigger_type)
	var gump := GFECreateGump("Configurar Gatilho", 600, 550);
	var y := SetupGumpFrame(gump, 600, 550, 0);

	var x := 50;

	// Section header
	GFTextMid(gump, 0, y, 600, GUMP_COLOR::SECTION_HEADER, "PARAMETROS DO GATILHO");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 520, 2);
	y := y + 20;

	case (trigger_type)
		"SPEECH":
			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Palavra-chave ou frase:");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 500, 25);
			GFTextEntry(gump, x + 5, y + 5, 490, 20, GUMP_COLOR::TEXT_ENTRY,"", ID_TRIGGER_KEYWORD);
			y := y + 35;
			GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "O jogador deve dizer exatamente esta palavra ou frase");
			GFTextLine(gump, x, y + 15, GUMP_COLOR::LABEL, "para ativar o evento.");
			y := y + 50;

			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Raio de audicao (em tiles):");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 100, 25);
			GFTextEntry(gump, x + 5, y + 5, 90, 20, GUMP_COLOR::TEXT_ENTRY,"8", ID_TRIGGER_RADIUS);
			y := y + 35;
			GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Distancia em que o NPC/item pode 'ouvir' o jogador");
		"ENTEREDAREA":
			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Raio de deteccao (em tiles):");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 100, 25);
			GFTextEntry(gump, x + 5, y + 5, 90, 20, GUMP_COLOR::TEXT_ENTRY,"8", ID_TRIGGER_RADIUS);
			y := y + 35;
			GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Distancia em que o jogador deve estar para ativar");
			GFTextLine(gump, x, y + 15, GUMP_COLOR::LABEL, "o evento. Padrao: 8 tiles (encontro).");
			GFTextLine(gump, x, y + 30, GUMP_COLOR::LABEL, "2 tiles = surpresa | 15 tiles = alerta antecipado");
		"LEFTAREA":
			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Raio de deteccao (em tiles):");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 100, 25);
			GFTextEntry(gump, x + 5, y + 5, 90, 20, GUMP_COLOR::TEXT_ENTRY,"8", ID_TRIGGER_RADIUS);
			y := y + 35;
			GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Distancia que o jogador deve sair para ativar");
			GFTextLine(gump, x, y + 15, GUMP_COLOR::LABEL, "o evento de saida (punicao por covardia!).");
		default:
			GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Este gatilho nao requer configuracao adicional.");
			GFTextLine(gump, x, y + 20, GUMP_COLOR::LABEL, "Clique em Proximo para continuar.");
	endcase

	// Add delay configuration section (for all trigger types)
	// Move down so "Distancia em que..." explanation stays with radius field above
	y := 350;  // Fixed position - moved down 30px

	// Section header
	GFTextMid(gump, 0, y, 600, GUMP_COLOR::SECTION_HEADER, "FREQUENCIA DE ATIVACAO");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 520, 2);
	y := y + 20;

	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Intervalo entre ativacoes (em minutos):");
	y := y + 25;

	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Minimo:");
	GFResizePic(gump, x + 70, y, 9350, 80, 25);
	GFTextEntry(gump, x + 75, y + 5, 70, 20, GUMP_COLOR::TEXT_ENTRY,"60", ID_EVENT_MIN_DELAY);

	GFTextLine(gump, x + 190, y, GUMP_COLOR::HELP_TEXT, "Maximo:");
	GFResizePic(gump, x + 270, y, 9350, 80, 25);
	GFTextEntry(gump, x + 275, y + 5, 70, 20, GUMP_COLOR::TEXT_ENTRY,"120", ID_EVENT_MAX_DELAY);
	y := y + 40;

	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "O evento so pode ativar novamente apos este intervalo");
	GFTextLine(gump, x, y + 15, GUMP_COLOR::LABEL, "(randomizado entre min e max)");

	// Divider before button
	y := y + 40;
	GFResizePic(gump, 40, y, 3000, 520, 2);

	// OK button (using okay button style)
	GFAddButton(gump, 270, y + 5, 2152, 2153, GF_CLOSE_BTN, BTN_NEXT);

	return gump;
endfunction

// Parse trigger configuration input
function ParseTriggerConfigInput(input, trigger_type)
	if (!input || !input[BTN_NEXT])
		return 0;
	endif

	var result := struct{};
	var params := struct{};

	case (trigger_type)
		"SPEECH":
			params.+keyword := Trim(GFExtractData(input, ID_TRIGGER_KEYWORD));
			if (!params.keyword || params.keyword == "")
				return 0;  // Keyword required
			endif
			// SPEECH also needs radius for hearing range
			var radius_str := Trim(GFExtractData(input, ID_TRIGGER_RADIUS));
			params.+radius := CInt(radius_str);
			if (params.radius < 1)
				params.radius := DEFAULT_TRIGGER_RADIUS;
			endif
		"ENTEREDAREA":
		"LEFTAREA":
			var radius_str := Trim(GFExtractData(input, ID_TRIGGER_RADIUS));
			params.+radius := CInt(radius_str);
			if (params.radius < 1)
				params.radius := DEFAULT_TRIGGER_RADIUS;
			endif
		default:
			// No params needed
	endcase

	// Parse delay configuration (applies to all trigger types)
	var min_delay_str := Trim(GFExtractData(input, ID_EVENT_MIN_DELAY));
	var max_delay_str := Trim(GFExtractData(input, ID_EVENT_MAX_DELAY));

	var min_delay := CInt(min_delay_str);
	var max_delay := CInt(max_delay_str);

	// Validate delays
	if (min_delay < 1)
		min_delay := DEFAULT_MIN_DELAY_MINUTES;
	endif
	if (max_delay < min_delay)
		max_delay := min_delay;  // Max can't be less than min
	endif

	// Return struct with both trigger params and delays
	result.+trigger_params := params;
	result.+min_delay := min_delay;
	result.+max_delay := max_delay;

	return result;
endfunction

// Action list gump - shows all configured actions
function BuildActionListGump(actions)
	var gump := GFECreateGump("Configurar Acoes do Evento", 600, 500);
	var y := SetupGumpFrame(gump, 600, 500, 0);

	var x := 30;

	// Instructions
	GFTextLine(gump, x, y, GUMP_COLOR::SECTION_HEADER, "Configuracao de Acoes");
	y := y + 25;
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "As acoes abaixo serao executadas em sequencia quando o gatilho ativar:");
	y := y + 30;

	// Action list
	if (!actions || actions.size() == 0)
		// Empty state
		GFResizePic(gump, x, y, 0x2486, 540, 80);  // Subtle background box
		GFTextLine(gump, x + 20, y + 15, GUMP_COLOR::HELP_TEXT, "Nenhuma acao configurada.");
		GFTextLine(gump, x + 20, y + 35, GUMP_COLOR::HELP_TEXT, "Clique em [Adicionar Acao] para criar uma acao.");
		GFTextLine(gump, x + 20, y + 55, GUMP_COLOR::HELP_TEXT, "Ou clique em [Concluir] para usar o sistema de spawn padrao.");
		y := y + 100;
	else
		var action_num := 1;
		foreach action in actions
			// Action summary line
			var action_desc := GetActionDescription(action);

			// Subtle background for each action
			var box_height := 35;
			if (action.delay && action.delay > 0)
				box_height := box_height + 18;
			endif
			if (action.condition && action.condition.type != "ALWAYS")
				box_height := box_height + 18;
			endif

			GFResizePic(gump, x, y, 0x2486, 540, box_height);

			// Edit button
			GFAddButton(gump, x + 10, y + 8, 0x659, 0x65A, GF_CLOSE_BTN, BTN_ACTION_EDIT_BASE + action_num);
			GFTooltipText(gump, "Editar esta acao");
			// Delete button
			GFAddButton(gump, x + 35, y + 8, 30008, 30009, GF_CLOSE_BTN, BTN_ACTION_DELETE_BASE + action_num);
			GFTooltipText(gump, "Excluir esta acao");

			// Action text
			GFTextLine(gump, x + 75, y + 8, 53, $"{action_num}. {action_desc}");

			var detail_y := y + 26;

			// Show delay if configured
			if (action.delay && action.delay > 0)
				GFTextLine(gump, x + 80, detail_y, 1153, $"Atraso: {action.delay}s");
				detail_y := detail_y + 18;
			endif

			// Show condition if configured
			if (action.condition && action.condition.type != "ALWAYS")
				var cond_desc := GetConditionDescription(action.condition);
				GFTextLine(gump, x + 80, detail_y, 1153, $"Condicao: {cond_desc}");
			endif

			y := y + box_height + 8;  // Add spacing between actions
			action_num := action_num + 1;

			if (action_num > 10)
				break;  // Max 10 actions
			endif
		endforeach
	endif

	// Add action button
	GFAddButton(gump, x, y + 5, 2152, 2153, GF_CLOSE_BTN, BTN_ACTION_ADD);
	GFTextLine(gump, x + 40, y + 10, GUMP_COLOR::HIGHLIGHT, "[Adicionar Acao]");

	// Done button (final finish button - OKAY style)
	GFAddButton(gump, 280, 435, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_ACTION_DONE);

	return gump;
endfunction

// Helper: Get action description for display
function GetActionDescription(action)
	case (action.type)
		"EMOTE":
			var text := action.params.text;
			if (text && Len(text) > 40)
				text := SubStr(text, 1, 40) + "...";
			endif
			return $"Emote: \"{text}\"";
		"SPAWN":
			return "Spawn criaturas";
		"TELEPORT":
			var tp_desc := $"Teleportar para ({action.params.x},{action.params.y},{action.params.z})";
			if (action.params.realm && action.params.realm != "")
				tp_desc := tp_desc + $" [{action.params.realm}]";
			endif
			return tp_desc;
		default:
			return action.type;
	endcase
endfunction

// Helper: Get condition description for display
function GetConditionDescription(condition)
	case (condition.type)
		"PLAYER_HAS_ITEM":
			return $"Jogador tem item {Hex(condition.params.objtype)}";
		default:
			return condition.type;
	endcase
endfunction

// Action type selection gump
function BuildActionTypeSelectionGump()
	var gump := GFECreateGump("Selecione o Tipo de Acao", 500, 350);
	var y := SetupGumpFrame(gump, 500, 350);

	var x := 40;
	var spacing := 70;

	// Instructions
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Que tipo de acao deseja adicionar?");
	y := y + 30;

	// EMOTE action
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_ACTION_TYPE_EMOTE);
	GFTextLine(gump, x + 40, y, GUMP_COLOR::HIGHLIGHT, "Emote/Mensagem");
	GFTextLine(gump, x + 40, y + 20, GUMP_COLOR::HELP_TEXT, "Exibir uma mensagem ou emote para criar atmosfera");
	y := y + spacing;

	// SPAWN action
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_ACTION_TYPE_SPAWN);
	GFTextLine(gump, x + 40, y, GUMP_COLOR::HIGHLIGHT, "Spawn de Criaturas");
	GFTextLine(gump, x + 40, y + 20, GUMP_COLOR::HELP_TEXT, "Criar criaturas no local do evento");
	y := y + spacing;

	// TELEPORT action
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_ACTION_TYPE_TELEPORT);
	GFTextLine(gump, x + 40, y, GUMP_COLOR::HIGHLIGHT, "Teleporte");
	GFTextLine(gump, x + 40, y + 20, GUMP_COLOR::HELP_TEXT, "Teleportar o jogador para outro local");

	return gump;
endfunction

// Action edit gump - configure action parameters
function BuildActionEditGump(action_type, existing_action := 0, who := 0)
	var title_text := "Configurar Acao: ";
	case (action_type)
		"EMOTE":
			title_text := title_text + "Emote/Mensagem";
		"SPAWN":
			title_text := title_text + "Spawn de Criaturas";
		"TELEPORT":
			title_text := title_text + "Teleporte";
	endcase

	var gump := GFECreateGump(title_text, 600, 650);
	var y := SetupGumpFrame(gump, 600, 650, 0);  // No divider, we'll add section headers

	var x := 50;

	// ==================== SECTION: TIMING ====================
	GFTextMid(gump, 0, y, 600, GUMP_COLOR::SECTION_HEADER, "TEMPORIZACAO");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 520, 2);
	y := y + 15;

	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Atraso antes da acao (em segundos, 0 = imediato):");
	y := y + 25;
	var delay_default := "0";
	if (existing_action && existing_action.delay)
		delay_default := CStr(existing_action.delay);
	endif
	GFResizePic(gump, x, y, 9350, 100, 25);
	GFTextEntry(gump, x + 5, y + 5, 90, 20, GUMP_COLOR::TEXT_ENTRY,delay_default, ID_ACTION_DELAY);
	GFTextLine(gump, x + 120, y + 5, GUMP_COLOR::LABEL, "Ex: 5 = aguardar 5 segundos");
	y := y + 45;

	// ==================== SECTION: ACTION PARAMETERS ====================
	GFTextMid(gump, 0, y, 600, GUMP_COLOR::SECTION_HEADER, "PARAMETROS DA ACAO");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 520, 2);
	y := y + 15;

	case (action_type)
		"EMOTE":
			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Texto do emote/mensagem:");
			y := y + 25;
			var emote_default := "";
			if (existing_action && existing_action.params && existing_action.params.text)
				emote_default := existing_action.params.text;
			endif
			GFResizePic(gump, x, y, 9350, 500, 65);
			GFTextEntry(gump, x + 5, y + 5, 490, 60, GUMP_COLOR::TEXT_ENTRY,emote_default, ID_ACTION_EMOTE_TEXT);
			y := y + 80;

			// Checkbox for showing emote on player vs item
			var show_on_player := 0;
			if (existing_action && existing_action.params && existing_action.params.show_on_source)
				show_on_player := existing_action.params.show_on_source;
			endif
			GFCheckBox(gump, x, y, 0x938, 0x939, show_on_player, BTN_EMOTE_SHOW_ON_PLAYER);
			GFTextLine(gump, x + 30, y + 2, GUMP_COLOR::HELP_TEXT, "Exibir emote acima do jogador (ao inves do item)");
			y := y + 35;

		"SPAWN":
			GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "O spawn de criaturas usa a configuracao padrao do sistema.");
			GFTextLine(gump, x, y + 20, GUMP_COLOR::LABEL, "Voce podera configurar criaturas, quantidades e tiers");
			GFTextLine(gump, x, y + 40, GUMP_COLOR::LABEL, "na proxima etapa do wizard.");
			y := y + 75;

		"TELEPORT":
			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Coordenadas do destino:");
			GFAddButton(gump, x + 390, y + 2, 2117, 2118, GF_CLOSE_BTN, BTN_TARGET_COORDS);
			GFTextLine(gump, x + 410, y + 5, GUMP_COLOR::LABEL, "[Target]");
			y := y + 25;
			var x_default := "";
			var y_default := "";
			var z_default := "";
			var realm_default := "";
			if (existing_action && existing_action.params)
				if (existing_action.params.x)
					x_default := CStr(existing_action.params.x);
				endif
				if (existing_action.params.y)
					y_default := CStr(existing_action.params.y);
				endif
				if (existing_action.params.z)
					z_default := CStr(existing_action.params.z);
				endif
				if (existing_action.params.realm)
					realm_default := existing_action.params.realm;
				endif
			endif

			// Set default realm to player's current realm if not set
			if (realm_default == "" && who)
				realm_default := who.realm;
			endif

			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "X:");
			GFResizePic(gump, x + 25, y, 9350, 80, 25);
			GFTextEntry(gump, x + 30, y + 5, 70, 20, GUMP_COLOR::TEXT_ENTRY,x_default, ID_ACTION_TELEPORT_X);

			GFTextLine(gump, x + 125, y, GUMP_COLOR::HELP_TEXT, "Y:");
			GFResizePic(gump, x + 150, y, 9350, 80, 25);
			GFTextEntry(gump, x + 155, y + 5, 70, 20, GUMP_COLOR::TEXT_ENTRY,y_default, ID_ACTION_TELEPORT_Y);

			GFTextLine(gump, x + 250, y, GUMP_COLOR::HELP_TEXT, "Z:");
			GFResizePic(gump, x + 275, y, 9350, 80, 25);
			GFTextEntry(gump, x + 280, y + 5, 70, 20, GUMP_COLOR::TEXT_ENTRY,z_default, ID_ACTION_TELEPORT_Z);
			y := y + 35;

			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Realm:");
			GFResizePic(gump, x + 60, y, 9350, 120, 25);
			GFTextEntry(gump, x + 65, y + 5, 110, 20, GUMP_COLOR::TEXT_ENTRY,realm_default, ID_ACTION_TELEPORT_REALM);
			y := y + 35;

			GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Ex: X=1000 Y=2000 Z=0 Realm=britannia");
			y := y + 35;
	endcase

	// ==================== SECTION: CONDITION ====================
	GFTextMid(gump, 0, y, 600, GUMP_COLOR::SECTION_HEADER, "CONDICAO DE EXECUCAO (OPCIONAL)");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 520, 2);
	y := y + 15;

	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Quando esta acao deve ser executada?");
	y := y + 25;

	// Determine current condition
	var current_condition := "NONE";
	if (existing_action && existing_action.condition && existing_action.condition.type)
		current_condition := existing_action.condition.type;
	endif

	// Radio button: Always execute
	GFRadioButton(gump, x, y, 0x25F8, 0x25FB, (current_condition == "NONE" ? 1 : 0), RADIO_CONDITION_NONE);
	GFTextLine(gump, x + 30, y + 2, GUMP_COLOR::HELP_TEXT, "Sempre");
	y := y + 30;

	// Radio button: Player has item
	GFRadioButton(gump, x, y, 0x25F8, 0x25FB, (current_condition == "PLAYER_HAS_ITEM" ? 1 : 0), RADIO_CONDITION_HAS_ITEM);
	GFTextLine(gump, x + 30, y + 2, GUMP_COLOR::HELP_TEXT, "Jogador possui item");
	y := y + 25;

	// Parameter fields for "Player has item" (always shown)
	var objtype_default := "";
	var amount_default := "1";
	if (existing_action && existing_action.condition && existing_action.condition.params)
		if (existing_action.condition.params.objtype)
			objtype_default := Hex(existing_action.condition.params.objtype);
		endif
		if (existing_action.condition.params.amount)
			amount_default := CStr(existing_action.condition.params.amount);
		endif
	endif

	GFTextLine(gump, x + 40, y, GUMP_COLOR::LABEL, "Item (ID ou Nome):");
	GFResizePic(gump, x + 180, y, 9350, 200, 25);
	GFTextEntry(gump, x + 185, y + 5, 190, 20, GUMP_COLOR::TEXT_ENTRY,objtype_default, ID_CONDITION_OBJTYPE);
	GFAddButton(gump, x + 390, y + 2, 2117, 2118, GF_CLOSE_BTN, BTN_TARGET_ITEM);
	GFTextLine(gump, x + 410, y + 5, GUMP_COLOR::LABEL, "[Target]");
	y := y + 35;

	GFTextLine(gump, x + 40, y, GUMP_COLOR::LABEL, "Quantidade:");
	GFResizePic(gump, x + 140, y, 9350, 70, 25);
	GFTextEntry(gump, x + 145, y + 5, 60, 20, GUMP_COLOR::TEXT_ENTRY,amount_default, ID_CONDITION_AMOUNT);
	y := y + 40;

	// Divider before button
	GFResizePic(gump, 40, y, 3000, 520, 2);

	// Save button
	GFAddButton(gump, 280, y + 20, 2152, 2153, GF_CLOSE_BTN, BTN_NEXT);

	return gump;
endfunction

// Parse action list input - determine what button was pressed
function ParseActionListInput(input)
	if (!input)
		return struct{ action := "cancel" };
	endif

	if (input[BTN_ACTION_ADD])
		return struct{ action := "add" };
	elseif (input[BTN_ACTION_DONE])
		return struct{ action := "done" };
	else
		// Check for edit/delete buttons
		var i;
		for (i := 1; i <= 10; i := i + 1)
			if (input[BTN_ACTION_EDIT_BASE + i])
				return struct{ action := "edit", index := i };
			elseif (input[BTN_ACTION_DELETE_BASE + i])
				return struct{ action := "delete", index := i };
			endif
		endfor
	endif

	return struct{ action := "cancel" };
endfunction

// Parse action type selection input
function ParseActionTypeSelectionInput(input)
	if (!input)
		return 0;
	endif

	if (input[BTN_ACTION_TYPE_EMOTE])
		return "EMOTE";
	elseif (input[BTN_ACTION_TYPE_SPAWN])
		return "SPAWN";
	elseif (input[BTN_ACTION_TYPE_TELEPORT])
		return "TELEPORT";
	endif

	return 0;
endfunction

// Parse action edit input - build ActionConfig from gump input
function ParseActionEditInput(input, action_type)
	if (!input || !input[BTN_NEXT])
		return 0;
	endif

	var action := ActionConfig(action_type);

	// Parse delay
	var delay_str := Trim(GFExtractData(input, ID_ACTION_DELAY));
	action.delay := CInt(delay_str);
	if (action.delay < 0)
		action.delay := 0;
	endif

	// Parse action-specific params
	action.params := struct{};

	case (action_type)
		"EMOTE":
			action.params.+text := Trim(GFExtractData(input, ID_ACTION_EMOTE_TEXT));
			if (!action.params.text || action.params.text == "")
				return 0;  // Text required
			endif

			// Check if emote should be shown on player
			action.params.+show_on_source := input[BTN_EMOTE_SHOW_ON_PLAYER] ? 1 : 0;

		"SPAWN":
			// SPAWN params will be filled in later from spawn config gump
			action.params.+use_spawn_config := 1;

		"TELEPORT":
			var x_str := Trim(GFExtractData(input, ID_ACTION_TELEPORT_X));
			var y_str := Trim(GFExtractData(input, ID_ACTION_TELEPORT_Y));
			var z_str := Trim(GFExtractData(input, ID_ACTION_TELEPORT_Z));
			var realm_str := Trim(GFExtractData(input, ID_ACTION_TELEPORT_REALM));

			action.params.+x := CInt(x_str);
			action.params.+y := CInt(y_str);
			action.params.+z := CInt(z_str);
			action.params.+realm := realm_str;

			if (!action.params.x || !action.params.y)
				return 0;  // X and Y required
			endif
	endcase

	// Parse condition based on radio button selection
	if (input[RADIO_CONDITION_HAS_ITEM])
		// Player has item condition
		var objtype_str := Trim(GFExtractData(input, ID_CONDITION_OBJTYPE));
		var amount_str := Trim(GFExtractData(input, ID_CONDITION_AMOUNT));

		if (objtype_str && objtype_str != "")
			action.condition := struct{};
			action.condition.+type := "PLAYER_HAS_ITEM";
			action.condition.+params := struct{};
			action.condition.params.+objtype := CInt(objtype_str);  // Accepts hex
			action.condition.params.+amount := CInt(amount_str);
			if (action.condition.params.amount < 1)
				action.condition.params.amount := 1;
			endif
		else
			// No objtype specified - invalid condition
			action.condition := 0;
		endif
	else
		// RADIO_CONDITION_NONE selected or no radio selected - always execute
		action.condition := 0;
	endif

	return action;
endfunction
