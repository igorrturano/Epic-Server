use uo;

include ":nature:include/nature";
include ":wrappers:Objects/Model/MobileWrapper";

function EvaluateCondition(condition, byref action_state, event, consume_items := 1)
	if (!condition)
		return 1;
	endif

	case (condition.type)
		"ACTION_SUCCEEDED":
			return EvaluateActionSucceeded(condition, action_state);
		"PLAYER_HAS_ITEM":
			return EvaluatePlayerHasItem(condition, event, consume_items);
		"NATURE":
			return EvaluateNature(condition.params);
		"CONDITION":
			return EvaluateCombinedCondition(condition.params, event, consume_items);
		"NONE":
			return 1;
		default:
			return 1;
	endcase
endfunction

// Combined condition - checks both HAS_ITEM and NATURE (AND logic)
function EvaluateCombinedCondition(params, event, consume_items := 1)
	// Check HAS_ITEM condition first
	if (params.has_item_objtype && params.has_item_objtype > 0)
		if (!event.source || !event.source.acctname)
			return 0;
		endif

		var required_objtype := params.has_item_objtype;
		var required_amount := params.has_item_amount ? params.has_item_amount : 1;
		var should_consume := params.should_consume_item ? 1 : 0;

		var found := 0;
		var to_consume := array{};

		if (event.source.backpack)
			var items := EnumerateItemsInContainer(event.source.backpack);
			foreach item in items
				if (item.objtype == required_objtype)
					found += item.amount;
					to_consume.append(item);
				endif
			endforeach
		endif

		var equipped := ListEquippedItems(event.source);
		foreach item in equipped
			if (item.objtype == required_objtype)
				found += item.amount;
				to_consume.append(item);
			endif
		endforeach

		if (found < required_amount)
			return 0;
		endif

		// Consume items if required (only when consume_items flag is set)
		if (should_consume && consume_items)
			var remaining := required_amount;
			foreach item in to_consume
				if (remaining <= 0)
					break;
				endif
				var take := remaining;
				if (take > item.amount)
					take := item.amount;
				endif
				remaining -= take;
				if (item.amount > take)
					SubtractAmount(item, take);
				else
					DestroyItem(item);
				endif
			endforeach
		endif
	endif

	// Check NATURE conditions
	if (!EvaluateNature(params))
		return 0;
	endif

	// Check CHARACTER conditions
	if (!EvaluateCharacter(params, event))
		return 0;
	endif

	return 1;
endfunction

function EvaluateActionSucceeded(condition, byref action_state)
	var action_id := condition.params.action_id;

	if (!action_id)
		return 0;
	endif

	var result := action_state["action_" + action_id];

	if (result && result > 0)
		return 1;
	endif

	return 0;
endfunction

function EvaluatePlayerHasItem(condition, event, consume_items := 1)
	if (!event.source || !event.source.acctname)
		return 0;
	endif

	var required_objtype := condition.params.objtype;
	var required_amount := condition.params.amount ? condition.params.amount : 1;
	var should_consume := condition.params.should_consume_item ? 1 : 0;
	var found := 0;
	var to_consume := array{};

	if (event.source.backpack)
		var items := EnumerateItemsInContainer(event.source.backpack);
		foreach item in items
			if (item.objtype == required_objtype)
				found += item.amount;
				to_consume.append(item);
			endif
		endforeach
	endif

	var equipped := ListEquippedItems(event.source);
	foreach item in equipped
		if (item.objtype == required_objtype)
			found += item.amount;
			to_consume.append(item);
		endif
	endforeach

	// Consume items if required (only when consume_items flag is set)
	if (should_consume && consume_items && found >= required_amount)
		var remaining := required_amount;
		foreach item in to_consume
			if (remaining <= 0)
				break;
			endif
			var take := remaining;
			if (take > item.amount)
				take := item.amount;
			endif
			remaining -= take;
			if (item.amount > take)
				SubtractAmount(item, take);
			else
				DestroyItem(item);
			endif
		endforeach
	endif

	return (found >= required_amount);
endfunction

function EvaluateNature(params)
	// Fase lunar (array - OR entre fases selecionadas)
	if (params.moon_phases && params.moon_phases.size() > 0)
		var current_moon := getMoonPhase();
		if (!(current_moon in params.moon_phases))
			return 0;
		endif
	endif

	// Dia/Noite
	if (params.day_night && params.day_night != "")
		if (GetGlobalProperty("dia") != params.day_night)
			return 0;
		endif
	endif

	// Estacao (array - OR entre estacoes selecionadas)
	if (params.seasons && params.seasons.size() > 0)
		var current_season := getSeason();
		if (!(current_season in params.seasons))
			return 0;
		endif
	endif

	// Hora (range)
	if (params.time_min || params.time_max)
		var hour := getCurrentHour();
		var min_h := CInt(params.time_min);
		var max_h := CInt(params.time_max);

		// Overnight range (ex: 22-4)
		if (min_h > max_h)
			if (hour < min_h && hour > max_h)
				return 0;
			endif
		else
			if (hour < min_h || hour > max_h)
				return 0;
			endif
		endif
	endif

	return 1;
endfunction

function EvaluateCharacter(params, event)
	if (!params.character_race && !params.character_culture && !params.character_religion)
		return 1;
	endif

	if (!event.source || !event.source.acctname)
		return 0;
	endif

	// Read chardata directly - MobileWrapper.GetRace() has a bug
	// (reads .race but chardata uses .raca)
	var chardata := GetObjProperty(event.source, "chardata");
	if (!chardata)
		return 0;
	endif

	// AND logic - all non-empty fields must match
	if (params.character_race && params.character_race != "")
		var player_race := chardata.raca ? chardata.raca : "";
		if (Upper(player_race) != Upper(params.character_race))
			return 0;
		endif
	endif

	if (params.character_culture && params.character_culture != "")
		var player_culture := chardata.culture ? chardata.culture : "";
		if (Upper(player_culture) != Upper(params.character_culture))
			return 0;
		endif
	endif

	if (params.character_religion && params.character_religion != "")
		var player_religion := chardata.religion ? chardata.religion : "";
		if (Upper(player_religion) != Upper(params.character_religion))
			return 0;
		endif
	endif

	return 1;
endfunction
