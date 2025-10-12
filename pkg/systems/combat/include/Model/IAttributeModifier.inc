class IAttributeModifier()
	uninit function IAttributeModifier(this, attribute, value);

	uninit function GetAttribute(this);
	uninit function GetValue(this);

	uninit function SetValue(this, value);

	uninit function DecreaseValue(this, value);
	uninit function IncreaseValue(this, value);

	uninit function ToString(this);
endclass