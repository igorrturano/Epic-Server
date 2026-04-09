include "system/observer/models/IEventListener";
include "system/exception/ParamException/InvalidParameterException";
include "Event";

class EventListener(IEventListener)
	var Type := EventType::None;

	function EventListener(this)
		this.Type := EventType::None;
	endfunction

	function GetType(this)
		return this.Type;
	endfunction
	
	uninit function OnEvent(this, event);
endclass