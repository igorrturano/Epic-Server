use uo;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:epicGumps";
include ":gumps:dropDownGump";
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
const ID_TRIGGER_ITEMS_LIST := 130;
const BTN_TRIGGER_TARGET_ITEM := 132;

// Action management buttons
const BTN_ACTION_ADD := 220;
const BTN_ACTION_EDIT_BASE := 230;  // 230-239 for editing actions 1-10
const BTN_ACTION_DELETE_BASE := 240;  // 240-249 for deleting actions 1-10
const BTN_ACTION_DONE := 250;
const BTN_ACTION_CONDITION := 251;

// Action type buttons
const BTN_ACTION_TYPE_EMOTE := 260;
const BTN_ACTION_TYPE_SPAWN := 261;
const BTN_ACTION_TYPE_TELEPORT := 262;
const BTN_ACTION_TYPE_CONVERSATION := 263;

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

// Nature condition checkboxes (separate gump)
const CHK_MOON_KALENDAE := 400;
const CHK_MOON_NONAE := 401;
const CHK_MOON_IDUS := 402;
const CHK_MOON_DECRESCENS := 403;

const RADIO_DAYNIGHT_ANY := 410;
const RADIO_DAYNIGHT_DIA := 411;
const RADIO_DAYNIGHT_NOITE := 412;

const CHK_SEASON_SUMMER := 420;
const CHK_SEASON_AUTUMN := 421;
const CHK_SEASON_WINTER := 422;
const CHK_SEASON_SPRING := 423;

const ID_COND_TIME_MIN := 430;
const ID_COND_TIME_MAX := 431;

const BTN_CONDITION_SKIP := 440;
const BTN_CONDITION_SAVE := 441;

// HAS_ITEM condition fields
const CHK_COND_HAS_ITEM := 450;
const ID_COND_ITEM_OBJTYPE := 451;
const ID_COND_ITEM_AMOUNT := 452;
const BTN_COND_TARGET_ITEM := 453;
const CHCK_COND_ITEM_CONSUME := 454;

// CHARACTER condition fields
const CHK_COND_CHARACTER := 460;

// CHARACTER condition - Dropdown configuration
// GerenciadorDropDown params: maxOpcoes, adicionalIdDropDown, adicionalIdOpcoes, maxDropDown
const COND_DD_MAX_OPCOES := 10;
const COND_DD_ID_DROPDOWN := 1460;
const COND_DD_ID_OPCOES := 1500;
const COND_DD_MAX_DROPDOWN := 3;
// Dropdown button IDs (must match adicionalIdDropDown + position)
const BTN_COND_RACE_DROPDOWN := 1461;
const BTN_COND_CULTURE_DROPDOWN := 1462;
const BTN_COND_RELIGION_DROPDOWN := 1463;

// Event delay fields
const ID_EVENT_MIN_DELAY := 340;
const ID_EVENT_MAX_DELAY := 341;
const CHK_ONE_TIME_ONLY := 342;

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
function BuildCreatureRow(byref gump, x, y, ids, existing_creature := 0)
	var name_default := "";
	var amount_default := "";
	var tier_default := "";
	var template_default := "";

	if (existing_creature)
		if (existing_creature.name)
			name_default := CStr(existing_creature.name);
		endif
		if (existing_creature.amount)
			amount_default := CStr(existing_creature.amount);
		endif
		if (existing_creature.tier)
			tier_default := CStr(existing_creature.tier);
		endif
		if (existing_creature.template)
			template_default := CStr(existing_creature.template);
		endif
	endif

	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Nome:");
	GFResizePic(gump, x + 100, y, 9350, 150, 25);
	GFTextEntry(gump, x + 105, y + 5, 140, 20, GUMP_COLOR::TEXT_ENTRY, name_default, ids.name);

	GFTextLine(gump, x + 265, y, GUMP_COLOR::HELP_TEXT, "Qtd:");
	GFResizePic(gump, x + 310, y, 9350, 60, 25);
	GFTextEntry(gump, x + 315, y + 5, 50, 20, GUMP_COLOR::TEXT_ENTRY, amount_default, ids.amount);

	GFTextLine(gump, x + 385, y, GUMP_COLOR::HELP_TEXT, "Tier:");
	GFResizePic(gump, x + 425, y, 9350, 60, 25);
	GFTextEntry(gump, x + 430, y + 5, 50, 20, GUMP_COLOR::TEXT_ENTRY, tier_default, ids.tier);
	y := y + 30;

	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Template:");
	GFResizePic(gump, x + 100, y, 9350, 385, 25);
	GFTextEntry(gump, x + 105, y + 5, 375, 20, GUMP_COLOR::TEXT_ENTRY, template_default, ids.template);

	return y;
