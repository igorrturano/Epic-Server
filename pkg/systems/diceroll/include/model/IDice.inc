class IDice()
	uninit function IDice(this, diceString);

	uninit function GetAmount(this);
	uninit function GetSides(this);
	uninit function GetModifier(this);
	uninit function GetMaximum(this);
	uninit function GetMinimum(this);

	uninit function Roll(this);
endclass