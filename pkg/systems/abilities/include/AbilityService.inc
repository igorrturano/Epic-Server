/***********************************
 *
 * AbilityService.inc
 *
 * Defines AbilityExecutionController — class invoked by every per-archetype
 * AbilityExecutor*.src to handle the cross-script-boundary ability lifecycle.
 *
 * Each AbilityExecutor*.src is a thin program that does:
 *     return AbilityExecutionController().Run(params);
 *
 * Routing (params[1] is a string action — necessary because the entry point
 * is a Start_Script call, which can't dispatch to methods directly):
 *   "active"          - Combat active hook (called from CombatAbilityController)
 *   "passive"         - Combat passive hook
 *   "query_is_on_use" - Cheap metadata query, no side effects
 *   "activate"        - Monitor activation lifecycle (cost/cooldown handled by AbilityMonitor.inc)
 *   "activate_async"  - Same, but for ON_USE abilities that block on client targeting
 *   "deactivate"      - Monitor deactivation
 *
 * Returns are structs (not class instances) — class instances would lose
 * their method tables on the trip back through run_script_to_completion.
 * Active/passive: struct{executed, singleUse}.
 * Activate paths: struct{hasError, errorMsg, duration, name}.
 *
 **********************************/

include ":abilities:controllers/AbilityMonitor";
include ":abilities:models/Ability";
include ":message:model/Message";

