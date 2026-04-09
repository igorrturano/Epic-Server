class IEvent()
	function IEvent(this)
	endfunction
	
	uninit function GetPID(this);
	uninit function GetSource(this);
	uninit function GetName(this);
	uninit function GetPayload(this);
	uninit function GetTimestamp(this);
endclass