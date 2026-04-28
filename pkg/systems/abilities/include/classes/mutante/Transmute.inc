use uo;

include ":abilities:models/CombatAbility";
include ":tn:tngumps";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include "include/say";

/**
 * Transmute (Transmutar) - ABILITY_ID::TRANSMUTAR (903)
 *
 * Active (ON_USE): Crafting ability. Shows gump to select element type
 * (Fire/Electricity/Ice). Then sequential targeting of 3 reagent types
 * (10x each) + 1 empty vial. Creates elemental resin item.
 *
 * Fire: 10x sulfur ash (0xF8C) + 10x dead wood (0xF90) + 10x spider webs (0xF8D) → 0x6071
 * Electricity: 10x black pearls (0xF7A) + 10x dead wood + 10x spider webs → 0xBD14
 * Ice: 10x solanaceae (0xF88) + 10x dead wood + 10x spider webs → 0x6070
 */
class Transmute(CombatAbility)

	function Transmute(this)
		Super(ABILITY_ID::TRANSMUTAR);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		// Crafting flavor — RadioGump + Target chain has no NPC path.
		if (mobile.IsNPC())
			return false;
		endif

		var who := mobile.GetObjRef();
		var custo := 10;

		var prot := RadioGump(who, 200, 200, "Qual o elemento da resina?", { "Fogo", "Eletricidade", "Gelo" });
		if (!prot)
			return false;
		endif

		var firstReag, secondReag, thirdReag, resultItem;
		var firstName, secondName, thirdName;

		if (prot == "Fogo")
			firstReag := 0xF8C; firstName := "cinzas de enxofre";
			secondReag := 0xF90; secondName := "madeiras mortas";
			thirdReag := 0xF8D; thirdName := "teias de aranha";
			resultItem := 0x6071;
		elseif (prot == "Eletricidade")
			firstReag := 0xF7A; firstName := "perolas negras";
			secondReag := 0xF90; secondName := "madeiras mortas";
			thirdReag := 0xF8D; thirdName := "teias de aranha";
			resultItem := 0xBD14;
		elseif (prot == "Gelo")
			firstReag := 0xF88; firstName := "solanaceas";
			secondReag := 0xF90; secondName := "madeiras mortas";
			thirdReag := 0xF8D; thirdName := "teias de aranha";
			resultItem := 0x6070;
		else
			return false;
		endif

		if (!_CollectReagent(who, firstReag, custo, $"Voce precisa de {custo} {firstName} para o inicio"))
			return false;
		endif

		if (!_CollectReagent(who, secondReag, custo, $"Agora voce precisa de {custo} {secondName} para continuar"))
			return false;
		endif

		if (!_CollectReagent(who, thirdReag, custo, $"Agora voce precisa de {custo} {thirdName} para continuar"))
			return false;
		endif

		// Collect empty vial
		SendSysMessageEX(who, "Em qual frasco voce vai colocar a resina elemental?", SSM_REQUEST);
		var targ := Target(who);
		if (!targ)
			SendSysMessageEX(who, "Cancelado.", SSM_FAIL);
			return false;
		endif

		if (targ.objtype != 0x0F0E)
			SendSysMessageEX(who, "Isto nao e um frasco de vidro.", SSM_FAIL);
			return false;
		endif

		if (!SubtractAmount(targ, 1))
			SendSysMessageEX(who, "Voce nao tem frascos suficientes", SSM_INFO);
			return false;
		endif

		// Mixing animation
		PlaySoundEffect(who, 0x243);
		PrintTextAbove(who, "*Misturando Reagentes*");
		sleep(1);
		PrintTextAbove(who, "*Misturando Reagentes*");
		sleep(1);
		PrintTextAbove(who, "*Misturando Reagentes*");
		sleep(1);
		PlaySoundEffect(who, 0x243);

		CreateItemInBackPack(who, resultItem, 1);
		PlaySoundEffect(who, 0x23F);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[TRANSMUTAR] Resina {prot} criada", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _CollectReagent(who, objtype, custo, promptMsg)
	SendSysMessageEX(who, promptMsg, SSM_REQUEST);
	var targ := Target(who);

	if (!targ)
		SendSysMessageEX(who, "Cancelado.", SSM_FAIL);
		return false;
	endif

	if (targ.objtype != objtype)
		SendSysMessageEx(who, "Isto nao sao reagentes.", SSM_FAIL);
		return false;
	endif

	if (targ.amount < custo)
		SendSysMessageEX(who, "Voce nao tem reagentes suficientes.", SSM_INFO);
		return false;
	endif

	if (!SubtractAmount(targ, custo))
		SendSysMessageEx(who, "Voce nao tem reagentes suficientes.", SSM_INFO);
		return false;
	endif

	return true;
endfunction
