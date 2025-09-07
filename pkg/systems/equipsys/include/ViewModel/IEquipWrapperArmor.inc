class IEquipWrapperArmor()
	uninit function GetTipoProtecao(this);
	uninit function GetCoverage(this);
	uninit function GetPhysicalResists(this);
	uninit function GetElementalResists(this);
	uninit function GetPenalties(this);

	uninit function SetPhysicalResists(this, resists);
	uninit function SetElementalResists(this, resists);
	uninit function SetPenalties(this, penalties);

	uninit function Update(this);
endclass