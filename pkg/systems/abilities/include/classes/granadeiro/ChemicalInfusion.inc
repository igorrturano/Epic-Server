use uo;
use os;

include ":abilities:models/CombatAbility";
include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":fls_core:packets";
include ":alchemy:alchemy";

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

		var new_elemento := 0;
		if (input[0] == 201) // Infusão Elétrica
			EraseObjProperty(who, "Infusão Ígnea");
			EraseObjProperty(who, "Infusão Gélida");
			SetObjProperty(who, "Infusão Elétrica", 1);
			new_elemento := ELEMENTO_ENERGIA;
		elseif (input[0] == 202) // Infusão Ígnea
			EraseObjProperty(who, "Infusão Gélida");
			EraseObjProperty(who, "Infusão Elétrica");
			SetObjProperty(who, "Infusão Ígnea", 1);
			new_elemento := ELEMENTO_FOGO;
		elseif (input[0] == 203) // Infusão Gélida
			EraseObjProperty(who, "Infusão Elétrica");
			EraseObjProperty(who, "Infusão Ígnea");
			SetObjProperty(who, "Infusão Gélida", 1);
			new_elemento := ELEMENTO_GELO;
		else
			return false;
		endif

		var cfg := GetObjProperty(who, "ExplosivoConfig");
		if (!cfg)
			cfg := struct{};
			cfg.+gatilho  := GATILHO_PAVIO;
			cfg.+elemento := ELEMENTO_FOGO;
			cfg.+involucro := INVOLUCRO_BASE;
			cfg.+carga    := CARGA_BALANCEADA;
			cfg.+reforco  := REFORCO_COMUM;
		endif
		cfg.elemento := new_elemento;
		SetObjProperty(who, "ExplosivoConfig", cfg);

		who.frozen := 1;
		var sound := 0x666;
		if (new_elemento == ELEMENTO_GELO)
			sound := 0x5c8;
		elseif (new_elemento == ELEMENTO_FOGO)
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

	var cfg := GetObjProperty(player, "ExplosivoConfig");
	var current_elemento := cfg.elemento;

	var options := array{ "Infusão Elétrica", "Infusão Ígnea", "Infusão Gélida" };
	var elementos := array{ ELEMENTO_ENERGIA, ELEMENTO_FOGO, ELEMENTO_GELO };

	GFResizePic(gump, 17, 31, 9390, 359, 125 + options.size() * 26);
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
	for (i := 1; i <= options.size(); i++)
		var color := (current_elemento == elementos[i]) ? "#00ff00" : "#000000";
		GFHTMLArea(gump, 63, y, 192, 24, "<BASEFONT color=#{}>{}".format(color, options[i]), 1, 0);
		GFAddButton(gump, 274, y + 5, 10006, 10006, GF_CLOSE_BTN, 200 + i);
		sleepms(2);
		y += 26;
	endfor

	return GFSendGump(player, gump);
endfunction
