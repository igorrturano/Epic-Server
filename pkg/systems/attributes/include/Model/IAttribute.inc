class IAttribute()
	uninit function IAttribute(this);

	uninit function GetName(this);

	uninit function GetTotalValue(this);
	uninit function GetBaseValue(this);
	uninit function GetModValue(this);

	uninit function SetBaseValue(this, value);
	uninit function SetModValue(this, value);

	uninit function IncreaseBaseValue(this, value);
	uninit function IncreaseModValue(this, value);
	uninit function DecreaseBaseValue(this, value);
	uninit function DecreaseModValue(this, value);
endclass