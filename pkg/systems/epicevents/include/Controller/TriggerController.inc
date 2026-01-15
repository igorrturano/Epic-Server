use uo;
use polsys;

include ":epicevents:Model/TriggerConfig";

// Main entry point - processes events
function ProcessTriggerEvent(item, event, triggerConfig)
	// Check cooldown first
	if (!CanTrigger(item, triggerConfig))
		return 0;
	endif

	// Validate trigger based on type
	case (triggerConfig.type)
		"SPEECH":
			return ValidateSpeechTrigger(event, triggerConfig);
		"ENTEREDAREA":
			return ValidateAreaTrigger(event, triggerConfig);
		"LEFTAREA":
			return ValidateAreaTrigger(event, triggerConfig);
		"ITEM_GIVEN":
			return ValidateItemGivenTrigger(event, triggerConfig);
		"DOUBLECLICKED":
			return ValidateDoubleClickTrigger(event, triggerConfig);
		default:
			print("[EpicEvents] WARNING - Unknown trigger type: '" + triggerConfig.type + "' - returning 0");
			return 0;
	endcase
endfunction

function ValidateSpeechTrigger(event, triggerConfig)
	var text := Lower(event.text);
	var keyword := Lower(triggerConfig.GetKeyword());

	if (!keyword || keyword == "")
		return 0;
	endif

	// Exact match or pattern match
	if (text == keyword || Find(text, keyword, 1))
		return 1;
	endif
	return 0;
endfunction

function ValidateAreaTrigger(event, triggerConfig)
	// Only trigger for players, not NPCs
	if (!event.source.acctname)
		return 0;
	endif

	return 1;
endfunction

function CanTrigger(item, triggerConfig)
	var next_trigger_time := GetObjProperty(item, "next_trigger_time");

	if (!next_trigger_time)
		return 1;
	endif

	var current_time := polcore().systime;
	if (current_time >= next_trigger_time)
		return 1;
	else
		return 0;
	endif
endfunction

function UpdateTriggerCooldown(item, cooldown_seconds)
	var next_time := polcore().systime + cooldown_seconds;
	SetObjProperty(item, "next_trigger_time", next_time);
	print("[EpicEvents] Cooldown set - next trigger in " + (cooldown_seconds / 60) + " minutes");
endfunction
