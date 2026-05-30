use uo;

include "UObjectWrapper";
include "IItemWrapper";
include "ContainerWrapper";
include "MobileWrapper";

include ":io:CFGFile";

class ItemWrapper(IItemWrapper, UObjectWrapper)
	function ItemWrapper(this, byRef item)
		Super(item);

		this.CfgFile := CFGFile("*", "itemdesc");
		this.Template := this.GetCfgFile().SetElementName(this.ObjType).GetElement();

		if (TypeOfInt(this.ObjRef) == OT_ITEMREF)
			this.Category := this.GetTemplate().Category;
		else
			var err;
			if (!this.Template)
				err := InvalidFormatException("The param must be a Item ObjRef or a valid Template!");
				this.ObjType := err;
				this.Name := err;
				this.Graphic := err;
				this.Color := err;
				this.Weight := err;

				this.Category := err;
			else
				err := IsTemplateException();
				this.Name := this.GetTemplate().desc;
				this.Graphic := this.GetTemplate().graphic;
				this.Color := this.GetTemplate().color;
				this.Weight := this.GetTemplate().weight;
				this.Category := this.GetTemplate().Category;
			endif

			this.ObjRef := err;
			this.Serial := err;
		endif
	endfunction

	// override
	function GetName(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().desc;
	endfunction

	// override
	function _SetName(this, name)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		Super::_SetName(this, name);
		this.GetObjRef().desc := name;

		return this;
	endfunction

	/**
	 * Obtém a quantidade do item.
	 *
	 * @return Integer com a quantidade do item ou Exception se o item for inválido
	 */
	function GetAmount(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().amount;
	endfunction

	/**
	 * Obtém o container que contém este item.
	 *
	 * @return ContainerWrapper do container que contém o item ou Exception se o item for inválido
	 */
	function GetContainer(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return ContainerWrapper(this.GetObjRef().container);
	endfunction

	/**
	 * Obtém o dono (mobile) deste item.
	 *
	 * @return MobileWrapper do dono do item ou Exception se o item for inválido
	 */
	function GetOwner(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return MobileWrapper(this.GetObjRef().character_owner);
	endfunction

	/**
	 * Obtém a qualidade do item.
	 *
	 * @return Double com o valor de qualidade do item ou Exception se o item for inválido
	 */
	function GetQuality(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return CDbl(this.GetObjRef().quality);
	endfunction

	function GetHP(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().hp;
	endfunction

	function SetHP(this, hp)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		this.GetObjRef().hp := hp;

		return this;
	endfunction

	function GetMaxHP(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().maxhp;
	endfunction
	
	/**
	 * Verifica se este item pode ser empilhado com outro item.
	 *
	 * @param otherItemWrapped ItemWrapper do outro item a ser verificado
	 * @return Boolean true se os itens podem ser empilhados juntos, false caso contrário
	 * @throws InvalidFormatException Se o parâmetro não for um objeto ItemWrapper
	 */
	function IsStackableWith(this, byRef otherItemWrapped)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(otherItemWrapped is @ItemWrapper))
			return InvalidFormatException("The param must be a ItemWrapper object!");
		endif

		return IsStackable(this.GetObjRef(), otherItemWrapped.GetObjRef());
	endfunction

	/**
	 * Verifica se o item pode ser movido.
	 *
	 * @return Boolean true se o item é movível, false caso contrário, ou Exception se o item for inválido
	 */
	function IsMovable(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().movable == true;
	endfunction
	
	/**
	 * Reserva o item para uso exclusivo, impedindo modificações concorrentes.
	 *
	 * @return Boolean indicando sucesso da operação de bloqueio ou Exception se o item for inválido
	 * @see Unlock
	 */
	function Lock(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return ReserveItem(this.GetObjRef());
	endfunction

	/**
	 * Libera o item previamente reservado, permitindo modificações.
	 *
	 * @return Boolean indicando sucesso da operação de desbloqueio ou Exception se o item for inválido
	 * @see Lock
	 */
	function Unlock(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return ReleaseItem(this.GetObjRef());
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return SubtractAmount(this.GetObjRef(), CInt(amount));
	endfunction

	/**
	 * Atualiza o item no jogo, enviando mudanças para todos os clientes.
	 *
	 * @return Boolean indicando sucesso da atualização ou Exception se o item for inválido
	 */
	function Update(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return UpdateItem(this.GetObjRef());
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
	function MoveToContainer(this, containerWrapped, x := -1, y := -1, add_to_existing_stack := 0)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(containerWrapped is @ContainerWrapper))
			return InvalidFormatException("The param containerWrapped must be a ContainerWrapper object!");
		endif

		return MoveItemToContainer(this.GetObjRef(), containerWrapped.GetObjRef(), x, y, add_to_existing_stack);
	endfunction

	function ToString(this)
		return $"Item {this.GetName()}:" +
			   $"Serial: {this.GetSerial()}, " +
			   $"ObjType: {this.GetObjType()}, " +
			   $"Graphic: {this.GetGraphic()}, " +
			   $"Color: {this.GetColor()}, " +
			   $"Weight: {this.GetWeight()}, " +
			   $"Category: {this.GetCategory()}";
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
