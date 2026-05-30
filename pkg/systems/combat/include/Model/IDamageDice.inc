class IDamageDice()
	uninit function IDamageDice(this, default slash, default piercing, default blunt, default fire, default cold, default poison, default energy);

	uninit function GetSlashDamage(this);
	uninit function GetPiercingDamage(this);
	uninit function GetBluntDamage(this);
	uninit function GetFireDamage(this);
	uninit function GetColdDamage(this);
	uninit function GetPoisonDamage(this);
	uninit function GetEnergyDamage(this);

	uninit function SetSlashDamage(this, value);
	uninit function SetPiercingDamage(this, value);
	uninit function SetBluntDamage(this, value);
	uninit function SetFireDamage(this, value);
	uninit function SetColdDamage(this, value);
	uninit function SetPoisonDamage(this, value);
	uninit function SetEnergyDamage(this, value);

	uninit function WithUpgradedPhysicalDice(this);

	uninit function ToString(this);
endclass