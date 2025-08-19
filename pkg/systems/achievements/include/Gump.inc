include ":gumps:epicGumps";
include ":gumps:model/Gump";
include ":gumps:model/ControlText";
include ":gumps:model/ControlButton";
include ":achievements:model/AchievementCombat";

const MAX_XP := 5000; // XP máximo para a barra de progresso

function ShowAchievementsGump(who)
	var gump := Gump(who, "Progresso de Conquistas", 680, 400);

	var text1 := ControlText("Label1", 45, 75, "Exemplo de texto", GUMP_TEXT_COLOR_RED);
	var button1 := ControlButton("Button1", 30, 80, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_UNPRESSED, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_PRESSED, GF_CLOSE_BTN);

	var text2 := ControlText("Label2", 45, 100, "Exemplo de texto 2", GUMP_TEXT_COLOR_RED);
	var button2 := ControlButton("Button2", 30, 105, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_UNPRESSED, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_PRESSED, GF_CLOSE_BTN);

	gump.AddControl(text1);
	gump.AddControl(text2);
	gump.AddControl(button1);
	gump.AddControl(button2);

	gump.Show();
endfunction