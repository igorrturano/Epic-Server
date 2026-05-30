class IArmorWrapper()
	uninit function GetTipoProtecao(this);
	uninit function GetCoverage(this);
	uninit function GetResists(this);
	uninit function GetResistsBase(this);
	uninit function GetResistsMod(this);

	uninit function SetResistsBase(this, resists);
	uninit function SetResistsMod(this, resists);

	uninit function Update(this);
endclass