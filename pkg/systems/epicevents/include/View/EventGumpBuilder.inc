use uo;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:epicGumps";
include ":epicevents:Model/EventConfig";
include ":epicevents:Model/ActionConfig";

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
const ID_ACTION_EMOTE_RANGE := 302;
const BTN_EMOTE_SHOW_ON_PLAYER := 303;
const ID_ACTION_TELEPORT_X := 310;
const ID_ACTION_TELEPORT_Y := 311;
const ID_ACTION_TELEPORT_Z := 312;

// Condition type buttons
const BTN_CONDITION_NONE := 270;
const BTN_CONDITION_PLAYER_HAS_ITEM := 271;
const BTN_CONDITION_ACTION_SUCCEEDED := 272;

// Condition field IDs
const ID_CONDITION_OBJTYPE := 320;
const ID_CONDITION_AMOUNT := 321;
const ID_CONDITION_ACTION_ID := 322;

// Radio button group and IDs for condition selection
const RADIO_CONDITION_GROUP := 1;
const RADIO_CONDITION_NONE := 330;
const RADIO_CONDITION_HAS_ITEM := 331;
const RADIO_CONDITION_ACTION_SUCCEEDED := 332;

// Event delay fields
const ID_EVENT_MIN_DELAY := 340;
const ID_EVENT_MAX_DELAY := 341;

const ID_NAME1 := 100;
const ID_AMOUNT1 := 101;
const ID_TIER1 := 102;
const ID_TEMPLATE1 := 103;

const ID_NAME2 := 104;
const ID_AMOUNT2 := 105;
const ID_TIER2 := 106;
const ID_TEMPLATE2 := 107;

const ID_NAME3 := 108;
const ID_AMOUNT3 := 109;
const ID_TIER3 := 110;
const ID_TEMPLATE3 := 111;

const ID_COLOR := 112;
const ID_EMOTE := 113;
const ID_MIN_DELAY := 114;
const ID_MAX_DELAY := 115;

