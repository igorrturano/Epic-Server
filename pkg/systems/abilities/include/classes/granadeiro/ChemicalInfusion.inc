use uo;
use os;

include ":abilities:models/CombatAbility";
include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":fls_core:packets";

/**
 * Chemical Infusion (Infusão Química) - ABILITY_ID::INFUSAO_QUIMICA (2501)
 *
 * Active (OnUse): Shows gump UI to switch between 3 elemental infusion types.
 * Sets one property on player (clears the others):
 * - "Infusão Elétrica" (Energy)
 * - "Infusão Ígnea" (Fire)
 * - "Infusão Gélida" (Cold)
 */
class ChemicalInfusion(CombatAbility)

	function ChemicalInfusion(this)
		Super(ABILITY_ID::INFUSAO_QUIMICA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var input := _CreateInfusionGump(who);
		if (input[0] == 201) // Infusão Elétrica
			EraseObjProperty(who, "Infusão Ígnea");
			EraseObjProperty(who, "Infusão Gélida");
			if (!GetObjProperty(who, "Infusão Elétrica"))
				SetObjProperty(who, "Infusão Elétrica", 1);
			endif
		elseif (input[0] == 202) // Infusão Ígnea
			EraseObjProperty(who, "Infusão Gélida");
			EraseObjProperty(who, "Infusão Elétrica");
			if (!GetObjProperty(who, "Infusão Ígnea"))
				SetObjProperty(who, "Infusão Ígnea", 1);
			endif
		elseif (input[0] == 203) // Infusão Gélida
			EraseObjProperty(who, "Infusão Elétrica");
			EraseObjProperty(who, "Infusão Ígnea");
			if (!GetObjProperty(who, "Infusão Gélida"))
				SetObjProperty(who, "Infusão Gélida", 1);
			endif
		else
			return false;
		endif

		who.frozen := 1;
		var sound := 0x666;
		if (GetObjProperty(who, "Infusão Gélida"))
			sound := 0x5c8;
		elseif (GetObjProperty(who, "Infusão Ígnea"))
			sound := 0x5cb;
		endif
		PrintTextAbove(who, "*Infundindo poção*");
		PlaySoundEffect(who, sound);
		PlaySoundEffect(who, 0x243);
		sleep(1);
		PlaySoundEffect(who, 0x241);
		sleep(3);
		who.frozen := 0;

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[INFUSAO_QUIMICA] Infusao alterada", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _CreateInfusionGump(player)
	var gump := GFCreateGump();
	var quinquilharias := GetObjProperty(player, "quinquilharias");
	if (!quinquilharias)
		quinquilharias := array{};
		quinquilharias.Append("Infusão Elétrica");
		quinquilharias.Append("Infusão Ígnea");
		quinquilharias.Append("Infusão Gélida");
		SetObjProperty(player, "quinquilharias", quinquilharias);
	endif

	GFResizePic(gump, 17, 31, 9390, 359, 125 + quinquilharias.size() * 26);
	GFHTMLArea(gump, 114, 69, 192, 20, "<BASEFONT Size=5 Color=#999999 >Granadeiro", 0, 0);
	GFHTMLArea(gump, 116, 71, 192, 20, "<BASEFONT Size=5 Color=#999999 >Granadeiro", 0, 0);
	GFHTMLArea(gump, 115, 70, 192, 20, "<BASEFONT Size=5 Color=#ffffff >Granadeiro", 0, 0);
	GFGumpPic(gump, 63, 91, 1469);
	GFGumpPic(gump, 80, 91, 1469);
	GFTextLine(gump, 63, 100, 0, "Aprimoramentos");
	GFTextLine(gump, 259, 100, 0, "Modificar");
	GFGumpPic(gump, 38, 103, 9773, 0);
	GFGumpPic(gump, 296, 0, 9000, 0);

	var y := 121;
	var i;
	for (i := 1; i <= quinquilharias.size(); i++)
		var upgrade := quinquilharias[i];
		var color := "#000000";
		GFHTMLArea(gump, 63, y, 192, 24, "<BASEFONT color=#{}>{}".format(color, upgrade), 1, 0);
		GFAddButton(gump, 274, y + 5, 10006, 10006, GF_CLOSE_BTN, 200 + i);
		sleepms(2);
		y += 26;
	endfor

	SetObjProperty(player, "quinquilharias", quinquilharias);
	return GFSendGump(player, gump);
endfunction
