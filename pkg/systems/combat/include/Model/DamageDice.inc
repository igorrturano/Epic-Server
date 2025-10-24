include "IDamageDice";
include ":diceroll:model/Dice";

include ":equipsys:utils/consts";

class DamageDice(IDamageDice)
	var _slash := Dice("1d6+2");
	var _piercing := Dice("1d6+2");
	var _blunt := Dice("1d6+2");
	var _fire := Dice("1d6+2");
	var _cold := Dice("1d6+2");
	var _poison := Dice("1d6+2");
	var _energy := Dice("1d6+2");

	function DamageDice(this, slash := DAMAGE_NONE, piercing := DAMAGE_NONE, blunt := DAMAGE_NONE, fire := DAMAGE_NONE, cold := DAMAGE_NONE, poison := DAMAGE_NONE, energy := DAMAGE_NONE)
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
			var err := error{ errortext := "Parameter must be a Dice object!" };
			this._slash := err;
			return err;
		endif

		this._slash := dice;

		return this;
	endfunction

	function SetPiercingDamage(this, dice)
		if (!(dice is @Dice))
			var err := error{ errortext := "Parameter must be a Dice object!" };
			this._piercing := err;
			return err;
		endif

		this._piercing := dice;

		return this;
	endfunction

	function SetBluntDamage(this, dice)
		if (!(dice is @Dice))
			var err := error{ errortext := "Parameter must be a Dice object!" };
			this._blunt := err;
			return err;
		endif

		this._blunt := dice;

		return this;
	endfunction

	function SetFireDamage(this, dice)
		if (!(dice is @Dice))
			var err := error{ errortext := "Parameter must be a Dice object!" };
			this._fire := err;
			return err;
		endif

		this._fire := dice;

		return this;
	endfunction

	function SetColdDamage(this, dice)
		if (!(dice is @Dice))
			var err := error{ errortext := "Parameter must be a Dice object!" };
			this._cold := err;
			return err;
		endif

		this._cold := dice;

		return this;
	endfunction

	function SetPoisonDamage(this, dice)
		if (!(dice is @Dice))
			var err := error{ errortext := "Parameter must be a Dice object!" };
			this._poison := err;
			return err;
		endif

		this._poison := dice;

		return this;
	endfunction

	function SetEnergyDamage(this, dice)
		if (!(dice is @Dice))
			var err := error{ errortext := "Parameter must be a Dice object!" };
			this._energy := err;
			return err;
		endif

		this._energy := dice;

		return this;
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