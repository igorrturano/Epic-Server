use uo;

include ":wrappers:Objects/Exceptions/_all";
include "system/exception/FormatException/InvalidFormatException";

include "IContainerWrapper";
include "ItemWrapper";

include ":io:CFGFile";

class ContainerWrapper(IContainerWrapper, ItemWrapper)
	var IsLockedVar := false;

	function ContainerWrapper(this, byRef object)
		Super(object);

		this.Template := this.GetCfgFile().SetElementName(this.ObjType).GetElement();
		this.Category := this.GetTemplate().Category;

		if (this.GetObjRef().isA(POLCLASS_CONTAINER))
			this.WeightMultiplier := this.GetObjRef().held_weight_multiplier;
			this.MaxItems := this.GetObjRef().max_items_mod;
			this.MaxSlots := this.GetObjRef().max_slots_mod;
			this.MaxWeight := this.GetObjRef().max_weight_mod;
		else // Caso seja template ou erro
			var err;
			if (!this.GetTemplate())
				err := InvalidFormatException("The param must be a Container ObjRef or a valid Template!");
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
			endif

			this.ObjRef := err;
			this.Serial := err;
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().held_weight_multiplier;
	endfunction

	/**
	 * Obtém o número máximo de itens permitidos no container
	 * 
	 * @return Integer Quantidade máxima de itens ou Exception se houver erro
	 */
	function GetMaxItems(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return this.GetObjRef().max_items_mod;
	endfunction

	/**
	 * Obtém o número máximo de slots disponíveis no container
	 * 
	 * @return Integer Quantidade máxima de slots ou Exception se houver erro
	 */
	function GetMaxSlots(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return this.GetObjRef().max_slots_mod;
	endfunction

	/**
	 * Obtém o peso máximo que o container pode suportar
	 * 
	 * @return Integer Peso máximo em stones ou Exception se houver erro
	 */
	function GetMaxWeight(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().max_weight_mod;
	endfunction

	/**
	 * Obtém a quantidade atual de itens dentro do container
	 * 
	 * Retorna a contagem total de itens, excluindo o próprio container.
	 * 
	 * @return Integer Quantidade de itens ou Exception se houver erro
	 */
	function GetItemsAmount(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return CInt(this.GetObjRef().item_count - 1); // Subtrai 1 para não contar o próprio container
	endfunction
	
	/**
	 * Define se o container está bloqueado
	 * 
	 * @param boolValue Boolean indicando se deve bloquear (true) ou desbloquear (false)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetLocked(this, boolValue)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		this.GetObjRef().locked := boolValue;

		return this;
	endfunction

	/**
	 * Define o multiplicador de peso do container
	 * 
	 * @param value Novo valor do multiplicador (será convertido para Double)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetWeightMultiplier(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		this.GetObjRef().held_weight_multiplier := CDbl(value);

		return this;
	endfunction

	/**
	 * Define o número máximo de itens permitidos no container
	 * 
	 * @param value Novo valor máximo de itens (será convertido para Integer)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetMaxItems(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		this.GetObjRef().max_items_mod := CInt(value);

		return this;
	endfunction

	/**
	 * Define o número máximo de slots disponíveis no container
	 * 
	 * @param value Novo valor máximo de slots (será convertido para Integer)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetMaxSlots(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		this.GetObjRef().max_slots_mod := CInt(value);

		return this;
	endfunction

	/**
	 * Define o peso máximo que o container pode suportar
	 * 
	 * @param value Novo valor de peso máximo em stones (será convertido para Integer)
	 * @return ContainerWrapper Retorna a própria instância para encadeamento ou Exception
	 */
	function SetMaxWeight(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		this.GetObjRef().max_weight_mod := CInt(value);

		return this;
	endfunction

	/**
	 * Verifica se o container está bloqueado
	 * 
	 * @return Boolean true se bloqueado, false caso contrário, ou Exception se houver erro
	 */
	function IsLocked(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return this.GetObjRef().locked;
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		if (!(item is @ItemWrapper))
			return InvalidFormatException("The param must be an Item Wrapper!");
		endif

		return ItemWrapper(FindObjtypeInContainer(this.GetObjRef(), item.GetObjType(), searchRecursively ? FINDOBJTYPE_RECURSIVE : FINDOBJTYPE_ROOT_ONLY));
	endfunction

	function ToString(this)
		return "Container: " +
			   $"Serial: {Hex(this.GetSerial())}, " +
			   $"ObjType: {Hex(this.GetObjType())}, " +
			   $"Name: {this.GetName()}, " +
			   $"Graphic: {Hex(this.GetGraphic())}, " +
			   $"Color: {Hex(this.GetColor())}, " +
			   $"Weight: {this.GetWeight()}, " +
			   $"MaxItems: {this.GetMaxItems()}, " +
			   $"MaxSlots: {this.GetMaxSlots()}, " +
			   $"MaxWeight: {this.GetMaxWeight()}, " +
			   $"WeightMultiplier: {this.GetWeightMultiplier()}, " +
			   $"IsLocked: {this.IsLocked()}, " +
			   $"ItemsAmount: {this.GetItemsAmount()}";
	endfunction
endclass
