use os;

include "IMasteryAbility";
include ":IO:CFGFile";

class MasteryAbility(IMasteryAbility)
	var Id;
	var Name;
	var Description;
	var ActivationMomentum;
	
	function MasteryAbility(this, abilityId)
		var cfgFile := CFGFile("mastery", "configs/abilities.cfg", false).SetElementName(abilityId);
		this.Id := abilityId;
		this.Name := cfgFile.GetPropertyValue("Name");
		this.Description := cfgFile.GetPropertyValue("Description");
		this.ActivationMomentum := cfgFile.GetPropertyValue("ActivationMomentum");
	endfunction

	function GetId(this)
		return this.Id;
	endfunction

	function GetName(this)
		return this.Name;
	endfunction

	function _SetName(this, value)
		this.Name := CStr(value);

		return this;
	endfunction

	function GetDescription(this)
		return this.Description;
	endfunction

	function SetDescription(this, value)
		this.Description := CStr(value);

		return this;
	endfunction

	function Execute(this, attackInfo)
		Start_Script(":combat:specialAttackScripts/" + Lower(this.GetName()), attackInfo);
	endfunction

endclass