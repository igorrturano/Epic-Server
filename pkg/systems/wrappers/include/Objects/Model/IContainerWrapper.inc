class IContainerWrapper()
	uninit function IContainerWrapper(this, byRef object);

	uninit function GetWeightMultiplier(this);
	uninit function GetMaxItems(this);
	uninit function GetMaxSlots(this);
	uninit function GetMaxWeight(this);
	
	uninit function SetLocked(this, boolValue);
	uninit function SetWeightMultiplier(this, value);
	uninit function SetMaxItems(this, value);
	uninit function SetMaxSlots(this, value);
	uninit function SetMaxWeight(this, value);

	uninit function IsLocked(this);
	uninit function GetInside(this, item, default searchRecursively);
	uninit function Contains(this, item, default searchRecursively);
endclass