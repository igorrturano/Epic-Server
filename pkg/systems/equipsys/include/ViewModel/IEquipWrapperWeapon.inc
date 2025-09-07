class IEquipWrapperWeapon()
	uninit function GetAttribute(this);
	uninit function GetTipoAtaque(this);
	uninit function GetSpeed(this);
	uninit function GetMainDamage(this);
	uninit function GetOffDamage(this);
	uninit function GetCriticalChance(this);
	uninit function GetMinRange(this);
	uninit function GetMaxRange(this);

	uninit function SetSpeed(this, speed);
	uninit function SetMainDamage(this, mainDamage);
	uninit function SetOffDamage(this, offDamage);
	uninit function SetCriticalChance(this, criticalChance);
	uninit function SetMinRange(this, minRange);
	uninit function SetMaxRange(this, maxRange);

	uninit function Update(this);
endclass