function BuildEventConfigGump()
	var gump := GFECreateGump("", GUMP_WIDTH, GUMP_HEIGHT, array{HIDE_BORDER});
	AdicionarMoldura(gump);
	GFResizePic(gump, 15, 70, INTERNAL_BACKGROUND, GUMP_WIDTH - 20, GUMP_HEIGHT - 80);
	GFTextMid(gump, 0, 35, GUMP_WIDTH, 53, "Epic Events - Configuracao de Spawn");

	var x := 30;
	var y := 90;
	var spacing := 70;

	// Section header
	GFTextMid(gump, 0, y, GUMP_WIDTH, 1149, "CRIATURAS");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, GUMP_WIDTH - 80, 2);
	y := y + 15;

	// Creature 1
	GFTextLine(gump, x, y, 1153, "Nome:");
	GFResizePic(gump, x + 100, y, 9350, 150, 25);
	GFTextEntry(gump, x + 105, y + 5, 140, 20, 1150, "", ID_NAME1);

	GFTextLine(gump, x + 265, y, 1153, "Qtd:");
	GFResizePic(gump, x + 310, y, 9350, 60, 25);
	GFTextEntry(gump, x + 315, y + 5, 50, 20, 1150, "", ID_AMOUNT1);

	GFTextLine(gump, x + 385, y, 1153, "Tier:");
	GFResizePic(gump, x + 425, y, 9350, 60, 25);
	GFTextEntry(gump, x + 430, y + 5, 50, 20, 1150, "", ID_TIER1);
	y := y + 30;

	GFTextLine(gump, x, y, 1153, "Template:");
	GFResizePic(gump, x + 100, y, 9350, 385, 25);
	GFTextEntry(gump, x + 105, y + 5, 375, 20, 1150, "", ID_TEMPLATE1);
	y := y + spacing;

	// Creature 2
	GFTextLine(gump, x, y, 1153, "Nome:");
	GFResizePic(gump, x + 100, y, 9350, 150, 25);
	GFTextEntry(gump, x + 105, y + 5, 140, 20, 1150, "", ID_NAME2);

	GFTextLine(gump, x + 265, y, 1153, "Qtd:");
	GFResizePic(gump, x + 310, y, 9350, 60, 25);
	GFTextEntry(gump, x + 315, y + 5, 50, 20, 1150, "", ID_AMOUNT2);

	GFTextLine(gump, x + 385, y, 1153, "Tier:");
	GFResizePic(gump, x + 425, y, 9350, 60, 25);
	GFTextEntry(gump, x + 430, y + 5, 50, 20, 1150, "", ID_TIER2);
	y := y + 30;

	GFTextLine(gump, x, y, 1153, "Template:");
	GFResizePic(gump, x + 100, y, 9350, 385, 25);
	GFTextEntry(gump, x + 105, y + 5, 375, 20, 1150, "", ID_TEMPLATE2);
	y := y + spacing;

	// Creature 3
	GFTextLine(gump, x, y, 1153, "Nome:");
	GFResizePic(gump, x + 100, y, 9350, 150, 25);
	GFTextEntry(gump, x + 105, y + 5, 140, 20, 1150, "", ID_NAME3);

	GFTextLine(gump, x + 265, y, 1153, "Qtd:");
	GFResizePic(gump, x + 310, y, 9350, 60, 25);
	GFTextEntry(gump, x + 315, y + 5, 50, 20, 1150, "", ID_AMOUNT3);

	GFTextLine(gump, x + 385, y, 1153, "Tier:");
	GFResizePic(gump, x + 425, y, 9350, 60, 25);
	GFTextEntry(gump, x + 430, y + 5, 50, 20, 1150, "", ID_TIER3);
	y := y + 30;

	GFTextLine(gump, x, y, 1153, "Template:");
	GFResizePic(gump, x + 100, y, 9350, 385, 25);
	GFTextEntry(gump, x + 105, y + 5, 375, 20, 1150, "", ID_TEMPLATE3);
	y := y + 50;

	// Section header
	GFTextMid(gump, 0, y, GUMP_WIDTH, 1149, "CONFIGURACOES ADICIONAIS");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, GUMP_WIDTH - 80, 2);
	y := y + 15;

	// Shared settings
	GFTextLine(gump, x, y, 1153, "Cor (opcional):");
	GFResizePic(gump, x + 150, y, 9350, 100, 25);
	GFTextEntry(gump, x + 155, y + 5, 90, 20, 1150, "", ID_COLOR);
	y := y + 35;

	GFTextLine(gump, x, y, 1153, "Emote (opcional):");
	y := y + 25;
	GFResizePic(gump, x, y, 9350, 480, 25);
	GFTextEntry(gump, x + 5, y + 5, 470, 20, 1150, "", ID_EMOTE);
	y := y + 40;

	GFTextLine(gump, x, y, 1153, "Delay (em minutos):");
	y := y + 25;

	GFTextLine(gump, x, y, 1153, "Minimo:");
	GFResizePic(gump, x + 80, y, 9350, 80, 25);
	GFTextEntry(gump, x + 85, y + 5, 70, 20, 1150, "60", ID_MIN_DELAY);

	GFTextLine(gump, x + 190, y, 1153, "Maximo:");
	GFResizePic(gump, x + 270, y, 9350, 80, 25);
	GFTextEntry(gump, x + 275, y + 5, 70, 20, 1150, "120", ID_MAX_DELAY);
	y := y + 40;

	// Divider before button
	GFResizePic(gump, 40, y, 3000, GUMP_WIDTH - 80, 2);

	// OK Button
	GFAddButton(gump, 250, y + 15, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_OK);

	return gump;
endfunction

