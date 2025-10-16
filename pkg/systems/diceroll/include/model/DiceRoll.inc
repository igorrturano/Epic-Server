use util;
use basic;
use math;

include "IDiceRoll";

class DiceRoll(IDiceRoll)
	var _dice := array{}; // Array de Dice()
	var _result := array{}; 

	function DiceRoll(this, dice) // dice pode ser string ou array de strings
		this._dice := this.AddDice(dice);
	endfunction

	function GetDice(this)
		return this._dice;
	endfunction

	function SetDice(this, dice)
		this._dice := array{};
		this._dice := this.AddDice(dice);
	endfunction

	function AddDice(this, dice)
		if (this._dice.size() == 0)
			this._dice := array{};
			this._result := array{};
		endif
		
		if (TypeOf(dice) != "Dice") // TODO: Verificar se o pol encara isso aqui como Dice ou como Class - Se não der certo, é só chamar uma função de Dice e verificar se retorna erro
			dice := error { errortext := "Dice format must be a string like '1d6+5'" };
		elseif (TypeOfInt(dice) == OT_ARRAY)
			foreach die in (dice)
				this.AddDice(die);
			endforeach
		endif

		this._dice.append(dice);
		this._result.append(error { errortext := "Dice not rolled yet" });
	endfunction

	function GetResult(this, resultType := DICE_RESULT::ALL)
		var result := error { errortext := "No result" };
		
		if (resultType == DICE_RESULT::ALL)
			result := this._result;
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