include ":gumps:epicGumps";
include ":gumpDesigner:gumps/Gump";
include ":gumpDesigner:controls/Text";
include ":gumpDesigner:controls/Button";
include ":gumpDesigner:controls/ImageGump";
include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/containers/ContainerGridLayout";
include ":achievements:models/Achievement";

function ShowAchievementsGump(who)
	var whoAchievs := who.GetListAchievement() ?: array{};
	print ($"Achievements do {who.name}: {whoAchievs}");
	
	var gump := Gump(who, "Progresso de Conquistas", 680, 400);
	
	var menuContainer := ContainerGridLayout("Layout", 25, 80, 100, 350, 4, 1);

	var combatContainer := Container("CombatContainer", 0, 0, 100, 100);
	combatContainer.SetVisible();
	var combatButton := Button("CombatButton", 0, 0, 0x15B9, 0x15BA, BUTTON_TYPE::DUMMY);
	var combatText := Text("CombatText", 25, 60, "Combate");
	combatContainer.AddControl(combatButton);
	combatContainer.AddControl(combatText);

	var skillsContainer := Container("SkillsContainer", 0, 0, 100, 100);
	skillsContainer.SetVisible();
	var skillsButton := Button("SkillsButton", 0, 0, 0x15B3, 0x15B4, BUTTON_TYPE::DUMMY);
	var skillsText := Text("SkillsText", 25, 60, "Skills", TEXT_COLOR::COLOR_DEFAULT, skillsContainer.GetWidth(), TEXT_ALIGN::ALIGN_CENTER);
	skillsContainer.AddControl(skillsButton);
	skillsContainer.AddControl(skillsText);

	var displayContainer := ContainerGridLayout("DisplayContainer", 130, 80, 535, 305, 12, 5);
	displayContainer.AddControl(Text("Header_Name", 0, 0, "Nome", TEXT_COLOR::COLOR_RED), 1, 1);
	displayContainer.AddControl(Text("Header_Type", 0, 0, "Tipo", TEXT_COLOR::COLOR_RED), 1, 2);
	displayContainer.AddControl(Text("Header_XP", 0, 0, "XP", TEXT_COLOR::COLOR_RED), 1, 3);
	displayContainer.AddControl(Text("Header_Level", 0, 0, "Level", TEXT_COLOR::COLOR_RED), 1, 4);
	displayContainer.AddControl(Text("Header_Value", 0, 0, "Valor", TEXT_COLOR::COLOR_RED), 1, 5);
	var i := 1;
	foreach achiev in (whoAchievs)
		i++;
		var achievRefName := Text("Achiev_RefName", 0, 0, achiev.GetRefName(), TEXT_COLOR::COLOR_YELLOW);
		var achievName := Text("Achiev_Name", 0, 0, achiev.GetName(), TEXT_COLOR::COLOR_YELLOW);
		var achievLevel := Text("Achiev_Level", 0, 0, achiev.GetLevel(), TEXT_COLOR::COLOR_YELLOW);
		var achievXP := Text("Achiev_XP", 0, 0, achiev.GetXP(), TEXT_COLOR::COLOR_YELLOW);
		var achievValue := Text("Achiev_Value", 0, 0, achiev.GetValue(), TEXT_COLOR::COLOR_YELLOW);
		displayContainer.AddControl(achievRefName, i, 1);
		displayContainer.AddControl(achievName, i, 2);
		displayContainer.AddControl(achievLevel, i, 3);
		displayContainer.AddControl(achievXP, i, 4);
		displayContainer.AddControl(achievValue, i, 5);
	endforeach

	menuContainer.AddControl(combatContainer, 1,1);
	menuContainer.AddControl(skillsContainer, 2,1);
	gump.AddControl(menuContainer);
	gump.AddControl(displayContainer);
	gump.Show();
endfunction