function ParseEventConfigInput(input)
	if (!input || !input[BTN_OK])
		return 0;
	endif

	var config := EventConfig();

	// Parse creature 1
	config.template1 := Trim(GFExtractData(input, ID_TEMPLATE1));
	config.amount1 := Trim(GFExtractData(input, ID_AMOUNT1));
	config.tier1 := Trim(GFExtractData(input, ID_TIER1));
	config.name1 := Trim(GFExtractData(input, ID_NAME1));

	// Parse creature 2
	config.template2 := Trim(GFExtractData(input, ID_TEMPLATE2));
	config.amount2 := Trim(GFExtractData(input, ID_AMOUNT2));
	config.tier2 := Trim(GFExtractData(input, ID_TIER2));
	config.name2 := Trim(GFExtractData(input, ID_NAME2));

	// Parse creature 3
	config.template3 := Trim(GFExtractData(input, ID_TEMPLATE3));
	config.amount3 := Trim(GFExtractData(input, ID_AMOUNT3));
	config.tier3 := Trim(GFExtractData(input, ID_TIER3));
	config.name3 := Trim(GFExtractData(input, ID_NAME3));

	// Parse shared
	config.color := Trim(GFExtractData(input, ID_COLOR));
	config.emote := Trim(GFExtractData(input, ID_EMOTE));
	config.min_delay := Trim(GFExtractData(input, ID_MIN_DELAY));
	config.max_delay := Trim(GFExtractData(input, ID_MAX_DELAY));

	return config;
endfunction

function AdicionarMoldura(byref gump)
	var molduras := array{struct{x := 0, y := 0, width := 25, height := 25, gump := 39925},
						  struct{x := GUMP_WIDTH - 25, y := 0, width := 35, height := 25, gump := 39927},
						  struct{x := 25, y := 0, width := GUMP_WIDTH - 41, height := 25, gump := 39926},
						  struct{x := 25, y := GUMP_HEIGHT - 25, width := GUMP_WIDTH - 25, height := 35, gump := 39932},
						  struct{x := 0, y := 25, width := 10, height := GUMP_HEIGHT - 50, gump := 39928},
						  struct{x := GUMP_WIDTH - 25, y := 25, width := 35, height := GUMP_HEIGHT - 50, gump := 39930},
						  struct{x := GUMP_WIDTH - 25, y := GUMP_HEIGHT - 25, width := 35, height := 35, gump := 39933},
						  struct{x := 0, y := GUMP_HEIGHT - 25, width := 35, height := 35, gump := 39931}};

	foreach moldura in molduras
		GFGumpPicTiled(gump, moldura.x, moldura.y, moldura.width, moldura.height, moldura.gump);
	endforeach
endfunction

// Trigger selection gump
function BuildTriggerSelectionGump()
	var gump := GFECreateGump("", 500, 450, array{HIDE_BORDER});

	// Frame
	GFResizePic(gump, 0, 0, 39925, 500, 25);
	GFResizePic(gump, 475, 0, 39927, 35, 25);
	GFResizePic(gump, 25, 0, 39926, 441, 25);
	GFResizePic(gump, 25, 425, 39932, 450, 35);
	GFResizePic(gump, 0, 25, 39928, 10, 400);
	GFResizePic(gump, 475, 25, 39930, 35, 400);
	GFResizePic(gump, 475, 425, 39933, 35, 35);
	GFResizePic(gump, 0, 425, 39931, 35, 35);

	// Background
	GFResizePic(gump, 15, 70, INTERNAL_BACKGROUND, 470, 350);

	// Title
	GFTextMid(gump, 80, 35, 420, 53, "Epic! Events - Selecione o Gatilho");

	var x := 40;
	var y := 90;
	var spacing := 65;

	// Instructions
	GFTextLine(gump, x, y, 2100, "Como este evento deve ser acionado?");
	y := y + 30;

	// SPEECH trigger
	GFAddButton(gump, x, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_TRIGGER_SPEECH);
	GFTextLine(gump, x + 40, y, 53, "Fala (Palavra-chave)");
	GFTextLine(gump, x + 40, y + 20, 2100, "Jogador diz uma palavra ou frase especifica");
	y := y + spacing;

	// ENTEREDAREA trigger
	GFAddButton(gump, x, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_TRIGGER_ENTEREDAREA);
	GFTextLine(gump, x + 40, y, 53, "Entrada em Area (Proximidade)");
	GFTextLine(gump, x + 40, y + 20, 2100, "Jogador entra na area ao redor do item");
	y := y + spacing;

	// LEFTAREA trigger
	GFAddButton(gump, x, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_TRIGGER_LEFTAREA);
	GFTextLine(gump, x + 40, y, 53, "Saida de Area");
	GFTextLine(gump, x + 40, y + 20, 2100, "Jogador sai da area (punicao por covardia!)");
	y := y + spacing;

	// ITEM_GIVEN trigger
	GFAddButton(gump, x, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_TRIGGER_ITEM_GIVEN);
	GFTextLine(gump, x + 40, y, 53, "Entrega de Item (Oferenda)");
	GFTextLine(gump, x + 40, y + 20, 1153, "Jogador entrega um item especifico");
	y := y + spacing;

	// DOUBLECLICKED trigger
	GFAddButton(gump, x, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_TRIGGER_DOUBLECLICKED);
	GFTextLine(gump, x + 40, y, 53, "Clique Duplo (Interacao)");
	GFTextLine(gump, x + 40, y + 20, 1153, "Jogador clica duas vezes no item");

	return gump;
