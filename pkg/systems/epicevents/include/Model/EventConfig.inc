use uo;

include ":epicevents:Model/ActionConfig";

class EventConfig()
	// Creature 1
	var template1;
	var amount1;
	var tier1;
	var name1;

	// Creature 2
	var template2;
	var amount2;
	var tier2;
	var name2;

	// Creature 3
	var template3;
	var amount3;
	var tier3;
	var name3;

	// Shared properties
	var color;
	var emote;
	var min_delay;
	var max_delay;

	// Event system properties
	var trigger_type;       // "SPEECH", "ENTEREDAREA", "LEFTAREA", "ITEM_GIVEN", "DOUBLECLICKED"
	var trigger_params;     // Struct with trigger-specific parameters
	var actions;            // Array of ActionConfig objects
	var global_cooldown;    // Cooldown in seconds between triggers

	function EventConfig(this)
		// Constructor - initialize with default values
		this.template1 := "";
		this.amount1 := "";
		this.tier1 := "";
		this.name1 := "";

		this.template2 := "";
		this.amount2 := "";
		this.tier2 := "";
		this.name2 := "";

		this.template3 := "";
		this.amount3 := "";
		this.tier3 := "";
		this.name3 := "";

		this.color := "";
		this.emote := "";
		this.min_delay := "60";
		this.max_delay := "120";

		// Event system defaults
		this.trigger_type := "";
		this.trigger_params := 0;
		this.actions := array{};
		this.global_cooldown := 3600;
	endfunction

	function ToStruct(this)
		var data := struct;

		// Creature 1
		data.+template1 := this.template1;
		data.+amount1 := this.amount1;
		data.+tier1 := this.tier1;
		data.+name1 := this.name1;

		// Creature 2
		data.+template2 := this.template2;
		data.+amount2 := this.amount2;
		data.+tier2 := this.tier2;
		data.+name2 := this.name2;

		// Creature 3
		data.+template3 := this.template3;
		data.+amount3 := this.amount3;
		data.+tier3 := this.tier3;
		data.+name3 := this.name3;

		// Shared
		data.+color := this.color;
		data.+emote := this.emote;
		data.+min_delay := this.min_delay;
		data.+max_delay := this.max_delay;

		// Event fields
		data.+trigger_type := this.trigger_type;
		data.+trigger_params := this.trigger_params;

		// Convert ActionConfig objects to structs for storage
		var actions_data := array{};
		foreach action in (this.actions)
			actions_data.append(action.ToStruct());
		endforeach
		data.+actions := actions_data;

		data.+global_cooldown := this.global_cooldown;

		return data;
	endfunction

	function FromStruct(this, data)
		if (!data)
			return 0;
		endif

		// Creature 1
		this.template1 := data.template1;
		this.amount1 := data.amount1;
		this.tier1 := data.tier1;
		this.name1 := data.name1;

		// Creature 2
		this.template2 := data.template2;
		this.amount2 := data.amount2;
		this.tier2 := data.tier2;
		this.name2 := data.name2;

		// Creature 3
		this.template3 := data.template3;
		this.amount3 := data.amount3;
		this.tier3 := data.tier3;
		this.name3 := data.name3;

		// Shared
		this.color := data.color;
		this.emote := data.emote;
		this.min_delay := data.min_delay;
		this.max_delay := data.max_delay;

		// Event system fields
		if (data.trigger_type)
			this.trigger_type := data.trigger_type;
		endif

		if (data.trigger_params)
			this.trigger_params := data.trigger_params;
		endif

		if (data.actions)
			// Reconstruct ActionConfig objects from stored structs
			this.actions := array{};
			foreach action_struct in (data.actions)
				var action := ActionConfig("");
				action.FromStruct(action_struct);
				this.actions.append(action);
			endforeach
		endif

		if (data.global_cooldown)
			this.global_cooldown := data.global_cooldown;
		endif

		return 1;
	endfunction

	function IsValid(this)
		// Validate delays
		var min_delay := CInt(this.min_delay);
		var max_delay := CInt(this.max_delay);

		if (min_delay < 1)
			return 0;
		endif

		if (max_delay < min_delay)
			return 0;
		endif

		// Check at least one creature is configured
		if (!this.template1 || this.template1 == "")
			return 0;
		endif

		// Validate amount for configured creatures
		if (this.template1 && this.template1 != "")
			if (CInt(this.amount1) < 1)
				return 0;
			endif
		endif

		if (this.template2 && this.template2 != "")
			if (CInt(this.amount2) < 1)
				return 0;
			endif
		endif

		if (this.template3 && this.template3 != "")
			if (CInt(this.amount3) < 1)
				return 0;
			endif
		endif

		return 1;
	endfunction

	function GetCreatureCount(this)
		var count := 0;

		if (this.template1 && this.template1 != "")
			count := count + 1;
		endif

		if (this.template2 && this.template2 != "")
			count := count + 1;
		endif

		if (this.template3 && this.template3 != "")
			count := count + 1;
		endif

		return count;
	endfunction
endclass
