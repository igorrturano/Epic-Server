use uo;

include ":wrappers:Objects/Exceptions/_all";

include "IUObjectWrapper";

include ":wrappers:Position/Model/PositionWrapper";
include "include/packets";

class UObjectWrapper(IUObjectWrapper)
	var _cfgFile;
	var _template;
	var _objRef; // ObjRef
	var _serial := 0x0;
	var _objType := 0x0;
	var _name := "";
	var _graphic := 0;
	var _color := 0;

	function UObjectWrapper(this, byRef object)
		if (TypeOfInt(object) == OT_MOBILEREF || TypeOfInt(object) == OT_ITEMREF)
			this._objRef := object;
			this._serial := object.serial;
			this._objType := object.objtype;
			this._name := object.name;
			this._graphic := object.graphic;
			this._color := object.color;
		else
			this._objType := object;

			var err := IsTemplateException();
			this._objRef := err;
			this._serial := err;

			err := InvalidTemplateException("Template not loaded properly!");
			this._cfgFile := err;
			this._template := err;
			this._name := err;
			this._graphic := err;
			this._color := err;
		endif
	endfunction

	function GetObjRef(this)
		return this._objRef;
	endfunction

	function GetSerial(this)
		return this._serial;
	endfunction

	function GetObjType(this)
		return this._objType;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetGraphic(this)
		return this._graphic;
	endfunction

	function _SetName(this, name)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		this._name := name;
		this._objRef.name := name;

		return this;
	endfunction

	function SetGraphic(this, graphic)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		this._graphic := graphic;
		this._objRef.graphic := graphic;

		return this;
	endfunction

	function SetColor(this, color)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		this._color := color;
		this._objRef.color := color;

		return this;
	endfunction

	function GetPosition(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return PositionWrapper(
			this._objRef.x,
			this._objRef.y,
			this._objRef.z,
			this._objRef.realm
		);
	endfunction

	function GetMobilesInLineOfSight(this, range)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return ListMobilesInLineOfSight(this._objRef, CInt(range));
	endfunction
	
	/**
	 * Move o objeto para uma localização especificada
	 * 
	 * @param {Object} this - Referência ao objeto wrapper
	 * @param {Position} position - Objeto de posição contendo coordenadas X, Y, Z e Realm
	 * @param {Integer} flags - Flags de movimento (padrão: MOVEOBJECT_NORMAL)
	 * 
	 * @return {Integer} Retorna o resultado da operação de movimento
	 * 
	 * @example
	 * var resultado := obj.MoveToLocation(novaPosicao, MOVEOBJECT_FORCELOCATION);
	 */
	function MoveToLocation(this, position, flags := MOVEOBJECT_NORMAL)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return MoveObjectToLocation(this._objRef, position.GetX(), position.GetY(), position.GetZ(), position.GetRealm(), flags);
	endfunction

	/**
	 * Reproduz um efeito sonoro no objeto.
	 *
	 * @param this Referência à instância do objeto wrapper
	 * @param soundId ID do efeito sonoro a ser reproduzido
	 * @return Retorna a própria instância (this) para permitir encadeamento de métodos
	 */
	function PlaySound(this, soundId)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		PlaySoundEffect(this._objRef, soundId);

		return this;
	endfunction

	/**
	 * Reproduz um efeito sonoro privado para o objeto
	 * 
	 * @param this Referência ao objeto wrapper atual
	 * @param soundId ID do som a ser reproduzido
	 * @param soundSource Fonte do som (opcional). Se não fornecido, usa a referência do próprio objeto
	 * 
	 * @return Retorna a instância atual do objeto (this) para permitir encadeamento de métodos
	 */
	function PlaySoundPrivate(this, soundId, soundSource := false)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		PlaySoundEffectPrivate(this._objRef, soundId, soundSource ?: this._objRef);

		return this;
	endfunction

	/**
	 * Reproduz um efeito visual animado que se move do objeto atual até um objeto alvo.
	 *
	 * @param this             Referência ao objeto wrapper atual (origem do efeito)
	 * @param targetWrapped    Objeto wrapper de destino para onde o efeito será direcionado
	 * @param effectId         ID do efeito visual a ser reproduzido
	 * @param speed            Velocidade do movimento do efeito (padrão: 10)
	 * @param loop             Define se o efeito deve repetir em loop (padrão: 0 - não repete)
	 * @param explode          Define se o efeito deve explodir ao atingir o alvo (padrão: 0 - não explode)
	 * @param color            Cor do efeito em formato numérico (padrão: 0)
	 * @param translucent      Define se o efeito é translúcido (padrão: 0 - não translúcido)
	 *
	 * @return this            Retorna a própria instância do objeto para permitir encadeamento de métodos
	 */
	function PlayEffectTo(this, targetWrapped, effectId, speed := 10, loop := 0, explode := 0, color := 0, translucent := 0)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		PlayMovingEffectXYZHued(this._objRef, targetWrapped.GetObjRef(), effectId, speed, loop, explode, color, translucent);

		return this;
	endfunction

	function ToString(this)
		return "UObject: " +
			$"Serial: {Hex(this._serial)}, " +
			$"ObjType: {Hex(this._objType)}, " +
			$"Name: {this._name}, " +
			$"Graphic: {Hex(this._graphic)}, " +
			$"Color: {Hex(this._color)}";
	endfunction
endclass