endfunction

function BuildEventConfigGump(existing_config := 0)
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
		var existing_creature := 0;
		if (existing_config && existing_config.creatures && i < existing_config.creatures.size())
			existing_creature := existing_config.creatures[i + 1];  // POL arrays are 1-indexed
		endif
		y := BuildCreatureRow(gump, x, y, ids, existing_creature);
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

	// Populate with existing values or defaults
	var color_default := "";
	var emote_default := "";
	if (existing_config)
		if (existing_config.color)
			color_default := CStr(existing_config.color);
		endif
		if (existing_config.emote)
			emote_default := CStr(existing_config.emote);
		endif
	endif

	// Shared settings
	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Cor (opcional):");
	GFResizePic(gump, x + 150, y, 9350, 100, 25);
	GFTextEntry(gump, x + 155, y + 5, 90, 20, GUMP_COLOR::TEXT_ENTRY, color_default, ID_COLOR);
	y := y + 35;

	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Emote (opcional):");
	y := y + 25;
	GFResizePic(gump, x, y, 9350, 480, 25);
	GFTextEntry(gump, x + 5, y + 5, 470, 20, GUMP_COLOR::TEXT_ENTRY, emote_default, ID_EMOTE);
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

	// ITEM_GIVEN trigger
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_TRIGGER_ITEM_GIVEN);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Entrega de Item (Oferenda)");
	GFTextLine(gump, x + 40, y + 22, GUMP_COLOR::LABEL, "Jogador entrega um item especifico ao NPC");
	y := y + spacing;

	// DOUBLECLICKED trigger
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_TRIGGER_DOUBLECLICKED);
	GFTextLine(gump, x + 40, y + 2, GUMP_COLOR::HIGHLIGHT, "Clique Duplo (Interacao)");
	GFTextLine(gump, x + 40, y + 22, GUMP_COLOR::LABEL, "Jogador clica duas vezes no NPC/item");

	return gump;
endfunction

// Trigger configuration gump (for triggers that need extra params)
function BuildTriggerConfigGump(trigger_type, existing_params := 0)
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
		"ITEM_GIVEN":
			var items_default := "";
			if (existing_params && existing_params.items_list)
				items_default := existing_params.items_list;
			endif

			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Itens aceitos para ativacao:");
			y := y + 25;
			GFResizePic(gump, x, y, 9350, 420, 25);
			GFTextEntry(gump, x + 5, y + 5, 410, 20, GUMP_COLOR::TEXT_ENTRY, items_default, ID_TRIGGER_ITEMS_LIST);
			GFAddButton(gump, x + 430, y + 2, 2152, 2153, GF_CLOSE_BTN, BTN_TRIGGER_TARGET_ITEM);
			GFTextLine(gump, x + 470, y + 2, GUMP_COLOR::HIGHLIGHT, "Target");
			y := y + 35;
			GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Formato: 0xOBJTYPE:QTD separados por virgula");
			GFTextLine(gump, x, y + 15, GUMP_COLOR::LABEL, "Exemplo: 0x1BF2:1, 0x0EED:5");
			GFTextLine(gump, x, y + 30, GUMP_COLOR::LABEL, "Qualquer um destes itens ativa o evento (logica OU).");
			y := y + 60;
			GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Nota: Este gatilho requer um NPC como alvo.");
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
	y := y + 45;

	GFCheckBox(gump, x, y, 0x938, 0x939, 0, CHK_ONE_TIME_ONLY);
	GFTextLine(gump, x + 25, y + 3, GUMP_COLOR::HIGHLIGHT, "Uso unico (destruir evento apos ativacao)");

	// Divider before button
	y := y + 35;
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
		"ITEM_GIVEN":
			var items_str := Trim(GFExtractData(input, ID_TRIGGER_ITEMS_LIST));
			if (!items_str || items_str == "")
				return 0;
			endif
			params.+items_list := items_str;
			params.+required_items := ParseItemsList(items_str);
			if (!params.required_items || params.required_items.size() == 0)
				return 0;
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
	result.+one_time_only := input[CHK_ONE_TIME_ONLY] ? 1 : 0;

	return result;
