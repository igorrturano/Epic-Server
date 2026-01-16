use uo;

function EvaluateCondition(condition, byref action_state, event)
	if (!condition)
		return 1;
	endif

	case (condition.type)
		"ACTION_SUCCEEDED":
			return EvaluateActionSucceeded(condition, action_state);
		"PLAYER_HAS_ITEM":
			return EvaluatePlayerHasItem(condition, event);
		"NONE":
			return 1;
		default:
			return 1;
	endcase
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

function EvaluatePlayerHasItem(condition, event)
	if (!event.source || !event.source.acctname)
		return 0;
	endif

	var required_objtype := condition.params.objtype;
	var required_amount := condition.params.amount ? condition.params.amount : 1;

	var found := 0;

	// O(1) search using FindObjtypeInContainer
	if (event.source.backpack)
		var items := FindObjtypeInContainer(event.source.backpack, required_objtype);
		foreach item in items
			found += item.amount;
		endforeach
	endif

	// Check equipped items (usually small list, O(1) in practice)
	var equipped := ListEquippedItems(event.source);
	foreach item in equipped
		if (item.objtype == required_objtype)
			found += item.amount;
		endif
	endforeach

	return (found >= required_amount);
endfunction
