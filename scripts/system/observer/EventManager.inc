include "exception/InvalidEventException";

include "IEventListener";

class EventManager()
	var eventListeners := array{};

	function EventManager(this)
		this.eventListeners := array{};
	endfunction

	function addListener(this, listener)
		if (!listener || !(listener is @IEventListener))
			return InvalidEventException("Listener must implement IEventListener");
		endif

		this.eventListeners.append(listener);
	endfunction

	function notify(this)
		foreach listener in (this.eventListeners)
			listener.notify();
		endforeach
	endfunction
endclass