endfunction

// Parse items list string "0x1234:1, 0x5678:3" into array of struct{ objtype, amount }
function ParseItemsList(items_str)
	var result := array{};
	var entries := SplitWords(items_str, ",");

	foreach entry in entries
		entry := Trim(entry);
		if (entry == "")
			continue;
		endif

		var parts := SplitWords(entry, ":");
		if (parts.size() < 1)
			continue;
		endif

		var objtype_str := Trim(parts[1]);
		var amount := 1;
		if (parts.size() >= 2)
			amount := CInt(Trim(parts[2]));
			if (amount < 1)
				amount := 1;
			endif
		endif

		var objtype := CInt(objtype_str);
		if (objtype > 0)
			var item_entry := struct{};
			item_entry.+objtype := objtype;
			item_entry.+amount := amount;
			result.append(item_entry);
		endif
	endforeach

	return result;
endfunction

// Format items array back to display string
function FormatItemsList(required_items)
	var parts := array{};
	foreach item in required_items
		parts.append($"{Hex(item.objtype)}:{item.amount}");
	endforeach
	return ", ".join(parts);
endfunction

// Condition configuration gump - unified conditions (HAS_ITEM + NATURE)
function BuildConditionGump(existing_condition := 0, gerenciador := 0)
	var gump := GFECreateGump("Condicoes de Ativacao", 500, 800);
	var y := SetupGumpFrame(gump, 500, 800);

	var x := 40;

	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Quando este evento pode ser ativado?");
	GFTextLine(gump, x, y + 20, GUMP_COLOR::HELP_TEXT, "(Deixe vazio para ativar sempre)");
	y := y + 50;

	// Get existing params
	var moon_phases := array{};
	var day_night := "";
	var seasons := array{};
	var time_min := "";
	var time_max := "";
	var has_item_enabled := 0;
	var has_item_objtype := "";
	var has_item_amount := "1";
	var should_consume_item := 0;

	var character_enabled := 0;
	var character_race := "";
	var character_culture := "";
	var character_religion := "";

	if (existing_condition && existing_condition.params)
		if (existing_condition.params.moon_phases)
			moon_phases := existing_condition.params.moon_phases;
		endif
		if (existing_condition.params.day_night)
			day_night := existing_condition.params.day_night;
		endif
		if (existing_condition.params.seasons)
			seasons := existing_condition.params.seasons;
		endif
		if (existing_condition.params.time_min)
			time_min := CStr(existing_condition.params.time_min);
		endif
		if (existing_condition.params.time_max)
			time_max := CStr(existing_condition.params.time_max);
		endif
		if (existing_condition.params.has_item_objtype)
			has_item_enabled := 1;
			has_item_objtype := Hex(existing_condition.params.has_item_objtype);
			if (existing_condition.params.has_item_amount)
				has_item_amount := CStr(existing_condition.params.has_item_amount);
			endif
			if (existing_condition.params.should_consume_item)
				should_consume_item := 1;
			endif
		endif
		if (existing_condition.params.character_race || existing_condition.params.character_culture || existing_condition.params.character_religion)
			character_enabled := 1;
			character_race := existing_condition.params.character_race ? existing_condition.params.character_race : "";
			character_culture := existing_condition.params.character_culture ? existing_condition.params.character_culture : "";
			character_religion := existing_condition.params.character_religion ? existing_condition.params.character_religion : "";
		endif
	endif

	// ==================== SECTION: HAS ITEM ====================
	GFTextMid(gump, 0, y, 500, GUMP_COLOR::SECTION_HEADER, "JOGADOR TEM ITEM");
	y := y + 20;
	GFResizePic(gump, 30, y, 3000, 440, 2);
	y := y + 15;

	GFCheckBox(gump, x, y, 0x938, 0x939, has_item_enabled, CHK_COND_HAS_ITEM);
	GFTextLine(gump, x + 25, y + 3, GUMP_COLOR::LABEL, "Exigir item:");

	GFTextLine(gump, x + 130, y + 3, GUMP_COLOR::HELP_TEXT, "ObjType:");
	GFResizePic(gump, x + 190, y, 9350, 80, 25);
	GFTextEntry(gump, x + 195, y + 5, 70, 20, GUMP_COLOR::TEXT_ENTRY, has_item_objtype, ID_COND_ITEM_OBJTYPE);

	GFAddButton(gump, x + 280, y, 2152, 2153, GF_CLOSE_BTN, BTN_COND_TARGET_ITEM);
	GFTextLine(gump, x + 320, y + 3, 53, "[Target]");
	y := y + 22;
	GFTextLine(gump, x + 25, y, 38, "(Obs: Target reseta opcoes - use primeiro!)");
	y := y + 22;

	GFTextLine(gump, x + 130, y + 3, GUMP_COLOR::HELP_TEXT, "Quantidade:");
	GFResizePic(gump, x + 220, y, 9350, 50, 25);
	GFTextEntry(gump, x + 225, y + 5, 40, 20, GUMP_COLOR::TEXT_ENTRY, has_item_amount, ID_COND_ITEM_AMOUNT);
	y := y + 40;

	GFTextLine(gump, x + 130, y + 3, GUMP_COLOR::HELP_TEXT, "Consumir item:");
	GFCheckBox(gump, x + 220, y, 0x938, 0x939, should_consume_item, CHCK_COND_ITEM_CONSUME);
	y := y + 40;

	// ==================== SECTION: CHARACTER ====================
	GFTextMid(gump, 0, y, 500, GUMP_COLOR::SECTION_HEADER, "PERSONAGEM (RACA/CULTURA/RELIGIAO)");
	y := y + 20;
	GFResizePic(gump, 30, y, 3000, 440, 2);
	y := y + 15;

	GFCheckBox(gump, x, y, 0x938, 0x939, character_enabled, CHK_COND_CHARACTER);
	GFTextLine(gump, x + 25, y + 3, GUMP_COLOR::LABEL, "Verificar personagem:");
	y := y + 28;

	// Race dropdown
	var race_display := character_race != "" ? character_race : "Qualquer";
	GFTextLine(gump, x + 10, y, GUMP_COLOR::HIGHLIGHT, "Raca:");
	GFAddButton(gump, x + 80, y, 2117, 2118, GF_CLOSE_BTN, BTN_COND_RACE_DROPDOWN);
	GFTextLine(gump, x + 105, y + 2, 1153, race_display);
	if (gerenciador)
		var dd_race := gerenciador.BuscaDropDownPorPosicao(1);
		if (dd_race)
			dd_race.AtribuiX(x + 80);
			dd_race.AtribuiY(y + 22);
		endif
	endif
	y := y + 28;

	// Culture dropdown
	var culture_display := character_culture != "" ? character_culture : "Qualquer";
	GFTextLine(gump, x + 10, y, GUMP_COLOR::HIGHLIGHT, "Cultura:");
	GFAddButton(gump, x + 80, y, 2117, 2118, GF_CLOSE_BTN, BTN_COND_CULTURE_DROPDOWN);
	GFTextLine(gump, x + 105, y + 2, 1153, culture_display);
	if (gerenciador)
		var dd_culture := gerenciador.BuscaDropDownPorPosicao(2);
		if (dd_culture)
			dd_culture.AtribuiX(x + 80);
			dd_culture.AtribuiY(y + 22);
		endif
	endif
	y := y + 28;

	// Religion dropdown
	var religion_display := character_religion != "" ? character_religion : "Qualquer";
	GFTextLine(gump, x + 10, y, GUMP_COLOR::HIGHLIGHT, "Religiao:");
	GFAddButton(gump, x + 80, y, 2117, 2118, GF_CLOSE_BTN, BTN_COND_RELIGION_DROPDOWN);
	GFTextLine(gump, x + 105, y + 2, 1153, religion_display);
	if (gerenciador)
		var dd_religion := gerenciador.BuscaDropDownPorPosicao(3);
		if (dd_religion)
			dd_religion.AtribuiX(x + 80);
			dd_religion.AtribuiY(y + 22);
		endif
	endif
	y := y + 28;

	GFTextLine(gump, x + 10, y, GUMP_COLOR::HELP_TEXT, "Logica E (todos devem bater). Dropdown = Qualquer para ignorar.");
	y := y + 30;

	// ==================== SECTION: NATURE ====================
	GFTextMid(gump, 0, y, 500, GUMP_COLOR::SECTION_HEADER, "CONDICOES DE NATUREZA");
	y := y + 20;
	GFResizePic(gump, 30, y, 3000, 440, 2);
	y := y + 15;

	// Fase Lunar (checkboxes - multipla selecao)
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Fase Lunar:");
	y := y + 22;
	GFCheckBox(gump, x, y, 0x938, 0x939, ("Kalendae" in moon_phases), CHK_MOON_KALENDAE);
	GFTextLine(gump, x + 25, y + 3, 1153, "Nova");
	GFCheckBox(gump, x + 90, y, 0x938, 0x939, ("Nonae" in moon_phases), CHK_MOON_NONAE);
	GFTextLine(gump, x + 115, y + 3, 1153, "Crescente");
	GFCheckBox(gump, x + 210, y, 0x938, 0x939, ("Idus" in moon_phases), CHK_MOON_IDUS);
	GFTextLine(gump, x + 235, y + 3, 1153, "Cheia");
	GFCheckBox(gump, x + 305, y, 0x938, 0x939, ("Decrescens" in moon_phases), CHK_MOON_DECRESCENS);
	GFTextLine(gump, x + 330, y + 3, 1153, "Minguante");
	y := y + 35;

	// Dia/Noite (radio - exclusivo)
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Periodo:");
	y := y + 22;
	GFSetRadioGroup(gump, 1);
	GFRadioButton(gump, x, y, 0x25F8, 0x25FB, (day_night == ""), RADIO_DAYNIGHT_ANY);
	GFTextLine(gump, x + 25, y + 3, 1153, "Qualquer");
	GFRadioButton(gump, x + 110, y, 0x25F8, 0x25FB, (day_night == "dia"), RADIO_DAYNIGHT_DIA);
	GFTextLine(gump, x + 135, y + 3, 1153, "Dia");
	GFRadioButton(gump, x + 190, y, 0x25F8, 0x25FB, (day_night == "noite"), RADIO_DAYNIGHT_NOITE);
	GFTextLine(gump, x + 215, y + 3, 1153, "Noite");
	y := y + 35;

	// Estacao (checkboxes - multipla selecao)
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Estacao:");
	y := y + 22;
	GFCheckBox(gump, x, y, 0x938, 0x939, (1 in seasons), CHK_SEASON_SUMMER);
	GFTextLine(gump, x + 25, y + 3, 1153, "Verao");
	GFCheckBox(gump, x + 95, y, 0x938, 0x939, (2 in seasons), CHK_SEASON_AUTUMN);
	GFTextLine(gump, x + 120, y + 3, 1153, "Outono");
	GFCheckBox(gump, x + 195, y, 0x938, 0x939, (4 in seasons), CHK_SEASON_WINTER);
	GFTextLine(gump, x + 220, y + 3, 1153, "Inverno");
	GFCheckBox(gump, x + 295, y, 0x938, 0x939, (5 in seasons), CHK_SEASON_SPRING);
	GFTextLine(gump, x + 320, y + 3, 1153, "Primavera");
	y := y + 35;

	// Hora
	GFTextLine(gump, x, y, GUMP_COLOR::LABEL, "Horario (0-23):");
	y := y + 22;
	GFTextLine(gump, x, y, GUMP_COLOR::HELP_TEXT, "Das:");
	GFResizePic(gump, x + 35, y, 9350, 50, 25);
	GFTextEntry(gump, x + 40, y + 5, 40, 20, GUMP_COLOR::TEXT_ENTRY, time_min, ID_COND_TIME_MIN);
	GFTextLine(gump, x + 100, y, GUMP_COLOR::HELP_TEXT, "ate:");
	GFResizePic(gump, x + 130, y, 9350, 50, 25);
	GFTextEntry(gump, x + 135, y + 5, 40, 20, GUMP_COLOR::TEXT_ENTRY, time_max, ID_COND_TIME_MAX);
	GFTextLine(gump, x + 200, y, 1153, "(vazio = qualquer)");
	y := y + 45;

	// Botoes
	GFAddButton(gump, 100, y, 2152, 2153, GF_CLOSE_BTN, BTN_CONDITION_SKIP);
	GFTextLine(gump, 140, y + 5, GUMP_COLOR::LABEL, "Limpar");
	GFAddButton(gump, 280, y, 2152, 2153, GF_CLOSE_BTN, BTN_CONDITION_SAVE);
	GFTextLine(gump, 320, y + 5, GUMP_COLOR::LABEL, "Salvar");

	return gump;
