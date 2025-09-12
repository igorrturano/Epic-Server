class IEquipWrapperWeapon()
	uninit function GetAttribute(this);
	uninit function GetTipoAtaque(this);
	uninit function GetSpeed(this);
	uninit function GetMainDamage(this);
	uninit function GetOffDamage(this);
	uninit function GetCriticalChance(this);
	uninit function GetMinRange(this);
	uninit function GetMaxRange(this);

	uninit function SetSpeed(this, value);
	uninit function SetMainDamage(this, slash, piercing, blunt);
	uninit function SetOffDamage(this, slash, piercing, blunt);
	uninit function SetCriticalChance(this, value);

	uninit function Update(this);
endclass