include ":gumps:epicGumps";
include ":gumpDesigner:Gump/Gump";
include ":gumpDesigner:Control/Text";
include ":gumpDesigner:Control/Button";
include ":gumpDesigner:Control/ImageGump";
include ":gumpDesigner:Control/Container/Container";
include ":gumpDesigner:Control/Container/ContainerGridLayout";
include ":achievements:model/AchievementCombat";

function ShowAchievementsGump(who)
	var gump := Gump(who, "Progresso de Conquistas", 680, 400);
	
	var gridLayout := ContainerGridLayout("Layout", 25, 80, 100, 350, 4, 1);

	var combatContainer := Container("CombatContainer", 0, 0, 100, 100);
	combatContainer.SetVisible(true);
	var combatButton := Button("CombatButton", 0, 0, 0x15B9, 0x15BA, BUTTON_TYPE::DUMMY);
	var combatText := Text("CombatText", 25, 60, "Combate");
	combatContainer.AddControl(combatButton);
	combatContainer.AddControl(combatText);

	var skillsContainer := Container("SkillsContainer", 0, 0, 100, 100);
	skillsContainer.SetVisible(true);
	var skillsButton := Button("SkillsButton", 0, 0, 0x15B3, 0x15B4, BUTTON_TYPE::DUMMY);
	var skillsText := Text("SkillsText", 25, 60, "Skills", TEXT_COLOR::COLOR_DEFAULT, skillsContainer.GetWidth(), TEXT_ALIGN::ALIGN_CENTER);
	skillsContainer.AddControl(skillsButton);
	skillsContainer.AddControl(skillsText);

	gridLayout.AddControl(combatContainer, 1,1);
	gridLayout.AddControl(skillsContainer, 2,1);
	gump.AddControl(gridLayout);
	gump.Show();
endfunction