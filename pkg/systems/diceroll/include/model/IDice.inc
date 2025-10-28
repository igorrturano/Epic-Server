class IDice()
	uninit function IDice(this, diceString);

	uninit function GetAmount(this);
	uninit function GetSides(this);
	uninit function GetModifier(this);
	uninit function GetMaximum(this);
	uninit function GetMinimum(this);
	uninit function GetResult(this);

	uninit function SetAmount(this, amount);
	uninit function SetSides(this, sides);
	uninit function SetModifier(this, modifier);

	uninit function Roll(this);
endclass