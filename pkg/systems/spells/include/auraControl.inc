include "include/sysEvent";

function AC_start(mobile)
	//SendSysMessage(mobile, "->"+"AC_start");
	if (mobile.name == "Zedh")
		EnableEvents(SYSEVENT_ENTEREDAREA, 10);
 		EnableEvents(SYSEVENT_LEFTAREA, 10);
		SendSysMessage(mobile, "->"+"mobile eh  zedh");
		var event;
		//while (mobile.connected)		
			event := Wait_For_Event(10000);
			if( event.type == SYSEVENT_ENTEREDAREA )
				SendSysMessage(mobile, "->"+"entraram na area do clerigo");
			elseif( event.type == SYSEVENT_LEFTAREA )
				SendSysMessage(mobile, "->"+"sairam da area do clerigo");
			endif		
		//endwhile
		
		
	endif

	return 1;
endfunction