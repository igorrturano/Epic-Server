class IWeaponWrapper()
	uninit function GetAttribute(this);
	uninit function IsGatheringTool(this);
	uninit function GetAttackType(this);
	uninit function GetSpeed(this);
	uninit function GetMainDamage(this);
	uninit function GetOffDamage(this);
	uninit function GetCriticalChance(this);
	uninit function GetMinRange(this);
	uninit function GetMaxRange(this);

	uninit function SetSpeed(this, value);
	uninit function SetMainDamage(this, slash, piercing, blunt);
	uninit function SetOffDamage(this, slash, piercing, blunt);
	uninit function SetCriticalChanceBase(this, value);
	uninit function SetCriticalChanceMod(this, value);

	uninit function Update(this);
endclass