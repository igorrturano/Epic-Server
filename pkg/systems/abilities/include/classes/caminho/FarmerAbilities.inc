use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":tn:tngumps";

// Rural Life - ABILITY_ID::RURAL_LIFE (3580)
// Passive: animals do not attack bearers (checked in animalBrain.inc).
// Active: target an owned pet and set a description shown on its tooltip.
class RuralLife(CombatAbility)
	function RuralLife(this)
		Super(ABILITY_ID::RURAL_LIFE);
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();

		Message("Escolha um animal para personalizar.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		var animal := Target(who);
		if (!animal)
			Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return;
		endif

		if (animal.master != who)
			Message("Esse animal nao e seu!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return;
		endif

		var original_name := animal.name;
		SetName(animal, "atualizando descricao");

		var repeating := 3;
		var str := "";
		Message("Descreva seu animal em ate 83 caracteres.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		while (repeating)
			str := SmallRequestGump(who, "Qual a nova descricao?", 1);
			if (Len(str) < 84)
				break;
			endif
			Message("Voce excedeu o numero maximo de 83 caracteres!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			repeating := repeating - 1;
			str := "";
		endwhile

		if (str)
			SetObjProperty(animal, "description", str);
		endif

		SetName(animal, original_name);
	endfunction
endclass

// Green Thumb - ABILITY_ID::GREEN_THUMB (3581)
// Passive: Seeds start at grow step 5.
// Effect handled in plantseed.src.
class GreenThumb(CombatAbility)
	function GreenThumb(this)
		Super(ABILITY_ID::GREEN_THUMB);
	endfunction
endclass

// Talented Farmer - ABILITY_ID::TALENTED_FARMER (3583)
// Passive: +20 farming skill bonus.
// Effect handled in harvestplant.src and plantseed.src.
class TalentedFarmer(CombatAbility)
	function TalentedFarmer(this)
		Super(ABILITY_ID::TALENTED_FARMER);
	endfunction
endclass

// No Waste - ABILITY_ID::NO_WASTE (3584)
// Passive: +skill/20 skinning yield.
// Effect handled in skinning.src.
class NoWaste(CombatAbility)
	function NoWaste(this)
		Super(ABILITY_ID::NO_WASTE);
	endfunction
endclass

// Guaranteed Replant - ABILITY_ID::GUARANTEED_REPLANT (3585)
// Passive: Always get seeds on harvest.
// Effect handled in harvestplant.src.
class GuaranteedReplant(CombatAbility)
	function GuaranteedReplant(this)
		Super(ABILITY_ID::GUARANTEED_REPLANT);
	endfunction
endclass

// Shepherd - ABILITY_ID::SHEPHERD (3586)
// Passive: Herd without taming + breed animals.
// Effect handled in herding scripts.
class Shepherd(CombatAbility)
	function Shepherd(this)
		Super(ABILITY_ID::SHEPHERD);
	endfunction
endclass

// Harvest Luck - ABILITY_ID::HARVEST_LUCK (3588)
// Passive: 2x harvest/honey/reagent yields.
// Effect handled in harvestplant.src and harvest_honey.src.
class HarvestLuck(CombatAbility)
	function HarvestLuck(this)
		Super(ABILITY_ID::HARVEST_LUCK);
	endfunction
endclass
