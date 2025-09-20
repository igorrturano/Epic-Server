use util;

include "IDice";

class Dice(IDice)
	var _diceString; // String representando o dado (ex: "2d6+3")
	var _amount := 1; // Quantidade de dados
	var _sides := 6; // Lados do dado
	var _modifier := 0; // Modificador a ser somado ao resultado do dado

	function Dice(this, diceString)
		this._diceString := diceString;
		var mod := SplitWords(diceString, "+")[2];
		var base := SplitWords(mod[1], "d");

		this._amount := CInt(base[1]);
		this._sides := CInt(base[2]);
		this._modifier := CInt(mod);
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

	function Roll(this)
		return CInt(RandomDiceRoll(this._diceString));
	endfunction

	function ToString(this)
		return this._diceString;
	endfunction
endclass