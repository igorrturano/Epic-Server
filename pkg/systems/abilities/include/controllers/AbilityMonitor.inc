use os;

include ":abilities:models/Ability";
include ":wrappers:Objects/Model/MobileWrapper";
include "system/exception/ProcessExeception/InvalidProcessException";

class AbilityMonitor()
	var Pid := 0;
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
		if (!(mobile is @MobileWrapper))
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		this.mobileSerial := mobile.GetSerial();

		var existingMonitor := mobile.GetActiveAbilityMonitor(this.GetAbility());
		if (existingMonitor is @AbilityMonitor)
			existingMonitor._GetProcess().SendEvent(ABILITY_EVENT_ID::REFRESH);
		endif
		
		var script := Start_Script($"{ABILITY_PKG}/AbilityMonitor", array{mobile, this});
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
	ABORT
endenum