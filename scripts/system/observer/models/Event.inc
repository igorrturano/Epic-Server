include "IEvent";

class Event(IEvent)
	var PID;
	var Source; 
	var Name;
	var Payload;
	var Timestamp;

	function Event(this)
		this.PID := 0;
		this.Source := 0;
		this.Name := "";
		this.Payload := 0;
		this.Timestamp := 0;
	endfunction

	function GetPID(this)
		return this.PID;
	endfunction

	function SetPID(this, pid)
		this.PID := pid;
	endfunction

	function GetSource(this)
		return this.Source;
	endfunction

	function SetSource(this, source)
		this.Source := source;
	endfunction

	function GetEventName(this)
		return this.Name;
	endfunction

	function SetEventName(this, eventName)
		this.Name := eventName;
	endfunction

	function GetPayload(this)
		return this.Payload;
	endfunction

	function SetPayload(this, payload)
		this.Payload := payload;
	endfunction

	function GetTimestamp(this)
		return this.Timestamp;
	endfunction

	function SetTimestamp(this, timestamp)
		this.Timestamp := timestamp;
	endfunction

	function ToString(this)
		return "Event[PID=" + this.PID + ", Source=" + this.Source + ", Name=" + this.Name + ", Payload=" + this.Payload + ", Timestamp=" + this.Timestamp + "]";
	endfunction
	
endclass