class AbilityExecutionController()

	function AbilityExecutionController(this)
	endfunction

	// Entry point — translates the string action from params[1] to the
	// appropriate method. Kept as the single boundary between the Start_Script
	// world (string-keyed dispatch) and the OOP world (typed method calls).
	function Run(this, params)
		var action := params[1];

		case (action)
			"active":
				return this.RunActive(CInt(params[2]), params[3]);
			"passive":
				var moment := "";
				if (params.size() >= 4)
					moment := params[4];
				endif
				// params[5] is dispatchedToDefender (0/1) — present when called
				// from _ExecutePassiveAbilities, absent for legacy call sites.
				var dispatchedToDefender := 0;
				if (params.size() >= 5)
					dispatchedToDefender := CInt(params[5]);
				endif
				return this.RunPassive(CInt(params[2]), moment, params[3], dispatchedToDefender);
			"query_is_on_use":
				return this.QueryIsOnUse(CInt(params[2]));
			"activate":
				return this.Activate(CInt(params[2]), params[3], params[4]);
			"activate_async":
				return this.ActivateAsync(CInt(params[2]), params[3], params[4]);
			"deactivate":
				return this.Deactivate(CInt(params[2]), params[3]);
		endcase

		return _ResultExecuted(0, 0);
	endfunction

	// ------------------------------------------------------------------------
	// Combat hooks — fired by CombatAbilityController on every hit/miss/parry.
	// ------------------------------------------------------------------------

	function RunActive(this, abilityId, attackInfo)
		var ability := this._InstantiateAbility(abilityId);
		if (!ability)
			return _ResultExecuted(0, 0);
		endif

		var executed := ability.ExecuteOn(attackInfo) ? 1 : 0;
		var singleUse := ability.IsSingleUse() ? 1 : 0;
		return _ResultExecuted(executed, singleUse);
	endfunction

	function RunPassive(this, abilityId, moment, attackInfo, dispatchedToDefender)
		var ability := this._InstantiateAbility(abilityId);
		if (!ability)
			return _ResultExecuted(0, 0);
		endif

		// Side gate — ability declares which side it applies to via
		// IsDefenderSide (default false = attacker). Dispatcher tells us which
		// side it iterated. Mismatch → skip, so a defender-side passive owned by
		// the attacker can't fire as if the attacker were defending.
		if (CInt(ability.IsDefenderSide) != CInt(dispatchedToDefender))
			print($"[PASSIVE-SIDE-GATE] SKIP id={abilityId} ability.IsDefenderSide={ability.IsDefenderSide} dispatchedToDefender={dispatchedToDefender} moment={moment}");
			return _ResultExecuted(0, 0);
		endif

		print($"[PASSIVE-SIDE-GATE] FIRE id={abilityId} side={dispatchedToDefender ? "DEFENDER" : "ATTACKER"} moment={moment}");

		var triggered := 0;
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

		return _ResultExecuted(triggered ? 1 : 0, 0);
	endfunction

	// ------------------------------------------------------------------------
	// Monitor lifecycle — called from AbilityMonitor.src.
	// ------------------------------------------------------------------------

	function QueryIsOnUse(this, abilityId)
		var ability := this._InstantiateAbility(abilityId);
		if (!ability)
			return 0;
		endif
		return ability.GetActivationMoment() == ABILITY_ACTIVATION::ON_USE ? 1 : 0;
	endfunction

	function Activate(this, abilityId, mobile, monitor)
		// Re-wrap: param wrapper from Start_Script has broken method dispatch.
		if (mobile.ObjRef)
			mobile := MobileWrapper(mobile.ObjRef);
		else
			return _ResultActivationError("Mobile not found.");
		endif

		var ability := this._InstantiateAbility(abilityId);
		if (!ability)
			return _ResultActivationError("Habilidade não implementada.");
		endif

		if (!ability.CanActivate(mobile))
			return _ResultActivationError("Condições para ativar habilidade não atendidas.");
		endif

		SetScriptController(mobile.GetObjRef());

		mobile.ActivateAbility(ability, monitor);
		ability.OnActivate(mobile);

		// monitor is the stale param instance; field access only.
		var duration := monitor.Duration;
		if (duration <= 0)
			duration := ability.GetDurationFor(mobile);
		endif
		if (duration <= 0)
			duration := 1;
		endif

		return _ResultActivationSuccess(duration, ability.GetName());
	endfunction

	function ActivateAsync(this, abilityId, mobile, monitor)
		// Re-wrap before any method calls; Activate re-wraps its own copy too.
		if (mobile.ObjRef)
			mobile := MobileWrapper(mobile.ObjRef);
		endif

		var result := this.Activate(abilityId, mobile, monitor);
		if (result.hasError)
			Message(result.errorMsg, Color(SYSTEM_COLOR::FAIL)).SendSys(mobile.GetObjRef());
			mobile.RollbackAbilityActivation(abilityId);
			return 0;
		endif

		// Execute the ON_USE ability now (may block for client targeting).
		var ability := this._InstantiateAbility(abilityId);
		ability.ExecuteOn(mobile);
		mobile.DeactivateAbility(ability);

		return 1;
	endfunction

	function Deactivate(this, abilityId, mobile)
		if (mobile.ObjRef)
			mobile := MobileWrapper(mobile.ObjRef);
		endif

		var ability := this._InstantiateAbility(abilityId);
		if (!ability)
			return 0;
		endif

		mobile.DeactivateAbility(ability);
		return 1;
	endfunction

	// ------------------------------------------------------------------------
	// Internal: returns a fresh CombatAbility subclass instance, or 0 on any
	// failure (unknown id, constructor error). Centralized so the four
	// _Result* paths above don't each redo the lookup-and-validate dance.
	// ------------------------------------------------------------------------
	function _InstantiateAbility(this, abilityId)
		if (!ABILITY_CLASSES.exists(abilityId))
			return 0;
		endif
		var ability := ABILITY_CLASSES[abilityId].new();
		if (ability.errortext)
			return 0;
		endif
		return ability;
	endfunction

endclass

// ----------------------------------------------------------------------------
// Result struct factories. Kept as free functions because the values cross the
// Start_Script boundary back to AbilityMonitor.src / CombatAbilityController —
// class instances would lose their method tables in transit, structs survive.
// ----------------------------------------------------------------------------

function _ResultExecuted(executed, singleUse)
	var result := struct{};
	result.+executed := executed;
	result.+singleUse := singleUse;
	return result;
endfunction

function _ResultActivationError(errorMsg)
	var result := struct{};
	result.+hasError := 1;
	result.+errorMsg := errorMsg;
	return result;
endfunction

function _ResultActivationSuccess(duration, name)
	var result := struct{};
	result.+hasError := 0;
	result.+errorMsg := "";
	result.+duration := duration;
	result.+name := name;
	return result;
endfunction
