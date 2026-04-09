use uo;
use os;

include "IEvent";

class Event(IEvent)
	var Type; // ENUM indicando o tipo do evento, para facilitar filtragem pelos listeners.
	var PID; // PID do processo que gerou o evento, para facilitar acesso sem precisar chamar GetProcess() repetidamente.
	var Source; // Instancia do listener que gerou o evento.
	var Name;
	var Timestamp;
	var Payload; // Qualquer dado adicional que o evento queira carregar. Pode ser acessado pelos listeners para obter mais contexto sobre o evento.

	function Event(this, source)
		this.Type := EventType::None; // Default, deve ser sobrescrito por eventos específicos
		this.PID := GetProcess().pid;
		this.Source := source;
		this.Name := "";
		this.Timestamp := POLCore().systime;
		this.Payload := 0;
	endfunction

	function GetType(this)
		return this.Type;
	endfunction
	
	function GetPID(this)
		return this.PID;
	endfunction

	function SetPID(this, pid)
		this.PID := pid;
	endfunction

	function GetEventProcess(this)
		return GetProcess(this.PID);
	endfunction

	function GetSource(this)
		return this.Source;
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
		return "Event{type: " + CStr(this.Type) +
				", pid: " + CStr(this.PID) +
				", source: " + CStr(this.Source) +
				", name: " + this.Name +
				", timestamp: " + CStr(this.Timestamp) +
				", payload: " + CStr(this.Payload) + "}";
	endfunction
endclass

enum class EventType
	None := "none",
	SysNotification := "sysNotification",

	// Outros tipos de eventos podem ser adicionados aqui conforme necessário
endenum