endfunction

// Parse condition gump input
function ParseConditionInput(input)
	if (!input || input[BTN_CONDITION_SKIP])
		return 0;  // Sem condicao
	endif

	// Check for target item button
	if (input[BTN_COND_TARGET_ITEM])
		return struct{ action := "target_item" };
	endif

	var params := struct{};

	// ===== HAS_ITEM section =====
	params.+has_item_objtype := 0;
	params.+has_item_amount := 0;
	params.+should_consume_item := 0;

	if (input[CHK_COND_HAS_ITEM])
		var objtype_str := Trim(GFExtractData(input, ID_COND_ITEM_OBJTYPE));
		var amount_str := Trim(GFExtractData(input, ID_COND_ITEM_AMOUNT));

		if (objtype_str && objtype_str != "")
			// Handle hex (0x1234) or decimal
			if (SubStr(objtype_str, 1, 2) == "0x")
				params.has_item_objtype := CInt(objtype_str);
			else
				params.has_item_objtype := CInt(objtype_str);
			endif
		endif

		params.has_item_amount := (amount_str && amount_str != "") ? CInt(amount_str) : 1;
		params.should_consume_item := input[CHCK_COND_ITEM_CONSUME] ? 1 : 0;
	endif

	// ===== NATURE section =====
	// Moon phases (array de selecionados)
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

	// Day/Night (unico)
	if (input[RADIO_DAYNIGHT_DIA])
		params.+day_night := "dia";
	elseif (input[RADIO_DAYNIGHT_NOITE])
		params.+day_night := "noite";
	else
		params.+day_night := "";
	endif

	// Seasons (array)
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

	// Time range
	var time_min_str := Trim(GFExtractData(input, ID_COND_TIME_MIN));
	var time_max_str := Trim(GFExtractData(input, ID_COND_TIME_MAX));
	params.+time_min := (time_min_str != "") ? CInt(time_min_str) : 0;
	params.+time_max := (time_max_str != "") ? CInt(time_max_str) : 0;

	// ===== CHARACTER section =====
	// Character values are handled via dropdowns in HandleConditionEdit
	// and merged into the result there. Initialize empty here.
	params.+character_race := "";
	params.+character_culture := "";
	params.+character_religion := "";

	// Check if any condition is configured
	var has_item_configured := (params.has_item_objtype > 0);
	var has_nature_configured := (params.moon_phases.size() > 0 || params.day_night != "" ||
	                              params.seasons.size() > 0 || params.time_min || params.time_max);
	// Character is configured via dropdown (values merged in HandleConditionEdit)
	// Check the checkbox to know if user wants character conditions
	var has_character_configured := input[CHK_COND_CHARACTER] ? 1 : 0;

	if (!has_item_configured && !has_nature_configured && !has_character_configured)
		return 0;
	endif

	return struct{ type := "CONDITION", params := params };
