use util;

include "IDice";

class Dice(IDice)
	var _diceString; // String representando o dado (ex: "2d6+3")
	var _amount := 2; // Quantidade de dados
	var _sides := 6; // Lados do dado
	var _modifier := 0; // Modificador a ser somado ao resultado do dado
	var _result := 0; // Resultado da última rolagem

	function Dice(this, diceString)
		this._diceString := diceString;
		var mod := SplitWords(diceString, "+")[2];
		var base := SplitWords(mod[1], "d");
		if (CInt(base)[0] < 1) base[0] := 1; endif // Garante que a quantidade de dados seja pelo menos 1

		this._amount := CInt(base[1]);
		this._sides := CInt(base[2]);
		this._modifier := CInt(mod);
		this._result := false;
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

	function SetAmount(this, amount)
		this._amount := amount;
		return this;
	endfunction

	function SetSides(this, sides)
		this._sides := sides;
		return this;
	endfunction

	function SetModifier(this, modifier)
		this._modifier := modifier;
		return this;
	endfunction

	function Roll(this)
		this._result := CInt(RandomDiceRoll(this._diceString));
		return this.GetResult();
	endfunction

	function ToString(this, isDetailed := false)
		if (!isDetailed) return this._diceString; endif
		var str := "";
		str += $"Esta rolagem contém {this._amount} dados de {this._sides} lados com um modificador de +{this._modifier}. O resultado foi: ";
		str += !this._result ? "nunca rolado." : $"{this._result}.";
		return str;
	endfunction
endclass