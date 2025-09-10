class IWeaponDamage()
	uninit function WeaponDamage(this,
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

	uninit function ToString(this);
endclass