endfunction

// Helper: Setup character dropdowns (race, culture, religion) for condition gump
function SetupCharacterDropdowns(gerenciador)
	// Race dropdown (position 1)
	var dd_race := gerenciador.ObtemDropDown(1, 250);
	var race_options := dictionary{
		"Qualquer" -> "",
		"Anão" -> "Anão",
		"Drow" -> "Drow",
		"Elfo" -> "Elfo",
		"Humano" -> "Humano",
		"Orc" -> "Orc",
		"Pequilho" -> "Pequilho"
	};
	dd_race.AdicionaOpcoes(race_options);

	// Culture dropdown (position 2)
	var dd_culture := gerenciador.ObtemDropDown(2, 250);
	var culture_options := dictionary{
		"Qualquer" -> "",
		"Aiglana" -> "Aiglana",
		"Alüriel" -> "Alüriel",
		"Badûran" -> "Badûran",
		"Björska" -> "Björska",
		"Gulthrak" -> "Gulthrak",
		"Har'oloth" -> "Har'oloth",
		"Colhesiana" -> "Colhesiana",
		"Povos do Leste" -> "Povos do Leste"
	};
	dd_culture.AdicionaOpcoes(culture_options);

	// Religion dropdown (position 3)
	var dd_religion := gerenciador.ObtemDropDown(3, 250);
	var religion_options := dictionary{
		"Qualquer" -> "",
		"Bokk Bharaz" -> "Bokk Bharaz",
		"Céticos" -> "Céticos",
		"Eluith'orth" -> "Eluith'orth",
		"Halleuad" -> "Halleuad",
		"Kai" -> "Kai",
		"Lórё" -> "Lórё",
		"Ushkr'ar" -> "Ushkr'ar",
		"Veneratio" -> "Veneratio"
	};
	dd_religion.AdicionaOpcoes(religion_options);
