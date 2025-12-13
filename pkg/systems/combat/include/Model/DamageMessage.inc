include ":wrappers:Objects/Model/UObjectWrapper";

class DamageMessage()
	var _source;
	var _target;
	var _damageAmount;
	var _damageType;
	var _isCritical;

	function DamageMessage(this, source, target, damageAmount, damageType, isCriticalDamage := false)
		this._source := source;
		this._target := target;
		this._damageAmount := CInt(damageAmount);
		this._damageType := CStr(damageType);
		this._isCritical := Boolean(isCriticalDamage);
	endfunction

	function GetSource(this)
		return this._source;
	endfunction

	function SetSource(this, wrapper)
		if (!(wrapper is @UObjectWrapper))
			return InvalidFormatException("The param wrapper must be a UObjectWrapper object!");
		endif
		
		this.source := wrapper;
	endfunction

	function GetTarget(this)
		return this._target;
	endfunction

	function SetTarget(this, wrapper)
		if (!(wrapper is @UObjectWrapper))
			return InvalidFormatException("The param wrapper must be a UObjectWrapper object!");
		endif
		
		this._target := wrapper;
	endfunction

	function GetDamageAmount(this)
		return this._damageAmount;
	endfunction

	function SetDamageAmount(this, value)
		this._damageAmount := CInt(value);
	endfunction

	function GetDamageType(this)
		return this._damageType;
	endfunction

	/**
	 * Define o tipo de dano da mensagem.
	 * 
	 * @param damageType Tipo de dano (DAMAGE_TYPE enum) a ser definido
	 *
	 * @return void
	 */
	function SetDamageType(this, damageType)
		this._damageType := CStr(damageType);
	endfunction

	function IsCritical(this)
		return this._isCritical;
	endfunction

	function SetCritical(this, value)
		this._isCritical := Boolean(value);
	endfunction

	function ToString(this)
		return $"{this._damageAmount} [{this._damageType}]";
	endfunction
endclass