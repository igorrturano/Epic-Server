include "system/observer/models/IEventListener";
include "system/exception/ParamException/InvalidParameterException";
include "SysNotificationEvent";

class SysNotificationListener(IEventListener)
	function Notify(this, event)
		if (!(event is @SysNotificationEvent))
			return InvalidParameterException("Parameter 'event' must be of type SysNotificationEvent.");
		endif
		
		foreach target in (event.GetTargets())
			event.GetMessage().SendSys(target);
		endforeach
	endfunction
endclass