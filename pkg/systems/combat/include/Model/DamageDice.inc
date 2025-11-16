include "IDamageDice";
include ":diceroll:model/Dice";

include ":equipsys:utils/consts";

class DamageDice(IDamageDice)
	var _slash; // Dice class
	var _piercing; // Dice class
	var _blunt; // Dice class
	var _fire; // Dice class
	var _cold; // Dice class
	var _poison; // Dice class
	var _energy; // Dice class

	function DamageDice(this, slash := false, piercing := false, blunt := false, fire := false, cold := false, poison := false, energy := false)
		this.SetSlashDamage(slash);
		this.SetPiercingDamage(piercing);
		this.SetBluntDamage(blunt);
		this.SetFireDamage(fire);
		this.SetColdDamage(cold);
		this.SetPoisonDamage(poison);
		this.SetEnergyDamage(energy);
	endfunction

	function GetSlashDamage(this)
		return this._slash;
	endfunction

	function GetPiercingDamage(this)
		return this._piercing;
	endfunction

	function GetBluntDamage(this)
		return this._blunt;
	endfunction

	function GetFireDamage(this)
		return this._fire;
	endfunction

	function GetColdDamage(this)
		return this._cold;
	endfunction

	function GetPoisonDamage(this)
		return this._poison;
	endfunction

	function GetEnergyDamage(this)
		return this._energy;
	endfunction

	function SetSlashDamage(this, dice)
		if (!(dice is @Dice))
			this._slash := error{errortext := "Parameter must be a Dice object!"};
			return this._slash;
		endif

		this._slash := dice;

		return this;
	endfunction

	function SetPiercingDamage(this, dice)
		if (!(dice is @Dice))
			this._piercing := error{errortext := "Parameter must be a Dice object!"};
			return this._piercing;
		endif

		this._piercing := dice;

		return this;
	endfunction

	function SetBluntDamage(this, dice)
		if (!(dice is @Dice))
			this._blunt := error{errortext := "Parameter must be a Dice object!"};
			return this._blunt;
		endif

		this._blunt := dice;

		return this;
	endfunction

	function SetFireDamage(this, dice)
		if (!(dice is @Dice))
			this._fire := error{errortext := "Parameter must be a Dice object!"};
			return this._fire;
		endif

		this._fire := dice;

		return this;
	endfunction

	function SetColdDamage(this, dice)
		if (!(dice is @Dice))
			this._cold := error{errortext := "Parameter must be a Dice object!"};
			return this._cold;
		endif

		this._cold := dice;

		return this;
	endfunction

	function SetPoisonDamage(this, dice)
		if (!(dice is @Dice))
			this._poison := error{errortext := "Parameter must be a Dice object!"};
			return this._poison;
		endif

		this._poison := dice;

		return this;
	endfunction

	function SetEnergyDamage(this, dice)
		if (!(dice is @Dice))
			this._energy := error{errortext := "Parameter must be a Dice object!"};
			return this._energy;
		endif

		this._energy := dice;

		return this;
	endfunction

	function RollSlashDamage(this)
		if (!this._slash)
			return error{errortext := "Slash damage not set!"};
		endif
		
		return this._slash.Roll();
	endfunction

	function RollPiercingDamage(this)
		if (!this._piercing)
			return error{errortext := "Piercing damage not set!"};
		endif
		
		return this._piercing.Roll();
	endfunction

	function RollBluntDamage(this)
		if (!this._blunt)
			return error{errortext := "Blunt damage not set!"};
		endif
		
		return this._blunt.Roll();
	endfunction

	function RollFireDamage(this)
		if (!this._fire)
			return error{errortext := "Fire damage not set!"};
		endif
		
		return this._fire.Roll();
	endfunction

	function RollColdDamage(this)
		if (!this._cold)
			return error{errortext := "Cold damage not set!"};
		endif
		
		return this._cold.Roll();
	endfunction

	function RollPoisonDamage(this)
		if (!this._poison)
			return error{errortext := "Poison damage not set!"};
		endif
		
		return this._poison.Roll();
	endfunction

	function RollEnergyDamage(this)
		if (!this._energy)
			return error{errortext := "Energy damage not set!"};
		endif
		
		return this._energy.Roll();
	endfunction

	function ToString(this)
		return $"Weapon Damage: " +
			$"Slash={this._slash.ToString()}, " +
			$"Piercing={this._piercing.ToString()}, " +
			$"Blunt={this._blunt.ToString()}, " +
			$"Fire={this._fire.ToString()}, " +
			$"Cold={this._cold.ToString()}, " +
			$"Poison={this._poison.ToString()}, " +
			$"Energy={this._energy.ToString()}";
	endfunction
endclass