endfunction

// Trigger configuration gump (for triggers that need extra params)
function BuildTriggerConfigGump(trigger_type)
	var gump := GFECreateGump("", 500, 450, array{HIDE_BORDER});

	// Frame
	GFResizePic(gump, 0, 0, 39925, 500, 25);
	GFResizePic(gump, 475, 0, 39927, 35, 25);
	GFResizePic(gump, 25, 0, 39926, 441, 25);
	GFResizePic(gump, 25, 425, 39932, 450, 35);
	GFResizePic(gump, 0, 25, 39928, 10, 400);
	GFResizePic(gump, 475, 25, 39930, 35, 400);
	GFResizePic(gump, 475, 425, 39933, 35, 35);
	GFResizePic(gump, 0, 425, 39931, 35, 35);

	// Background
	GFResizePic(gump, 15, 70, INTERNAL_BACKGROUND, 470, 350);

	// Title
	GFTextMid(gump, 0, 35, 500, 53, "Configurar Gatilho");

	var x := 30;
	var y := 90;

	// Section header
	GFTextMid(gump, 0, y, 500, 1149, "PARAMETROS DO GATILHO");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 420, 2);
	y := y + 15;

	case (trigger_type)
		"SPEECH":
			GFTextLine(gump, x, y, 1153, "Palavra-chave ou frase:");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 440, 25);
			GFTextEntry(gump, x + 5, y + 5, 430, 20, 1150, "", ID_TRIGGER_KEYWORD);
			y := y + 35;
			GFTextLine(gump, x, y, 2100, "O jogador deve dizer exatamente esta palavra ou frase");
			GFTextLine(gump, x, y + 15, 2100, "para ativar o evento.");
			y := y + 45;

			GFTextLine(gump, x, y, 1153, "Raio de audicao (em tiles):");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 100, 25);
			GFTextEntry(gump, x + 5, y + 5, 90, 20, 1150, "8", ID_TRIGGER_RADIUS);
			y := y + 35;
			GFTextLine(gump, x, y, 2100, "Distancia em que o NPC/item pode 'ouvir' o jogador");
		"ENTEREDAREA":
			GFTextLine(gump, x, y, 1153, "Raio de deteccao (em tiles):");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 100, 25);
			GFTextEntry(gump, x + 5, y + 5, 90, 20, 1150, "8", ID_TRIGGER_RADIUS);
			y := y + 35;
			GFTextLine(gump, x, y, 2100, "Distancia em que o jogador deve estar para ativar");
			GFTextLine(gump, x, y + 15, 2100, "o evento. Padrao: 8 tiles (encontro).");
			GFTextLine(gump, x, y + 30, 2100, "2 tiles = surpresa | 15 tiles = alerta antecipado");
		"LEFTAREA":
			GFTextLine(gump, x, y, 1153, "Raio de deteccao (em tiles):");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 100, 25);
			GFTextEntry(gump, x + 5, y + 5, 90, 20, 1150, "8", ID_TRIGGER_RADIUS);
			y := y + 35;
			GFTextLine(gump, x, y, 2100, "Distancia que o jogador deve sair para ativar");
			GFTextLine(gump, x, y + 15, 2100, "o evento de saida (punicao por covardia!).");
		default:
			GFTextLine(gump, x, y, 2100, "Este gatilho nao requer configuracao adicional.");
			GFTextLine(gump, x, y + 20, 2100, "Clique em Proximo para continuar.");
	endcase

	// Add delay configuration section (for all trigger types)
	y := 250;  // Fixed position

	// Section header
	GFTextMid(gump, 0, y, 500, 1149, "FREQUENCIA DE ATIVACAO");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 420, 2);
	y := y + 15;

	GFTextLine(gump, x, y, 2100, "Intervalo entre ativacoes (em minutos):");
	y := y + 25;

	GFTextLine(gump, x, y, 1153, "Minimo:");
	GFResizePic(gump, x + 70, y, 9350, 80, 25);
	GFTextEntry(gump, x + 75, y + 5, 70, 20, 1150, "60", ID_EVENT_MIN_DELAY);

	GFTextLine(gump, x + 170, y, 1153, "Maximo:");
	GFResizePic(gump, x + 240, y, 9350, 80, 25);
	GFTextEntry(gump, x + 245, y + 5, 70, 20, 1150, "120", ID_EVENT_MAX_DELAY);
	y := y + 35;

	GFTextLine(gump, x, y, 2100, "O evento so pode ativar novamente apos este intervalo");
	GFTextLine(gump, x, y + 15, 2100, "(randomizado entre min e max)");

	// Divider before button
	y := y + 35;
	GFResizePic(gump, 40, y, 3000, 420, 2);

	// Next button
	GFAddButton(gump, 220, 390, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_NEXT);

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
				params.radius := 8;  // Default
			endif
		"ENTEREDAREA":
		"LEFTAREA":
			var radius_str := Trim(GFExtractData(input, ID_TRIGGER_RADIUS));
			params.+radius := CInt(radius_str);
			if (params.radius < 1)
				params.radius := 8;  // Default
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
		min_delay := 60;  // Default 1 hour
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
	var gump := GFECreateGump("", 600, 500, array{HIDE_BORDER});

	// Frame
	GFResizePic(gump, 0, 0, 39925, 600, 25);
	GFResizePic(gump, 575, 0, 39927, 35, 25);
	GFResizePic(gump, 25, 0, 39926, 541, 25);
	GFResizePic(gump, 25, 475, 39932, 550, 35);
	GFResizePic(gump, 0, 25, 39928, 10, 450);
	GFResizePic(gump, 575, 25, 39930, 35, 450);
	GFResizePic(gump, 575, 475, 39933, 35, 35);
	GFResizePic(gump, 0, 475, 39931, 35, 35);

	// Background
	GFResizePic(gump, 15, 70, INTERNAL_BACKGROUND, 570, 400);

	// Title
	GFTextMid(gump, 80, 35, 520, 53, "Configurar Acoes do Evento");

	var x := 30;
	var y := 90;

	// Instructions
	GFTextLine(gump, x, y, 1149, "Configuracao de Acoes");
	y := y + 3;
	GFGumpPicTiled(gump, x, y, 540, 1, 0x0A52);
	y := y + 10;
	GFTextLine(gump, x, y, 2100, "As acoes abaixo serao executadas em sequencia quando o gatilho ativar:");
	y := y + 30;

	// Action list
	if (!actions || actions.size() == 0)
		// Empty state
		GFResizePic(gump, x, y, 0x2486, 540, 80);  // Subtle background box
		GFTextLine(gump, x + 20, y + 15, 1153, "Nenhuma acao configurada.");
		GFTextLine(gump, x + 20, y + 35, 1153, "Clique em [Adicionar Acao] para criar uma acao.");
		GFTextLine(gump, x + 20, y + 55, 1153, "Ou clique em [Concluir] para usar o sistema de spawn padrao.");
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
			GFAddButton(gump, x + 10, y + 8, 2117, 2118, GF_CLOSE_BTN, BTN_ACTION_EDIT_BASE + action_num);
			// Delete button
			GFAddButton(gump, x + 35, y + 8, 0x9CC, 0x9CC, GF_CLOSE_BTN, BTN_ACTION_DELETE_BASE + action_num);

			// Action text
			GFTextLine(gump, x + 65, y + 8, 53, $"{action_num}. {action_desc}");

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
	GFAddButton(gump, x, y + 10, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_ACTION_ADD);
	GFTextLine(gump, x + 40, y + 10, 53, "[Adicionar Acao]");

	// Done button
	GFAddButton(gump, 280, 435, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_ACTION_DONE);
	GFTextLine(gump, 320, 435, 53, "[Concluir]");

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
			return $"Teleportar para ({action.params.x},{action.params.y},{action.params.z})";
		default:
			return action.type;
	endcase
