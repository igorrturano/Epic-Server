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

		// this._cfgFile := CFGFile("containers", "itemdesc");
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

	function GetWeightMultiplier(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.held_weight_multiplier;
	endfunction

	function GetMaxItems(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this._objRef.max_items_mod;
	endfunction

	function GetMaxSlots(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this._objRef.max_slots_mod;
	endfunction

	function GetMaxWeight(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.max_weight_mod;
	endfunction

	function GetItemsAmount(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return CInt(this._objRef.item_count - 1); // Subtrai 1 para não contar o próprio container
	endfunction
	
	function SetLocked(this, boolValue)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.locked := boolValue;

		return this;
	endfunction

	function SetWeightMultiplier(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.held_weight_multiplier := CDbl(value);

		return this;
	endfunction

	function SetMaxItems(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.max_items_mod := CInt(value);

		return this;
	endfunction

	function SetMaxSlots(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.max_slots_mod := CInt(value);

		return this;
	endfunction

	function SetMaxWeight(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this._objRef.max_weight_mod := CInt(value);

		return this;
	endfunction

	function IsLocked(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this._objRef.locked;
	endfunction

	function Contains(this, item, searchRecursively := false)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (!(item is @ItemWrapper))
			return InvalidFormatException("The param must be an item Wrapper!");
		endif

		return FindObjtypeInContainer(this.GetObjRef(), item.GetObjType(), searchRecursively ? FINDOBJTYPE_RECURSIVE : FINDOBJTYPE_ROOT_ONLY) != error;
	endfunction

	/*
	 * Obtém um item do contêiner pelo tipo de objeto
	 *
	 * @param objType     Tipo do objeto a ser procurado no contêiner
	 * @param flag        Sinalizador de busca (padrão: FINDOBJTYPE_ROOT_ONLY)
	 *                    Define o escopo da busca no contêiner
	 *
	 * @return ItemWrapper Retorna um ItemWrapper contendo o objeto encontrado,
	 *                     ou um wrapper vazio se nenhum item for encontrado
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
