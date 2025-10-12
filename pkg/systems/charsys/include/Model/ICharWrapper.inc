class ICharWrapper()
	uninit function ICharWrapper(this, byRef char);

	uninit function GetMobRef(this);
	uninit function GetName(this);
	uninit function GetStr(this);
	uninit function GetDex(this);
	uninit function GetInt(this);
	uninit function GetCurrentHP(this);
	uninit function GetCurrentStamina(this);
	uninit function GetCurrentMana(this);
	uninit function GetMaxHP(this);
	uninit function GetMaxStamina(this);
	uninit function GetMaxMana(this);
	uninit function GetGold(this);
	uninit function GetWeight(this);
	uninit function GetStatCap(this);

	uninit function IsNPC(this);

	uninit function ApplyItemModifiers(this, byRef itemWrapped);
	uninit function RemoveItemModifiers(this, byRef itemWrapped);
endclass