endfunction

// Action list gump - shows all configured actions
function BuildActionListGump(actions, event_condition := 0)
	var gump := GFECreateGump("Configurar Acoes do Evento", 600, 550);
	var y := SetupGumpFrame(gump, 600, 550, 0);

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
		var num_actions := actions.size();
		if (num_actions > 10)
			num_actions := 10;  // Max 10 actions
		endif

		for action_num := 1 to num_actions
			var action := actions[action_num];

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
		endfor
	endif

	// Add action button
	GFAddButton(gump, x, y + 5, 2152, 2153, GF_CLOSE_BTN, BTN_ACTION_ADD);
	GFTextLine(gump, x + 40, y + 10, GUMP_COLOR::HIGHLIGHT, "[Adicionar Acao]");
	y := y + 35;

	// Condition section
	GFResizePic(gump, x, y, 3000, 540, 2);
	y := y + 15;

	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_ACTION_CONDITION);
	if (event_condition && event_condition.type && event_condition.type != "NONE")
		var cond_desc := GetConditionDescription(event_condition);
		GFTextLine(gump, x + 40, y + 5, GUMP_COLOR::HIGHLIGHT, "[Configurar Condicao]");
		GFTextLine(gump, x + 220, y + 5, 53, cond_desc);
	else
		GFTextLine(gump, x + 40, y + 5, GUMP_COLOR::HIGHLIGHT, "[Configurar Condicao]");
		GFTextLine(gump, x + 220, y + 5, GUMP_COLOR::HELP_TEXT, "(Nenhuma - sempre ativo)");
	endif

	// Done button (final finish button - OKAY style)
	GFAddButton(gump, 280, 485, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_ACTION_DONE);

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
		"CONVERSATION":
			return "Conversa (Dialogo)";
		default:
			return action.type;
	endcase
