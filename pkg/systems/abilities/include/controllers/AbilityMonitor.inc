use os;

include ":abilities:AbilityRegistry";
include ":abilities:models/Ability";
include ":wrappers:Objects/Model/MobileWrapper";
include ":message:model/Message";
include "system/exception/ProcessExeception/InvalidProcessException";

class AbilityMonitor()
	var Pid := 0;
	var ActivationTime := 0;
	var MobileSerial := 0x0;
	var AbilityId := 0;
	var Duration := 0;

	function AbilityMonitor(this, ability)
		this.AbilityId := ability.GetId();
	endfunction

	function GetDuration(this)
		return this.Duration;
	endfunction

	function SetDuration(this, duration)
		this.Duration := duration;
	endfunction

	function GetPid(this)
		return this.Pid;
	endfunction

	function SetPid(this, pid)
		this.Pid := pid;
	endfunction

	function _GetProcess(this)
		if (!this.Pid)
			return InvalidProcessException("AbilityMonitor GetProcess called with no active process.");
		endif

		return GetProcess(this.Pid);
	endfunction

	function GetMobileSerial(this)
		return this.MobileSerial;
	endfunction

	function GetAbilityId(this)
		return this.AbilityId;
	endfunction

	function SetAbilityId(this, abilityId)
		this.AbilityId := abilityId;
	endfunction

	function GetAbility(this)
		return CreateAbility(this.AbilityId);
	endfunction

	function ActivateOn(this, mobile)
		if (!(mobile is @MobileWrapper))
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		var ability := Ability(this.AbilityId);
		if (!ability.GetName())
			Message("Habilidade não implementada.", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile.GetObjRef());
			return error{"errortext" := $"Ability {this.AbilityId} not found in config"};
		endif

		var mobileRef := mobile.GetObjRef();

		// Validate stamina cost
		if (ability.GetCostSTAM() > 0 && ability.GetCostSTAM() > mobile.GetCurrentStamina())
			Message("Stamina insuficiente!", Color(SYSTEM_COLOR::FAIL)).SendSys(mobileRef);
			return error{"errortext" := "Insufficient stamina"};
		endif

		// Validate mana cost
		if (ability.GetCostMANA() > 0 && ability.GetCostMANA() > mobile.GetCurrentMana())
			Message("Mana insuficiente!", Color(SYSTEM_COLOR::FAIL)).SendSys(mobileRef);
			return error{"errortext" := "Insufficient mana"};
		endif

		// Validate HP cost
		if (ability.GetCostHP() > 0 && ability.GetCostHP() > mobile.GetCurrentHP())
			Message("HP insuficiente!", Color(SYSTEM_COLOR::FAIL)).SendSys(mobileRef);
			return error{"errortext" := "Insufficient HP"};
		endif

		// Validate cooldown
		if (mobile.IsAbilityOnCooldown(ability.GetId()))
			var remaining := mobile.GetAbilityCooldownRemaining(ability.GetId());
			Message($"Aguarde {remaining}s para usar novamente.", Color(SYSTEM_COLOR::FAIL)).SendSys(mobileRef);
			return error{"errortext" := "Ability on cooldown"};
		endif

		if (!mobile.GetObjRef().cmdlevel)
			// === CONSUME COSTS ===
			if (ability.GetCostSTAM() > 0)
				mobile.ConsumeStamina(ability.GetCostSTAM());
			endif
			if (ability.GetCostMANA() > 0)
				mobile.ConsumeMana(ability.GetCostMANA());
			endif
			if (ability.GetCostHP() > 0)
				mobile.ConsumeHP(ability.GetCostHP());
			endif

			// === SET COOLDOWN ===
			if (ability.GetAbilityCooldown() > 0)
				mobile.SetAbilityCooldown(ability.GetId(), ability.GetAbilityCooldown());
			endif
		endif

		this.Duration := ability.GetDurationFor(mobile);

		// === ACTIVATE ===
		this.MobileSerial := mobile.GetSerial();
		this.ActivationTime := POLCore().systime;

		var existingMonitor := mobile.GetAbilityMonitor(ability);
		if (existingMonitor is @AbilityMonitor)
			existingMonitor._GetProcess().SendEvent(ABILITY_EVENT_ID::REFRESH);
		else
			Start_Script($"{ABILITY_PKG}AbilityMonitor", array{mobile, this});
		endif
	endfunction

	function Deactivate(this)
		if (!this.pid)
			return InvalidProcessException("AbilityMonitor Deactivate called with no active process.");
		endif

		this._GetProcess().SendEvent(ABILITY_EVENT_ID::ABORT);
	endfunction

	function ToString(this)
		return $"AbilityMonitor(abilityId={this.AbilityId}, mobileSerial={this.MobileSerial}, pid={this.Pid})";
	endfunction
endclass

enum class ABILITY_EVENT_ID
	EXPIRE,
	REFRESH,
	ABORT,
	COMPLETE // Single-use abilities end silently after execution
endenum