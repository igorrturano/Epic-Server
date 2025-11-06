class EventHandler()
	var _parent;
	var _control;
	var _action;

	function EventHandler(this, control, action := 0)
		this._control := control;
		this._action := action;
	endfunction

	function GetParent(this)
		return this._parent;
	endfunction

	function GetControl(this)
		return this._control;
	endfunction

	function GetAction(this)
		return this._action;
	endfunction

	function SetParent(this, parent)
		this._parent := parent;
		return this;
	endfunction

	function SetControl(this, control)
		this._control := control;
		return this;
	endfunction

	function SetAction(this, action)
		this._action := action;
		return this;
	endfunction

	function ToString(this)
		return "EventHandled: (control=" + this._control
			+ ", action=" + this._action
			+ ")";
	endfunction
endclass

enum class EVENT_ACTION
	CLOSED
endenum