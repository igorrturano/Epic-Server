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
include ":message:model/Message";
include ":attributes:attributes";
include ":hotbar:include/cooldown_packet";

function _ControllerResult(executed, singleUse)
	var result := struct{};
	result.+executed := executed;
	result.+singleUse := singleUse;
	return result;
endfunction

function _ActivationError(errorMsg)
	var result := struct{};
	result.+hasError := 1;
	result.+errorMsg := errorMsg;
	return result;
endfunction

function _RollbackActivation(who, abilityId)
	var ability := Ability(abilityId);
	var mobileRef := who.GetObjRef();
	EraseObjProperty(mobileRef, $"AbilityCooldown_{abilityId}");
	SendCooldownPacket(mobileRef, ability.GetName(), 0, ability.GetIcon(), COOLDOWN_TYPE_ABILITY);
	if (ability.GetCostSTAM() > 0)
		who.RestoreStamina(ability.GetCostSTAM());
	endif
	if (ability.GetCostMANA() > 0)
		who.RestoreMana(ability.GetCostMANA());
	endif
	if (ability.GetCostHP() > 0)
		who.RestoreHP(ability.GetCostHP());
	endif
endfunction

function _ActivationSuccess(duration, name)
	var result := struct{};
	result.+hasError := 0;
	result.+errorMsg := "";
	result.+duration := duration;
	result.+name := name;
	return result;
endfunction

function RunAbilityExecutor(params)
	var action := params[1];

	case (action)
		"active":     return _RunControllerActive(params);
		"passive":    return _RunControllerPassive(params);
		"query_is_on_use": return _RunMonitorQueryIsOnUse(params);
		"activate":   return _RunMonitorActivate(params);
		"activate_async": return _RunMonitorActivateAsync(params);
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
		ABILITY_ACTIVATION::ON_EQUIP:
			triggered := ability.OnPassiveOnEquip(attackInfo);
		ABILITY_ACTIVATION::ON_UNEQUIP:
			triggered := ability.OnPassiveOnUnequip(attackInfo);
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

	// Re-wrap the mobile in this script's context. The wrapper passed via
	// run_script_to_completion has its fields intact but method dispatch is
	// broken on the receiver side, so any subsequent ability.CanActivate(who)
	// or who.ActivateAbility(...) call would crash silently. Building a fresh
	// MobileWrapper here gives us a wrapper whose methods dispatch correctly
	// for the rest of this function.
	if (who.ObjRef)
		who := MobileWrapper(who.ObjRef);
	else
		return _ActivationError("Mobile not found.");
	endif

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

	// Field access — monitor is the stale param instance; method dispatch on
	// it would crash the script the same way wrapper methods do.
	var duration := monitor.Duration;
	if (duration <= 0)
		duration := ability.GetDurationFor(who);
	endif
	if (duration <= 0)
		duration := 1;
	endif

	return _ActivationSuccess(duration, ability.GetName());
endfunction

// Query-only helper. This does not activate ability effects.
function _RunMonitorQueryIsOnUse(params)
	var abilityId := CInt(params[2]);

	if (!ABILITY_CLASSES.exists(abilityId))
		return 0;
	endif

	var ability := ABILITY_CLASSES[abilityId].new();
	if (ability.errortext)
		return 0;
	endif

	if (ability.GetActivationMoment() == ABILITY_ACTIVATION::ON_USE)
		return 1;
	endif

	return 0;
endfunction

function _RunMonitorActivateAsync(params)
	var abilityId := CInt(params[2]);
	var who := params[3];

	// Re-wrap so any field-access fallbacks below have a working wrapper.
	if (who.ObjRef)
		who := MobileWrapper(who.ObjRef);
	endif

	var result := _RunMonitorActivate(params);
	if (result.hasError)
		Message(result.errorMsg, Color(SYSTEM_COLOR::FAIL)).SendSys(who.GetObjRef());
		_RollbackActivation(who, abilityId);
		return 0;
	endif

	// Execute the ON_USE ability (may block for targeting)
	var ability := ABILITY_CLASSES[abilityId].new();
	ability.ExecuteOn(who);
	who.DeactivateAbility(ability);

	return 1;
endfunction

function _RunMonitorDeactivate(params)
	var abilityId := CInt(params[2]);
	var who := params[3];

	// Re-wrap so method dispatch works in this script context.
	if (who.ObjRef)
		who := MobileWrapper(who.ObjRef);
	endif

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
