use uo;

include "include/sysEvent";
include ":gumps:gumps";

const COAL_OBJTYPE := 0x1bde;
const POWDER_OBJTYPE := 0x9092;
const CANNONBALL_OBJTYPE_PB := 0x4224;
const PB_HANDCANNON_OBJTYPE := 0xFFC7;
const PB_HARPOON_OBJTYPE := 0xFFC6;
const PB_POWDERTRAP_OBJTYPE := 0xFFC8;

/*
 * PassiveBrain - Event-driven brain for golems and constructs.
 * Handles item storage (coal/powder/cannonballs) and gadget swap via double-click.
 */
function PassiveBrain()
	EnableEvents(SYSEVENT_ITEM_GIVEN, 5);
	EnableEvents(SYSEVENT_DOUBLECLICKED);

	while (me)
		var ev := os::wait_for_event(120);

		if (ShouldGoDormant(polcore().systime))
			return;
		endif

		if (!ev)
			continue;
		endif

		case (ev.type)
			SYSEVENT_ITEM_GIVEN:
				_PB_HandleItemGiven(ev);
			SYSEVENT_DOUBLECLICKED:
				_PB_HandleDoubleClick(ev);
		endcase
	endwhile
endfunction

function _PB_HandleItemGiven(ev)
	var giver := ev.source;
	var item := ev.item;

	// Only golems accept items
	if (me.npctemplate != ":epicai:PowerArmor")
		MoveItemToContainer(item, giver.backpack);
		return;
	endif

	// Coal
	if (item.objtype == COAL_OBJTYPE)
		var stored := CInt(GetObjProperty(me, "golem_coal"));
		SetObjProperty(me, "golem_coal", stored + item.amount);
		PrintTextAbove(me, $"*Armazena {item.amount} carvoes*");
		DestroyItem(item);
		return;
	endif

	// Gunpowder
	if (item.objtype == POWDER_OBJTYPE)
		var stored := CInt(GetObjProperty(me, "golem_powder"));
		SetObjProperty(me, "golem_powder", stored + item.amount);
		PrintTextAbove(me, $"*Armazena {item.amount} polvoras*");
		DestroyItem(item);
		return;
	endif

	// Cannonball
	if (item.objtype == CANNONBALL_OBJTYPE_PB)
		var stored := CInt(GetObjProperty(me, "golem_cannonball"));
		SetObjProperty(me, "golem_cannonball", stored + item.amount);
		PrintTextAbove(me, $"*Armazena {item.amount} balas de canhao*");
		DestroyItem(item);
		return;
	endif

	// Reject everything else
	MoveItemToContainer(item, giver.backpack);
	PrintTextAbove(me, "*Rejeita o item*");
endfunction

function _PB_HandleDoubleClick(ev)
	var clicker := ev.source;

	// Only golems
	if (me.npctemplate != ":epicai:PowerArmor")
		return;
	endif

	// Only owner
	if (CInt(GetObjProperty(me, "golem_maker")) != clicker.serial)
		PrintTextAbove(me, "*Ignora*");
		return;
	endif

	// Show gadget management gump
	_PB_GadgetGump(clicker);
endfunction

