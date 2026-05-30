include "Control";

class RadioGroup(Control)
	var _groupId := 0;
	var _buttons := array{};

	function RadioGroup(this, name, x, y)
		Super(GUMP_CONTROL::RADIOGROUP, name, x, y);
	endfunction

	function GetGroupId(this)
		return this._groupId;
	endfunction

	function GetButtons(this)
		return this._buttons;
	endfunction

	function SetGroupId(this, groupId)
		this._groupId := groupId;
	endfunction

	function SetButtons(this, buttons)
		this._buttons := buttons;
		return this;
	endfunction

	function AddButton(this, button)
		this._buttons.append(button);
		return this;
	endfunction

endclass