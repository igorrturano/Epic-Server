include "Tooltip";

class TooltipItem(Tooltip)
	var _itemSerial;

	function TooltipItem(this, name, itemSerial)
		Super(name);
		this._itemSerial := itemSerial;
	endfunction

	function GetItemSerial(this)
		return this._itemSerial;
	endfunction

	function SetItemSerial(this, itemSerial)
		this._itemSerial := itemSerial;
	endfunction

	function SelfCreate(this, byRef gump)
		GFItemProperty(gump, this.GetItemSerial());
	endfunction

	function ToString(this)
		return "TooltipItem(type=" + this.GetType()
			+ ", name=" + this._GetName()
			+ ", itemSerial=" + this._itemSerial
			+ ")";
	endfunction
endclass