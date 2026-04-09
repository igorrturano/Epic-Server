use uo;
use os;

include "IEvent";

class Event(IEvent)
	var PID; // PID do processo que gerou o evento, para facilitar acesso sem precisar chamar GetProcess() repetidamente.
	var Source; // Instancia do processo que gerou o evento.
	var Name;
	var Timestamp;
	var Payload; // Qualquer dado adicional que o evento queira carregar. Pode ser acessado pelos listeners para obter mais contexto sobre o evento.

	function Event(this, source)
		this.PID := source.pid;
		this.Source := source;
		this.Name := "";
		this.Timestamp := POLCore().systime;
		this.Payload := 0;
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

	function SetTimestamp(this, timestamp)
		this.Timestamp := timestamp;
	endfunction

	function GetPayload(this)
		return this.Payload;
	endfunction

	function SetPayload(this, payload)
		this.Payload := payload;
	endfunction

	function ToString(this)
		return "Event[PID=" + this.PID + ", Source=" + this.Source + ", Name=" + this.Name + ", Payload=" + this.Payload + ", Timestamp=" + this.Timestamp + "]";
	endfunction
	
endclass