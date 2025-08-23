include "Control";

class RadioGroup(Control)
	var _groupId;
	var _buttons;

	function RadioGroup(this, name, x, y)
		Super(GUMP_CONTROL::RADIOGROUP, name, x, y);
		this._buttons := array{};
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
	endfunction

	function AddButton(this, button)
		this._buttons.append(button);
	endfunction

endclass