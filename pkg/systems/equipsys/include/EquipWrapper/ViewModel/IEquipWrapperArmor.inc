class IEquipWrapperArmor()
	uninit function GetTipoProtecao(this);
	uninit function GetCoverage(this);
	uninit function GetResists(this);
	uninit function GetResistsMod(this);
	uninit function GetPenalties(this);

	uninit function SetResists(this, resists);
	uninit function SetResistsMod(this, resists);
	uninit function SetPenalties(this, penalties);

	uninit function Update(this);
endclass