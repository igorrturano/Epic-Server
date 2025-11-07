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
	
	uninit function GetSlashRawDamage(this);
	uninit function GetPiercingRawDamage(this);
	uninit function GetBluntRawDamage(this);
	uninit function GetFireRawDamage(this);
	uninit function GetColdRawDamage(this);
	uninit function GetPoisonRawDamage(this);
	uninit function GetEnergyRawDamage(this);
	uninit function GetSlashBaseDamage(this);
	uninit function GetPiercingBaseDamage(this);
	uninit function GetBluntBaseDamage(this);
	uninit function GetFireBaseDamage(this);
	uninit function GetColdBaseDamage(this);
	uninit function GetPoisonBaseDamage(this);
	uninit function GetEnergyBaseDamage(this);

	uninit function SetSlashRawDamage(this, value);
	uninit function SetPiercingRawDamage(this, value);
	uninit function SetBluntRawDamage(this, value);
	uninit function SetFireRawDamage(this, value);
	uninit function SetColdRawDamage(this, value);
	uninit function SetPoisonRawDamage(this, value);
	uninit function SetEnergyRawDamage(this, value);
	uninit function SetSlashBaseDamage(this, value);
	uninit function SetPiercingBaseDamage(this, value);
	uninit function SetBluntBaseDamage(this, value);
	uninit function SetFireBaseDamage(this, value);
	uninit function SetColdBaseDamage(this, value);
	uninit function SetPoisonBaseDamage(this, value);
	uninit function SetEnergyBaseDamage(this, value);

	uninit function ToString(this);
endclass