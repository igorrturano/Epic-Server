use util;

class Dice()
	var _diceString; // String representando o dado (ex: "2d6+3")
	var _amount := 2; // Quantidade de dados
	var _sides := 6; // Lados do dado
	var _modifier := 0; // Modificador a ser somado ao resultado do dado
	var _result := 0; // Resultado da última rolagem

	function Dice(this, diceString)
		this._diceString := diceString;
		diceString := SplitWords(diceString, "+");
		var mod := diceString[2];
		var base := SplitWords(diceString[1], "d");
		if (CInt(base)[0] < 1) base[0] := 1; endif // Garante que a quantidade de dados seja pelo menos 1

		this.SetAmount(base[1]);
		this.SetSides(base[2]);
		this.SetModifier(mod);
	endfunction

	function GetAmount(this)
		return this._amount;
	endfunction

	function GetSides(this)
		return this._sides;
	endfunction

	function GetModifier(this)
		return this._modifier;
	endfunction

	function GetMaximum(this)
		return (this._amount * this._sides) + this._modifier;
	endfunction

	function GetMinimum(this)
		return this._amount + this._modifier;
	endfunction

	function GetResult(this)
		return this._result;
	endfunction

	function GetDiceString(this)
		return this._diceString;
	endfunction

	function SetAmount(this, amount)
		this._amount := CInt(amount);
		return this;
	endfunction

	function SetSides(this, sides)
		this._sides := CInt(sides);
		return this;
	endfunction

	function SetModifier(this, modifier)
		this._modifier := CInt(modifier);
		return this;
	endfunction

	function Roll(this)
		if (this._amount < 1)
			this._result := error { errortext := "Amount of dice less than 1 or invalid" };
		elseif (this._sides < 2)
			this._result := error { errortext := "Sides of dice less than 2 or invalid" };
		else
			this._result := CInt(RandomDiceRoll(this._diceString));
		endif

		return this;
	endfunction

	function ToString(this, isDetailed := false)
		if (!isDetailed) return this._diceString; endif

		return $"Esta rolagem contém {this._amount} dado" + (this._amount > 1 ? "s" : "")
			+ $" de {this._sides} lado" + (this._sides > 1 ? "s" : "")
			+ $" com um modificador de +{this._modifier}"
			+ ". O resultado foi: " + (!this._result ? "nunca rolado." : $"{this._result}.");
	endfunction

	// Retorna uma nova Dice com lados upgrade (1d4→1d6, 1d6→1d8, 1d8→1d10,
	// 1d10→1d12, capa em 1d12). Mantem amount e modifier. Se nao for um dado
	// valido (sides < 2), retorna a propria sem mudanca.
	function WithUpgradedSides(this)
		if (this._sides < 2) return this; endif
		var newSides := this._sides + 2;
		if (newSides > 12) newSides := 12; endif
		var spec := CStr(this._amount) + "d" + CStr(newSides);
		if (this._modifier) spec := spec + "+" + CStr(this._modifier); endif
		return Dice(spec);
	endfunction
endclass
