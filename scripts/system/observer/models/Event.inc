use uo;
use os;

include "IEvent";

class Event(IEvent)
	var PID;
	var Name;
	var Timestamp;

	function Event(this, source)
		this.PID := source.pid;
		this.Name := "";
		this.Timestamp := POLCore().systime;
	endfunction

	function GetPID(this)
		return this.PID;
	endfunction

	function SetPID(this, pid)
		this.PID := pid;
	endfunction

	function GetSource(this)
		return GetProcess(this.PID);
	endfunction

	function SetSource(this, source)
		this.Source := source;
	endfunction

	function GetName(this)
		return this.Name;
	endfunction

	function _SetName(this, eventName)
		this.Name := eventName;
	endfunction

	function GetTimestamp(this)
		return this.Timestamp;
	endfunction

	function ToString(this)
		return "Event[PID=" + this.PID + ", Source=" + this.Source + ", Name=" + this.Name + ", Payload=" + this.Payload + ", Timestamp=" + this.Timestamp + "]";
	endfunction
	
endclass