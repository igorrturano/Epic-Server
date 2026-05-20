class IDamage()
	uninit function IDamage(this,
		default slash,
		default piercing,
		default blunt,

		default fire,
		default cold,
		default poison,
		default energy,
		default forced
	);

	uninit function GetSlashRawDamage(this);
	uninit function GetPiercingRawDamage(this);
	uninit function GetBluntRawDamage(this);
	uninit function GetFireRawDamage(this);
	uninit function GetColdRawDamage(this);
	uninit function GetPoisonRawDamage(this);
	uninit function GetEnergyRawDamage(this);
	uninit function GetForcedRawDamage(this);
	uninit function GetSlashBaseDamage(this);
	uninit function GetPiercingBaseDamage(this);
	uninit function GetBluntBaseDamage(this);
	uninit function GetFireBaseDamage(this);
	uninit function GetColdBaseDamage(this);
	uninit function GetPoisonBaseDamage(this);
	uninit function GetEnergyBaseDamage(this);
	uninit function GetForcedBaseDamage(this);

	uninit function SetSlashRawDamage(this, value);
	uninit function SetPiercingRawDamage(this, value);
	uninit function SetBluntRawDamage(this, value);
	uninit function SetFireRawDamage(this, value);
	uninit function SetColdRawDamage(this, value);
	uninit function SetPoisonRawDamage(this, value);
	uninit function SetEnergyRawDamage(this, value);
	uninit function SetForcedRawDamage(this, value);
	uninit function SetSlashBaseDamage(this, value);
	uninit function SetPiercingBaseDamage(this, value);
	uninit function SetBluntBaseDamage(this, value);
	uninit function SetFireBaseDamage(this, value);
	uninit function SetColdBaseDamage(this, value);
	uninit function SetPoisonBaseDamage(this, value);
	uninit function SetEnergyBaseDamage(this, value);
	uninit function SetForcedBaseDamage(this, value);

	uninit function IncreaseSlashRawDamage(this, value);
	uninit function IncreasePiercingRawDamage(this, value);
	uninit function IncreaseBluntRawDamage(this, value);
	uninit function IncreaseFireRawDamage(this, value);
	uninit function IncreaseColdRawDamage(this, value);
	uninit function IncreasePoisonRawDamage(this, value);
	uninit function IncreaseEnergyRawDamage(this, value);
	uninit function IncreaseForcedRawDamage(this, value);
	uninit function IncreaseSlashBaseDamage(this, value);
	uninit function IncreasePiercingBaseDamage(this, value);
	uninit function IncreaseBluntBaseDamage(this, value);
	uninit function IncreaseFireBaseDamage(this, value);
	uninit function IncreaseColdBaseDamage(this, value);
	uninit function IncreasePoisonBaseDamage(this, value);
	uninit function IncreaseEnergyBaseDamage(this, value);
	uninit function IncreaseForcedBaseDamage(this, value);

	uninit function DecreaseSlashRawDamage(this, value);
	uninit function DecreasePiercingRawDamage(this, value);
	uninit function DecreaseBluntRawDamage(this, value);
	uninit function DecreaseFireRawDamage(this, value);
	uninit function DecreaseColdRawDamage(this, value);
	uninit function DecreasePoisonRawDamage(this, value);
	uninit function DecreaseEnergyRawDamage(this, value);
	uninit function DecreaseForcedRawDamage(this, value);
	uninit function DecreaseSlashBaseDamage(this, value);
	uninit function DecreasePiercingBaseDamage(this, value);
	uninit function DecreaseBluntBaseDamage(this, value);
	uninit function DecreaseFireBaseDamage(this, value);
	uninit function DecreaseColdBaseDamage(this, value);
	uninit function DecreasePoisonBaseDamage(this, value);
	uninit function DecreaseEnergyBaseDamage(this, value);
	uninit function DecreaseForcedBaseDamage(this, value);

	uninit function IncreaseRawDamageByType(this, dmgType, value);

	// Reducao multiplicativa por % (0-100). Base = Base * (100 - pct) / 100.
	// NOTE: No DecreaseForcedPercentDamage by design — FORCED bypasses resist.
	uninit function DecreaseSlashPercentDamage(this, pct);
	uninit function DecreasePiercingPercentDamage(this, pct);
	uninit function DecreaseBluntPercentDamage(this, pct);
	uninit function DecreaseFirePercentDamage(this, pct);
	uninit function DecreaseColdPercentDamage(this, pct);
	uninit function DecreasePoisonPercentDamage(this, pct);
	uninit function DecreaseEnergyPercentDamage(this, pct);

	uninit function ToString(this);
endclass
