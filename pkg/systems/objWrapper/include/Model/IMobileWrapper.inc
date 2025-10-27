class IMobileWrapper()
	uninit function IMobileWrapper(this, byRef mobRef);

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
	uninit function GetSkill(this, skillName);
	uninit function GetSkillBase(this, skillName);
	uninit function GetSkillMod(this, skillName);

	uninit function SetSkillBase(this, skillName, value);
	uninit function SetSkillMod(this, skillName, value);

	uninit function IsNPC(this);

	uninit function ApplyItemModifiers(this, byRef itemWrapped);
	uninit function RemoveItemModifiers(this, byRef itemWrapped);
endclass