endfunction

// Helper: Get condition description for display
function GetConditionDescription(condition)
	case (condition.type)
		"PLAYER_HAS_ITEM":
			return $"Item: {Hex(condition.params.objtype)}";

		"NATURE":
		"CONDITION":
			var parts := array{};

			// HAS_ITEM
			if (condition.params.has_item_objtype && condition.params.has_item_objtype > 0)
				var amount := condition.params.has_item_amount ? condition.params.has_item_amount : 1;
				parts.append($"Item: {Hex(condition.params.has_item_objtype)} x{amount}");
			endif

			// Moon phases (array)
			if (condition.params.moon_phases && condition.params.moon_phases.size() > 0)
				var moon_str := "Lua: ";
				for i := 1 to condition.params.moon_phases.size()
					moon_str := moon_str + condition.params.moon_phases[i];
					if (i < condition.params.moon_phases.size())
						moon_str := moon_str + "/";
					endif
				endfor
				parts.append(moon_str);
			endif

			// Day/Night
			if (condition.params.day_night && condition.params.day_night != "")
				parts.append(condition.params.day_night == "dia" ? "Dia" : "Noite");
			endif

			// Seasons (array)
			if (condition.params.seasons && condition.params.seasons.size() > 0)
				var season_names := array{};
				foreach s in (condition.params.seasons)
					case (CInt(s))
						1: season_names.append("Verao");
						2: season_names.append("Outono");
						4: season_names.append("Inverno");
						5: season_names.append("Primavera");
					endcase
				endforeach
				var season_str := "";
				for i := 1 to season_names.size()
					season_str := season_str + season_names[i];
					if (i < season_names.size())
						season_str := season_str + "/";
					endif
				endfor
				parts.append(season_str);
			endif

			// Time
			if (condition.params.time_min || condition.params.time_max)
				parts.append($"{condition.params.time_min}h-{condition.params.time_max}h");
			endif

			if (parts.size() == 0)
				return "(nenhum filtro)";
			endif

			var result := "";
			for i := 1 to parts.size()
				result := result + parts[i];
				if (i < parts.size())
					result := result + ", ";
				endif
			endfor
			return result;

		default:
			return condition.type;
	endcase
