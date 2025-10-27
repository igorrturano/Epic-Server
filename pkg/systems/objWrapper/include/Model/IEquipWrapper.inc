class IEquipWrapper()
	uninit function GetTemplate(this);
	uninit function GetItemProp(this);
	uninit function GetWeight(this);
	uninit function GetMaxHP(this);
	uninit function GetModifiers(this);
	uninit function GetModifier(this, modifierName);

	uninit function SetWeight(this, weight);
	uninit function SetMaxHP(this, maxHP);
	
	uninit function AddModifier(this, modifiers);

	uninit function Update(this);
endclass