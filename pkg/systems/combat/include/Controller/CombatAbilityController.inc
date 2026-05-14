/***********************************
 *
 * CombatAbilityController.inc
 *
 * Created: 29/12/2025 08:51AM
 *
 * Responsável por:
 * Gerenciar e executar habilidades de combate nos momentos corretos.
 *
 * Princípio: Combat code triggers abilities, AbilityMonitor tracks state.
 *
 **********************************/

include ":debug:Controller/DebugController";
include ":abilities:AbilityRegistry";
include ":abilities:controllers/AbilityMonitor";

function ExecuteCombatAbilitiesBeforeDamage(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::BEFORE_DAMAGE, attackInfo);
endfunction

function ExecuteCombatAbilitiesOnHit(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_HIT, attackInfo);
endfunction

function ExecuteCombatAbilitiesAfterDamage(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::AFTER_DAMAGE, attackInfo);
endfunction

function ExecuteCombatAbilitiesOnCritical(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_CRITICAL, attackInfo);
endfunction

function ExecuteCombatAbilitiesOnKill(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_KILL, attackInfo);
endfunction

function ExecuteCombatAbilitiesBeforeHit(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::BEFORE_HIT, attackInfo);
endfunction

function ExecuteCombatAbilitiesOnMiss(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_MISS, attackInfo);
endfunction

function ExecuteCombatAbilitiesOnMissAttacker(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_MISS, attackInfo);
endfunction

function ExecuteCombatAbilitiesOnParry(byRef attackInfo)
	_ExecuteCombatAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_PARRY, attackInfo);
endfunction

function _ExecuteCombatAbilities(byRef mobile, activationMoment, byRef attackInfo)
	var abilitiesList := mobile.GetActiveAbilities()[activationMoment];

	if (!abilitiesList)
		return 0;
	endif

	var abilitiesExecuted := 0;

	foreach abilityKey in (abilitiesList.keys())
		var serializedMonitor := abilitiesList[abilityKey];
		var abilityId := serializedMonitor.AbilityId;

		if (abilityId)
			var execResult := run_script_to_completion(":abilities:AbilityController", array{"active", abilityId, attackInfo});
			if (!execResult || execResult.errortext)
				continue;
			endif

			if (execResult.executed)
				abilitiesExecuted++;
			endif

			if (execResult.executed && execResult.singleUse && serializedMonitor.Pid)
				var monitorProcess := GetProcess(serializedMonitor.Pid);
				if (monitorProcess)
					monitorProcess.SendEvent(ABILITY_EVENT_ID::COMPLETE);
				endif
			endif
		endif
	endforeach

	return abilitiesExecuted;
endfunction

function ExecutePassiveAbilitiesOnParry(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::ON_PARRY, attackInfo);
endfunction

function ExecutePassiveAbilitiesOnKill(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_KILL, attackInfo);
endfunction

function ExecutePassiveAbilitiesAfterDamage(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::AFTER_DAMAGE, attackInfo);
endfunction

function ExecutePassiveAbilitiesAttackerBeforeDamage(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::BEFORE_DAMAGE, attackInfo);
endfunction

function ExecutePassiveAbilitiesOnMissAttacker(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::ON_MISS, attackInfo);
endfunction

function ExecutePassiveAbilitiesDefenderBeforeDamage(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::BEFORE_DAMAGE, attackInfo);
endfunction

function ExecutePassiveAbilitiesAttackerBeforeHit(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetAttacker(), ABILITY_ACTIVATION::BEFORE_HIT, attackInfo);
endfunction

function ExecutePassiveAbilitiesDefenderBeforeHit(byRef attackInfo)
	_ExecutePassiveAbilities(attackInfo.GetDefender(), ABILITY_ACTIVATION::BEFORE_HIT_DEFENDER, attackInfo);
endfunction

function ExecutePassiveAbilitiesOnEquip(byRef mobile, byRef equip)
	_ExecutePassiveAbilities(mobile, ABILITY_ACTIVATION::ON_EQUIP, equip);
endfunction

function ExecutePassiveAbilitiesOnUnequip(byRef mobile, byRef equip)
	_ExecutePassiveAbilities(mobile, ABILITY_ACTIVATION::ON_UNEQUIP, equip);
endfunction

function _ExecutePassiveAbilities(byRef mobile, passiveMoment, byRef attackInfo)
	var abilities := mobile.GetAbilities();

	if (!abilities)
		return 0;
	endif

	// Which side is this mobile playing in the current attack? The wrappers
	// above us (ExecutePassiveAbilitiesAttacker* / Defender*) already encoded
	// the side by which mobile they passed — we project it back out so the
	// AbilityService gate can compare against ability.IsDefenderSide.
	var dispatchedToDefender :=
		(mobile.GetObjRef().serial != attackInfo.GetAttacker().GetObjRef().serial);

	var abilitiesExecuted := 0;

	foreach level in (abilities.keys())
		var abilityId := CInt(abilities[level]);
		var execResult := run_script_to_completion(":abilities:AbilityController", array{"passive", abilityId, attackInfo, passiveMoment, dispatchedToDefender});
		if (!execResult || execResult.errortext)
			continue;
		endif

		if (execResult.executed)
			abilitiesExecuted++;
		endif
	endforeach

	return abilitiesExecuted;
endfunction
