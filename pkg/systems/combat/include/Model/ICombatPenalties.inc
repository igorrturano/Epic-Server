class ICombatPenalties()
	uninit function GetAttributePenalty(this);
	uninit function GetSkillPenalty(this);

	uninit function SetAttributePenalty(this, value);
	uninit function SetSkillPenalty(this, value);

	uninit function IncreaseAttributePenalty(this, value);
	uninit function IncreaseSkillPenalty(this, value);

	uninit function DecreaseAttributePenalty(this, value);
	uninit function DecreaseSkillPenalty(this, value);

	uninit function ToString(this);
endclass