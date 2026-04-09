class IEquipWrapper()
	uninit function GetTemplate(this);
	uninit function GetEquipProp(this);
	uninit function GetWeight(this);
	uninit function GetMaxHP(this);
	uninit function GetModifiers(this);
	uninit function GetModifier(this, modifierName);

	uninit function SetModifier(this, modifier);

	uninit function Update(this);
endclass