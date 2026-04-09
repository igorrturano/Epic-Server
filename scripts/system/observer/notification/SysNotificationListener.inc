include "system/observer/models/IEventListener";
include "system/exception/ParamException/InvalidParameterException";
include "SysNotificationEvent";

class SysNotificationListener(IEventListener)
	var Type := EventType::SysNotification;

	function SysNotificationListener(this)
		this.Type := EventType::SysNotification;
	endfunction

	function GetType(this)
		return this.Type;
	endfunction
	
	function OnEvent(this, event)
		if (event is @SysNotificationEvent)
			foreach target in (event.GetTargets())
				event.GetMessage().SendSys(target);
			endforeach
		endif
	endfunction
endclass