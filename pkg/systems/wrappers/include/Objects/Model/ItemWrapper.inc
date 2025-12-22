use uo;

include "UObjectWrapper";
include "IItemWrapper";
include "ContainerWrapper";

include ":io:CFGFile";

class ItemWrapper(IItemWrapper, UObjectWrapper)
	function ItemWrapper(this, byRef item)
		Super(item);

		this._cfgFile := CFGFile("*", "itemdesc");
		this._template := this._cfgFile.SetElementName(this._objtype).GetElement();

		if (TypeOfInt(this._objRef) == OT_ITEMREF)
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

	/**
	 * Obtém a quantidade do item.
	 *
	 * @return Integer com a quantidade do item ou Exception se o item for inválido
	 */
	function GetAmount(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.amount;
	endfunction

	/**
	 * Obtém o container que contém este item.
	 *
	 * @return ContainerWrapper do container que contém o item ou Exception se o item for inválido
	 */
	function GetContainer(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return ContainerWrapper(this._objRef.container);
	endfunction

	/**
	 * Obtém a qualidade do item.
	 *
	 * @return Double com o valor de qualidade do item ou Exception se o item for inválido
	 */
	function GetQuality(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return CDbl(this._objRef.quality);
	endfunction
	
	/**
	 * Verifica se este item pode ser empilhado com outro item.
	 *
	 * @param otherItemWrapped ItemWrapper do outro item a ser verificado
	 * @return Boolean true se os itens podem ser empilhados juntos, false caso contrário
	 * @throws InvalidFormatException Se o parâmetro não for um objeto ItemWrapper
	 */
	function IsStackableWith(this, byRef otherItemWrapped)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (!(otherItemWrapped is @ItemWrapper))
			return InvalidFormatException("The param must be a ItemWrapper object!");
		endif

		return IsStackable(this._objRef, otherItemWrapped.GetObjRef());
	endfunction

	/**
	 * Verifica se o item pode ser movido.
	 *
	 * @return Boolean true se o item é movível, false caso contrário, ou Exception se o item for inválido
	 */
	function IsMovable(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.movable == true;
	endfunction
	
	/**
	 * Reserva o item para uso exclusivo, impedindo modificações concorrentes.
	 *
	 * @return Boolean indicando sucesso da operação de bloqueio ou Exception se o item for inválido
	 * @see Unlock
	 */
	function Lock(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return ReserveItem(this._objRef);
	endfunction

	/**
	 * Libera o item previamente reservado, permitindo modificações.
	 *
	 * @return Boolean indicando sucesso da operação de desbloqueio ou Exception se o item for inválido
	 * @see Lock
	 */
	function Unlock(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return ReleaseItem(this._objRef);
	endfunction

	/**
	 * Consome uma quantidade específica do item.
	 *
	 * Esta função reduz a quantidade do item pelo valor especificado.
	 *
	 * @param amount Quantidade a ser consumida do item
	 * @return Boolean indicando se a operação foi bem-sucedida ou Exception se o item for inválido
	 */
	function Consume(this, amount)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return SubtractAmount(this._objRef, CInt(amount));
	endfunction

	/**
	 * Atualiza o item no jogo, enviando mudanças para todos os clientes.
	 *
	 * @return Boolean indicando sucesso da atualização ou Exception se o item for inválido
	 */
	function Update(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return UpdateItem(this._objRef);
	endfunction

	/**
	 * Move o item para um container especificado.
	 *
	 * @param containerWrapped Referência encapsulada do container de destino (ContainerWrapper)
	 * @param x Coordenada X dentro do container (padrão: -1 para posição automática)
	 * @param y Coordenada Y dentro do container (padrão: -1 para posição automática)
	 * @param add_to_existing_stack Se deve adicionar a uma pilha existente (padrão: 0)
	 * @return Boolean indicando sucesso da movimentação ou Exception se o item for inválido
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
	QUIVER := "Quiver",
	BANDOLIER := "Bandolier"
endenum
