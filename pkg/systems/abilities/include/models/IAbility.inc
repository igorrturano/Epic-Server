// V3 Ability Interface - metadata and lifecycle hooks
class IAbility()
	// Metadata getters
	uninit function GetId(this);
	uninit function GetName(this);
	uninit function GetClass(this);
	uninit function GetFolk(this);
	uninit function GetDescription(this);
	uninit function GetActivationMoment(this);
	uninit function GetRequirement(this);
	uninit function GetCostHP(this);
	uninit function GetCostSTAM(this);
	uninit function GetCostMANA(this);
	uninit function GetAbilityCooldown(this);
	uninit function GetDuration(this);
	uninit function GetIcon(this);

	// V3 lifecycle hooks
	uninit function CanActivate(this, mobile);
	uninit function OnActivate(this, mobile);
	uninit function GetDurationFor(this, mobile);
	uninit function ExecuteOn(this, attackInfo);
endclass
