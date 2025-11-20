use uo;

include "IContainerWrapper";
include "ItemWrapper";

class ContainerWrapper(IContainerWrapper, ItemWrapper)
	var _weightMultiplier := 1.0;
	var _maxItems := 0;
	var _maxSlots := 0;
	var _maxWeight := 0;
	var _isLocked := false;

	function ContainerWrapper(this, byRef objRef)
		Super(objRef);
	endfunction

	function GetWeightMultiplier(this)
		return this._weightMultiplier;
	endfunction

	function GetMaxItems(this)
		return this._maxItems;
	endfunction

	function GetMaxSlots(this)
		return this._maxSlots;
	endfunction

	function GetMaxWeight(this)
		return this._maxWeight;
	endfunction
	
	function SetLocked(this, boolValue)
		this._isLocked := boolValue;
		return this;
	endfunction

	function SetWeightMultiplier(this, value)
		this._weightMultiplier := value;
		return this;
	endfunction

	function SetMaxItems(this, value)
		this._maxItems := value;
		return this;
	endfunction

	function SetMaxSlots(this, value)
		this._maxSlots := value;
		return this;
	endfunction

	function SetMaxWeight(this, value)
		this._maxWeight := value;
		return this;
	endfunction

	function IsLocked(this)
		return this._isLocked;
	endfunction

	/*
	 * Verifica se um determinado tipo de objeto existe dentro do contêiner.
	 *
	 * @param {ItemWrapper} object - O objeto ItemWrapper a ser procurado no contêiner.
	 * @param {Integer} flag - (Opcional) Flag de busca. Padrão: FINDOBJTYPE_ROOT_ONLY.
	 *                         Define o escopo da busca dentro do contêiner.
	 *						   POSSÍVEIS VALORES: FINDOBJTYPE_ROOT_ONLY, FINDOBJTYPE_IGNORE_LOCKED,
	 											  FINDOBJTYPE_RECURSIVE.
	 *
	 * @return {Integer|Error} Retorna a quantidade de objetos encontrados do tipo especificado,
	 *                         ou um objeto de erro se o parâmetro object não for um ItemWrapper.
	 *
	 * @error errortext - "The object must be an ItemWrapper." quando o objeto fornecido
	 *                    não é uma instância de ItemWrapper.
	 */
	function Contains(this, object, flag := FINDOBJTYPE_ROOT_ONLY)
		if (!(object is @ItemWrapper))
			return error{errortext := "The object must be an ItemWrapper."};
		endif

		return FindObjtypeInContainer(this.GetObjRef(), object.GetObjType(), flag);
	endfunction


	
endclass
