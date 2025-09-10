class IDamage()
	uninit function IDamage(this,
		default slash,
		default piercing,
		default blunt,

		default fire,
		default cold,
		default poison,
		default energy
	);
	
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

	uninit function IncreaseSlashDamage(this, value);
	uninit function IncreasePiercingDamage(this, value);
	uninit function IncreaseBluntDamage(this, value);
	uninit function IncreaseFireDamage(this, value);
	uninit function IncreaseColdDamage(this, value);
	uninit function IncreasePoisonDamage(this, value);
	uninit function IncreaseEnergyDamage(this, value);

	uninit function DecreaseSlashDamage(this, value);
	uninit function DecreasePiercingDamage(this, value);
	uninit function DecreaseBluntDamage(this, value);
	uninit function DecreaseFireDamage(this, value);
	uninit function DecreaseColdDamage(this, value);
	uninit function DecreasePoisonDamage(this, value);
	uninit function DecreaseEnergyDamage(this, value);

	uninit function ToString(this);
endclass