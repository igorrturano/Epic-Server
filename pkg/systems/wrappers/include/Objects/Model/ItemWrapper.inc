use uo;

include "UObjectWrapper";
include "IItemWrapper";
include "ContainerWrapper";

include ":io:CFGFile";

class ItemWrapper(IItemWrapper, UObjectWrapper)
	var _category := ""; // ITEM_CATEGORY enum

	function ItemWrapper(this, byRef item)
		Super(item);

		this._cfgFile := CFGFile("*", "itemdesc");
		this._template := this._cfgFile.SetElementName(this._objtype).GetElementValue();

		if (TypeOfInt(item) == OT_ITEMREF)
			this._category := this._template.Category;
		else
			var err;
			if (!this._template)
				err := InvalidFormatException("The param must be a Item ObjRef or a valid Template!");
				this._objType := err;
				this._name := err;
				this._graphic := err;
				this._color := err;
				this._weight := err;

				this._category := err;
			else
				err := IsTemplateException();
				this._name := this._template.desc;
				this._graphic := this._template.graphic;
				this._color := this._template.color;
				this._weight := this._template.weight;

				this._category := this._template.Category;
			endif

			this._objRef := err;
			this._serial := err;
		endif
	endfunction

	function GetCategory(this)
		return this._category;
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

		return ContainerWrapper(this._objRef.container);
	endfunction

	function GetQuality(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return CDbl(this._objRef.quality);
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

	function IsMovable(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.movable == true;
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

	/**
	 * Move o item para um container especificado.
	 *
	 * @param containerWrapped Referência encapsulada do container de destino
	 * @param x Coordenada X dentro do container (padrão: -1 para posição automática)
	 * @param y Coordenada Y dentro do container (padrão: -1 para posição automática)
	 * @param add_to_existing_stack Se deve adicionar a uma pilha existente (padrão: 0)
	 * @return Resultado da operação de movimentação ou objeto Exception em caso de erro
	 */
	function MoveToContainer(this, byRef containerWrapped, x := -1, y := -1, add_to_existing_stack := 0)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return MoveItemToContainer(this._objRef, containerWrapped.GetObjRef(), x, y, add_to_existing_stack);
	endfunction

	function ToString(this)
		return $"Item {this._name}:" +
			   $"Serial: {this._serial}, " +
			   $"ObjType: {this._objType}, " +
			   $"Graphic: {this._graphic}, " +
			   $"Color: {this._color}, " +
			   $"Weight: {this._weight}, " +
			   $"Category: {this._category}";
	endfunction
endclass

enum class ITEM_TYPE
	ARMOR,
	WEAPON,
	CONTAINER
endenum

enum class ITEM_CATEGORY
	QUIVER := "Quiver"
endenum
