class IShieldAttributes()
	uninit function IShieldAttributes(this, default parryBonus, default parryDelay);

	uninit function GetParryBonus(this);
	uninit function GetParryDelay(this);

	uninit function SetParryBonus(this, value);
	uninit function SetParryDelay(this, value);
endclass