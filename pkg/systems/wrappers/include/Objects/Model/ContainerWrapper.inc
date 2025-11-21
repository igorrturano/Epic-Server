use uo;

include ":wrappers:Objects/Exceptions/_all";
include "system/exception/FormatException/InvalidFormatException";

include "IContainerWrapper";
include "ItemWrapper";

include ":io:CFGFile";

class ContainerWrapper(IContainerWrapper, ItemWrapper)
	var _isLocked := false;

	function ContainerWrapper(this, byRef objRef)
		Super(objRef);

		this._cfgFile := CFGFile("containers", "itemdesc");
		if (objRef.isA(POLCLASS_CONTAINER))
			this._template := this._cfgFile.SetElementName(this._objtype).GetElementValue();
			this._name := this._objRef.name;
			this._weightMultiplier := this._objRef.held_weight_multiplier;
			this._maxItems := this._objRef.max_items_mod;
			this._maxSlots := this._objRef.max_slots_mod;
			this._maxWeight := this._objRef.max_weight_mod;
		else // Caso seja apenas um template
			var err;
			if (!this._template)
				err := InvalidFormatException("The param must be a Container ObjRef or a valid Template!");
				this._objType := err;
			else
				err := IsTemplateException();
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
		
		return this._objRef != error ? this._objRef.max_items_mod : this._template.MaxItems;
	endfunction

	function GetMaxSlots(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this._objRef != error ? this._objRef.max_slots_mod : this._template.MaxSlots;
	endfunction

	function GetMaxWeight(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef != error ? this._objRef.max_weight_mod : this._template.MaxWeight;
	endfunction

	function GetItemsAmount(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return CInt(this._objRef.item_count - 1); // Subtrai 1 para não contar o próprio container
	endfunction
	
	function SetLocked(this, boolValue)
		if (this._objRef == error) return this._objRef; endif
		
		this._objRef.locked := boolValue;

		return this;
	endfunction

	function SetWeightMultiplier(this, value)
		if (this._objRef == error) return this._objRef; endif
		
		this._objRef.held_weight_multiplier := CDbl(value);

		return this;
	endfunction

	function SetMaxItems(this, value)
		if (this._objRef == error) return this._objRef; endif
		
		this._objRef.max_items_mod := CInt(value);

		return this;
	endfunction

	function SetMaxSlots(this, value)
		if (this._objRef == error) return this._objRef; endif
		
		this._objRef.max_slots_mod := CInt(value);

		return this;
	endfunction

	function SetMaxWeight(this, value)
		if (this._objRef == error) return this._objRef; endif
		
		this._objRef.max_weight_mod := CInt(value);

		return this;
	endfunction

	function IsLocked(this)
		if (this._objRef == error) return this._objRef; endif
		
		return Boolean(this._objRef.locked);
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
	function GetItemInside(this, item, searchRecursively := false)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		var flag := searchRecursively ? FINDOBJTYPE_RECURSIVE : FINDOBJTYPE_ROOT_ONLY;

		return ItemWrapper(FindObjtypeInContainer(this.GetObjRef(), item.GetObjType(), flag));
	endfunction
endclass
