include "IDamageDice";
include ":diceroll:model/Dice";
include "Damage";

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
		return this._slash ?: error{errortext := "Slash is not a valid Dice!"};
	endfunction

	function GetPiercingDamage(this)
		return this._piercing ?: error{errortext := "Piercing is not a valid Dice!"};
	endfunction

	function GetBluntDamage(this)
		return this._blunt ?: error{errortext := "Blunt is not a valid Dice!"};
	endfunction

	function GetFireDamage(this)
		return this._fire ?: error{errortext := "Fire is not a valid Dice!"};
	endfunction

	function GetColdDamage(this)
		return this._cold ?: error{errortext := "Cold is not a valid Dice!"};
	endfunction

	function GetPoisonDamage(this)
		return this._poison ?: error{errortext := "Poison is not a valid Dice!"};
	endfunction

	function GetEnergyDamage(this)
		return this._energy ?: error{errortext := "Energy is not a valid Dice!"};
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

	function GetMinimum(this, physicalChoice := "")
		var minDamage := 0;

		
		if (this.GetSlashDamage() is @Dice && ( physicalChoice == "" ||  DAMAGE_TYPE::SLASH == physicalChoice))
			minDamage += this.GetSlashDamage().GetMinimum();
		endif

		if (this.GetPiercingDamage() is @Dice && ( physicalChoice == "" ||  DAMAGE_TYPE::PIERCING == physicalChoice))
			minDamage += this.GetPiercingDamage().GetMinimum();
		endif

		if (this.GetBluntDamage() is @Dice && ( physicalChoice == "" ||  DAMAGE_TYPE::BLUNT == physicalChoice))
			minDamage += this.GetBluntDamage().GetMinimum();
		endif

		if (this.GetFireDamage() is @Dice)
			minDamage += this.GetFireDamage().GetMinimum();
		endif

		if (this.GetColdDamage() is @Dice)
			minDamage += this.GetColdDamage().GetMinimum();
		endif

		if (this.GetPoisonDamage() is @Dice)
			minDamage += this.GetPoisonDamage().GetMinimum();
		endif

		if (this.GetEnergyDamage() is @Dice)
			minDamage += this.GetEnergyDamage().GetMinimum();
		endif

		return minDamage;
	endfunction

	function GetMaximum(this, physicalChoice := "")
		var maxDamage := 0;

		if (this.GetSlashDamage() is @Dice && ( physicalChoice == "" || DAMAGE_TYPE::SLASH == physicalChoice ))
			maxDamage += this.GetSlashDamage().GetMaximum();
		endif

		if (this.GetPiercingDamage() is @Dice && ( physicalChoice == "" || DAMAGE_TYPE::PIERCING == physicalChoice))
			maxDamage += this.GetPiercingDamage().GetMaximum();
		endif

		if (this.GetBluntDamage() is @Dice && ( physicalChoice == "" || DAMAGE_TYPE::BLUNT == physicalChoice))
			maxDamage += this.GetBluntDamage().GetMaximum();
		endif

		if (this.GetFireDamage() is @Dice)
			maxDamage += this.GetFireDamage().GetMaximum();
		endif

		if (this.GetColdDamage() is @Dice)
			maxDamage += this.GetColdDamage().GetMaximum();
		endif

		if (this.GetPoisonDamage() is @Dice)
			maxDamage += this.GetPoisonDamage().GetMaximum();
		endif

		if (this.GetEnergyDamage() is @Dice)
			maxDamage += this.GetEnergyDamage().GetMaximum();
		endif

		return maxDamage;
	endfunction

	function ToString(this)
		return $"Weapon Damage: " +
			$"Slash={this.GetSlashDamage().ToString()}, " +
			$"Piercing={this.GetPiercingDamage().ToString()}, " +
			$"Blunt={this.GetBluntDamage().ToString()}, " +
			$"Fire={this.GetFireDamage().ToString()}, " +
			$"Cold={this.GetColdDamage().ToString()}, " +
			$"Poison={this.GetPoisonDamage().ToString()}, " +
			$"Energy={this.GetEnergyDamage().ToString()}";
	endfunction

	// Retorna um DamageDice novo com os 3 dados fisicos (slash/piercing/blunt)
	// upgraded um tier. Elementais inalterados. Usado pelo bonus de 1H sword
	// solo em CalculateWeaponDamage (espada com mao off-hand livre permite
	// swing mais aberto). Cada Dice fisico vira upgraded via Dice.WithUpgradedSides.
	function WithUpgradedPhysicalDice(this)
		var slash    := this._slash    is @Dice ? this._slash.WithUpgradedSides()    : this._slash;
		var piercing := this._piercing is @Dice ? this._piercing.WithUpgradedSides() : this._piercing;
		var blunt    := this._blunt    is @Dice ? this._blunt.WithUpgradedSides()    : this._blunt;
		return DamageDice(slash, piercing, blunt, this._fire, this._cold, this._poison, this._energy);
	endfunction
endclass
