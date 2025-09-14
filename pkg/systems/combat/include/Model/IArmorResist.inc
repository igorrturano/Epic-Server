class IArmorResist()
	uninit function IArmorResist(this,
		default slash,
		default piercing,
		default blunt,
		
		default fire,
		default cold,
		default poison,
		default energy
	);

	uninit function GetSlashResist(this);
	uninit function GetPiercingResist(this);
	uninit function GetBluntResist(this);
	uninit function GetFireResist(this);
	uninit function GetColdResist(this);
	uninit function GetPoisonResist(this);
	uninit function GetEnergyResist(this);

	uninit function ToString(this);
endclass