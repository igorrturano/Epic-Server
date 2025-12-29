/***********************************
 *
 * habilidades.inc
 *
 * Created: 29/12/2025 08:51AM
 *
 * Responsável por:
 * Gerenciar a aplicar os cálculos das habilidades de combate.
 *
 **********************************/

include ":debug:Controller/DebugController";
include ":abilities:models/Ability";

function ExecuteCombatAbilitiesBeforeHit(byRef attackInfo)
	ExecuteAbilities(attackInfo.GetAttacker().GetActiveAbilites()[ABILITY_ACTIVATION::BEFORE_HIT], attackInfo);
	ExecuteAbilities(attackInfo.GetDefender().GetActiveAbilites()[ABILITY_ACTIVATION::BEFORE_HIT], attackInfo);
endfunction

function ExecuteCombatAbilitiesBeforeDamage(byRef attackInfo)
	ExecuteAbilities(attackInfo.GetAttacker().GetActiveAbilites()[ABILITY_ACTIVATION::BEFORE_DAMAGE], attackInfo);
	ExecuteAbilities(attackInfo.GetDefender().GetActiveAbilites()[ABILITY_ACTIVATION::BEFORE_DAMAGE], attackInfo);
endfunction

function ExecuteCombatAbilitiesOnHit(byRef attackInfo)
	ExecuteAbilities(attackInfo.GetAttacker().GetActiveAbilites()[ABILITY_ACTIVATION::ON_HIT], attackInfo);
	ExecuteAbilities(attackInfo.GetDefender().GetActiveAbilites()[ABILITY_ACTIVATION::ON_HIT], attackInfo);
endfunction

function ExecuteCombatAbilitiesAfterDamage(byRef attackInfo)
	ExecuteAbilities(attackInfo.GetAttacker().GetActiveAbilites()[ABILITY_ACTIVATION::AFTER_DAMAGE], attackInfo);
	ExecuteAbilities(attackInfo.GetDefender().GetActiveAbilites()[ABILITY_ACTIVATION::AFTER_DAMAGE], attackInfo);
endfunction

function ExecuteCombatAbilitiesOnMiss(byRef attackInfo)
	ExecuteAbilities(attackInfo.GetAttacker().GetActiveAbilites()[ABILITY_ACTIVATION::ON_MISS], attackInfo);
	ExecuteAbilities(attackInfo.GetDefender().GetActiveAbilites()[ABILITY_ACTIVATION::ON_MISS], attackInfo);
endfunction

function ExecuteCombatAbilitiesOnCritical(byRef attackInfo)
	ExecuteAbilities(attackInfo.GetAttacker().GetActiveAbilites()[ABILITY_ACTIVATION::ON_CRITICAL], attackInfo);
	ExecuteAbilities(attackInfo.GetDefender().GetActiveAbilites()[ABILITY_ACTIVATION::ON_CRITICAL], attackInfo);
endfunction

function ExecuteCombatAbilitiesOnKill(byRef attackInfo)
	ExecuteAbilities(attackInfo.GetAttacker().GetActiveAbilites()[ABILITY_ACTIVATION::ON_KILL], attackInfo);
	ExecuteAbilities(attackInfo.GetDefender().GetActiveAbilites()[ABILITY_ACTIVATION::ON_KILL], attackInfo);
endfunction

function ExecuteAbilities(byRef abilitiesList, byRef attackInfo)
	var abilitiesExecuted := 0;
	
	foreach ability in (abilitiesList)
		ability.ExecuteOn(attackInfo);
		abilitiesExecuted++;
	endforeach

	return abilitiesExecuted;
endfunction

function DebugExecutedAbilities(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_HAB, Message("=== Habilidades executadas neste ataque (ATACANTE) ===", Color(COLOR_NAME::PURPLE)));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_HAB, Message("=== Habilidades executadas neste ataque (DEFENSOR) ===", Color(COLOR_NAME::PURPLE)));
	
endfunction
