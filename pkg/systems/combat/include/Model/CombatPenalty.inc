include "ICombatPenalty";

include ":attributes:attributeConstants";

class CombatPenalty(ICombatPenalty)
	var _attribute := "";
	var _value := 0;

	function CombatPenalty(this, attribute := "", value := 0)
		this._attribute := attribute;
		this._value := CInt(value);
	endfunction

	function GetAttribute(this)
		return this._attribute;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function SetAttribute(this, attribute)
		this._attribute := attribute;
	endfunction

	function SetValue(this, value)
		this._value := CInt(value);
	endfunction

	function IncreaseValue(this, value)
		this._value += CInt(value);
	endfunction

	function DecreaseValue(this, value)
		if ( this._value == 0 ) return; endif
		this._value -= CInt(value);
	endfunction

	function ToString(this)
		return this._attribute + ": " + this._value;
	endfunction
endclass