endfunction

// Action type selection gump
function BuildActionTypeSelectionGump()
	var gump := GFECreateGump("Selecione o Tipo de Acao", 500, 420);
	var y := SetupGumpFrame(gump, 500, 420);

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
	y := y + spacing;

	// CONVERSATION action
	GFAddButton(gump, x, y, 2152, 2153, GF_CLOSE_BTN, BTN_ACTION_TYPE_CONVERSATION);
	GFTextLine(gump, x + 40, y, GUMP_COLOR::HIGHLIGHT, "Conversa (Dialogo)");
	GFTextLine(gump, x + 40, y + 20, GUMP_COLOR::HELP_TEXT, "Arvore de dialogo interativa com o NPC");

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
		"CONVERSATION":
			title_text := title_text + "Conversa (Dialogo)";
	endcase

	var gump := GFECreateGump(title_text, 600, 500);
	var y := SetupGumpFrame(gump, 600, 500, 0);  // No divider, we'll add section headers

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
				// Use TypeOf to check if coordinate exists (0 is valid coordinate)
				if (TypeOf(existing_action.params.x) != "Error")
					x_default := CStr(existing_action.params.x);
				endif
				if (TypeOf(existing_action.params.y) != "Error")
					y_default := CStr(existing_action.params.y);
				endif
				if (TypeOf(existing_action.params.z) != "Error")
					z_default := CStr(existing_action.params.z);
				endif
				if (existing_action.params.realm)
					realm_default := CStr(existing_action.params.realm);
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
	elseif (input[BTN_ACTION_CONDITION])
		return struct{ action := "condition" };
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
	elseif (input[BTN_ACTION_TYPE_CONVERSATION])
		return "CONVERSATION";
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
	case (action_type)
		"EMOTE":
			var emote_text := Trim(GFExtractData(input, ID_ACTION_EMOTE_TEXT));
			if (!emote_text || emote_text == "")
				return 0;  // Text required
			endif
			var show_on_source := input[BTN_EMOTE_SHOW_ON_PLAYER] ? 1 : 0;
			action.params := struct{ text := emote_text, show_on_source := show_on_source };

		"SPAWN":
			// SPAWN params will be filled in later from spawn config gump
			action.params := struct{ use_spawn_config := 1 };

		"TELEPORT":
			var x_str := Trim(GFExtractData(input, ID_ACTION_TELEPORT_X));
			var y_str := Trim(GFExtractData(input, ID_ACTION_TELEPORT_Y));
			var z_str := Trim(GFExtractData(input, ID_ACTION_TELEPORT_Z));
			var realm_str := Trim(GFExtractData(input, ID_ACTION_TELEPORT_REALM));

			var tp_x := CInt(x_str);
			var tp_y := CInt(y_str);
			var tp_z := CInt(z_str);

			if (!tp_x || !tp_y)
				return 0;  // X and Y required
			endif

			action.params := struct{ x := tp_x, y := tp_y, z := tp_z, realm := realm_str };
	endcase

	return action;
endfunction
