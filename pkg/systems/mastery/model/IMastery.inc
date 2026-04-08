class IMastery()
	uninit function IMastery(this);

	uninit function GetType(this);
	uninit function SetType(this, value);

	uninit function GetDescription(this);
	uninit function SetDescription(this, value);

	uninit function GetLevel(this);
	uninit function SetLevel(this, value);
	uninit function AddLevel(this, value);

	uninit function GetXP(this);
	uninit function SetXP(this, value);
	uninit function AddXP(this, value);

	uninit function GetAttribute(this, attribute);
	uninit function SetAttribute(this, attribute, value);
	uninit function IncreaseAttribute(this, attribute, value);
endclass