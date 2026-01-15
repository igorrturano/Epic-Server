use uo;

function EvaluateCondition(condition, byref action_state, event)
	if (!condition)
		return 1;  // No condition = always pass
	endif

	case (condition.type)
		"ACTION_SUCCEEDED":
			return EvaluateActionSucceeded(condition, action_state);
		"PLAYER_HAS_ITEM":
			return EvaluatePlayerHasItem(condition, event);
		default:
			return 1;  // Unknown condition = pass by default
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

	// Check backpack items
	foreach item in EnumerateItemsInContainer(event.source.backpack)
		if (item.objtype == required_objtype)
			found += item.amount;
		endif
	endforeach

	// Check equipped items
	var equipped := ListEquippedItems(event.source);
	foreach item in equipped
		if (item.objtype == required_objtype)
			found += item.amount;
		endif
	endforeach

	if (found >= required_amount)
		return 1;
	endif

	return 0;
endfunction
