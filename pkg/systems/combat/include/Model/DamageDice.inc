use util;

include "IDamageDice";

class DamageDice(IDamageDice)
	var _dice := array{}; // e.g. "1d6+5"
	var _result := array{}; 

	function DamageDice(this, dice := array{})
		this._dice := dice;
	endfunction

	function SetDice(this, value)
		this._dice := array{value};
	endfunction

	function AddDice(this, value)
		if (this._dice.size == 0)
			this._dice := array{value};
		else
			this._dice.append(value);
		endif
	endfunction

	function GetDice(this)
		return this._dice;
	endfunction

	function Roll(this)
		if (this._dice.size() < 1) return error { errortext := "No dice to roll" }; endif

		this._result := array{};
		foreach die in (this._dice)
			this._result.append(struct{
				value := RandomDiceRoll(die)
			});
		endforeach
	endfunction

	function ToString()
	endfunction
endclass