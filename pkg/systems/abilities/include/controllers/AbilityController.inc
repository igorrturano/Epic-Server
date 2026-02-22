include ":abilities:AbilityRegistry";
include ":message:model/Message";

function BuyAbility(who, ability)
	// V3: Check if ability is migrated to class-based system
	if (!ABILITY_CLASSES.exists(ability.GetId()))
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

function RemoveAbility(who, ability)
	if (!who.HasAbility(ability))
		Message("Você não possui a habilidade " + ability.GetName() + ".", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	endif
	
	var state := who.RemoveAbility(ability);
	if (state is @Exception)
		Message($"Erro ao remover a habilidade {ability.GetName()}: {state.GetMessage()}", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	elseif (state == error)
		Message($"Erro desconhecido ao remover a habilidade {ability.GetName()}: {state}", Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		return false;
	endif

	Message("Habilidade " + ability.GetName() + " removida com sucesso!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who.GetObjRef());
	return true;
endfunction