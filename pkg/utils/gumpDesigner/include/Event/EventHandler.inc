class EventHandler()
	var _parent;
	var _event;
	var _action;

	function EventHandler(this, event, action := 0)
		this._event := event;
		this._action := action;
	endfunction

	function GetParent(this)
		return this._parent;
	endfunction

	function GetEvent(this)
		return this._event;
	endfunction

	function GetAction(this)
		return this._action;
	endfunction

	function SetParent(this, parent)
		this._parent := parent;
	endfunction

	function SetEvent(this, event)
		this._event := event;
	endfunction

	function SetAction(this, action)
		this._action := action;
	endfunction

	function ToString(this)
		return "EventHandled: (event=" + this._event
			+ ", action=" + this._action
			+ ")";
	endfunction
endclass