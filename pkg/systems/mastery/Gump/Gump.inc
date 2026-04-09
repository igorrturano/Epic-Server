include ":gumpDesigner:Gump/Gump";
include ":gumpDesigner:Control/Container/ContainerGridLayout";

include ":wrappers:Objects/Model/MobileWrapper";

function ShowMasteryGump(player)
	player := MobileWrapper(player);
	var playerMasteries := player.GetMasteries();
	
	var gump := Gump(player.GetObjRef(), "Menu de Maestrias", 400, 300);
	var container := ContainerGridLayout("MasteryGrid", 25, 80, 340, 100, 3, 6).SetVisible(true);


	gump.AddControl(container);
	gump.Show();
endfunction