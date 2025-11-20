use uo;

include "UObjectWrapper";
include "IItemWrapper";

class ItemWrapper(IItemWrapper, UObjectWrapper)
	function ItemWrapper(this, byRef item)
		Super(item);
	endfunction

	function IsStackableWith(this, byRef otherItemWrapped)
		return IsStackable(this._objRef, otherItemWrapped.GetObjRef());
	endfunction
	
	function Lock(this)
		return ReserveItem(this._objRef);
	endfunction

	function Unlock(this)
		return ReleaseItem(this._objRef);
	endfunction

	function Consume(this, amount)
		return SubtractAmount(this._objRef, CInt(amount));
	endfunction

	function Update(this)
		return UpdateItem(this._objRef);
	endfunction

	function MoveToContainer(this, byRef containerWrapped, x := -1, y := -1, add_to_existing_stack := 0)
		return MoveItemToContainer(this._objRef, containerWrapped.GetObjRef(), x, y, add_to_existing_stack);
	endfunction

	function MoveToTrade(this, byRef mobileWrapped)
		return MoveItemToSecureTradeWin(this._objRef, mobileWrapped.GetObjRef());
	endfunction
endclass
