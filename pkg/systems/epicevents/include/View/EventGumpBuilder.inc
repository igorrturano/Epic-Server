use uo;

include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:epicGumps";
include ":epicevents:Model/EventConfig";

const GUMP_BACKGROUND := 39929;
const INTERNAL_BACKGROUND := 9350;
const GUMP_WIDTH := 540;
const GUMP_HEIGHT := 650;

const BTN_OK := 200;

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
	GFTextMid(gump, 80, 35, 420, 53, "Epic Events - Configuracao");

	var x_label := 30;
	var x_field := 100;
	var y := 80;
	var spacing := 80;

	// Creature 1
	GFTextLine(gump, x_label, y, 2100, "Nome:");
	GFEAddTextBox(gump, x_field, y, 150, 20, COLOR_YELLOW, "", ID_NAME1);
	GFTextLine(gump, x_field + 160, y, 2100, "Quantidade:");
	GFEAddTextBox(gump, x_field + 250, y, 50, 20, COLOR_YELLOW, "", ID_AMOUNT1);
	GFTextLine(gump, x_field + 310, y, 2100, "Tier:");
	GFEAddTextBox(gump, x_field + 350, y, 50, 20, COLOR_YELLOW, "", ID_TIER1);
	y := y + 25;
	GFTextLine(gump, x_label, y, 2100, "Template:");
	GFEAddTextBox(gump, x_field, y, 400, 20, COLOR_YELLOW, "", ID_TEMPLATE1);
	y := y + spacing;

	// Creature 2
	GFTextLine(gump, x_label, y, 2100, "Nome:");
	GFEAddTextBox(gump, x_field, y, 150, 20, COLOR_YELLOW, "", ID_NAME2);
	GFTextLine(gump, x_field + 160, y, 2100, "Quantidade:");
	GFEAddTextBox(gump, x_field + 250, y, 50, 20, COLOR_YELLOW, "", ID_AMOUNT2);
	GFTextLine(gump, x_field + 310, y, 2100, "Tier:");
	GFEAddTextBox(gump, x_field + 350, y, 50, 20, COLOR_YELLOW, "", ID_TIER2);
	y := y + 25;
	GFTextLine(gump, x_label, y, 2100, "Template:");
	GFEAddTextBox(gump, x_field, y, 400, 20, COLOR_YELLOW, "", ID_TEMPLATE2);
	y := y + spacing;

	// Creature 3
	GFTextLine(gump, x_label, y, 2100, "Nome:");
	GFEAddTextBox(gump, x_field, y, 150, 20, COLOR_YELLOW, "", ID_NAME3);
	GFTextLine(gump, x_field + 160, y, 2100, "Quantidade:");
	GFEAddTextBox(gump, x_field + 250, y, 50, 20, COLOR_YELLOW, "", ID_AMOUNT3);
	GFTextLine(gump, x_field + 310, y, 2100, "Tier:");
	GFEAddTextBox(gump, x_field + 350, y, 50, 20, COLOR_YELLOW, "", ID_TIER3);
	y := y + 25;
	GFTextLine(gump, x_label, y, 2100, "Template:");
	GFEAddTextBox(gump, x_field, y, 400, 20, COLOR_YELLOW, "", ID_TEMPLATE3);
	y := y + spacing;

	// Shared settings
	GFTextLine(gump, x_label, y, 2100, "Cor (opcional):");
	GFEAddTextBox(gump, x_field + 80, y, 100, 20, COLOR_YELLOW, "", ID_COLOR);
	y := y + 30;

	GFTextLine(gump, x_label, y, 2100, "Emote (opcional):");
	GFEAddTextBox(gump, x_label, y + 25, 470, 20, COLOR_YELLOW, "", ID_EMOTE);
	y := y + 55;

	GFTextLine(gump, x_label, y, 2100, "Delay (em minutos):");
	y := y + 20;
	GFTextLine(gump, x_label, y, 2100, "Minimo:");
	GFEAddTextBox(gump, x_field, y, 100, 20, COLOR_YELLOW, "1", ID_MIN_DELAY);
	GFTextLine(gump, x_field + 120, y, 2100, "Maximo:");
	GFEAddTextBox(gump, x_field + 200, y, 100, 20, COLOR_YELLOW, "5", ID_MAX_DELAY);
	y := y + 40;

	// OK Button
	GFAddButton(gump, 260, y, 0x81C, 0x81B, GF_CLOSE_BTN, BTN_OK);

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
