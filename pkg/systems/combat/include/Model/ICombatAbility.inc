class ICombatAbility()
	uninit function ICombatAbility(this);

	uninit function SetName(this, name);
	uninit function SetDescription(this, description);
	uninit function SetMomentum(this, momentum);
	uninit function SetCooldown(this, cooldown);
	uninit function SetRange(this, range);
	uninit function SetDamage(this, damage);

	uninit function GetName(this);
	uninit function GetDescription(this);
	uninit function GetMomentum(this);
	uninit function GetCooldown(this);
	uninit function GetRange(this);
	uninit function GetDamage(this);
endclass