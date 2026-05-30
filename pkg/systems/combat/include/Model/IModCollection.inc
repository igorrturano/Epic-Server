class IModCollection()

	uninit function InsertMod(this, byref damageMod);
	uninit function RemoveMod(this, byref damageMod);
	uninit function Contains(this, damageMod);
	uninit function Count(this);
	
	uninit function FindModBySource(this, damageSource);
	uninit function FindMod(this, damageMod);
	uninit function GetTotal(this);
	uninit function IsValidMod(this, mod);
endclass