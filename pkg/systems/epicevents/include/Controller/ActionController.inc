use uo;
use os;

include ":epicevents:Controller/ConditionController";
include ":epicevents:Model/ActionConfig";

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
	var script_path := ":epicevents:actions/" + action.type;
	var script_params := array{item, event, action.params, action_state};

	var script := start_script(script_path, script_params);

	if (!script || script.errortext)
		print("[EpicEvents] ERROR: Failed to start action script [" + action.type + "]: " + script.errortext);
		return 0;
	endif

	return script.wait();
endfunction
