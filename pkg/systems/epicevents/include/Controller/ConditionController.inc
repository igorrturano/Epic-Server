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
		"BOSS_ALIVE":
			return EvaluateBossAlive(condition, event);
		default:
			print("[EpicEvents] Unknown condition type: " + condition.type + " - passing by default");
			return 1;  // Unknown condition = pass by default
	endcase
endfunction

function EvaluateActionSucceeded(condition, byref action_state)
	var action_id := condition.params.action_id;

	if (!action_id)
		print("[EpicEvents] ACTION_SUCCEEDED condition - no action_id specified");
		return 0;
	endif

	var result := action_state["action_" + action_id];

	if (result && result > 0)
		print("[EpicEvents] ACTION_SUCCEEDED condition passed - action " + action_id + " succeeded");
		return 1;
	endif

	print("[EpicEvents] ACTION_SUCCEEDED condition failed - action " + action_id + " did not succeed");
	return 0;
endfunction

function EvaluatePlayerHasItem(condition, event)
	if (!event.source || !event.source.acctname)
		print("[EpicEvents] PLAYER_HAS_ITEM condition - no player source");
		return 0;
	endif

	var required_objtype := condition.params.objtype;
	var required_amount := condition.params.amount ? condition.params.amount : 1;

	var found := 0;
	foreach item in EnumerateItemsInContainer(event.source.backpack)
		if (item.objtype == required_objtype)
			found += item.amount;
		endif
	endforeach

	if (found >= required_amount)
		print("[EpicEvents] PLAYER_HAS_ITEM condition passed - found " + found + "/" + required_amount);
		return 1;
	endif

	print("[EpicEvents] PLAYER_HAS_ITEM condition failed - found " + found + "/" + required_amount);
	return 0;
endfunction

function EvaluateBossAlive(condition, event)
	var boss_serial := condition.params.boss_serial;

	if (!boss_serial)
		print("[EpicEvents] BOSS_ALIVE condition - no boss_serial specified");
		return 0;
	endif

	var boss := SystemFindObjectBySerial(boss_serial);

	if (boss && !boss.dead && boss.connected)
		print("[EpicEvents] BOSS_ALIVE condition passed - boss is alive");
		return 1;
	endif

	print("[EpicEvents] BOSS_ALIVE condition failed - boss not alive or not found");
	return 0;
endfunction
