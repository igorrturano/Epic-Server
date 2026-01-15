use uo;

include ":epicevents:Controller/SpawnController";
include ":epicevents:Controller/ConditionController";
include ":epicevents:Model/EventConfig";
include ":epicevents:Model/ActionConfig";

function ExecuteActions(item, event, actions)
	if (!actions || actions.size() == 0)
		return 0;
	endif

	var action_state := struct{};  // Track state for conditions
	var action_index := 1;
	var any_executed := 0;  // Track if at least one action successfully executed

	foreach action in actions
		// Wait if delay configured
		if (action.delay && action.delay > 0)
			sleepms(action.delay * 1000);
		endif

		// Evaluate condition if exists
		if (action.condition)
			if (!EvaluateCondition(action.condition, action_state, event))
				action_index := action_index + 1;
				continue;  // Skip this action
			endif
		endif

		// Execute action
		var result := ExecuteAction(item, event, action, action_state);

		// Store result for subsequent conditions
		action_state.insert("action_" + action_index, result);

		if (result)
			any_executed := 1;  // Mark that at least one action succeeded
		endif

		action_index := action_index + 1;
	endforeach

	return any_executed;
endfunction

function ExecuteAction(item, event, action, byref action_state)
	case (action.type)
		"SPAWN":
			return ExecuteSpawnAction(item, action.params);
		"EMOTE":
			return ExecuteEmoteAction(item, event, action.params);
		"TELEPORT":
			return ExecuteTeleportAction(event.source, action.params);
		default:
			print("[EpicEvents] ERROR: Unknown action type: " + action.type);
			return 0;
	endcase
endfunction

function ExecuteSpawnAction(item, params)
	// Delegate to SpawnController (V1 logic)
	var config := EventConfig();
	config.FromStruct(params);
	return SpawnAllCreatures(item, config);
endfunction

function ExecuteEmoteAction(item, event, params)
	var emote_text := params.text;
	var range := params.range ? params.range : 10;

	if (!emote_text || emote_text == "")
		return 0;
	endif

	// Show emote above item or source
	if (params.show_on_source && event.source)
		PrintTextAbove(event.source, emote_text);
	else
		PrintTextAbove(item, emote_text);
	endif

	// Notify nearby players
	var chars := ListMobilesNearLocation(item.x, item.y, item.z, range, item.realm);
	foreach char in chars
		if (char.acctname)
			SendSysMessageEx(char, emote_text, SSM_INFO);
		endif
	endforeach

	return 1;
endfunction

function ExecuteTeleportAction(mobile, params)
	if (!mobile || !mobile.acctname)
		return 0;
	endif

	var x := CInt(params.x);
	var y := CInt(params.y);
	var z := CInt(params.z);
	var realm := params.realm ? params.realm : mobile.realm;

	if (!x || !y)
		return 0;
	endif

	MoveObjectToLocation(mobile, x, y, z, realm, MOVEOBJECT_FORCELOCATION);
	return 1;
endfunction
