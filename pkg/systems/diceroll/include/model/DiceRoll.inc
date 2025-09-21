use util;
use basic;
use math;

include "IDiceRoll";

class DiceRoll(IDiceRoll)
	var _dice := array{}; // Array de Dice()
	var _result := array{}; 

	function DiceRoll(this, dice := array{})
		this._dice := this.AddDice(dice);
	endfunction

	function SetDice(this, value)
		this._dice := array{};
		this._dice := this.AddDice(value);
	endfunction

	function AddDice(this, value)
		if (this._dice.size() == 0)
			this._dice := array{};
			this._result := array{};
		endif
		
		if (TypeOf(value) != "Dice")
			value := error { errortext := "Dice format must be a string like '1d6+5'" };
		elseif (TypeOfInt(value) == OT_ARRAY)
			foreach dice in (value)
				this.AddDice(dice);
			endforeach
		endif

		this._dice.append(value);
		this._result.append(error { errortext := "Dice not rolled yet" });
	endfunction

	function GetDice(this)
		return this._dice;
	endfunction

	function GetResult(this, resultType := DICE_RESULT::ALL)
		var result := error { errortext := "No result" };
		
		if (resultType == DICE_RESULT::ALL)
			result := this._result.size() > 1 ? this._result : this._result[1];
		elseif (resultType == DICE_RESULT::HIGHER)
			result := Max(this._result);
		elseif (resultType == DICE_RESULT::LOWER)
			result := Min(this._result);
		elseif (resultType == DICE_RESULT::SUM)
			var sum := 0;
			
			foreach res in (this._result)
				sum += CInt(res);
			endforeach

			result := sum;
		elseif (resultType == DICE_RESULT::AVERAGE)
			var sum := 0;
			var amt := 0;

			foreach res in (this._result)
				sum += CInt(res);
				amt += 1;
			endforeach

			result := CInt(sum / amt);
		endif

		return result;
	endfunction

	function Roll(this)
		if (this._dice.size() < 1) return error { errortext := "No dice to roll" }; endif

		this._result := array{};
		foreach dice in (this._dice)
			this._result.append(dice.Roll());
		endforeach

		return this.GetResult();
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