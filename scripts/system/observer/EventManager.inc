include "exception/InvalidEventException";

include "models/Event";
include "models/EventListener";

class EventManager()
	var EventListeners := dictionary{};

	function EventManager(this)
		this.EventListeners := dictionary{};
	endfunction

	function AddListener(this, listener)
		if (!(listener is @EventListener))
			return InvalidEventException("Parameter 'listener' must be of type EventListener.");
		endif

		if (!(listener.GetType() in this.EventListeners))
			this.EventListeners[listener.GetType()] := array{};
		endif


		this.EventListeners[listener.GetType()].append(listener);
	endfunction

	function Notify(this, event)
		if (!(event is @Event))
			return InvalidEventException("Parameter 'event' must be of type Event.");
		endif

		if (event.GetType() in this.EventListeners)
			foreach listener in (this.EventListeners[event.GetType()])
				listener.OnEvent(event);
			endforeach
		endif
	endfunction
endclass