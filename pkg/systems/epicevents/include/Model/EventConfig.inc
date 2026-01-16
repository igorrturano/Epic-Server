use uo;

include ":epicevents:Model/ActionConfig";
include ":epicevents:Model/CreatureSpawnInfo";

const DEFAULT_MIN_DELAY_MINUTES := 60;
const DEFAULT_MAX_DELAY_MINUTES := 120;

class EventConfig()
	// Creatures array - dynamic list of CreatureSpawnInfo objects
	var creatures;

	// Shared properties
	var color;
	var emote;
	var min_delay;
	var max_delay;

	// Event system properties
	var trigger_type;
	var trigger_params;
	var actions;

	function EventConfig(this)
		this.creatures := array{};
		this.color := "";
		this.emote := "";
		this.min_delay := DEFAULT_MIN_DELAY_MINUTES;
		this.max_delay := DEFAULT_MAX_DELAY_MINUTES;
		this.trigger_type := "";
		this.trigger_params := 0;
		this.actions := array{};
	endfunction

	function ToStruct(this)
		var data := struct;

		// Serialize creatures array
		var creatures_data := array{};
		foreach creature in (this.creatures)
			var creature_struct := struct{};
			creature_struct.+template := creature.template;
			creature_struct.+amount := creature.amount;
			creature_struct.+tier := creature.tier;
			creature_struct.+name := creature.name;
			creatures_data.append(creature_struct);
		endforeach
		data.+creatures := creatures_data;

		data.+color := this.color;
		data.+emote := this.emote;
		data.+min_delay := this.min_delay;
		data.+max_delay := this.max_delay;
		data.+trigger_type := this.trigger_type;
		data.+trigger_params := this.trigger_params;

		// Serialize actions array
		var actions_data := array{};
		foreach action in (this.actions)
			actions_data.append(action.ToStruct());
		endforeach
		data.+actions := actions_data;

		return data;
	endfunction

	function FromStruct(this, data)
		if (!data)
			return 0;
		endif

		// Deserialize creatures array
		this.creatures := array{};
		if (data.creatures)
			foreach creature_struct in (data.creatures)
				var creature := CreatureSpawnInfo();
				creature.template := creature_struct.template;
				creature.amount := creature_struct.amount;
				creature.tier := creature_struct.tier;
				creature.name := creature_struct.name;
				this.creatures.append(creature);
			endforeach
		endif

		this.color := data.color;
		this.emote := data.emote;
		// Store as integers (convert from potentially string storage)
		this.min_delay := CInt(data.min_delay);
		this.max_delay := CInt(data.max_delay);

		if (data.trigger_type)
			this.trigger_type := data.trigger_type;
		endif

		if (data.trigger_params)
			this.trigger_params := data.trigger_params;
		endif

		if (data.actions)
			this.actions := array{};
			foreach action_struct in (data.actions)
				var action := ActionConfig("");
				action.FromStruct(action_struct);
				this.actions.append(action);
			endforeach
		endif

		return 1;
	endfunction

	function IsValid(this)
		// Validate delays (already stored as integers)
		if (this.min_delay < 1)
			return 0;
		endif

		if (this.max_delay < this.min_delay)
			return 0;
		endif

		// Check at least one creature is configured
		if (this.creatures.size() == 0)
			return 0;
		endif

		// Validate each creature
		foreach creature in (this.creatures)
			if (!creature.IsValid())
				return 0;
			endif
		endforeach

		return 1;
	endfunction

	function GetCreatureCount(this)
		return this.creatures.size();
	endfunction
endclass
