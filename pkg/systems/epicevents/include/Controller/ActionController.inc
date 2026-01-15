use uo;

include ":epicevents:Controller/SpawnController";
include ":epicevents:Controller/ConditionController";
include ":epicevents:Model/EventConfig";
include ":epicevents:Model/ActionConfig";
include ":message:model/Message";

function ExecuteActions(item, event, actions)
	if (!actions || actions.size() == 0)
		return 0;
	endif

	var action_state := struct{};
	var action_index := 1;
	var any_executed := 0;

	foreach action in actions
		if (action.delay && action.delay > 0)
			sleepms(action.delay * 1000);
		endif

		if (action.condition)
			if (!EvaluateCondition(action.condition, action_state, event))
				action_index := action_index + 1;
				continue;
			endif
		endif

		var result := ExecuteAction(item, event, action, action_state);
		action_state.insert("action_" + action_index, result);

		if (result)
			any_executed := 1;
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
	var config := EventConfig();
	config.FromStruct(params);
	return SpawnAllCreatures(item, config);
endfunction

function ExecuteEmoteAction(item, event, params)
	var emote_text := params.text;

	if (!emote_text || emote_text == "")
		return 0;
	endif

	var emote_target := item;
	if (params.show_on_source && event.source)
		emote_target := event.source;
	endif

	PrintTextAbove(emote_target, emote_text);
	// Commenting because it wasn't working as expected
	// Message(emote_text).SendEmote(emote_target);
	return 1;
endfunction

function ExecuteTeleportAction(mobile, params)
	if (!mobile || !mobile.acctname)
		return 0;
	endif

	if (!params)
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
