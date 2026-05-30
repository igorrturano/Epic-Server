include "IAttribute";

class Attribute(IAttribute)
	var _name := "";
	var _baseValue := 0;
	var _modValue := 0;
	
	function Attribute(this, name)
	endfunction

	function GetName(this)
	endfunction

	function GetTotalValue(this)
	endfunction

	function GetBaseValue(this)
	endfunction

	function GetModValue(this)
	endfunction

	function SetBaseValue(this, value)
	endfunction

	function SetModValue(this, value)
	endfunction

	function IncreaseBaseValue(this, value)
	endfunction

	function IncreaseModValue(this, value)
	endfunction

	function DecreaseBaseValue(this, value)
	endfunction

	function DecreaseModValue(this, value)
	endfunction

	uninit function ToString(this);
endclass