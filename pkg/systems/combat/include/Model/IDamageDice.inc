class IDamageDice()
	uninit function IDamageDice(this);

	uninit function SetDie(this, value); // e.g. "1d6+5"

	uninit function AddDie(this, die); // e.g. "1d6+5"
	
	uninit function Roll(this);
endclass