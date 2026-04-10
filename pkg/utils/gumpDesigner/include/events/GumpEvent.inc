include "IGumpEvent";

class GumpEvent(IGumpEvent)
	var _control;
	var _action;

	function GumpEvent(this, control, action := 0)
		this._control := control;
		this._action := action;
	endfunction

	function GetControl(this)
		return this._control;
	endfunction

	function SetControl(this, control)
		this._control := control;
		return this;
	endfunction

	function GetAction(this)
		return this._action;
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

enum class EventAction
	Closed
endenum