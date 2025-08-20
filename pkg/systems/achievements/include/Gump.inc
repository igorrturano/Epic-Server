include ":gumps:epicGumps";
include ":gumps:model/Gump";
include ":gumps:model/Control/ControlText";
include ":gumps:model/Control/ControlTextCropped";
include ":gumps:model/Control/ControlButton";
include ":achievements:model/AchievementCombat";

const MAX_XP := 5000; // XP máximo para a barra de progresso

function ShowAchievementsGump(who)
	var gump := Gump(who, "Progresso de Conquistas", 680, 400);

	var text1 := ControlText("Label1", 45, 75, "Exemplo de texto", GUMP_TEXT_COLOR::COLOR_RED);
	var button1 := ControlButton("Button1", 30, 80, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_UNPRESSED, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_PRESSED, GF_CLOSE_BTN);

	var text2 := ControlText("Label2", 45, 100, "Exemplo de texto 2", GUMP_TEXT_COLOR::COLOR_RED);
	var button2 := ControlButton("Button2", 30, 105, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_UNPRESSED, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_PRESSED, GF_CLOSE_BTN);

	var hist := "Nas ruínas enevoadas de Eldoria, o aprendiz Kael encontrou um cristal pulsante. Ao tocá-lo, vislumbres de batalhas antigas guiaram sua mente até um altar escondido. Ali libertou um eco de guardiã ancestral que, em gratidão, marcou sua lâmina com runas de luz. Agora ele segue, decidido a restaurar o equilíbrio. Enquanto viaja, percebe fissuras no tecido da magia: fontes secas, feras transtornadas, memórias alheias soprando em sua própria voz. Cada vilarejo lhe oferece fragmentos de um mapa etéreo que só brilha quando o luar toca o aço rúnico. Perseguido por sombras que repetem seus passos com atraso funesto, Kael aprende a ouvir o silêncio entre os trovões. Sabe que, se falhar, a mesma névoa que velou o passado engolirá o amanhã, e ainda assim avança.";
	var text3 := ControlTextCropped("Label3", 45, 125, 300, 800, hist, GUMP_TEXT_COLOR::COLOR_BLUE);
	var button3 := ControlButton("Button3", 30, 130, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_UNPRESSED, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_PRESSED, GF_CLOSE_BTN);

	gump.AddControl(text1);
	gump.AddControl(text2);
	gump.AddControl(text3);
	gump.AddControl(button1);
	gump.AddControl(button2);
	gump.AddControl(button3);

	gump.Show();
endfunction