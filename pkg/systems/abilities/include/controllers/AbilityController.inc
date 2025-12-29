include ":abilities:models/Ability";
include ":message:model/Message";

function ActivateAbility(who, ability)
	if (!ability.GetScript())
		Message("Habilidade desconhecida.", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	endif

	Start_Script($":abilities:scripts/{ability.GetScript()}", who.GetObjRef());

	who.ActivateAbility(ability);
endfunction

function DeactivateAbility(who, ability)
	if (!ability.GetScript())
		Message("Habilidade desconhecida.", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	endif

	who.DeactivateAbility(ability);
endfunction