endfunction

// Helper: Get condition description for display
function GetConditionDescription(condition)
	case (condition.type)
		"PLAYER_HAS_ITEM":
			return $"Jogador tem item {Hex(condition.params.objtype)}";
		"ACTION_SUCCEEDED":
			return $"Acao #{condition.params.action_id} bem-sucedida";
		default:
			return condition.type;
	endcase
endfunction

// Action type selection gump
function BuildActionTypeSelectionGump()
	var gump := GFECreateGump("", 500, 350, array{HIDE_BORDER});

	// Frame
	GFResizePic(gump, 0, 0, 39925, 500, 25);
	GFResizePic(gump, 475, 0, 39927, 35, 25);
	GFResizePic(gump, 25, 0, 39926, 441, 25);
	GFResizePic(gump, 25, 325, 39932, 450, 35);
	GFResizePic(gump, 0, 25, 39928, 10, 300);
	GFResizePic(gump, 475, 25, 39930, 35, 300);
	GFResizePic(gump, 475, 325, 39933, 35, 35);
	GFResizePic(gump, 0, 325, 39931, 35, 35);

	// Background
	GFResizePic(gump, 15, 70, INTERNAL_BACKGROUND, 470, 250);

	// Title
	GFTextMid(gump, 80, 35, 420, 53, "Selecione o Tipo de Acao");

	var x := 40;
	var y := 90;
	var spacing := 70;

	// Instructions
	GFTextLine(gump, x, y, 2100, "Que tipo de acao deseja adicionar?");
	y := y + 30;

	// EMOTE action
	GFAddButton(gump, x, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_ACTION_TYPE_EMOTE);
	GFTextLine(gump, x + 40, y, 53, "Emote/Mensagem");
	GFTextLine(gump, x + 40, y + 20, 1153, "Exibir uma mensagem ou emote para criar atmosfera");
	y := y + spacing;

	// SPAWN action
	GFAddButton(gump, x, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_ACTION_TYPE_SPAWN);
	GFTextLine(gump, x + 40, y, 53, "Spawn de Criaturas");
	GFTextLine(gump, x + 40, y + 20, 1153, "Criar criaturas no local do evento");
	y := y + spacing;

	// TELEPORT action
	GFAddButton(gump, x, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_ACTION_TYPE_TELEPORT);
	GFTextLine(gump, x + 40, y, 53, "Teleporte");
	GFTextLine(gump, x + 40, y + 20, 1153, "Teleportar o jogador para outro local");

	return gump;
