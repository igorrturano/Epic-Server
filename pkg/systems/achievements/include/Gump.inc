include ":gumps:epicGumps";
include ":gumpDesigner:Gump/Gump";
include ":gumpDesigner:Control/Text";
include ":gumpDesigner:Control/TextCropped";
include ":gumpDesigner:Control/Button";
include ":gumpDesigner:Control/ImageGump";
include ":gumpDesigner:Control/ImageGumpTiled";
include ":gumpDesigner:Control/ImageTiled";
include ":gumpDesigner:Control/TextBox";
include ":gumpDesigner:Control/Tooltip";
include ":gumpDesigner:Control/CheckBox";
include ":gumpDesigner:Control/RadioButton";
include ":achievements:model/AchievementCombat";

function ShowAchievementsGump(who)
	var gump := Gump(who, "Progresso de Conquistas", 680, 400);

	var text1 := Text("Label1", 45, 75, "Exemplo de texto", GUMP_TEXT_COLOR::COLOR_RED);
	var button1 := Button("Button1", 30, 80, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_UNPRESSED, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_PRESSED, GF_CLOSE_BTN);

	var text2 := Text("Label2", 45, 100, "Exemplo de texto 2", GUMP_TEXT_COLOR::COLOR_RED);
	var button2 := Button("Button2", 30, 105, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_UNPRESSED, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_PRESSED, GF_CLOSE_BTN);

	var hist := "Nas ruínas enevoadas de Eldoria, o aprendiz Kael encontrou um cristal pulsante. Ao tocá-lo, vislumbres de batalhas antigas guiaram sua mente até um altar escondido. Ali libertou um eco de guardiã ancestral que, em gratidão, marcou sua lâmina com runas de luz. Agora ele segue, decidido a restaurar o equilíbrio. Enquanto viaja, percebe fissuras no tecido da magia: fontes secas, feras transtornadas, memórias alheias soprando em sua própria voz. Cada vilarejo lhe oferece fragmentos de um mapa etéreo que só brilha quando o luar toca o aço rúnico. Perseguido por sombras que repetem seus passos com atraso funesto, Kael aprende a ouvir o silêncio entre os trovões. Sabe que, se falhar, a mesma névoa que velou o passado engolirá o amanhã, e ainda assim avança.";
	var text3 := TextCropped("Label3", 45, 125, 300, 800, hist, GUMP_TEXT_COLOR::COLOR_BLUE);
	var button3 := Button("Button3", 30, 130, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_UNPRESSED, GUMP_CONTROL_BUTTON_IMAGE::CLOSE_PRESSED, GF_CLOSE_BTN);

	var imageGump1 := ImageGump("ImageGump1", 30, 200, 0x775f);
	imageGump1.SetTooltip(TooltipText("TooltipText1", "Testando Tooltip: Isso aqui é uma imagem!!!"));

	var imageGumpTiled1 := ImageGumpTiled("ImageGumpTiled1", 100, 200, 100, 100, 0x775f);
	var imageTiled1 := ImageTiled("ImageTiled2", 400, 200, 100, 100, 0x775f);

	var text4 := Text("Label4", 45, 150, "Digite seu nome aqui: ", GUMP_TEXT_COLOR::COLOR_RED);
	var textBox1 := TextBox("TextBox1", 180, 150, 200, 1, GUMP_TEXT_COLOR::COLOR_YELLOW);

	var checkBox1 := CheckBox("CheckBox1", 45, 180, GUMP_CONTROL_CHECKBOX_IMAGE::DEFAULT_UNCHECKED, GUMP_CONTROL_CHECKBOX_IMAGE::DEFAULT_CHECKED);

	var radioButton1 := RadioButton("RadioButton1", 45, 210, GUMP_CONTROL_RADIOBUTTON_IMAGE::DEFAULT_UNPRESSED, GUMP_CONTROL_RADIOBUTTON_IMAGE::DEFAULT_PRESSED);

	gump.AddControl(text1);
	gump.AddControl(text2);
	gump.AddControl(text3);
	gump.AddControl(button1);
	gump.AddControl(button2);
	gump.AddControl(button3);
	// gump.AddControl(imageGump1);
	// gump.AddControl(imageGumpTiled1);
	// gump.AddControl(imageTiled1);
	gump.AddControl(text4);
	gump.AddControl(textBox1);
	gump.AddControl(checkBox1);
	gump.AddControl(radioButton1);

	gump.Show();

	print(textBox1.ToString());
endfunction