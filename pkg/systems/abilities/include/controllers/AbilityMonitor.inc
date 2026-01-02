use os;

include ":abilities:models/Ability";
include ":wrappers:Objects/Model/MobileWrapper";
include "system/exception/ProcessExeception/InvalidProcessException";

class AbilityMonitor()
	var pid := 0;
	var mobileSerial := 0x0;
	var abilityId := 0;
	var charges := 0;

	function AbilityMonitor(this, ability)
		this.abilityId := ability.GetId();
		this.charges := ability.GetCharges();
	endfunction

	function GetPid(this)
		return this.pid;
	endfunction

	function SetPid(this, pid)
		this.pid := pid;
	endfunction

	function _GetProcess(this)
		if (!this.pid)
			return InvalidProcessException("AbilityMonitor GetProcess called with no active process.");
		endif

		return GetProcess(this.pid);
	endfunction

	function GetMobileSerial(this)
		return this.mobileSerial;
	endfunction

	function GetAbilityId(this)
		return this.abilityId;
	endfunction

	function SetAbilityId(this, abilityId)
		this.abilityId := abilityId;
	endfunction

	function GetAbility(this)
		return Ability(this.abilityId);
	endfunction

	function GetCharges(this)
		return this.charges;
	endfunction

	function SetCharges(this, amount)
		this.charges := amount;
	endfunction

	function ActivateOn(this, mobile)
		this.mobileSerial := mobile.GetSerial();
		var script := Start_Script($"{ABILITY_PKG}/AbilityMonitor", array{mobile, this});
	endfunction

	function Deactivate(this)
		if (!this.pid)
			return InvalidProcessException("AbilityMonitor Deactivate called with no active process.");
		endif

		this._GetProcess().sendevent(ABILITY_EVENT_ID::ABORT);
	endfunction

	function ToString(this)
		return $"AbilityMonitor(abilityId={this.abilityId}, mobileSerial={this.mobileSerial}, pid={this.pid}, charges={this.charges})";
	endfunction
endclass

enum class ABILITY_EVENT_ID
	ABORT,
	EXPIRE,
	REFRESH
endenum