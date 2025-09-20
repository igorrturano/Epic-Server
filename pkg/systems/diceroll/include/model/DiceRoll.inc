use util;
use basic;

include "IDiceRoll";

class DiceRoll(IDiceRoll)
	var _dice := array{}; // e.g. "1d6+5"
	var _result := array{}; 

	function DiceRoll(this, dice := array{})
		this._dice := this.AddDice(dice);
	endfunction

	function SetDice(this, value)
		this._dice := this.AddDice(value);
	endfunction

	function AddDice(this, value)
		if (this._dice.size() == 0)
			this._dice := array{};
			this._result := array{};
		endif
		
		if (TypeOfInt(value) == OT_STRING)
			value := error { errortext := "Die format must be a string like '1d6+5'" };
		elseif (TypeOfInt(value) == OT_ARRAY)
			foreach die in (value)
				this.AddDice(die);
			endforeach
		else
			value := error { errortext := "Die format must be a string like '1d6+5'" };
		endif

		this._dice.append(value);
		this._result.append(error { errortext := "Die not rolled yet" });
	endfunction

	function GetDice(this)
		return this._dice;
	endfunction

	function Roll(this)
		if (this._dice.size() < 1) return error { errortext := "No dice to roll" }; endif

		this._result := array{};
		foreach die in (this._dice)
			this._result.append(struct{
				value := CInt(RandomDiceRoll(die))
			});
		endforeach
	endfunction

	function GetResult(this, resultType := DICE_RESULT::ALL)
		var result := error { errortext := "No result" };
		
		if (resultType == DICE_RESULT::ALL)
			result := this._result;
		elseif (resultType == DICE_RESULT::HIGHER)
			// TODO: Lógica de maior valor
			result := CInt(0);
		elseif (resultType == DICE_RESULT::LOWER)
			// TODO: Lógica de menor valor
			result := CInt(0);
		elseif (resultType == DICE_RESULT::SUM)
			// TODO: Lógica de soma
			result := CInt(0);
		elseif (resultType == DICE_RESULT::AVERAGE)
			// TODO: Lógica de média
			result := CInt(0);
		endif

		return result;
	endfunction

	function ToString(this)
	endfunction
endclass

enum class DICE_RESULT
	ALL,
	HIGHER,
	LOWER,
	SUM,
	AVERAGE
endenum