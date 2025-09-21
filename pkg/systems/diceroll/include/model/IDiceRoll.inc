class IDiceRoll()
	uninit function IDiceRoll(this);

	uninit function GetDice(this);
	uninit function GetResult(this, default resultType);

	uninit function SetDice(this, dice); // Dice or array of Dice
	uninit function AddDice(this, dice); // Dice
	
	uninit function Roll(this);
endclass