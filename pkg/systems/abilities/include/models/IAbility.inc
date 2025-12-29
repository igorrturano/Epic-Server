class IAbility()
	uninit function IAbility(this, abilityEnum);

	uninit function GetId(this);
	uninit function GetName(this);
	uninit function GetClass(this);
	uninit function GetLevel(this);
	uninit function GetFolk(this);
	uninit function GetDescription(this);
	uninit function GetActivationMode(this);
	uninit function GetActivationMoment(this);
	uninit function GetRequirement(this);
	uninit function GetCostHP(this);
	uninit function GetCostSTAM(this);
	uninit function GetCostMANA(this);
	uninit function GetCharges(this);
	uninit function GetCooldown(this);
	uninit function GetEffectDuration(this);
	uninit function GetScript(this);

	uninit function ExecuteOn(this, targetInfo);
endclass