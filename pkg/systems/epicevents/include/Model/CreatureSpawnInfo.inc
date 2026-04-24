use uo;

class CreatureSpawnInfo()
	var template;
	var amount;
	var tier;
	var name;

	function CreatureSpawnInfo(this)
		// Constructor - initialize with default values
		this.template := "";
		this.amount := "";
		this.tier := "";
		this.name := "";
	endfunction

	function IsValid(this)
		if (!this.template || this.template == "")
			return 0;
		endif

		if (this.GetAmount() < 1)
			return 0;
		endif

		return 1;
	endfunction

	function GetAmount(this)
		var amt := CInt(this.amount);
		if (amt < 1)
			amt := 1;
		endif

		return amt;
	endfunction

	function GetTier(this)
		if (!this.tier || this.tier == "")
			return "0";
		endif

		return this.tier;
	endfunction
endclass
