use uo;

include "UObjectWrapper";
include "IItemWrapper";

class ItemWrapper(UObjectWrapper, IItemWrapper)
	function ItemWrapper(this, byRef item)
		Super(item);
	endfunction

	function MoveToContainer(this, containerWrapped, x := -1, y := -1, add_to_existing_stack := 0)
		return MoveItemToContainer(this._objRef, containerWrapped.GetObjRef(), x, y, add_to_existing_stack);
	endfunction

	function MoveToTrade(this, mobileWrapped)
		return MoveItemToSecureTradeWin(this._objRef, mobileWrapped.GetObjRef());
	endfunction
endclass
