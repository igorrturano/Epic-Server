include "exception/InvalidEventException";

include "models/IEventListener";

class EventManager()
	var EventListeners := array{};

	function EventManager(this)
		this.EventListeners := array{};
	endfunction

	function AddListener(this, listener)
		if (!(listener is @IEventListener))
			return InvalidEventException("Listener must implement IEventListener");
		endif

		this.EventListeners.append(listener);
	endfunction

	function Notify(this, event)
		if (!(event is @Event))
			return InvalidEventException("Parameter 'event' must be of type Event.");
		endif

		foreach listener in (this.EventListeners)
			listener.Notify(event);
		endforeach
	endfunction
endclass