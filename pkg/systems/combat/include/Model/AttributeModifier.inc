include "IAttributeModifier";

include ":attributes:attributeConstants";
include ":attributes:skills";

class AttributeModifier(IAttributeModifier)
	var _attribute := ""; // attribute constant in attributeConstants.inc
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

	function ApplyOn(this, who)
		AP_SetSkillMod(
			who,
			this._attribute,
			AP_GetSkillMod(who, this._attribute) + this._value
		);
	endfunction

	function RemoveFrom(this, who)
		AP_SetSkillMod(
			who,
			this._attribute,
			AP_GetSkillMod(who, this._attribute) - this._value
		);
	endfunction

	function ToString(this)
		return this._attribute + ": " + this._value;
	endfunction
endclass
