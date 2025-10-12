include "IAttributeModifier";

include ":attributes:attributeConstants";

class AttributeModifier(IAttributeModifier)
	var _attribute := ""; // enum
	var _value := 0;

	function AttributeModifier(this, attribute := "", value := 0)
		this._attribute := attribute;
		this._value := CInt(value);
	endfunction

	function GetAttribute(this)
		return this._attribute;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function SetValue(this, value)
		this._value := CInt(value);
		return this;
	endfunction

	function DecreaseValue(this, value)
		this._value -= CInt(value);
		return this;
	endfunction

	function IncreaseValue(this, value)
		this._value += CInt(value);
		return this;
	endfunction

	function ToString(this)
		return this._attribute + ": " + this._value;
	endfunction
endclass

enum class COMBATPENALTY_ATTRIBUTE
	STRENGTH,
	DEXTERITY,
	INTELLIGENCE,
	SNEAK,
	MECHANICS,
	MAGERY,
	AWARENESS
endenum