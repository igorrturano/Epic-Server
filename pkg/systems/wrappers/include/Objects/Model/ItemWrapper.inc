use uo;

include "UObjectWrapper";
include "IItemWrapper";

include ":io:CFGFile";

class ItemWrapper(IItemWrapper, UObjectWrapper)
	function ItemWrapper(this, byRef item)
		Super(item);

		this._cfgFile := CFGFile("*", "itemdesc");
		this._template := this._cfgFile.SetElementName(this._objtype).GetElementValue();

		if (TypeOfInt(item) == OT_ITEMREF)
			// Feito assim para caso precisar personalizar algo futuramente
		else
			var err;
			if (!this._template)
				err := InvalidFormatException("The param must be a Item ObjRef or a valid Template!");
				this._name := err;
				this._graphic := err;
				this._color := err;
				this._weight := err;
			else
				err := IsTemplateException();
				this._name := this._template.desc;
				this._graphic := this._template.graphic;
				this._color := this._template.color;
				this._weight := this._template.weight;
			endif

			// Aqui ficam os que recebem erro de qualquer jeito
			this._objRef := err;
			this._serial := err;
		endif
	endfunction

	function GetAmount(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.amount;
	endfunction

	function GetContainer(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		// TODO: Retornar um ContainerWrapper ao invés do ObjRef puro
		return this._objRef.container;
	endfunction
	
	function IsStackableWith(this, byRef otherItemWrapped)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (!(otherItemWrapped is @ItemWrapper))
			return InvalidFormatException("The param must be a ItemWrapper object!");
		endif

		return IsStackable(this._objRef, otherItemWrapped.GetObjRef());
	endfunction
	
	function Lock(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return ReserveItem(this._objRef);
	endfunction

	function Unlock(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

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
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return SubtractAmount(this._objRef, CInt(amount));
	endfunction

	function Update(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return UpdateItem(this._objRef);
	endfunction

	function MoveToContainer(this, byRef containerWrapped, x := -1, y := -1, add_to_existing_stack := 0)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return MoveItemToContainer(this._objRef, containerWrapped.GetObjRef(), x, y, add_to_existing_stack);
	endfunction
endclass
