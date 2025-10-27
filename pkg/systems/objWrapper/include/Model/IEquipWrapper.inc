class IEquipWrapper()
	uninit function GetTemplate(this);
	uninit function GetItemRef(this);
	uninit function GetItemProp(this);
	uninit function GetName(this);
	uninit function GetDesc(this);
	uninit function GetGraphic(this);
	uninit function GetWeight(this);
	uninit function GetMaxHP(this);
	uninit function GetModifiers(this);
	uninit function GetModifier(this, modifierName);

	uninit function _SetName(this, name);
	uninit function SetDesc(this, desc);
	uninit function SetGraphic(this, graphic);
	uninit function SetWeight(this, weight);
	uninit function SetMaxHP(this, maxHP);
	
	uninit function AddModifier(this, modifiers);

	uninit function Update(this);
endclass