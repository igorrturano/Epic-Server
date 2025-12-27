include ":abilities:models/Ability";
include ":message:model/Message";

function ActivateAbility(who, abilityID)
	var ability := Ability(abilityID);

	if (!ability.GetScript())
		Message("Habilidade desconhecida.", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	endif

	

endfunction
