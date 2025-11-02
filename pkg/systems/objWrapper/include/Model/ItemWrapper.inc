include "UObjectWrapper";
include "IItemWrapper";

class ItemWrapper(UObjectWrapper, IItemWrapper)
	function ItemWrapper(this, byRef item)
		Super(item);
	endfunction
endclass