function _PB_GadgetGump(who)
	var current := GetObjProperty(me, "InstalledGadget");
	var current_name := "Nenhum";
	case (current)
		"handcannon": current_name := "Canhao de Mao";
		"harpoon":    current_name := "Arpao Mecanico";
		"powdertrap": current_name := "Armadilha de Polvora";
	endcase

	var has_cannon  := FindObjtypeInContainer(who.backpack, PB_HANDCANNON_OBJTYPE);
	var has_harpoon := FindObjtypeInContainer(who.backpack, PB_HARPOON_OBJTYPE);
	var has_trap    := FindObjtypeInContainer(who.backpack, PB_POWDERTRAP_OBJTYPE);

	var coal := CInt(GetObjProperty(me, "golem_coal"));
	var powder := CInt(GetObjProperty(me, "golem_powder"));
	var cannonball := CInt(GetObjProperty(me, "golem_cannonball"));

	var gump := GFCreateGump();
	GFClosable(gump, 1);
	GFMovable(gump, 1);
	GFResizePic(gump, 0, 0, 0x0A28, 400, 320);

	GFHTMLArea(gump, 0, 16, 400, 25, "<CENTER><BASEFONT SIZE=5 COLOR=#DDAA44>Golem - Gerenciamento</BASEFONT></CENTER>", 0, 0);

	// Ammo display
	GFTextLine(gump, 20, 45, 0x35, $"Gadget Instalado: {current_name}");
	GFTextLine(gump, 20, 65, 0x3B2, $"Carvao: {coal}");
	GFTextLine(gump, 150, 65, 0x3B2, $"Polvora: {powder}");
	GFTextLine(gump, 280, 65, 0x3B2, $"Balas: {cannonball}");

	// Gadget slots
	GFHTMLArea(gump, 0, 95, 400, 25, "<CENTER><BASEFONT SIZE=4 COLOR=#AAAAAA>Trocar Gadget</BASEFONT></CENTER>", 0, 0);

	var gadgets := array{
		struct{ key := "handcannon", name := "Canhao de Mao",         objtype := PB_HANDCANNON_OBJTYPE, has := has_cannon,  desc := "AoE explosivo" },
		struct{ key := "harpoon",    name := "Arpao Mecanico",        objtype := PB_HARPOON_OBJTYPE,    has := has_harpoon, desc := "Puxa o alvo"   },
		struct{ key := "powdertrap", name := "Lancador de Minas",     objtype := PB_POWDERTRAP_OBJTYPE, has := has_trap,    desc := "Mina invisivel" }
	};

	var x := 15;
	var btn_id := 1;
	foreach g in gadgets
		var is_current := (current == g.key);
		GFResizePic(gump, x, 115, is_current ? 0x0BB8 : 0x13BE, 120, 170);
		GFTilePic(gump, x + 35, 130, CInt(g.objtype), 0);
		GFTextLine(gump, x + 5, 185, is_current ? 0x35 : 0x0, g.name);
		GFTextLine(gump, x + 5, 205, 0x3B2, g.desc);

		if (is_current)
			GFTextLine(gump, x + 20, 235, 0x35, "[Instalado]");
		elseif (g.has)
			GFAddButton(gump, x + 15, 240, 0xFA5, 0xFA7, GF_CLOSE_BTN, btn_id);
			GFTextLine(gump, x + 50, 238, 0x44, "Instalar");
		else
			GFTextLine(gump, x + 15, 235, 0x26, "Indisponivel");
		endif

		btn_id += 1;
		x += 125;
	endforeach

	// Remove button
	if (current)
		GFAddButton(gump, 155, 295, 0xFB1, 0xFB3, GF_CLOSE_BTN, 10);
		GFTextLine(gump, 190, 293, 0x26, "Remover gadget");
	endif

	var result := GFSendGump(who, gump);
	var butid := result[0];
	if (!butid)
		return;
	endif

	// Removal animation (5 seconds) - same pattern as steamarms_use.src
	if (current)
		PrintTextAbove(me, "*Removendo o mecanismo atual...*");
		for i := 1 to 5
			PerformAction(who, 19); // ATTACK_XBOW
			PlaySoundEffect(who, 0x22C);
			sleep(1);
			if (who.dead)
				return;
			endif
		endfor
		PerformAction(who, 12); // ATTACK_2HAND_BLUNT
	endif

	// Installation animation (10 seconds) - only if not just removing
	if (butid != 10)
		PrintTextAbove(me, "*Instalando novo mecanismo...*");
		for i := 1 to 5
			PerformAction(who, 19); // ATTACK_XBOW
			PlaySoundEffect(who, 0x22C);
			sleep(2);
			if (who.dead)
				return;
			endif
		endfor
		PerformAction(who, 12); // ATTACK_2HAND_BLUNT
	endif

	// Return current gadget to player's backpack
	if (current)
		var return_objtype := 0;
		case (current)
			"handcannon": return_objtype := PB_HANDCANNON_OBJTYPE;
			"harpoon":    return_objtype := PB_HARPOON_OBJTYPE;
			"powdertrap": return_objtype := PB_POWDERTRAP_OBJTYPE;
		endcase
		if (return_objtype)
			var returned := CreateItemInContainer(who.backpack, return_objtype, 1);
			if (returned)
				PrintTextAbove(me, "*Devolve o gadget antigo*");
			endif
		endif
	endif

	case (butid)
		1:
			var gadget := FindObjtypeInContainer(who.backpack, PB_HANDCANNON_OBJTYPE);
			if (gadget)
				SubtractAmount(gadget, 1);
				SetObjProperty(me, "InstalledGadget", "handcannon");
				PrintTextAbove(me, "*Canhao de Mao instalado*");
			endif
		2:
			var gadget := FindObjtypeInContainer(who.backpack, PB_HARPOON_OBJTYPE);
			if (gadget)
				SubtractAmount(gadget, 1);
				SetObjProperty(me, "InstalledGadget", "harpoon");
				PrintTextAbove(me, "*Arpao Mecanico instalado*");
			endif
		3:
			var gadget := FindObjtypeInContainer(who.backpack, PB_POWDERTRAP_OBJTYPE);
			if (gadget)
				SubtractAmount(gadget, 1);
				SetObjProperty(me, "InstalledGadget", "powdertrap");
				PrintTextAbove(me, "*Lancador de Minas instalado*");
			endif
		10:
			EraseObjProperty(me, "InstalledGadget");
			PrintTextAbove(me, "*Gadget removido*");
	endcase

	PlaySoundEffect(who, 0x02B); // completion sound
endfunction
