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
	var condition;  // Nature-based condition for trigger activation
	var one_time_only;  // Destroy event item after first trigger

	function EventConfig(this)
		this.creatures := array{};
		this.color := "";
		this.emote := "";
		this.min_delay := DEFAULT_MIN_DELAY_MINUTES;
		this.max_delay := DEFAULT_MAX_DELAY_MINUTES;
		this.trigger_type := "";
		this.trigger_params := 0;
		this.actions := array{};
		this.condition := 0;
		this.one_time_only := 0;
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
		data.+one_time_only := this.one_time_only;

		// Serialize actions array
		var actions_data := array{};
		foreach action in (this.actions)
			actions_data.append(action.ToStruct());
		endforeach
		data.+actions := actions_data;

		// Serialize condition with explicit array handling
		if (this.condition && this.condition.params)
			var cond_data := struct{};
			cond_data.+type := this.condition.type;

			var params_data := struct{};

			// HAS_ITEM fields
			if (this.condition.params.has_item_objtype)
				params_data.+has_item_objtype := this.condition.params.has_item_objtype;
			endif
			if (this.condition.params.has_item_amount)
				params_data.+has_item_amount := this.condition.params.has_item_amount;
			endif
			if (this.condition.params.should_consume_item)
				params_data.+should_consume_item := this.condition.params.should_consume_item;
			endif

			// NATURE fields - explicitly copy arrays
			if (this.condition.params.moon_phases)
				var moon_arr := array{};
				foreach phase in (this.condition.params.moon_phases)
					moon_arr.append(phase);
				endforeach
				params_data.+moon_phases := moon_arr;
			endif

			if (this.condition.params.day_night)
				params_data.+day_night := this.condition.params.day_night;
			endif

			if (this.condition.params.seasons)
				var seasons_arr := array{};
				foreach s in (this.condition.params.seasons)
					seasons_arr.append(s);
				endforeach
				params_data.+seasons := seasons_arr;
			endif

			if (this.condition.params.time_min)
				params_data.+time_min := this.condition.params.time_min;
			endif
			if (this.condition.params.time_max)
				params_data.+time_max := this.condition.params.time_max;
			endif

			cond_data.+params := params_data;
			data.+condition := cond_data;
		else
			data.+condition := 0;
		endif

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

		this.one_time_only := data.one_time_only ? 1 : 0;

		if (data.actions)
			this.actions := array{};
			foreach action_struct in (data.actions)
				var action := ActionConfig("");
				action.FromStruct(action_struct);
				this.actions.append(action);
			endforeach
		endif

		// Deserialize condition with explicit array handling
		if (data.condition && data.condition.type)
			var cond := struct{};
			cond.+type := data.condition.type;

			var params := struct{};

			if (data.condition.params)
				// HAS_ITEM fields
				if (data.condition.params.has_item_objtype)
					params.+has_item_objtype := CInt(data.condition.params.has_item_objtype);
				endif
				if (data.condition.params.has_item_amount)
					params.+has_item_amount := CInt(data.condition.params.has_item_amount);
				endif
				if (data.condition.params.should_consume_item)
					params.+should_consume_item := CInt(data.condition.params.should_consume_item);
				endif

				// NATURE fields - explicitly rebuild arrays
				if (data.condition.params.moon_phases)
					var moon_arr := array{};
					foreach phase in (data.condition.params.moon_phases)
						moon_arr.append(phase);
					endforeach
					params.+moon_phases := moon_arr;
				else
					params.+moon_phases := array{};
				endif

				if (data.condition.params.day_night)
					params.+day_night := data.condition.params.day_night;
				else
					params.+day_night := "";
				endif

				if (data.condition.params.seasons)
					var seasons_arr := array{};
					foreach s in (data.condition.params.seasons)
						seasons_arr.append(CInt(s));
					endforeach
					params.+seasons := seasons_arr;
				else
					params.+seasons := array{};
				endif

				if (data.condition.params.time_min)
					params.+time_min := CInt(data.condition.params.time_min);
				endif
				if (data.condition.params.time_max)
					params.+time_max := CInt(data.condition.params.time_max);
				endif
			endif

			cond.+params := params;
			this.condition := cond;
		else
			this.condition := 0;
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
