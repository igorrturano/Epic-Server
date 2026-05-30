use uo;
use polsys;

include ":epicevents:Model/TriggerConfig";
include "include/sysEvent";

// Main entry point - processes events
function ProcessTriggerEvent(item, event, triggerConfig)
	// Check cooldown first
	if (!CanTrigger(item, triggerConfig))
		return 0;
	endif

	// Reject events whose type doesn't match the configured trigger.
	// Without this, custom events broadcast via SendEvent (e.g. EVID_NOISE
	// from EmitNoise) fall through to validators that return 1 unconditionally
	// and spuriously fire actions on unrelated nearby event NPCs.
	if (!EventTypeMatchesTrigger(event, triggerConfig.type))
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
			return 0;
	endcase
endfunction

function EventTypeMatchesTrigger(event, trigger_type)
	var ev_type := event.type;
	if (ev_type == error || !ev_type)
		return 0;
	endif

	case (trigger_type)
		"SPEECH":         return (ev_type == SYSEVENT_SPEECH);
		"ENTEREDAREA":    return (ev_type == SYSEVENT_ENTEREDAREA);
		"LEFTAREA":       return (ev_type == SYSEVENT_LEFTAREA);
		"ITEM_GIVEN":     return (ev_type == SYSEVENT_ITEM_GIVEN);
		"DOUBLECLICKED":  return (ev_type == SYSEVENT_DOUBLECLICKED);
	endcase
	return 0;
endfunction

function ValidateSpeechTrigger(event, triggerConfig)
	var text := Lower(event.text);
	var keyword := Lower(triggerConfig.GetKeyword());

	if (!keyword || keyword == "")
		return 0;
	endif

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
	endif

	return 0;
endfunction

function UpdateTriggerCooldown(item, cooldown_seconds)
	var next_time := polcore().systime + cooldown_seconds;
	SetObjProperty(item, "next_trigger_time", next_time);
endfunction

function ValidateItemGivenTrigger(event, triggerConfig)
	var required_items := triggerConfig.GetRequiredItems();

	if (!required_items || required_items.size() == 0)
		return 0;
	endif

	// Check if given item matches required items
	foreach req_item in required_items
		if (event.item.objtype == req_item.objtype)
			if (!req_item.amount || event.item.amount >= req_item.amount)
				return 1;
			endif
		endif
	endforeach

	return 0;
endfunction

function ValidateDoubleClickTrigger(event, triggerConfig)
	// Double-click always valid if event matches
	return 1;
endfunction