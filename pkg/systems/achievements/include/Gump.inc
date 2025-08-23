include ":gumps:epicGumps";
include ":gumpDesigner:Gump/Gump";
include ":achievements:model/AchievementCombat";

function ShowAchievementsGump(who)
	var gump := Gump(who, "Progresso de Conquistas", 680, 400);
endfunction