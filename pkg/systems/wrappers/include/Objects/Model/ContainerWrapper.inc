use uo;

include ":wrappers:Objects/Exceptions/_all";
include "system/exception/FormatException/InvalidFormatException";

include "IContainerWrapper";
include "ItemWrapper";

include ":io:CFGFile";

class ContainerWrapper(IContainerWrapper, ItemWrapper)
	var _isLocked := false;

	function ContainerWrapper(this, byRef object)
		Super(object);

		this._template := this._cfgFile.SetElementName(this._objtype).GetElementValue();
		this._category := this._template.Category;

		if (object.isA(POLCLASS_CONTAINER))
			this._weightMultiplier := this._objRef.held_weight_multiplier;
			this._maxItems := this._objRef.max_items_mod;
			this._maxSlots := this._objRef.max_slots_mod;
			this._maxWeight := this._objRef.max_weight_mod;
		else // Caso seja template ou erro
			var err;
			if (!this._template)
				err := InvalidFormatException("The param must be a Container ObjRef or a valid Template!");
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
			endif

			this._objRef := err;
			this._serial := err;
		endif
	endfunction

	/**
	 * Obtém o multiplicador de peso do container
	 * 
	 * Este valor afeta quanto do peso dos itens dentro do container
	 * é considerado no peso total do container.
	 * 
	 * @return Double O multiplicador de peso ou Exception se houver erro
	 */
	function GetWeightMultiplier(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.held_weight_multiplier;
	endfunction

	/**
	 * Obtém o número máximo de itens permitidos no container
	 * 
	 * @return Integer Quantidade máxima de itens ou Exception se houver erro
	 */
	function GetMaxItems(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this._objRef.max_items_mod;
	endfunction

	/**
	 * Obtém o número máximo de slots disponíveis no container
	 * 
	 * @return Integer Quantidade máxima de slots ou Exception se houver erro
	 */
	function GetMaxSlots(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this._objRef.max_slots_mod;
	endfunction

	/**
	 * Obtém o peso máximo que o container pode suportar
	 * 
	 * @return Integer Peso máximo em stones ou Exception se houver erro
	 */
	function GetMaxWeight(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.max_weight_mod;
	endfunction

	/**
	 * Obtém a quantidade atual de itens dentro do container
	 * 
	 * Retorna a contagem total de itens, excluindo o próprio container.
	 * 
	 * @return Integer Quantidade de itens ou Exception se houver erro
	 */
	function GetItemsAmount(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return CInt(this._objRef.item_count - 1); // Subtrai 1 para não contar o próprio container
	endfunction
	
	/**
	 * Define se o container está bloqueado
	 * 
	 * @param boolValue Boolean indicando se deve bloquear (true) ou desbloquear (false)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetLocked(this, boolValue)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.locked := boolValue;

		return this;
	endfunction

	/**
	 * Define o multiplicador de peso do container
	 * 
	 * @param value Novo valor do multiplicador (será convertido para Double)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetWeightMultiplier(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.held_weight_multiplier := CDbl(value);

		return this;
	endfunction

	/**
	 * Define o número máximo de itens permitidos no container
	 * 
	 * @param value Novo valor máximo de itens (será convertido para Integer)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetMaxItems(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.max_items_mod := CInt(value);

		return this;
	endfunction

	/**
	 * Define o número máximo de slots disponíveis no container
	 * 
	 * @param value Novo valor máximo de slots (será convertido para Integer)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetMaxSlots(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.max_slots_mod := CInt(value);

		return this;
	endfunction

	/**
	 * Define o peso máximo que o container pode suportar
	 * 
	 * @param value Novo valor de peso máximo em stones (será convertido para Integer)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetMaxWeight(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.max_weight_mod := CInt(value);

		return this;
	endfunction

	/**
	 * Verifica se o container está bloqueado
	 * 
	 * @return Boolean true se bloqueado, false caso contrário, ou Exception se houver erro
	 */
	function IsLocked(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this._objRef.locked;
	endfunction

	/**
	 * Verifica se o container contém um item do tipo especificado
	 * 
	 * @param item ItemWrapper do tipo de item a ser procurado
	 * @param searchRecursively Boolean indicando se deve buscar recursivamente (dentro dos containers
	 							que estão dentro dele) [padrão: false]
	 * @return Boolean true se contém, false caso contrário, ou Exception se houver erro
	 * @throws InvalidFormatException quando o parâmetro não é um ItemWrapper
	 */
	function Contains(this, item, searchRecursively := false)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (!(item is @ItemWrapper))
			return InvalidFormatException("The param must be an item Wrapper!");
		endif

		return FindObjtypeInContainer(this.GetObjRef(), item.GetObjType(), searchRecursively ? FINDOBJTYPE_RECURSIVE : FINDOBJTYPE_ROOT_ONLY) != error;
	endfunction

	/**
	 * Obtém um item do container pelo tipo de objeto
	 * 
	 * Procura no container por um item do tipo especificado e retorna
	 * um wrapper contendo o primeiro item encontrado.
	 * 
	 * @param item ItemWrapper do tipo de item a ser procurado
	 *
	 * @param searchRecursively Boolean indicando se deve buscar recursivamente (dentro dos containers
	 							que estão dentro dele) [padrão: false]
	 *
	 * @return ItemWrapper Wrapper contendo o objeto encontrado ou vazio se não encontrado,
	 *                     ou Exception se houver erro
	 *
	 * @throws InvalidFormatException quando o parâmetro não é um ItemWrapper
	 */
	function GetInside(this, item, searchRecursively := false)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		if (!(item is @ItemWrapper))
			return InvalidFormatException("The param must be an Item Wrapper!");
		endif

		return ItemWrapper(FindObjtypeInContainer(this.GetObjRef(), item.GetObjType(), searchRecursively ? FINDOBJTYPE_RECURSIVE : FINDOBJTYPE_ROOT_ONLY));
	endfunction

	function ToString(this)
		return "Container: " +
			   $"Serial: {Hex(this._serial)}, " +
			   $"ObjType: {Hex(this._objType)}, " +
			   $"Name: {this._name}, " +
			   $"Graphic: {Hex(this._graphic)}, " +
			   $"Color: {Hex(this._color)}, " +
			   $"Weight: {this._weight}, " +
			   $"MaxItems: {this.GetMaxItems()}, " +
			   $"MaxSlots: {this.GetMaxSlots()}, " +
			   $"MaxWeight: {this.GetMaxWeight()}, " +
			   $"WeightMultiplier: {this.GetWeightMultiplier()}, " +
			   $"IsLocked: {this.IsLocked()}, " +
			   $"ItemsAmount: {this.GetItemsAmount()}";
	endfunction
endclass
