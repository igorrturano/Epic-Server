/***********************************
 *
 * AbilityService.inc
 *
 * Shared logic for archetype-specific ability executors.
 * Each executor .src includes its partial registry + this file.
 *
 * Handles:
 *   "active"/"passive" - Combat ability controller (from AbilityController.src)
 *   "activate"         - Monitor activation lifecycle (from AbilityMonitor.src)
 *   "deactivate"       - Monitor deactivation (from AbilityMonitor.src)
 *
 **********************************/

include ":abilities:controllers/AbilityMonitor";
include ":abilities:models/Ability";

function _ControllerResult(executed, singleUse)
	var result := struct{};
	result.+executed := executed;
	result.+singleUse := singleUse;
	return result;
endfunction

function _ActivationError(errorMsg)
	var result := struct{};
	result.+error := 1;
	result.+errorMsg := errorMsg;
	result.+isOnUse := 0;
	result.+duration := 0;
	result.+name := "";
	return result;
endfunction

function _ActivationSuccess(isOnUse, duration, name)
	var result := struct{};
	result.+error := 0;
	result.+errorMsg := "";
	result.+isOnUse := isOnUse;
	result.+duration := duration;
	result.+name := name;
	return result;
endfunction

function RunAbilityExecutor(params)
	var action := params[1];

	case (action)
		"active":     return _RunControllerActive(params);
		"passive":    return _RunControllerPassive(params);
		"activate":   return _RunMonitorActivate(params);
		"deactivate": return _RunMonitorDeactivate(params);
	endcase

	return _ControllerResult(0, 0);
endfunction

// ========================================
// Controller logic (extracted from AbilityController.src)
// ========================================

function _RunControllerActive(params)
	var abilityId := CInt(params[2]);
	var attackInfo := params[3];

	if (!ABILITY_CLASSES.exists(abilityId))
		return _ControllerResult(0, 0);
	endif

	var ability := ABILITY_CLASSES[abilityId].new();
	if (ability.errortext)
		return _ControllerResult(0, 0);
	endif

	var executed := ability.ExecuteOn(attackInfo);
	var singleUse := 0;
	var executedInt := 0;
	if (ability.IsSingleUse())
		singleUse := 1;
	endif
	if (executed)
		executedInt := 1;
	endif
	return _ControllerResult(executedInt, singleUse);
endfunction

function _RunControllerPassive(params)
	var abilityId := CInt(params[2]);
	var attackInfo := params[3];
	var moment := "";

	if (params.size() >= 4)
		moment := params[4];
	endif

	if (!ABILITY_CLASSES.exists(abilityId))
		return _ControllerResult(0, 0);
	endif

	var ability := ABILITY_CLASSES[abilityId].new();
	if (ability.errortext)
		return _ControllerResult(0, 0);
	endif

	var triggered := 0;
	var triggeredInt := 0;
	case (moment)
		ABILITY_ACTIVATION::ON_PARRY:
			triggered := ability.OnPassiveOnParry(attackInfo);
		ABILITY_ACTIVATION::ON_KILL:
			triggered := ability.OnPassiveOnKill(attackInfo);
		ABILITY_ACTIVATION::ON_MISS:
			triggered := ability.OnPassiveOnMiss(attackInfo);
		ABILITY_ACTIVATION::BEFORE_DAMAGE:
			triggered := ability.OnPassiveBeforeDamage(attackInfo);
		ABILITY_ACTIVATION::AFTER_DAMAGE:
			triggered := ability.OnPassiveAfterDamage(attackInfo);
		ABILITY_ACTIVATION::BEFORE_HIT:
			triggered := ability.OnPassiveBeforeHitAttacker(attackInfo);
		ABILITY_ACTIVATION::BEFORE_HIT_DEFENDER:
			triggered := ability.OnPassiveBeforeHitDefender(attackInfo);
	endcase

	if (triggered)
		triggeredInt := 1;
	endif

	return _ControllerResult(triggeredInt, 0);
endfunction

// ========================================
// Monitor activation logic (extracted from AbilityMonitor.src)
// ========================================

function _RunMonitorActivate(params)
	var abilityId := CInt(params[2]);
	var who := params[3];
	var monitor := params[4];

	if (!ABILITY_CLASSES.exists(abilityId))
		return _ActivationError("Habilidade não implementada.");
	endif

	var ability := ABILITY_CLASSES[abilityId].new();
	if (ability.errortext)
		return _ActivationError("Habilidade não implementada.");
	endif

	if (!ability.CanActivate(who))
		return _ActivationError("Condições para ativar habilidade não atendidas.");
	endif

	SetScriptController(who.GetObjRef());

	who.ActivateAbility(ability, monitor);
	ability.OnActivate(who);

	// ON_USE abilities execute immediately and end
	if (ability.GetActivationMoment() == ABILITY_ACTIVATION::ON_USE)
		ability.ExecuteOn(who);
		who.DeactivateAbility(ability);
		return _ActivationSuccess(1, 0, "");
	endif

	var duration := monitor.GetDuration();
	if (duration <= 0)
		duration := ability.GetDurationFor(who);
	endif
	if (duration <= 0)
		duration := 1;
	endif

	return _ActivationSuccess(0, duration, ability.GetName());
endfunction

function _RunMonitorDeactivate(params)
	var abilityId := CInt(params[2]);
	var who := params[3];

	if (!ABILITY_CLASSES.exists(abilityId))
		return 0;
	endif

	var ability := ABILITY_CLASSES[abilityId].new();
	if (ability.errortext)
		return 0;
	endif

	who.DeactivateAbility(ability);
	return 1;
endfunction
