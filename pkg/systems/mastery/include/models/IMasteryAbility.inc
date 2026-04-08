class IMasteryAbility()
	uninit function IMasteryAbility(this, activationMomentum); // ATTACK_MOMENT::ENGAGING or ATTACK_MOMENT::COMBATING

	uninit function GetId(this);
	uninit function GetName(this);
	uninit function _SetName(this, value);
	uninit function GetDescription(this);
	uninit function SetDescription(this, value);
	uninit function GetActivationMomentum(this);

	uninit function Execute(this, attackInfo);
endclass