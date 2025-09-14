include ":gumps:epicGumps";
include ":gumpDesigner:Gump/Gump";
include ":gumpDesigner:Control/Text";
include ":gumpDesigner:Control/Button";
include ":gumpDesigner:Container/Container";
include ":achievements:model/AchievementCombat";

function ShowAchievementsGump(who)
	var gump := Gump(who, "Progresso de Conquistas", 680, 400);
	
	var combatContainer := Container(gump, 20, 80, 100, 100);
	var combatButton := Button("CombatButton", 0, 0, BUTTON_IMAGE::CLOSE_UNPRESSED, BUTTON_IMAGE::CLOSE_PRESSED, BUTTON_TYPE::DUMMY);
	var combatText := Text("CombatText", 20, 0, "Combate");

	combatContainer.AddControl(combatButton);
	combatContainer.AddControl(combatText);

	gump.AddControl(combatContainer);
	gump.Show();
endfunction