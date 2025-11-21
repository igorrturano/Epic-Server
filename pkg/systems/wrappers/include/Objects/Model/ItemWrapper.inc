use uo;

include "UObjectWrapper";
include "IItemWrapper";

class ItemWrapper(IItemWrapper, UObjectWrapper)
	function ItemWrapper(this, byRef item)
		Super(item);

		var cfgFile := CFGFile("*", "itemdesc");
		if (TypeOfInt(item) == OT_ITEMREF) // Caso o itemRef exista
			this._template := cfgFile.SetElementName(item.objtype).GetElementValue();
		else // Caso seja apenas um template
			this._template := cfgFile.SetElementName(item).GetElementValue();
			this._objRef := error{errortext := "This item is just a template!"};
			this._name := this._template.desc;
			this._graphic := this._template.graphic;
			this._color := this._template.color;
			this._weight := this._template.weight;
		endif
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

	/*
	 * Consome uma quantidade específica do item.
	 *
	 * Esta função reduz a quantidade do item pelo valor especificado.
	 *
	 * @param this     Referência ao objeto ItemWrapper atual
	 * @param amount   Quantidade a ser consumida do item
	 *
	 * @return Retorna o resultado da função SubtractAmount, indicando se a 
	 *         operação foi bem-sucedida ou não
	 */
	function Consume(this, amount)
		return SubtractAmount(this._objRef, CInt(amount));
	endfunction

	function Update(this)
		return UpdateItem(this._objRef);
	endfunction

	function MoveToContainer(this, byRef containerWrapped, x := -1, y := -1, add_to_existing_stack := 0)
		return MoveItemToContainer(this._objRef, containerWrapped.GetObjRef(), x, y, add_to_existing_stack);
	endfunction
endclass
