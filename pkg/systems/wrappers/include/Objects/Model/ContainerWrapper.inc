include "IContainerWrapper";
include "ItemWrapper";

class ContainerWrapper(IContainerWrapper, ItemWrapper)
	var _weightMultiplier := 1.0;
	var _maxItems := 0;
	var _maxSlots := 0;
	var _maxWeight := 0;
	var _isLocked := false;

	function ContainerWrapper(this, byRef objRef)
		Super(objRef);
	endfunction

	function GetWeightMultiplier(this)
		return this._weightMultiplier;
	endfunction

	function GetMaxItems(this)
		return this._maxItems;
	endfunction

	function GetMaxSlots(this)
		return this._maxSlots;
	endfunction

	function GetMaxWeight(this)
		return this._maxWeight;
	endfunction
	
	function SetLocked(this, boolValue)
		this._isLocked := boolValue;
		return this;
	endfunction

	function SetWeightMultiplier(this, value)
		this._weightMultiplier := value;
		return this;
	endfunction

	function SetMaxItems(this, value)
		this._maxItems := value;
		return this;
	endfunction

	function SetMaxSlots(this, value)
		this._maxSlots := value;
		return this;
	endfunction

	function SetMaxWeight(this, value)
		this._maxWeight := value;
		return this;
	endfunction

	function IsLocked(this)
		return this._isLocked;
	endfunction

	function Contains(this, object)
		// Implementation to check if the container contains the specified object
	endfunction


	
endclass
