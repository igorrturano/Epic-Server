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

	uninit function SetSlashResist(this, value);
	uninit function SetPiercingResist(this, value);
	uninit function SetBluntResist(this, value);
	uninit function SetFireResist(this, value);
	uninit function SetColdResist(this, value);
	uninit function SetPoisonResist(this, value);
	uninit function SetEnergyResist(this, value);
endclass