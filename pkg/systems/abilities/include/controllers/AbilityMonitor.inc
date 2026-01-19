use os;

include ":abilities:models/Ability";
include ":wrappers:Objects/Model/MobileWrapper";
include ":message:model/Message";
include "system/exception/ProcessExeception/InvalidProcessException";

class AbilityMonitor()
	var Pid := 0;
	var ActivationTime := 0;
	var MobileSerial := 0x0;
	var AbilityId := 0;
	var Charges := 0;

	function AbilityMonitor(this, ability)
		this.AbilityId := ability.GetId();
		this.Charges := ability.GetCharges();
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
		return Ability(this.AbilityId);
	endfunction

	function GetCharges(this)
		return this.Charges;
	endfunction

	function SetCharges(this, amount)
		this.Charges := amount;
	endfunction

	function ConsumeCharge(this, amount)
		if (this.GetCharges() > 0)
			this.Charges -= amount;
		endif

		if (this.GetCharges() <= 0)
			this.Charges := 0;
		endif

		return this;
	endfunction

	function ActivateOn(this, mobile)
		print($"[ABILITY_MONITOR] ActivateOn called, abilityId={this.AbilityId}");
		if (!(mobile is @MobileWrapper))
			print("[ABILITY_MONITOR] ERROR: mobile is not MobileWrapper");
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		var ability := this.GetAbility();
		print($"[ABILITY_MONITOR] Got ability: {ability.GetName()}");
		var mobileRef := mobile.GetObjRef();

		// === PRE-ACTIVATION VALIDATION ===
		print("[ABILITY_MONITOR] Starting pre-activation validation");

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
		var cooldownProp := $"AbilityCooldown_{ability.GetId()}";
		var cooldownEnd := mobile.GetProperty(cooldownProp);
		if (cooldownEnd && POLCore().systime < cooldownEnd)
			var remaining := cooldownEnd - POLCore().systime;
			Message($"Aguarde {remaining}s para usar novamente.", Color(SYSTEM_COLOR::FAIL)).SendSys(mobileRef);
			return error{"errortext" := "Ability on cooldown"};
		endif
		print("[ABILITY_MONITOR] Validation passed");

		// === CONSUME COSTS ===
		// Commenting jsut for tests
		// if (ability.GetCostSTAM() > 0)
		// 	mobile.ConsumeStamina(ability.GetCostSTAM());
		// endif
		// if (ability.GetCostMANA() > 0)
		// 	mobile.ConsumeMana(ability.GetCostMANA());
		// endif
		// if (ability.GetCostHP() > 0)
		// 	mobile.ConsumeHP(ability.GetCostHP());
		// endif

		// === SET COOLDOWN ===
		// Commenting just so I can test abilities faster
		// if (ability.GetCooldown() > 0)
		// 	SetObjProperty(mobileRef, cooldownProp, POLCore().systime + ability.GetCooldown());
		// endif

		// === ACTIVATE ===
		print("[ABILITY_MONITOR] === ACTIVATE section ===");
		this.MobileSerial := mobile.GetSerial();
		this.ActivationTime := POLCore().systime;
		print($"[ABILITY_MONITOR] MobileSerial={this.MobileSerial}, checking for existing monitor...");

		var existingMonitor := mobile.GetAbilityMonitor(ability);
		print($"[ABILITY_MONITOR] existingMonitor result: {existingMonitor}");
		if (existingMonitor is @AbilityMonitor)
			print($"[ABILITY_MONITOR] Refreshing existing AbilityMonitor script for ability {ability.GetId()}");
			existingMonitor._GetProcess().SendEvent(ABILITY_EVENT_ID::REFRESH);
		else
			print($"[ABILITY_MONITOR] Starting AbilityMonitor script for ability {ability.GetId()}");
			var script := Start_Script($"{ABILITY_PKG}AbilityMonitor", array{mobile, this});
			print($"[ABILITY_MONITOR] Script started: {script}");
		endif
	endfunction

	function Deactivate(this)
		if (!this.pid)
			return InvalidProcessException("AbilityMonitor Deactivate called with no active process.");
		endif

		this._GetProcess().SendEvent(ABILITY_EVENT_ID::ABORT);
	endfunction

	function ToString(this)
		return $"AbilityMonitor(abilityId={this.abilityId}, mobileSerial={this.mobileSerial}, pid={this.pid}, charges={this.charges})";
	endfunction
endclass

enum class ABILITY_EVENT_ID
	EXPIRE,
	REFRESH,
	USED,
	ABORT,
	SET_DURATION
endenum