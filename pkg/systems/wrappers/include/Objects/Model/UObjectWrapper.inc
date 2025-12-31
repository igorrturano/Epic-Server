use uo;

include ":wrappers:Objects/Exceptions/_all";
include "system/exception/FormatException/InvalidFormatException";

include "IUObjectWrapper";

include ":wrappers:Position/Model/PositionWrapper";
include "include/packets";
include "system/text/Color";

class UObjectWrapper(IUObjectWrapper)
	var _cfgFile;
	var _template;
	var _objRef; // ObjRef
	var _serial := 0x0;
	var _objType := 0x0;
	var _name := "";
	var _graphic := 0;
	var _color := 0;
	var _weight := 0;
	var _category := ""; // CATEGORY enum
	var _cProps;

	function UObjectWrapper(this, byRef object)
		if (TypeOfInt(object) == OT_MOBILEREF || TypeOfInt(object) == OT_ITEMREF)
			this._objRef := object;
			this._serial := object.serial;
			this._objType := object.objtype;
			this._name := object.name;
			this._graphic := object.graphic;
			this._color := object.color;
			this._weight := object.weight;
		elseif (TypeOfInt(object) == OT_CONFIGELEMREF)
			this._template := object;
			this._cProps := object.CProps;
		elseif (object.isVendorTemplate)
			this._objType := object.objtype;
			this._name := object.name;
			this._graphic := object.graphic;
			this._color := object.color;
			this._weight := object.weight;
			this._cProps := object.CProps;
		else
			var objRefBySerial := SystemFindObjectBySerial(object);
			if (objRefBySerial)
				object := objRefBySerial;
				this._objRef := object;
				this._serial := object.serial;
				this._objType := object.objtype;
				this._name := object.name;
				this._graphic := object.graphic;
				this._color := object.color;
				this._weight := object.weight;
			else
				this._objType := object;
			endif
		endif
	endfunction

	function GetCfgFile(this)
		return this._cfgFile;
	endfunction
	
	function GetTemplate(this)
		return this._template;
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

	function GetColor(this)
		return this._color;
	endfunction

	function GetWeight(this)
		return this._weight;
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

	/**
	 * Obtém a categoria do objeto.
	 *
	 * @return String contendo a categoria do objeto (valor do enum CATEGORY das classes filhas)
	 */
	function GetCategory(this)
		return this._category;
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
	
	/**
	 * Obtém o mapa de propriedades personalizadas do template.
	 *
	 * @return Mapa contendo as propriedades personalizadas do template
	 */
	function GetCProps(this)
		return this._cProps;
	endfunction

	function GetProperty(this, propName)
		if (this._objRef is @Exception && !this.GetCProps())
			return this._objRef;
		endif

		return this.GetObjRef() ? this.GetObjRef().GetProp(propName) : this.GetCProps()[propName];
	endfunction

	function SetProperty(this, propName, propValue)
		if (this._objRef is @Exception && !this.GetCProps())
			return this._objRef;
		endif

		return this.GetObjRef() ? this.GetObjRef().SetProp(propName, propValue) : this.GetCProps().insert(propName, propValue);

		return this;
	endfunction

	function EraseProperty(this, propName)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		this.GetObjRef() ? this.GetObjRef().EraseProp(propName) :  this.GetCProps().erase(propName);

		return this;
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
	function PlayEffectTo(this, targetWrapped, effectId, speed := 10, loop := 0, explode := false, color := 0, translucent := false)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (color)
			if (color is @Color)
				color := color.GetValue();
			else
				return InvalidFormatException("The param color must be a Color object!");
			endif
		endif

		PlayMovingEffectXYZHued(this._objRef, targetWrapped.GetObjRef(), effectId, speed, loop, explode, color, translucent);

		return this;
	endfunction

	/**
	 * Reproduz um efeito visual centralizado no objeto.
	 *
	 * @param effectId O identificador do efeito a ser reproduzido
	 * @param speed A velocidade de reprodução do efeito (padrão: 1)
	 * @param loop Define se o efeito deve repetir (0 = não repete, padrão: 0)
	 * @param color A cor do efeito (padrão: 0)
	 * @param translucent Define se o efeito é translúcido (padrão: false)
	 *
	 * @return Retorna o próprio objeto para encadeamento de métodos, ou uma exceção se a referência do objeto for inválida
	 */
	function PlayEffect(this, effectId, speed := 1, loop := 0, color := 0, translucent := false)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (color)
			if (color is @Color)
				color := color.GetValue();
			else
				return InvalidFormatException("The param color must be a Color object!");
			endif
		endif

		PlayObjectCenteredEffectEx(this._objRef, effectId, speed, loop, color, translucent);

		return this;
	endfunction

	function ToString(this)
		return "UObject: " +
			$"Serial: {this._serial}, " +
			$"ObjType: {this._objType}, " +
			$"Name: {this._name}, " +
			$"Graphic: {this._graphic}, " +
			$"Color: {this._color}";
	endfunction
endclass
