use uo;

include ":epicevents:Controller/SpawnController";
include ":epicevents:Controller/ConditionController";
include ":epicevents:Model/EventConfig";
include ":epicevents:Model/ActionConfig";

function ExecuteActions(item, event, actions)
	if (!actions || actions.size() == 0)
		print("[EpicEvents] No actions to execute");
		return 0;
	endif

	var action_state := struct{};  // Track state for conditions
	var action_index := 1;

	print("[EpicEvents] Executing " + actions.size() + " action(s)");

	foreach action in actions
		print("[EpicEvents] Action " + action_index + ": " + action.type);

		// Wait if delay configured
		if (action.delay && action.delay > 0)
			print("[EpicEvents] Delaying " + action.delay + " seconds before action");
			sleepms(action.delay * 1000);
		endif

		// Evaluate condition if exists
		if (action.condition)
			print("[EpicEvents] Evaluating condition: " + action.condition.type);
			if (!EvaluateCondition(action.condition, action_state, event))
				print("[EpicEvents] Condition failed - skipping action");
				action_index := action_index + 1;
				continue;  // Skip this action
			endif
			print("[EpicEvents] Condition passed - executing action");
		endif

		// Execute action
		var result := ExecuteAction(item, event, action, action_state);

		// Store result for subsequent conditions
		action_state.insert("action_" + action_index, result);

		if (result)
			print("[EpicEvents] Action " + action_index + " completed successfully");
		else
			print("[EpicEvents] Action " + action_index + " failed");
		endif

		action_index := action_index + 1;
	endforeach

	print("[EpicEvents] All actions processed");
	return 1;
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
	print("[EpicEvents] Executing SPAWN action");
	// Delegate to SpawnController (V1 logic)
	var config := EventConfig();
	config.FromStruct(params);
	return SpawnAllCreatures(item, config);
endfunction

function ExecuteEmoteAction(item, event, params)
	print("[EpicEvents] Executing EMOTE action");
	var emote_text := params.text;
	var range := params.range ? params.range : 10;

	if (!emote_text || emote_text == "")
		print("[EpicEvents] EMOTE action - no text specified");
		return 0;
	endif

	// Show emote above item or source
	if (params.show_on_source && event.source)
		PrintTextAbove(event.source, emote_text);
		print("[EpicEvents] Emote displayed above player: " + emote_text);
	else
		PrintTextAbove(item, emote_text);
		print("[EpicEvents] Emote displayed above item: " + emote_text);
	endif

	// Notify nearby players
	var chars := ListMobilesNearLocation(item.x, item.y, item.z, range, item.realm);
	var player_count := 0;
	foreach char in chars
		if (char.acctname)
			SendSysMessageEx(char, emote_text, SSM_INFO);
			player_count := player_count + 1;
		endif
	endforeach

	print("[EpicEvents] Emote sent to " + player_count + " player(s) within " + range + " tiles");
	return 1;
endfunction

function ExecuteTeleportAction(mobile, params)
	print("[EpicEvents] Executing TELEPORT action");

	if (!mobile || !mobile.acctname)
		print("[EpicEvents] TELEPORT action - invalid mobile or not a player");
		return 0;
	endif

	var x := CInt(params.x);
	var y := CInt(params.y);
	var z := CInt(params.z);
	var realm := params.realm ? params.realm : mobile.realm;

	if (!x || !y)
		print("[EpicEvents] TELEPORT action - invalid coordinates");
		return 0;
	endif

	MoveObjectToLocation(mobile, x, y, z, realm, MOVEOBJECT_FORCELOCATION);
	print("[EpicEvents] Player " + mobile.name + " teleported to " + x + "," + y + "," + z);
	return 1;
endfunction
