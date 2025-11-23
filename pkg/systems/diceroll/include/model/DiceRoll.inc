use util;
use basic;
use math;

include "Dice";

class DiceRoll()
	var _dice := array{}; // Array of Dice()

	function DiceRoll(this, dice) // dice pode ser string ou array de strings
		this._dice := array{};

		this.AddDice(dice);
	endfunction

	function GetDice(this)
		return this._dice;
	endfunction

	function AddDice(this, dice)
		if (TypeOfInt(dice) == OT_STRING)
			dice := Dice(dice);
		elseif (TypeOfInt(dice) == OT_ARRAY)
			foreach die in (dice)
				this.AddDice(die);
			endforeach

			return this;
		else
			this._dice := error { errortext := "Parameter must be a string or array of strings in a valid dice format like '1d6+5'!" };
			return this._dice;
		endif

		this._dice.append(dice);

		return this;
	endfunction

	function GetResult(this, resultType := DICE_RESULT::ALL)
		var result := array{};

		foreach die in (this._dice)
			result.append(die.GetResult());
		endforeach
		
		if (resultType == DICE_RESULT::ALL)
			return result; // Se for tudo, já retorna logo, pra economizar processamento
		elseif (resultType == DICE_RESULT::HIGHER)
			result := Max(result);
		elseif (resultType == DICE_RESULT::LOWER)
			result := Min(result);
		elseif (resultType == DICE_RESULT::SUM)
			var sum := 0;
			
			foreach res in (result)
				sum += CInt(res);
			endforeach

			result := sum;
		elseif (resultType == DICE_RESULT::AVERAGE)
			var sum := 0;
			var amt := 0;

			foreach res in (result)
				sum += CInt(res);
				amt += 1;
			endforeach

			result := CInt(sum / amt);
		endif

		return result;
	endfunction

	function Roll(this)
		if (this._dice == error || this._dice.size() < 1) return error { errortext := "No dice to roll" }; endif

		foreach dice in (this._dice)
			dice.Roll();
		endforeach

		return this;
	endfunction

	function ToString(this)
		var str := "";
		foreach dice in (this._dice)
			if (str != "") str += " + "; endif
			str += dice.ToString();
		endforeach
		return str;
	endfunction
endclass

enum class DICE_RESULT
	ALL,
	HIGHER,
	LOWER,
	SUM,
	AVERAGE
endenum