endfunction

// Action edit gump - configure action parameters
function BuildActionEditGump(action_type, existing_action := 0)
	var gump := GFECreateGump("", 600, 650, array{HIDE_BORDER});

	// Frame
	GFResizePic(gump, 0, 0, 39925, 600, 25);
	GFResizePic(gump, 575, 0, 39927, 35, 25);
	GFResizePic(gump, 25, 0, 39926, 541, 25);
	GFResizePic(gump, 25, 625, 39932, 550, 35);
	GFResizePic(gump, 0, 25, 39928, 10, 600);
	GFResizePic(gump, 575, 25, 39930, 35, 600);
	GFResizePic(gump, 575, 625, 39933, 35, 35);
	GFResizePic(gump, 0, 625, 39931, 35, 35);

	// Background
	GFResizePic(gump, 15, 70, INTERNAL_BACKGROUND, 570, 550);

	// Title
	var title_text := "Configurar Acao: ";
	case (action_type)
		"EMOTE":
			title_text := title_text + "Emote/Mensagem";
		"SPAWN":
			title_text := title_text + "Spawn de Criaturas";
		"TELEPORT":
			title_text := title_text + "Teleporte";
	endcase
	GFTextMid(gump, 0, 35, 600, 53, title_text);

	var x := 30;
	var y := 90;

	// ==================== SECTION: TIMING ====================
	GFTextMid(gump, 0, y, 600, 1149, "TEMPORIZACAO");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 520, 2);
	y := y + 15;

	GFTextLine(gump, x, y, 1153, "Atraso antes da acao (em segundos, 0 = imediato):");
	y := y + 25;
	var delay_default := "0";
	if (existing_action && existing_action.delay)
		delay_default := CStr(existing_action.delay);
	endif
	GFResizePic(gump, x, y, 9350, 100, 25);
	GFTextEntry(gump, x + 5, y + 5, 90, 20, 1150, delay_default, ID_ACTION_DELAY);
	GFTextLine(gump, x + 120, y + 5, 2100, "Ex: 5 = aguardar 5 segundos");
	y := y + 45;

	// ==================== SECTION: ACTION PARAMETERS ====================
	GFTextMid(gump, 0, y, 600, 1149, "PARAMETROS DA ACAO");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 520, 2);
	y := y + 15;

	case (action_type)
		"EMOTE":
			GFTextLine(gump, x, y, 1153, "Texto do emote/mensagem:");
			y := y + 25;
			var emote_default := "";
			if (existing_action && existing_action.params && existing_action.params.text)
				emote_default := existing_action.params.text;
			endif
			GFResizePic(gump, x, y, 9350, 540, 65);
			GFTextEntry(gump, x + 5, y + 5, 530, 60, 1150, emote_default, ID_ACTION_EMOTE_TEXT);
			y := y + 80;

			GFTextLine(gump, x, y, 1153, "Alcance (em tiles):");
			y := y + 25;
			var range_default := "10";
			if (existing_action && existing_action.params && existing_action.params.range)
				range_default := CStr(existing_action.params.range);
			endif
			GFResizePic(gump, x, y, 9350, 80, 25);
			GFTextEntry(gump, x + 5, y + 5, 70, 20, 1150, range_default, ID_ACTION_EMOTE_RANGE);
			GFTextLine(gump, x + 100, y + 5, 2100, "Distancia maxima para players verem (0 = sem limite)");
			y := y + 40;

			// Checkbox for showing emote on player vs item
			var show_on_player := 0;
			if (existing_action && existing_action.params && existing_action.params.show_on_source)
				show_on_player := existing_action.params.show_on_source;
			endif
			GFAddButton(gump, x, y, (show_on_player ? 0x939 : 0x938), (show_on_player ? 0x938 : 0x939), GF_CLOSE_BTN, BTN_EMOTE_SHOW_ON_PLAYER);
			GFTextLine(gump, x + 30, y + 2, 1153, "Exibir emote acima do jogador (ao inves do item)");
			y := y + 35;

		"SPAWN":
			GFTextLine(gump, x, y, 2100, "O spawn de criaturas usa a configuracao padrao do sistema.");
			GFTextLine(gump, x, y + 20, 2100, "Voce podera configurar criaturas, quantidades e tiers");
			GFTextLine(gump, x, y + 40, 2100, "na proxima etapa do wizard.");
			y := y + 75;

		"TELEPORT":
			GFTextLine(gump, x, y, 1153, "Coordenadas do destino (use .where no jogo):");
			y := y + 25;
			var x_default := "";
			var y_default := "";
			var z_default := "";
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
			endif

			GFTextLine(gump, x, y, 1153, "X:");
			GFResizePic(gump, x + 25, y, 9350, 80, 25);
			GFTextEntry(gump, x + 30, y + 5, 70, 20, 1150, x_default, ID_ACTION_TELEPORT_X);

			GFTextLine(gump, x + 125, y, 1153, "Y:");
			GFResizePic(gump, x + 150, y, 9350, 80, 25);
			GFTextEntry(gump, x + 155, y + 5, 70, 20, 1150, y_default, ID_ACTION_TELEPORT_Y);

			GFTextLine(gump, x + 250, y, 1153, "Z:");
			GFResizePic(gump, x + 275, y, 9350, 80, 25);
			GFTextEntry(gump, x + 280, y + 5, 70, 20, 1150, z_default, ID_ACTION_TELEPORT_Z);
			y := y + 35;

			GFTextLine(gump, x, y, 2100, "Ex: X=1000 Y=2000 Z=0 (altura do terreno)");
			y := y + 35;
	endcase

	// ==================== SECTION: CONDITION ====================
	GFTextMid(gump, 0, y, 600, 1149, "CONDICAO DE EXECUCAO (OPCIONAL)");
	y := y + 20;
	GFResizePic(gump, 40, y, 3000, 520, 2);
	y := y + 15;

	GFTextLine(gump, x, y, 2100, "Quando esta acao deve ser executada?");
	y := y + 25;

	// Determine current condition
	var current_condition := "NONE";
	if (existing_action && existing_action.condition && existing_action.condition.type)
		current_condition := existing_action.condition.type;
	endif

	// Radio button: Always execute
	GFRadioButton(gump, x, y, 0x25F8, 0x25FB, (current_condition == "NONE" ? 1 : 0), RADIO_CONDITION_NONE);
	GFTextLine(gump, x + 30, y + 2, 1153, "Sempre");
	y := y + 30;

	// Radio button: Player has item
	GFRadioButton(gump, x, y, 0x25F8, 0x25FB, (current_condition == "PLAYER_HAS_ITEM" ? 1 : 0), RADIO_CONDITION_HAS_ITEM);
	GFTextLine(gump, x + 30, y + 2, 1153, "Jogador possui item");
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

	GFTextLine(gump, x + 40, y, 2100, "ObjType (hex):");
	GFResizePic(gump, x + 150, y, 9350, 100, 25);
	GFTextEntry(gump, x + 155, y + 5, 90, 20, 1150, objtype_default, ID_CONDITION_OBJTYPE);

	GFTextLine(gump, x + 270, y, 2100, "Qtd:");
	GFResizePic(gump, x + 310, y, 9350, 70, 25);
	GFTextEntry(gump, x + 315, y + 5, 60, 20, 1150, amount_default, ID_CONDITION_AMOUNT);
	y := y + 40;

	// Radio button: Action succeeded
	GFRadioButton(gump, x, y, 0x25F8, 0x25FB, (current_condition == "ACTION_SUCCEEDED" ? 1 : 0), RADIO_CONDITION_ACTION_SUCCEEDED);
	GFTextLine(gump, x + 30, y + 2, 1153, "Acao anterior bem-sucedida");
	y := y + 25;

	// Parameter fields for "Action succeeded" (always shown)
	var action_id_default := "1";
	if (existing_action && existing_action.condition && existing_action.condition.params && existing_action.condition.params.action_id)
		action_id_default := CStr(existing_action.condition.params.action_id);
	endif

	GFTextLine(gump, x + 40, y, 2100, "Numero da acao:");
	GFResizePic(gump, x + 170, y, 9350, 70, 25);
	GFTextEntry(gump, x + 175, y + 5, 60, 20, 1150, action_id_default, ID_CONDITION_ACTION_ID);
	y := y + 40;

	// Divider before button
	GFResizePic(gump, 40, y, 3000, 520, 2);

	// Save button
	GFAddButton(gump, 280, 590, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_NEXT);

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

			var range_str := Trim(GFExtractData(input, ID_ACTION_EMOTE_RANGE));
			action.params.+range := CInt(range_str);
			if (action.params.range < 0)
				action.params.range := 10;
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

			action.params.+x := CInt(x_str);
			action.params.+y := CInt(y_str);
			action.params.+z := CInt(z_str);

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

	elseif (input[RADIO_CONDITION_ACTION_SUCCEEDED])
		// Action succeeded condition
		var action_id_str := Trim(GFExtractData(input, ID_CONDITION_ACTION_ID));

		if (action_id_str && action_id_str != "")
			action.condition := struct{};
			action.condition.+type := "ACTION_SUCCEEDED";
			action.condition.+params := struct{};
			action.condition.params.+action_id := CInt(action_id_str);
			if (action.condition.params.action_id < 1)
				action.condition.params.action_id := 1;
			endif
		else
			// No action ID specified - invalid condition
			action.condition := 0;
		endif
	else
		// RADIO_CONDITION_NONE selected or no radio selected - always execute
		action.condition := 0;
	endif

	return action;
endfunction
