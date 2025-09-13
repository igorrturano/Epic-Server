class IEquipWrapper()
	uninit function GetTemplate(this);
	uninit function GetItem(this);
	uninit function GetProp(this);
	uninit function GetName(this);
	uninit function GetDesc(this);
	uninit function GetGraphic(this);
	uninit function GetWeight(this);
	uninit function GetMaxHP(this);

	uninit function _SetName(this, name);
	uninit function SetDesc(this, desc);
	uninit function SetGraphic(this, graphic);
	uninit function SetWeight(this, weight);
	uninit function SetMaxHP(this, maxHP);

	uninit function Update(this);
endclass