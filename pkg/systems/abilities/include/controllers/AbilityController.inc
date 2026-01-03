include ":abilities:models/Ability";
include ":message:model/Message";

function BuyAbility(who, ability)
	if (!ability.GetScript())
		Message("Habilidade ainda não implementada.", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	endif

	if (who.HasAbility(ability))
		Message("Você já possui a habilidade " + ability.GetName() + ".", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	endif

	var state := who.AddAbility(ability);
	if (state is @Exception)
		Message($"Erro ao adquirir a habilidade {ability.GetName()}: {state.GetMessage()}", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	elseif (state == error)
		Message($"Erro desconhecido ao adquirir a habilidade {ability.GetName()}: {state}", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	endif

	Message("Habilidade " + ability.GetName() + " adquirida com sucesso!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who.GetObjRef());
	return true;
endfunction
